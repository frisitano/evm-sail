# evm-sail Proof Lowering

This directory contains proof-oriented lowering artifacts for relating the
RISC-V guest to `evm-sail`.

The files here are not an alternate executable guest. They expose small,
pure, byte-level Sail relations that mirror selected parts of the executable
model while avoiding host I/O, C maps, crypto accelerators, and zkVM runtime
effects. Those relations are the source-side contracts we can compare against
RISC-V slices and later compose into larger equivalence obligations.

## Current Slice

- `sail/schema_prefix.sail`
  - Lowers the schema-prefix check in
    `sail/lib/ssz/stateless_input.sail::input_well_formed`.
  - Captures that the `SszStatelessInput` schema id is parsed as big-endian:
    schema id `1` is bytes `00 01`.
- `sail/io_contracts.sail`
  - Lowers the pure predicate over the schema bytes and offsets loaded by
    `sail/lib/ssz/stateless_input.sail::input_well_formed`.
  - Models the fixed 18-byte SSZ header without pretending random-access input
    memory is a linked Sail list: two schema bytes plus four decoded
    little-endian offsets cross into the relation as values.
  - Keeps the later payload, witness, chain-config, and public-key regions as
    length/offset obligations for subsequent slices.
- `smt/schema-prefix-refinement.smt2`
  - A small standalone SMT sanity check for the schema-prefix relation shape.
- `riscv/schema-prefix-slice/`
  - A tiny `no_std` RV64 guest slice with stable symbols:
    `proof_schema_prefix_ok` and `proof_input_header_well_formed`.
  - Uses the same big-endian schema prefix and little-endian offset checks as
    the Sail proof adapters.
- `extern-boundary.md`
  - Inventories the generated Sail-to-Coq extern/impure boundary.
  - Groups the host-facing operations into proof contracts that can be shared
    by generated `evm-sail` Coq proofs and later RISC-V/Islaris equivalence
    obligations.
- `coq/ExternBoundary.v`
  - The initial Coq contract skeleton for the private input oracle, output
    trace, accelerator/crypto calls, memory/stack/code views, world-state/code
    DB, and witness/MPT DB.

## Local Checks

From the repository root:

```sh
rtk make -C proof check
rtk make -C proof check-extern-boundary
rtk z3 proof/smt/schema-prefix-refinement.smt2
rtk make -C proof check-riscv
rtk make -C proof check-proof-pipeline
```

`check-riscv` builds the slice for the same non-compressed ZisK target used by
the executable guest:

```text
riscv64ima-zisk-zkvm-elf
```

The target is intentionally `rv64ima`, not `rv64imac`. The ZisK Islaris config
sets `misa.C = 0`, so the proof ELF must not contain compressed instructions.
`check-riscv-no-rvc` uses `objdump` to assert that every emitted instruction is
32-bit wide and that `.text` is 4-byte aligned.

`check-proof-pipeline` adds the current proof-lowering checks:

- Rust formatting for the RISC-V slice.
- Sail-to-Isla IR generation for `sail/schema_prefix.sail` and
  `sail/io_contracts.sail`.
- Sail-to-Coq generation for the same source-side adapters, producing Coq
  definitions for `schema_prefix_ok` and `input_header_well_formed`.
- A one-instruction Islaris stack-store smoke check that locks in the memory
  alignment assumptions needed by the ZisK RISC-V frontend.
- Full executable ELF extraction through `isla-riscv-dump` for the small proof
  ELF.
- Full per-instruction Coq generation through Islaris for the small proof ELF.
- A stable staged Dune layout for the generated Islaris theory, rooted at
  `/tmp/evm-sail-proof-lowering/coq-stage/` by default.

The generated Islaris dump goes to
`/tmp/evm-sail-proof-lowering/islaris-dump/elf.dump` by default and records:

```text
Full executable ELF
Max instructions: full target
Memory alignment: Strict
//@isla-config: riscv64_zisk_isla_coq.toml
```

This is deliberately a full-ELF path. `--max-instructions` remains a diagnostic
option in the lower-level extractor, but the proof pipeline does not pass it.
`objdump` is used for provenance and checks; executable extraction reads the ELF
bytes directly through `isla-riscv-dump`.

The strict memory-alignment default is narrower than changing the executable
slice: it adds explicit low-bit alignment facts for multi-byte memory
footprints in the Islaris dump. The native extraction mode remains available
with `ISLARIS_MEMORY_ALIGNMENT=native` for comparing against the raw
`ere-guests` flags, but the automated proof-lowering check uses strict
alignment because the current proof ELF contains stack and global word accesses
whose footprints otherwise remain too unconstrained for Islaris.

