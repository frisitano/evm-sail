#!/usr/bin/env bash
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
BUILD="${ZKVM_BUILD:-$ROOT/zkvm/build-zisk}"
COMMAND="${1:-guest}"
DEPENDENCIES="${ZISK_DEPENDENCIES:-$ROOT/zkvm/.deps/zisk/artifacts.json}"

# The ZisK guest is linked directly by the C toolchain: gcc links the C entry
# (main.c), the evmsail archive, and the upstream isolated ziskos static library
# through the release's own linker. Dependency preparation is a separate step;
# this build consumes only its verified artifact paths.
build_guest() {
    local ziskos_lib linker_script
    ziskos_lib="$(cd "$ROOT" && uv run --frozen python "$HERE/dependencies.py" path "$DEPENDENCIES" staticlib)"
    linker_script="$(cd "$ROOT" && uv run --frozen python "$HERE/dependencies.py" path "$DEPENDENCIES" linker)"
    mkdir -p "$BUILD"
    BUILD="$(cd "$BUILD" && pwd)"

    ZKVM_PLATFORM=zisk ZKVM_BUILD="$BUILD" \
        bash "$ROOT/zkvm/build.sh" zisk-lib

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
        -nostdlib -static -T "$linker_script" \
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
