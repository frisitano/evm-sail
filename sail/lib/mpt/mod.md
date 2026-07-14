# The trie

The Merkle-Patricia trie (YP Appendix D): the authenticated radix-16
structure behind the state root, storage roots, and the transaction,
receipt, and withdrawal commitments.

- [Trie paths and hex-prefix encoding](primitives.md) — nibble paths
  and their compact encoding.
- [Trie nodes](nodes.md) — leaf, extension, and branch forms and their
  RLP references.
- [Trie updates and the canonical builder](updates.md) — sorted update
  streams and root computation from scratch.
- [The witness-native trie](trie.md) — reading and updating a trie
  through its witness nodes.
