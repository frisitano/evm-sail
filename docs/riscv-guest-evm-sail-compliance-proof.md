# RISC-V Guest Compliance With evm-sail

Date: 2026-07-02

Local snapshot used for this analysis:

- `evm-sail`: `d79f90231bf6713c0e5cffb0fdff06e39a41b16b`
- `ere-guests`: `afc3f4e0fdd8f23353469fdc088af51c3a765194`
- Relevant Reth guest entrypoint:
  `/Users/f/dev/ethereum/ere-guests/crates/stateless-validator-reth/src/guest.rs`
- Relevant evm-sail executable harness boundary, for testing and IO comparison:
  `zkvm/README.md`, `zkvm/zkvm_io.h`, `zkvm/runtime/zkvm_input.c`,
  `zkvm/ere-guest/README.md`

## Spike Results

These results were produced locally on 2026-07-02 and are intended to evaluate
direction, not to certify the real Reth guest yet.

### Compatible Version Gate

The proof target must pin four independent version axes:

- Source of truth: `evm-sail`
  `d79f90231bf6713c0e5cffb0fdff06e39a41b16b`.
- Fixture semantics evidence: execution-specs branch `projects/zkevm` at
  `02c6c2510916e470f2c1e5191589212ca75d4948`, generated with `t8n = 2.19.0`.
  The local aligned corpora are:
  - `zkvm/.fixtures/current-02c6-full/`
  - `zkvm/.fixtures/current-state-02c6-full/`
- Reth guest source: `ere-guests`
  `afc3f4e0fdd8f23353469fdc088af51c3a765194`.
- Reth guest dependency graph, from the `ere-guests` lockfile:
  - Reth `v2.3.0`:
    `9384bc53d8c0c77e59cac83fdaaf3b372c6d2216`
  - `revm = 40.0.3`, checksum
    `823da6e5509bb8e5dcd91295870e494917a030ad506fc83301f3f08ad8b15b17`
  - `stateless` / `tries` branch `feature/v2.3.0`:
    `81f662b467ec6c25c323b3429c5ab4305a3088ac`
  - `zkvm-standards`:
    `282cd356c3a0498416bb0619f9c8a347ce9933fb`
  - `ere v0.12.2`:
    `c31ee8bf5985bcdbee93217c868eb6d9c811aefb`

For the Reth guest, the compatible path is to compile from the local
`ere-guests` checkout through its `ere_dockerized` `compile_guest` flow. The
`artifact-registry.json` file only contains a prebuilt Zesu/ZisK artifact, not
a Reth artifact, so it should not be used as a shortcut for Reth compliance.
Any downloaded Reth ELF would need an external provenance record proving it was
built from the same `ere-guests` commit, lockfile, backend, compiler image, and
feature set.

The selected first Reth guest candidate is now the ZisK backend. `evm-sail`
remains the source-of-truth specification; its local zkVM compilation target is
not part of the semantic reference. The useful comparison target is the Reth
guest under `ere-guests/bin/stateless-validator-reth/zisk`, built from
`ere-guests afc3f4e0fdd8f23353469fdc088af51c3a765194` with the proof-oriented
profile added in that checkout. The backend target is
`riscv64ima-zisk-zkvm-elf`, which is preferable for the first Isla/Islaris slice
because it is RV64, avoids compressed instructions, and already has an ERE
platform ABI. SP1/Jolt-style RV64IMAC and other zkVM guests remain useful
fallbacks or comparison points, but they are not the primary proof candidate.

### Toolchain Inventory

Available locally:

- Rust `cargo 1.95.0`, `rustc 1.95.0`.
- RISC-V Rust targets:
  `riscv32imac-unknown-none-elf`, `riscv64imac-unknown-none-elf`.
- Sail `0.20.1`.
- Z3 `4.16.0`.
- Rust-bundled LLVM tools:
  `llvm-objdump`, `llvm-readobj`, `llvm-nm`, `llvm-objcopy`, `llvm-strip`.

Not available on the local PATH during this spike:

- ZisK Rust toolchain `zisk`;
- HolBA / HOL4 `Holmake`;
- Islaris / Isla;
- Rocq/Coq;
- cvc5;
- system `readelf` / `llvm-readelf`.

This means HolBA and Islaris remain research directions rather than completed
local proof runs. The local executable spikes below only evaluate ELF slicing
and SMT proof-obligation workflow.

### Symbol-Rich RISC-V ELF Spike

Added `proof-spikes/riscv-symbol-slice/`, a tiny `no_std` RISC-V guest that
compiles for `riscv64imac-unknown-none-elf` with debug info and stable symbols.
The important symbols are visible in the ELF:

