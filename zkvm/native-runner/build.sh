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
#   eval "$(opam env --root=/Users/f/.opam --switch=sail)"
#   ./build.sh
# ===========================================================================
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"      # repo root (evm-sail)
BUILD="${NATIVE_BUILD:-$HERE/.build}"
C_OPTIMIZED_SPLICE="$ROOT/sail/splices/c_optimized.sail"
C_OPTIMIZED_PROFILE_SPLICE="$ROOT/sail/splices/c_optimized_profile.sail"
mkdir -p "$BUILD"

SAIL="$(bash "$ROOT/zkvm/resolve_optimized_sail.sh")"
export SAIL
CC="${CC:-cc}"
EVM_PROFILE="${EVM_PROFILE:-off}"
EVM_BUILD_MODE="${EVM_BUILD_MODE:-optimized}"
EVM_CAPACITY_MODE="${EVM_CAPACITY_MODE:-fixed}"
case "$EVM_PROFILE" in
  off|on) ;;
  *) echo "error: EVM_PROFILE must be off or on" >&2; exit 2 ;;
esac
case "$EVM_BUILD_MODE" in
  standard|optimized) ;;
  *) echo "error: EVM_BUILD_MODE must be standard or optimized" >&2; exit 2 ;;
esac
if [ "$EVM_BUILD_MODE" = standard ] && [ "$EVM_PROFILE" = on ]; then
  echo "error: EVM_PROFILE=on is available only for optimized builds" >&2
  exit 2
fi
case "$EVM_CAPACITY_MODE" in
  fixed|measure) ;;
  *) echo "error: EVM_CAPACITY_MODE must be fixed or measure" >&2; exit 2 ;;
esac

# --- Sail C runtime include dir (where sail.h lives) ------------------------
# Query Sail rather than deriving this from the executable path. This also
# supports an uninstalled compiler worktree whose wrapper sets SAIL_DIR.
SAIL_LIB="$("$SAIL" --dir)/lib"
if [ ! -f "$SAIL_LIB/sail.h" ]; then
  echo "error: sail.h not found under $SAIL_LIB" >&2
  exit 1
fi

# sail256: GMP-free fixed-width Sail runtime, host-optimized (SF_RUNTIME overrides).
SF="${SF_RUNTIME:-$ROOT/zkvm/runtime/sail256}"
RT="$ROOT/zkvm/runtime"
FFI_ROOT="$ROOT/ffi"
if [ "$EVM_BUILD_MODE" = standard ]; then
  MODEL_FFI="$FFI_ROOT/spec"
else
  MODEL_FFI="$FFI_ROOT/optimized"
fi

# Inject the owning FFI headers directly. There is deliberately no aggregate
# model/input umbrella: each external operation is declared by its subsystem.
MODEL_HEADERS=(
  region_access.h hash.h precompiles.h output.h scratch.h memory.h
  transient_storage.h stack.h frame_stack.h code_db.h kernel_state.h trie_node_db.h
  state_db.h
)
if [ "$EVM_PROFILE" = on ]; then
  MODEL_HEADERS+=(cycle_scopes.h)
fi
if [ "$EVM_BUILD_MODE" = optimized ]; then
  MODEL_HEADERS+=(word_bytes.h preimage.h htr.h mpt.h state.h interpreter.h)
fi
MODEL_INCLUDE_FLAGS=()
for header in "${MODEL_HEADERS[@]}"; do
  MODEL_INCLUDE_FLAGS+=(--c-include "$header")
done

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

CFLAGS=(-O2 -Wno-error=implicit-function-declaration)
if [ "$EVM_BUILD_MODE" = standard ]; then
  GMP_CFLAGS=()
  GMP_LINK_FLAGS=(-lgmp)
  if command -v pkg-config >/dev/null 2>&1 && pkg-config --exists gmp; then
    read -r -a GMP_CFLAGS <<< "$(pkg-config --cflags gmp)"
    read -r -a GMP_LINK_FLAGS <<< "$(pkg-config --libs gmp)"
  fi
  CFLAGS+=("${GMP_CFLAGS[@]}")
else
  CFLAGS+=(-DEVMSAIL_NATIVE_DEBUG_AGGREGATES)
  if [ "$EVM_CAPACITY_MODE" = fixed ]; then
    CFLAGS+=(-DEVMSAIL_POINTER_ABI -DEVMSAIL_CAPACITY_FIXED)
  else
    CFLAGS+=(-DEVMSAIL_CAPACITY_MEASURE)
  fi
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
  --c-preserve leaf_child_ref
  --c-preserve resume_frame
  --c-preserve debug_account_storage_root
)
for s in ${EXTRA_PRESERVE:-}; do PRESERVE_FLAGS+=(--c-preserve "$s"); done
SAIL_CMD=(
  "$SAIL" -c -O --Oconstant-fold --c-no-main --c-no-rts
  --c-specialize
  "${PRESERVE_FLAGS[@]}"
  "${MODEL_INCLUDE_FLAGS[@]}"
)
if [ "${EVM_SAIL_LOG:-off}" = on ]; then
  SAIL_CMD+=(--c-specialize-log)
fi
if [ "$EVM_BUILD_MODE" = optimized ]; then
  SAIL_CMD+=(--c-require-bounded-int --splice "$C_OPTIMIZED_SPLICE")
  if [ "$EVM_PROFILE" = on ]; then
    SAIL_CMD+=(--splice "$C_OPTIMIZED_PROFILE_SPLICE")
  fi
