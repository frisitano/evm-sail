#!/usr/bin/env bash
# ===========================================================================
# Build evm-sail as a GMP-free guest for the eth-act zkVM RISC-V
# standard target (riscv64im_zicclsm-unknown-none-elf) and (optionally) run it
# on spike.
#
#   VEC=<input> ./build.sh guest  - build the evm-sail block guest ELF
#   VEC=<input> ./build.sh run    - build the guest and run it on spike
#   ./build.sh clean              - remove build artifacts
#
# VEC = a raw schema-prefixed SszStatelessInput file, baked into the ELF as
# the private input. The canonical driver is harness/run.py
# --guest --spike (it supplies VEC per fixture and REBAKE_ONLY after the
# first build).
#
# Requires: sail (opam), riscv64-unknown-elf-gcc, spike.
# ===========================================================================
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RT="$HERE/runtime"
# Override for concurrency: two gate runs sharing one build dir race on the
# baked input vector + relinked ELF (run.py --guest --spike isolates itself).
BUILD="${ZKVM_BUILD:-$HERE/build}"
ROOT="$(cd "$HERE/.." && pwd)"

SAIL="${SAIL:-sail}"
GCC="${GCC:-riscv64-unknown-elf-gcc}"
SPIKE="${SPIKE:-spike}"
HOSTCC="${HOSTCC:-cc}"
HOSTCXX="${HOSTCXX:-c++}"
SPIKE_INC="${SPIKE_INC:-/opt/homebrew/Cellar/riscv-isa-sim/main/include}"
ACCEL_SO="$BUILD/accel_device.so"

# Standard target: RV64IM + Zicclsm, LP64 soft-float, machine mode, freestanding.
ARCH=(-march=rv64im_zicclsm -mabi=lp64 -mcmodel=medany)
# Freestanding includes FIRST so <stdio.h>/<stdlib.h>/<string.h>/<gmp.h> resolve
# to our shims + vendored mini-gmp instead of newlib / libgmp.
CFLAGS=("${ARCH[@]}" -O2 -ffreestanding -nostdlib -fno-builtin
        -fno-stack-protector -fno-pic -mno-relax -DNDEBUG
        -ffunction-sections -fdata-sections
        -I"$RT/sailfix" -I"$RT/freestanding" -I"$RT" -I"$ROOT/zkvm" -I"$ROOT/ffi")
# -lgcc supplies compiler runtime helpers; --gc-sections drops the unused Sail
# diagnostic/format surface (and its gmp_printf/asprintf references).
LDFLAGS=(-T "$RT/link.ld" -Wl,--no-relax -Wl,--gc-sections -nostdlib -static)

# Sail nostd runtime library (ships with the toolchain).
sail_lib() {
  local sb; sb="$(command -v "$SAIL")"
  echo "$(dirname "$sb")/../share/sail/lib"
}

# spike memory ranges MUST match link.ld: MAIN, then (omitted guard gap), STACK.
SPIKE_MEM="0x80000000:0x10000000,0x90010000:0x04000000"
# spike's --isa string does not name Zicclsm; that extension only mandates
# transparent misaligned load/store support, which spike provides via
# --misaligned. So rv64im + --misaligned is the Zicclsm-equivalent run config.
SPIKE_ISA="rv64im_zicntr"   # zicntr enables instret for the harness cost readout
SPIKE_FLAGS=(--isa="$SPIKE_ISA" --misaligned -m"$SPIKE_MEM" --extlib="$ACCEL_SO" --device=accel)

mkdir -p "$BUILD"

build_runtime() {
  # start.S is the machine-mode platform crt0 + trap vector (vendor/platform
  # glue, not the proven STF). It uses Zicsr CSR ops (mtvec/mcause/...) to map
  # guard-region / misaligned faults to standardized abnormal termination, so it
  # is assembled with Zicsr added. The compiled model stays rv64im_zicclsm.
  "$GCC" -march=rv64im_zicsr_zicclsm -mabi=lp64 -mcmodel=medany \
      -ffreestanding -nostdlib -fno-builtin -fno-pic -mno-relax \
      -I"$RT/freestanding" -I"$RT" -Wall -Wextra \
      -c "$RT/start.S" -o "$BUILD/start.o"
  "$GCC" "${CFLAGS[@]}" -Wall -Wextra -c "$RT/htif.c"  -o "$BUILD/htif.o"
}