```text
0000000080000000 T _start
000000008000002a T proof_digest_wrapper
0000000080000068 T proof_entrypoint
00000000800000ac T proof_schema_prefix_ok
00000000800010da R PROOF_INPUT
00000000800020e4 B PROOF_OUTPUT
```

The generated ELF contains DWARF sections and four loadable segments:

```text
PT_LOAD RX: .text at 0x80000000, 218 bytes
PT_LOAD R : .rodata at 0x800010da, 8 bytes
PT_LOAD RW: .bss at 0x800020e4, 4 bytes
```

The debug ELF and stripped ELF have different full-file hashes, as expected:

```text
debug ELF    81445ae65e60156408a45e5e93957b52ea472c62336cd2d858924ef175c6bf29
stripped ELF 193e59685b36f134e33d2911562a749f856b55b3a0f9c07623ca913a94b81574
```

However, dumping `.text` from both artifacts produced identical bytes:

```text
.text debug    691ca69c6772e25cfbd70ea8f0f663e0517b7759a058ad3139967723dddb1396
.text stripped 691ca69c6772e25cfbd70ea8f0f663e0517b7759a058ad3139967723dddb1396
```

`cmp` also confirmed the two `.text` dumps are byte-identical. This validates
the intended proof workflow at toy scale:

1. build a symbol-rich ELF to choose and debug proof slices;
2. prove over the actual loaded instruction/data bytes;
3. strip only non-loaded metadata for deployment;
4. record that the relevant loaded bytes are unchanged.

The same check should be repeated for the selected Reth guest, but over every
loadable segment, not only `.text`.

### SMT Micro-Spike

Added `proof-spikes/smt-slices/schema-prefix-refinement.smt2`, a small Z3
query for the predicate extracted from the toy RISC-V disassembly:

```text
spec_ok  = input != 0 && len >= 2 && byte0 == 0x01 && byte1 == 0x00
riscv_ok = if input == 0 || len < 2
           then false
           else ((byte0 xor 1) | byte1) == 0
```

The query asserts `spec_ok xor riscv_ok` and Z3 returns:

```text
unsat
```

This is useful evidence that the counterexample-query shape is practical for
small extracted predicates. It is not a binary proof: the SMT file is a
hand-modeled slice. The secure version still needs either a proof-producing
lifter/executor or proof obligations replayed in a theorem prover.

## Recommendation

Treat `evm-sail` as the source of truth. The RISC-V guest ELF is the target
implementation whose behavior must refine the evm-sail semantics. Reth,
execution-specs, EELS/EEST fixtures, and `ere-guests` are useful comparison and
test sources, but they are not the proof oracle for this effort.

Do not start with whole-ELF symbolic equivalence between the full Reth guest and
the full evm-sail guest. The desired theorem is not "two peers are equivalent";
it is "this pinned RISC-V ELF is compliant with the evm-sail spec." Whole-ELF
proof remains the end goal, but the first tractable path is a layered refinement
proof:

1. Define an authoritative byte-level relation from evm-sail:

   ```text
   EvmSailSpec(input_bytes) = output_ssz_bytes
   ```

   This relation should be induced by the evm-sail stateless validator
   semantics, including its SSZ decoding, witness/trie interpretation, EVM
   transition semantics, and public-output encoding.

2. Mechanize or freeze that evm-sail relation as the proof target:

   ```text
   evm-sail execution on input_bytes emits output_ssz_bytes
   ```

3. Prove that the selected RISC-V guest ELF refines that evm-sail relation at
   the zkVM public boundary:

   ```text
   RethGuestElf(input_bytes) emits sha256(output_ssz_bytes)
   ```

   The digest is not optional for the current `ere-guests` Reth entrypoint:
   `entrypoint<P>()` runs `run_stateless_guest`, SHA-256 hashes the serialized
   output, and writes the 32-byte digest because some zkVMs only support 32-byte
   public values. Compliance for the deployed ELF therefore needs to compare
   `sha256(evm_sail_output)` with the Reth public output, or we need a
   verification-oriented Reth guest variant that publishes raw SSZ bytes.

4. Use Sail's theorem-prover backends to expose enough of evm-sail as a formal
   specification, then use Islaris/Rocq/Iris as the primary proof track for
   machine-code contracts against that specification. Run a parallel HolBA spike
   only if the Islaris workflow is too manual for the selected binary slice. Do
   not bet the first milestone on Iris-Lean: I did not find a mature
   Iris-in-Lean stack comparable to Rocq Iris plus Islaris for post-link RISC-V
   machine-code proof.

## Local Boundary Facts

