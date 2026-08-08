---
hide:
  - toc
---

# EVM Sail

An executable, machine-checked specification of the Ethereum execution
layer — and a workbench for asking hard questions about the guests that
prove it.

The specification is written in [Sail](https://github.com/rems-project/sail),
the language used to give ARM, RISC-V, and CHERI their official semantics.
One source defines the state transition function; from it we extract a
reference implementation in C, proof objects for Lean and Coq, an
executable Python rendering, and a research-centric optimised zkEVM
guest. Because they all come from the same text, a claim proved about the
specification is a claim about the binary that runs.

## What this is for

**Specifying precisely.** Every rule cites the Yellow Paper section or the
EIP it implements, and fork-dependent behaviour is gated on an explicit
fork ordering rather than duplicated per release. The specification is
gated byte-exact against
[`ethereum/execution-specs`](https://github.com/ethereum/execution-specs)
across the full stateless test corpus, so "precise" means tested, not
merely stated.

**Verifying.** The impure boundary is deliberately small and enumerable —
the hashing core, the input oracle, guest output, the world-state and
block-environment stores, host buffers, and the trie node database. Proof
targets receive those as named axioms and everything else as ordinary
definitions. Optimizations never become axioms: a refinement that makes
the optimised guest faster is a *lowering* of a body the proofs still
see in full.

**Measuring, and understanding the trade-offs.** A zkEVM guest is priced
in retired instructions, which inverts much of the usual performance
intuition — memory traffic is pure cost, code size is free, and a
constant address beats a cached pointer. We instrument this guest and its
peers with the same semantic phases and compare them on identical inputs,
so the question "where does the cost actually live?" has an answer with
numbers attached rather than folklore. The
[benchmarks](performance.md) publish those numbers; the
[engineering notes](engineering.md) record what we learned producing them,
including the experiments that failed.

**Verifying the binary, not just the model.** Extraction closes most of
the gap between a specification and the thing that runs, but not all of
it: a compiler still stands between them. Closing that last step is the
end goal, and more than one route is being explored.

One is **binary analysis** — take the compiled RISC-V guest and verify it
against authoritative ISA semantics, in the style of
[Islaris](https://doi.org/10.1145/3519939.3523434), so the proof
obligation lands on the executed machine code rather than on the source
it was generated from. Sail is a natural substrate for that, since the
same language already provides the RISC-V semantics such a proof must be
stated against.

The other direction runs the opposite way: rather than analysing a binary
after the fact, *construct* the guest so that it carries its proofs.
[evm-asm](https://github.com/Verified-zkEVM/evm-asm) takes this approach —
a verified macro assembler in Lean 4 where specification, code, and proof
are co-located, execution cost is bounded explicitly, and the maturity of
each proof surface is tracked as data rather than prose. The two
approaches trade off differently: post-hoc analysis accepts an ordinary
compiler and pays for it in proof effort over generated code, while
verified construction gets stronger guarantees by giving up the compiler.
We are interested in both, and in what each says about the other.

That also shapes how the stack is built. Nothing about binary-level
verification of a stateless validator is specific to *this* guest: the
witness format, the phase structure, and the properties worth proving are
shared with every other zkEVM guest. So the tooling here — the semantic
scopes, the fixture harness, the contract boundaries, the comparison
infrastructure — is deliberately built to generalize, with the ambition of
extending it to verify production guests such as reth's and ethrex's
rather than only our own.

The wider ambition is that specification, verification, and performance
analysis should not be three separate artifacts that drift apart. Here
they are three views of one document — and the intended end state is that
the third view checks the first against the binary itself.

## Where to start

- **[Spec](introduction.md)** — the specification itself: the state
  transition, the EVM, gas, the tries, and stateless validation.
- **[Spec extractions](extractions.md)** — how the same source becomes C
  backends, Lean and Coq developments, and a Python rendering, with links
  to the generated sources.
- **[Performance engineering](engineering.md)** — the cost model,
  type-driven lowering, register custody, and the measured results behind
  the guest's design.
- **[zkEVM benchmarks](performance.md)** — this guest against reth and
  ethrex on identical fixtures, broken down by execution phase.

## Status

This project is **experimental**. It is a research vehicle for the ideas
above, not production consensus software, and it should not be relied on
to validate real chain data. Interfaces, layouts, and internal structure
change frequently; results published here are reproducible from the
repository at the commit each page records, and not guaranteed stable
across commits.

It is also developed **in collaboration with AI**. Substantial parts of
the specification, the compiler work, the C backends, the tooling, and
this documentation were written with AI assistance under human direction
and review. We think that collaboration is a legitimate and interesting
part of the experiment, and we would rather say so plainly than leave it
implicit — with the corresponding caveat that every claim on this site
should be read as something to verify against the corpus gates and the
published measurements rather than to take on trust.
