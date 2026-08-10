# Lean Extraction

`make extract-lean` at the repository root generates the complete Lean 4 model
into `src/`; `make build-lean` compiles it with `lake`. The generated Lean
sources are kept in version control; only `src/.lake/` is ignored.

`contract/` holds the hand-maintained Lean side of the impure `val` boundary:
`HostAxioms.lean` mirrors the host contract layer and `Specialization.lean`
pins the representations the backend specializes. `make check-contracts`
enforces their invariants.

Generation requires Sail's Lean support library as a Lake package under
`src/.lake/packages/Sail`. `extract-lean` clones it at the revision the backend
expects when it is absent; `LEAN_SAIL_LIB` and `LEAN_SAIL_LIB_REQUIRE` point
generation at a checkout elsewhere.

`make lean-harness` builds `runner/` into the shared library behind the same
`test_utils.c` ABI the native fixture harness uses, so the extraction runs the
retained corpus byte-exact against the reference output:

```sh
python3 harness/run.py --lean --jobs 8 --quiet zkvm/.fixtures/current-v062-full
```

`runner/Runner.lean` also owns what `fatal_error` owns in C: the extracted
`fatal_error` is the canonical non-returning body, so a rejected block's public
output is published by the runner rather than by the model.

The `run` executable that `--lean-executable` declares is not part of either
target. It has no default target and cannot link on its own, because the
accelerator externs are satisfied by `ffi/accelerators.c`, which only
`lean-harness` links.
