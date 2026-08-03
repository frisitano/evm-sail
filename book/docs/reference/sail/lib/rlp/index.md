# RLP

Recursive Length Prefix (YP Appendix B): the positional, length-prefixed
encoding of nested byte strings that protocol structures serialize to,
and the injective basis of every trie commitment.

- [RLP encoding](encoding.md) — canonical sizing and an exact-length,
  scratch-backed encoder.
- [RLP decoding](decoding.md) — strict cursor traversal, typed scalars, and
  canonical-form rules.
- [Transaction decoding](codecs/transactions.md) — typed transaction
  envelopes (EIP-2718) and per-type payload rules.
- [Transaction signing](codecs/transaction_signing.md), [contract
  addresses](codecs/address.md), [receipts](codecs/receipts.md),
  [withdrawals](codecs/withdrawals.md), [block
  headers](codecs/block_header.md), [state leaves](codecs/state.md), and the
  [block access list](codecs/block_access_list.md) — protocol-specific codecs
  built from the shared primitives.
