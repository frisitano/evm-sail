/* ======================================================================== */
/* EIP-7928 BLOCK ACCESS LIST -- item-count validation                      */
/*                                                                          */
/* The consensus gas-limit check on the block's BlockAccessList: EELS       */
/* validate_block_access_list_gas_limit rejects the block when the total    */
/* item count exceeds block_gas_limit / BLOCK_ACCESS_LIST_ITEM (2000). The  */
/* accumulator that RECOMPUTES the BAL from execution (and the keccak(rlp)  */
/* hash) lives in ffi/state_db.c (bal_*); this file is the pure item-count  */
/* over the RLP-encoded list the payload carries. Consumed only by          */
/* the stateless guest (main.sail), but it is consensus logic, so it lives  */
/* with the model rather than the driver.                                   */
/* ======================================================================== */

/* Items contributed by one account entry: rlp([ address, storage_changes,
 * storage_reads, balance_changes, nonce_changes, code_changes ]). Per account
 * the count is 1 + #storage_changes + #storage_reads, where storage_changes
 * (RLP item 1) is a list of [slot, changes] pairs and storage_reads (item 2)
 * is a list of slots. */
function bal_account_items(acc : list(byte)) -> int = {
    let items = rlp_node_items(acc);
    let sc = list_nth(items, 1, RLP_NONE);
    let sr = list_nth(items, 2, RLP_NONE);
    1 + list_len(rlp_node_items(sc.full)) + list_len(rlp_node_items(sr.full))
}

/* total item count over the whole BlockAccessList (a ByteList in the payload):
 * #accounts + sum over accounts of (#storage_changes + #storage_reads). */
function bal_item_count() -> int = {
    let b = pl_bal_start();
    let e = payload_end();
    if b >= e then {
        0
    } else {
        var total : int = 0;
        var rest : list(RlpField) = rlp_node_items(ssz_bytes(b, e - b));
        var more : bool = true;
        while more do {
            match rest {
                a :: r => {
                    total = total + bal_account_items(a.full);
                    rest = r
                },
                [||] => more = false,
            }
        };
        total
    }
}
