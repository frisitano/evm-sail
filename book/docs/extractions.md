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

Every target's contract layer and generated output lives in the
repository — every target has the same shape, an axiom or contract layer
plus the generated sources extracted from the Sail model:

| target | contract layer | generated output |
| --- | --- | --- |
| C (reference) | [`extractions/c/spec/contract/`](https://github.com/frisitano/evm-sail/tree/main/extractions/c/spec/contract) | [`extractions/c/spec/src/`](https://github.com/frisitano/evm-sail/tree/main/extractions/c/spec/src) |
| C (optimized) | [`extractions/c/optimised/contract/`](https://github.com/frisitano/evm-sail/tree/main/extractions/c/optimised/contract) | [`extractions/c/optimised/src/`](https://github.com/frisitano/evm-sail/tree/main/extractions/c/optimised/src) |
| Lean | [`HostAxioms.lean`](https://github.com/frisitano/evm-sail/blob/main/extractions/lean/contract/HostAxioms.lean) | [`extractions/lean/src/`](https://github.com/frisitano/evm-sail/tree/main/extractions/lean/src) |
| Coq | [`ExternBoundary.v`](https://github.com/frisitano/evm-sail/blob/main/extractions/coq/contract/ExternBoundary.v) | [`extractions/coq/src/`](https://github.com/frisitano/evm-sail/tree/main/extractions/coq/src) |
| Python | [`HostContract.py`](https://github.com/frisitano/evm-sail/blob/main/extractions/python/contract/HostContract.py) | [`extractions/python/src/`](https://github.com/frisitano/evm-sail/tree/main/extractions/python/src) |

The Sail source these are extracted from is
[`sail/`](https://github.com/frisitano/evm-sail/tree/main/sail); the
optimized C refinements applied on top of it are
[`sail/optimised/`](https://github.com/frisitano/evm-sail/tree/main/sail/optimised).


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
