# Model Extractions

This directory contains maintained generated views of the same Sail model used
by the native and zkVM executables.

- `contracts/` records the semantic obligations at the impure host boundary
  and contains the small Sail relations used to check that boundary.
- `c/` contains the optimized C representation of the complete model.
- `coq/` generates the complete model and the small contract relations with
  Sail's Coq backend.
- `lean/` generates and compiles the complete model with Sail's Lean 4 backend.

Generated C, Coq, and Lean sources are written below `c/`, `coq/`, and `lean/`
and are kept in version control. The C tree mirrors active `sail/` source paths
below `c/evm/`; `evm.c` is its unity-build entry point, `evm.h` is its public
generated interface, and `evm_internal.h` supplies shared context when tooling
parses a fragment. Lean's compiled `.lake/build/` output and C generator scratch
remain ignored. The repository's root `Makefile` owns all extraction commands.

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
rtk make extract-c
rtk make extract-coq
rtk make extract-lean
```

Or run all three with:

```sh
rtk make extract
```

All three full-model extractions use `sail/evm.sail_project` directly. The C
target retains Sail's default name mangling, enables `--c-specialize`, and loads
`sail/splices/c_optimized.sail`, matching the optimized native and zkVM data
representations. Coq and Lean do not load the C-only splice, so semantic
quantity types remain visible in the proof models: byte and protocol
quantities, gas, gas costs, and gas constants are distinct wrappers over
natural numbers, while gas refunds retain their signed-integer semantics.

See [`c/README.md`](c/README.md) for the maintained C source layout,
regeneration, and validation contract.
