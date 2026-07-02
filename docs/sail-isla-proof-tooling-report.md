# Sail and Isla Tooling Report for RISC-V Guest Compliance

Date: 2026-07-02

This report evaluates the local Sail and Isla checkouts under
`/Users/f/dev/ethereum/` for the goal of proving that a pinned RISC-V guest ELF
is compliant with `evm-sail`. The intended direction is:

```text
evm-sail is the source-of-truth EVM/stateless-validation specification.
The RISC-V guest ELF is the target implementation.
The proof obligation is refinement of the ELF behavior to the evm-sail spec.
```

This is not about compiling `evm-sail` to a zkVM target. The zkVM target is
irrelevant to the semantic reference here.

## Local Tool Snapshots

Local repositories inspected:

| Repository | Path | Branch | Commit |
| --- | --- | --- | --- |
| Sail | `/Users/f/dev/ethereum/sail` | `sail2` | `7ed92aaec7f9c59718aeef6d9385f851c2fc240e` |
| Isla | `/Users/f/dev/ethereum/isla` | `master` | `7f6882b3f468fe34df38ae3ffc0aede5baa0e7d6` |
| evm-sail worktree | `/Users/f/dev/ethereum/evm-sail/.worktrees/refactor-evm-sail-formal-verification-alignment` | `worker/refactor-evm-sail-formal-verification-alignment` | active worktree, dirty |

The local `sail` checkout is the Sail compiler/tooling repository. It is not
the Sail RISC-V ISA model. The Sail README points to `riscv/sail-riscv` as the
RISC-V model. The local `isla` checkout includes RISC-V configs and the
Sail-to-Isla bridge, but we still need either a Sail RISC-V model checkout or a
known-good generated `riscv64.ir`/`riscv32.ir` artifact.

## What Sail Gives Us

Sail is the right language-level anchor for this project because `evm-sail` is
already written in Sail and Sail has a path to executable testing, symbolic
analysis, and theorem-prover exports.

Useful Sail capabilities:

| Capability | Local evidence | Use for this proof |
| --- | --- | --- |
| Type checking and dependent bitvector sizes | Sail README describes lightweight dependent typing checked with Z3. | Keep the EVM spec and proof adapters honest about byte widths, word widths, fork flags, and bounded vector lengths. |
| C/OCaml executable generation | Sail README describes C/OCaml emulator generation. | Maintain the current fast executable oracle for fixture testing while adding proof adapters. |
| SMT backend | `src/sail_smt_backend`, `$property`, `$counterexample`, `-smt_auto`, Z3/cvc support. | Prove small pure properties directly from Sail, especially parser, arithmetic, gas, and bounded data-structure lemmas. |
| Isla IR generation through plugin support | Isla's `isla-sail` plugin consumes Sail files and emits `.ir`. | Feed evm-sail proof slices into Isla for symbolic execution. |
| Coq/Rocq, Isabelle, HOL4 backends | Sail README and backend directories. | Long-term proof checker path for contracts that should not remain only SMT queries. |
| Coq Isla trace translation | `src/sail_coq_backend` has `-coq_isla` support. | Important bridge if we use Islaris/Iris-style proofs and want Isla symbolic traces reflected into Coq/Rocq. |

The most important design implication is that we should add proof-oriented
Sail entrypoints rather than aim Isla directly at the whole current executable
runner. The current `evm-sail` runner is intentionally practical: it uses C FFI
maps, crypto accelerators, host I/O, and witness-backed trie databases. That is
good for execution and fixtures, but too impure and too large for first-pass
symbolic equivalence.

Recommended Sail proof adapter shape:

```text
proof/sail/
  spec_adapter.sail       explicit pure/bounded entrypoints over bytes/records
  evm_step_contracts.sail opcode- or block-local contracts
  trie_contracts.sail     bounded MPT/state-root contracts
  io_contracts.sail       input/output schema contracts
  crypto_axioms.sail      abstract keccak/sha/precompile contracts
```

The adapter should expose relations like:

```text
spec_decode(input_bytes) -> option DecodedInput
spec_execute(fork, decoded_input, witness) -> SpecResult
spec_public_output(result) -> bytes
```

