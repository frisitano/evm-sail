# Model Extractions

This directory contains maintained generated views of the same Sail model used
by the native and zkVM executables.

- `contracts/` records the semantic obligations at the impure host boundary
  and contains the small Sail relations used to check that boundary.
- `coq/` generates the complete model and the small contract relations with
  Sail's Coq backend.
- `lean/` generates and compiles the complete model with Sail's Lean 4 backend.

Generated Coq and Lean sources are written below `coq/` and `lean/` and are
kept in version control. Generated C is an ephemeral build artifact under
`build/c-spec/` or `build/c-optimised/`; it is compile-checked but not retained
as a readable mirror. Lean's compiled `.lake/build/` output remains ignored.
The repository's root `Makefile` owns all backend commands.

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
rtk make c-spec
rtk make c-optimised
rtk make extract-coq
rtk make extract-lean
```

Run both proof extractions with:

```sh
rtk make extract
```

All full-model backends use `sail/evm.sail_project` directly and are generated
by the same custom Sail compiler selected by
`zkvm/resolve_optimized_sail.sh`. Both C targets retain Sail's default name
mangling and enable `--c-specialize`. Only `c-optimised` also requires bounded
integers and loads the ordered modules in `sail/optimised/manifest`, matching
the optimized native and zkVM data representations; `c-spec` retains the
generated GMP-backed ABI and explicit Sail operations. Coq and Lean use the
compiler's standard proof backends and do not load the C-only overrides, so semantic quantity types
remain visible in the proof models: byte and protocol quantities, gas, gas
costs, and gas constants are distinct wrappers over natural numbers, while gas
refunds retain their signed-integer semantics. Upstream Sail is not a supported
alternative extraction path.
