# The state kernel

The protocol's view of mutable execution state, built as pure operations
over the host's state store axioms. Kernel operations own the semantic
rules — account existence and emptiness, warm/cold tracking, journalled
undo, and refund accounting — while the host merely stores the values
they read and write. Every operation is named `k_*` and grouped by the
subsystem it governs.

- [The execution environment](environment.md) — the kernel registers
  holding the block environment and the active transaction context.
- [Accounts](accounts.md) — reading, creating, and mutating account
  state through the journal.
- [Account code](code.md) — code resolution, including delegation
  designators (EIP-7702).
- [Storage](storage.md) — persistent and transient storage access with
  original-value tracking.
- [Logs and refunds](logs.md) — the log series and the refund counter.
- [Selfdestruct and creation flags](selfdestruct.md) — per-transaction
  account lifecycle marks (EIP-6780).
- [The transaction lifecycle](lifecycle.md) — transaction-scoped
  begin, checkpoint, commit, and revert sequencing.
- [Kernel scratch allocation](scratch.md) — cursor-owned reservations
  in the host scratch arena.
