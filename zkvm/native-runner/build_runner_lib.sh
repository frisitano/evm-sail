#!/usr/bin/env bash
# ===========================================================================
# Build libevmsail_runner.{dylib,so}: the EEST state-test runner
# (sail/evm.sail_project evm, EVM_ENTRY=runner) as a SHARED LIBRARY for the
# in-process ctypes harness in harness/run.py -- replacing the per-case
# subprocess exe (runner_ffi.c stdin/stdout).
#
# This mirrors the guest lib toolchain (build.sh + build_lib.sh) with the ONLY
# difference being EVM_ENTRY=runner. The runner shares sail/host/io.sail with the
# guest, so test_utils.c's sail256 I/O ABI (ssz_src_*, el_emit_out) fits both,
# and test_utils.c also provides the reset (clear_memory) + large-stack run_once
# + the post-run state snapshot (evmsail_dump_snapshot).
#
#   export PATH="$HOME/.opam/sail/bin:$PATH"
#   eval "$(opam env --root=/Users/f/.opam --switch=sail)"
#   ./build_runner_lib.sh
# ===========================================================================
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"      # repo root (evm-sail)
BUILD="$HERE/.build"
mkdir -p "$BUILD"

SAIL="${SAIL:-sail}"
CC="${CC:-cc}"
SAILBIN="$(command -v "$SAIL")"
SAIL_LIB="$(cd "$(dirname "$SAILBIN")/../share/sail/lib" && pwd)"
[ -f "$SAIL_LIB/sail.h" ] || { echo "error: sail.h not found under $SAIL_LIB" >&2; exit 1; }

SF="${SF_RUNTIME:-$ROOT/zkvm/runtime/sail256}"   # GMP-free fixed-width runtime
RT="$ROOT/zkvm/runtime"; ZKVM="$ROOT/zkvm"; FFI="$ROOT/ffi"
ACCEL="$ROOT/zkvm/accel-host"; ACCEL_LIB="$ACCEL/target/release"
CFLAGS=(-O2 -Wno-error=implicit-function-declaration)
if [ -n "${SANITIZE:-}" ]; then CFLAGS+=(-g -fsanitize=address,undefined -fno-omit-frame-pointer); fi

# 1. accel-host crypto cdylib (industry crypto behind the precompile adapters)
if [ ! -f "$ACCEL_LIB/libzkvm_accel_host.dylib" ] && [ ! -f "$ACCEL_LIB/libzkvm_accel_host.so" ]; then
  echo "# building accel-host cdylib (one-time)..."
  ( cd "$ACCEL" && cargo build --release --offline --target-dir target )
fi

# 2. sail256 GMP-free runtime objects (SF include FIRST: GMP-free sail.h)
SF_OBJS=()
for src in sail.c sail_native.c sail_failure.c; do
  o="$BUILD/sf_${src%.c}.o"
  "$CC" "${CFLAGS[@]}" -c -I"$SF" -I"$SAIL_LIB" "$SF/$src" -o "$o"
  SF_OBJS+=("$o")
done

# 3. generate the RUNNER model C (no main/rts; preserve the Sail `main` entry so
#    test_utils' run_once can call zmain, and compute_state_root, which no Sail
#    code in this entry calls -- the harness dump invokes it directly C-side
#    (test_utils.c 'G' result section) to read the post-state root).
( cd "$ROOT" && "$SAIL" -c -O --c-no-main --c-no-rts --c-preserve main \
    --c-preserve compute_state_root \
    --c-include zkvm_input.h \
    sail/evm.sail_project evm \
    --variable EVM_ENTRY=runner \
    -o "$BUILD/zkvm_runner" )

# 4. compile the generated model
"$CC" "${CFLAGS[@]}" -I"$SF" -I"$SAIL_LIB" -I"$ZKVM" -I"$RT" -I"$FFI" \
    -c "$BUILD/zkvm_runner.c" -o "$BUILD/zkvm_runner.o"

# 4b. journal glue: JEntry values cross the extern boundary as the GENERATED
#     struct zJEntry, so the glue compiles against this build's model header.
"$CC" "${CFLAGS[@]}" -I"$BUILD" -I"$SF" -I"$SAIL_LIB" -I"$ZKVM" -I"$RT" -I"$FFI" \
    -DEVMSAIL_MODEL_H='"zkvm_runner.h"' \
    -c "$FFI/journal_glue.c" -o "$BUILD/journal_glue.o"

# 5. the harness shim: I/O buffers + ssz_src + clear_memory + run_once + dump
"$CC" "${CFLAGS[@]}" -I"$SF" -I"$SAIL_LIB" -I"$ZKVM" -I"$RT" -I"$FFI" \
    -c "$HERE/test_utils.c" -o "$BUILD/test_utils.o"

# 6. C host backends + direct host crypto/precompile adapters
HOST_OBJS=()
for hc in memory transient_storage state_db stack code_db kernel_state trie_node_db host_crypto precompiles returndata; do
  o="$BUILD/$hc.o"
  "$CC" "${CFLAGS[@]}" -I"$SF" -I"$SAIL_LIB" -I"$FFI" -c "$FFI/$hc.c" -o "$o"
  HOST_OBJS+=("$o")
done

# 7. link the shared library (model + shim + host + runtime + accel)
case "$(uname -s)" in
  Darwin) SHFLAG=(-dynamiclib -install_name "@rpath/libevmsail_runner.dylib"); EXT=dylib ;;
  *)      SHFLAG=(-shared); EXT=so ;;
esac
OUT="$BUILD/libevmsail_runner.$EXT"
LINK_CMD=("$CC" "${CFLAGS[@]}" "${SHFLAG[@]}"
    "$BUILD/zkvm_runner.o" "$BUILD/journal_glue.o" "$BUILD/test_utils.o"
    "${HOST_OBJS[@]}" "${SF_OBJS[@]}"
    -L"$ACCEL_LIB" -lzkvm_accel_host -Wl,-rpath,"$ACCEL_LIB"
    -o "$OUT")
echo "# link runner lib:"
printf '  %q' "${LINK_CMD[@]}"; echo
"${LINK_CMD[@]}"
echo "built $OUT"
