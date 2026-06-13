#!/usr/bin/env bash
# Sail formatter wrapper around the official `sail --fmt`.
#   scripts/fmt-sail.sh            format every *.sail in place (make fmt)
#   scripts/fmt-sail.sh --check    verify formatting, exit 1 if any file differs
#
# NOTE: `sail --fmt` is opinionated (4-space indent, reflowed lines). This repo
# is currently hand-formatted (2-space, column-aligned tables); adopting the
# formatter repo-wide is a deliberate one-time reindent, so it is NOT part of
# `make all`. Run it only when you mean to switch the whole tree to Sail style.
set -euo pipefail
cd "$(dirname "$0")/.."
SAIL="${SAIL:-sail}"
files=$(find evm zkvm revm-eest -name '*.sail' | sort)

if [ "${1:-apply}" = "--check" ]; then
  rc=0; n=0; tmp=$(mktemp)
  for f in $files; do
    if ! "$SAIL" --fmt --fmt-emit stdout "$f" > "$tmp" 2>/dev/null; then
      echo "  WARN sail --fmt failed: $f"; rc=1; continue
    fi
    if ! diff -q "$f" "$tmp" >/dev/null 2>&1; then
      echo "  needs formatting: $f"; rc=1; n=$((n+1))
    fi
  done
  rm -f "$tmp"
  [ "$rc" -eq 0 ] && echo "fmt-check: clean" || echo "fmt-check: $n file(s) differ from sail --fmt"
  exit $rc
else
  n=0; for f in $files; do "$SAIL" --fmt --fmt-emit file "$f"; n=$((n+1)); done
  echo "formatted $n files with sail --fmt"
fi