The two guests are close at the input schema but differ at the public output
boundary.

The `ere-guests` Reth validator:

- Uses `StatelessInput::from_schema_prefixed_ssz(input_bytes)`.
- On decode failure, returns `StatelessValidationResult::default().to_ssz()`.
- Computes `new_payload_request_root` by SSZ hash-tree-root.
- Calls Reth stateless validation through
  `stateless_validation_with_trie::<SparseState, _, _>(...)`.
- Serializes `StatelessValidationResult` as plain SSZ bytes.
- The guest `entrypoint` writes `sha256(output_ssz_bytes)`, not
  `output_ssz_bytes`.

The evm-sail executable validator:

- Reads the same schema-prefixed SSZ `SszStatelessInput` from the
  `read_input(const uint8_t **, size_t *)` interface.
- Executes against a witness-backed trie node database keyed by secure node
  hashes.
- Recomputes the post-state root and emits canonical
  `SszStatelessValidationResult` bytes via `write_output`.
- The current evm-sail executable harness buffers the raw SSZ result and
  flushes it through one `write_output` call.

Because evm-sail is the source of truth, these facts create the first proof
split:

- Raw semantic compliance: compare evm-sail output bytes with
  `run_stateless_guest::<P>(input_bytes)`.
- Deployed-ELF compliance: compare `sha256(evm-sail output bytes)` with the
  Reth ELF public output.

Both are useful, but only the second is the target security theorem for the
deployed digest-publishing ELF. The raw comparison is a debugging aid and an
intermediate contract, not a separate source of authority.

## What The Secure Theorem Should Say

For a pinned RISC-V ELF hash, pinned input ABI, pinned accelerator ABI, and
evm-sail semantics revision:

```text
For all input byte strings i in the admitted stateless-input domain,
if evm-sail execution on i terminates normally and emits SSZ bytes o,
then executing the Reth RISC-V guest ELF from the canonical zkVM initial
machine state with private input i terminates normally, writes exactly
sha256(o) as its public output, performs no disallowed host calls, and
does not rely on out-of-contract memory behavior.
```

The converse can be stated if needed:

```text
If the Reth ELF emits digest d for input i, then evm-sail emits SSZ bytes o
such that d = sha256(o).
```

The forward direction is the core compliance theorem. The converse is useful
when we want to rule out target behavior that is not explained by evm-sail, but
it should not turn Reth into a co-equal specification.

The proof should explicitly account for:

- malformed schema-prefixed SSZ input;
- fork configuration and payload-shape validation;
- failed stateless validation as a normal output with
  `successful_validation = false`;
- abnormal guest failure, trap, panic, abort, or illegal host call as proof
  rejection;
- crypto/precompile calls as either proved implementations or explicit
  accelerator contracts.

## Tooling Evaluation

| Tooling path | Theorem strength | Fit for this project | Main risks | Recommendation |
| --- | --- | --- | --- | --- |
| Islaris + Rocq/Iris + Sail RISC-V | Very strong: machine code against authoritative Sail ISA semantics, with separation-logic contracts. | Best conceptual fit. It is explicitly designed to verify machine code against Sail ISA models and has RISC-V support. | Workflow appears research-grade and manual. Current docs require instrumented `objdump` snippets and generated Coq traces, so scaling to a huge Rust ELF is not immediate. | Primary path for the first serious proof spike. Start with tiny guests and selected routines before full Reth. |
| HolBA/HOL4 | Strong: proof-producing RISC-V binary contracts through BIR symbolic execution, with backlifting support. | Good parallel feasibility check, especially for automated forward symbolic execution of RISC-V routines. | Introduces BIR and HOL4 into the trusted story rather than staying directly on Sail RISC-V. Backlifter is experimental. | Run a parallel spike on the same tiny guest or selected leaf routine. Keep as fallback or complementary evidence. |
| Bounded SMT refinement checking | Potentially high coverage as a counterexample finder and proof-obligation generator. | Useful for comparing bounded evm-sail slices against bounded RISC-V guest slices. | Not a final proof unless the extraction, symbolic execution, memory model, loop bounds, and solver result are connected to a theorem. IR-based binary lifting has a known semantic-equivalence problem. | Add as a parallel engineering lane, but do not make it the security theorem by itself. |
| Source-level Rust verification of Reth/revm modules | Good module reasoning, weaker deployed-binary guarantee. | Helpful for SSZ/trie/execution routines if we can isolate pure Rust components. | Leaves compiler, linker, platform, allocator, and inline assembly in the trusted computing base. | Useful as supporting work, not the final secure compliance proof. |
| Sail-to-Coq/Isabelle/HOL4 proof of evm-sail | Strong for the specification side. | Required to make evm-sail the formal source of truth rather than just an executable oracle. | Does not itself prove the Reth ELF. Full EVM/state-root proof is large. | Treat as the spec track. Build incrementally around the stateless validator relation and key helper lemmas. |
| Iris-Lean / Lean-first | Attractive long term if the project wants Lean. | Sail has a Lean backend in the repository, but I did not find a mature Islaris-equivalent machine-code workflow in Lean. | Likely infrastructure-building before proof work. | Defer unless the strategic goal is to build that ecosystem. |

