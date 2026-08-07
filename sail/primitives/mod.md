# Protocol types

The data-model layer of the specification: the containers, enumerations,
and value types every other section operates on. Definitions here carry
no execution semantics — validation rules, gas accounting, and the state
transition itself live in the operative sections that consume these
types.

Almost every rule in the specification is conditional on the active
protocol fork, so the layer starts with the fork enumeration and the
chain parameters that select it:

- [Protocol forks](fork.md) — the ordered fork enumeration and its
  comparisons.
- [Chain configuration](chain_config.md) — the activation schedule and
  chain identity a block is validated against.

The protocol containers follow (YP §4): [accounts and
storage](account.md), [transactions, logs, and receipts](tx.md), and
[block types](block.md). The execution engine's own types come next —
[EVM execution types](evm.md), [gas types](gas.md), the [bounded
protocol quantities](quantities.md), and the [system addresses and
topics](system.md) reserved by protocol upgrades.

The remaining pages define the byte-level and serialization-facing
vocabulary: [regions and byte slices](bytes.md), [code](code.md),
[cryptographic primitives](crypto.md), [RLP field references](rlp.md),
[SSZ collections](ssz.md), and the [stateless
input](stateless_input.md) envelope.
