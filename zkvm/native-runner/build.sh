#!/usr/bin/env bash
# ===========================================================================
# Build a NATIVE (host, not RISC-V) conformance executable for the evm-sail zkVM
# stateless block guest (sail/evm.sail_project evm -> main).
#
# This mirrors the guest sail-compile flags in zkvm/build.sh (--c-no-main and
# --c-preserve main) with the directly-linked Rust accel-host cdylib. Optimized
# builds use the GMP-free sail256 runtime; standard builds use Sail's normal C
# runtime because the canonical ABI includes sail_int and lbits values.
#
# I/O + run harness = test_utils.c, the ONE shared native implementation of
# read_input/write_output plus large-stack run_once and clear_memory, also
# linked by the ctypes libraries.
#
# Idempotent: rebuilds the accel-host cdylib only if its library is missing.
#
# Requires: feature-capable Sail (resolved below), a C compiler, cargo. Standard
# builds additionally require GMP. NO HTIF, NO spike.
#
#   export PATH="$HOME/.opam/sail/bin:$PATH"
#   eval "$(opam env --root="$HOME/.opam" --switch=sail)"
#   ./build.sh
# ===========================================================================
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"      # repo root (evm-sail)
BUILD="${NATIVE_BUILD:-$HERE/.build}"
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
OPTIMIZED_MODEL_MANIFEST_CANONICAL="$BUILD/generated-model-sources.list"
OPTIMIZED_FFI_MANIFEST_CANONICAL="$BUILD/optimized-ffi-sources.list"
OPTIMIZED_SPLICE_MANIFEST_CANONICAL="$BUILD/optimized-splices.list"
PROFILE_SPLICE_MANIFEST_CANONICAL="$BUILD/profile-splices.list"
mkdir -p "$BUILD"

canonicalize_manifest() {
  ( cd "$ROOT" && uv run --frozen python -m devtools.build_support canonicalize-manifest \
    --root "$1" --output "$3" "$2" ) >/dev/null
}

SAIL="${SAIL:-sail}"
export SAIL
CC="${CC:-cc}"
EVM_PROFILE="${EVM_PROFILE:-off}"
EVM_BUILD_MODE="${EVM_BUILD_MODE:-optimized}"
C_SPEC_SPECIALIZATION_LIMIT="${C_SPEC_SPECIALIZATION_LIMIT:-256}"
C_OPT_SPECIALIZATION_LIMIT="${C_OPT_SPECIALIZATION_LIMIT:-256}"
case "$EVM_PROFILE" in
  off|on) ;;
  *) echo "error: EVM_PROFILE must be off or on" >&2; exit 2 ;;
esac
case "$EVM_BUILD_MODE" in
  standard|optimized) ;;
  *) echo "error: EVM_BUILD_MODE must be standard or optimized" >&2; exit 2 ;;
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
# Experimental: merged register-file struct (--c-register-file); the native
# test harness keeps debug registers as globals for its diagnostics.
EVM_REGISTER_FILE="${EVM_REGISTER_FILE:-off}"
case "$EVM_REGISTER_FILE" in
  off|on) ;;
  *) echo "error: EVM_REGISTER_FILE must be off or on" >&2; exit 2 ;;
esac
if [ "$EVM_REGISTER_FILE" = on ]; then
  CONST_TABLE_FLAGS+=(--c-register-file --c-register-file-exclude host/debug_enabled)
fi
# Experimental: thread the register-file base pointer through generated
# functions (--c-register-file-thread); requires EVM_REGISTER_FILE=on.
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
# loop (omits the evm/interpreter.sail override and drops the hand-written
# evm/interpreter.c from the staged manifest);
# EVM_INLINE_ATTR=on passes --c-inline-attr for $[c_inline] dispatch fusion.
# See zkvm/build.sh for the authoritative description.
EVM_GENERATED_INTERP="${EVM_GENERATED_INTERP:-off}"
case "$EVM_GENERATED_INTERP" in
  off|on) ;;
  *) echo "error: EVM_GENERATED_INTERP must be off or on" >&2; exit 2 ;;
esac
if [ "$EVM_BUILD_MODE" = optimized ]; then
  canonicalize_manifest "$C_OPTIMISED_DIR" "$C_OPTIMISED_MANIFEST" \
    "$OPTIMIZED_SPLICE_MANIFEST_CANONICAL"
  EVM_INLINE_ATTR="${EVM_INLINE_ATTR:-on}"
else
  EVM_INLINE_ATTR="${EVM_INLINE_ATTR:-off}"