## Debug-Symbol Build Strategy

Yes: compile the RISC-V ELF with symbols and debug information for the proof
workflow. The important constraint is that debug metadata is proof-engineering
metadata, not trusted semantics. The final theorem should quantify over the
actual post-link executable bytes in loadable segments.

Recommended artifact split:

- `reth-guest.proof.elf`: unstripped ELF with `.symtab`, DWARF, build id, and
  stable function labels.
- `reth-guest.deploy.elf`: deployment artifact, stripped only if the zkVM
  packaging requires it.
- `reth-guest.load-segments.sha256`: digest over loadable executable and data
  segments, used to show the proof ELF and deployed ELF execute the same bytes.
- `reth-guest.readelf.txt`: section headers, program headers, and symbol table.
- `reth-guest.objdump.txt`: disassembly with symbol and source-line context.
- `reth-guest.linker-map.txt`: linker map if the toolchain can emit one.

Useful compiler/linker knobs for proof builds:

- keep full debug info, for example Rust `-C debuginfo=2`;
- keep frame pointers, for example `-C force-frame-pointers=yes`, if the target
  supports it and the deployed/proved binary is built the same way;
- use `panic=abort` and a single, explicit abort path;
- avoid changing optimization level only for the proof unless the proof build is
  the binary being certified;
- avoid LTO for the first modular proof spike if function boundaries matter, but
  treat this as a different certified binary if deployment uses LTO;
- add narrow `#[inline(never)]` / `#[no_mangle]` proof-boundary shims around
  IO, SSZ, trie, digest, and validation entrypoints rather than trying to make
  every Rust internal symbol stable.

Debug info should be used to:

- choose address ranges for function-level contracts;
- build a call graph and proof-slice manifest;
- map failed proof obligations back to Rust source;
- identify stack-frame and local-variable layout during manual proof work.

Debug info should not be used to:

- define the semantics of an instruction;
- justify a memory access that is not valid in the machine state;
- assume a function boundary that the executable control flow does not respect;
- prove a different binary than the deployed one.

The safest workflow is to build one symbol-rich ELF, prove against its loadable
segments, then strip only non-loaded metadata for deployment and record that the
loadable segment digests are identical.

## Islaris Versus Direct Iris

Iris by itself is not a RISC-V binary verifier. It is the separation-logic
framework in Rocq. To use "just Iris" for this project, we would still need a
sound front end that:

- decodes the RISC-V ELF into instructions;
- gives those instructions formal semantics;
- symbolically executes them or provides weakest-precondition rules;
- connects those rules to the Sail RISC-V ISA model;
- emits proof obligations over the actual executable bytes.

That is effectively the infrastructure Islaris is trying to provide: Sail/Isla
symbolic execution of machine code plus Iris/Rocq contracts. The weakness is not
the proof story; it is engineering maturity and scalability. The Islaris README
still describes manual `objdump` instrumentation and generated per-address Coq
trace files, which is a warning sign for full-Reth-ELF scale.

So the decision should not be "Islaris or Iris." It should be:

1. Use Iris/Rocq as the proof logic if we choose the Rocq track.
2. Use Islaris as the first candidate binary front end because it is already
   connected to Sail-style ISA semantics.
3. Use the symbol-rich ELF to make Islaris slices smaller and more modular.
4. In parallel, test HolBA because its RISC-V workflow emphasizes automated
   proof-producing symbolic execution of binary contracts.
5. Avoid a bespoke SMT symbolic executor as the security proof unless we also
   prove that executor sound with respect to the selected RISC-V ISA semantics.

The pragmatic near-term position is: start with Islaris for tiny wrappers and
selected proof-critical slices, run HolBA on the same slices as an automation
comparison, and only build custom Iris/symbolic-execution infrastructure if both
existing tracks fail for reasons we can clearly name.

## HolBA Track

