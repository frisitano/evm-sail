# Host interface

The boundary between the specification and its execution environment.
Sail owns all protocol semantics; the host owns only containers and raw
byte storage, exposed to the specification as bodyless axioms — `val`
declarations with stated contracts and no definitions. Proof targets
treat the axioms as uninterpreted functions constrained by their
contracts; the executable build binds them to an implementation.

Data crosses the boundary in two forms. Scalar values pass directly
through axiom arguments and results. Bulk bytes never cross at all:
they stay in host-owned *regions* — the stateless input buffer, frame
memories, the output buffer, the scratch and absolute-memory arenas —
and the specification manipulates them through nominal region slices naming
an offset and length. The slice type itself identifies stateless input,
memory, code, log data, output, or scratch; there is no runtime source tag.
The *accelerator interface* (the eth-act zkvm-standards surface) is the
third piece: cryptographic operations the execution environment proves
natively, reached through the same axiom discipline.

!!! note "Non-normative"
    These pages document the model's host interface — internal
    contracts of the executable specification, not protocol rules. The
    protocol semantics built on top of these axioms live in
    [the state kernel](../kernel/index.md) and the normative sections.

- [The accelerator interface](accelerators.md) — raw cryptographic
  operations (EIP-152, EIP-2537, EIP-4844, EIP-7951).
- [Region access](region_access.md) — reading region-backed byte
  views.
- [Code storage](code.md) — content-addressed code and `JUMPDEST`
  bitmaps.
- [Validation debug (optional)](debug_disabled.md) — stubbed diagnostic
  hooks.
- [Ancestor hashes](environment.md) — the `BLOCKHASH` table.
- [Per-frame memory](memory.md) — the frame memory region.
- [Witness node database](nodes.md) — the hash-to-span node index.
- [The output buffer](output.md) — frozen frame output and the public
  output stream.
- [Scratch arena](scratch.md) — bulk byte construction space.
- [The operand stack](stack.md) — the per-frame word stack.
- [State store axioms](state.md) — the mutable state containers.
