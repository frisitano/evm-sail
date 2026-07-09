#!/usr/bin/env bash
# ===========================================================================
# Build a NATIVE (host, not RISC-V) conformance runner for the evm-sail zkVM
# stateless block guest (sail/evm.sail_project evm, EVM_ENTRY=guest -> main).
#
# This mirrors the guest sail-compile flags in zkvm/build.sh (--c-no-main,
# --c-preserve main, --c-include zkvm_input.h) with the host-optimized sail256
# runtime and the directly-linked Rust accel-host cdylib.
#
# I/O + run harness = test_utils.c, the ONE shared native surface (input
# buffer + ssz_src, emit_out sink, large-stack run_once, clear_memory) also
# linked by the ctypes libs. The real guests' I/O (zkvm_input.c baked/ere,
# zkvm_io.c HTIF) is never linked into host builds.
#
# Idempotent: rebuilds the accel-host cdylib only if its library is missing.
#
# Requires: sail (opam), a C compiler, cargo. NO gmp, NO HTIF, NO spike.
#
#   export PATH="$HOME/.opam/sail/bin:$PATH"
#   eval "$(opam env --root=/Users/f/.opam --switch=sail)"
#   ./build.sh
# ===========================================================================
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"      # repo root (evm-sail)
BUILD="$HERE/.build"
mkdir -p "$BUILD"

SAIL="${SAIL:-sail}"
CC="${CC:-cc}"

# --- Sail C runtime include dir (where sail.h lives) ------------------------
SAILBIN="$(command -v "$SAIL")"
SAIL_LIB="$(cd "$(dirname "$SAILBIN")/../share/sail/lib" && pwd)"
if [ ! -f "$SAIL_LIB/sail.h" ]; then
  echo "error: sail.h not found under $SAIL_LIB" >&2
  exit 1
fi

# sail256: GMP-free fixed-width Sail runtime, host-optimized (SF_RUNTIME overrides).
SF="${SF_RUNTIME:-$ROOT/zkvm/runtime/sail256}"
RT="$ROOT/zkvm/runtime"
ZKVM="$ROOT/zkvm"          # zkvm_input.h / zkvm_io.h live here
FFI="$ROOT/ffi"

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
if [ -n "${SANITIZE:-}" ]; then CFLAGS+=(-g -fsanitize=address,undefined -fno-omit-frame-pointer); fi

# --- 2. sail256 GMP-free runtime objects -----------------------------------
SF_OBJS=()
for src in sail.c sail_native.c sail_failure.c; do
  o="$BUILD/sf_${src%.c}.o"
  "$CC" "${CFLAGS[@]}" -c -I"$SF" -I"$SAIL_LIB" "$SF/$src" -o "$o"
  SF_OBJS+=("$o")
done

# --- 3. generate guest C (no main, preserve entry symbol) -------------------
#   run from repo root so the project files resolve their Sail sources.
#   EXTRA_PRESERVE (space-separated Sail names) keeps otherwise-DCE'd functions
#   externally linkable (none needed today; build_runner_lib.sh preserves
#   compute_state_root via its own flag).
PRESERVE_FLAGS=(--c-preserve main)
for s in ${EXTRA_PRESERVE:-}; do PRESERVE_FLAGS+=(--c-preserve "$s"); done
( cd "$ROOT" && "$SAIL" -c -O --c-no-main --c-no-rts "${PRESERVE_FLAGS[@]}" \
    --c-include zkvm_input.h \
    sail/evm.sail_project evm \
    --variable EVM_BACKEND=build \
    --variable EVM_ENTRY=guest \
    -o "$BUILD/zkvm_block" )

# NOTE: the toolchain's sail.h (-I"$SAIL_LIB") #includes <gmp.h>. The GMP-free
# sail256 runtime ships its own GMP-free sail.h, so -I"$SF" MUST precede
# -I"$SAIL_LIB" in every unit that includes sail.h.

# --- 4. compile generated model --------------------------------------------
#   -I zkvm/runtime so the generated C finds the injected zkvm_input.h.
"$CC" "${CFLAGS[@]}" -I"$SF" -I"$SAIL_LIB" -I"$ZKVM" -I"$RT" -I"$FFI" \
    -c "$BUILD/zkvm_block.c" -o "$BUILD/zkvm_block.o"

# --- 5. shared harness I/O + CLI main ---------------------------------------
#   test_utils.c is the ONE native I/O + run_once surface (input buffer,
#   ssz_src accessors, emit_out sink, large-stack run, clear_memory) shared by
#   this exe and the ctypes libs (build_lib.sh / build_runner_lib.sh). The real
#   guest keeps its own I/O in zkvm_input.c / zkvm_io.c (baked vector, HTIF) --
#   never linked here.
"$CC" "${CFLAGS[@]}" -I"$SF" -I"$SAIL_LIB" -I"$ZKVM" -I"$RT" -I"$FFI" \
    -c "$HERE/test_utils.c" -o "$BUILD/test_utils.o"
"$CC" "${CFLAGS[@]}" -c "$HERE/main.c" -o "$BUILD/main.o"

# --- 6. C host backends + direct host crypto/precompile adapters ------------
HOST_OBJS=()
for hc in memory transient_storage state_db stack code_db kernel_state trie_node_db host_crypto precompiles returndata; do
  o="$BUILD/$hc.o"
  "$CC" "${CFLAGS[@]}" -I"$SF" -I"$SAIL_LIB" -I"$FFI" -c "$FFI/$hc.c" -o "$o"
  HOST_OBJS+=("$o")
done

# --- 7. link ----------------------------------------------------------------
OUT="$BUILD/zkvm_native"
LINK_CMD=("$CC" "${CFLAGS[@]}"
    "$BUILD/zkvm_block.o" "$BUILD/test_utils.o" "$BUILD/main.o"
    "${HOST_OBJS[@]}" "${SF_OBJS[@]}"
    "${ACCEL_FLAGS[@]}" "${STACK_FLAGS[@]}"
    -o "$OUT")
echo "# link:"
printf '  %q' "${LINK_CMD[@]}"; echo
"${LINK_CMD[@]}"

echo "built $OUT"