For proof work, crypto should initially be abstracted as deterministic
uninterpreted functions over byte strings. The early theorem should prove that
the guest passes the same bytes to the same abstract crypto functions, not that
Keccak or SHA-256 are internally correct.

## What Isla Gives Us

Isla is a symbolic execution engine for Sail-generated IR. It is best viewed as
a symbolic executor and trace/summarization engine, not as the final proof
checker by itself.

Useful Isla capabilities:

| Tool/path | What it does | Use for this proof |
| --- | --- | --- |
| `isla-sail` | Sail plugin that compiles Sail specs to `.ir`. Requires latest Sail `sail2` rather than an opam release. | Build Isla IR for proof-specific evm-sail adapters and for Sail RISC-V. |
| `isla-property` | Checks Sail property functions against a generated `.ir` file. | Discharge small boolean contracts and find counterexamples. |
| `isla-execute-function` | Symbolically executes a named function with concrete or symbolic arguments. | Generate summaries of evm-sail adapter functions and, where applicable, ISA functions. |
| `isla-footprint` | Generates simplified instruction footprints from a Sail ISA model. | Get RISC-V instruction-level read/write/branch/memory summaries for guest slices. |
| `--abstract target:property` | Replaces functions with property-constrained abstractions. | Abstract crypto, precompiles, host calls, and large library functions while preserving contracts. |
| Function linearization | Rewrites some symbolic control flow into `ite` expressions. | Helpful for pure helper functions; not safe as a blanket transformation over stateful EVM execution. |
| `configs/riscv64.toml` and `configs/riscv32.toml` | RISC-V configs for Sail RISC-V execution. | Starting point for the guest ISA model, memory map, and toolchain commands. |

The local Isla repository has a useful but important limitation: the `isla-elf`
helper currently has concrete architecture support for AArch64. The generic ELF
parsing layer exists, but `isla-footprint --elf` is wired through
`AArch64` in `src/footprint.rs`. For RISC-V ELF work we should plan on one of
two paths:

1. Use external symbol and disassembly tools (`llvm-nm`, `llvm-objdump`, DWARF
   readers) to extract RISC-V function ranges and instruction bytes, then feed
   instructions/opcodes into Isla's RISC-V ISA machinery.
2. Extend `isla-elf` with a RISC-V `Architecture` implementation and teach the
   footprint path to use it.

The first path is faster for early spikes. The second path is cleaner for a
repeatable proof pipeline.

## Immediate Fit Against evm-sail

Good fits for Sail/Isla immediately:

- Pure bitvector and word helpers.
- RLP and SSZ parser lemmas under explicit bounds.
- Gas formulas and fork condition predicates.
- Individual opcode contracts with explicit pre-state and post-state records.
- Memory, stack, and storage helper contracts after they are exposed as explicit
  values rather than hidden host state.
- MPT/state-root lemmas over bounded update lists and abstract hashes.
- Public I/O contracts: schema prefix handling, input buffer layout, output
  bytes, digest boundary.

Bad first targets:

- Whole `evm-sail` runner symbolic execution.
- Whole Reth guest ELF symbolic execution in one query.
- Inlining Keccak/SHA/precompile implementations into the proof.
- Directly symbolically executing unbounded trie/database traversal without
  path and size bounds.
- Treating EEST/EELS fixtures as the proof oracle. They are validation
  evidence, not the spec.

The current evm-sail state-root refactor is helpful for proof work because it
makes the model closer to a textbook authenticated trie:

- account keys are secure keys, `keccak256(address)`;
- storage keys are secure keys, `keccak256(slot)`;
- node database maps node hash to trie node;
- post-state roots recurse over witness nodes and ordered updates along touched
  paths;
- deletes canonicalize the MPT shape.

That shape gives us a natural contract boundary for bounded MPT proofs:

```text
Given a witness node database, a pre-root, and an ordered update set,
state_root_from(pre_root, updates) returns the Yellow-Paper MPT root
for the updated secure-key account/storage map.
```

The proof adapter should make that relation explicit and avoid exposing the
native C map implementation.

## RISC-V Guest ELF Strategy

The target should be a symbol-rich debug build of the Reth RISC-V guest from
`/Users/f/dev/ethereum/ere-guests`, pinned to a compatible dependency graph and
guest backend. The deployed stripped ELF can be related to the debug ELF by
hashing loaded segments.

Required pinned metadata:

