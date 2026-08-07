# SSZ

Simple Serialize: the fixed/variable-part offset encoding the stateless
input envelope uses. The specification decodes the envelope in place —
fields resolve to spans of the input buffer rather than copies.

- [SSZ source decoding](ssz.md) — offset arithmetic and span extraction
  over the input buffer.
- [The stateless input decoder](stateless_input.md) — decoding and
  indexing the execution witness: headers, blocks, and trie nodes.
