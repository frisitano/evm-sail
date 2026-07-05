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
  `EVM_ENTRY=core|guest|runner|witness_probe|bench` and
  `EVM_BACKEND=spec|build`.
- `sail/runner.sail` is the EEST state-test runner entry file selected with
  `EVM_ENTRY=runner`.
- `sail/host/state.sail` and `sail/host/kernel.sail` define the host world-state
  interface used by EVM execution.
- `sail/lib/mpt.sail` owns account/storage MPT root computation and stateless
  witness traversal.
- `sail/iface/*.sail` holds the abstract `val` contract layer for the impure
  host interface (see `proof/extern-boundary.md`); `sail/spec/*.sail` and
  `sail/build/*.sail` are the backend definitions selected by `EVM_BACKEND`.
- `ffi/` contains native C backends for performance-sensitive host structures:
  memory/calldata/returndata, `state_db.c` for accounts and persistent
  storage cache/update rows, `transient_storage.c`
  for transient storage, code DB, node DB, operand stack, and accelerator shims.
- `revm-eest/run_eest.py` runs EEST state-test fixtures through the project
  runner entry.
- `revm-eest/src/main.rs` is the parallel Rust EEST state-test runner.
- `zkvm/native-runner/run_fixtures.py` runs blockchain/stateless fixtures that
  carry `statelessInputBytes`.

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

Small, checked-in fixture anchors live under `revm-eest/fixtures/`:

- `revm-eest/fixtures/smoke/`
  - `state_root_transfer.json`
  - `state_root_precompile.json`
- `revm-eest/fixtures/eels/shanghai_push0/`
  - Generated from EELS `tests/shanghai/eip3855_push0`.
  - Expected Sail root run: `10/10` passed, `10/10` state-root matches.
- `revm-eest/fixtures/eels/cancun_selfdestruct/`
  - Generated from EELS `tests/cancun/eip6780_selfdestruct/test_selfdestruct.py`.
  - Expected Sail root run: `114/114` passed, `114/114` state-root matches.

The `eels/*` directories preserve generated `state_tests/`,
`blockchain_tests/`, `blockchain_tests_engine/`, and `.meta/` content. The
generated blockchain fixtures in these two subsets do not carry
`statelessInputBytes`; use `zkvm/native-runner/run_fixtures.py` only for
blockchain fixtures that contain those fields.

## EEST Commands

Run from `revm-eest/`:

```sh
rtk python3 run_eest.py fixtures/smoke/state_root_transfer.json fixtures/smoke/state_root_precompile.json --fork Cancun --quiet --timeout 30 --root
rtk python3 run_eest.py fixtures/eels/shanghai_push0/state_tests/for_shanghai --fork Shanghai --quiet --timeout 30 --root
rtk python3 run_eest.py fixtures/eels/cancun_selfdestruct/state_tests/for_cancun --fork Cancun --quiet --timeout 30 --root
```

Use `--rebuild` when generated C or FFI changes need a fresh runner binary.

## zkVM Guest Smoke Gate

EEST-generated stateless fixtures (Amsterdam blockchain_tests carrying
`statelessInputBytes`/`statelessOutputBytes`, filled from execution-specs
`projects/zkevm` @ `02c6c2510916`) live under `zkvm/fixtures/smoke/`. Both
gates diff the guest output byte-exactly against the reference:

```sh
rtk python3 zkvm/native-runner/run_fixtures.py --bin zkvm/native-runner/.build/zkvm_native --quiet zkvm/fixtures/smoke
rtk python3 zkvm/run_guest_smoke.py            # spike guest; full build once, then REBAKE_ONLY per vector
```

Expected: 10 pass each. The hand-rolled `zkvm/vectors/fixture_*.ssz`
(gen_vector.py) are development probes only; their placeholder
parent_hash/receipts_root/block_hash fields do not validate and they are NOT
the smoke gate.

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
  - Rust state runner result: `13917/13917 passed`, `0 timeouts`.

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
revm-eest/fixtures/eels/shanghai_push0/
revm-eest/fixtures/eels/cancun_selfdestruct/
```

Then rerun the EEST commands above and update this file if the expected counts
change.
