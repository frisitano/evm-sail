function storage_wset_union_rows(ak : bits(256)) -> int = unsigned(storage_wset_union_count(ak))

function storage_wset_union_row(ak : bits(256), j : int) -> (word, word) =
    (storage_wset_union_slot(ak, get_slice_int(64, j, 0)), storage_wset_union_val(ak, get_slice_int(64, j, 0)))

function storage_has_live_acct_hash(ak : bits(256)) -> bool = {
    let n = storage_wset_union_rows(ak);
    var j : int = 0;
    var found : bool = false;
    while (j < n) & not_bool(found) do {
        let (_, ev) = storage_wset_union_row(ak, j);
        if word_nonzero(ev) then {
            found = true
        } else {
            ()
        };
        j = j + 1
    };
    found
}

function storage_has_live_addr(a : address) -> bool =
    storage_has_live_acct_hash(keccak256_address(a))

/* --- EIP-2929 access lists: mark an address/slot warm, return its PRIOR warm
 * bit (the EVM charges COLD_*_COST on a false result, WARM_STORAGE_READ_COST on
 * true). The warming is journaled so a frame revert un-warms it. --- */
function k_access_account(a : address) -> bool = {
    let prior = warm_addr_contains(a);
    if not_bool(prior) then {
        warm_addr_insert(a);
        journal_push(JWarmA(a))
    };
    prior
}
function k_slot_is_warm(a : address, s : word) -> bool = {
    let prior = warm_slot_contains(a, s);
    if not_bool(prior) then {
        warm_slot_insert(a, s);
        journal_push(JWarmS(a, s))
    };
    prior
}

/* --- persistent storage (SLOAD/SSTORE): the C cache/update store, whose update
 * layers are pushed/popped by k_snapshot / k_commit / k_revert --- */

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
 * the witness walk. Storage reverts run through the JStor journal.
 *
 * stateless_storage is the base storage primitive (declared in
 * host/state.sail, defined in lib/mpt.sail as an authenticated MPT
 * point-get -- one walk for both the witness and the harness-built alloc
 * trie). Everything above it -- the overlay, the read-through, the journal
 * -- is common. */
function k_sload(a : address, s : word) -> StorageValue = {
    match storage_tx_get(a, s) {
        Some(e) => return e,
        None() => (),
    };
    let v : word = match storage_block_get(a, s) {
        Some(e) => e.curr,
        None() => stateless_storage(a, s),
    };
    storage_tx_cache(a, s, v);
    struct { curr = v, orig = v }
}

/* SSTORE: journal the pre-write current (the value a frame revert must put
 * back), then update the tx row. current comes from the caller -- the SSTORE
 * opcode already resolved it for the EIP-2200 gas policy, and its k_sload
 * guarantees the tx layer holds the slot (cache-on-every-read), with orig
 * already frozen at the tx-start value. NO account leaf refresh: the storage
 * root is derived at compute_root, and the witness account iteration unions
 * storage-changed accounts from the storage write-set (lib/mpt.sail
 * state_updates_map). */
function k_sstore(a : address, s : word, v : word, current : word) -> unit = {
    journal_push(JStor(a, s, current)); /* undo -> the value this write overwrites */
    storage_tx_update(a, s, v)
}

/* EIP-1153 transient storage (TLOAD/TSTORE): per-tx scratch, discarded at tx end,
 * not part of the state trie. TSTORE journals the prior value for frame revert. */
function k_tload(a : address, s : word) -> word = {
    transient_load(a, s)
}
function k_tstore(a : address, s : word, v : word) -> unit = {
    journal_push(JTran(a, s, transient_load(a, s)));
    transient_store(a, s, v)
}
