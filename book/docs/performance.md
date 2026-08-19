---
hide:
  - toc
---

# zkEVM benchmarks

EVM Sail, reth, and ethrex run as zkEVM guests on ZisK over identical
stateless inputs. The toggle switches every view between **proving cost**
and **instruction steps**; the fixture is the only other selector.

<div
  data-evmsail-performance-dashboard
  data-catalog="assets/generated/performance/catalog.json"
>
  Loading the ZisK profile dashboard…
</div>

!!! warning "How to read these comparisons"

    All three guests are built locally from pinned sources with the
    configuration we could reproduce — not necessarily the one their authors
    would choose, and we do not tune them. Treat the numbers as *this build,
    this workload*: useful for locating cost and tracking our own progress,
    not as a verdict on another project. Build provenance appears under each
    guest; recipes are in `devtools/benchmarks/zisk-guests/README.md`, and corrections are
    welcome.

!!! info "How to read the counters"

    **Phases** are exclusive: they partition a guest's total into input
    decode, witness indexing, execution, state root, and
    receipts/commitments, with any remainder shown as unattributed.
    **Semantic scopes** are inclusive, so sibling counters may overlap.
    **Function symbols** are exclusive and reported in instruction steps
    only, since ZisK attributes proving cost per operation rather than per
    ELF symbol. A missing tag means *not instrumented*, never zero.

    The instrumented comparison ELFs carry profiling syscalls that the
    emulator counts but the prover cannot consume; a provable build omits
    them.

!!! note "Dataset"

    The catalog and per-fixture shards are committed, so this page renders
    without re-running anything. Re-export an existing result with
    `uv run --frozen python -m devtools.benchmarks.zisk --regenerate-dashboard results.json
    --dashboard-dir …`. Fixtures come from the
    [devnet batch catalog](https://pub-df22334654034ebab51bc096137a59d8.r2.dev/devnets/glamsterdam-devnet-7/index.html).
