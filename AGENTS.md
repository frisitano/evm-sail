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
  contains the full stateless validator. `EVM_PROFILE=on|off` controls optional
  cycle scopes. `EVM_DEBUG=on|off` controls native-test validation diagnostics;
  real zkVM guest builds set it to `off`.
- `sail/host/state.sail` is the declaration-only host world-state surface for
  impure FFI contracts. `sail/host/environment.sail` declares the fixed-size,
  O(1) ancestor-hash host table. `sail/host/kernel/environment.sail` owns the
  kernel registers, and the remaining `sail/host/kernel/` modules group pure
  Sail `k_*` operations by subsystem.
- `sail/executor/payload.sail` owns the transaction and withdrawal MPT roots
  used to reconstruct and validate the execution payload's block hash.
- `sail/lib/mpt/` is the generic trie implementation, split by dependency
  layer into paths/hex-prefix primitives, node encoding and decoding, ordered
  updates/canonical rebuilding, and authenticated traversal. Its public root
  entry is `trie_root(base_root, updates)` (witness-native overlay, fail-closed
  on missing node material). `sail/lib/state_trie.sail`
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
  `ffi/`. Generated aggregate values cross through three hand-written glue
  translation units compiled per build against the GENERATED model header
  (`-DEVMSAIL_MODEL_H`, `-I` build dir), so generated layouts are never
  hand-mirrored: `ffi/journal_glue.c` handles structured account/storage
  rows and options (the rollback journal itself is C-private);
  `ffi/hash_glue.c` handles the hash axioms
  (`keccak256_segments` / `sha256_segments : list(Bytes) -> hash`), segmented
  byte equality, and log records; and `ffi/code_glue.c` constructs aggregate
  `option(Code)` lookup results. Sail emits fixed 256-bit JUMPDEST chunks
  directly into one length-preallocated packed C table, so no generated list
  crosses that boundary. A
  hash preimage is a list of Bytes segments -- materialized bytes or
  source-tagged slices -- crossing in ONE call; the old streaming hash channel
  and fused source hashers are gone. Everything else,
  including the former C fast-path hooks (`keccak256_word`,
  `keccak256_address`, `sha256_pair`), is a pure Sail body compiled and
  executed directly. SSZ decoding reads only from the single input
  `ByteSlice`; the old byte-at-a-time `ssz_src_*` oracle no longer exists. The
  dormant one-call C versions in
  `ffi/host_crypto.c` (shape-matching ones only) are candidates for future
  link-time override (weak-stub mechanism, not yet wired). The old `sail/c/*.sail` extern-binding menu and the
  `EVM_BACKEND=spec|build` project variable were deleted (this change).
- `ffi/` contains native C backends for performance-sensitive host structures:
  memory/generic byte slices/output storage, the Sail-cursor-owned executor
  scratch arena, `state_db.c` for accounts and
  persistent storage cache/update rows, `transient_storage.c` for transient
  storage, the content-addressed code and packed JUMPDEST arenas, node DB,
  operand stack, and accelerator shims.
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
  `--debug` invokes the
  native-only `evmsail_debug_dump` after a failure; it is not linked into the
  real guest. `--profile` enables optional cycle-scope markers.
  This is the sole fixture runner; the parallel Rust runner was removed. The
  old `runner_ffi.c` (stdin ssz_src) is deleted, and the driver/test
  world-wipe lives in `test_utils.c` `evmsail_clear_memory` (the model no
  longer defines `k_world_reset`). The dedicated `witness_probe` re-root
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

Optimized native and RISC-V C builds run `zkvm/resolve_optimized_sail.sh` and
require spliceable type definitions plus bound-driven C specialization. The
model never carries C-representation annotations: the backend selects native
representations from the semantic type bounds. Set `SAIL` explicitly to test
another compiler. Pure model and extraction targets continue to use upstream
Sail.

Run from repo root unless noted:

```sh
rtk make check
rtk make lint
rtk make fmt-check
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
