# Optimized C Model Extraction

`make extract-c` at the repository root generates the maintained complete-model
C artifacts:

```text
extractions/c/
├── README.md
└── evm/
    ├── evm.c                  # implementation unity entry point
    ├── evm.h                  # complete public generated interface
    ├── evm_internal.h         # private unity/editor context
    ├── prelude.c
    ├── primitives/
    │   ├── quantities.c
    │   └── ...
    ├── host/
    │   └── ...
    ├── lib/
    │   └── ...
    ├── executor/
    │   └── ...
    └── main.c
```

Each active `sail/<path>.sail` file has a corresponding
`extractions/c/evm/<path>.c`. The `evm` directory and umbrella basename are
stable: downstream tools compile `evm.c` or include `evm.h` without depending
on a native or zkVM build directory.

Sail's C backend emits one translation unit rather than independent modules.
The source-aligned files are therefore unity fragments, not standalone C
translation units. Their ordinary `.c` suffix makes them visible to editors
and language tooling, but they still depend on umbrella context. When a
fragment is opened or parsed directly, its generated guard includes the private
`evm_internal.h` context so types, static helpers, and generated globals are in
scope. When `evm.c` includes that fragment, the guard exposes only its own
payload because the unity context is already present. Compile only the root
`evm.c`; do not compile the mirrored `.c` files separately. Backend-wide
support definitions, globals, and initialization live in `evm_internal.h`,
while generated function definitions retain project order in the mirrored
fragments. Sail's complete public type and declaration surface remains in the
single root `evm.h`. The generator checks that concatenating its split C
payloads exactly reconstructs the raw Sail C output before writing the
maintained tree.

## Representation Decision

The maintained extraction is the *optimized-model C output*. It loads
`sail/evm.sail_project` with profiling and debug diagnostics disabled, enables
Sail's `-O` and `--c-specialize` passes, retains the backend's default name
mangling, and loads `sail/splices/c_optimized.sail`. Nominal words, addresses,
hashes, and bounded quantities use the same specialized C representations as
optimized native and zkVM builds. Those executable builds still generate and
compile their own copies in isolated build directories.

## Ownership and Regeneration

The tracked extraction owns `evm.c`, public `evm.h`, private
`evm_internal.h`, and all mirrored `.c` fragments below `evm/`. Injected
headers name the impure host contracts, but all hand-written implementations
remain under `ffi/`; neither FFI sources nor a Sail runtime are copied here. The
active source list comes from
`sail/evm.sail_project` with `EVM_PROFILE=off` and `EVM_DEBUG=off`, so stale
fragments are removed when that list changes.

Regenerate and validate the artifacts with:

```sh
rtk make extract-c
```

The target checks that every active Sail source has a C fragment and checks the
generated interface for `main`, transaction execution, state-root, trie-root,
and stateless-input entry points. It also writes a machine-local, ignored
`evm/compile_commands.json` with one entry for the unity source and every
fragment. Clang-based editors discover that database from the fragment's parent
directory and receive the generated model, FFI, zkVM runtime, and local Sail
runtime include paths. The target syntax-checks every fragment independently
with those flags, then compiles the optimized unity source into
`build/extract-c/evm.o`. Raw generator output and marker probes also stay below
`build/extract-c/`. Both that directory and the compilation database are ignored
and `make clean` removes them; `make clean` deliberately preserves the maintained
C sources.

For a focused runtime validation of the same optimized representation, rebuild
one native fixture in optimized mode:

```sh
rtk python3 harness/run.py --build optimized --rebuild \
  zkvm/.fixtures/current-v062-full/blockchain_tests/for_amsterdam/shanghai/eip3855_push0/push0/push0_contracts.json \
  --limit 1 --quiet
```

In a worktree where the execution-specs checkout is not the repository's
sibling, set `EXECSPECS_ROOT` to that checkout before running the fixture.

Use `make extract` to regenerate C together with the maintained Coq and Lean 4
outputs.
