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
  hand-mirrored: `ffi/journal_glue.c` handles journal entries and structured
  state rows/options; `ffi/hash_glue.c` handles the hash axioms
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
require spliceable type definitions plus the `$[c_repr uint64]` newtype
extension. Set `SAIL` explicitly to test another compiler; the resolver probes
the generated representation before starting the full build. Pure model and
extraction targets continue to use upstream Sail.

Run from repo root unless noted:

```sh
rtk make check
rtk make lint
rtk make fmt-check
```

`make fmt-check` may surface pre-existing formatting drift. Do not format
unrelated files unless that is part of the requested task.

## Stable EEST Fixtures

Small, checked-in fixture anchors live under `harness/fixtures/`:

- `harness/fixtures/smoke/`
  - `state_root_transfer.json`
  - `state_root_precompile.json`
- `harness/fixtures/eels/shanghai_push0/`
  - Generated from EELS `tests/shanghai/eip3855_push0`.
  - Expected Sail run: `10/10` byte-exact passes.
- `harness/fixtures/eels/cancun_selfdestruct/`
  - Generated from EELS `tests/cancun/eip6780_selfdestruct/test_selfdestruct.py`.
  - Expected Sail run: `114/114` byte-exact passes.

The `eels/*` directories preserve generated `state_tests/`,
`blockchain_tests/`, `blockchain_tests_engine/`, and `.meta/` content. The
generated blockchain fixtures in these two subsets do not carry
`statelessInputBytes`; `run.py` runs embedded
`statelessInputBytes` blocks directly when a fixture has them, and otherwise
builds guest inputs from the state-test cases via t8n.

The `smoke/` fixtures are hand-maintained: their tx is signed from the
canonical EEST secret key (sender `0xa94f...bf0b`) because the SSZ input path
derives the sender from the recovered public key -- a fixture with an
unsignable placeholder sender cannot run. Their post `hash` values were
computed with EELS t8n (`state_test` mode).

## EEST Commands

Run from `harness/`:

```sh
rtk python3 run.py fixtures/smoke/state_root_transfer.json fixtures/smoke/state_root_precompile.json --fork Cancun --quiet --timeout 30
rtk python3 run.py fixtures/eels/shanghai_push0/state_tests/for_shanghai --fork Shanghai --quiet --timeout 30
rtk python3 run.py fixtures/eels/cancun_selfdestruct/state_tests/for_cancun --fork Cancun --quiet --timeout 30
```

All runs are byte-exact vs the EELS reference; state tests are materialized as
Amsterdam blocks regardless of the fixture's fill fork. Use `--rebuild` when
generated C or FFI changes need a fresh guest
library.

## zkVM Guest Smoke Gate

Same harness, real RISC-V ELF on spike (`--spike`; run one small state-test
file for the smoke gate):

```sh
rtk python3 harness/run.py --spike harness/fixtures/eels/shanghai_push0/state_tests/for_shanghai/shanghai/eip3855_push0/push0/push0_contracts.json --fork Shanghai --quiet
```

The guest inputs + expected outputs are built per case by the in-process EELS
t8n (`ssz_builder.py` guest mode) -- there is NO checked-in stateless fixture
corpus anymore (`zkvm/fixtures/` deleted; fixtures carrying
`statelessInputBytes`/`statelessOutputBytes` still run directly if pointed at,
e.g. under `zkvm/.fixtures/`). The Spike vehicle pays the full RISC-V model
build on the first case, then runs the unchanged ELF with a new runtime input
for each remaining case. It uses an isolated per-invocation `ZKVM_BUILD`
directory because concurrent compiles sharing `zkvm/build/` race on generated
objects.

