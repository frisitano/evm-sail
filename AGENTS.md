# evm-sail Agent Notes

## Source Of Truth

This file is the single repo-local instruction source for coding agents. Keep
`CLAUDE.md` as a thin import of this file; update this file instead of
duplicating instructions elsewhere.

## Local Operating Rules

- In Tau's Codex environment, prefix shell commands with `rtk` as required by
  `/Users/f/.codex/RTK.md`.
- Work in the active worktree. Do not write to the protected main checkout at
  `/Users/f/dev/ethereum/evm-sail` unless Tau explicitly asks for that checkout
  to be modified.
- Before edits, check the worktree state with `rtk git status --short --branch`.
- Prefer `rg` / `rg --files` for search.
- Use `apply_patch` for hand-written file edits.
- Do not clean, revert, or remove unrelated user changes or generated artifacts
  unless explicitly asked.

## Repo Map

- `sail/evm.sail_project` is the single model project and `sail/main.sail` is
  its only executable entry. There is no entry/backend selection: every build
  contains the full stateless validator. The canonical project contains no
  cycle-scope declarations or calls. `EVM_PROFILE=on` is an optimized-C
  build-only switch that applies the ordered
  `sail/optimised/profile/manifest` overrides after the ordinary optimized
  overrides; standard builds reject it. Optimized overrides mirror the
  canonical module tree under `sail/optimised/`: an override of
  `sail/lib/rlp/encoding.sail`, for example, lives in
  `sail/optimised/lib/rlp/encoding.sail`. `sail/optimised/manifest` fixes their
  application order.
  `EVM_DEBUG=on|off` controls native-test validation diagnostics; real zkVM
  guest builds set it to `off`.
- `sail/host/state.sail` is the declaration-only host world-state surface for
  impure FFI contracts, while `sail/host/journal.sail` owns the closed journal
  entry algebra and its checkpoint/commit/revert contracts.
  `sail/host/environment.sail` declares the fixed-size,
  O(1) ancestor-hash host table. `sail/kernel/environment.sail` owns the kernel
  registers, and the remaining `sail/kernel/` modules group pure Sail `k_*`
  operations by subsystem.
- `sail/executor/payload.sail` owns the transaction and withdrawal MPT roots
  used to reconstruct and validate the execution payload's block hash. Its
  `IndexedTrieSource` variants share the indexed transaction, withdrawal, and
  receipt trie reduction without erasing the source identity into booleans.
- `sail/lib/rlp/encoding.sail` and `decoding.sail` are the shared RLP
  primitives. Encoding callers size first and use `RlpEncoder`, which owns an
  exact scratch reservation and verifies the written width at `finish`.
  Protocol codecs live under `sail/lib/rlp/codecs/` (addresses, transaction
  envelopes and signing preimages, receipts, withdrawals, block headers,
  state leaves, and the block access list); executor modules retain lifecycle
  and validation policy rather than wire-format details.
- `sail/lib/mpt/` is the generic trie implementation, split by dependency
  layer into paths/hex-prefix primitives, node/reference types, the node RLP
  codec, ordered updates/canonical rebuilding, and authenticated traversal.
  Its public root
  entry is `trie_root(base_root, updates)` (witness-native overlay, fail-closed
  on missing node material). `TrieUpdateSource` defunctionalizes account and
  per-account storage update streams behind one cursor/reducer; callers select
  a source variant rather than duplicating the trie algorithm.
  `sail/lib/state_trie.sail`
  owns Ethereum account/storage decoding, stateless reads, and post-state-root
  assembly over that core. The Yellow Paper Appendix C/D equations are kept as
  documentation on the internal functions; an empty base computes TRIE(I)
  directly.
