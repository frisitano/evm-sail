#!/usr/bin/env bash
# Build the Lean extraction behind the same test_utils.c ABI used by the
# native Sail-C fixture harness.
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
MODEL="$ROOT/extractions/lean/src"
FFI="$ROOT/extractions/lean/ffi"
BUILD="$HERE/.lake/build"
LIB="$BUILD/lib"
ACCEL="$ROOT/zkvm/accel-host"
ACCEL_LIB="$ACCEL/target/release"

if [ ! -f "$ACCEL_LIB/libzkvm_accel_host.dylib" ] && \
   [ ! -f "$ACCEL_LIB/libzkvm_accel_host.so" ]; then
  echo "# building accel-host cdylib (one-time)..."
  (cd "$ACCEL" && cargo build --release --target-dir target)
fi

cd "$HERE"
lake update evm
lake build EvmLeanRunner:static @evm/Evm:static @Sail/Sail:static

LEAN_PREFIX="$(lean --print-prefix)"
CC="${CC:-cc}"
mkdir -p "$BUILD" "$LIB"
"$CC" -O2 -fPIC -I"$LEAN_PREFIX/include" \
  -c "$FFI/accelerators.c" -o "$BUILD/lean_accelerators.o"
"$CC" -O2 -fPIC -I"$LEAN_PREFIX/include" \
  -c "$FFI/test_utils.c" -o "$BUILD/lean_test_utils.o"

RUNNER_LIB="$LIB/libevm_x2dlean_x2drunner_EvmLeanRunner.a"
MODEL_LIB="$MODEL/.lake/build/lib/libevm_Evm.a"
SAIL_LIB="$MODEL/.lake/packages/Sail/.lake/build/lib/liblean_x2dsail_Sail.a"

case "$(uname -s)" in
  Darwin)
    EXT=dylib
    ARCHIVES=(
      "-Wl,-force_load,$RUNNER_LIB"
      "-Wl,-force_load,$MODEL_LIB"
      "-Wl,-force_load,$SAIL_LIB"
    )
    ;;
  *)
    EXT=so
    ARCHIVES=(
      -Wl,--whole-archive
      "$RUNNER_LIB"
      "$MODEL_LIB"
      "$SAIL_LIB"
      -Wl,--no-whole-archive
    )
    ;;
esac

OUT="$LIB/libevmsail_lean_guest.$EXT"
lake env leanc -shared -o "$OUT" \
  "$BUILD/lean_test_utils.o" \
  "${ARCHIVES[@]}" \
  "$BUILD/lean_accelerators.o" \
  -L"$ACCEL_LIB" -lzkvm_accel_host -Wl,-rpath,"$ACCEL_LIB" \
  -Wl,-rpath,"$LEAN_PREFIX/lib/lean"
echo "built $OUT"