fi
case "$EVM_INLINE_ATTR" in
  off|on) ;;
  *) echo "error: EVM_INLINE_ATTR must be off or on" >&2; exit 2 ;;
esac
if [ "$EVM_INLINE_ATTR" = on ]; then
  CONST_TABLE_FLAGS+=(--c-inline-attr)
fi
EVM_ALWAYS_INLINE_ATTR="${EVM_ALWAYS_INLINE_ATTR:-on}"
case "$EVM_ALWAYS_INLINE_ATTR" in
  off|on) ;;
  *) echo "error: EVM_ALWAYS_INLINE_ATTR must be off or on" >&2; exit 2 ;;
esac
if [ "$EVM_ALWAYS_INLINE_ATTR" = on ]; then
  CONST_TABLE_FLAGS+=(--c-always-inline-attr)
fi
if [ "$EVM_BUILD_MODE" = standard ] && [ "$EVM_PROFILE" = on ]; then
  echo "error: EVM_PROFILE=on is available only for optimized builds" >&2
  exit 2
fi

# --- Sail C runtime include dir (where sail.h lives) ------------------------
# Query Sail rather than deriving this from the executable path, so the
# location moves with the compiler rather than being assumed here.
SAIL_LIB="$("$SAIL" --dir)/lib"
if [ ! -f "$SAIL_LIB/sail.h" ]; then
  echo "error: sail.h not found under $SAIL_LIB" >&2
  exit 1
fi

# sail256: GMP-free fixed-width Sail runtime, host-optimized (SF_RUNTIME overrides).
SF="${SF_RUNTIME:-$ROOT/zkvm/runtime/sail256}"
RT="$ROOT/zkvm/runtime"
FFI_ROOT="$ROOT/extractions/c"
if [ "$EVM_BUILD_MODE" = standard ]; then
  MODEL_FFI="$FFI_ROOT/spec/contract"
  MODEL_C_INCLUDE_FLAGS=(-I"$MODEL_FFI")
  MODEL_SOURCE="$BUILD/zkvm_block.c"
  MODEL_HEADER="zkvm_block.h"
else
  MODEL_FFI="$FFI_ROOT/optimised/contract"
  MODEL_HEADER="$OPTIMIZED_PACKAGE/spec.h"
  MODEL_C_INCLUDE_FLAGS=(-I"$OPTIMIZED_GENERATED/include" -I"$OPTIMIZED_STAGED_FFI")
fi
MODEL_HEADER_FLAG="-DEVMSAIL_MODEL_H=\"$MODEL_HEADER\""

# Inject the owning FFI headers directly. There is deliberately no aggregate
# model/input umbrella: each external operation is declared by its subsystem.
if [ "$EVM_BUILD_MODE" = optimized ]; then
  MODEL_HEADERS=()
  NATIVE_TEST_SOURCE="$OPTIMIZED_STAGED_FFI/test/native.c"
  NATIVE_DEBUG_SOURCE="$OPTIMIZED_STAGED_FFI/test/debug.c"
  ADDRESS_RESULT_SOURCE=""
else
  MODEL_HEADERS=(
    sail_failure.h exceptions.h region_access.h hash.h precompiles.h output.h scratch.h
    memory.h transient_storage.h stack.h frame_stack.h code_db.h
    kernel_state.h trie_node_db.h state_db.h native_test.h
  )
  MODEL_STATE_SOURCE="$MODEL_FFI/state.c"
  STATE_KERNEL_SOURCE="$MODEL_FFI/state_db.c"
  HASH_SOURCE="$MODEL_FFI/hash.c"
  CODE_SOURCE="$MODEL_FFI/code.c"
  REGION_ACCESS_SOURCE="$MODEL_FFI/region_access.c"
  FRAME_STACK_SOURCE="$MODEL_FFI/frame_stack.c"
  NATIVE_TEST_SOURCE="$MODEL_FFI/native_test.c"
  NATIVE_DEBUG_SOURCE=""
  ADDRESS_RESULT_SOURCE="$MODEL_FFI/address_result.c"
fi
if [ "$EVM_PROFILE" = on ]; then
  MODEL_HEADERS+=(cycle_scopes.h)
fi
MODEL_INCLUDE_FLAGS=()
# Bash 3.2 with `set -u` rejects expansion of an explicitly empty array. The
# optimized, profile-off build deliberately injects no legacy model headers.
if [ "$EVM_BUILD_MODE" != optimized ] || [ "$EVM_PROFILE" = on ]; then
  for header in "${MODEL_HEADERS[@]}"; do
    MODEL_INCLUDE_FLAGS+=(--c-include "$header")
  done