- The impure host interface is an abstract `val` contract layer declared inline
  in the module files themselves as `val X = impure { c: "sym" } : T`
  (`primitives/crypto.sail` hashing, `lib/ssz/ssz.sail` input,
  `host/output.sail` guest output, `host/state.sail` / `host/environment.sail` /
  `host/memory.sail` world state, block environment, and buffers,
  `host/nodes.sail` trie node DB; see
  `extractions/contracts/ExternBoundary.v`). These `c:`-bound vals are the TRUE
  axioms (crypto core, I/O oracle, mutable host stores) -- extraction targets
  see them as bodyless parameters; executables link their C definitions from
  `ffi/`. The C backends are split completely: `ffi/spec/` owns the generated
  GMP-backed ABI, while `ffi/optimized/` owns the fixed-layout ABI,
  pointer-based stores and whole-operation replacements
  selected by the custom compiler. A build compiles exactly one directory;
  neither backend includes private headers or implementation from the other.
  Both compile against the GENERATED model header (`-DEVMSAIL_MODEL_H`, `-I`
  build dir) and name its concrete types directly, so generated layouts are
  never hand-mirrored or hidden behind a second ABI alias layer. Optimized
  headers include `evmsail/model.h`, the single generated-header selection
  shim required because Sail emits `--c-include` headers before its own model
  header; it declares no wrapper types or runtime interface.
  `ffi/spec/state.c` and
  `ffi/spec/code.c` construct the generated account/storage and `option(Code)`
  values; `ffi/spec/frame_stack.c` applies generated ownership operations only
  to continuation slots actually reached. Their optimized counterparts use
  direct fixed-layout assignment. Each backend's crypto implementation handles
  the hash axioms and segmented byte equality for its own ABI. Optimized log
  records and their block bloom are owned by `src/host/state/logs.c`, while
  `src/executor/receipts.c` owns encoded receipt retention and delegates only
  the ordered receipt-root reduction to the generic MPT module. Sail emits fixed
  256-bit JUMPDEST chunks
  directly into one length-preallocated packed C table, so no generated list
  crosses that boundary. A
  hash preimage is a list of Bytes segments -- materialized bytes or
  nominal region slices -- crossing in ONE call; the old streaming hash channel
  and fused source hashers are gone. Everything else,
  including the former C fast-path hooks (`keccak256_word`,
  `keccak256_address`, `sha256_pair`), is a pure Sail body compiled and
  executed directly. Fallible optimized-only C externs carry `$[c_throws]`;
  the custom compiler marks those calls as throwing and emits the ordinary
  generated `have_exception` unwind immediately after each call. Their C
  implementations raise the generated `InvalidBlock` exception directly,
  without result-union wrappers or process aborts. This annotation exists only
  in the optimized C splice, while spec C and proof extraction retain the
  explicit Sail bodies and throws. SSZ decoding reads only from
  `StatelessInputSlice`; the old byte-at-a-time `ssz_src_*` oracle no longer
  exists.
  `ffi/spec/hash.c`, `ffi/optimized/src/primitives/crypto.c`, and each backend's
  accelerator module call the root `zkvm_accelerators.h` contract directly; the
  former `host_crypto.c` forwarding layer has been removed.
  `ffi/optimized/src/lib/htr.c` is a narrower optimized-C refinement: the
  `sail/optimised/lib/htr.sail` override replaces the complete
  `htr_new_payload_request` operation with one pure C call that consumes the
  validated `StatelessInputRef` slices directly. Spec C and Lean/Coq
  extraction retain the explicit equations in `sail/lib/htr.sail`, so this
  optimization is not a proof axiom. The optimized implementation currently
  stages 32-byte leaves and 64-byte hash pairs locally. A raw-pointer input
  experiment saved only about 0.009% of whole-program ZisK steps and was
  rejected in favour of the validated-reference boundary;
  `zkvm_accelerators.h` remains unchanged.
  Optimized C also injects the header-only
  `ffi/optimized/include/evmsail/primitives/word.h`
  refinements for fixed hash/address ↔ native-word conversions. Spec C
  and proof extraction retain the direct canonical-byte concatenation and
  fixed-slice endian equations in `sail/prelude.sail`; neither standard nor
  optimized builds use temporary reversal vectors.
  The old `sail/c/*.sail` extern-binding menu and the
  `EVM_BACKEND=spec|build` project variable were deleted (this change).