`run-islaris-coq` invokes Islaris on the full dump:

```sh
rtk make -C proof run-islaris-coq
```

That target is part of `check-proof-pipeline`. It produces an `instrs.v` table
covering the full proof ELF entry range, currently from `0x80000000` through
`0x80000190`.

`run-sail-coq` lowers the source-side proof adapters through Sail's Coq
backend:

```sh
rtk make -C proof run-sail-coq
```

The emitted Coq files live under `/tmp/evm-sail-proof-lowering/sail-coq/` by
default. The important source-side symbols are:

```text
schema_prefix.v: Definition schema_prefix_ok
io_contracts.v: Definition input_header_well_formed
```

The full `evm-sail` sources also have explicit Sail-to-Coq targets. These are
kept separate from `check-proof-pipeline` because they take minutes rather than
seconds and are source-side proof artifacts, not the small RISC-V slice smoke
test:

```sh
rtk make -C proof check-evm-sail-coq
```

`check-evm-sail-coq` extracts the complete model and its sole executable entry
from `sail/evm.sail_project` into:

```text
/tmp/evm-sail-proof-lowering/evm-sail-coq/evm.v
/tmp/evm-sail-proof-lowering/evm-sail-coq/evm_types.v
```

The important initial symbols include:

```text
evm.v: Definition process_transaction
evm.v: Definition compute_state_root
evm.v: Definition decode_stateless_input_ref
evm.v: Definition main
```

## Full evm-sail zkVM Guest ELF

The full executable `evm-sail` zkVM guest is a separate lowering target from
the small proof slice above and from the production `ere-guests` Reth guest
below:

```sh
rtk make -C proof check-evm-sail-zkvm-dump
```

This first builds the guest at:

```text
zkvm/build/zkvm_guest.elf
```

and checks the resulting ELF with `objdump`: the emitted text must be
non-compressed 32-bit RISC-V instructions, `.text` must be at least 4-byte
aligned, the instruction count must cover the full guest rather than a small
slice, and the text section must not contain `ecall`, `ebreak`, `mret`, or
`sret`.

The dump is written to:

```text
/tmp/evm-sail-proof-lowering/evm-sail-zkvm-islaris/elf.dump
```

with no `--symbol` and no `--max-instructions`, so it is a full-ELF artifact.
The evm-sail guest path intentionally uses:

```text
EVM_SAIL_ZKVM_ISLARIS_CONSTRAINTS=zisk-local
ISLARIS_MEMORY_ALIGNMENT=strict
ISLARIS_CONFIG=riscv64_zisk_isla_coq.toml
```

`zisk-local` follows the way the checked-in Islaris RISC-V examples are
structured: constraints are annotations on the next instruction, not a global
block repeated before every instruction. Memory instructions get the ZisK
machine-mode/no-RVC/MPRV profile facts plus RAM-range and, in strict mode,
low-bit alignment facts. PC-relative control-flow instructions get a PC
alignment fact. `jalr` gets its target low-bit alignment fact. CSR/system
instructions get the trap-profile facts needed for direct-mode trap handling.
The older `baseline` mode remains available for existing callers and for
comparison, but it is intentionally not the default for the full evm-sail guest
dump.

Full Islaris lowering for this guest is explicit because it produces one Coq
file per instruction:

```sh
rtk make -C proof run-evm-sail-zkvm-islaris-coq
```

For practical progress, use shards:

```sh
rtk make -C proof run-evm-sail-zkvm-islaris-coq-shard \
  EVM_SAIL_ZKVM_SHARD_INDEX=0 \
  EVM_SAIL_ZKVM_SHARD_SIZE=1024
```

The shard runner writes bounded temporary dumps, copies generated
per-instruction Coq files back into the common full-guest coverage directory,
and refreshes the progress report. After complete coverage, synthesize the
final instruction index with:

```sh
rtk make -C proof finalize-evm-sail-zkvm-islaris-coq
```

`stage-islaris-coq` copies the generated Islaris instruction theory into a
small Dune workspace with symlinks to the Islaris Coq theories:

```sh
rtk make -C proof stage-islaris-coq
```

The normal proof pipeline checks that this staged theory layout exists and
imports the generated `isla.instructions.evm_sail_schema_prefix` files. Actual
`.vo` compilation is an explicit target because this environment's active opam
switch is the Sail switch and does not currently provide `coqc`:

```sh
rtk make -C proof check-islaris-coq-compile
```

