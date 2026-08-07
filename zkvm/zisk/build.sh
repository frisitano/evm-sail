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
    BUILD="$(cd "$BUILD" && pwd)"

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

build_guest_c() {
    mkdir -p "$BUILD"
    BUILD="$(cd "$BUILD" && pwd)"

    ZKVM_PLATFORM=zisk ZKVM_BUILD="$BUILD" \
        bash "$ROOT/zkvm/build.sh" zisk-lib

    # ziskos as a plain static library: entry point, io, DMA memory ops, and
    # the accelerator crypto, linkable without the Rust bin crate. Built once
    # from the pinned checkout and cached beside the other build artifacts.
    local ziskos_lib="$BUILD/libziskos.a"
    if [ ! -f "$ziskos_lib" ]; then
        local checkout
        checkout="$(cargo metadata --manifest-path "$HERE/Cargo.toml" --format-version 1 2>/dev/null \
            | python3 -c 'import json,sys; m=json.load(sys.stdin); print(next(p["manifest_path"] for p in m["packages"] if p["name"]=="ziskos"))' \
            | xargs dirname | xargs dirname | xargs dirname)"
        RUSTUP_TOOLCHAIN="$TOOLCHAIN" cargo build --release \
            --manifest-path "$checkout/ziskos-staticlib/Cargo.toml" \
            --target "$TARGET" \
            --target-dir "$BUILD/ziskos-staticlib"
        cp "$BUILD/ziskos-staticlib/$TARGET/release/libziskos_staticlib.a" "$ziskos_lib"
    fi

    local gcc="${GCC:-riscv64-unknown-elf-gcc}"
    "$gcc" -march=rv64ima -mabi=lp64 -mcmodel=medany -msmall-data-limit=0 \
        -O2 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -fno-pic -mno-relax \
        -Wall -Wextra -c "$HERE/main.c" -o "$BUILD/zisk_cmain.o"
    "$gcc" -march=rv64ima -mabi=lp64 -mcmodel=medany -mno-relax \
        -nostdlib -static -T "$HERE/link_main.ld" \
        -Wl,--no-relax -Wl,--gc-sections \
        "$BUILD/zisk_cmain.o" \
        "$BUILD/libevmsail_zisk.a" "$ziskos_lib" -lgcc \
        -o "$BUILD/stateless-validator-evm-sail-zisk.elf"
    echo "built $BUILD/stateless-validator-evm-sail-zisk.elf (C-direct link)"
}

case "$COMMAND" in
    guest) build_guest ;;
    guest-c) build_guest_c ;;
    clean) rm -rf "$BUILD" ;;
    *) echo "usage: $0 {guest|guest-c|clean}" >&2; exit 2 ;;
esac
