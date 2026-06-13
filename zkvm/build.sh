#!/usr/bin/env bash
# ===========================================================================
# Build the el-ir Sail EVM as a GMP-free guest for the eth-act zkVM RISC-V
# standard target (riscv64im_zicclsm-unknown-none-elf) and (optionally) run it
# on spike.
#
#   ./build.sh derisk   - build+run the HTIF de-risk program (harness check)
#   ./build.sh guest    - build the full el-ir block guest ELF
#   ./build.sh run       - build the guest and run it on spike
#   ./build.sh clean     - remove build artifacts
#
# Requires: sail (opam), riscv64-unknown-elf-gcc, spike.
# ===========================================================================
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RT="$HERE/runtime"
BUILD="$HERE/build"
ELIR="$(cd "$HERE/.." && pwd)"

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
        -I"$RT/sailfix" -I"$RT/freestanding" -I"$RT" -I"$ELIR/zkvm" -I"$ELIR/ffi")
# -lgcc supplies compiler runtime helpers; --gc-sections drops the unused Sail
# diagnostic/format surface (and its gmp_printf/asprintf references).
LDFLAGS=(-T "$RT/link.ld" -Wl,--no-relax -Wl,--gc-sections -nostdlib -static)

# Sail nostd runtime library (ships with the toolchain).
sail_lib() {
  local sb; sb="$(command -v "$SAIL")"
  echo "$(dirname "$sb")/../share/sail/lib"
}

# spike memory ranges MUST match link.ld: MAIN, then (omitted guard gap), STACK.
SPIKE_MEM="0x80000000:0x10000000,0x90010000:0x00100000"
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

cmd_derisk() {
  build_runtime
  "$GCC" "${CFLAGS[@]}" -Wall -Wextra -c "$RT/derisk_main.c" -o "$BUILD/derisk_main.o"
  "$GCC" "${CFLAGS[@]}" "${LDFLAGS[@]}" \
      "$BUILD/start.o" "$BUILD/htif.o" "$BUILD/derisk_main.o" -o "$BUILD/derisk.elf"
  echo "built $BUILD/derisk.elf"
  echo "--- spike run ---"
  "$SPIKE" "${SPIKE_FLAGS[@]}" "$BUILD/derisk.elf"
}