After installing the Islaris Coq opam dependencies, set
`REQUIRE_COQ_COMPILE=1` to make that compile step part of the full pipeline:

```sh
rtk make -C proof check-proof-pipeline REQUIRE_COQ_COMPILE=1
```

`proof/islaris/stack-store-smoke.dump` is the reduced regression for the former
frontier: the first stack store, `sd ra,8(sp)`. It documents and checks the
stack alignment and RAM-bound assumptions used by the full dump.

The first symbol to compare against `schema_prefix_ok` is
`proof_schema_prefix_ok`.

Do not use the older `proof-spikes/riscv-symbol-slice` schema predicate as the
source of truth for this obligation: that spike encoded byte prefix `01 00`,
while both `evm-sail` and `ere-guests` parse schema id `1` as big-endian
prefix `00 01`.

## Production Reth ZisK ELF

The small proof ELF above is the current proof-contract laboratory. The final
target is the production Reth ZisK guest from the sibling `ere-guests` checkout.
That path is now a first-class proof target:

```sh
rtk make -C proof check-ere-full-dump
```

This uses:

```text
/Users/f/dev/ethereum/ere-guests/scripts/extract-zisk-elf-for-islaris.sh
```

with no `--symbol` and no `--max-instructions`, so it extracts the full
executable ELF. The target checks the generated dump and provenance for:

```text
Full executable ELF
Max instructions: full target
Memory alignment: Native
mode=full-executable-elf
max_instructions=full
memory_alignment=native
islaris_config=riscv64_zisk_isla_coq.toml
zisk_target=riscv64ima-zisk-zkvm-elf
zisk_profile=proof
```

The current local Reth ZisK artifact produces:

```text
/Users/f/dev/ethereum/ere-guests/proof-artifacts/reth-zisk/islaris/elf.dump
524596 RV64 instructions, last PC 00000000802004cc
```

Full Islaris Coq generation for this production guest is available as an
explicit proof-lowering target:

```sh
rtk make -C proof run-ere-full-islaris-coq
```

That target does not use the native dump directly. It first builds a second
full-ELF dump with `--memory-alignment strict`:

```sh
rtk make -C proof check-ere-full-proof-dump
```

The strict proof dump lives at:

```text
/Users/f/dev/ethereum/ere-guests/proof-artifacts/reth-zisk/islaris-strict/elf.dump
```

It still uses no `--symbol` and no `--max-instructions`; the instruction count
and PCs match the native full dump. The intended proof-lowering differences are
side conditions, not executable slicing: multi-byte loads and stores receive
explicit low-bit alignment constraints before their RAM-range constraints, and
the reset baseline constrains `mtvec.bits[1:0] = 0b00` so trap-generating
instructions use the direct-mode trap vector. Those constraints make
`isla-footprint` able to lower word-sized accesses and early trap/CSR
instructions that are too unconstrained in the native dump. The native dump
remains the `ere-guests` flag/provenance parity artifact; the strict dump is the
proof-lowering artifact whose side conditions must be discharged in the
equivalence argument.

By default, production Islaris lowering writes to:

```text
/tmp/evm-sail-proof-lowering/reth-zisk-full-strict-coq/
```

and uses Coq namespace:

```text
isla.instructions.reth_zisk_full_strict
```

This is intentionally not part of `check-proof-pipeline` yet: the dump is
hundreds of megabytes and the generated theory contains one file per
instruction. Use it to measure whether the current Islaris path scales to the
production ELF before making it a default gate.

A sampled monolithic run on 2026-07-02 did start lowering the real guest, but
only reached hundreds of instructions after startup. That is enough to prove the
tool path is connected, and it also shows that the production proof needs
resume/shard accounting rather than a blind multi-day single process.

That same native monolithic run exposed the first proof-lowering blocker at:

```text
0x0000000080000034: 0062a023
```

This store failed under native memory alignment because the footprint lacked
the low-bit alignment fact for a multi-byte access. The next blocker was the
first trap/CSR frontier at `0x0000000080000040` (`ecall`) and
`0x0000000080000048` (`rdcycle`), which required the direct-mode `mtvec`
baseline side condition described above.

As of 2026-07-03, the first 14336 instructions lower successfully from the
strict full-ELF dump, including the previous `a80000034.v`, `a80000040.v`, and
`a80000048.v` frontier files. The current contiguous frontier covers
`0x0000000080000000` through `0x000000008000dffc`; the next ungenerated
instruction is `0x000000008000e000`.

