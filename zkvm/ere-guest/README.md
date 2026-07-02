# evmsail-ere-guest — Sail EVM stateless validator as an ere guest

This packages the same Sail-EVM stateless block validator that runs on spike
(`zkvm/zkvm_block.sail` → C → `zkvm/build/zkvm_guest.elf`) as a guest for
[ere](https://github.com/eth-act/ere), so it can be compiled, executed, proven,
and benchmarked alongside the reth/ethrex guests under the same zkVMs.

## Status — STARTING ARTIFACT, NOT YET BUILT

Verified in this repo:
- **The I/O ABI already matches ere.** ere's `Platform::read_input`/`write_output`
  default impls call the eth-act `zkvm-standards` C symbols `read_input`/
  `write_output` — exactly the interface this model already uses. No rewrite.
- **`-DERE_GUEST` hook is in place and spike-verified.** `runtime/zkvm_input.c`
  takes its witness from `evmsail_set_input()` (the host `read_input()` buffer)
  instead of the baked vector, and buffers the SSZ result for a single
  `write_output()` (ere's commit-once contract). The spike build (no `ERE_GUEST`)
  is byte-identical and still validates.
- **`ere_bridge.c`** is the guest entry: `read_input` → `evmsail_set_input` →
  `model_init` → `zkvm_run` → `evmsail_flush_output` → `model_fini`.

NOT done here (requires external toolchains not installed in this environment):
- Building/proving. Needs the ere host crates + an RV64 zkVM SDK.
- `Cargo.toml` / `build.rs` / `src/main.rs` are TEMPLATES (SDK-specific entry
  macro, target sysroot, allocator) — they will not compile as-is.

## Hard constraint: RV64 only

The model is RV64 (256-bit words over 64-bit limbs; sailfix/mini-gmp). ere's
RV32 backends (SP1, RISC0, Airbender) cannot run it without a 32-bit port of the
bigint runtime.

For the current RISC-V proof effort, do not use this evm-sail guest as the
implementation target. Treat evm-sail as the source-of-truth spec and compare it
against the Reth ZisK guest from `ere-guests`, built for
`riscv64ima-zisk-zkvm-elf` with its proof profile. This template can still be
revived later if we want evm-sail itself to run inside an ere-backed RV64 zkVM.

## Islaris bridge

The current binary-proof bridge starts in the sibling `ere-guests` and
`islaris` checkouts:

```sh
cd /Users/f/dev/ethereum/ere-guests
scripts/build-reth-zisk-proof-candidate.sh
scripts/extract-zisk-elf-for-islaris.sh

cd /Users/f/dev/ethereum/islaris
make update_etc
PATH=$PWD/bin:$PATH dune exec -- islaris \
  /Users/f/dev/ethereum/ere-guests/proof-artifacts/reth-zisk/islaris/elf.dump \
  -j 4 \
  -o instructions/reth_zisk_full \
  --coqdir=isla.instructions.reth_zisk_full \
  --arch=riscv64
```

The extractor writes a full annotated executable-ELF dump under
`proof-artifacts/reth-zisk/islaris/` by calling Isla's `isla-riscv-dump`
binary. The dump bytes therefore come from the linked ELF and its symbol table,
not from shell parsing of disassembler text. With no `--symbol` and no
`--max-instructions`, the default path covers the whole guest. The bounded
`--symbol` / `--max-instructions` options remain useful for diagnostics and
small proof spikes, but they are not the production equivalence target.

Each dump selects Islaris' `riscv64_zisk_isla_coq.toml` config, which matches
the ZisK Rust target `riscv64ima-zisk-zkvm-elf` by disabling compressed RISC-V
instructions. The current full-guest path preserves the native `ere-guests`
memory-alignment mode and records those settings in
`proof-artifacts/reth-zisk/islaris/provenance.txt`.

From the `evm-sail` worktree, the repeatable check is:

```sh
rtk make -C proof check-ere-full-dump
```

## Also note

- The guest is still the **single-fixture demonstration** (`zkvm_block.sail`
  decodes exactly one legacy tx and seeds two accounts). Benchmarking real
  blocks needs the guest generalized to arbitrary tx/account counts first.
- Crypto: wire the zkVM's native keccak/sha256/secp256k1 **precompiles** behind
  the `zkvm-standards` header. The portable-C reference `zkvm_accelerators.c` has
  been removed; the single crypto source is the Rust `accel-host`, served as proven
  precompiles (see the spike `--extlib` accel device for the host-side model).

## Build sketch (once the toolchain is present)

1. `sail -c zkvm/zkvm_block.sail -o generated/zkvm_block` (the model core C).
2. Fill the SDK entry macro in `src/main.rs` and the target in `build.rs`.
3. `ere` compiles the crate for the chosen RV64 zkVM and runs/proves it; the
   public output's byte 32 is `successful_validation`.