fi

# --- 1. accel-host crypto cdylib (idempotent) ------------------------------
ACCEL="$ROOT/zkvm/accel-host"
ACCEL_LIB="$ACCEL/target/release"
if [ ! -f "$ACCEL_LIB/libzkvm_accel_host.dylib" ] && \
   [ ! -f "$ACCEL_LIB/libzkvm_accel_host.so" ]; then
  echo "# building accel-host cdylib (one-time)..."
  ( cd "$ACCEL" && cargo build --release --offline --target-dir target )
fi
ACCEL_FLAGS=(-L"$ACCEL_LIB" -lzkvm_accel_host -Wl,-rpath,"$ACCEL_LIB")

# --- big main-thread stack (recursive Sail list ops over multi-MB inputs) ---
case "$(uname -s)" in
  Darwin) STACK_FLAGS=(-Wl,-stack_size,0x20000000) ;;
  *)      STACK_FLAGS=(-Wl,-z,stacksize=0x20000000) ;;
esac

if [ "$EVM_BUILD_MODE" = optimized ]; then
  EVM_OPT_LEVEL="${EVM_OPT_LEVEL:-3}"
  EVM_LTO="${EVM_LTO:-on}"
else
  EVM_OPT_LEVEL="${EVM_OPT_LEVEL:-2}"
  EVM_LTO="${EVM_LTO:-off}"
fi
case "$EVM_OPT_LEVEL" in
  0|1|2|3|s|z|g) ;;
  *) echo "error: EVM_OPT_LEVEL must be a compiler optimization level" >&2; exit 2 ;;
esac
case "$EVM_LTO" in
  off|on) ;;
  *) echo "error: EVM_LTO must be off or on" >&2; exit 2 ;;
esac
CFLAGS=(-O"$EVM_OPT_LEVEL" -Wno-error=implicit-function-declaration)
if [ "$EVM_LTO" = on ]; then CFLAGS+=(-flto); fi
MODEL_CFLAGS=()
if [ "$EVM_BUILD_MODE" = standard ]; then
  GMP_CFLAGS=()
  GMP_LINK_FLAGS=(-lgmp)
  if command -v pkg-config >/dev/null 2>&1 && pkg-config --exists gmp; then
    read -r -a GMP_CFLAGS <<< "$(pkg-config --cflags gmp)"
    read -r -a GMP_LINK_FLAGS <<< "$(pkg-config --libs gmp)"
  fi
  CFLAGS+=("${GMP_CFLAGS[@]}")
else
  CFLAGS+=(
    -DEVMSAIL_NATIVE_TEST
    -DEVMSAIL_OPTIMIZED_FFI
    -ffunction-sections
    -fdata-sections
  )
  MODEL_CFLAGS+=(-fvisibility=hidden)
fi
if [ -n "${SANITIZE:-}" ]; then CFLAGS+=(-g -fsanitize=address,undefined -fno-omit-frame-pointer); fi

# --- 2. Sail C runtime objects ---------------------------------------------
if [ "$EVM_BUILD_MODE" = standard ]; then
  RUNTIME_DIR="$SAIL_LIB"
  RUNTIME_SOURCES=(sail.c rts.c elf.c sail_failure.c)
  RUNTIME_LINK_FLAGS=("${GMP_LINK_FLAGS[@]}")
else
  RUNTIME_DIR="$SF"
  RUNTIME_SOURCES=(sail.c sail_native.c sail_failure.c)
  RUNTIME_LINK_FLAGS=()
fi
RUNTIME_OBJS=()
for src in "${RUNTIME_SOURCES[@]}"; do
  o="$BUILD/sf_${src%.c}.o"
  "$CC" "${CFLAGS[@]}" -c -I"$RUNTIME_DIR" -I"$SAIL_LIB" "$RUNTIME_DIR/$src" -o "$o"
  RUNTIME_OBJS+=("$o")
done

# --- 3. generate guest C (no main, preserve entry symbol) -------------------
#   run from repo root so the project files resolve their Sail sources.
#   test_utils.c's debug snapshot calls these model functions directly, so they
#   must remain externally linkable even when the guest entry does not call them.
#   EXTRA_PRESERVE adds any one-off inspection symbols.
PRESERVE_FLAGS=(
  --c-preserve main
  --c-preserve resume_frame
  --c-preserve validation_debug_record
  --c-preserve write_invalid_result
)
if [ "$EVM_BUILD_MODE" = standard ]; then
  PRESERVE_FLAGS+=(--c-preserve debug_account_storage_root)
