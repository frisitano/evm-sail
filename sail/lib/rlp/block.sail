/* ======================================================================== */
/* RLP: BLOCK HEADER + WITHDRAWAL                                           */
/*                                                                          */
/* RLP codecs for block-level types: the canonical block-header encoding    */
/* (block_header_hash), per-withdrawal encoding (withdrawal_rlp), and the   */
/* parent-header field readers over the RLP items captured by the input     */
/* pass. Pure encode/decode -- no registers, no stores.                     */
/* ======================================================================== */

/* the parent header's state_root (field 3) = pre-state root. The witness
 * headers run header[0] (oldest) .. header[n-1] (newest); the block being
 * validated extends the NEWEST, so its parent is header[n-1]. */
function parent_state_root(items : list(RlpField)) -> hash =
    bytes_be256(list_nth(items, 3, RLP_NONE).content)

function parent_header_int_field(items : list(RlpField), idx : int) -> int =
    rlp_field_int(rlp_item(items, idx))

function withdrawal_rlp(w : Withdrawal) -> list(byte) = {
    var payload : list(byte) = [||];
    payload = list_append(payload, rlp_int(w.index));
    payload = list_append(payload, rlp_int(w.validator_index));
    payload = list_append(payload, rlp_addr(w.address));
    payload = list_append(payload, rlp_int(w.amount));
    rlp_list(payload)
}

/* Block hash: keccak(rlp([...header fields...])), the canonical field
 * layout. extraData is re-read from its witness span. */
function block_header_hash(
    h : BlockHeader,
    transactions_root : hash,
    withdrawals_root : hash,
    requests_hash : hash,
    block_access_list_hash : hash,
) -> (
    hash
) = {
    var payload : list(byte) = rlp_word(h.parent_hash);
    payload = list_append(payload, rlp_word(EMPTY_OMMER_HASH));
    payload = list_append(payload, rlp_addr(h.fee_recipient));
    payload = list_append(payload, rlp_word(h.state_root));
    payload = list_append(payload, rlp_word(transactions_root));
    payload = list_append(payload, rlp_word(h.receipts_root));
    payload = list_append(payload, rlp_bytes(h.logs_bloom));
    payload = list_append(payload, rlp_int(0));
    payload = list_append(payload, rlp_int(h.number));
    payload = list_append(payload, rlp_int(h.gas_limit));
    payload = list_append(payload, rlp_int(h.gas_used));
    payload = list_append(payload, rlp_int(h.timestamp));
    payload = list_append(payload, rlp_bytes(ssz_bytes(pl_extra_start(), pl_txs_start() - pl_extra_start())));
    payload = list_append(payload, rlp_word(h.prev_randao));
    payload = list_append(payload, rlp_bytes(zero_bytes(8)));
    payload = list_append(payload, rlp_int_word(h.base_fee));
    payload = list_append(payload, rlp_word(withdrawals_root));
    payload = list_append(payload, rlp_int(unsigned(h.blob_gas_used)));
    payload = list_append(payload, rlp_int(unsigned(h.excess_blob_gas)));
    payload = list_append(payload, rlp_word(h.parent_beacon_block_root));
    payload = list_append(payload, rlp_word(requests_hash));
    payload = list_append(payload, rlp_word(block_access_list_hash));
    payload = list_append(payload, rlp_int(unsigned(h.slot_number)));
    keccak_rlp_payload([||], payload)
}