HolBA is worth treating as the strongest near-term alternative to Islaris, not
as generic SMT testing. It is a HOL4-based binary analysis framework with
RISC-V support. Its repository describes a proof-producing binary lifter,
proof-producing symbolic execution, weakest-precondition tooling, and RISC-V
examples for automated contract proofs. The 2025 RISC-V HolBA paper is directly
on point: it presents a workflow for trustworthy formal verification of RISC-V
binaries using forward symbolic execution and binary contracts.

The main attraction for this project is automation. Compared with the current
Islaris workflow, HolBA appears more oriented toward automated forward symbolic
execution of bounded binary contracts. That is exactly what we need for the
first modular slices:

- `entrypoint` wrapper: read input, call validator, hash output, write digest;
- `read_input`/`write_output` ABI wrappers;
- fixed-size SSZ prefix checks;
- digest wrapper code;
- small trie/path helper routines;
- proof-boundary shims compiled with stable symbols.

The main cost is the semantic bridge. HolBA lifts binaries into BIR, an
intermediate language, and proves contracts in HOL4. That may be perfectly
sound, but it means the final story is no longer "RISC-V execution directly
against Sail RISC-V plus Iris contracts." The trust and proof chain becomes:

```text
RISC-V binary
  -> proof-producing HolBA lifter
  -> BIR program
  -> proof-producing symbolic execution / WP
  -> HOL4 contract
  -> backlifted ISA-level claim
  -> refinement of EvmSailSpec
```

That is still much stronger than an unchecked SMT co-execution lane, because
the lifting and symbolic execution are intended to produce HOL4-checkable proof
artifacts. It is weaker or at least less direct than an ideal Sail-RISC-V
machine-code proof if we want the whole stack to stay close to Sail and Iris.

Recommended HolBA spike:

1. Build a tiny RISC-V ELF with the same zkVM IO shape and full symbols.
2. Prove a contract for a single address range:

   ```text
   if input span contains schema id 0x0001
   then output span contains a fixed valid result digest
   ```

3. Repeat on the real Reth guest `entrypoint` wrapper, but summarize
   `run_stateless_guest` as a function contract.
4. Compare proof burden against the same slice in Islaris:
   setup time, manual annotations, loop handling, memory model, generated proof
   size, and ease of composing contracts.
5. Keep HolBA if it can produce reusable binary contracts faster than Islaris
   while preserving an explicit path back to the evm-sail relation.

My current ranking:

1. Use HolBA first for automation feasibility on small RISC-V slices.
2. Use Islaris first where direct Sail-RISC-V/Iris alignment matters more than
   automation.
3. Use bounded SMT co-execution to find counterexamples and shape contracts.
4. Do not choose a single final proof stack until HolBA and Islaris have both
   been tried on the same symbol-rich `entrypoint` wrapper.

## SMT Co-Execution Lane

Symbolically executing evm-sail and the RISC-V guest, extracting both to SMT,
and asking a solver for counterexamples is a good idea as an engineering lane.
It should be phrased as refinement against evm-sail, not peer equivalence:

```text
EvmSailSpec(i, o)
RiscVGuest(i, d)
d != sha256(o)
```

The solver query asks whether there exists an admitted input `i` and reachable
target execution that violates the evm-sail result. If the query is satisfiable,
we get a concrete counterexample. If it is unsatisfiable, we have strong
bounded evidence, and possibly a proof obligation that can be replayed or
checked in a theorem prover.

This can be very effective for bounded slices:

- IO wrapper behavior;
- schema-prefix and small SSZ decoding paths;
- output encoding and digest wrapper logic;
- arithmetic, stack, memory, and simple control-flow opcodes;
- trie nibble/path manipulation with fixed-depth bounds;
- warm-slot/access-list bookkeeping;
- small transaction fragments with fixed gas and memory bounds.

It is much less realistic as a direct whole-validator proof:

- loops depend on transaction count, gas, witness size, memory growth, and trie
  depth;
- heap allocation and Rust library code create large symbolic states;
- cryptographic functions explode unless modeled as contracts or
  uninterpreted functions;
- path count grows quickly around error handling, fork guards, and validation
  branches;
- the final claim would trust the evm-sail-to-SMT extractor, RISC-V-to-SMT
  extractor, memory model, and solver unless those are themselves proved or
  independently checked.

The secure version of this lane is not "SMT says unsat, therefore verified."
It is one of:

1. a proof-producing symbolic executor whose steps are checked in Rocq/HOL4;
2. SMT-generated obligations that are replayed in Iris/Rocq or HOL4;
3. SMT with independently checked certificates for the bit-vector/array
   fragments we rely on;
4. bounded counterexample search used only to shape later mechanized contracts.

