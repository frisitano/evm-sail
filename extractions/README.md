# Model Extractions

This directory contains maintained generated views of the same Sail model used
by the native and zkVM executables.

- `contracts/` records the semantic obligations at the impure host boundary
  and contains the small Sail relations used to check that boundary.
- `coq/` generates the complete model and the small contract relations with
  Sail's Coq backend.
- `lean/` generates and compiles the complete model with Sail's Lean 4 backend.
- `python/` contains a complete executable, source-aligned `evm` package, a
  narrow EVM adapter, and a smoke test for the numeric boundary.

Generated Coq, Lean, and Python sources are written below their backend
directories and are kept in version control. Generated C is an ephemeral build
artifact under `build/c-spec/` or `build/c-optimised/`; it is compile-checked
but not retained as a readable mirror. Lean's compiled `.lake/build/` output
and Python bytecode remain ignored. The repository's root `Makefile` owns all
backend commands.

`contracts/ExternBoundary.v`, `contracts/HostAxioms.lean`, and
`contracts/HostContract.py` give the host
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
rtk make extract-python
rtk make python-lint
```

`extract-python` runs the same lint automatically after generation. The check
uses Ruff 0.15.22 with its complete default error families (`E4,E7,E9,F`),
including `F821` for unresolved names in executable annotations. `E741` and
`F841` are explicitly excluded because the review extraction preserves Sail's
source-level variable spelling and intentionally unused local bindings.
Generated runtime,
type, and cross-source dependencies are explicit imports, so a misspelled name
cannot be hidden by wildcard-import ambiguity. Cross-file calls import only
the functions they use from their complete package paths, such as
`from evm.primitives.bytes import bytes_list`; cyclic value and type edges
instead use readable module-qualified access. All imports precede generated
declarations. The Python package requires `ethereum-types==0.4.1` and isolates
that third-party dependency in `evm/_runtime.py`; generated source modules and
the copied `evm/HostContract.py` import representations from the runtime
instead of importing `ethereum_types` directly.

The mandatory numeric model maps Sail `int` to Python `int`, `nat` to `Uint`,
and exact unsigned ranges `0 .. 2**N - 1` to `U8`, `U16`, `U32`, `U64`, or
`U256` for the five supported standard widths. Other constant non-negative
ranges use cached `BoundedUint[lo, hi]` subclasses. Dependent or
runtime-bounded non-negative ranges use `Uint` plus strict Pydantic constraints
that retain erased numeric parameters and enforce relational Sail invariants.
Numeric Sail enums use `UintEnum`; non-numeric enums use Python `Enum`.
Exact-width `bits(N)` values use `Bits`, and configured named byte aliases use
the corresponding `BytesN` class, including `Bytes20` for addresses and
`Bytes32` for hashes.

Generated arithmetic projects nominal unsigned operands to Python integers for
mathematical intermediates and reconstructs the declared numeric type at
result boundaries. `HostContract.py` implements ordinary host operations
directly over an explicit `HostState`; generated extern calls link to those
named functions without a string-keyed dispatch registry. Only the typed
accelerator contract remains abstract. The Pydantic validation layer is
Python-specific; the optimized C extraction remains unchanged. Compile,
annotation-resolution, and behavioral smoke checks remain separate validation
layers.

Or run all maintained extractions with:

```sh
rtk make extract
```

All full-model backends use `sail/evm.sail_project` directly and are generated
by the same custom Sail compiler selected by
`zkvm/resolve_optimized_sail.sh`. `c-spec` retains Sail's default name mangling,
the generated GMP-backed ABI, and explicit Sail operations. `c-optimised` uses
the optimized package/source-tree emitter, fixed-width specialization, unmangled
source names, external handwritten host-slice types, and the ordered modules in
`sail/optimised/manifest`, matching the optimized native and zkVM data
representations. Its generated package stays under the ignored build tree,
separate from `ffi/optimized/`. Its translation units mirror paths below
`sail/` and are consumed through the compiler-written `src/spec/sources.list`;
this does not split the semantic `evm` project module. Coq and Lean use the compiler's standard proof
backends and do not load the C-only overrides, so semantic quantity types
remain visible in the proof models: byte and protocol quantities, gas, gas
costs, and gas constants are distinct wrappers over natural numbers, while gas
refunds retain their signed-integer semantics. Upstream Sail is not a supported
alternative extraction path.

Python likewise uses the compiler's standard typed AST and does not load the
C-only splice. See [`python/README.md`](python/README.md) for the executable
Python package.
