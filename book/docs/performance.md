---
hide:
  - toc
---

# zkVM benchmarks

This dashboard compares EVM Sail, reth, and ethrex as ZisK guests on the exact
same stateless fixture input. ZisK supplies the zkVM execution and proving-cost
model used by every result on this page. Each guest panel shows what its ELF
was built from — the repository commit for EVM Sail and the pinned release for
reth and ethrex — as recorded by the benchmark runner; an unrecorded build is
labelled as unknown rather than guessed.

The fixture is the only selector. Two always-present rankings order the
fixtures from high to low: one by the largest guest instruction-step count and
one by block gas used. Selecting a ranking row (or an entry in the fixture
list) loads that fixture's full comparison: guests ranked from most to fewest
instruction steps on a shared scale, with proving cost alongside.

When a guest's benchmark measurements carry per-phase step attributions
(input decode, witness indexing, execution, state root, and
receipts/commitments), the comparison adds a stacked per-phase breakdown per
guest. These five phases are the cross-guest comparison tier: each guest
emits them as mutually non-overlapping scopes over its own pipeline. Guests
without phase instrumentation are labelled as such — absence of a phase is
never rendered as zero cost.

The common ZisK operation-cost table includes every costed operation used by at
least one implementation for the selected input, and aligns operations such as
`keccak`, `sha256`, `add`, and `dma_memcpy` across all three guests. These are
ZisK operations (including accelerator operations and RISC-V-derived machine
operations), not EVM opcodes. EVM Sail's semantic profile tags add an
inclusive protocol-pipeline breakdown; a guest without the equivalent tag is
shown as **not instrumented**, rather than as a zero-cost implementation.
Exact ELF function symbols are listed per guest, so each implementation can be
inspected without combining unrelated symbols into one oversized comparison.

The catalog contains one row per fixture file. Individual block profiles are
loaded only after their fixture is selected, and long fixture lists are capped
at 200 visible options until filtered. This keeps the page responsive when the
full retained corpus is published.

<div
  data-evmsail-performance-dashboard
  data-catalog="assets/generated/performance/catalog.json"
>
  Loading the ZisK profile dashboard…
</div>

!!! info "How to read the counters"

    Semantic scope totals are **inclusive**. A transaction frame can contain
    account loads, storage loads, hashing, and mutations, so adding sibling or
    cross-cutting counters can double-count the same proving cost or guest
    steps. Comparison bars within a row share a scale; a missing profile tag is
    not interpreted as zero.

    Per-phase segments are **exclusive**: they partition a guest's measured
    steps into input decode, witness indexing, execution, state root, and
    receipts/commitments, with any remainder shown as unattributed guest
    overhead.

    Function steps are **exclusive**: each executed instruction belongs to the
    ELF symbol whose address range contains it. The selected guest's function
    rows therefore partition its total steps. If ZisK detects a call-stack
    mismatch, inclusive call attribution is unavailable, but this complete
    exclusive symbol inventory remains valid.

!!! note "Dataset availability"

    The profile JSON is generated from the local retained fixture corpus and is
    intentionally not committed. Documentation builds without generated data
    still succeed and show the command needed to populate the dashboard. An
    existing benchmark result can be re-exported without re-running any
    emulator via `tools/benchmark_zisk.py --regenerate-dashboard results.json
    --dashboard-dir …`. Public Glamsterdam runs use the linked
    [devnet batch catalog](https://pub-df22334654034ebab51bc096137a59d8.r2.dev/devnets/glamsterdam-devnet-7/index.html),
    whose exact URL is also retained in each generated dashboard catalog and
    benchmark report.
