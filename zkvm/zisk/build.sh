#!/usr/bin/env bash
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
BUILD="${ZKVM_BUILD:-$ROOT/zkvm/build-zisk}"
COMMAND="${1:-guest}"
TARGET="riscv64ima-zisk-zkvm-elf"
TOOLCHAIN="${ZISK_TOOLCHAIN:-zisk}"

# The ZisK guest is linked directly by the C toolchain: gcc links the C entry
# (main.c), the evmsail archive, and the prebuilt ziskos static library
# through link_main.ld. There is no Rust-driven final link. Cargo.toml and
# Cargo.lock remain solely as the pinned ziskos version source; the harness
# checks the ziskemu version against that lock.
build_guest() {
    mkdir -p "$BUILD"
    BUILD="$(cd "$BUILD" && pwd)"

    ZKVM_PLATFORM=zisk ZKVM_BUILD="$BUILD" \
        bash "$ROOT/zkvm/build.sh" zisk-lib

    # ziskos as a plain static library: entry point, io, DMA memory ops, and
    # the accelerator crypto, linkable without any Rust bin crate. Built once
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
    # EVM_LTO=full performs whole-program LTO at this link; the machine-code
    # _start in libziskos.a roots the symbol graph through main.
    local opt_flags=(-O3)
    local lto_flags=()
    if [ "${EVM_LTO:-full}" = full ]; then
        lto_flags=(-flto)
    fi
    local -a c_entry=()
    local -a unity_roots=()
    if [ "${EVM_UNITY:-off}" = off ]; then
        "$gcc" -march=rv64ima -mabi=lp64 -mcmodel=medany -msmall-data-limit=0 \
            "${opt_flags[@]}" -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -fno-pic -mno-relax \
            ${lto_flags[@]+"${lto_flags[@]}"} \
            -Wall -Wextra -c "$HERE/main.c" -o "$BUILD/zisk_cmain.o"
        c_entry=("$BUILD/zisk_cmain.o")
    else
        # main lives in the unity archive. Root it before scanning that archive;
        # libziskos introduces its reference only later in the link order.
        unity_roots=(-Wl,-u,main)
    fi
    "$gcc" -march=rv64ima -mabi=lp64 -mcmodel=medany -mno-relax \
        "${opt_flags[@]}" \
        -nostdlib -static -T "$HERE/link_main.ld" \
        ${lto_flags[@]+"${lto_flags[@]}"} \
        -Wl,--no-relax -Wl,--gc-sections \
        "${unity_roots[@]}" \
        "${c_entry[@]}" \
        "$BUILD/libevmsail_zisk.a" "$ziskos_lib" -lgcc \
        -o "$BUILD/stateless-validator-evm-sail-zisk.elf"
    echo "built $BUILD/stateless-validator-evm-sail-zisk.elf (C-direct link)"
}

case "$COMMAND" in
    guest) build_guest ;;
    clean) rm -rf "$BUILD" ;;
    *) echo "usage: $0 {guest|clean}" >&2; exit 2 ;;
esac
