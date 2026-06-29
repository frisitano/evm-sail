#!/usr/bin/env bash
# ===========================================================================
# Build a NATIVE (host, not RISC-V) conformance runner for the evm-sail zkVM
# stateless block guest (sail/main.sail -> main).
#
# This mirrors:
#   - the EEST sail256 native build in revm-eest/run_eest.py (accel-host cdylib,
#     GMP-free sail256 runtime, host_*.c backends, acc_shim marshalling, big
#     main-thread stack), and
#   - the guest sail-compile flags in zkvm/build.sh (--c-no-main, --c-preserve
#     zkvm_run, --c-include zkvm_input.h).
#
# It compiles zkvm/runtime/zkvm_input.c with -DERE_GUEST so the SSZ input is
# supplied at runtime (evmsail_set_input) and output is buffered for a single
# write_output (provided here natively by native_io.c — NOT the spike HTIF I/O).
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

# sail256: GMP-free fixed-width Sail runtime (matches run_eest.py SAIL256 path).
SF="$ROOT/zkvm/runtime/sail256"
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

# --- 2. sail256 GMP-free runtime objects -----------------------------------
SF_OBJS=()
for src in sail.c sail_native.c sail_failure.c; do
  o="$BUILD/sf_${src%.c}.o"
  "$CC" "${CFLAGS[@]}" -c -I"$SF" -I"$SAIL_LIB" "$SF/$src" -o "$o"
  SF_OBJS+=("$o")
done

# --- 3. generate guest C (no main, preserve entry symbol) -------------------
#   run from repo root so the relative $include in sail/main.sail resolves.
( cd "$ROOT" && "$SAIL" -c -O --c-no-main --c-no-rts --c-preserve main \
    --c-include zkvm_input.h \
    sail/main.sail -o "$BUILD/zkvm_block" )

# NOTE: the toolchain's sail.h (-I"$SAIL_LIB") #includes <gmp.h>. The GMP-free
# sail256 runtime ships its own GMP-free sail.h, so -I"$SF" MUST precede
# -I"$SAIL_LIB" in every unit that includes sail.h (exactly like run_eest.py's
# `-I{sfdir} -I{lib}` ordering).

# --- 4. compile generated model --------------------------------------------
#   -I zkvm/runtime so the generated C finds the injected zkvm_input.h.
"$CC" "${CFLAGS[@]}" -I"$SF" -I"$SAIL_LIB" -I"$ZKVM" -I"$RT" -I"$FFI" \
    -c "$BUILD/zkvm_block.c" -o "$BUILD/zkvm_block.o"

# --- 5. runtime input hook (-DERE_GUEST) + native IO + main -----------------
"$CC" "${CFLAGS[@]}" -DERE_GUEST -I"$SF" -I"$SAIL_LIB" -I"$ZKVM" -I"$RT" -I"$FFI" \
    -c "$RT/zkvm_input.c" -o "$BUILD/zkvm_input.o"
"$CC" "${CFLAGS[@]}" -c "$HERE/native_io.c" -o "$BUILD/native_io.o"
"$CC" "${CFLAGS[@]}" -I"$SF" -I"$SAIL_LIB" -c "$HERE/main.c" -o "$BUILD/main.o"

# --- 6. C host backends + acc_shim (acc_shim WITHOUT -DACCEL_MMIO) ----------
HOST_OBJS=()
for hc in host_mem host_map host_stack host_word host_code host_nodedb host_acctmap host_preimage acc_shim; do
  o="$BUILD/$hc.o"
  "$CC" "${CFLAGS[@]}" -I"$SF" -I"$SAIL_LIB" -I"$FFI" -c "$FFI/$hc.c" -o "$o"
  HOST_OBJS+=("$o")
done

# --- 7. link ----------------------------------------------------------------
OUT="$BUILD/zkvm_native"
LINK_CMD=("$CC" "${CFLAGS[@]}"
    "$BUILD/zkvm_block.o" "$BUILD/zkvm_input.o" "$BUILD/native_io.o" "$BUILD/main.o"
    "${HOST_OBJS[@]}" "${SF_OBJS[@]}"
    "${ACCEL_FLAGS[@]}" "${STACK_FLAGS[@]}"
    -o "$OUT")
echo "# link:"
printf '  %q' "${LINK_CMD[@]}"; echo
"${LINK_CMD[@]}"

echo "built $OUT"
