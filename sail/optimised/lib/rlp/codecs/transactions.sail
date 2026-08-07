/* Optimized whole-envelope override for
   `sail/lib/rlp/codecs/transactions.sail`. */

val decode_transaction_envelope = impure { c: "decode_transaction" } : forall 'tx_off 'tx_len 'public_key_off 'blob_limit,
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
    decode_transaction_envelope(tx, pubkey, blob_limit)

/*! Optimized preparation stores the decoded rows in transaction workspace;
    these overrides preserve the canonical list/cursor semantics without
    allocating Sail list nodes in the guest. */
val prepare_authorization_list = impure { c: "prepare_authorizations" } : AuthorizationListRef -> PreparedAuthorizationList

function prepare_authorizations(authorizations : AuthorizationListRef) -> PreparedAuthorizationList =
    prepare_authorization_list(authorizations)

val prepared_authorization_list_head = impure { c: "prepared_authorization_head" } : PreparedAuthorizationList -> Authorization

function prepared_authorization_head(authorizations : PreparedAuthorizationList) -> Authorization =
    prepared_authorization_list_head(authorizations)

val prepared_authorization_list_tail = impure { c: "prepared_authorization_tail" } : forall 'count,
                                                                                          0
                                                                                       <  'count
                                                                                       &  'count
                                                                                       <= prepared_authorization_count_bound. (
        PreparedAuthorizationList,
        int('count),
    ) -> PreparedAuthorizationList

function prepared_authorization_tail(authorizations, count) =
    prepared_authorization_list_tail(authorizations, count)
