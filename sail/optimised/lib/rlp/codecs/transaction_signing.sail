/* Optimized C overrides for `sail/lib/rlp/codecs/transaction_signing.sail`. */

/*
 * Complete preimage refinements. The standard model retains the explicit
 * scratch/RLP equations; optimized C computes the same exact widths and
 * writes each complete preimage once into borrowed shared scratch.
 */

val tx_signing_hash_for_envelope = impure { c: "tx_signing_hash" } : (byte, StatelessInputSlice, word) -> hash

function tx_signing_hash(t : TxType, content_src : StatelessInputSlice, v : word) -> hash =
    tx_signing_hash_for_envelope(tx_envelope_type(t), content_src, v)