else
  PRESERVE_FLAGS+=(
    --c-preserve sload_cost
    --c-preserve sstore_sentry_cost
    --c-preserve sstore_costs
  )
  if [ "$EVM_GENERATED_INTERP" = off ]; then
    for callback in \
      account_execution_context exceptional_state frame_output \
      opcode_frame_status refresh_account_execution_context \
      resume_frame run_call run_create run_frame_entry_encoded opcode_available \
      execute_push_encoded execute_dup_encoded execute_swap_encoded \
      execute_log_encoded execute_deep_stack_encoded; do
      PRESERVE_FLAGS+=(--c-preserve "$callback")
    done
    while IFS= read -r callback; do
      PRESERVE_FLAGS+=(--c-preserve "$callback")
    done < <(rg -o --no-filename 'execute_[A-Za-z0-9_]+' \
      "$MODEL_FFI/src/evm/interpreter.c" | sort -u)
  fi
fi
for s in ${EXTRA_PRESERVE:-}; do PRESERVE_FLAGS+=(--c-preserve "$s"); done
if [ "$EVM_BUILD_MODE" = optimized ]; then
  SAIL_CMD=(
    "$SAIL" "${SAIL_Z3_FLAGS[@]}"
    -c -O --Oconstant-fold --all-modules
    --c-optimized-model --c-package "$OPTIMIZED_PACKAGE"
    --c-output-dir "$OPTIMIZED_GENERATED"
    --c-specialization-limit "$C_OPT_SPECIALIZATION_LIMIT"
    --c-optimized-source-root sail
    --c-optimized-include-dir "$MODEL_FFI/include"
    --c-optimized-external-type StatelessInputSliceFields=evmsail/host/types.h
    --c-optimized-external-type ScratchSliceFields=evmsail/host/types.h
    --c-optimized-external-type EvmMemorySliceFields=evmsail/host/types.h
    --c-optimized-external-type CodeRegionSliceFields=evmsail/host/types.h
    --c-optimized-external-type LogDataSliceFields=evmsail/host/types.h
    --c-optimized-external-type OutputSliceFields=evmsail/host/types.h
    --c-optimized-external-type PreparedAuthorizationList=evmsail/host/types.h
    --c-optimized-external-type StackPointer=evmsail/host/stack.h
    --c-optimized-byte-pointer-field StatelessInputSliceFields.bytes=__direct
    --c-optimized-byte-pointer-field ScratchSliceFields.bytes=__direct
    --c-optimized-byte-pointer-field EvmMemorySliceFields.bytes=__direct
    --c-optimized-byte-pointer-field CodeRegionSliceFields.bytes=__direct
    --c-optimized-byte-pointer-field CodeFields.bytes=__direct
    --c-optimized-byte-pointer-field LogDataSliceFields.bytes=__direct
    --c-optimized-byte-pointer-field OutputSliceFields.bytes=__direct
    ${CONST_TABLE_FLAGS[@]+"${CONST_TABLE_FLAGS[@]}"}
    "${PRESERVE_FLAGS[@]}"
  )
else
  SAIL_CMD=(
    "$SAIL" "${SAIL_Z3_FLAGS[@]}"
    -c -O --Oconstant-fold --c-no-main --c-no-rts
    --c-specialize
    --c-specialization-limit "$C_SPEC_SPECIALIZATION_LIMIT"
    "${PRESERVE_FLAGS[@]}"
  )
fi
if [ "$EVM_BUILD_MODE" != optimized ] || [ "$EVM_PROFILE" = on ]; then
  SAIL_CMD+=("${MODEL_INCLUDE_FLAGS[@]}")
fi
if [ "${EVM_SAIL_LOG:-off}" = on ]; then
  SAIL_CMD+=(--c-specialize-log)
fi
if [ "$EVM_BUILD_MODE" = optimized ]; then
  while IFS= read -r relative || [ -n "$relative" ]; do
    [ -z "$relative" ] && continue
    if [ "$EVM_GENERATED_INTERP" = on ] && [ "$relative" = "evm/interpreter.sail" ]; then
      continue
    fi
    SAIL_CMD+=(--splice "$C_OPTIMISED_DIR/$relative")
  done < "$OPTIMIZED_SPLICE_MANIFEST_CANONICAL"
  if [ "$EVM_PROFILE" = on ]; then
    canonicalize_manifest "$C_PROFILE_DIR" "$C_PROFILE_MANIFEST" \
      "$PROFILE_SPLICE_MANIFEST_CANONICAL"
    while IFS= read -r relative || [ -n "$relative" ]; do
      [ -z "$relative" ] && continue
      SAIL_CMD+=(--splice "$C_PROFILE_DIR/$relative")
    done < "$PROFILE_SPLICE_MANIFEST_CANONICAL"
  fi
