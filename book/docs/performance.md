---
hide:
  - toc
---

# zkVM benchmarks

This dashboard compares EVM Sail, reth, and ethrex as ZisK guests on the exact
same stateless fixture input. ZisK supplies the zkVM execution and proving-cost
model used by every result on this page. The guest table and grouped bars use a
shared scale for proving cost or instruction steps. Exact ELF function symbols
are shown in a guest-selectable table, so each implementation can be inspected
without combining unrelated symbols into one oversized comparison.

The common ZisK operation-cost table includes every costed operation used by at
least one implementation for the selected input, and aligns operations such as
`keccak`, `sha256`, `add`, and `dma_memcpy` across all three guests. These are
ZisK operations (including accelerator operations and RISC-V-derived machine
operations), not EVM opcodes.

EVM Sail's semantic profile tags add an inclusive protocol-pipeline breakdown.
A guest without the equivalent tag is shown as **not instrumented**, rather
than as a zero-cost implementation. Instruction steps are selected by default;
switch to proving cost when you want ZisK's weighted cost comparison.

The catalog contains one row per fixture file. Individual block profiles are
loaded only after their fixture is selected, and both long fixture and case
lists are capped at 200 visible options until filtered. This keeps the page
responsive when the full retained corpus is published.

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

    Function steps are **exclusive**: each executed instruction belongs to the
    ELF symbol whose address range contains it. The selected guest's function
    rows therefore partition its total steps. If ZisK detects a call-stack
    mismatch, inclusive call attribution is unavailable, but this complete
    exclusive symbol inventory remains valid.

!!! note "Dataset availability"

    The profile JSON is generated from the local retained fixture corpus and is
    intentionally not committed. Documentation builds without generated data
    still succeed and show the command needed to populate the dashboard.
    Public Glamsterdam runs use the linked
    [devnet batch catalog](https://pub-df22334654034ebab51bc096137a59d8.r2.dev/devnets/glamsterdam-devnet-7/index.html),
    whose exact URL is also retained in each generated dashboard catalog and
    benchmark report.