The latest Islaris frontend change also gets past the RV64A frontier at
`0x0000000080004868` (`amoadd.d.rl`). That instruction produced a symbolic
failed-write branch in `isla-footprint` that Sail retired through
`internal_error`. For ZisK proof-profile instructions with the RISC-V atomic
opcode field `0x2f`, Islaris now adds `--continue-on-error --kill-at
zinternal_errorzIERetiredz5zK,zexecute`, which keeps the successful footprint
path and prunes that execute-time error branch. The matcher reads the low
seven opcode bits from the encoded instruction byte, so it also covers later
atomic encodings whose last byte is not literally `0x2f`, such as the
`0x00000000800073e8` frontier instruction (`00b535af`).

Use the progress target to audit the current production-lowering output against
the strict full dump:

```sh
rtk make -C proof report-ere-full-islaris-coq
```

This writes
`/tmp/evm-sail-proof-lowering/reth-zisk-full-strict-coq-progress.md` by
default. The report counts a non-empty `a<pc>.v` file as a lowered instruction
and compares those files against all `524596` PCs in `islaris-strict/elf.dump`.
Zero-byte `.isla` files are treated as interrupted in-flight worker output, not
as proof coverage.

Two check targets use the same accounting:

```sh
rtk make -C proof check-ere-full-islaris-progress
rtk make -C proof check-ere-full-islaris-coq
```

`check-ere-full-islaris-progress` only asserts that at least one production
instruction has been lowered. `check-ere-full-islaris-coq` still runs the
long-running monolithic Islaris target, then requires complete per-instruction
coverage and a generated `instrs.v`.

For practical production progress, run bounded shards and merge only the
generated per-instruction files into the common coverage directory:

```sh
rtk make -C proof run-ere-full-islaris-coq-shard \
  ERE_ZISK_FULL_SHARD_INDEX=0 \
  ERE_ZISK_FULL_SHARD_SIZE=1024
```

Shard indices are zero-based and cover
`[index * size, min((index + 1) * size, instruction_count))` in the full dump.
Each shard writes a temporary dump under
`/tmp/evm-sail-proof-lowering/reth-zisk-full-strict-shards/`, runs Islaris on
that bounded dump, copies any generated `a<pc>.v` and `a<pc>.isla` files into
`/tmp/evm-sail-proof-lowering/reth-zisk-full-strict-coq/`, then refreshes the
progress report. The copy step runs even when Islaris reports a failing
instruction later in the shard, so successful instructions are not lost. This
preserves the full-ELF coverage target while avoiding a single-process
multi-day run.

After every PC in the full dump has a non-empty generated Coq file, synthesize
the final full instruction index and Dune theory:

```sh
rtk make -C proof finalize-ere-full-islaris-coq
```

This target refuses to write `instrs.v` unless coverage is complete.

## Next Source-Side Coq Steps

1. Select or install a Coq switch with Sail's generated-library dependencies,
   then compile `evm.v` to a `.vo` file.
2. Audit the generated extern/impure boundary in the generated Coq files:
   crypto accelerators, SSZ input reads, output emission, and C-backed host
   structures should become explicit assumptions or imported contracts.
3. Add a small hand-written Coq harness module that imports the generated
   theory and states the first source-level lemmas around
   `process_transaction` and `compute_state_root`.
4. Add a second harness for the executable entry point around
   `decode_stateless_input_ref` and `main`, then relate that guest-level
   contract back to the core EVM semantics.

## Next RISC-V Lowering Steps

1. Use `report-ere-full-islaris-coq` to track production Reth ZisK lowering
   coverage against the full `524596`-instruction dump.
2. Run `run-ere-full-islaris-coq-shard` across the production dump until the
   report reaches complete coverage. The next contiguous shard starts at index
   `8` with `ERE_ZISK_FULL_SHARD_SIZE=1024`. Then run
   `finalize-ere-full-islaris-coq`.
3. Install or select an Islaris Coq opam switch and run
   `check-islaris-coq-compile` so the staged instruction table produces
   `.vo` files.
4. Add the first Coq bridge lemma relating the staged
   `isla.instructions.evm_sail_schema_prefix.instrs.instr_map` entry for
   `proof_schema_prefix_ok` to the Sail-generated `schema_prefix_ok`
   definition.
5. Extract the `proof_schema_prefix_ok` RISC-V instructions into an Isla or SMT
   summary and compare it to `schema_prefix_ok`.
6. Extend the same flow to `proof_input_header_well_formed`.
7. Extend `io_contracts.sail` with bounded payload/witness/chain-config region
   contracts.
8. Add abstract crypto and trie contracts before attempting block execution.
9. Only after the small contracts are stable, compose them into a larger
   stateless-validation relation.
