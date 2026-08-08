# Extractions

The Sail model is the single source of truth; every executable and every
proof object is an **extraction** of it. Each target consumes the same
canonical specification and differs only in what it treats as axiomatic:
the impure host interface — the `val X = impure { c: "sym" } : T`
contracts for the crypto core, the input oracle, and the mutable host
stores — appears to every target as a set of bodyless typed parameters,
and each target supplies (or assumes) those axioms in its own idiom.

| target | contract layer | output |
| --- | --- | --- |
| **C (spec)** | GMP-backed reference ABI | the byte-exact reference validator used by the harness |
| **C (optimized)** | fixed-layout ABI: 4×u64 words, u64-lane addresses, pointer-backed stores, cursor tokens | the production zkVM guest |
| **Lean** | axiom stubs for the host interface | the model as Lean definitions for proof work |
| **Coq** | `ExternBoundary` parameters | the model as Coq definitions |
| **Python** | host-contract protocol stubs | an executable Python rendering, comparable against `ethereum/execution-specs` |

Both C backends compile against the *generated* model header and name its
concrete types directly — layouts are never hand-mirrored. The optimized
backend may additionally replace whole operations with semantically equal
C refinements selected by the custom compiler's splice mechanism; the
spec backend and every proof target retain the explicit Sail bodies, so
optimizations are never proof axioms.

## Source locations

- C contract implementations: `ffi/spec/` and `ffi/optimized/`
- Proof-target contract stubs: `extractions/contracts/`
  (`ExternBoundary.v`, `HostAxioms.lean`, `HostContract.py`)

*This layout is being reorganized: the C backends move to
`extractions/c/contracts/{spec,optimised}` and each proof target gains its
own `contract/` and committed `src/` directories
(`extractions/{lean,coq,python}/…`), making every target — C included —
the same shape: an axiom interface plus generated output. This page's
links will follow the move.*

## What the axioms are

The extraction boundary is deliberately small and enumerable: the hashing
core, the stateless-input oracle, guest output, the world-state and block
environment stores, host buffers, and the trie node database. Everything
else — the EVM, the gas schedule, RLP, SSZ, the Merkle Patricia Trie, the
full stateless validation pipeline — is pure Sail, compiled or extracted
directly. A proof about the model therefore rests only on those named
axioms plus the target language's soundness; a guest binary implements
exactly the same names in C. The harness closes the loop by gating the
compiled targets byte-exact against the `ethereum/execution-specs`
reference on every fixture corpus.
