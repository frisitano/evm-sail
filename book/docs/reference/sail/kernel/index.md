# The state kernel

The protocol's state semantics, built over the [host
interface](../host/index.md)'s containers: every read, write, checkpoint,
revert, and merge rule is defined here, in the specification. State is
layered — a transaction overlay over a block overlay over the
authenticated witness — and every frame boundary takes a checkpoint so
a reverting frame unwinds precisely its own effects.

- [State: accounts](accounts.md) — account reads, mutations, emptiness
  (EIP-161), and value transfer.
- [State: account code](code.md) — code resolution and EIP-7702
  delegation.
- [The execution environment](environment.md) — block- and
  transaction-scoped context registers.
- [State: the transaction lifecycle](lifecycle.md) — snapshot, revert,
  reset, and the transaction-end merge (EIP-6780).
- [State: logs and refunds](logs.md) — log emission and the refund
  counter.
- [Kernel scratch allocation](scratch.md) — the bump cursor over the
  scratch arena.
- [State: selfdestruct and creation flags](selfdestruct.md) — the
  per-transaction account lifecycle flags.
- [State: storage](storage.md) — persistent, transient (EIP-1153), and
  warm/cold (EIP-2929) storage access.
