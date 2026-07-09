#!/usr/bin/env bash
# ===========================================================================
# Build libevmsail_guest.{dylib,so}: the evm-sail stateless guest as a SHARED
# LIBRARY for an in-process (ctypes) conformance harness, instead of the
# per-fixture subprocess exe (build.sh).
#
# It reuses every object build.sh produces (generated model C, sail256 runtime,
# FFI host backends, and the shared test_utils.c harness I/O) and simply drops
# the exe's CLI driver main.o -- the exe and this lib share ONE native I/O +
# run_once surface (test_utils.c).
#
#   export PATH="$HOME/.opam/sail/bin:$PATH"
#   eval "$(opam env --root=/Users/f/.opam --switch=sail)"
#   ./build_lib.sh
# ===========================================================================
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
BUILD="$HERE/.build"

# 1. produce all objects (generated model C, sf runtime, FFI, test_utils
#    harness). No EXTRA_PRESERVE needed: the reset shim (test_utils.c) calls
#    the FFI reset symbols directly, so it depends on no Sail-level reset.
"$HERE/build.sh"

SAIL="${SAIL:-sail}"
CC="${CC:-cc}"
SAILBIN="$(command -v "$SAIL")"
SAIL_LIB="$(cd "$(dirname "$SAILBIN")/../share/sail/lib" && pwd)"
SF="${SF_RUNTIME:-$ROOT/zkvm/runtime/sail256}"
ZKVM="$ROOT/zkvm"; RT="$ROOT/zkvm/runtime"; FFI="$ROOT/ffi"
ACCEL_LIB="$ROOT/zkvm/accel-host/target/release"
CFLAGS=(-O2 -Wno-error=implicit-function-declaration)
if [ -n "${SANITIZE:-}" ]; then CFLAGS+=(-g -fsanitize=address,undefined -fno-omit-frame-pointer); fi

# 2. link a shared library: build.sh's objects (incl. test_utils.o), minus main.o
HOST_OBJS=()
for hc in memory transient_storage state_db stack code_db kernel_state trie_node_db host_crypto precompiles returndata; do
  HOST_OBJS+=("$BUILD/$hc.o")
done
case "$(uname -s)" in
  Darwin) SHFLAG=(-dynamiclib -install_name "@rpath/libevmsail_guest.dylib"); EXT=dylib ;;
  *)      SHFLAG=(-shared); EXT=so ;;
esac
OUT="$BUILD/libevmsail_guest.$EXT"
# test_utils.o is the self-contained I/O + harness: it owns set_input, ssz_src_*,
# el_emit_out, the output buffer, and run_once. The real guests' I/O
# (zkvm_input.c / zkvm_io.c) is never linked into host builds.
LINK_CMD=("$CC" "${CFLAGS[@]}" "${SHFLAG[@]}"
    "$BUILD/zkvm_block.o" "$BUILD/test_utils.o"
    "${HOST_OBJS[@]}"
    "$BUILD/sf_sail.o" "$BUILD/sf_sail_native.o" "$BUILD/sf_sail_failure.o"
    -L"$ACCEL_LIB" -lzkvm_accel_host -Wl,-rpath,"$ACCEL_LIB"
    -o "$OUT")
echo "# link lib:"
printf '  %q' "${LINK_CMD[@]}"; echo
"${LINK_CMD[@]}"
echo "built $OUT"
