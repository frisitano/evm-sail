#!/usr/bin/env bash
# ===========================================================================
# Build libevmsail_guest.{dylib,so}: the evm-sail stateless guest as a SHARED
# LIBRARY for an in-process (ctypes) conformance harness, instead of the
# per-fixture subprocess exe (build.sh).
#
# It reuses every object build.sh produces (generated model C, its selected Sail
# runtime, FFI host backends, and the shared test_utils.c harness I/O) and simply
# drops the exe's CLI driver main.o -- the exe and this lib share ONE native I/O
# + run_once surface (test_utils.c).
#
#   export PATH="$HOME/.opam/sail/bin:$PATH"
#   eval "$(opam env --root=/Users/f/.opam --switch=sail)"
#   ./build_lib.sh
#   EVM_OPT_LEVEL=3 EVM_LTO=on ./build_lib.sh  # heavier release experiment
# ===========================================================================
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
BUILD="${NATIVE_BUILD:-$HERE/.build}"

# 1. produce all objects (generated model C, sf runtime, FFI, test_utils
#    harness). No EXTRA_PRESERVE needed: the reset shim (test_utils.c) calls
#    the FFI reset symbols directly, so it depends on no Sail-level reset.
"$HERE/build.sh"

CC="${CC:-cc}"
ZKVM="$ROOT/zkvm"; RT="$ROOT/zkvm/runtime"; FFI="$ROOT/ffi"
ACCEL_LIB="$ROOT/zkvm/accel-host/target/release"
EVM_OPT_LEVEL="${EVM_OPT_LEVEL:-2}"
EVM_LTO="${EVM_LTO:-off}"
OPT_FLAGS=(-O"$EVM_OPT_LEVEL")
if [ "$EVM_LTO" = on ]; then OPT_FLAGS+=(-flto); fi
CFLAGS=("${OPT_FLAGS[@]}" -Wno-error=implicit-function-declaration)
if [ -n "${SANITIZE:-}" ]; then CFLAGS+=(-g -fsanitize=address,undefined -fno-omit-frame-pointer); fi

if [ "${EVM_BUILD_MODE:-optimized}" = standard ]; then
  RUNTIME_OBJS=("$BUILD/sf_sail.o" "$BUILD/sf_rts.o" "$BUILD/sf_elf.o"
                "$BUILD/sf_sail_failure.o")
  RUNTIME_LINK_FLAGS=(-lgmp)
  if command -v pkg-config >/dev/null 2>&1 && pkg-config --exists gmp; then
    read -r -a RUNTIME_LINK_FLAGS <<< "$(pkg-config --libs gmp)"
  fi
else
  RUNTIME_OBJS=("$BUILD/sf_sail.o" "$BUILD/sf_sail_native.o" "$BUILD/sf_sail_failure.o")
  RUNTIME_LINK_FLAGS=()
fi

# 2. link a shared library: build.sh's objects (incl. test_utils.o), minus main.o
HOST_OBJS=()
for hc in memory scratch transient_storage state_db stack code_db kernel_state trie_node_db host_crypto precompiles output; do
  HOST_OBJS+=("$BUILD/$hc.o")
done
if [ "${EVM_PROFILE:-off}" = on ]; then HOST_OBJS+=("$BUILD/cycle_scopes.o"); fi
case "$(uname -s)" in
  Darwin) SHFLAG=(-dynamiclib -install_name "@rpath/libevmsail_guest.dylib"); EXT=dylib ;;
  *)      SHFLAG=(-shared); EXT=so ;;
esac
OUT="$BUILD/libevmsail_guest.$EXT"
# test_utils.o is the self-contained native I/O harness: it owns set_input,
# read_input, write_output, the output buffer, and run_once. The real guest's
# Spike I/O device adapter is never linked into host builds.
LINK_CMD=("$CC" "${CFLAGS[@]}" "${SHFLAG[@]}"
    "$BUILD/zkvm_block.o" "$BUILD/journal_glue.o" "$BUILD/hash_glue.o" "$BUILD/code_glue.o" "$BUILD/byte_slice_glue.o" "$BUILD/address_result_glue.o" "$BUILD/test_utils.o"
    "${HOST_OBJS[@]}" "${RUNTIME_OBJS[@]}"
    -L"$ACCEL_LIB" -lzkvm_accel_host -Wl,-rpath,"$ACCEL_LIB")
if [ "${EVM_BUILD_MODE:-optimized}" = standard ]; then
  LINK_CMD+=("${RUNTIME_LINK_FLAGS[@]}")
fi
if [ "${EVM_BUILD_MODE:-optimized}" = optimized ]; then
  case "$(uname -s)" in
    Darwin) LINK_CMD+=(-Wl,-dead_strip) ;;
    *)      LINK_CMD+=(-Wl,--gc-sections) ;;
  esac
fi
LINK_CMD+=(-o "$OUT")
echo "# link lib:"
printf '  %q' "${LINK_CMD[@]}"; echo
"${LINK_CMD[@]}"
echo "built $OUT (${EVM_BUILD_MODE:-optimized})"
