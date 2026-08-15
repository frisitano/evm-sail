# Developer tools

All handwritten Python and repository developer utilities live in this tree.
Run Python entry points from the repository root with module syntax so imports
and shared path handling are consistent:

```sh
python3 -m devtools.harness.cli --help
python3 -m devtools.benchmarks.zisk --help
python3 -m devtools.optimised_c.check --help
```

- `harness/` owns fixture discovery, SSZ construction, guest adapters, and the
  native/Lean/Python/Spike/ZisK execution CLI. Stateful execution backends
  implement the shared `GuestBackend` protocol; fixture transforms remain pure
  functions.
- `benchmarks/` owns ZisK comparison, guest staging, and the machine-local guest
  directory.
- `optimised_c/` owns optimized-C build metadata, conformance, formatting,
  linting, packaging, and quality evaluation.
- `docs/` owns documentation checks and Sail readability summaries.
- `tests/` is the pytest suite for handwritten tooling.

The root `Makefile` exposes the supported quality gates. `python-tools-check`
runs Ruff lint and format checks, strict mypy, and pytest. Fixture execution is
kept separate as `python-tools-fixture-smoke` because it builds and runs the
model against the retained v0.6.2 corpus.