- `ffi/spec/` and `ffi/optimized/` each contain a complete native C backend for
  memory/nominal region access/output storage, the Sail-cursor-owned executor
  scratch arena, account and persistent-storage state, the EIP-7928 recorder,
  transient storage, code/JUMPDEST storage, node DB, operand stack, and the
  suspended-frame stack. The optimized backend may additionally replace
  high-level Sail operations without imposing those representations on the
  spec build. Its source hierarchy follows Sail at established module
  boundaries. `ffi/optimized/include/evmsail/` contains the Sail-facing public
  declarations; `ffi/optimized/src/` contains implementations and private
  headers. In particular, `src/host/state.c` adapts generated aggregate values
  while `src/host/state/` owns the dense semantic state, transaction logs, BAL
  metadata, and state-root iterators. Large optimized storage is pointer-backed
  and bound once from `src/workspace.c` before model initialization. Modules
  use direct indexing into that stable, pre-provisioned region; they must not
  own capacity-sized backing arrays or allocate, resize, or free storage. The
  optimized code subsystem similarly separates content identity, packed
  JUMPDEST metadata, and owned synthesized bytes into `CodeTable`,
  `JumpdestTable`, and `CodeArena`. A nonzero `CodeId` is the opaque offset
  carried by Sail's code-region slice; immutable witness code remains borrowed,
  while only unstable memory/output or synthesized code is copied into the
  arena. Delegation classification is cached on the code row. Operand stacks,
  suspended frames, ancestor hashes, logs, and receipts follow the same
  one-time workspace-binding rule rather than owning capacity-sized static
  arrays.
  The hand-written raw-byte interpreter (`ffi/optimized/src/evm/interpreter.c`)
  is the SINGLE optimized interpreter: a computed-goto loop (one 256-entry
  label table, labels-as-values) with pc, gas, and the code slice held in
  loop LOCALS. In-TU fast-path arms (ALU, PUSH/POP/DUP/SWAP, EXP, JUMPDEST)
  charge and validate against the local gas, never touch the model registers
  on success, and dispatch the next opcode from their own tail; every in-TU
  failure branches to the single `interp_exc` tail, which publishes the local
  gas and performs the canonical `exc_halt` exactly once (exc_halt reads and
  then zeroes the gas register, so the publish-before-halt order is a
  consensus requirement). Arms whose handler is a generated Sail body
  publish the locals to the model registers first and reload after; frame
  entry additionally reloads the code locals. There is no switch-dispatch
  variant and no `EVM_INTERP` knob. Labels-as-values is a sanctioned,
  interpreter-only exception to the indirect-control-flow rule: it is an
  indirect BRANCH within one function over a closed static table.
  The optimized FFI audit enforces both the source manifest and this production
  policy. Closed families of optimized-host behavior use explicit tags and
  first-order dispatch rather than function pointers or callbacks; this keeps
  control flow visible to the compiler and proof-oriented source, and the audit
  rejects indirect C function declarations and calls. The BAL recorder uses distinct keyed hash
  tables for storage reads `(address, slot)` and storage changes
  `(address, slot, block_access_index)` plus field changes keyed by
  `(address, block_access_index)`. It sorts the dense rows once and exposes one
  account-delimited event stream; canonical RLP cursors, not the host stream,
  drive validation and enforce the address-plus-storage-key item limit.
  At the `ffi/` root, only the standardized `zkvm_accelerators.h` and
  `zkvm_io.h` platform contracts are shared. See `ffi/README.md`; common
  protocol behavior belongs in Sail rather than in a shared C compatibility
  layer.