Deleted (recover from git history if needed): `sail/bin/runner.sail`,
`zkvm/native-runner/build_runner_lib.sh`, `zkvm/run_guest_smoke.py`, and
`zkvm/native-runner/run_fixtures*.py` (superseded by `run.py`),
`zkvm/vectors/` + `gen_vector.py` (dev probes), `zkvm/ere-guest/` (unbuilt ere
SDK template), `runtime/derisk_main.c` + `runtime/traptest_main.c` (platform
bring-up probes). `build.sh guest` produces an input-agnostic ELF; `VEC` is
accepted only by `build.sh run` and is passed to Spike at runtime.

Full post-Berlin EEST state and EELS/stateless blockchain sweeps require an
external generated fixture corpus; do not claim the full suite has been rerun
unless the external state/blockchain fixture directories were actually run.

## Aligned Full Fixture Corpora

Large unpacked fixture corpora may live under `zkvm/.fixtures/`. This directory
is ignored by git and is workspace-local; do not assume its contents are present
in a fresh checkout.

Current aligned corpora in this worktree were generated from
`/Users/f/dev/ethereum/execution-specs` on branch `projects/zkevm` at commit
`02c6c2510916e470f2c1e5191589212ca75d4948` with `t8n = 2.19.0`:

- `zkvm/.fixtures/current-02c6-full/`
  - Amsterdam blockchain/stateless corpus.
  - Generated 2026-07-01 with `fill -m "blockchain_test or blockchain_test_engine" --fork Amsterdam ./tests/`.
  - Fixture generation result: `43111 passed, 14 skipped`.
  - `evm_sail_consumer` validation result: `20442 passed, 1104 skipped`.
  - Current native `harness/run.py` result over the 23,266 blocks carrying
    embedded `statelessInputBytes`/`statelessOutputBytes`: `23266/23266`
    byte-exact in both standard and optimized builds (8 jobs). This count does
    not include generated blocks without an embedded stateless pair.
- `zkvm/.fixtures/current-state-02c6-full/`
  - Amsterdam state-test corpus.
  - Generated 2026-07-02 with `fill -m state_test --fork Amsterdam ./tests/`.
  - Fixture generation result: `13917 passed, 6 skipped`.
  - The current unpack at this path contains only 14 selected fixture shards
    yielding 964 executable cases; despite the directory name, do not report it
    as the original full 13917-case state corpus.
- `zkvm/.fixtures/current-state-02c6-berlin-amsterdam/`
  - Historical state-test corpus covering the mainline forks from Berlin
    through Amsterdam.
  - The generated tests are stored as pytest-worker shards named
    `*.partial.gwN.jsonl`. Here `partial` means that the file contains one
    partition of the full fixture output, not that its fixtures are incomplete:
    every JSONL record contains one complete generated state-test fixture
    (`k` is the pytest node ID and `v` is the fixture JSON).
  - Reuse these shards for historical `harness/run.py` sweeps. Do not regenerate
    or discard them merely because their names contain `partial`.
  - Current native result across the eight mainline fork directories:
    `6422/6422` byte-exact in both standard and optimized builds (8 jobs).

Do not use `zkvm/.fixtures/fixtures/` as aligned proof unless explicitly
requested. That unpack is older metadata:
`refs/tags/tests-zkevm@v0.4.1`, commit
`b6b764ff21bb754b79e11ef5dc7ad1f79996e923`, generated 2026-05-18.

## Regenerating The Checked-In EELS Subsets

The local EELS checkout used in this environment is
`/Users/f/dev/ethereum/execution-specs`. From that repo:

```sh
rtk uv run fill --output /private/tmp/evm-sail-eest-push0-fixtures --clean --no-html --skip-index --fork Shanghai tests/shanghai/eip3855_push0
rtk uv run fill --output /private/tmp/evm-sail-eest-selfdestruct-fixtures --clean --no-html --skip-index --fork Cancun tests/cancun/eip6780_selfdestruct/test_selfdestruct.py
```

After regeneration, copy the outputs back to:

```text
harness/fixtures/eels/shanghai_push0/
harness/fixtures/eels/cancun_selfdestruct/
```

Then rerun the EEST commands above and update this file if the expected counts
change.
