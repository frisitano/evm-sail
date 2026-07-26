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
mkdir -p "$BUILD"

SAIL="$(bash "$ROOT/zkvm/resolve_optimized_sail.sh")"
export SAIL
CC="${CC:-cc}"
EVM_PROFILE="${EVM_PROFILE:-off}"
EVM_BUILD_MODE="${EVM_BUILD_MODE:-optimized}"
case "$EVM_PROFILE" in
  off|on) ;;
  *) echo "error: EVM_PROFILE must be off or on" >&2; exit 2 ;;
esac
case "$EVM_BUILD_MODE" in
  standard|optimized) ;;
  *) echo "error: EVM_BUILD_MODE must be standard or optimized" >&2; exit 2 ;;
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
FFI="$ROOT/ffi"

# Inject the owning FFI headers directly. There is deliberately no aggregate
# model/input umbrella: each external operation is declared by its subsystem.
MODEL_HEADERS=(
  byte_slice_glue.h host_crypto.h precompiles.h output.h scratch.h memory.h
  transient_storage.h stack.h frame_stack.h code_db.h kernel_state.h trie_node_db.h
  state_db.h cycle_scopes.h
)
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
  CFLAGS+=(-DEVMSAIL_STANDARD_ABI)
  GMP_CFLAGS=()
  GMP_LINK_FLAGS=(-lgmp)
  if command -v pkg-config >/dev/null 2>&1 && pkg-config --exists gmp; then
    read -r -a GMP_CFLAGS <<< "$(pkg-config --cflags gmp)"
    read -r -a GMP_LINK_FLAGS <<< "$(pkg-config --libs gmp)"
  fi
  CFLAGS+=("${GMP_CFLAGS[@]}")
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
  --c-preserve debug_account_storage_root
  --c-preserve debug_rebuild_state_root
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
fi
SAIL_CMD+=(
  sail/evm.sail_project evm
  --variable EVM_PROFILE="$EVM_PROFILE"
  --variable EVM_DEBUG=on
  -o "$BUILD/zkvm_block"
)
( cd "$ROOT" && "${SAIL_CMD[@]}" )

# NOTE: the toolchain's sail.h (-I"$SAIL_LIB") #includes <gmp.h>. In optimized
# builds the GMP-free sail256 runtime ships its own sail.h, so -I"$RUNTIME_DIR"
# MUST precede -I"$SAIL_LIB" in every unit that includes sail.h.

# --- 4. compile generated model --------------------------------------------
"$CC" "${CFLAGS[@]}" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$FFI" \
    -c "$BUILD/zkvm_block.c" -o "$BUILD/zkvm_block.o"

# --- 4b. state aggregate glue: account/storage rows use generated layouts;
#     the rollback journal itself is C-private.
"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$FFI" \
    -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
    -c "$FFI/journal_glue.c" -o "$BUILD/journal_glue.o"

"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$FFI" \
    -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
    -c "$FFI/hash_glue.c" -o "$BUILD/hash_glue.o"

"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$FFI" \
    -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
    -c "$FFI/code_glue.c" -o "$BUILD/code_glue.o"

"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$FFI" \
    -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
    -c "$FFI/byte_slice_glue.c" -o "$BUILD/byte_slice_glue.o"

"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$FFI" \
    -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
    -c "$FFI/address_result_glue.c" -o "$BUILD/address_result_glue.o"

"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$FFI" \
    -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
    -c "$FFI/frame_stack_glue.c" -o "$BUILD/frame_stack_glue.o"

# --- 5. shared harness I/O + CLI main ---------------------------------------
#   test_utils.c supplies the native standard I/O implementation, large-stack
#   run, and clear-memory hooks shared by this executable and build_lib.sh.
"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$RT" -I"$FFI" \
    -DEVMSAIL_MODEL_H='"zkvm_block.h"' \
    -c "$HERE/test_utils.c" -o "$BUILD/test_utils.o"
"$CC" "${CFLAGS[@]}" -c "$HERE/main.c" -o "$BUILD/main.o"

# --- 6. C host backends + direct host crypto/precompile adapters ------------
HOST_OBJS=()
for hc in memory scratch transient_storage state_db stack code_db kernel_state trie_node_db host_crypto precompiles output; do
  o="$BUILD/$hc.o"
  "$CC" "${CFLAGS[@]}" -I"$RUNTIME_DIR" -I"$SAIL_LIB" -I"$FFI" -c "$FFI/$hc.c" -o "$o"
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
    "$BUILD/zkvm_block.o" "$BUILD/journal_glue.o" "$BUILD/hash_glue.o" "$BUILD/code_glue.o" "$BUILD/byte_slice_glue.o" "$BUILD/address_result_glue.o" "$BUILD/frame_stack_glue.o" "$BUILD/test_utils.o" "$BUILD/main.o"
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
