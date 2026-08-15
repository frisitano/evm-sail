#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 3 ]]; then
    echo "usage: $0 EVMSAIL_ELF RETH_ELF ETHREX_ELF" >&2
    exit 2
fi

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
guest_dir="$root/devtools/benchmarks/zisk-guests"
mkdir -p "$guest_dir"

stage_guest() {
    local source="$1"
    local destination="$2"
    if [[ ! -f "$destination" ]] || ! cmp -s "$source" "$destination"; then
        cp "$source" "$destination"
    fi
}

stage_guest "$1" "$guest_dir/stateless-validator-evm-sail-zisk.elf"
stage_guest "$2" "$guest_dir/stateless-validator-reth-zisk.elf"
stage_guest "$3" "$guest_dir/stateless-validator-ethrex-zisk.elf"

(
    cd "$guest_dir"
    sha256sum \
        stateless-validator-evm-sail-zisk.elf \
        stateless-validator-reth-zisk.elf \
        stateless-validator-ethrex-zisk.elf > SHA256SUMS
)

echo "staged benchmark guests in $guest_dir"
