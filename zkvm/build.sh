#!/usr/bin/env bash
# ===========================================================================
# Build evm-sail as a GMP-free guest for the eth-act zkVM RISC-V
# standard target (riscv64im_zicclsm-unknown-none-elf) and (optionally) run it
# on spike.
#
#   ./build.sh guest              - build the input-agnostic guest ELF
#   VEC=<input> ./build.sh run    - supply input at runtime and run on spike
#   ./build.sh clean              - remove build artifacts
#
# VEC is a raw schema-prefixed SszStatelessInput file used only by the Spike
# validation device. It is never compiled or linked into the ELF. The canonical
# driver is harness/run.py --spike, which builds the guest once and supplies a
# different runtime input to the unchanged ELF for each fixture.
#
# Requires: feature-capable Sail (resolved below), riscv64-unknown-elf-gcc,
# spike.
# ===========================================================================
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RT="$HERE/runtime"
# Override for concurrency: two builds sharing one directory still race on
# generated objects and the linked ELF (run.py --spike isolates itself).
BUILD="${ZKVM_BUILD:-$HERE/build}"
ROOT="$(cd "$HERE/.." && pwd)"
C_SPLICE="$ROOT/sail/splices/c_optimized.sail"

SAIL="${SAIL:-}"
GCC="${GCC:-riscv64-unknown-elf-gcc}"
SPIKE="${SPIKE:-spike}"
HOSTCC="${HOSTCC:-cc}"
HOSTCXX="${HOSTCXX:-c++}"
SPIKE_INC="${SPIKE_INC:-/opt/homebrew/Cellar/riscv-isa-sim/main/include}"
SPIKE_DEVICES_SO="$BUILD/spike_devices.so"
EVM_PROFILE="${EVM_PROFILE:-off}"
case "$EVM_PROFILE" in
  off|on) ;;
  *) echo "error: EVM_PROFILE must be off or on" >&2; exit 2 ;;
esac
PROFILE_OBJ=""

# Standard target: RV64IM + Zicclsm, LP64 soft-float, machine mode, freestanding.
ARCH=(-march=rv64im_zicclsm -mabi=lp64 -mcmodel=medany)
# Freestanding includes FIRST so <stdio.h>/<stdlib.h>/<string.h>/<gmp.h> resolve
# to our shims + vendored mini-gmp instead of newlib / libgmp.
CFLAGS=("${ARCH[@]}" -O2 -ffreestanding -nostdlib -fno-builtin
        -fno-stack-protector -fno-pic -mno-relax -DNDEBUG
        -ffunction-sections -fdata-sections
        -I"$RT/sail256" -I"$RT/freestanding" -I"$RT"
        -I"$ROOT/zkvm" -I"$ROOT/zkvm/io-device" -I"$ROOT/ffi")
# -lgcc supplies compiler runtime helpers; --gc-sections drops the unused Sail
# diagnostic/format surface (and its gmp_printf/asprintf references).
LDFLAGS=(-T "$RT/link.ld" -Wl,--no-relax -Wl,--gc-sections -nostdlib -static)

# Sail nostd runtime library (ships with the toolchain).
sail_lib() {
  echo "$("$SAIL" --dir)/lib"
}

# spike memory ranges MUST match link.ld: MAIN, then (omitted guard gap), STACK.
SPIKE_MEM="0x80000000:0x10000000,0x90010000:0x04000000"
# spike's --isa string does not name Zicclsm; that extension only mandates
# transparent misaligned load/store support, which spike provides via
# --misaligned. So rv64im + --misaligned is the Zicclsm-equivalent run config.
SPIKE_ISA="rv64im"
SPIKE_FLAGS=(--isa="$SPIKE_ISA" --misaligned -m"$SPIKE_MEM" --extlib="$SPIKE_DEVICES_SO" --device=accel)

# Inject the owning FFI headers directly. There is deliberately no aggregate
# model/input umbrella: each external operation is declared by its subsystem.
MODEL_HEADERS=(
  byte_slice_glue.h host_crypto.h precompiles.h output.h scratch.h memory.h
  transient_storage.h stack.h code_db.h kernel_state.h trie_node_db.h
  state_db.h cycle_scopes.h
)
MODEL_INCLUDE_FLAGS=()
for header in "${MODEL_HEADERS[@]}"; do
  MODEL_INCLUDE_FLAGS+=(--c-include "$header")