For evm-sail specifically, we also need to account for what can actually be
symbolically extracted. Pure Sail functions are plausible candidates. Any
remaining external C helper, host IO, allocator behavior, precompile, or
accelerator call needs either a symbolic model or an explicit contract. This is
another reason to start with small slices and make the evm-sail relation
explicit before trying to push the whole validator through SMT.

Recommended use:

- Add an `smt-slices/` spike after the byte-level harness exists.
- Start with evm-sail SSZ/output helpers and a tiny RISC-V proof guest.
- Encode the query as "find a RISC-V behavior not allowed by evm-sail."
- Treat satisfying assignments as bugs or spec-boundary mismatches.
- Treat unsat as bounded evidence unless the extractor and solver result are
  checked by a theorem prover.

## Modular Versus Opcode-By-Opcode

Opcode-by-opcode compliance is too narrow for the stateless validator as a
whole. It can prove that a target EVM step refines the evm-sail step for a
single opcode under matched preconditions, but the deployed guest also depends
on input decoding, payload conversion, trie witness interpretation, block
validation, receipts/logs, state-root recomputation, public-output encoding, and
zkVM platform behavior.

The better decomposition is module-by-module, with opcode-level contracts only
inside the EVM execution module:

1. IO ABI contract:
   `read_input` returns a stable read-only private input span; `write_output`
   concatenates public output spans.
2. SSZ input/output contract:
   schema-prefixed input decoding and plain result encoding agree byte-for-byte.
3. Payload root contract:
   the target computes the SSZ hash-tree-root that evm-sail computes for
   `NewPayloadRequest`.
4. Witness/trie contract:
   witness nodes are interpreted as the same secure Merkle Patricia trie, and
   target state-root recomputation refines evm-sail's hashed account and
   storage-key semantics.
5. EVM transition contract:
   for each transaction, the target state transition refines the evm-sail
   transition under matched fork, block environment, access list, gas, call,
   precompile, selfdestruct, and warm-slot semantics.
6. Crypto/precompile contract:
   keccak256, sha256, ripemd160, secp256k1, and other precompiles either run
   through a proved implementation or through an explicitly assumed zkVM
   accelerator contract.
7. Output-normalization contract:
   raw `StatelessValidationResult` SSZ bytes are hashed exactly once in the
   current Reth public-output wrapper.
8. ELF/platform contract:
   the RISC-V binary obeys the memory map, stack/heap invariants, ABI, allowed
   host calls, and normal/abnormal termination semantics.

This modular strategy gives useful partial theorems early. A failed theorem can
be localized to SSZ, trie, execution, crypto, or platform glue instead of
appearing as one opaque whole-program mismatch.

## Proof Architecture

### Layer 0: Executable Compliance Harness

Before the mechanized proof, build a deterministic harness that runs evm-sail
and the target guest on the same fixtures:

```text
input.ssz
  -> evm-sail native/zkVM runner
  -> raw_output.ssz
  -> sha256(raw_output.ssz)

input.ssz
  -> ere-guests Reth run_stateless_guest
  -> raw_output.ssz

input.ssz
  -> pinned Reth guest ELF
  -> public_output_digest
```

Expected checks:

```text
evm_sail_raw_output == reth_run_stateless_guest_raw_output
sha256(evm_sail_raw_output) == reth_guest_elf_public_output
```

This is not the final proof, but it provides the exact byte relation the proof
must establish. The left-hand side is always evm-sail; the Reth values are
targets being checked against it.

### Layer 1: evm-sail Specification Relation

Define a proof-oriented evm-sail relation over SSZ bytes, not over high-level
Rust data structures and not over an external "common" spec:

```text
EvmSailSpec(input_bytes, output_bytes)
```

The relation should be generated from or justified by evm-sail and should
specify:

- how schema id `0x0001` is parsed;
- what default result is produced for decode failure;
- how chain config is checked;
- how the execution witness is consumed;
- how validation failure differs from abnormal guest failure;
- how `StatelessValidationResult` is encoded.

The relation can initially be represented as an evm-sail executable oracle plus
prose constraints, then migrated into Coq/Rocq, HOL4, Isabelle, or
Sail-generated theorem-prover definitions. The important point is that the
authority flows from evm-sail into the relation, not from Reth or
execution-specs into evm-sail.

### Layer 2: evm-sail Spec Extraction And Lemmas

For evm-sail, the first proof obligation is not "prove evm-sail equivalent to
Reth." It is to make the evm-sail stateless validator semantics precise enough
that target binaries can refine it:

```text
evm_sail_main(input_bytes) emits output_bytes
implies EvmSailSpec(input_bytes, output_bytes)
```

Useful sublemmas:

