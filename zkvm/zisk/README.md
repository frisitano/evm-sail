# ZisK guest

The production guest links the generated C model directly with ZisK
**v1.4.0-alpha**, immutable revision
`64a1f1664184e76053686745d50c6b9aaa51b034`. `Cargo.toml` and `Cargo.lock`
identify the runtime; the version-pin crate is not compiled into the guest.
The previous indirect `ere-platform-zisk` dependency is unnecessary for this
C link and has been removed.

## Build

Use the repository's custom Sail compiler and RISC-V GCC with LP64 soft-float
support. Build the pinned upstream archive using its documented isolated build
procedure, with custom Rust **zisk-4.1.0** (Rust 1.99.0-dev, LLVM 23.1.1) and
its bundled LLVM tools. The runtime version and custom compiler release have
different identities. Do not use the old zisk-0.5.1 compiler or an ordinary
host Rust compiler for this archive.

```sh
make zisk-deps
make zisk-guest
ZISKEMU=/path/to/v1.4.0-alpha/ziskemu uv run --frozen python -m devtools.harness.cli \
  --zisk --quiet \
  zkvm/.fixtures/current-v2106-full/blockchain_tests/for_amsterdam/shanghai/eip3855_push0/push0/push0_contracts.json
```

`make zisk-deps` is the explicit dependency step. It fetches the immutable
revision into `zkvm/.deps/zisk/source`, verifies the checkout and upstream linker,
and invokes upstream `make libziskos FEATURES=` directly from the repository
Make recipe. The resulting
`zkvm/.deps/zisk/artifacts.json` records direct archive/linker paths, content
hashes, source identity, compiler version and build command. Artifacts remain in
the upstream checkout; no linker copy is introduced. Existing checkouts are
verified and never reset or switched. Set `ZISK_SOURCE=/path/to/zisk` during
preparation to use an existing pinned checkout instead of fetching one.

The upstream build expects the custom toolchain under the name `zisk`, or
`ZISK_TOOLCHAIN` selecting it. Follow upstream installation instructions for
your host. Toolchain installation is separate from dependency preparation.
Upstream owns fat-LTO finalization and allocator-symbol isolation; preparation
always invokes its build and publishes a manifest only after success. A failed
build leaves the previous manifest intact; changed artifact bytes invalidate it.
Keep source and target directories local to one build owner.

`make zisk-guest` only compiles the C guest and links the prepared inputs. It
checks their pinned identity and actual hashes before compilation; missing or
changed dependencies fail with a preparation error. It does not fetch sources,
invoke Cargo metadata, or build the runtime. `ZISK_DEPENDENCIES` selects a custom
manifest location for both steps. Provision the pinned fixture corpus with
`make eest-provision`.

`make zisk-smoke` provisions and authenticates that corpus, builds one guest
ELF and executes all six embedded PUSH0 cases, checking expected output bytes
and zero padding. Run `make zisk-deps` first. GitHub CI runs this smoke on
Ubuntu 24.04 x86_64, using checksum-pinned compiler/emulator downloads declared
in `config/toolchains.toml` and the normal repository Python/Sail setup actions.
The CI setup script only installs tools into runner temporary storage; Make
continues to own source preparation and the isolated upstream runtime build.

Cargo, custom Rust, Make and the guest C compiler come from the invoking
environment. Devenv supplies these tools and their environment; it invokes
the same `make zisk-deps` target. The repository does not download toolchains
or invoke a private provisioning build command. The Make recipe owns the
fetch/check, upstream build and artifact-recording sequence. Python helpers
only verify/fetch the pinned source and record/validate artifact identities.

## Linker provenance and memory ownership

Preparation verifies source revision, tracked-file integrity and linker bytes.
`build.sh` verifies and consumes the prepared paths, linking directly with upstream
[`ziskbuild/zisk_linker_script.ld`](https://github.com/0xPolygonHermez/zisk/blob/64a1f1664184e76053686745d50c6b9aaa51b034/ziskbuild/zisk_linker_script.ld).
Its SHA-256 is
`b6746bc7ddf7f48c56ba938350bf91a4ebbf68521d4400e7486c08bb0008670c`.
It remains in the upstream dependency under its upstream license; there is no
independently maintained local script or download of `latest`.

ROM is `[0x90000000,0x98000000)`. RAM is `[0xa0000000,0xc0000000)`:
4 MiB initial stack, 32 KiB general registers, 32 KiB float registers,
128 KiB output reservation, then initialized data and BSS. The remaining
model heap ends at `0xbffc0000`, below ZisK library and float scratch reservations.
`platform.c` returns `_heap_start`/`_heap_end` to the existing C allocator,
which aligns allocations to 16 bytes. The optimized workspace reserves its
fixed 256 MiB in 16-byte-aligned BSS and retains it for the execution lifetime,
preserving the allocation-free backend contract. The C heap starts after this
reservation. Exhaustion aborts the guest. The isolated runtime owns a separate
8 MiB BSS scratch heap and resets it at each runtime ABI call; it must never
provide persistent model storage.

The runtime's `_start` initializes `gp` and `sp`, runs constructors, calls
`main`, runs destructors and exits. `main.c` retains explicit
`zkvm_init`/`zkvm_start`/`zkvm_deinit` ordering. The emulator loads initialized
segments and zero-filled BSS. Upstream `KEEP` preserves init/fini arrays under
`--gc-sections`. Small-data emission stays disabled for C: `.srodata.*` is
not collected by the upstream script. The unconditional `_heap_start` and
`_heap_end` symbols remain discoverable in the final ELF.

Provisioning supports Apple Silicon macOS and x86_64 Linux upstream release
assets. Local CPU interpreter smoke checks do not establish Linux execution,
prover correctness, or end-to-end proof generation. The fixture harness requires
the matching emulator version and compares expected output bytes, including
zero padding; a successful compile alone is not a runtime check.