cmd_guest() {
  local lib; lib="$(sail_lib)"
  build_runtime
  # 0. Embed the schema-prefixed SSZ test vector as the preloaded private input
  #    (zkvm_input_bytes). A real zkVM host supplies these bytes instead. The
  #    canonical driver is harness/run.py --guest --spike, which always
  #    passes VEC (a raw statelessInputBytes file).
  local vec="${VEC:-}"
  if [ -z "$vec" ]; then
    echo "error: set VEC=<raw SszStatelessInput file> (see run.py --guest --spike)" >&2
    exit 2
  fi
  python3 - "$vec" "$BUILD/zkvm_input_data.c" <<'PY'
import sys
data = open(sys.argv[1], "rb").read()
with open(sys.argv[2], "w") as f:
    f.write("/* generated from the SSZ test vector; do not edit */\n")
    f.write("const unsigned char zkvm_input_bytes[] = {\n")
    for i in range(0, len(data), 16):
        f.write("  " + ",".join(str(b) for b in data[i:i+16]) + ",\n")
    f.write("};\n")
    f.write("const unsigned long zkvm_input_bytes_len = %d;\n" % len(data))
PY
  "$GCC" "${CFLAGS[@]}" -Wno-unused -c "$BUILD/zkvm_input_data.c" -o "$BUILD/zkvm_input_data.o"
  # REBAKE_ONLY=1: reuse every compiled object and only rebake the input vector
  # and relink -- the per-vector fast path for the EEST guest smoke gate.
  if [ -n "${REBAKE_ONLY:-}" ] && [ -f "$BUILD/zkvm_block.o" ]; then
    link_guest
    return
  fi
  # 1. Sail -> C: no main, no Sail runtime harness (we supply our own).
  #    --c-include injects the guest extern decls (host crypto/precompile
  #    adapters + zkvm_input private-input reader) so the generated call sites
  #    compile.
  if [ -n "${GUEST:-}" ]; then
    "$SAIL" -c --c-no-main --c-no-rts --c-preserve main \
        --c-include zkvm_input.h \
        "$GUEST" -o "$BUILD/zkvm_block"
  else
    local guest_entry="${GUEST_ENTRY:-guest}"
    ( cd "$ROOT" && "$SAIL" -c --c-no-main --c-no-rts --c-preserve main \
        --c-include zkvm_input.h \
        sail/evm.sail_project evm \
        --variable EVM_ENTRY="$guest_entry" \
        -o "$BUILD/zkvm_block" )
  fi
  # 2. Compile the generated model (stock Sail GMP-ABI, backed by mini-gmp).
  #    The model calls setup_rts/cleanup_rts (provided by runtime.c) without a
  #    prototype since --c-no-rts omits rts.h; downgrade that to a warning.
  "$GCC" "${CFLAGS[@]}" -I"$lib" \
      -Wno-unused -Wno-error=implicit-function-declaration \
      -c "$BUILD/zkvm_block.c" -o "$BUILD/zkvm_block.o"
  # 2b. journal glue: JEntry crosses the extern boundary as the GENERATED
  #     struct zJEntry, so the glue compiles against this build's model header.
  "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
      -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$ROOT/ffi/journal_glue.c" -o "$BUILD/journal_glue.o"
  "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
      -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$ROOT/ffi/hash_glue.c" -o "$BUILD/hash_glue.o"
  "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
      -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$ROOT/ffi/code_glue.c" -o "$BUILD/code_glue.o"
  # 3. GMP-free fixed-width Sail runtime (sailfix) replacing stock sail.c +
  #    mini-gmp: sail_int = 512-bit sign-magnitude, lbits = 256-bit inline.
  "$GCC" "${CFLAGS[@]}" -I"$lib" \
      -Wno-unused -Wno-error=implicit-function-declaration \
      -c "$RT/sailfix/sail.c" -o "$BUILD/sail.o"
  # 3b. Host crypto/precompile adapters: identical code to the native build,
  #     written against the zkvm_accelerators.h API. On the guest that API is
  #     implemented by accel_guest.c, which marshals each call to the host
  #     accel device over MMIO (zkvm/zkvm_accel_mmio.h) -- so the crypto itself
  #     never executes as guest instructions and the guest links NO crypto
  #     code, while ffi/ stays free of any MMIO special-casing.
  for hc in host_crypto precompiles; do
    "$GCC" "${CFLAGS[@]}" -I"$lib" -I"$ROOT/ffi" \
        -Wno-unused -c "$ROOT/ffi/$hc.c" -o "$BUILD/$hc.o"
  done
  "$GCC" "${CFLAGS[@]}" -I"$ROOT/ffi" -I"$ROOT/zkvm" -Wall -Wextra \
      -c "$RT/accel_guest.c" -o "$BUILD/accel_guest.o"
  # 3b'. Host accelerator device (spike --extlib): models a zkVM crypto precompile,
  #      backed by the SAME Rust accel-host crypto the native build links
  #      (keccak/sha256/secp256k1 verify). The crypto never enters guest instret.
  ACCEL="$ROOT/zkvm/accel-host"; ACCEL_LIB="$ACCEL/target/release"
  if [ ! -f "$ACCEL_LIB/libzkvm_accel_host.dylib" ] && [ ! -f "$ACCEL_LIB/libzkvm_accel_host.so" ]; then
    ( cd "$ACCEL" && cargo build --release --target-dir target )  # local target (matches link path)
  fi
  "$HOSTCXX" -std=c++17 -fPIC -shared -I"$SPIKE_INC" -I"$ROOT/ffi" -I"$ROOT/zkvm" -undefined dynamic_lookup \
      -o "$ACCEL_SO" "$ROOT/zkvm/accel-device/accel_device.cc" \
      -L"$ACCEL_LIB" -lzkvm_accel_host -Wl,-rpath,"$ACCEL_LIB"
  # 3c. C host backends: memory/generic byte slices, transient storage,
  #     returndata, operand stack, code/JUMPDEST arenas, and witness/account
  #     databases.
  for hc in memory transient_storage state_db stack code_db kernel_state trie_node_db returndata; do
    "$GCC" "${CFLAGS[@]}" -I"$lib" \
        -Wno-unused -c "$ROOT/ffi/$hc.c" -o "$BUILD/$hc.o"
  done
  # 4. Our freestanding runtime + IO + harness.
  "$GCC" "${CFLAGS[@]}" -I"$lib" -Wall -Wextra \
      -c "$RT/runtime.c" -o "$BUILD/runtime.o"
  "$GCC" "${CFLAGS[@]}" -Wall -Wextra -c "$RT/zkvm_io.c"  -o "$BUILD/zkvm_io.o"
  # private-input reader FFI for the Sail SSZ decoder (needs sail.h types).
  "$GCC" "${CFLAGS[@]}" -I"$lib" -Wall -Wextra -c "$RT/zkvm_input.c" -o "$BUILD/zkvm_input.o"
  # harness reads minstret (rdinstret) to report block execution cost -> Zicsr
  # (platform glue, like start.S; the proven model stays rv64im_zicclsm).
  "$GCC" "${CFLAGS[@]}" -march=rv64im_zicsr_zicclsm -I"$lib" -Wall -Wextra \
      -c "$RT/harness.c" -o "$BUILD/harness.o"
  # 5. Link the static guest ELF with the vendor linker script.
  link_guest
}