fi
SAIL_CMD+=(
  sail/evm.sail_project evm
  --variable EVM_DEBUG=on
  -o "$BUILD/zkvm_block"
)
( cd "$ROOT" && "${SAIL_CMD[@]}" )

# NOTE: the toolchain's sail.h (-I"$SAIL_LIB") #includes <gmp.h>. In optimized
# builds the GMP-free sail256 runtime ships its own sail.h, so -I"$RUNTIME_DIR"
# MUST precede -I"$SAIL_LIB" in every unit that includes sail.h.

# --- 4. compile generated model --------------------------------------------
"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$MODEL_FFI" -I"$FFI_ROOT" \
    -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
    -c "$BUILD/zkvm_block.c" -o "$BUILD/zkvm_block.o"

# --- 4b. Selected model backend. The spec and optimized builds each own their
#     complete FFI implementation; only standardized platform headers are shared.
"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$MODEL_FFI" -I"$FFI_ROOT" \
    -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
    -c "$MODEL_FFI/state.c" -o "$BUILD/model_state.o"

"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$MODEL_FFI" -I"$FFI_ROOT" \
    -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
    -c "$MODEL_FFI/hash.c" -o "$BUILD/hash.o"

"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$MODEL_FFI" -I"$FFI_ROOT" \
    -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
    -c "$MODEL_FFI/code.c" -o "$BUILD/model_code.o"

"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$MODEL_FFI" -I"$FFI_ROOT" \
    -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
    -c "$MODEL_FFI/region_access.c" -o "$BUILD/region_access.o"

"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$MODEL_FFI" -I"$FFI_ROOT" \
    -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
    -c "$MODEL_FFI/address_result.c" -o "$BUILD/model_address_result.o"

"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$MODEL_FFI" -I"$FFI_ROOT" \
    -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
    -c "$MODEL_FFI/frame_stack.c" -o "$BUILD/model_frame_stack.o"

HTR_OBJ=""
PREIMAGE_OBJ=""
MPT_OBJ=""
INTERPRETER_OBJ=""
if [ "$EVM_BUILD_MODE" = optimized ]; then
  "$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$MODEL_FFI" -I"$FFI_ROOT" \
      -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$MODEL_FFI/preimage.c" -o "$BUILD/preimage.o"
  PREIMAGE_OBJ="$BUILD/preimage.o"
  "$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$MODEL_FFI" -I"$FFI_ROOT" \
      -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$MODEL_FFI/htr.c" -o "$BUILD/htr.o"
  HTR_OBJ="$BUILD/htr.o"
  "$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$MODEL_FFI" -I"$FFI_ROOT" \
      -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$MODEL_FFI/mpt.c" -o "$BUILD/mpt.o"
  MPT_OBJ="$BUILD/mpt.o"
  "$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$MODEL_FFI" -I"$FFI_ROOT" \
      -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$MODEL_FFI/interpreter.c" -o "$BUILD/interpreter.o"
  INTERPRETER_OBJ="$BUILD/interpreter.o"
fi

# --- 5. backend-specific native harness + CLI main ---------------------------
#   Each backend owns its lifecycle and debug bridge so generated-model ABI
#   details do not leak back into a shared native utility.
"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$HERE" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$MODEL_FFI" -I"$FFI_ROOT" \
    -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
    -c "$MODEL_FFI/native_test.c" -o "$BUILD/native_test.o"
"$CC" "${CFLAGS[@]}" -c "$HERE/main.c" -o "$BUILD/main.o"

# --- 6. C host backends + direct precompile adapter -------------------------
HOST_OBJS=()
for hc in capacity memory scratch transient_storage state_db stack code_db kernel_state trie_node_db precompiles output; do
  o="$BUILD/$hc.o"
  "$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$MODEL_FFI" -I"$FFI_ROOT" \
      -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
      -c "$MODEL_FFI/$hc.c" -o "$o"
  HOST_OBJS+=("$o")
done
if [ "$EVM_PROFILE" = on ]; then
  o="$BUILD/cycle_scopes.o"
  "$CC" "${CFLAGS[@]}" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -c "$RT/cycle_scopes.c" -o "$o"
  HOST_OBJS+=("$o")
fi

# --- 7. link ----------------------------------------------------------------
OUT="$BUILD/zkvm_native"
LINK_CMD=("$CC" "${CFLAGS[@]}"
    "$BUILD/zkvm_block.o" "$BUILD/model_state.o" "$BUILD/hash.o" "$BUILD/model_code.o" "$BUILD/region_access.o" "$BUILD/model_address_result.o" "$BUILD/model_frame_stack.o" "$BUILD/native_test.o" "$BUILD/main.o"
    ${PREIMAGE_OBJ:+"$PREIMAGE_OBJ"}
    ${HTR_OBJ:+"$HTR_OBJ"}
    ${MPT_OBJ:+"$MPT_OBJ"}
    ${INTERPRETER_OBJ:+"$INTERPRETER_OBJ"}
    "${HOST_OBJS[@]}" "${RUNTIME_OBJS[@]}"
    "${ACCEL_FLAGS[@]}")
if [ "$EVM_BUILD_MODE" = standard ]; then
  LINK_CMD+=("${RUNTIME_LINK_FLAGS[@]}")
fi
LINK_CMD+=("${STACK_FLAGS[@]}" -o "$OUT")
echo "# link:"
printf '  %q' "${LINK_CMD[@]}"; echo
"${LINK_CMD[@]}"

echo "built $OUT ($EVM_BUILD_MODE)"
