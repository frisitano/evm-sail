# Protocol codecs

The wire formats of the protocol structures, defined over the shared
RLP primitives. Each codec owns exact size computation and the encoded
or decoded form of one structure family; lifecycle and validation
policy stay with the executor modules that consume them.

- [CREATE address RLP codec](address.md) — the sender-and-nonce
  preimage of contract addresses (YP §7).
- [Transaction RLP decoding](transactions.md) — typed transaction
  envelopes (EIP-2718) and per-type payload rules.
- [Transaction RLP signing preimages](transaction_signing.md) — the
  byte strings transaction signatures commit to.
- [Receipt RLP codec](receipts.md) — receipts and their typed
  envelopes (YP §4.4.1).
- [Withdrawal RLP codec](withdrawals.md) — withdrawal encoding for the
  withdrawals trie (EIP-4895).
- [Block header RLP codec](block_header.md) — header encoding and the
  block-hash preimage.
- [State trie RLP codec](state.md) — account and storage leaf
  encodings (YP Appendix C).
- [Block access list RLP decoding](block_access_list.md) — the
  canonical access-list stream (EIP-7928).
