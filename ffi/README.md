# C backend ownership

The generated C model links exactly one complete host backend:

- `spec/` implements the readable specification build against Sail's generated
  GMP-backed C ABI.
- `optimized/` implements the production fixed-layout ABI and owns all
  optimized-only whole-operation replacements, capacity planning, and
  pointer-based storage.

The two directories are mutually exclusive implementations of the same Sail
host contracts. A source or header in one backend must not include a private
source or header from the other backend. Builds put the selected backend first
on the include path and compile implementation files only from that directory.
Both backends use the concrete types from the generated model header directly.
Do not add aliases or call/return macros over those generated types: such a
layer hides the actual ABI without providing representation independence.
Small semantic conversion helpers may remain backend-local where byte-order or
ownership operations are genuinely required.

Only standardized platform contracts live at this directory's root:

- `zkvm_accelerators.h`
- `zkvm_io.h`

Shared protocol behavior belongs in Sail. Shared platform behavior belongs
behind one of the standardized headers above. Do not introduce a shared C
adapter layer merely to make the two backend implementations look alike: their
representations and useful optimization boundaries are intentionally allowed
to diverge.
