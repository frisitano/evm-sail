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
SAIL_Z3_MEMO_PATH="${SAIL_Z3_MEMO_PATH:-$ROOT/sail_smt_cache}"
SAIL_Z3_FLAGS=(--memo-z3 --memo-z3-path "$SAIL_Z3_MEMO_PATH")
C_OPTIMISED_DIR="$ROOT/sail/optimised"
C_OPTIMISED_MANIFEST="$C_OPTIMISED_DIR/manifest"
C_PROFILE_DIR="$C_OPTIMISED_DIR/profile"
C_PROFILE_MANIFEST="$C_PROFILE_DIR/manifest"
OPTIMIZED_PACKAGE="evmsail"
OPTIMIZED_GENERATED="$BUILD/generated"
# Sail regenerates this tree unconditionally; start from empty so the
# packager's header-collision check never trips on a stale staging copy.
rm -rf "$OPTIMIZED_GENERATED"
OPTIMIZED_MODEL_MANIFEST="$OPTIMIZED_GENERATED/src/spec/sources.list"
OPTIMIZED_STAGED_FFI="$OPTIMIZED_GENERATED/src/ffi"

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
# Experimental: lower constant-armed generated switches to static const
# tables (--c-const-match-tables). Build-only; default off.
EVM_CONST_TABLES="${EVM_CONST_TABLES:-off}"
case "$EVM_CONST_TABLES" in
  off|on) ;;
  *) echo "error: EVM_CONST_TABLES must be off or on" >&2; exit 2 ;;
esac
CONST_TABLE_FLAGS=()
if [ "$EVM_CONST_TABLES" = on ]; then
  CONST_TABLE_FLAGS=(--c-const-match-tables)
fi
# Experimental: merge model registers into one struct so every access shares
# a single base materialization (--c-register-file). Debug-module registers
# stay plain globals so the ZisK debug reporter's externs keep linking.
EVM_REGISTER_FILE="${EVM_REGISTER_FILE:-off}"
case "$EVM_REGISTER_FILE" in
  off|on) ;;
  *) echo "error: EVM_REGISTER_FILE must be off or on" >&2; exit 2 ;;
esac
if [ "$EVM_REGISTER_FILE" = on ]; then
  CONST_TABLE_FLAGS+=(--c-register-file)
  if [ "$EVM_DEBUG" = on ]; then
    CONST_TABLE_FLAGS+=(--c-register-file-exclude host/debug_enabled)
  fi
fi
# Experimental: thread the register-file base pointer through generated
# functions (--c-register-file-thread) so member accesses become single
# offset loads/stores from an argument register instead of re-materializing
# the struct address per function. Requires EVM_REGISTER_FILE=on.
EVM_REGISTER_FILE_THREAD="${EVM_REGISTER_FILE_THREAD:-off}"
case "$EVM_REGISTER_FILE_THREAD" in
  off|on) ;;
  *) echo "error: EVM_REGISTER_FILE_THREAD must be off or on" >&2; exit 2 ;;
esac
if [ "$EVM_REGISTER_FILE_THREAD" = on ]; then
  if [ "$EVM_REGISTER_FILE" != on ]; then
    echo "error: EVM_REGISTER_FILE_THREAD=on requires EVM_REGISTER_FILE=on" >&2; exit 2
  fi
  CONST_TABLE_FLAGS+=(--c-register-file-thread)
fi
# Experimental: EVM_GENERATED_INTERP=on keeps the GENERATED Sail interpreter
# loop instead of the hand-written C override. The evm/interpreter.sail splice
# (which rebinds interpret to the C loop) is swapped for
# evm/interpreter_generated.sail ($[c_inline] annotations only), and the
# hand-written extractions/c/optimised/contract/src/evm/interpreter.c is dropped from the staged
# source manifest so the generated interpret() is the one linked.
EVM_GENERATED_INTERP="${EVM_GENERATED_INTERP:-off}"
case "$EVM_GENERATED_INTERP" in
  off|on) ;;
  *) echo "error: EVM_GENERATED_INTERP must be off or on" >&2; exit 2 ;;
esac
# Experimental: EVM_INLINE_ATTR=on passes --c-inline-attr so functions
# annotated $[c_inline] in the optimised splices are inlined into their
# callers (dispatch fusion for the generated interpreter).
EVM_INLINE_ATTR="${EVM_INLINE_ATTR:-off}"
case "$EVM_INLINE_ATTR" in
  off|on) ;;
  *) echo "error: EVM_INLINE_ATTR must be off or on" >&2; exit 2 ;;