- `harness/run.py` is the SINGLE fixture harness for the single executable
  entry, built ONCE as a shared library and driven IN-PROCESS via ctypes
  (`harness/dump_state.py`). Each case is serialized to the SSZ
  `SszStatelessInput` (`ssz_builder.py`, under the execution-specs venv).
  The Amsterdam stateless full-block validator is gated BYTE-EXACT against the
  EELS reference. State-test cases are executed through the in-process EELS
  t8n, which builds a fully VALID single-tx block input AND the reference
  `run_stateless_guest` output bytes; fixtures that already carry
  `statelessInputBytes`/`statelessOutputBytes` are fed directly. `--spike`
  swaps the execution vehicle for the REAL RISC-V guest ELF on spike
  (`zkvm/build.sh`); the ELF is built once without input, and each fixture is
  supplied at runtime through the standard `ffi/zkvm_io.h` `read_input` ABI.
  `--zisk` similarly builds the production input-agnostic ZisK ELF and drives
  it with `ziskemu`. The harness requires the emulator version to equal the
  `ziskos` version in `zkvm/zisk/Cargo.lock`; set `ZISKEMU` to select a
  compatible binary.
  `tools/zisk-guests/` is the stable machine-local home for the optimized EVM
  Sail, reth, and ethrex comparison ELFs. The binaries are ignored by Git;
  `tools/stage_zisk_guests.sh` refreshes them and their checksums, and
  `tools/benchmark_zisk.py` uses these three guests by default.
  `--debug` invokes the
  native-only `guest_debug_dump` after a failure; it is not linked into the
  real guest. `--profile` enables optional cycle-scope markers.
  This is the sole fixture runner; the parallel Rust runner was removed. The
  old `runner_ffi.c` (stdin ssz_src) is deleted, and the driver/test
  world-wipe lives in each backend's `native_test.c` as `guest_reset` (the
  model no longer defines `k_world_reset`). The
  dedicated `witness_probe` re-root
  harness was also removed: every stateless account/storage lookup walks the
  witness trie from the authenticated root (parsing each node on the path), so
  the full-corpus byte-exact checks exercise + validate
  the witness node reader (`lib/mpt/trie.sail`).

## Current State-Root Model

The old list-backed post-state-root model has been removed. The current model is
recursive and trie-shaped:

- State-root traversal is keyed by secure keys: `keccak256(address)` for
  accounts and `keccak256(slot)` for storage.
- The node DB maps node hash to decoded trie node and supports traversing
  authenticated pre-state/witness subtrees.
- Account execution state is split between authenticated/cache material and
  layered updates. Persistent storage follows the same cache/update pattern.
- Native full-state fixture roots can iterate the cache/update union. Stateless
  roots should use update-only traversal over the authenticated witness base.
- Delete handling must canonicalize MPT shape: collapse empty leaves/extensions
  and branch nodes with a single remaining child when sufficient node material is
  available.

## Build And Lint

Every model check, extraction target, and executable build resolves the same
custom Sail compiler through `zkvm/resolve_optimized_sail.sh`. That compiler
supports the standard Sail backends plus spliceable type definitions and
bound-driven C specialization. With `--c-specialize`, proof-backed fixed
representations are preserved across function arguments and results, and the
compiler also specializes function-body locals/intermediate values and call
destinations/arguments. Representation demands clone dependent function
signatures transitively through the call graph, while whole-body lifetime
analysis accounts for every value assigned to a mutable local. Prefer precise
semantic types and constraints
(`range`, singleton `int('n)`, dependent results, finite sets, and relational
constraints that express real protocol invariants) whenever they give the
compiler useful bounds. Keep related existential indices in scope by unpacking
a dependent value once and accessing its fields directly; avoid projection
helpers that widen those fields to broad aliases and discard their
relationships. Do not add artificial protocol caps solely to obtain a narrower
lowering. An unbounded mutable `nat` does not become bounded merely because its
initializer is small, and a concrete broad function signature is not narrowed
from an incidental caller; express the real invariant in the semantic type or
retain the mathematical representation. `make c-optimised` enables
`--c-specialize-log` by default so its output records the inferred argument
bounds, representation demands, and specialization worklist.

When several finite execution modes share an algorithm, represent the mode in
Sail with one closed enum/union and dispatch once at the semantic boundary.
This is the preferred defunctionalization boundary for shared cursor/reducer
machinery, and optimized C must mirror that source algebra rather than invent
parallel boolean flags or a C-only semantic switch. Keep genuine runtime
predicates (success/failure, warm/cold, presence, equality, decoder facts) as
booleans; do not introduce wrapper variants that add no impossible-state or
shared-algorithm benefit.

When arithmetic depends on fork-selected bounds, pass the `ProtocolProfile`
(or the smallest correlated profile subrecord) as an explicit argument. Do not
read `k_protocol_profile` inside the helper being constrained. Keeping the
finite profile combinations in the function signature lets optimized C
specialization propagate the selected fields through arguments, results, and
body intermediates. Read the register at the outer execution boundary and pass
the profile inward.

