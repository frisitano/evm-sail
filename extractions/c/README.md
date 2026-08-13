# C backend ownership

The generated C model links exactly one complete host backend:

- `spec/` implements the readable specification build against Sail's generated
  GMP-backed C ABI.
- `optimised/` implements the production fixed-layout ABI and owns all
  optimized-only whole-operation replacements and pointer-based storage.

The optimized backend uses the conventional split while mirroring the Sail
source tree:

- `optimised/include/evmsail/` contains public declarations injected into the
  generated model.
- `optimised/src/` contains implementations and private headers.
- `optimised/src/host/state.c` implements the generated aggregate boundary;
  `optimised/src/host/state/` is split by semantic ownership:
  `account.c` owns account schema, values, logs, and authentication metadata;
  `storage.c` owns the corresponding persistent-storage state;
  `block_access_list.c` owns BAL history and iteration; and `store.c` contains
  only the lifecycle ordering that coordinates those independent modules.
- `optimised/src/host/state/logs.c` owns emitted log records and the cumulative
  block bloom. `optimised/src/executor/receipts.c` owns retained encoded
  receipts; the generic MPT implementation consumes their spans but does not
  own receipt storage.

Large optimized-backend storage is owned by one pre-provisioned workspace.
`optimised/src/workspace.c` binds a typed pointer for each module exactly once
before generated-model initialization; modules then use ordinary direct
indexing such as `entries[id]`. A module must not define its own capacity-sized
backing array or allocate, resize, or free storage. Native tests provide one
zero-filled backing object, Spike maps a dedicated workspace region, and ZisK
obtains the equivalent region from its platform allocator during startup.
`workspace_claim` only partitions that stable region; it is not a general heap
and is never called from an execution hot path.

Do not add flat optimized-root implementation files or an umbrella header.
Split a responsibility at a real Sail module boundary and place it under the
matching `include/evmsail/` or `src/` path.

The two directories are mutually exclusive implementations of the same Sail
host contracts. A source or header in one backend must not include a private
source or header from the other backend. Builds put the selected backend first
on the include path and compile implementation files only from that directory.
`EVMSAIL_MODEL_H` selects the generated model umbrella header. The optimized
host prelude then gives the selected fixed representations their
semantic names (`Address`, `Hash32`, `LogsBloom`, and `U256`). Raw generated
representation names must not leak into subsystem headers or implementations.
This is a naming boundary only: the aliases preserve the exact generated ABI
and add no wrapper calls, allocation, conversion, or ownership layer.

Only standardized platform contracts live at this directory's root:

- `zkvm_accelerators.h`
- `zkvm_io.h`

Shared protocol behavior belongs in Sail. Shared platform behavior belongs
behind one of the standardized headers above. Do not introduce a shared C
adapter layer merely to make the two backend implementations look alike: their
representations and useful optimization boundaries are intentionally allowed
to diverge.

## Optimized extraction review metadata

`make c-optimised-compdb` writes the ignored root `compile_commands.json` from
the generated and optimized-FFI source manifests, then checks exact membership
and runs one representative `clang -fsyntax-only` command. Entries for the
handwritten backend always point to `optimised/contract/src`, not to staged
package copies, so clangd diagnostics lead to editable files.

`make c-optimised-evaluate` writes a machine-readable
`build/extraction-quality/record.json` using schema
`evm-sail-extraction-quality/v1` and a concise Markdown summary beside it. The
record pins both repository commits, the compiler source commit and binary
hash, the extraction recipe and manifest, artifact hashes, objective metrics,
gate outcomes, and explicit reasons for every skipped higher-cost gate. A red
gate remains a recorded result; callers can pass `--require-pass` directly to
the evaluator when a blocking experiment or release must be green.

For a review record, identify both sides of the compiler provenance explicitly:

```sh
make c-optimised-evaluate \
  SAIL=/absolute/path/to/the/verified/sail \
  SAIL_SOURCE=/absolute/path/to/its/source-checkout
```

`SAIL_SOURCE` has no machine-specific default. When omitted, the evaluator may
discover an installed compiler's source checkout, but review and publication
runs should set it so the recorded source commit is reproducible.
