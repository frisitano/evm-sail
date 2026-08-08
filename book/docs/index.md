---
hide:
  - toc
---

# EVM Sail

An executable, machine-checked specification of the Ethereum execution
layer, written in [Sail](https://github.com/rems-project/sail) — and a
workbench for asking hard questions about the guests that prove it.

One source defines the state transition function. From it we extract a
reference implementation in C, proof objects for Lean and Coq, an
executable Python rendering, and a research-centric optimised zkEVM guest
— through a [custom Sail compiler](https://github.com/frisitano/sail/tree/evm-sail)
that lowers the specification's semantic types to machine representations.

## What this is for

**Precision.** Protocol quantities carry their real limits in their types:
a stack height is not an integer that happens to stay under 1024, it is a
value whose type says so. The typechecker holds those bounds, and every
operation must show its result still lies within them.

**Verification.** The impure boundary is small and enumerable — hashing,
input, output, the state stores — and proof targets receive exactly those
as axioms, with everything else as ordinary definitions.

**Verifying the binary.** Extraction still leaves a compiler between the
specification and the machine code. Closing that gap is the end goal, and
two routes are being explored: binary analysis against authoritative ISA
semantics in the style of
[Islaris](https://doi.org/10.1145/3519939.3523434), and verified
construction as in [evm-asm](https://github.com/Verified-zkEVM/evm-asm).

**Measurement.** A zkEVM guest is priced in retired instructions, which
inverts much of the usual performance intuition. We instrument this guest
and its peers with the same semantic phases on identical inputs, so "where
does the cost live?" has an answer with numbers attached.

**Optimisation.** The fast guest is compiled from the same text as the
proofs, not written beside it. A refinement replaces a body with a faster
one, never with an assumption — so no optimisation ever becomes an axiom.

**Generality.** Nothing about verifying a stateless validator is specific
to our guest, so the tooling is built to extend to others.

## Where to start

- **[Spec](introduction.md)** — the state transition, the EVM, gas, the
  tries, and stateless validation.
- **[Spec extractions](extractions.md)** — how one source becomes C
  backends, Lean and Coq developments, and a Python rendering.
- **[Performance engineering](engineering.md)** — the cost model, the
  compiler machinery, and what the experiments showed.
- **[zkEVM benchmarks](performance.md)** — this guest against reth and
  ethrex, broken down by execution phase.

## Status

**Experimental.** A research vehicle, not production consensus software;
interfaces and internals change frequently.

**Built with AI.** Substantial parts of the specification, compiler, and
tooling were written with AI assistance under human direction — so treat
what you read here as something to check against the corpus gates and the
published measurements.
