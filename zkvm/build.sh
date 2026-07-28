#!/usr/bin/env bash
# ===========================================================================
# Build evm-sail as a GMP-free guest for the eth-act zkVM standard interface.
# The platform-neutral model archive can be linked into ZisK; the standalone
# RV64IM ELF can be run directly on Spike.
#
#   ./build.sh guest              - build the input-agnostic guest ELF
#   ./build.sh zisk-lib           - build the model/runtime archive for ZisK
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
COMMAND="${1:-run}"
PLATFORM="${ZKVM_PLATFORM:-}"
if [ -z "$PLATFORM" ]; then
  if [ "$COMMAND" = zisk-lib ]; then PLATFORM=zisk; else PLATFORM=spike; fi
fi
case "$PLATFORM" in
  spike|zisk) ;;
  *) echo "error: ZKVM_PLATFORM must be spike or zisk" >&2; exit 2 ;;
esac
# Override for concurrency: two builds sharing one directory still race on
# generated objects and the linked ELF (run.py --spike isolates itself).
BUILD="${ZKVM_BUILD:-$HERE/build}"
ROOT="$(cd "$HERE/.." && pwd)"
C_SPLICE="$ROOT/sail/splices/c_optimized.sail"
C_PROFILE_SPLICE="$ROOT/sail/splices/c_optimized_profile.sail"

SAIL="${SAIL:-}"
GCC="${GCC:-riscv64-unknown-elf-gcc}"
AR="${AR:-riscv64-unknown-elf-ar}"
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
EVM_DEBUG="${EVM_DEBUG:-off}"
case "$EVM_DEBUG" in
  off|on) ;;
  *) echo "error: EVM_DEBUG must be off or on" >&2; exit 2 ;;
esac
PROFILE_OBJ=""

# Both guests use the standard LP64 soft-float ABI. Spike models RV64IM with
# transparent misaligned accesses; ZisK's Rust target additionally enables A.
if [ "$PLATFORM" = zisk ]; then
  # ZisK's built-in linker script gathers .rodata.* and .bss.*, but not the
  # RISC-V small-data .srodata.* / .sbss.* families.  Keeping small-data
  # emission disabled ensures every C section lands in a linker-owned output
  # section that the emulator can load without overlapping orphan sections.
  ARCH=(-march=rv64ima -mabi=lp64 -mcmodel=medany -msmall-data-limit=0)
else
  ARCH=(-march=rv64im_zicclsm -mabi=lp64 -mcmodel=medany)
fi
# Freestanding includes FIRST so <stdio.h>/<stdlib.h>/<string.h>/<gmp.h> resolve
# to our shims + vendored mini-gmp instead of newlib / libgmp.
CFLAGS=("${ARCH[@]}" -O2 -ffreestanding -nostdlib -fno-builtin
        -fno-stack-protector -fno-pic -mno-relax -DNDEBUG
        -ffunction-sections -fdata-sections
        -I"$RT/sail256" -I"$RT/freestanding" -I"$RT"
        -I"$ROOT/zkvm" -I"$ROOT/zkvm/io-device" -I"$ROOT/ffi")
if [ "$PLATFORM" = zisk ]; then
  CFLAGS+=(
    -DEVMSAIL_EXTERNAL_HEAP
    -DEVMSAIL_PLATFORM_LIBC_MEMORY
    -DEVMSAIL_PLATFORM_ZISK
  )
  if [ "$EVM_DEBUG" = on ]; then
    CFLAGS+=(-DEVMSAIL_DEBUG)
  fi
fi
if [ -z "${GUEST:-}" ]; then
  CFLAGS+=(
    -DEVMSAIL_OPTIMIZED_ABI
    -DEVMSAIL_POINTER_ABI
    -DEVMSAIL_CAPACITY_FIXED
  )
fi
# --gc-sections drops the unused Sail diagnostic/format surface (and its
# gmp_printf/asprintf references).
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
  region_access.h hash_glue.h precompiles.h output.h scratch.h memory.h
  frame_stack.h
  transient_storage.h stack.h code_db.h kernel_state.h trie_node_db.h
  state_db.h
)
if [ "$EVM_PROFILE" = on ]; then
  MODEL_HEADERS+=(cycle_scopes.h)