- SSZ decoder reads exactly the bytes described by the schema.
- MPT witness lookup is keyed by secure node hash.
- Account and storage state roots are recomputed from hashed account/slot keys.
- Transaction execution preserves the state relation expected by the trie
  recomputation step.
- Public output is only the canonical result, not debug output.

The current EEST/EELS corpus remains important as regression evidence, but it
should be treated as test evidence, not a substitute for these lemmas.

### Layer 3: RISC-V ELF Correctness To The Relation

For the Reth RISC-V guest, prove a machine-code contract:

```text
Given private input i at the zkVM input span and an otherwise canonical
initial machine state, the ELF either:

1. terminates normally and writes sha256(o), where EvmSailSpec(i, o), or
2. terminates abnormally only under an admitted abnormal condition.
```

For the current Reth guest, this should be split into:

- a wrapper theorem for `entrypoint`: read input, call validator, hash result,
  write digest;
- contracts for `run_stateless_guest`;
- contracts or assumptions for Reth/revm/stateless trie validation internals;
- contracts for platform calls and accelerators.

Trying to inline all of Reth/revm into one symbolic trace is likely to explode.
The proof should use function contracts and symbolic summaries aggressively.

## Trust Boundary

The final claim should make the trusted computing base explicit.

Trusted or assumed unless separately proven:

- the selected zkVM's implementation of `read_input` and `write_output`;
- the selected zkVM's accelerator/precompile implementations, if used;
- the hash functions if treated axiomatically rather than proven from code;
- the theorem prover kernel and proof tooling;
- the correctness of any binary loader/ELF-to-memory initialization model;
- the exact RISC-V ISA model variant and enabled extensions.

Not trusted if the machine-code proof is completed:

- Rust compiler correctness for the Reth guest;
- linker correctness beyond the fact that the proof is over the final ELF;
- the source-level intent of Reth/revm;
- hand-audited assembly, allocator behavior, or platform glue, except where
  explicitly assumed as host/platform contract.

This is why proving the final ELF is valuable. Source-level verification alone
would leave the compiler, linker, allocator, and target-specific runtime in the
trusted computing base.

## Main Practical Risks

- Guest target selection: the proof model must match the deployed Reth guest
  ELF target. This is independent of evm-sail's local executable harness target,
  because evm-sail is the specification rather than the implementation being
  verified.
- Output mismatch: evm-sail emits raw SSZ; current Reth guest publishes
  `sha256(raw SSZ)`.
- Binary size: the full Reth guest may be too large for direct Islaris
  trace-generation without modular summaries.
- Heap and allocator: compiled Rust guest code will use allocation patterns
  that need either proof summaries or a memory/allocator contract.
- Panics and error paths: decode failure is a normal default result in the Reth
  library path, but other panic/abort paths must be classified carefully.
- Crypto/precompiles: proving cryptographic implementations inside the guest is
  probably not the right first step. Accelerator contracts are more realistic.
- Fork surface: Amsterdam/Gloas-era stateless inputs, Cancun/Prague behavior,
  selfdestruct, access lists, warm slots, refunds, and blob/execution-request
  fields all need precise fork guards.
- Boundedness: any mechanized proof over loops needs input-size, witness-size,
  transaction-count, gas, and memory-growth bounds. These should come from the
  SSZ schema and EVM gas rules, not ad hoc proof assumptions.

## Proposed Milestones

### Milestone 1: Pin The Exact Compliance Relation

Deliverables:

- Build or obtain the exact RISC-V Reth guest ELF.
- Produce a symbol-rich proof ELF and, if needed, a stripped deployment ELF with
  identical loadable segment digests.
- Record symbol table, DWARF availability, disassembly, linker map, build id,
  ELF hash, loadable segment hash, target triple, ISA extensions, platform, and
  dependency commits.
- Add a proof-slice manifest that maps stable symbols and address ranges to the
  intended modular contracts.
- Add a local conformance runner that compares:
  `evm_sail_raw_output`, `reth_run_stateless_guest_raw_output`, and
  `reth_elf_public_digest`.
- Decide whether to keep the deployed digest boundary or add a
  verification-only raw-output Reth guest.

Exit criterion:

```text
For the aligned fixture corpus:
evm_sail_raw == reth_raw
sha256(evm_sail_raw) == reth_elf_public
```

### Milestone 2: Tiny ELF Proof Spike

Use Islaris first. Prove a tiny RISC-V guest contract over the same IO shape:

```text
read_input -> deterministic transform -> write_output
```

Examples:

- echo input length and SHA-256 digest;
- parse the schema id and return default output on mismatch;
- write a fixed `StatelessValidationResult` for a small fixed input.

