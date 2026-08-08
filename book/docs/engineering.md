# Performance engineering notes

This page records the performance model, the compiler machinery, and the
empirical observations behind EVM Sail's guest performance — including the
experiments that failed, since several of the most useful results here are
refutations. Everything below was measured on real devnet blocks with
byte-exact output gating; the companion [zkVM benchmarks](performance.md)
page carries the current numbers.

## The cost model comes first

A ZisK guest is measured along **two axes**, and they are not the same
number. The first is **retired RISC-V instructions** — "steps" — which is
what the emulator counts and what most of this page optimizes. The second
is **proving cost**, the prover's own weighting of the trace: operations
are not uniform there, because accelerated operations consume rows in
dedicated tables (the arithmetic and hashing precompiles) rather than
ordinary execution rows, and the resources a proof consumes are a
weighted sum rather than a raw instruction count. The
[benchmarks page](performance.md) exposes both, and a change can move
them in different directions — routing an operation to a precompile can
cut its steps by an order of magnitude while adding table area, which is
why accelerator decisions are argued on both axes rather than on steps
alone.

Within the step axis, the model is unusually simple: there are no caches
to warm, no branch predictor to please, no latency to hide, and no
penalty for code size — every executed instruction costs one step. That
inverts several classical intuitions:

- A link-time-constant address is the cheapest object in the model — it can
  be rematerialized anywhere (`auipc`/`addi`) and never needs to be kept
  alive or spilled.
- Memory traffic is pure cost. A load is a step regardless of locality;
  "cache-friendly layout" buys nothing, while *eliminating* an access buys
  its full price.
- Dispatch overhead is arithmetic, not prediction. A computed `goto` costs
  its index load and jump; there is no misprediction penalty to amortize,
  so classic threaded-code arguments must be re-derived from instruction
  counts alone (they mostly still hold, for smaller margins).
- Big straight-line code is free. Inlining, unrolled tables, and per-opcode
  specialization never pay an instruction-cache tax.

Most observations on this page reduce to one discipline: **count the
instructions on the hot path, and be suspicious of any mechanism whose
cost is an assumption** — while remembering that the step count is one of
two prices, and that anything touching the accelerated operations must be
read against proving cost as well.

## Type-driven lowering: semantic types become machine types

The custom Sail compiler's central feature is that **the specification's
semantic types drive the machine representation**. The base model is
written against mathematically honest types — `range(0, 1024)` stack
heights, `int('n)` singleton widths, dependent results, finite enums —
and the C backend's specialization pass (`--c-specialize`) performs
interval and bound analysis over them:

- A `range`-typed value whose bounds fit a machine word lowers to `uint16_t`
  / `uint32_t` / `uint64_t` rather than a GMP integer. Arithmetic on it
  lowers to native operations accompanied by *semantic proofs* — the
  compiler discharges, per operation, that the result interval fits the
  chosen representation (visible in build logs as
  `__sail_proven_native_add … intervals=[0..1024,1..1]`).
- Representation demands propagate **transitively through the call graph**:
  when a caller's narrowed type reaches a callee, the callee is cloned at
  the narrowed representation, including its locals and intermediate
  values, with whole-body lifetime analysis covering every mutable local.
- The doctrine is to express the *real protocol invariant* and let the
  lowering follow — never to add an artificial cap solely to obtain a
  narrower type. An unbounded mutable `nat` does not become bounded because
  its initializer is small; if a value is genuinely bounded, the bound
  belongs in its type, where the specification, the prover, and the
  compiler all see the same fact.
- Fork-dependent bounds are threaded as explicit `ProtocolProfile`
  arguments rather than read ambiently, so the finite profile combinations
  specialize through arguments, results, and body intermediates.

