#!/usr/bin/env bash
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
BUILD="${ZKVM_BUILD:-$ROOT/zkvm/build-zisk}"
COMMAND="${1:-guest}"
TARGET="riscv64ima-zisk-zkvm-elf"
PACKAGE="stateless-validator-evm-sail-zisk"
TOOLCHAIN="${ZISK_TOOLCHAIN:-zisk}"

build_guest() {
    mkdir -p "$BUILD"

    ZKVM_PLATFORM=zisk ZKVM_BUILD="$BUILD" \
        bash "$ROOT/zkvm/build.sh" zisk-lib

    local sysroot rustc rustflags
    sysroot="$(RUSTUP_TOOLCHAIN="$TOOLCHAIN" rustc --print sysroot)"
    rustc="$sysroot/bin/rustc"
    rustflags="${RUSTFLAGS:-}"
    if [ -n "$rustflags" ]; then
        rustflags="$rustflags "
    fi
    # LLD's RISC-V relaxation rewrites some canonical .rodata/.bss inputs into
    # orphan .srodata/.sbss output sections. ZisK rejects the resulting
    # overlapping load ranges, so keep the final Rust link unrelaxed just as
    # the C compilation already is.
    rustflags="${rustflags}-C link-arg=--no-relax"

    EVMSAIL_ZISK_LIB_DIR="$BUILD" \
    EVMSAIL_ZISK_DEBUG="${EVM_DEBUG:-off}" \
    CARGO_TARGET_DIR="$BUILD/cargo-target" \
    RUSTC="$rustc" \
    RUSTFLAGS="$rustflags" \
        cargo build \
            --manifest-path "$HERE/Cargo.toml" \
            --target "$TARGET" \
            --release

    cp "$BUILD/cargo-target/$TARGET/release/$PACKAGE" \
        "$BUILD/stateless-validator-evm-sail-zisk.elf"
    echo "built $BUILD/stateless-validator-evm-sail-zisk.elf"
}

case "$COMMAND" in
    guest) build_guest ;;
    clean) rm -rf "$BUILD" ;;
    *) echo "usage: $0 {guest|clean}" >&2; exit 2 ;;
esac