Run the same tiny guest through HolBA if Islaris setup or automation is too
fragile. Compare actual proof burden, not just documentation promises.

Exit criterion:

- one checked proof over a post-link RISC-V binary;
- explicit memory map and IO contract;
- documented proof steps and pain points.

### Milestone 3: SSZ And Output Contract

Prove or mechanize enough of the SSZ boundary to show:

- schema prefix handling;
- result encoding;
- default decode-failure result;
- digest wrapper correctness.

This milestone intentionally avoids full EVM execution. It creates the first
reusable contracts needed by the real guest.

### Milestone 4: Trie/Witness Contract

Prove agreement on:

- witness node hash lookup;
- secure account key = `keccak256(address)`;
- secure storage key = `keccak256(slot)`;
- branch/extension/leaf traversal;
- deletion/collapse invariants;
- post-state-root recomputation over update paths.

This is the most important shared semantic layer after SSZ because it constrains
the state-root soundness story directly.

### Milestone 5: EVM Step Refinement

At this point opcode-level proof becomes useful. Prove selected EVM transition
contracts in increasing scope:

1. arithmetic/stack/memory opcodes;
2. storage reads/writes and access lists;
3. calls and returndata;
4. create/create2;
5. selfdestruct and account deletion;
6. precompiles and crypto contracts;
7. fork-specific gas/refund behavior.

The theorem should be a state-transition refinement, not just "same opcode
result" in isolation.

### Milestone 6: Reth Guest Contract Scaling

Apply Islaris/HolBA contracts to progressively larger Reth ELF regions:

- wrapper/output digest;
- SSZ decode/encode routines;
- trie/witness routines;
- execution loop with summarized contracts that refine evm-sail state
  transitions;
- full ELF, only if the summary approach scales.

If this stalls on full Reth/revm complexity, the fallback should be a
verification-oriented guest with a smaller implementation surface, not a weaker
theorem over the same enormous ELF.

## Open Decisions

- Is the desired final public value raw SSZ or SHA-256 digest of SSZ?
- Are crypto/precompile accelerators inside the theorem or explicit trusted
  contracts?
- Should the first formal spec live in Rocq/Coq, HOL4, Isabelle, or Sail plus
  generated theorem-prover definitions?
- Which evm-sail revision and generated theorem-prover artifacts define the
  source-of-truth semantics?
- What exact input bounds are admitted in the formal theorem?

## Concrete Next Step

Extend the new `proof-spikes/` area in two directions:

1. Build the selected Reth/ZisK backend ELF from
   `ere-guests afc3f4e0fdd8f23353469fdc088af51c3a765194` plus the local
   proof-profile addition:

   ```sh
   rtk /Users/f/dev/ethereum/ere-guests/scripts/build-reth-zisk-proof-candidate.sh
   ```

   The script records:

   ```text
   backend = Reth ZisK
   target = riscv64ima-zisk-zkvm-elf
   compiler toolchain
   Cargo.lock hash
   feature set
   proof ELF hash
   symbol table
   disassembly
   ```

2. `proof-spikes/byte-relation/`: a native differential runner that consumes one
   SSZ fixture and prints:

   ```text
   evm_sail_raw_output
   sha256(evm_sail_raw_output)
   reth_run_stateless_guest_raw_output
   reth_guest_elf_public_output
   ```

3. `proof-spikes/islaris-io/` or `proof-spikes/holba-io/`: a tiny RISC-V guest
   using the same `read_input`/`write_output` ABI and one checked proof of its
   output contract.

That gives us one provenance artifact, one executable byte-relation artifact,
and one proof-tool feasibility artifact before we commit to a full Reth ELF
proof plan.

## Sources

- Sail ISA language and theorem-prover backends:
  https://github.com/rems-project/sail
- Sail RISC-V model:
  https://github.com/riscv/sail-riscv
- Islaris repository:
  https://github.com/rems-project/islaris
- Islaris PLDI 2022 paper:
  https://www.cl.cam.ac.uk/~pes20/2022-pldi-islaris.pdf
- Iris project:
  https://iris-project.org/
- HolBA repository:
  https://github.com/kth-step/HolBA
- HolBA RISC-V symbolic execution paper:
  https://arxiv.org/abs/2503.14135
- Formal-ISA-based symbolic execution paper:
  https://arxiv.org/abs/2404.04132
- zkVM standards IO interface:
  https://raw.githubusercontent.com/eth-act/zkvm-standards/main/standards/io-interface/README.md
- zkVM standards accelerator interface:
  https://raw.githubusercontent.com/eth-act/zkvm-standards/main/standards/c-interface-accelerators/README.md