link_guest() {
  "$GCC" "${CFLAGS[@]}" "${LDFLAGS[@]}" \
      "$BUILD/start.o" "$BUILD/htif.o" "$BUILD/zkvm_io.o" "$BUILD/zkvm_input.o" \
      "$BUILD/zkvm_input_data.o" \
      "$BUILD/runtime.o" "$BUILD/harness.o" "$BUILD/sail.o" \
      "$BUILD/host_crypto.o" "$BUILD/precompiles.o" "$BUILD/accel_guest.o" \
      "$BUILD/journal_glue.o" "$BUILD/hash_glue.o" "$BUILD/code_glue.o" \
      "$BUILD/memory.o" "$BUILD/transient_storage.o" "$BUILD/state_db.o" "$BUILD/stack.o" \
      "$BUILD/code_db.o" "$BUILD/kernel_state.o" "$BUILD/trie_node_db.o" "$BUILD/returndata.o" \
      "$BUILD/zkvm_block.o" \
      -o "$BUILD/zkvm_guest.elf"
  echo "built $BUILD/zkvm_guest.elf"
  riscv64-unknown-elf-size "$BUILD/zkvm_guest.elf" 2>/dev/null || true
}

cmd_run() {
  cmd_guest
  echo "--- spike run (${SPIKE_FLAGS[*]}) ---"
  set +e
  "$SPIKE" "${SPIKE_FLAGS[@]}" "$BUILD/zkvm_guest.elf"
  local rc=$?
  set -e
  echo "--- spike exit code: $rc ---"
  return $rc
}

cmd_clean() { rm -rf "$BUILD"; echo "cleaned"; }

# The one-time platform bring-up probes (derisk: HTIF smoke; traptest:
# guard-region enforcement) were deleted once the real guest gate
# (run.py --guest --spike) covered the platform end-to-end; recover them
# from git history if the platform glue (start.S / link.ld / htif.c) changes.
case "${1:-run}" in
  guest)    cmd_guest ;;
  run)      cmd_run ;;
  clean)    cmd_clean ;;
  *) echo "usage: $0 {guest|run|clean}"; exit 2 ;;
esac
