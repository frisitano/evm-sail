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

- `sail/evm.sail_project` is the model project file. Use the `evm` module with
  `EVM_ENTRY=core|guest|runner`. There is no backend variable: the Sail
  specification is single-configuration.
- `sail/bin/` holds the executable entry files (`bin/main.sail` the stateless
  guest, `bin/runner.sail` the EEST state-test runner), selected with
  `EVM_ENTRY=guest|runner`.
- `sail/host/state.sail` and `sail/host/kernel.sail` define the host world-state
  interface used by EVM execution.
- `sail/lib/mpt.sail` is the single trie implementation: one public entry
  `trie_root(base_root, updates)` (witness-native overlay, fail-closed on
  missing node material) plus account/storage MPT root computation and
  stateless witness traversal. The Yellow Paper Appendix C/D equations are
  kept as documentation on the internal functions; an empty base computes
  TRIE(I) directly.
- The impure host interface is an abstract `val` contract layer declared inline
  in the module files themselves as `val X = impure { c: "sym" } : T`
  (`host/io.sail` crypto/oracle, `host/state.sail` / `host/memory.sail` world
  state and buffers, `lib/mpt.sail` trie node refs, `lib/rlp.sail`
  create_address; see `proof/extern-boundary.md`). These `c:`-bound vals are the
  TRUE axioms (crypto core, I/O oracle, mutable host stores) -- proof targets
  see them as bodyless parameters; executables link their C definitions from
  `ffi/`. The boundary is scalar-typed with ONE exception: the journal
  (`journal_push : JEntry -> unit` / `journal_pop : unit -> JEntry`) crosses
  whole JEntry union values; `ffi/journal_glue.c` compiles per build against
  the GENERATED model header (`-DEVMSAIL_MODEL_H`, `-I` build dir) so the
  `struct zJEntry` layout is never hand-mirrored, and (en/de)codes against the
  scalar journal rows in `ffi/kernel_state.c`. Everything else, including the
  former C fast-path hooks
  (`keccak256_word`, `keccak256_address`, `sha256_pair`, `sha256_digests3`,
  `ssz_src_le`, `ssz_src_be`), is a pure Sail body compiled and executed
  directly; the dormant one-call C versions in `ffi/host_crypto.c` and the I/O
  shims are candidates for future link-time override (weak-stub mechanism, not
  yet wired). The old `sail/c/*.sail` extern-binding menu and the
  `EVM_BACKEND=spec|build` project variable were deleted (this change).
- `ffi/` contains native C backends for performance-sensitive host structures:
  memory/calldata/returndata, `state_db.c` for accounts and persistent
  storage cache/update rows, `transient_storage.c`
  for transient storage, code DB, node DB, operand stack, and accelerator shims.
- `harness/run.py` is the SINGLE fixture harness for both executable
  entries, built ONCE as shared libraries and driven IN-PROCESS via ctypes
  (`harness/dump_state.py`). Each case is serialized to the SSZ
  `SszStatelessInput` (`ssz_builder.py`, under the execution-specs venv).
  - Default (runner, `EVM_ENTRY=runner`, `build_runner_lib.sh` ->
    `libevmsail_runner.dylib`): cross-fork state-test execution. The runner
    emits NO byte stream: the harness reads the result C-side from the
    `evmsail_dump_snapshot` 'G' section -- the per-entry `zkvm_out_gas`
    register, the post-state root (the dump calls the model's own
    `compute_state_root`, `--c-preserve`d in the runner build), and any Sail
    exception that escaped the run (the runner catches nothing; the dump
    captures `have_exception` + the InvalidBlock BlockError variant +
    `throw_location`, e.g. `InvalidBlock(WitnessDeficient) @
    sail/lib/mpt.sail:1275`). The root IS the pass criterion. `--dump` prints
    the model's live post-run state (write-set accounts+storage, stack,
    memory) from the same snapshot.
  - `--guest` (`EVM_ENTRY=guest`, `build_lib.sh` -> `libevmsail_guest.dylib`):
    the Amsterdam stateless full-block validator, gated BYTE-EXACT against the
    EELS reference. State-test cases are executed through the in-process EELS
    t8n (ssz_builder guest mode), which builds a fully VALID single-tx block
    input AND the reference `run_stateless_guest` output bytes; fixtures that
    already carry `statelessInputBytes`/`statelessOutputBytes` are fed
    directly. `--spike` swaps the execution vehicle for the REAL RISC-V guest
    ELF on spike (zkvm/build.sh; baked vector, REBAKE_ONLY per case). This
    subsumes the old `zkvm/native-runner/run_fixtures*.py` and
    `zkvm/run_guest_smoke.py` guest runners (deleted).
  This is the sole fixture runner; the parallel Rust runner was removed. The
  old `runner_ffi.c` (stdin ssz_src) is deleted, and the driver/test
  world-wipe lives in `test_utils.c` `evmsail_clear_memory` (the model no
  longer defines `k_world_reset`). The dedicated `witness_probe` re-root
  harness was also removed: every stateless account/storage lookup walks the
  witness trie from the authenticated root (parsing each node on the path), so
  the runner's full-corpus post-state-root checks already exercise + validate
  the witness node reader (`lib/mpt.sail`).

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
  - Expected Sail root run: `10/10` passed, `10/10` state-root matches.
