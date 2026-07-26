#!/usr/bin/env bash

set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/.." && pwd)"
LOCAL_FEATURE_SAIL="$ROOT/../sail/.worktrees/implement-per-newtype-c-representation-overrides-in-sail/sail"

if [ -n "${SAIL:-}" ]; then
    candidate="$SAIL"
elif [ -x "$LOCAL_FEATURE_SAIL" ]; then
    candidate="$LOCAL_FEATURE_SAIL"
elif command -v sail >/dev/null 2>&1; then
    candidate="$(command -v sail)"
else
    echo "error: custom Sail was not found; set SAIL to the repository compiler with standard proof backends and bound-driven C specialization" >&2
    exit 1
fi

if [[ "$candidate" != */* ]]; then
    if ! resolved="$(command -v "$candidate")"; then
        echo "error: SAIL=$candidate is not executable" >&2
        exit 1
    fi
    candidate="$resolved"
elif [ ! -x "$candidate" ]; then
    echo "error: SAIL=$candidate is not executable" >&2
    exit 1
fi

printf '%s\n' "$candidate"
