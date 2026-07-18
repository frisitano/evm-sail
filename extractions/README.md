# Model Extractions

This directory contains the maintained theorem-prover views of the same Sail
model used by the native and zkVM executables.

- `contracts/` records the semantic obligations at the impure host boundary
  and contains the small Sail relations used to check that boundary.
- `coq/` generates the complete model and the small contract relations with
  Sail's Coq backend.
- `lean/` generates and compiles the complete model with Sail's Lean 4 backend.

Generated Coq and Lean sources are written directly below `coq/` and `lean/`
and are kept in version control. Lean's compiled `.lake/build/` output remains
ignored. The repository's root `Makefile` owns all extraction commands:

`contracts/ExternBoundary.v` and `contracts/HostAxioms.lean` give the host
account, persistent-storage, transient-storage, access-warmth, and log externs
one extensional world-state semantics. The contract exposes total logical maps,
transaction-entry originals, transaction snapshots, rollback, transaction
commit, and exact ordered deltas. Sail's opaque `StateCheckpoint` is connected
to a semantic snapshot only by a ghost relation; numeric handles, registries,
cache rows, generations, undo cursors, allocation, physical cleanup, and
copy-on-write rules are backend choices, not specification behavior.

```sh
rtk make check-contracts
rtk make extract-coq
rtk make extract-lean
```

Or run all three with:

```sh
rtk make extract
```

Both full-model extractions use `sail/evm.sail_project` directly. They do not
load the optimized C splice. Consequently the semantic quantity types remain
visible in the extracted model: byte and protocol quantities, gas, gas costs,
and gas constants are distinct wrappers over natural numbers, while gas
refunds retain their signed-integer semantics. C representation choices are
not assumptions of either extraction.