fi
SAIL_CMD+=(
  sail/evm.sail_project evm
  --variable EVM_DEBUG=on
)
if [ "$EVM_BUILD_MODE" = standard ]; then
  SAIL_CMD+=(-o "$BUILD/zkvm_block")
fi
( cd "$ROOT" && "${SAIL_CMD[@]}" )
if [ "$EVM_BUILD_MODE" = optimized ]; then
  package_args=()
  if [ "$EVM_GENERATED_INTERP" = on ]; then
    package_args+=(--exclude-ffi-source evm/interpreter.c)
  fi
  ( cd "$ROOT" && uv run --frozen python -m devtools.optimised_c.package \
    "$OPTIMIZED_GENERATED" ${package_args[@]+"${package_args[@]}"} )
  canonicalize_manifest "$OPTIMIZED_GENERATED/src/spec" \
    "$OPTIMIZED_MODEL_MANIFEST" "$OPTIMIZED_MODEL_MANIFEST_CANONICAL"
  canonicalize_manifest "$OPTIMIZED_STAGED_FFI" \
    "$OPTIMIZED_STAGED_FFI/sources.list" "$OPTIMIZED_FFI_MANIFEST_CANONICAL"
fi

# NOTE: the toolchain's sail.h (-I"$SAIL_LIB") #includes <gmp.h>. In optimized
# builds the GMP-free sail256 runtime ships its own sail.h, so -I"$RUNTIME_DIR"
# MUST precede -I"$SAIL_LIB" in every unit that includes sail.h.

# --- 4. compile generated model --------------------------------------------
MODEL_OBJS=()
if [ "$EVM_BUILD_MODE" = optimized ]; then
  [ -s "$OPTIMIZED_MODEL_MANIFEST" ] || { echo "error: missing generated model manifest" >&2; exit 2; }
  while IFS= read -r relative || [ -n "$relative" ]; do
    [ -z "$relative" ] && continue
    source="$OPTIMIZED_GENERATED/src/spec/$relative"
    [ -f "$source" ] || { echo "error: missing generated model source: $relative" >&2; exit 2; }
    object_name="${relative%.c}"
    object_name="${object_name//\//__}"
    object="$BUILD/model__${object_name}.o"
    "$CC" "${CFLAGS[@]}" "${MODEL_CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" "${MODEL_C_INCLUDE_FLAGS[@]}" -I"$FFI_ROOT" \
        "$MODEL_HEADER_FLAG" -c "$source" -o "$object"
    MODEL_OBJS+=("$object")
  done < "$OPTIMIZED_MODEL_MANIFEST_CANONICAL"
  [ "${#MODEL_OBJS[@]}" -gt 0 ] || { echo "error: empty generated model manifest" >&2; exit 2; }
else
  object="$BUILD/zkvm_block.o"
  "$CC" "${CFLAGS[@]}" "${MODEL_CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" "${MODEL_C_INCLUDE_FLAGS[@]}" -I"$FFI_ROOT" \
      "$MODEL_HEADER_FLAG" -c "$MODEL_SOURCE" -o "$object"
  MODEL_OBJS+=("$object")
fi

# --- 4b. Selected model backend. Optimized sources are a deterministic
# Sail-shaped manifest; the specification backend retains its standalone ABI.
MODEL_BACKEND_OBJS=()
if [ "$EVM_BUILD_MODE" = optimized ]; then
  while IFS= read -r relative || [ -n "$relative" ]; do
    case "$relative" in ''|'#'*) continue ;; esac
    source="$OPTIMIZED_STAGED_FFI/$relative"
    [ -f "$source" ] || { echo "error: missing optimized source: $relative" >&2; exit 2; }
    object_name="${relative%.c}"
    object_name="${object_name//\//__}"
    object="$BUILD/optimized__${object_name}.o"
    "$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" "${MODEL_C_INCLUDE_FLAGS[@]}" -I"$FFI_ROOT" \
        "$MODEL_HEADER_FLAG" \
        -c "$source" -o "$object"
    MODEL_BACKEND_OBJS+=("$object")
  done < "$OPTIMIZED_FFI_MANIFEST_CANONICAL"