fi
if [ -z "${GUEST:-}" ]; then
  MODEL_HEADERS+=(word_bytes_glue.h preimage_glue.h htr_glue.h mpt_glue.h journal_glue.h interpreter_glue.h blob_fee_glue.h)
fi
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

resolve_sail() {
  if [ -z "${GUEST:-}" ]; then
    SAIL="$(bash "$HERE/resolve_optimized_sail.sh")"
  else
    SAIL="${SAIL:-sail}"
  fi
  export SAIL
}

compile_common() {
  local lib; lib="$(sail_lib)"
  local trie_aggregate_glue_flags=()
  local state_access_aggregate_glue_flags=()
  if [ -z "${GUEST:-}" ] && [ "$EVM_DEBUG" = off ]; then
    trie_aggregate_glue_flags=(-DEVMSAIL_NO_TRIE_AGGREGATE_GLUE)
  fi
  if [ -z "${GUEST:-}" ]; then
    state_access_aggregate_glue_flags=(-DEVMSAIL_NO_STATE_ACCESS_AGGREGATE_GLUE)
  fi
  # 1. Sail -> C: no main, no Sail runtime harness (we supply our own).
  if [ -n "${GUEST:-}" ]; then
    "$SAIL" -c --c-no-main --c-no-rts --c-preserve main \
        "${MODEL_INCLUDE_FLAGS[@]}" \
        "$GUEST" -o "$BUILD/zkvm_block"
  else
    local profile_splice_flags=()
    if [ "$EVM_PROFILE" = on ]; then
      profile_splice_flags=(--splice "$C_PROFILE_SPLICE")
    fi
    ( cd "$ROOT" && "$SAIL" -c -O --Oconstant-fold --c-no-main --c-no-rts \
        --c-preserve main --c-preserve leaf_child_ref \
        --c-preserve resume_frame \
        --c-specialize --c-require-bounded-int \
        "${MODEL_INCLUDE_FLAGS[@]}" \
        --splice "$C_SPLICE" \
        "${profile_splice_flags[@]}" \
        sail/evm.sail_project evm \
        --variable EVM_DEBUG="$EVM_DEBUG" \
        -o "$BUILD/zkvm_block" )
  fi
  # 2. Compile the generated model.  Proven small ranges lower to native
  #    integers; mathematical int/nat values use sail256's exact bounded ABI.
  #    The model calls setup_rts/cleanup_rts (provided by runtime.c) without a
  #    prototype since --c-no-rts omits rts.h; downgrade that to a warning.
  "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" \
      -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -Wno-unused -Wno-error=implicit-function-declaration \
      -c "$BUILD/zkvm_block.c" -o "$BUILD/zkvm_block.o"
  # 2b. state aggregate glue: account/storage rows cross the extern boundary
  #     using generated layouts; the rollback journal itself is C-private.
  "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
      -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      "${trie_aggregate_glue_flags[@]}" \
      "${state_access_aggregate_glue_flags[@]}" \
      -c "$ROOT/ffi/journal_glue.c" -o "$BUILD/journal_glue.o"
  "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
      -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$ROOT/ffi/hash_glue.c" -o "$BUILD/hash_glue.o"
  "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
      -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$ROOT/ffi/code_glue.c" -o "$BUILD/code_glue.o"
  "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
      -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$ROOT/ffi/region_access.c" -o "$BUILD/region_access.o"
  "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
      -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$ROOT/ffi/address_result_glue.c" -o "$BUILD/address_result_glue.o"
  "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
      -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$ROOT/ffi/frame_stack_glue.c" -o "$BUILD/frame_stack_glue.o"
  HTR_GLUE_OBJ=""
  PREIMAGE_GLUE_OBJ=""
  MPT_GLUE_OBJ=""
  INTERPRETER_GLUE_OBJ=""
  BLOB_FEE_GLUE_OBJ=""
  if [ -z "${GUEST:-}" ]; then
    "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
        -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
        -c "$ROOT/ffi/preimage_glue.c" -o "$BUILD/preimage_glue.o"
    PREIMAGE_GLUE_OBJ="$BUILD/preimage_glue.o"
    "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
        -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
        -c "$ROOT/ffi/htr_glue.c" -o "$BUILD/htr_glue.o"
    HTR_GLUE_OBJ="$BUILD/htr_glue.o"
    "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
        -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
        -c "$ROOT/ffi/mpt_glue.c" -o "$BUILD/mpt_glue.o"
    MPT_GLUE_OBJ="$BUILD/mpt_glue.o"
    "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
        -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
        -c "$ROOT/ffi/interpreter_glue.c" -o "$BUILD/interpreter_glue.o"
    INTERPRETER_GLUE_OBJ="$BUILD/interpreter_glue.o"
    "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" -I"$ROOT/ffi" \
        -Wno-unused -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
        -c "$ROOT/ffi/blob_fee_glue.c" -o "$BUILD/blob_fee_glue.o"
    BLOB_FEE_GLUE_OBJ="$BUILD/blob_fee_glue.o"
  fi
  # 3. GMP-free Sail runtime: exact bounded integers and inline 256-bit lbits.
  "$GCC" "${CFLAGS[@]}" -I"$lib" \
      -Wno-unused -Wno-error=implicit-function-declaration \
      -c "$RT/sail256/sail.c" -o "$BUILD/sail.o"
  # 3b. The precompile adapter is platform-neutral and calls only the standard
  #     zkvm_accelerators.h interface.
  "$GCC" "${CFLAGS[@]}" -I"$lib" -I"$ROOT/ffi" \
      -Wno-unused -c "$ROOT/ffi/precompiles.c" -o "$BUILD/precompiles.o"
  # 3c. C host backends: nominal region access, transient storage,
  #     output arena, operand stack, code/JUMPDEST arenas, and witness/account
  #     databases.
  for hc in capacity memory scratch transient_storage state_db stack code_db kernel_state trie_node_db output; do
    "$GCC" "${CFLAGS[@]}" -I"$lib" \
        -Wno-unused -c "$ROOT/ffi/$hc.c" -o "$BUILD/$hc.o"
  done
}

