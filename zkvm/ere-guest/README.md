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
- **`-DERE_GUEST` hook is in place and spike-verified.** `runtime/el_input.c`
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

## Hard constraint: RV64 only → OpenVM or ZisK

The model is RV64 (256-bit words over 64-bit limbs; sailfix/mini-gmp). ere's
RV32 backends (SP1, RISC0, Airbender) cannot run it without a 32-bit port of the
bigint runtime. Target the RV64 backends: **OpenVM** or **ZisK**
(`riscv64ima-unknown-none-elf`).

## Also note

- The guest is still the **single-fixture demonstration** (`zkvm_block.sail`
  decodes exactly one legacy tx and seeds two accounts). Benchmarking real
  blocks needs the guest generalized to arbitrary tx/account counts first.
- Crypto: link `zkvm_accelerators.c` (reference) or wire the zkVM's native
  keccak/sha256 **precompiles** behind the same `zkvm-standards` header — the
  latter is the real proving-cost win (see the spike `--extlib` accel device for
  the host-side model).

## Build sketch (once the toolchain is present)

1. `sail -c zkvm/zkvm_block.sail -o generated/zkvm_block` (the model core C).
2. Fill the SDK entry macro in `src/main.rs` and the target in `build.rs`.
3. `ere` compiles the crate for the chosen RV64 zkVM and runs/proves it; the
   public output's byte 32 is `successful_validation`.