This is the same bargain [Ho, Fromherz, and Protzenko](https://doi.org/10.1145/3607844)
describe for verified systems code: abstractions that a verifier reasons
about at full generality, and that specialization erases before code
generation, so modularity in the specification costs nothing in the
binary. Our version is driven by the semantic types rather than by
explicit staging annotations.

Two-representation types extend this to structures: canonical 20-byte
addresses and 32-byte digests keep byte-index semantics in Sail and in
proof extraction, while optimized C represents them as `u64` lanes
(`fixed_bytes_u64_lanes`). The operand-stack cursor (`StackTop`) is the
same pattern applied to custody: an opaque 64-bit token that the spec-C
backend implements as a frame height and the optimized backend as a raw
row pointer — both passed by value, neither visible to the specification.

## Limbs, lanes, and when hardware wins

The EVM word is four little-endian `u64` limbs (`c_repr` fixed layout),
chosen so that host ABIs, accelerator ABIs, and the model agree without
conversion. Observations that survived measurement:

- **Schoolbook beats syscalls for cheap ops.** A four-limb ADD is ~20
  instructions on a load-store ISA (eight loads, carry chain, four stores —
  the irreducible floor), below the fixed cost of any accelerator call.
  reth's guest reaches the same conclusion: its interpreter arithmetic is
  stock software.
- **Syscalls win where software is catastrophic.** 256-bit division
  (schoolbook: 1,000–4,000 steps) routes through the ZisK division fcall
  plus `ARITH256` verification (~100–200 steps); `MULMOD`/`ADDMOD` route
  through the proven `ARITH256_MOD` precompile and skip verification code
  entirely. These are genuine advantages over guests that keep software
  arithmetic.
- **Wide accumulators**: multiplication uses `u128` accumulation in C;
  exponentiation uses a bounded square-and-multiply ladder driven by the
  exponent's bit length.
- The proving-side economics matter too: every precompile row consumes
  arithmetic-table area, so ops are routed to hardware only when the step
  savings dominate on *both* sides of the prover.

## Registers, custody, and why code structure decides

The guest builds with `-mcmodel=medany -mno-relax`, so each distinct global
referenced by a function pays an `auipc`-based address materialization.
This made register/state placement the campaign's central question, and the
answers were bought with A/B measurements:

- **Merged register file** (every model register a member of one struct,
  hot members at the smallest offsets): a modest, real win — one base
  materialization amortized across all register accesses in a function.
- **Threading the file's base pointer through calls**: *refuted, +7%.* A
  constant base is rematerializable for free; a threaded pointer is a live
  value paying a move (and often a spill) at **every call edge**, and call
  edges vastly outnumber function entries. The cost model punishes
  per-edge overhead far more than per-entry overhead.
- **Pinning registers globally** (`register uint64_t … asm("s10")`):
  *refuted twice.* It measured slightly negative even in its best case —
  reserving allocatable registers program-wide costs more than hot-cell
  access saves — and GCC rejects global register variables under LTO
  entirely. Manual placement also forfeits the allocator's freedom, which
  turned out to be worth more than the placement.
- **Carried values** (the state-passing convention): the winner, ‑5.25% on
  its landing day. Hot state — pc, gas, the stack cursor, the memory
  cursor — flows through handler signatures as values and returns.
  A value has **no address**: nothing can alias it, so the compiler
  promotes it to machine registers *by construction*, on every toolchain,
  with no analysis to trust. Every alternative (ambient globals, unity
  builds, whole-program LTO, raised inline budgets, refs) asked alias
  analysis to prove facts it structurally could not — because the halt
  path really did write the gas cell — and each plateaued within noise of
  the others. The migration did not inform the compiler better; it
  **changed the facts**, moving the writes into the dataflow until there
  was nothing left to prove.

The resulting custody taxonomy is the page's most reusable artifact:

| state class | mechanism | rationale |
| --- | --- | --- |
| touched every step (pc, gas, cursors) | carried values | promotion by construction |
| immutable within a scope (frame code, message, profile) | registers | scoped constants; ambient reads of non-varying state are honest |
| rare-write effectful counters | registers / explicit params | below the tuple-rent threshold |
| platform storage (stack rows, arena, tries) | host axioms | C-owned, spec sees typed operations |
| link-time constants (dispatch tables, costs) | statics | the compiler proves everything alone |

The rule that generates the table: **mechanism follows access frequency**.
Carried state must earn its tuple slot per step; register state must be
constant-like within its scope; anything else is paying rent it cannot
cover.

In register-allocation terms, the carried convention manufactures clean
**webs** — the flow-directed connected components of definitions and uses
that [Quiring, Van Horn, Reppy, and Shivers](https://doi.org/10.1145/3729280)
formalize as a well-typedness-preserving transformation domain. Each
carried value is a short, call-free web the allocator colors trivially
(Chaitin 1982; Briggs et al. 1994), whereas memory-resident state
fragments every web at each potential clobber, which is precisely why the
alias-analysis route kept plateauing. SSA-based reasoning (Cytron et al.
1991) sees the carried loop exactly as written — which is also why the
same signatures extract to pure functions for the proof targets.

## Code shape is an optimizer input

Identical semantics compile very differently depending on shape; three
measured examples:

- **Inline budgets are caller-sized.** GCC refuses to inline callees into
  functions exceeding `large-function-insns`, so a caller's *size* silently
  changes what gets inlined *into* it. A macro-expanded interpreter body
  crossed the cap and lost the inlining of its stack primitives; compact
  per-op helpers stayed under it. The budgets are tunable in principle —
  `--param large-function-insns` / `large-function-growth` /
  `inline-unit-growth` — but raising them on our LTO *partial-link*
  invocation produced a bit-identical binary, because those parameters do
  not reach the LTRANS phase where the decision is actually made. The
  lesson is therefore narrower than "flags don't work": the flag has to
  reach the phase that makes the choice, and until it does, caller
  structure is what decides.
- **Tuple returns follow the ABI.** Small carried tuples (≤16 bytes)
  return in registers; wider ones lower via `sret` — the ABI itself passes
  a hidden pointer to a *caller local*, which SROA then promotes after
  inlining. "Pass by reference" is thus the lowering of value semantics,
  not an alternative to it.
- **In-place operand windows.** Composing `pop`/`pop`/`push` materializes
  intermediate (word, cursor) tuples; the optimized overlay rewrites the
  pure-ALU family to read its operand slots, retreat the cursor once, and
  write the result in place — the form a hand-written interpreter would
  use, derived from the specification's own axioms.

## Interpreter dispatch

The optimised guest's interpreter is a computed-`goto` loop (labels-as-values;
one 256-entry table) with arm-tail dispatch: each fast-path arm fetches
and jumps to the next opcode directly, and all failure edges branch to a
single halt tail that performs the canonical exceptional halt exactly
once. This is classic threaded code (Bell 1973) re-justified under the
step-count model (Ertl & Gregg 2003 analyze the branch-prediction
argument, which does not apply here; the surviving benefit is the removed
loop-head round-trip). The parallel *generated* interpreter track —
byte-keyed decode tables, `$[c_inline]` fusion of the dispatch layers, and
eventually guaranteed-tail-call (`musttail`) per-opcode handlers over the
uniform carried-state signatures — aims to retire the hand-written loop by
deriving the same shape from the specification.

Closed families elsewhere in the model — trie update sources, cursor and
reducer machinery, call and create modes — are **defunctionalized** the
same way: a tagged union plus a single first-order dispatch, never a
function pointer, so control flow stays visible to both the compiler and
the proof targets. [Brandon et al.](https://doi.org/10.1145/3591260) give
the general account of why specializing such families beats indirect
calls; the optimized-FFI audit enforces the discipline mechanically by
rejecting indirect C calls outright.

## Methodology

Rules that this campaign's failures made non-negotiable:

- **Equal-verdict comparison only.** A validator that rejects a block
  exits before its post-state commitments (~20% of a block's steps). A
  faster run with a different output verdict flag is a bug report, not a
  benchmark — this page exists partly because an "86M-step breakthrough"
  was, for several hours, exactly that.
- **Byte-exact gates at every stage**, native-first: the harness compares
  full guest output against the EELS reference before any ZisK
  measurement is trusted.
- **A measured noise band** (~±0.2–1% from link-order churn) below which
  single-config deltas are not conclusions.
- **Refutations are results.** Threading, pinning, flag-tuning, and one
  cherished phantom number all died to measurements; each removal narrowed
  the design space more than most wins did.

## Compiler work: landed and underway

| area | status |
| --- | --- |
| bound-driven C specialization (ranges → native types, semantic proofs, transitive clones) | landed |
| fixed representations: 4×u64 words, u64-lane addresses/digests, region slices | landed |
| merged register file with hot-member ordering | landed |
| constant-armed match → static const tables; byte-keyed kind tables | landed |
| `$[c_inline]` attribute inlining (dispatch fusion) | landed |
| spliced overrides take precedence over built-in specialized externals | landed |
| `$[c_throws]` effect propagation for optimized-only fallible externs | landed |
| `--c-register-pin` (global register variables) | landed, measured negative; retained as a documented experiment |
| register-file base-pointer threading | landed, measured negative; default off |
| constant-payload match arms as parallel kind/payload tables | planned |
| wide carried tuples lowered as in/out parameters | planned |
| guaranteed-tail-call handler dispatch (GCC ≥ 15 `musttail`) | planned |

## References

**On specification and verification of machine-level semantics**

- A. Armstrong et al., *ISA Semantics for ARMv8-A, RISC-V, and
  CHERI-MIPS* (POPL 2019) — the Sail language this specification is
  written in.
- M. Sammler, A. Hammond, R. Lepigre, B. Campbell, et al.,
  [*Islaris: Verification of Machine Code Against Authoritative ISA
  Semantics*](https://doi.org/10.1145/3519939.3523434) (PLDI 2022) —
  verifying machine code against the same authoritative semantics the
  hardware vendors publish; the closest analogue to what we are trying to
  do for a zkEVM guest.
- M. Sammler, R. Lepigre, R. Krebbers, K. Memarian, et al.,
  [*RefinedC: Automating the Foundational Verification of C Code with
  Refined Ownership Types*](https://doi.org/10.1145/3453483.3454036)
  (PLDI 2021) — foundational verification of exactly the kind of C our
  backends contain.
- S. Keuchel, S. Huyghebaert, G. Lukyanov, D. Devriese,
  [*Verified Symbolic Execution with Kripke Specification
  Monads*](https://doi.org/10.1145/3547628) (ICFP 2022) — symbolic
  execution for Sail-style specifications without meta-programming.
- G. Wood, *Ethereum: A Secure Decentralised Generalised Transaction
  Ledger* (the Yellow Paper) — the machine state μ and the state-passing
  formulation that the carried-value convention restores.
- [`ethereum/execution-specs`](https://github.com/ethereum/execution-specs)
  — the executable Python reference this model is gated byte-exact
  against.

**On specialization, defunctionalization, and transformation**

- S. Ho, A. Fromherz, J. Protzenko, [*Modularity, Code Specialization,
  and Zero-Cost Abstractions for Program
  Verification*](https://doi.org/10.1145/3607844) (ICFP 2023) — the
  argument that verification-time abstraction and compile-time erasure
  can coexist; the philosophy behind our type-driven lowering.
- B. Quiring, D. Van Horn, J. Reppy, O. Shivers, [*Webs and
  Flow-Directed Well-Typedness Preserving Program
  Transformations*](https://doi.org/10.1145/3729280) — webs as the
  natural unit for flow-directed transformation, and why preserving
  well-typedness through them matters.
- W. Brandon, B. Driscoll, F. Dai, W. Berkow, M. Milano, [*Better
  Defunctionalization through Lambda Set
  Specialization*](https://doi.org/10.1145/3591260) (PLDI 2023) — closed
  families specialized rather than dispatched indirectly.
- G. Chaitin, *Register Allocation and Spilling via Graph Coloring*
  (SIGPLAN 1982); P. Briggs, K. Cooper, L. Torczon, *Improvements to
  Graph Coloring Register Allocation* (TOPLAS 1994).
- R. Cytron et al., *Efficiently Computing Static Single Assignment Form
  and the Control Dependence Graph* (TOPLAS 1991).

**On interpreters**

- J. Bell, *Threaded Code* (CACM 1973).
- M. A. Ertl, D. Gregg, *The Structure and Performance of Efficient
  Interpreters* (JILP 2003) — dispatch cost analysis; read against the
  step-count model, since its branch-prediction argument does not apply.
- [evmone](https://github.com/ethereum/evmone) and
  [revm](https://github.com/bluealloy/revm) — contemporary interpreter
  designs whose measured behaviour on the same fixtures anchors the
  comparisons on the [benchmarks page](performance.md).

**On the target**

- The [ZisK](https://github.com/0xPolygonHermez/zisk) zkVM — the step
  cost model, the proving-cost model, precompile syscalls, and fcall
  hints.
