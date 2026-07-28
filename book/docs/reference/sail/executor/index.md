# Block execution

The block-level state transition function (YP §11): everything that
turns a parent state and a new block into a validation verdict and a
post-state. A block executes as pre-execution system calls, the
transactions in order, and post-execution processing; every intermediate
commitment the header claims is recomputed and compared.

- [The block driver](block.md) — block-start system calls, the
  transaction loop, and gas/blob accounting.
- [Protocol system calls](system_calls.md) — the beacon-root, history,
  withdrawal, deposit, and consolidation calls protocol upgrades
  install.
- [Payload commitments](payload.md) — transactions root, withdrawals
  root, and the header's other body commitments.
- [Receipts, blooms, and the receipts trie](receipts.md) — per-
  transaction receipts (YP §4.4.1) and their trie.
- [The block access list](block_access_list.md) — recording and
  encoding state accesses (EIP-7928).
- [Stateless block validation](stateless.md) — executing against a
  witness instead of a full state.
- [The public output](result.md) — the committed result stream.
- [The guest entry point](../main.md) — the top-level program tying the
  stages together.
