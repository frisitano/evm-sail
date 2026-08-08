---
hide:
  - toc
---

# zkEVM benchmarks

This dashboard compares EVM Sail, reth, and ethrex as zkEVM guests on ZisK,
on the exact same stateless fixture input. ZisK supplies the execution and
proving-cost model used by every result on this page. Each guest panel shows what its ELF
was built from — the repository commit for EVM Sail and the pinned release for
reth and ethrex — as recorded by the benchmark runner; an unrecorded build is
labelled as unknown rather than guessed.

The **Measure** toggle at the top of the dashboard switches the whole page
between ZisK **instruction steps** — the deterministic execution size of a
guest run — and ZisK **proving cost**, the weighted cost the prover charges
for that same run. Every view below it follows the selected measure: the
fixture ranking, the guest comparison table and its bar scale, the per-phase
segments, the semantic-scope table, and all axis labels, column headers, and
tooltips.

The fixture is the only selector. Two always-present rankings order the
fixtures from high to low: one by the largest guest total in the selected
measure and one by block gas used. Selecting a ranking row (or an entry in the
fixture list) loads that fixture's full comparison: guests ranked from most to
fewest in the selected measure on a shared scale, with the other measure
alongside.

When a guest's benchmark measurements carry per-phase attributions
(input decode, witness indexing, execution, state root, and
receipts/commitments), the comparison adds a stacked per-phase breakdown per
guest, in the selected measure. These five phases are the cross-guest
comparison tier: each guest emits them as mutually non-overlapping scopes over
its own pipeline. Guests without phase instrumentation are labelled as such —
absence of a phase is never rendered as zero cost.

The common ZisK operation-cost table includes every costed operation used by at
least one implementation for the selected input, and aligns operations such as
`keccak`, `sha256`, `add`, and `dma_memcpy` across all three guests. These are
ZisK operations (including accelerator operations and RISC-V-derived machine
operations), not EVM opcodes. That table is always proving cost, because ZisK
does not report instruction steps per operation. The semantic-scope table below
it lists every profile tag any guest reported, in the selected measure; a guest
without the equivalent tag is shown as **not instrumented**, rather than as a
zero-cost implementation.

Exact ELF function symbols are listed one guest per tab, so each
implementation can be inspected without combining unrelated symbols into one
oversized comparison. The symbol filter applies within the selected tab.
Executed-symbol attribution is reported in instruction steps only; selecting
proving cost labels that table rather than inventing a per-symbol cost.

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
    total — steps or proving cost, whichever is selected — into input decode,
    witness indexing, execution, state root, and receipts/commitments, with any
    remainder shown as unattributed guest overhead.

    Function steps are **exclusive**: each executed instruction belongs to the
    ELF symbol whose address range contains it. The selected guest tab's
    function rows therefore partition that guest's total steps. If ZisK detects a call-stack
    mismatch, inclusive call attribution is unavailable, but this complete
    exclusive symbol inventory remains valid.

!!! warning "How to read these comparisons"

    All three guests are built and measured **locally**, from pinned sources,
    on the same emulator and the same inputs. That makes the comparison
    internally consistent, but it does **not** make it an authoritative
    statement about how fast reth or ethrex can be as zkVM guests: we build
    them with the configuration we could reproduce, which is not necessarily
    the configuration their authors would choose, and we do not tune them.
    Read these numbers as *this build, this configuration, this workload* —
    useful for locating where cost lives and for tracking our own progress,
    not as a verdict on another project's engineering. The build provenance
    shown under each guest records exactly what was measured; the recipes are
    in `tools/zisk-guests/README.md`, and corrections from the respective
    teams are welcome.

    The instrumented comparison ELFs also carry profiling syscalls, which the
    emulator counts but the ASM prover cannot consume; a provable build omits
    them.

!!! note "Dataset availability"

    The dashboard dataset — the fixture catalog and the per-fixture shards —
    is committed, so this page renders without re-running any benchmark. An
    existing benchmark result can be re-exported without re-running any
    emulator via `tools/benchmark_zisk.py --regenerate-dashboard results.json
    --dashboard-dir …`. Public Glamsterdam runs use the linked
    [devnet batch catalog](https://pub-df22334654034ebab51bc096137a59d8.r2.dev/devnets/glamsterdam-devnet-7/index.html),
    whose exact URL is also retained in each generated dashboard catalog and
    benchmark report.
