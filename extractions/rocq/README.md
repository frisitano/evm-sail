# Rocq Extraction

`make extract-rocq` at the repository root stages and validates the contract relations and
complete model under `contracts/` and `model/` in this directory. The generated
Rocq sources are kept in version control. The Sail compiler continues to call
this backend `--coq`; repository-owned targets and paths use the current Rocq
name.

`make verify-rocq` regenerates the model, checks the committed sources for
drift, and compiles `evm_types.v`, the extern contract, and the maintained RLP
cursor proof. `make build-rocq` additionally performs the expensive exhaustive
elaboration of the complete transparent `evm.v` model and is intended as an
explicit deep check.

The staging step gives repeated `len` and `off` binders in generated dependent
tuple patterns unique names before validation. This is a semantics-neutral
compatibility normalization for Rocq 9.2; it can be removed when the Sail
backend emits collision-free patterns itself.
