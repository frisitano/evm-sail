#!/usr/bin/env bash
# Sail linter for evm-sail: zero compiler warnings + source hygiene.
# Run via `make lint`. Honors $SAIL (defaults to `sail` on PATH).
set -euo pipefail
cd "$(dirname "$0")/.."
SAIL="${SAIL:-sail}"
fail=0

echo "== sail warnings (--all-warnings) =="
# The model and the zkVM guest are kept warning-clean; the type checker is the
# linter (unused bindings, shadowing, incomplete matches, etc.).
for root in evm/evm.sail zkvm/zkvm_block.sail; do
  out=$("$SAIL" --all-warnings "$root" 2>&1 || true)
  if printf '%s\n' "$out" | grep -qiE "warning|error"; then
    echo "  FAIL $root:"; printf '%s\n' "$out" | grep -iE "warning|error" | head -20
    fail=1
  else
    echo "  ok   $root"
  fi
done

echo "== source hygiene (*.sail) =="
files=$(find evm examples zkvm -name '*.sail')
TAB=$(printf '\t')
ws=$(grep -rlE ' +$' --include='*.sail' evm examples zkvm || true)
[ -n "$ws" ] && { echo "  FAIL trailing whitespace:"; printf '    %s\n' $ws; fail=1; }
tabs=$(grep -rl "$TAB" --include='*.sail' evm examples zkvm || true)
[ -n "$tabs" ] && { echo "  FAIL tabs (use spaces):"; printf '    %s\n' $tabs; fail=1; }
for f in $files; do
  [ -n "$(tail -c1 "$f")" ] && { echo "  FAIL no final newline: $f"; fail=1; }
done
[ "$fail" -eq 0 ] && echo "  ok   no trailing ws / tabs / missing newlines"

if [ "$fail" -eq 0 ]; then echo "lint: clean"; else echo "lint: FAILED"; exit 1; fi