compile_profile_scope() {
  local lib; lib="$(sail_lib)"
  if [ "$EVM_PROFILE" = on ]; then
    local profile_arch=()
    if [ "$PLATFORM" = zisk ]; then
      profile_arch=(-march=rv64ima_zicsr)
    fi
    "$GCC" "${CFLAGS[@]}" "${profile_arch[@]}" -I"$lib" -Wall -Wextra \
        -c "$RT/cycle_scopes.c" -o "$BUILD/cycle_scopes.o"
    PROFILE_OBJ="$BUILD/cycle_scopes.o"
  fi
  printf '%s\n' "$EVM_PROFILE" > "$BUILD/evm_profile"
}

cmd_guest() {
  resolve_sail
  local lib; lib="$(sail_lib)"
  build_runtime
  compile_common
  # Spike implements the standard accelerator surface through its MMIO device.
  "$GCC" "${CFLAGS[@]}" -I"$ROOT/ffi" -I"$ROOT/zkvm" -Wall -Wextra \
      -c "$RT/accel_guest.c" -o "$BUILD/accel_guest.o"
  ACCEL="$ROOT/zkvm/accel-host"; ACCEL_LIB="$ACCEL/target/release"
  if [ ! -f "$ACCEL_LIB/libzkvm_accel_host.dylib" ] && [ ! -f "$ACCEL_LIB/libzkvm_accel_host.so" ]; then
    ( cd "$ACCEL" && cargo build --release --target-dir target )
  fi
  "$HOSTCXX" -std=c++17 -fPIC -shared -I"$SPIKE_INC" -I"$ROOT/ffi" -I"$ROOT/zkvm" -undefined dynamic_lookup \
      -o "$SPIKE_DEVICES_SO" \
      "$ROOT/zkvm/accel-device/accel_device.cc" "$ROOT/zkvm/io-device/io_device.cc" \
      -L"$ACCEL_LIB" -lzkvm_accel_host -Wl,-rpath,"$ACCEL_LIB"
  # 4. Our freestanding runtime + standard IO implementation + harness.
  "$GCC" "${CFLAGS[@]}" -I"$lib" -Wall -Wextra \
      -c "$RT/runtime.c" -o "$BUILD/runtime.o"
  "$GCC" "${CFLAGS[@]}" -Wall -Wextra \
      -c "$ROOT/zkvm/io-device/guest.c" -o "$BUILD/zkvm_io.o"
  "$GCC" "${CFLAGS[@]}" -I"$lib" -Wall -Wextra \
      -c "$RT/harness.c" -o "$BUILD/harness.o"
  compile_profile_scope
  # 5. Link the static guest ELF with the vendor linker script.
  link_guest
}

