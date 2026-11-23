#!/usr/bin/env bash
# CI provisioning only. Source preparation and staticlib builds belong to Make.
set -euo pipefail

if [[ "$(uname -s)" != Linux || "$(uname -m)" != x86_64 ]]; then
    echo "ZisK CI provisioning requires Linux x86_64" >&2
    exit 1
fi

tools="$RUNNER_TEMP/zisk-tools"
downloads="$RUNNER_TEMP/zisk-downloads"
mkdir -p "$tools/guest" "$tools/runtime" "$downloads"

download() {
    local url="$1" expected="$2" archive="$3"
    if [[ ! -f "$archive" ]]; then
        curl --fail --location --retry 3 "$url" --output "$archive"
    fi
    # Verify restored cache entries too; never extract unchecked release bytes.
    printf '%s  %s\n' "$expected" "$archive" | sha256sum --check --strict
}

download "$ZISK_CI_RUST_URL" "$ZISK_CI_RUST_SHA256" "$downloads/rust.tar.gz"
download "$ZISK_CI_RUNTIME_URL" "$ZISK_CI_RUNTIME_SHA256" "$downloads/runtime.tar.gz"
tar -xzf "$downloads/rust.tar.gz" -C "$tools/guest"
tar -xzf "$downloads/runtime.tar.gz" -C "$tools/runtime"
rustup toolchain link zisk "$tools/guest"
rustup run zisk rustc -vV
cargo +zisk --version
"$tools/runtime/bin/ziskemu" --version

{
    echo "ZISK_TOOLCHAIN=zisk"
    echo "ZISKEMU=$tools/runtime/bin/ziskemu"
    echo "RUSTC=$tools/guest/bin/rustc"
    echo "RUSTDOC=$tools/guest/bin/rustdoc"
    echo "GCC=riscv64-unknown-elf-gcc"
    echo "AR=riscv64-unknown-elf-ar"
    echo "AR_FULL_LTO=riscv64-unknown-elf-gcc-ar"
} >> "$GITHUB_ENV"
