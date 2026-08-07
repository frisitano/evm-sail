/*md
# State: storage

Warm/cold accounting (EIP-2929), persistent storage (`SLOAD`/`SSTORE`),
and transient storage (EIP-1153), over the host state stores.
*/

val k_aload : address -> Account
val stateless_storage_by_key : (hash, hash) -> word
val precompile_id_for_address : address -> precompile_id

function storage_key(a : address, s : word) -> StorageKey = struct { addr = a, slot = s }

/*! Returns the address's EIP-2929 warm bit without changing state. Active
    precompiles are warm independently of the BAL-derived account table. */
function k_account_is_warm(a : address) -> bool = {
    let precompile_id = precompile_id_for_address(a);
    if precompile_id != NotPrecompile then {
        true
    } else {
        account_is_warm(a)
    }
}

/*! Marks an address warm after the caller has established that its access gas
    is affordable. Active precompiles need no host-table entry. */
function k_account_mark_warm(a : address) -> unit = {
    let precompile_id = precompile_id_for_address(a);
    if precompile_id != NotPrecompile then {
        return ()
    };
    account_mark_warm(a)
}

/*! Returns a storage slot's EIP-2929 warm bit without changing state. */
function k_slot_is_warm(a : address, s : word) -> bool = storage_is_warm(a, s)

/*! Marks a storage slot warm after the caller has paid its access charge. */
function k_slot_mark_warm(a : address, s : word) -> unit = storage_mark_warm(a, s)

/*! Warms an explicitly addressed storage slot while preparing a transaction's
    access list, before any EVM frame owns the execution context. */
function k_prewarm_slot(a : address, s : word) -> unit = storage_mark_warm(a, s)

/* --- persistent storage (SLOAD/SSTORE): the C cache/update store, whose
 * mutations are recorded between k_journal_checkpoint / k_journal_revert boundaries --- */

/*! Resolves a slot to its live [StorageValue][type-StorageValue]: `curr`
    is the value `SLOAD` pushes; `orig` is the EIP-2200 transaction-start
    value the `SSTORE` gas policy compares against. The guarded
    `SLOAD`/`SSTORE` opcode paths are the only callers, so reaching this
    function consults the transaction overlay first. A real row hit was already
    recorded when that row was established; misses record the EIP-7928 storage
    read before consulting either a transaction-local clear generation or
    block-scoped state. A clear generation makes an uncached slot known-zero,
    but is not itself a slot-cache hit. BAL reads survive frame rollback (the
    encoder removes slots that also have a storage change).
    [stateless_storage_by_key][] is the base primitive — an authenticated MPT
    point-get, one walk for both the witness and the harness-built alloc trie;
    everything above it (the overlay, the read-through, the journal) is common. */
function k_sload(a : address, s : word) -> StorageValue = {
    let key = storage_key(a, s);
    let tx_value = storage_tx_get(key);
    match tx_value {
        StorageTxHit(value) => return value,
        StorageTxCleared(_) => {
            bal_storage_read(a, s);
            return struct { curr = ZERO_WORD, orig = ZERO_WORD }
        },
        StorageTxMiss(_) => bal_storage_read(a, s),
    };

    let block_value = storage_block_get(key);
    if block_value.found then {
        return struct { curr = block_value.value.curr, orig = block_value.value.curr }
    };

    let acc = k_aload(a);
    let slot_hash = keccak256_word(s);
    let value =
        if acc.storage_cleared then ZERO_WORD else stateless_storage_by_key(acc.info.storage_root, slot_hash);
    storage_block_cache(key, slot_hash, value);
    struct { curr = value, orig = value }
}

/*! `SSTORE`: creates or updates the live transaction row. The preceding
    [k_sload][] supplies the transaction-original value; the host keeps
    clear generations and frame undo history private. */
function k_sstore(a : address, s : word, v : StorageValue) -> unit = {
    let key = storage_key(a, s);
    storage_tx_update(struct { key = key, value = v })
}

/*! `TLOAD` (EIP-1153): reads per-transaction transient storage, which is
    discarded at transaction end and is not part of the state trie. */
function k_tload(a : address, s : word) -> word = {
    transient_load(a, s)
}
/*! `TSTORE` (EIP-1153): writes transient storage. Frame rollback is part
    of the host's semantic checkpoint contract. */
function k_tstore(a : address, s : word, v : word) -> unit = transient_store(a, s, v)