- `harness/fixtures/eels/cancun_selfdestruct/`
  - Generated from EELS `tests/cancun/eip6780_selfdestruct/test_selfdestruct.py`.
  - Expected Sail root run: `114/114` passed, `114/114` state-root matches.

The `eels/*` directories preserve generated `state_tests/`,
`blockchain_tests/`, `blockchain_tests_engine/`, and `.meta/` content. The
generated blockchain fixtures in these two subsets do not carry
`statelessInputBytes`; `run.py --guest` runs embedded
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

Guest (byte-exact vs the EELS reference; state tests are executed as Amsterdam
blocks regardless of the fixture's fill fork):

```sh
rtk python3 run.py --guest fixtures/eels/shanghai_push0/state_tests/for_shanghai --fork Shanghai --quiet
rtk python3 run.py --guest fixtures/eels/cancun_selfdestruct/state_tests/for_cancun --fork Cancun --quiet
```

Use `--rebuild` when generated C or FFI changes need a fresh runner/guest
library.

## zkVM Guest Smoke Gate

Same harness, real RISC-V ELF on spike (`--spike`; run one small state-test
file, each case = rebake + relink + spike boot, so keep it smoke-scale):

```sh
rtk python3 harness/run.py --guest --spike harness/fixtures/eels/shanghai_push0/state_tests/for_shanghai/shanghai/eip3855_push0/push0/push0_contracts.json --fork Shanghai --quiet
```

The guest inputs + expected outputs are built per case by the in-process EELS
t8n (`ssz_builder.py` guest mode) -- there is NO checked-in stateless fixture
corpus anymore (`zkvm/fixtures/` deleted; fixtures carrying
`statelessInputBytes`/`statelessOutputBytes` still run directly if pointed at,
e.g. under `zkvm/.fixtures/`). The spike vehicle pays the full RISC-V model
build on the first case, then REBAKE_ONLY per case, in an isolated
per-invocation ZKVM_BUILD dir (concurrent gates sharing `zkvm/build/` race on
the baked vector/ELF).

Deleted (recover from git history if needed): `zkvm/run_guest_smoke.py` and
`zkvm/native-runner/run_fixtures*.py` (superseded by run.py --guest),
`zkvm/vectors/` + `gen_vector.py` (dev probes), `zkvm/ere-guest/` (unbuilt ere
SDK template), `runtime/derisk_main.c` + `runtime/traptest_main.c` (platform
bring-up probes). build.sh's `VEC` is required (no default vector).

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
- `zkvm/.fixtures/current-state-02c6-full/`
  - Amsterdam state-test corpus.
  - Generated 2026-07-02 with `fill -m state_test --fork Amsterdam ./tests/`.
  - Fixture generation result: `13917 passed, 6 skipped`.
  - State runner result: `13917/13917 passed`, `0 timeouts` (run.py).

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
