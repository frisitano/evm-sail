/* Optimized whole-envelope override for
   `sail/lib/rlp/codecs/transactions.sail`. */

val optimized_rlp_decode_tx =
    impure { c: "decode_transaction" } :
    forall 'tx_off 'tx_len 'public_key_off 'blob_limit,
                                   source_valid_range('tx_off, 'tx_len)
                                &  0
                                <= 'tx_len
                                &  'tx_len
                                <= transaction_length_bound
                                &  source_valid_range('public_key_off, 65)
                                &  transaction_blob_limit_value('blob_limit). (
        StatelessInputSliceFields('tx_off, 'tx_len),
        StatelessInputSliceFields('public_key_off, 65),
        int('blob_limit),
    ) -> Transaction

function rlp_decode_tx(tx, pubkey, blob_limit) =
    optimized_rlp_decode_tx(tx, pubkey, blob_limit)
