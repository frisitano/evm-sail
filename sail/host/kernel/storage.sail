/*md
# State: storage

Warm/cold accounting (EIP-2929), persistent storage (`SLOAD`/`SSTORE`),
and transient storage (EIP-1153), over the host state stores.
*/

val k_aload : address -> Account
val stateless_storage : (hash, word) -> word

function storage_key(a : address, s : word) -> StorageKey = struct { addr = a, slot = s }

/*! Marks an address warm and returns its prior warm bit (EIP-2929: the
    EVM charges the cold cost on `false`, the warm cost on `true`). The
    warming is journaled so a frame revert un-warms it. */
function k_access_account(a : address) -> bool = {
    let prior = warm_addr_touch(a);
    if not_bool(prior) then {
        journal_push(JWarmA(a))
    };
    prior
}
/*! Marks a storage slot warm and returns its prior warm bit; journaled
    like [k_access_account][]. */
function k_slot_is_warm(a : address, s : word) -> bool = {
    let prior = warm_slot_touch(a, s);
    if not_bool(prior) then {
        journal_push(JWarmS(a, s))
    };
    prior
}

/* --- persistent storage (SLOAD/SSTORE): the C cache/update store, whose
 * mutations are recorded between k_state_checkpoint / k_revert boundaries --- */

/*! Resolves a slot to its live [StorageValue][type-StorageValue]: `curr`
    is the value `SLOAD` pushes; `orig` is the EIP-2200 transaction-start
    value the `SSTORE` gas policy compares against. The guarded
    `SLOAD`/`SSTORE` opcode paths are the only callers, so reaching this
    function is the semantic EIP-7928 storage-access boundary: reads are
    recorded independently of the cache layer that supplies the value and
    survive frame rollback (the BAL encoder removes slots that also have a
    storage change). [stateless_storage][] is the base primitive — an
    authenticated MPT point-get, one walk for both the witness and the
    harness-built alloc trie; everything above it (the overlay, the
    read-through, the journal) is common. */
function k_sload(a : address, s : word) -> StorageValue = {
    bal_storage_read(a, s);
    let key = storage_key(a, s);
    match storage_tx_get(key) {
        Some(e) => return e,
        None() => (),
    };
    let acc = k_aload(a);
    match storage_block_get(key) {
        Some(e) => return struct { curr = e.curr, orig = e.curr },
        None() => (),
    };
    let v = if acc.storage_cleared then {
            ZERO_WORD
        } else {
            stateless_storage(acc.info.storage_root, s)
        };
    storage_block_cache(key, v);
    struct { curr = v, orig = v }
}

/*! `SSTORE`: creates or updates the live transaction row. The preceding
    [k_sload][] supplies the transaction-original value; the host keeps
    clear generations and frame undo history private. */
function k_sstore(a : address, s : word, v : StorageValue) -> unit =
    storage_tx_update(struct { key = storage_key(a, s), value = v })

/*! `TLOAD` (EIP-1153): reads per-transaction transient storage, which is
    discarded at transaction end and is not part of the state trie. */
function k_tload(a : address, s : word) -> word = {
    transient_load(a, s)
}
/*! `TSTORE` (EIP-1153): writes transient storage, journaling the prior
    value for frame revert. */
function k_tstore(a : address, s : word, v : word) -> unit = {
    journal_push(JTran(a, s, transient_load(a, s)));
    transient_store(a, s, v)
}
