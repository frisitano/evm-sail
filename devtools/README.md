# Developer tools

All handwritten Python and repository developer utilities live in this tree.
They require Python 3.12.x exactly; `pyproject.toml` declares the Python and
dependency versions, while `uv.lock` fixes the environment. `uv` installs its
managed Python automatically. Other Python minor versions are not supported.
Run Python entry points from the repository root with module syntax so imports
and shared path handling are consistent:

```sh
uv run --frozen python -m devtools.harness.cli --help
uv run --frozen python -m devtools.benchmarks.zisk --help
uv run --frozen python -m devtools.optimised_c.check --help
```

- `harness/` owns fixture discovery, SSZ construction, guest adapters, and the
  native/Lean/Python/Spike/ZisK execution CLI. Stateful execution backends
  implement the shared `GuestBackend` protocol; fixture transforms remain pure
  functions.
- `benchmarks/` owns ZisK comparison, guest staging, and the machine-local guest
  directory.
- `optimised_c/` owns optimized-C build metadata, conformance, formatting,
  linting, packaging, and quality evaluation.
- `rocq.py` normalizes colliding binder names in generated dependent tuple
  patterns before the staged Rocq tree is checked or published.
- `python_extraction.py` localizes the state-trie callback imports emitted into
  the generic MPT reducer, avoiding a split-module import cycle.
- `docs/` owns documentation checks and Sail readability summaries.
- `tests/` is the pytest suite for handwritten tooling.

The root `Makefile` exposes the supported quality gates. `python-tools-check`
runs Ruff lint and format checks, strict mypy, and pytest. Fixture execution is
kept separate as `python-tools-fixture-smoke` because it builds and runs the
model against the retained v0.6.2 corpus.