The custom compiler also recognizes `$[c_throws]` on impure externs and
propagates the `Throw` effect to generated C call sites. These features are
C-backend concerns: the base specification never carries C-representation or
optimized-exception annotations, while `sail/optimised/` may refine a semantic
type to an optimized-only C representation. In particular, canonical 20-byte
addresses and 32-byte digests retain byte-index semantics in Sail but use
`fixed_bytes_u64_lanes` in optimized C. Lean/Coq extraction retains the
ordinary semantic types without loading the C-only splice. Set `SAIL`
explicitly to test another build of this custom compiler; upstream Sail is not
a supported fallback for repository targets. Sail/Z3 query memoization is
explicitly enabled for model checks, extraction, and native/guest generation,
using the repo-local `sail_smt_cache` file. `Z3_MEMO_PATH` overrides it in Make
and `SAIL_Z3_MEMO_PATH` overrides it in the build scripts. Ordinary
`make clean` preserves this cache; use `make clear-z3-memo` only when compiler,
constraints, or solver changes require invalidating it.

An optimized splice that replaces a canonical Sail function with one C
operation must still define the canonical function: a same-named `val` alone
does not replace the base function body and can leave its generic call graph
reachable during C specialization. Bind the C operation under a name that
describes its semantic boundary, then keep the smallest canonical wrapper that
calls it. Do not use a generic `optimized_*` prefix. Wrappers may additionally
adapt arguments or results, check conditions, access registers, or compose
operations when that work is part of the refinement.

Run from repo root unless noted:

```sh
rtk make check
rtk make lint
rtk make fmt-check
rtk make c-spec
rtk make c-optimised
```

`make fmt-check` may surface pre-existing formatting drift. Do not format
unrelated files unless that is part of the requested task.

## tests-zkevm v0.6.2 Fixture Corpus

The only retained fixture corpus is
`zkvm/.fixtures/current-v062-full/`. It is ignored by git and workspace-local;
do not assume it exists in a fresh checkout.

- Source: `tests-zkevm@v0.6.2`, commit
  `e5a8caf1b8055e4d805c7fb169edfa710914b7da`.
- Generated 2026-07-19 with `t8n = 2.19.0` using
  `fill -m "blockchain_test or blockchain_test_engine" --fork Amsterdam -n 8`.
- Layout: `blockchain_tests/` and `blockchain_tests_engine/`, each containing
  Amsterdam and BPO2-to-Amsterdam transition fixtures.
- Inventory: 6,333 JSON files containing 26,104 embedded
  `statelessInputBytes`/`statelessOutputBytes` pairs.

There are no checked-in smoke, EELS subset, historical state-test, or older
zkevm fixture sets. Do not recreate or use pre-v0.6.2 corpora as aligned
evidence.

Run one native embedded fixture from the repository root:

```sh
rtk python3 harness/run.py --limit 1 --quiet \
  zkvm/.fixtures/current-v062-full/blockchain_tests/for_amsterdam/shanghai/eip3855_push0/push0/push0_contracts.json
```

Run the complete retained corpus with parallel native workers:

```sh
rtk python3 harness/run.py --jobs 8 --quiet zkvm/.fixtures/current-v062-full
```

The same fixture can drive the real RISC-V guest on Spike or the production
ZisK guest:

```sh
rtk python3 harness/run.py --spike --limit 1 --quiet \
  zkvm/.fixtures/current-v062-full/blockchain_tests/for_amsterdam/shanghai/eip3855_push0/push0/push0_contracts.json
rtk env ZISKEMU=/path/to/matching/ziskemu python3 harness/run.py --zisk --limit 1 --quiet \
  zkvm/.fixtures/current-v062-full/blockchain_tests/for_amsterdam/shanghai/eip3855_push0/push0/push0_contracts.json
```

The ELF is built once without fixture input; each embedded
`statelessInputBytes` value is supplied at runtime through `ffi/zkvm_io.h`.
Use `--rebuild` after generated C or FFI changes. A complete v0.6.2 result has
not yet been recorded here; do not reuse pass counts from deleted corpora.