- `evm-sail` commit and fork semantics.
- EEST/EELS fixture generation commits used as regression evidence.
- `ere-guests` commit and lockfile.
- Reth/revm/stateless dependency commits and versions.
- RISC-V target width and extensions, for example RV32IMA/RV32IMAC or
  RV64IMA/RV64IMAC.
- Compiler image/toolchain, optimization flags, panic strategy, linker script,
  memory map, and ABI.
- Debug ELF hash, stripped ELF hash, and per-loadable-segment hashes.

The symbolic path should be function- or slice-oriented:

1. Compile the guest with symbols and DWARF.
2. Extract symbol ranges and call graph.
3. Start with shallow slices:
   - schema prefix parser;
   - public output digest wrapper;
   - fixed-size byte-copy and endian helpers;
   - SSZ list/vector length checks;
   - trie key hashing boundary.
4. Generate RISC-V instruction summaries with Isla against Sail RISC-V.
5. Compare those summaries against Sail spec-adapter summaries.
6. Compose proven summaries into larger function contracts.

The guest's interaction with zkVM I/O should be modeled as an ABI contract:

```text
input buffer pointer + input length
output buffer pointer + output length
crypto/precompile accelerator call boundaries
return/status convention
```

For the current Reth guest shape, the public boundary may be a digest of raw
SSZ output rather than the raw SSZ bytes themselves. The proof must therefore
either compare:

```text
guest_public_output == sha256(evm_sail_output_bytes)
```

or use a verification-oriented guest variant that exposes raw output bytes.

## Recommended Architecture

Use Sail and Isla in a layered refinement pipeline:

```text
             evm-sail source specification
                         |
          proof-oriented Sail adapter functions
                         |
              isla-sail generated Sail IR
                         |
          Isla symbolic summaries / properties
                         |
             SMT obligations and traces
                         |
        Coq/Rocq + Iris/Islaris proof replay

RISC-V guest ELF + symbols + DWARF
                         |
       objdump/nm or RISC-V support in isla-elf
                         |
       Sail RISC-V IR + Isla instruction summaries
                         |
          function/slice postconditions
                         |
        same SMT obligations / Coq proof replay
```

The first milestone should use SMT for bounded counterexample-finding and
contract debugging. The security-grade result should not stop there. It should
move stable contracts into a proof assistant path, most likely Coq/Rocq plus
Iris/Islaris, because Isla alone is an executor and solver frontend rather than
a small trusted proof kernel.

## Tradeoffs

| Option | Strengths | Weaknesses | Recommendation |
| --- | --- | --- | --- |
| Sail SMT backend only | Directly works on Sail properties; simple for pure bounded functions. | Not binary-aware; limited for large stateful code; proof certificates are not the main workflow. | Use for pure evm-sail lemmas and quick counterexamples. |
| Isla over evm-sail adapters | Uses Sail semantics directly; good traces and summaries; supports abstraction. | Needs careful pure/bounded entrypoints; path explosion on full EVM. | Primary near-term spec exploration path. |
| Isla over Sail RISC-V instruction model | Authoritative ISA semantics; good for post-link reasoning. | Needs Sail RISC-V IR; direct local RISC-V ELF support is incomplete. | Primary near-term binary-semantics path, with an extraction bridge. |
| Extend `isla-elf` for RISC-V | Clean repeatable ELF-to-Isla workflow. | Requires engineering for RISC-V opcodes, relocations, compressed instructions, and symbol handling. | Do after the first symbol/disassembly-driven spike succeeds. |
| Islaris/Iris/Coq | Strongest story for sequential binary proofs against Sail semantics. | Heavier proof engineering; requires proof assistant setup and expertise. | Best medium-term proof-checking target. |
| Sail Coq/Rocq backend plus `-coq_isla` | Bridges generated Sail definitions and Isla trace values. | Generated code can be large; needs adapter lemmas and careful abstraction. | Use for stable contracts after Isla/SMT spike validates shape. |
| `isla-axiomatic` | Strong for relaxed-memory litmus/concurrency. | The guest is expected to be sequential; relaxed memory is not central. | Not a core path unless guest concurrency/atomics become relevant. |
| HolBA | Mature binary-analysis direction in HOL ecosystems. | Less directly aligned with Sail/Isla RISC-V semantics and evm-sail source. | Keep as fallback/comparison, not primary. |
| Monolithic whole-ELF SMT | Conceptually simple statement. | Path explosion; hard to debug; likely infeasible early. | Avoid. Build modular contracts first. |