done

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
  if [ -z "${GUEST:-}" ]; then
    SAIL="$(bash "$HERE/resolve_optimized_sail.sh")"
  else
    SAIL="${SAIL:-sail}"
  fi
  export SAIL
  local lib; lib="$(sail_lib)"
  build_runtime
  # 1. Sail -> C: no main, no Sail runtime harness (we supply our own).
  if [ -n "${GUEST:-}" ]; then
    "$SAIL" -c --c-no-main --c-no-rts --c-preserve main \
        "${MODEL_INCLUDE_FLAGS[@]}" \
        "$GUEST" -o "$BUILD/zkvm_block"
  else
    ( cd "$ROOT" && "$SAIL" -c --c-no-main --c-no-rts --c-preserve main \
        --c-specialize \
        "${MODEL_INCLUDE_FLAGS[@]}" \
        --splice "$C_SPLICE" \
        sail/evm.sail_project evm \
        --variable EVM_PROFILE="$EVM_PROFILE" \
        --variable EVM_DEBUG=off \
        -o "$BUILD/zkvm_block" )
  fi
  # 2. Compile the generated model.  Proven small ranges lower to native
  #    integers; mathematical int/nat values use sail256's exact bounded ABI.
  #    The model calls setup_rts/cleanup_rts (provided by runtime.c) without a
  #    prototype since --c-no-rts omits rts.h; downgrade that to a warning.
  "$GCC" "${CFLAGS[@]}" -I"$lib" \
      -Wno-unused -Wno-error=implicit-function-declaration \
      -c "$BUILD/zkvm_block.c" -o "$BUILD/zkvm_block.o"
  # 2b. state aggregate glue: account/storage rows cross the extern boundary
  #     using generated layouts; the rollback journal itself is C-private.
  "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
      -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$ROOT/ffi/journal_glue.c" -o "$BUILD/journal_glue.o"
  "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
      -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$ROOT/ffi/hash_glue.c" -o "$BUILD/hash_glue.o"
  "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
      -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$ROOT/ffi/code_glue.c" -o "$BUILD/code_glue.o"
  "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
      -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$ROOT/ffi/byte_slice_glue.c" -o "$BUILD/byte_slice_glue.o"
  # 3. GMP-free Sail runtime: exact bounded integers and inline 256-bit lbits.
  "$GCC" "${CFLAGS[@]}" -I"$lib" \
      -Wno-unused -Wno-error=implicit-function-declaration \
      -c "$RT/sail256/sail.c" -o "$BUILD/sail.o"
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
  # 3b'. Spike validation devices: the accelerator models zkVM crypto
  #      precompiles through the SAME Rust accel-host implementation the native
  #      build links, while the input device supplies read_input bytes at
  #      runtime. Neither crypto nor fixture bytes are linked into the guest.
  ACCEL="$ROOT/zkvm/accel-host"; ACCEL_LIB="$ACCEL/target/release"
  if [ ! -f "$ACCEL_LIB/libzkvm_accel_host.dylib" ] && [ ! -f "$ACCEL_LIB/libzkvm_accel_host.so" ]; then
    ( cd "$ACCEL" && cargo build --release --target-dir target )  # local target (matches link path)
  fi
  "$HOSTCXX" -std=c++17 -fPIC -shared -I"$SPIKE_INC" -I"$ROOT/ffi" -I"$ROOT/zkvm" -undefined dynamic_lookup \
      -o "$SPIKE_DEVICES_SO" \
      "$ROOT/zkvm/accel-device/accel_device.cc" "$ROOT/zkvm/io-device/io_device.cc" \
      -L"$ACCEL_LIB" -lzkvm_accel_host -Wl,-rpath,"$ACCEL_LIB"
  # 3c. C host backends: memory/generic byte slices, transient storage,
  #     output arena, operand stack, code/JUMPDEST arenas, and witness/account
  #     databases.
  for hc in memory scratch transient_storage state_db stack code_db kernel_state trie_node_db output; do
    "$GCC" "${CFLAGS[@]}" -I"$lib" \
        -Wno-unused -c "$ROOT/ffi/$hc.c" -o "$BUILD/$hc.o"
  done
  # 4. Our freestanding runtime + standard IO implementation + harness.
  "$GCC" "${CFLAGS[@]}" -I"$lib" -Wall -Wextra \
      -c "$RT/runtime.c" -o "$BUILD/runtime.o"
  "$GCC" "${CFLAGS[@]}" -Wall -Wextra \
      -c "$ROOT/zkvm/io-device/guest.c" -o "$BUILD/zkvm_io.o"
  "$GCC" "${CFLAGS[@]}" -I"$lib" -Wall -Wextra \
      -c "$RT/harness.c" -o "$BUILD/harness.o"
  if [ "$EVM_PROFILE" = on ]; then
    "$GCC" "${CFLAGS[@]}" -I"$lib" -Wall -Wextra \
        -c "$RT/cycle_scopes.c" -o "$BUILD/cycle_scopes.o"
    PROFILE_OBJ="$BUILD/cycle_scopes.o"
  fi
  printf '%s\n' "$EVM_PROFILE" > "$BUILD/evm_profile"
  # 5. Link the static guest ELF with the vendor linker script.
  link_guest
}