else
  backend_sources=(
    "$MODEL_STATE_SOURCE:model_state"
    "$HASH_SOURCE:hash"
    "$CODE_SOURCE:model_code"
    "$REGION_ACCESS_SOURCE:region_access"
    "$FRAME_STACK_SOURCE:model_frame_stack"
  )
  if [ -n "$ADDRESS_RESULT_SOURCE" ]; then
    backend_sources+=("$ADDRESS_RESULT_SOURCE:model_address_result")
  fi
  for source_and_name in "${backend_sources[@]}"; do
    source="${source_and_name%:*}"
    object="$BUILD/${source_and_name##*:}.o"
    "$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" "${MODEL_C_INCLUDE_FLAGS[@]}" -I"$FFI_ROOT" \
        "$MODEL_HEADER_FLAG" \
        -c "$source" -o "$object"
    MODEL_BACKEND_OBJS+=("$object")
  done
fi

# --- 5. backend-specific native harness + CLI main ---------------------------
#   Each backend owns its lifecycle and debug bridge so generated-model ABI
#   details do not leak back into a shared native utility.
"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$HERE" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" "${MODEL_C_INCLUDE_FLAGS[@]}" -I"$FFI_ROOT" \
    "$MODEL_HEADER_FLAG" \
    -c "$NATIVE_TEST_SOURCE" -o "$BUILD/native_test.o"
NATIVE_TEST_OBJS=("$BUILD/native_test.o")
if [ -n "$NATIVE_DEBUG_SOURCE" ]; then
  "$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$HERE" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" "${MODEL_C_INCLUDE_FLAGS[@]}" -I"$FFI_ROOT" \
      "$MODEL_HEADER_FLAG" \
      -c "$NATIVE_DEBUG_SOURCE" -o "$BUILD/native_debug.o"
  NATIVE_TEST_OBJS+=("$BUILD/native_debug.o")
fi
"$CC" "${CFLAGS[@]}" -c "$HERE/main.c" -o "$BUILD/main.o"

# --- 6. C host backends + direct precompile adapter -------------------------
HOST_OBJS=()
if [ "$EVM_BUILD_MODE" = standard ]; then
  HOST_SOURCES=(capacity.c exceptions.c memory.c scratch.c transient_storage.c stack.c code_db.c kernel_state.c precompiles.c output.c)
  HOST_SOURCES+=(trie_node_db.c state_db.c)
  for source in "${HOST_SOURCES[@]}"; do
    object_name="$(basename "${source%.c}")"
    o="$BUILD/$object_name.o"
    "$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" "${MODEL_C_INCLUDE_FLAGS[@]}" -I"$FFI_ROOT" \
        "$MODEL_HEADER_FLAG" \
        -c "$MODEL_FFI/$source" -o "$o"
    HOST_OBJS+=("$o")
  done
fi
if [ "$EVM_PROFILE" = on ]; then
  o="$BUILD/cycle_scopes.o"
  "$CC" "${CFLAGS[@]}" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -c "$RT/cycle_scopes.c" -o "$o"
  HOST_OBJS+=("$o")
fi

# --- 7. link ----------------------------------------------------------------
OUT="$BUILD/zkvm_native"
LINK_CMD=("$CC" "${CFLAGS[@]}"
    "${MODEL_OBJS[@]}" "${MODEL_BACKEND_OBJS[@]}"
    "${NATIVE_TEST_OBJS[@]}" "$BUILD/main.o"
    "${RUNTIME_OBJS[@]}"
    "${ACCEL_FLAGS[@]}")
if [ "$EVM_BUILD_MODE" = standard ] || [ "$EVM_PROFILE" = on ]; then
  LINK_CMD+=("${HOST_OBJS[@]}")
fi
if [ "$EVM_BUILD_MODE" = standard ]; then
  LINK_CMD+=("${RUNTIME_LINK_FLAGS[@]}")
else
  case "$(uname -s)" in
    Darwin) LINK_CMD+=(-Wl,-dead_strip) ;;
    *)      LINK_CMD+=(-Wl,--gc-sections) ;;
  esac
fi
LINK_CMD+=("${STACK_FLAGS[@]}" -o "$OUT")
echo "# link:"
printf '  %q' "${LINK_CMD[@]}"; echo
"${LINK_CMD[@]}"

echo "built $OUT ($EVM_BUILD_MODE)"