esac
if [ "$EVM_INLINE_ATTR" = on ]; then
  CONST_TABLE_FLAGS+=(--c-inline-attr)
fi
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
FFI_ROOT="$ROOT/extractions/c"
if [ -z "${GUEST:-}" ]; then
  MODEL_FFI="$FFI_ROOT/optimised/contract"
  MODEL_HEADER="$OPTIMIZED_PACKAGE/spec.h"
  MODEL_C_INCLUDE_FLAGS=(-I"$OPTIMIZED_GENERATED/include" -I"$OPTIMIZED_STAGED_FFI")
else
  MODEL_FFI="$FFI_ROOT/spec/contract"
  MODEL_SOURCE="$BUILD/zkvm_block.c"
  MODEL_HEADER="zkvm_block.h"
  MODEL_C_INCLUDE_FLAGS=(-I"$MODEL_FFI")
fi
MODEL_HEADER_FLAG="-DEVMSAIL_MODEL_H=\"$MODEL_HEADER\""
# Freestanding includes FIRST so <stdio.h>/<stdlib.h>/<string.h>/<gmp.h> resolve
# to our shims + vendored mini-gmp instead of newlib / libgmp.
CFLAGS=("${ARCH[@]}" -O2 -ffreestanding -nostdlib -fno-builtin
        -fno-stack-protector -fno-pic -mno-relax -DNDEBUG
        -ffunction-sections -fdata-sections
        -I"$RT/sail256" -I"$RT/freestanding" -I"$RT"
        -I"$ROOT/zkvm" -I"$ROOT/zkvm/io-device"
        "${MODEL_C_INCLUDE_FLAGS[@]}" -I"$FFI_ROOT")
if [ -z "${GUEST:-}" ]; then
  CFLAGS+=(-DEVMSAIL_OPTIMIZED_FFI)
fi
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
# EVM_LTO=on compiles every C object as LTO bitcode; cmd_zisk_lib then runs
# the link-time optimization in a gcc partial link so the final Rust link
# consumes one ordinary machine-code object (rust-lld cannot read GIMPLE).
# EVM_LTO=full keeps bitcode all the way into the archive for a final link
# that is itself gcc -flto (the C-direct guest); the entry objects then
# participate in whole-program optimization too.
EVM_LTO="${EVM_LTO:-off}"
case "$EVM_LTO" in
  on|full|off) ;;
  *) echo "error: EVM_LTO must be off, on, or full" >&2; exit 2 ;;
esac
if [ "$EVM_LTO" != off ]; then
  CFLAGS+=(-flto)
fi
if [ "$EVM_LTO" = full ]; then
  AR="${AR_FULL_LTO:-riscv64-unknown-elf-gcc-ar}"
fi
# --gc-sections drops the unused Sail diagnostic/format surface (and its
# gmp_printf/asprintf references).
LDFLAGS=(-T "$RT/link.ld" -Wl,--no-relax -Wl,--gc-sections -nostdlib -static)

# Sail nostd runtime library (ships with the toolchain).
sail_lib() {
  echo "$("$SAIL" --dir)/lib"
}

# spike memory ranges MUST match link.ld: MAIN, then (omitted guard gap), STACK.
SPIKE_MEM="0x80000000:0x10000000,0x90010000:0x04000000,0xA0000000:0x10000000"
# spike's --isa string does not name Zicclsm; that extension only mandates
# transparent misaligned load/store support, which spike provides via
# --misaligned. So rv64im + --misaligned is the Zicclsm-equivalent run config.
SPIKE_ISA="rv64im"
SPIKE_FLAGS=(--isa="$SPIKE_ISA" --misaligned -m"$SPIKE_MEM" --extlib="$SPIKE_DEVICES_SO" --device=accel)

# Inject the owning FFI headers directly. There is deliberately no aggregate
# model/input umbrella: each external operation is declared by its subsystem.
if [ -z "${GUEST:-}" ]; then
  MODEL_HEADERS=()
else
  MODEL_HEADERS=(
    sail_failure.h region_access.h hash.h precompiles.h output.h scratch.h
    memory.h frame_stack.h transient_storage.h stack.h code_db.h
    kernel_state.h trie_node_db.h state_db.h
  )
fi
if [ "$EVM_PROFILE" = on ]; then
  MODEL_HEADERS+=(cycle_scopes.h)
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
  SAIL="${SAIL:-sail}"
  export SAIL
}