## Concrete Near-Term Spikes

### 1. Build compatibility gate

Goal: prove the local Sail and Isla clones can build together.

Commands to validate, after installing opam/build dependencies:

```sh
rtk make -C /Users/f/dev/ethereum/sail install
rtk make -C /Users/f/dev/ethereum/isla/isla-sail
rtk cargo build --release --manifest-path /Users/f/dev/ethereum/isla/Cargo.toml
```

Then run Isla's property tests:

```sh
rtk ruby /Users/f/dev/ethereum/isla/test/run_tests.rb
```

Expected result: local `isla-sail/plugin.cmxs`, Isla binaries under
`target/release/`, and passing property tests.

### 2. Generate an evm-sail adapter IR

Goal: compile a small proof-only Sail adapter to Isla IR.

Start with a pure bounded function such as a schema prefix predicate:

```text
schema_prefix_ok(input0, input1, len) =
  len >= 2 && input0 == 0x01 && input1 == 0x00
```

Then compile and check a property:

```sh
rtk sail -plugin /Users/f/dev/ethereum/isla/isla-sail/plugin.cmxs \
  -isla proof/sail/schema_prefix.sail \
  -o build/proof/schema_prefix

rtk /Users/f/dev/ethereum/isla/target/release/isla-property \
  -A build/proof/schema_prefix.ir \
  -p prop \
  -C /Users/f/dev/ethereum/isla/configs/plain.toml
```

This proves the plumbing before we involve EVM state.

### 3. Get Sail RISC-V IR

Goal: obtain the RISC-V ISA IR consumed by Isla.

Options:

1. Clone/build `riscv/sail-riscv` with the local `sail2` toolchain and
   `isla-sail`.
2. Use a pinned `riscv64.ir` or `riscv32.ir` from `rems-project/isla-snapshots`
   for early experiments, then replace it with a locally generated artifact.

For the final proof path, locally generated IR from a pinned Sail RISC-V commit
is better than an opaque snapshot.

### 4. Symbol-rich RISC-V ELF slice

Goal: prove we can extract and symbolically summarize a tiny guest function.

Use the pattern already validated in `proof-spikes/riscv-symbol-slice/`:

- compile a `no_std` RISC-V ELF with symbols and DWARF;
- record debug and stripped hashes;
- prove loaded segment bytes are identical after stripping;
- extract symbol ranges with `llvm-nm`/`llvm-objdump`;
- feed the relevant instructions into Isla using the RISC-V config and IR.

Repeat this against the selected Reth ZisK backend. The companion
`ere-guests` checkout now has `scripts/build-reth-zisk-proof-candidate.sh`,
which builds `bin/stateless-validator-reth/zisk` with a symbol-preserving
`proof` Cargo profile for target `riscv64ima-zisk-zkvm-elf` and emits
provenance, hashes, symbols, and disassembly under
`proof-artifacts/reth-zisk/`. It also has
`scripts/extract-zisk-elf-for-islaris.sh`, which converts the full executable
ELF into an annotated dump under `proof-artifacts/reth-zisk/islaris/` by
delegating to Isla's `isla-riscv-dump` binary. Its `--symbol` and
`--max-instructions` flags are still useful for diagnostic slices, but the proof
path defaults to the full guest.

The sibling `islaris` checkout now has a dedicated
`riscv64_zisk_isla_coq.toml` config. Generated ZisK dumps select it with
`//@isla-config: riscv64_zisk_isla_coq.toml`, keeping the Islaris run on RV64IMA
with compressed instructions disabled.

### 5. First relational proof obligation

Goal: compare a Sail spec slice and a RISC-V implementation slice.

Use the shape:

```text
assume shared ABI preconditions
assume same input bytes
assume same abstract crypto function results
assert guest_postcondition != evm_sail_postcondition
check-sat
```

Expected result for a valid slice:

```text
unsat
```

This is initially an SMT workflow for counterexample-finding. Once stable, move
the relation into Coq/Rocq/Islaris.

### 6. Abstract crypto and precompiles