cmd_guest() {
  local lib; lib="$(sail_lib)"
  build_runtime
  # 0. Embed the schema-prefixed SSZ test vector as the preloaded private input
  #    (zkvm_input_bytes). A real zkVM host supplies these bytes instead.
  local vec="${VEC:-$ELIR/zkvm/vectors/fixture_block.ssz}"
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
  # 1. Sail -> C: no main, no Sail runtime harness (we supply our own).
  #    --c-include injects the guest extern decls (keccak accelerator +
  #    el_input private-input reader) so the generated call sites compile.
  "$SAIL" -c --c-no-main --c-no-rts --c-preserve zkvm_run \
      --c-include el_input.h \
      "${GUEST:-$ELIR/zkvm/zkvm_block.sail}" -o "$BUILD/zkvm_block"
  # 2. Compile the generated model (stock Sail GMP-ABI, backed by mini-gmp).
  #    The model calls setup_rts/cleanup_rts (provided by runtime.c) without a
  #    prototype since --c-no-rts omits rts.h; downgrade that to a warning.
  "$GCC" "${CFLAGS[@]}" -I"$lib" \
      -Wno-unused -Wno-error=implicit-function-declaration \
      -c "$BUILD/zkvm_block.c" -o "$BUILD/zkvm_block.o"
  # 3. GMP-free fixed-width Sail runtime (sailfix) replacing stock sail.c +
  #    mini-gmp: sail_int = 512-bit sign-magnitude, lbits = 256-bit inline.
  "$GCC" "${CFLAGS[@]}" -I"$lib" \
      -Wno-unused -Wno-error=implicit-function-declaration \
      -c "$RT/sailfix/sail.c" -o "$BUILD/sail.o"
  # 3b. Keccak-256 + SHA-256 C accelerators (c-interface-accelerators path).
  #     Pure C; need sail.h (-I$lib) for the unit/sail_int extern ABI types.
  "$GCC" "${CFLAGS[@]}" -I"$lib" \
      -Wno-unused -c "$ELIR/ffi/zkvm_accelerators.c" -o "$BUILD/zkvm_accelerators.o"
  "$GCC" "${CFLAGS[@]}" -I"$lib" -I"$ELIR/ffi" -DACCEL_MMIO \
      -Wno-unused -c "$ELIR/ffi/acc_shim.c" -o "$BUILD/acc_shim.o"
  # 3b'. Host accelerator device (spike --extlib): the crypto runs on the host,
  #      not as guest instructions. Reuses the SAME zkvm_accelerators behind the
  #      eth-act standard header, compiled for the HOST.
  "$HOSTCC" -O2 -I"$ELIR/ffi" -c "$ELIR/ffi/zkvm_accelerators.c" -o "$BUILD/zkvm_accel_ref_host.o"
  "$HOSTCXX" -std=c++17 -fPIC -shared -I"$SPIKE_INC" -I"$ELIR/ffi" -undefined dynamic_lookup \
      -o "$ACCEL_SO" "$ELIR/zkvm/accel-device/accel_device.cc" "$BUILD/zkvm_accel_ref_host.o"
  # 3c. C host backends: memory/calldata, overlay maps, operand stack, word
  #     predicates, code store + frame descriptors.
  for hc in host_mem host_map host_stack host_word host_code host_nodedb; do
    "$GCC" "${CFLAGS[@]}" -I"$lib" \
        -Wno-unused -c "$ELIR/ffi/$hc.c" -o "$BUILD/$hc.o"
  done
  # 4. Our freestanding runtime + IO + harness.
  "$GCC" "${CFLAGS[@]}" -I"$lib" -Wall -Wextra \
      -c "$RT/runtime.c" -o "$BUILD/runtime.o"
  "$GCC" "${CFLAGS[@]}" -Wall -Wextra -c "$RT/zkvm_io.c"  -o "$BUILD/zkvm_io.o"
  # private-input reader FFI for the Sail SSZ decoder (needs sail.h types).
  "$GCC" "${CFLAGS[@]}" -I"$lib" -Wall -Wextra -c "$RT/el_input.c" -o "$BUILD/el_input.o"
  # harness reads minstret (rdinstret) to report block execution cost -> Zicsr
  # (platform glue, like start.S; the proven model stays rv64im_zicclsm).
  "$GCC" "${CFLAGS[@]}" -march=rv64im_zicsr_zicclsm -I"$lib" -Wall -Wextra \
      -c "$RT/harness.c" -o "$BUILD/harness.o"
  # 5. Link the static guest ELF with the vendor linker script.
  "$GCC" "${CFLAGS[@]}" "${LDFLAGS[@]}" \
      "$BUILD/start.o" "$BUILD/htif.o" "$BUILD/zkvm_io.o" "$BUILD/el_input.o" \
      "$BUILD/zkvm_input_data.o" \
      "$BUILD/runtime.o" "$BUILD/harness.o" "$BUILD/sail.o" \
      "$BUILD/zkvm_accelerators.o" "$BUILD/acc_shim.o" \
      "$BUILD/host_mem.o" "$BUILD/host_map.o" "$BUILD/host_stack.o" \
      "$BUILD/host_word.o" "$BUILD/host_code.o" "$BUILD/host_nodedb.o" \
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

# Guard-region enforcement test: build a program that touches a guard region and
# run it on spike. Expected: the trap vector reports ABNORMAL termination and a
# NON-zero exit code (kind 0 = null trap, kind 1 = stack guard).
cmd_traptest() {
  local kind="${2:-0}"
  build_runtime
  "$GCC" "${CFLAGS[@]}" -Wall -Wextra -c "$RT/runtime.c" -o "$BUILD/runtime.o"
  "$GCC" "${CFLAGS[@]}" -DZKVM_TRAP_KIND="$kind" \
      -c "$RT/traptest_main.c" -o "$BUILD/traptest_main.o"
  "$GCC" "${CFLAGS[@]}" "${LDFLAGS[@]}" \
      "$BUILD/start.o" "$BUILD/htif.o" "$BUILD/runtime.o" "$BUILD/traptest_main.o" \
      -o "$BUILD/traptest.elf"
  echo "--- spike run (trap kind=$kind; expect non-zero exit) ---"
  set +e
  "$SPIKE" "${SPIKE_FLAGS[@]}" "$BUILD/traptest.elf"
  echo "--- spike exit code: $? ---"
  set -e
}

cmd_clean() { rm -rf "$BUILD"; echo "cleaned"; }

case "${1:-run}" in
  derisk)   cmd_derisk ;;
  guest)    cmd_guest ;;
  run)      cmd_run ;;
  traptest) cmd_traptest "$@" ;;
  clean)    cmd_clean ;;
  *) echo "usage: $0 {derisk|guest|run|traptest [0|1]|clean}"; exit 2 ;;
esac
