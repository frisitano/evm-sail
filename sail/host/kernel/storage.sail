val k_aload : address -> Account
val stateless_storage : (hash, word) -> word

function storage_key(a : address, s : word) -> StorageKey = struct { addr = a, slot = s }

/* --- EIP-2929 access lists: mark an address/slot warm, return its PRIOR warm
 * bit (the EVM charges COLD_*_COST on a false result, WARM_STORAGE_READ_COST on
 * true). The warming is journaled so a frame revert un-warms it. --- */
function k_access_account(a : address) -> bool = {
    let prior = warm_addr_touch(a);
    if not_bool(prior) then {
        journal_push(JWarmA(a))
    };
    prior
}
function k_slot_is_warm(a : address, s : word) -> bool = {
    let prior = warm_slot_touch(a, s);
    if not_bool(prior) then {
        journal_push(JWarmS(a, s))
    };
    prior
}

/* --- persistent storage (SLOAD/SSTORE): the C cache/update store, whose
 * mutations are recorded between k_snapshot / k_revert boundaries --- */

/* k_sload: resolve the slot to its live StorageEntry -- curr is the value
 * SLOAD pushes; orig is the EIP-2200 tx-start value the SSTORE gas policy
 * compares against. One resolution serves both views: a tx entry carries the
 * pair; a block-layer hit or a pre-state walk yields curr == orig = the
 * resolved value (nothing this tx has changed it yet), which is ALWAYS cached
 * as a tx read entry, so after any k_sload the slot is live in the tx layer
 * -- the invariant k_sstore's update relies on. storage_tx_cache also marks
 * the EIP-7928 read; the serializer emits distinct read slots not present in
 * storage_changes, so re-reads and reads of written slots add nothing to the
 * BAL. The merge propagates reads to the block layer, so a later tx skips
 * the witness walk. The host owns frame undo records and clear epochs.
 *
 * stateless_storage is the base storage primitive (declared above and defined
 * in lib/state_trie.sail as an authenticated MPT
 * point-get -- one walk for both the witness and the harness-built alloc
 * trie). Everything above it -- the overlay, the read-through, the journal
 * -- is common. */
function k_sload(a : address, s : word) -> StorageValue = {
    let key = storage_key(a, s);
    match storage_tx_get(key) {
        Some(e) => return e,
        None() => (),
    };
    let acc = k_aload(a);
    let v : word = match storage_block_get(key) {
        Some(e) => e.curr,
        None() => if acc.storage_cleared then {
            ZERO_WORD
        } else {
            stateless_storage(acc.info.storage_root, s)
        },
    };
    storage_tx_cache(key, v);
    struct { curr = v, orig = v }
}

/* SSTORE: update the live tx row. The host records its prior current value for
 * frame rollback. The opcode's preceding k_sload guarantees the row exists and
 * that orig remains frozen at the tx-start value. NO account leaf refresh: the
 * storage root is derived by compute_state_root from the cumulative block map. */
function k_sstore(a : address, s : word, v : word) -> unit =
    storage_tx_update(storage_key(a, s), v)

/* EIP-1153 transient storage (TLOAD/TSTORE): per-tx scratch, discarded at tx end,
 * not part of the state trie. TSTORE journals the prior value for frame revert. */
function k_tload(a : address, s : word) -> word = {
    transient_load(a, s)
}
function k_tstore(a : address, s : word, v : word) -> unit = {
    journal_push(JTran(a, s, transient_load(a, s)));
    transient_store(a, s, v)
}