Goal: prevent crypto from dominating early symbolic execution.

Use Isla function abstraction where possible:

```text
--abstract keccak256:keccak_contract
--abstract sha256:sha256_contract
--abstract precompile_call:precompile_contract
```

The contracts should state determinism and byte-level input/output equality
between guest and spec boundaries. Internal correctness of Keccak/SHA can be a
separate proof or a trusted primitive depending on the zkVM/host model.

### 7. Extend `isla-elf`

Goal: remove hand extraction and make the RISC-V slice reproducible from the
linked ELF.

The initial extension is now in progress:

```text
isla-elf/src/arch/riscv64.rs
isla-elf/src/arch.rs
src/footprint.rs architecture selection
src/riscv-dump.rs
```

Current minimum scope:

- fixed-width RV64 instruction decoding for the ZisK RV64IMA target;
- linked-ELF symbol range extraction using section virtual addresses;
- architecture selection in `isla-footprint`;
- an `isla-riscv-dump` frontend that emits Islaris-compatible `.dump` files.

Still missing before this is a final proof-quality extractor:

- explicit ELF machine/ABI validation for the selected RISC-V target;
- relocations needed by future guest builds, if any;
- compressed-instruction support if a future target enables RVC.

## Proposed Repository Layout

Add proof artifacts under `proof/` rather than mixing them into the executable
zkVM harness:

```text
proof/
  README.md
  toolchain.toml
  sail/
    spec_adapter.sail
    schema_prefix.sail
    trie_contracts.sail
    crypto_axioms.sail
  riscv/
    extract_symbols.rs
    loaded_segments.rs
    README.md
  isla/
    README.md
    generated/
  smt/
    README.md
  coq/
    README.md
```

Suggested `toolchain.toml` fields:

```toml
[evm_sail]
commit = "..."

[sail]
commit = "7ed92aaec7f9c59718aeef6d9385f851c2fc240e"

[isla]
commit = "7f6882b3f468fe34df38ae3ffc0aede5baa0e7d6"

[sail_riscv]
commit = "..."
generated_ir_sha256 = "..."

[guest]
repo = "/Users/f/dev/ethereum/ere-guests"
commit = "..."
backend = "..."
target = "..."
debug_elf_sha256 = "..."
stripped_elf_sha256 = "..."
loaded_segments_sha256 = "..."
```

Generated `.ir`, traces, and solver logs should probably be ignored by default
unless they are small fixtures used by tests.

## Recommended Plan

1. Use Sail as the authoritative spec language and as the source of pure,
   bounded proof adapters.
2. Use `isla-sail` to compile those adapters to Isla IR.
3. Use Isla to produce symbolic summaries and counterexamples for the adapters.
4. Obtain Sail RISC-V IR and use Isla to summarize selected RISC-V guest slices.
5. Compare Sail-spec summaries and RISC-V summaries with bounded SMT queries.
6. Move stable slice contracts into Coq/Rocq plus Iris/Islaris for a
   proof-checked path.
7. Only then scale from helper slices to opcode groups, transaction execution,
   trie/state-root handling, and finally the full stateless validator boundary.

The key engineering choice is modularity. A monolithic proof that the whole
Reth guest ELF equals the whole evm-sail runner is not a good first target.
The robust path is a library of small contracts:

```text
parser contracts
+ byte/layout contracts
+ crypto boundary contracts
+ opcode/state transition contracts
+ trie/state-root contracts
+ public-output contract
= full guest compliance theorem
```

## Bottom Line

Sail and Isla are a strong fit for this ambition, but only if we use them in the
way their architecture encourages:

- Sail defines the source-of-truth spec and proof-oriented contracts.
- Isla symbolically executes Sail-generated IR and RISC-V ISA semantics.
- SMT gives fast bounded checks and counterexamples.
- Coq/Rocq/Iris/Islaris should carry the final high-assurance proof story.
- RISC-V ELF ingestion is the main local tooling gap; start with symbol plus
  disassembly extraction, then extend `isla-elf` for RISC-V if the spike pays
  off.

The next concrete milestone should be small and falsifiable: build local
Sail+Isla, generate IR for one evm-sail adapter, get a Sail RISC-V IR, and
prove a tiny RISC-V ELF predicate refines the matching Sail predicate.
