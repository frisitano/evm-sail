# RLP

Recursive Length Prefix (YP Appendix B): the positional, length-prefixed
encoding of nested byte strings that protocol structures serialize to,
and the injective basis of every trie commitment.

- [RLP encoding](encoding.md) — exact size computation and the
  scratch-backed encoder.
- [RLP decoding](decoding.md) — the strict decoder and canonical-form
  rules.
- [Protocol codecs](codecs/index.md) — the wire formats of the
  protocol structures built on these primitives.