compile_common() {
  local lib; lib="$(sail_lib)"
  # 1. Sail -> C: no main, no Sail runtime harness (we supply our own).
  if [ -n "${GUEST:-}" ]; then
    "$SAIL" "${SAIL_Z3_FLAGS[@]}" \
        -c --c-no-main --c-no-rts --c-preserve main \
        "${MODEL_INCLUDE_FLAGS[@]}" \
        "$GUEST" -o "$BUILD/zkvm_block"
  else
    local optimized_splice_flags=()
    local relative
    while IFS= read -r relative || [ -n "$relative" ]; do
      [ -z "$relative" ] && continue
      if [ "$EVM_GENERATED_INTERP" = on ] && [ "$relative" = "evm/interpreter.sail" ]; then
        relative="evm/interpreter_generated.sail"
      fi
      optimized_splice_flags+=(--splice "$C_OPTIMISED_DIR/$relative")
    done < "$C_OPTIMISED_MANIFEST"
    local profile_splice_flags=()
    if [ "$EVM_PROFILE" = on ]; then
      while IFS= read -r relative || [ -n "$relative" ]; do
        [ -z "$relative" ] && continue
        profile_splice_flags+=(--splice "$C_PROFILE_DIR/$relative")
      done < "$C_PROFILE_MANIFEST"
    fi
    ( cd "$ROOT" && "$SAIL" "${SAIL_Z3_FLAGS[@]}" \
        -c -O --Oconstant-fold --all-modules \
        --c-optimized-model --c-package "$OPTIMIZED_PACKAGE" \
        --c-output-dir "$OPTIMIZED_GENERATED" \
        --c-optimized-source-root sail \
        --c-optimized-include-dir "$MODEL_FFI/include" \
        --c-optimized-external-type StatelessInputSliceFields=evmsail/host/types.h \
        --c-optimized-external-type ScratchSliceFields=evmsail/host/types.h \
        --c-optimized-external-type EvmMemorySliceFields=evmsail/host/types.h \
        --c-optimized-external-type CodeRegionSliceFields=evmsail/host/types.h \
        --c-optimized-external-type LogDataSliceFields=evmsail/host/types.h \
        --c-optimized-external-type OutputSliceFields=evmsail/host/types.h \
        --c-optimized-external-type PreparedAuthorizationList=evmsail/host/types.h \
        --c-optimized-byte-pointer-field StatelessInputSliceFields.bytes=__direct \
        --c-optimized-byte-pointer-field ScratchSliceFields.bytes=__direct \
        --c-optimized-byte-pointer-field EvmMemorySliceFields.bytes=__direct \
        --c-optimized-byte-pointer-field CodeRegionSliceFields.bytes=__direct \
        --c-optimized-byte-pointer-field CodeFields.bytes=__direct \
        --c-optimized-byte-pointer-field LogDataSliceFields.bytes=__direct \
        --c-optimized-byte-pointer-field OutputSliceFields.bytes=__direct \
        --c-preserve main --c-preserve resume_frame \
        --c-preserve validation_debug_record --c-preserve write_invalid_result \
        --c-preserve sload_cost --c-preserve sstore_sentry_cost \
        --c-preserve sstore_costs \
        --c-specialize-log "${MODEL_INCLUDE_FLAGS[@]}" \
        ${CONST_TABLE_FLAGS[@]+"${CONST_TABLE_FLAGS[@]}"} \
        "${optimized_splice_flags[@]}" \
        ${profile_splice_flags[@]+"${profile_splice_flags[@]}"} \
        sail/evm.sail_project evm \
        --variable EVM_DEBUG="$EVM_DEBUG" )
    python3 "$ROOT/tools/package_optimised_c.py" "$OPTIMIZED_GENERATED"
    if [ "$EVM_GENERATED_INTERP" = on ]; then
      grep -v '^evm/interpreter\.c$' "$OPTIMIZED_STAGED_FFI/sources.list" \
        > "$OPTIMIZED_STAGED_FFI/sources.list.tmp"
      mv "$OPTIMIZED_STAGED_FFI/sources.list.tmp" "$OPTIMIZED_STAGED_FFI/sources.list"
    fi
  fi
  # 2. Compile the generated model.  Proven small ranges lower to native
  #    integers; mathematical int/nat values use sail256's exact bounded ABI.
  #    The model calls setup_rts/cleanup_rts (provided by runtime.c) without a
  #    prototype since --c-no-rts omits rts.h; downgrade that to a warning.
  MODEL_OBJS=()
  if [ -z "${GUEST:-}" ]; then
    [ -s "$OPTIMIZED_MODEL_MANIFEST" ] || { echo "error: missing generated model manifest" >&2; exit 2; }
    while IFS= read -r relative || [ -n "$relative" ]; do
      [ -z "$relative" ] && continue
      local source="$OPTIMIZED_GENERATED/src/spec/$relative"
      [ -f "$source" ] || { echo "error: missing generated model source: $relative" >&2; exit 2; }
      local object_name="${relative%.c}"
      object_name="${object_name//\//__}"
      local object="$BUILD/model__${object_name}.o"
      "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" \
          "$MODEL_HEADER_FLAG" \
          -Wno-unused -Wno-error=implicit-function-declaration \
          -c "$source" -o "$object"
      MODEL_OBJS+=("$object")
    done < "$OPTIMIZED_MODEL_MANIFEST"
    [ "${#MODEL_OBJS[@]}" -gt 0 ] || { echo "error: empty generated model manifest" >&2; exit 2; }
  else
    local object="$BUILD/zkvm_block.o"
    "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" \
        "$MODEL_HEADER_FLAG" \
        -Wno-unused -Wno-error=implicit-function-declaration \
        -c "$MODEL_SOURCE" -o "$object"
    MODEL_OBJS+=("$object")
  fi
  # 2b. Selected model backend. Optimized bindings follow the Sail module
  #     tree and are compiled from one deterministic manifest. The spec backend
  #     retains its independent GMP ABI and flat implementation.
  MODEL_BACKEND_OBJS=()
  if [ -z "${GUEST:-}" ]; then
    while IFS= read -r relative || [ -n "$relative" ]; do
      case "$relative" in ''|'#'*) continue ;; esac
      local source="$OPTIMIZED_STAGED_FFI/$relative"
      [ -f "$source" ] || { echo "error: missing optimized source: $relative" >&2; exit 2; }
      local object_name="${relative%.c}"
      object_name="${object_name//\//__}"
      local object="$BUILD/optimized__${object_name}.o"
      "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" \
          -Wno-unused "$MODEL_HEADER_FLAG" \
          -c "$source" -o "$object"
      MODEL_BACKEND_OBJS+=("$object")
    done < "$OPTIMIZED_STAGED_FFI/sources.list"
  else
    local source_and_name source object
    local -a backend_sources=(
      "state.c:model_state"
      "hash.c:hash"
      "code.c:model_code"
      "region_access.c:region_access"
      "address_result.c:model_address_result"
      "frame_stack.c:model_frame_stack"
      "precompiles.c:precompiles"
      "capacity.c:capacity"
      "memory.c:memory"
      "scratch.c:scratch"
      "transient_storage.c:transient_storage"
      "stack.c:stack"
      "code_db.c:code_db"
      "kernel_state.c:kernel_state"
      "output.c:output"
      "trie_node_db.c:trie_node_db"
      "state_db.c:state_db"
    )
    for source_and_name in "${backend_sources[@]}"; do
      source="${source_and_name%:*}"
      object="$BUILD/${source_and_name##*:}.o"
      "$GCC" "${CFLAGS[@]}" -I"$BUILD" -I"$lib" \
          -Wno-unused "$MODEL_HEADER_FLAG" \
          -c "$MODEL_FFI/$source" -o "$object"
      MODEL_BACKEND_OBJS+=("$object")
    done
  fi
  # 3. GMP-free Sail runtime: exact bounded integers and inline 256-bit lbits.
  "$GCC" "${CFLAGS[@]}" -I"$lib" \
      -Wno-unused -Wno-error=implicit-function-declaration \
      -c "$RT/sail256/sail.c" -o "$BUILD/sail.o"
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
  "$GCC" "${CFLAGS[@]}" -I"$ROOT/extractions/c" -I"$ROOT/zkvm" -Wall -Wextra \
      -c "$RT/accel_guest.c" -o "$BUILD/accel_guest.o"
  # Spike has no arithmetic precompile: the portable software provider owns
  # the zkvm_bigint.h contract here. The ZisK library build links
  # zkvm/zisk/bigint.c instead; the two providers must never co-link.
  "$GCC" "${CFLAGS[@]}" -I"$ROOT/extractions/c" -Wall -Wextra \
      -c "$RT/bigint_portable.c" -o "$BUILD/bigint_portable.o"
  ACCEL="$ROOT/zkvm/accel-host"; ACCEL_LIB="$ACCEL/target/release"
  if [ ! -f "$ACCEL_LIB/libzkvm_accel_host.dylib" ] && [ ! -f "$ACCEL_LIB/libzkvm_accel_host.so" ]; then
    ( cd "$ACCEL" && cargo build --release --target-dir target )
  fi
  "$HOSTCXX" -std=c++17 -fPIC -shared -I"$SPIKE_INC" -I"$ROOT/extractions/c" -I"$ROOT/zkvm" -undefined dynamic_lookup \
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
  # In partial-LTO mode the platform entry points are called only from the
  # later Rust link, so those two objects stay out of LTO: as machine code
  # they root the symbol graph and stop zkvm_start/heap_region from being
  # internalized. A full-LTO link roots from libziskos.a's _start instead.
  entry_lto=()
  if [ "$EVM_LTO" = on ]; then
    entry_lto=(-fno-lto)
  fi
  "$GCC" "${CFLAGS[@]}" ${entry_lto[@]+"${entry_lto[@]}"} -I"$lib" -Wall -Wextra \
      -c "$RT/harness.c" -o "$BUILD/harness.o"
  "$GCC" "${CFLAGS[@]}" ${entry_lto[@]+"${entry_lto[@]}"} -I"$lib" -Wall -Wextra \
      -c "$HERE/zisk/platform.c" -o "$BUILD/zisk_platform.o"
  # ZisK provider for the zkvm_bigint.h contract: forwards to the ziskos
  # zisklib arith256 exports, resolved by the final guest link (Rust crate
  # graph or libziskos.a). The Spike-only portable provider must not co-link.
  "$GCC" "${CFLAGS[@]}" -I"$ROOT/extractions/c" -Wall -Wextra \
      -c "$HERE/zisk/bigint.c" -o "$BUILD/zisk_bigint.o"
  compile_profile_scope
  # `ar crs` updates an existing archive without removing members that are no
  # longer named. Recreate it so renamed/deleted backend objects cannot survive a
  # rebuild and contribute stale symbols to the final guest.
  rm -f "$BUILD/libevmsail_zisk.a"
  if [ "$EVM_LTO" = on ]; then
    # Run LTO now: a relocatable partial link over the bitcode objects emits
    # one optimized machine-code object the Rust linker can consume. The
    # entry points are only referenced by the later Rust link, so pin them
    # against LTO internalization.
    local lto_roots=(-Wl,-u,zkvm_start -Wl,-u,zkvm_exit -Wl,-u,heap_region)
    if [ "$EVM_DEBUG" = on ]; then
      lto_roots+=(-Wl,-u,zisk_report_debug)
    fi
    # -O3 at the LTO partial link raises the inline budget so the host stack
    # and word primitives fold into the interpreter's dispatch arms; code
    # size is not a guest cost.
    "$GCC" "${CFLAGS[@]}" -O3 -r -nostdlib "${lto_roots[@]}" \
        "$BUILD/runtime.o" "$BUILD/harness.o" "$BUILD/zisk_platform.o" \
        "$BUILD/zisk_bigint.o" "$BUILD/sail.o" \
        "${MODEL_BACKEND_OBJS[@]}" \
        ${PROFILE_OBJ:+"$PROFILE_OBJ"} "${MODEL_OBJS[@]}" \
        -o "$BUILD/evmsail_lto.o"
    "$AR" crs "$BUILD/libevmsail_zisk.a" "$BUILD/evmsail_lto.o"
  else
    "$AR" crs "$BUILD/libevmsail_zisk.a" \
        "$BUILD/runtime.o" "$BUILD/harness.o" "$BUILD/zisk_platform.o" \
        "$BUILD/zisk_bigint.o" "$BUILD/sail.o" \
        "${MODEL_BACKEND_OBJS[@]}" \
        ${PROFILE_OBJ:+"$PROFILE_OBJ"} "${MODEL_OBJS[@]}"
  fi
  echo "built $BUILD/libevmsail_zisk.a"
}

link_guest() {
  "$GCC" "${CFLAGS[@]}" "${LDFLAGS[@]}" \
      "$BUILD/start.o" "$BUILD/htif.o" "$BUILD/zkvm_io.o" \
      "$BUILD/runtime.o" "$BUILD/harness.o" "$BUILD/sail.o" \
      "$BUILD/accel_guest.o" "$BUILD/bigint_portable.o" "${MODEL_BACKEND_OBJS[@]}" \
      ${PROFILE_OBJ:+"$PROFILE_OBJ"} \
      "${MODEL_OBJS[@]}" \
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