cmd_zisk_lib() {
  resolve_sail
  local lib; lib="$(sail_lib)"
  compile_common
  "$GCC" "${CFLAGS[@]}" -I"$lib" -Wall -Wextra \
      -c "$RT/runtime.c" -o "$BUILD/runtime.o"
  "$GCC" "${CFLAGS[@]}" -I"$lib" -Wall -Wextra \
      -c "$RT/harness.c" -o "$BUILD/harness.o"
  "$GCC" "${CFLAGS[@]}" -I"$lib" -Wall -Wextra \
      -c "$HERE/zisk/platform.c" -o "$BUILD/zisk_platform.o"
  compile_profile_scope
  # `ar crs` updates an existing archive without removing members that are no
  # longer named. Recreate it so renamed/deleted glue objects cannot survive a
  # rebuild and contribute stale symbols to the final guest.
  rm -f "$BUILD/libevmsail_zisk.a"
  "$AR" crs "$BUILD/libevmsail_zisk.a" \
      "$BUILD/runtime.o" "$BUILD/harness.o" "$BUILD/zisk_platform.o" "$BUILD/sail.o" \
      "$BUILD/precompiles.o" \
      "$BUILD/journal_glue.o" "$BUILD/hash_glue.o" "$BUILD/code_glue.o" "$BUILD/region_access.o" "$BUILD/address_result_glue.o" "$BUILD/frame_stack_glue.o" \
      ${PREIMAGE_GLUE_OBJ:+"$PREIMAGE_GLUE_OBJ"} \
      ${HTR_GLUE_OBJ:+"$HTR_GLUE_OBJ"} \
      ${MPT_GLUE_OBJ:+"$MPT_GLUE_OBJ"} \
      ${INTERPRETER_GLUE_OBJ:+"$INTERPRETER_GLUE_OBJ"} \
      ${BLOB_FEE_GLUE_OBJ:+"$BLOB_FEE_GLUE_OBJ"} \
      "$BUILD/memory.o" "$BUILD/scratch.o" "$BUILD/transient_storage.o" "$BUILD/state_db.o" "$BUILD/stack.o" \
      "$BUILD/code_db.o" "$BUILD/kernel_state.o" "$BUILD/trie_node_db.o" "$BUILD/output.o" "$BUILD/capacity.o" \
      ${PROFILE_OBJ:+"$PROFILE_OBJ"} "$BUILD/zkvm_block.o"
  echo "built $BUILD/libevmsail_zisk.a"
}

link_guest() {
  "$GCC" "${CFLAGS[@]}" "${LDFLAGS[@]}" \
      "$BUILD/start.o" "$BUILD/htif.o" "$BUILD/zkvm_io.o" \
      "$BUILD/runtime.o" "$BUILD/harness.o" "$BUILD/sail.o" \
      "$BUILD/precompiles.o" "$BUILD/accel_guest.o" \
      "$BUILD/journal_glue.o" "$BUILD/hash_glue.o" "$BUILD/code_glue.o" "$BUILD/region_access.o" "$BUILD/address_result_glue.o" "$BUILD/frame_stack_glue.o" \
      ${PREIMAGE_GLUE_OBJ:+"$PREIMAGE_GLUE_OBJ"} \
      ${HTR_GLUE_OBJ:+"$HTR_GLUE_OBJ"} \
      ${MPT_GLUE_OBJ:+"$MPT_GLUE_OBJ"} \
      ${INTERPRETER_GLUE_OBJ:+"$INTERPRETER_GLUE_OBJ"} \
      ${BLOB_FEE_GLUE_OBJ:+"$BLOB_FEE_GLUE_OBJ"} \
      "$BUILD/memory.o" "$BUILD/scratch.o" "$BUILD/transient_storage.o" "$BUILD/state_db.o" "$BUILD/stack.o" \
      "$BUILD/code_db.o" "$BUILD/kernel_state.o" "$BUILD/trie_node_db.o" "$BUILD/output.o" "$BUILD/capacity.o" \
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
case "$COMMAND" in
  guest)    cmd_guest ;;
  zisk-lib) cmd_zisk_lib ;;
  run)      cmd_run ;;
  clean)    cmd_clean ;;
  *) echo "usage: $0 {guest|zisk-lib|run|clean}"; exit 2 ;;
esac