link_guest() {
  "$GCC" "${CFLAGS[@]}" "${LDFLAGS[@]}" \
      "$BUILD/start.o" "$BUILD/htif.o" "$BUILD/zkvm_io.o" \
      "$BUILD/runtime.o" "$BUILD/harness.o" "$BUILD/sail.o" \
      "$BUILD/host_crypto.o" "$BUILD/precompiles.o" "$BUILD/accel_guest.o" \
      "$BUILD/journal_glue.o" "$BUILD/hash_glue.o" "$BUILD/code_glue.o" "$BUILD/byte_slice_glue.o" \
      "$BUILD/memory.o" "$BUILD/scratch.o" "$BUILD/transient_storage.o" "$BUILD/state_db.o" "$BUILD/stack.o" \
      "$BUILD/code_db.o" "$BUILD/kernel_state.o" "$BUILD/trie_node_db.o" "$BUILD/output.o" \
      ${PROFILE_OBJ:+"$PROFILE_OBJ"} \
      "$BUILD/zkvm_block.o" \
      -o "$BUILD/zkvm_guest.elf"
  echo "built $BUILD/zkvm_guest.elf"
  riscv64-unknown-elf-size "$BUILD/zkvm_guest.elf" 2>/dev/null || true
}

cmd_run() {
  if [ -z "${RUN_ONLY:-}" ]; then
    cmd_guest
  elif [ ! -f "$BUILD/zkvm_guest.elf" ] || [ ! -f "$SPIKE_DEVICES_SO" ]; then
    echo "error: RUN_ONLY requires an existing guest build" >&2
    exit 2
  fi
  local vec="${VEC:-}"
  if [ -z "$vec" ]; then
    echo "error: set VEC=<raw SszStatelessInput file> for the Spike run" >&2
    exit 2
  fi
  echo "--- spike run (${SPIKE_FLAGS[*]}) ---"
  set +e
  "$SPIKE" "${SPIKE_FLAGS[@]}" --device="evmsail_input,$vec" "$BUILD/zkvm_guest.elf"
  local rc=$?
  set -e
  echo "--- spike exit code: $rc ---"
  return $rc
}

cmd_clean() { rm -rf "$BUILD"; echo "cleaned"; }

# The one-time platform bring-up probes (derisk: HTIF smoke; traptest:
# guard-region enforcement) were deleted once the real guest gate
# (run.py --spike) covered the platform end-to-end; recover them
# from git history if the platform glue (start.S / link.ld / htif.c) changes.
case "${1:-run}" in
  guest)    cmd_guest ;;
  run)      cmd_run ;;
  clean)    cmd_clean ;;
  *) echo "usage: $0 {guest|run|clean}"; exit 2 ;;
esac
