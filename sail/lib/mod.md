# Serialization and tries

The three encodings the protocol commits to, and the helpers built on
them. RLP (YP Appendix B) is the wire and trie encoding of protocol
structures; SSZ frames the stateless input envelope; the Merkle-Patricia
trie (YP Appendix D) turns key–value state into the roots the header
commits to.

- [Byte conversions](bytes.md), [contract address
  derivation](address.md), and [transaction signature rules](tx.md) —
  shared helpers over the primitive types.
- [RLP](rlp/index.md) — encoding, decoding, and transaction envelopes.
- [SSZ](ssz/index.md) — decoding the witness envelope.
- [SSZ hash_tree_root](htr.md) — the Merkleization of the stateless
  input.
- [The trie](mpt/index.md) — paths, nodes, updates, and the
  witness-native walker.
- [The state trie](state_trie.md) — accounts and storage over the trie,
  producing the post-state root.
