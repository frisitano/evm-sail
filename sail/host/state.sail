/* ======================================================================== */
/* HOST STATE                                                               */
/*                                                                          */
/* The world state the EVM acts on (Yellow Paper section 4.1, the state     */
/* sigma): the account table, persistent storage (EIP-2200), transient      */
/* storage (EIP-1153), the EIP-2929 warm address and slot sets, logs, the   */
/* gas-refund counter, the per-tx created-account set (EIP-6780), the block */
/* and transaction environment, and the journal (the undo log that makes a  */
/* call frame's effects atomically revertible).                             */
/*                                                                          */
/* This is the privileged host's data; the EVM (user space) reaches it      */
/* ONLY through the host kernel functions (the k_* syscall surface) in      */
/* host/kernel.sail. Accounts, storage, transient storage, the warm access  */
/* sets, logs, the SELFDESTRUCT/created sets, and the journal are all held  */
/* in C-backed structures (the host holds no Sail data buffers).            */
/* ======================================================================== */

/* ------------------------------ structs --------------------------------- */

/* The account table is the C-backed write-set overlay (keccak(address) -> row;
 * ffi/state_db.c acct_wset_*). Persistent storage is keyed by (keccak(address),
 * keccak(slot)) in the same overlay; transient storage remains C-backed because
 * it is an execution-local word table. Warm address/slot sets are C-backed too
 * (ffi/kernel_state.c). No Sail association lists for sigma. */

/* ======================================================================== */
/* C-BACKED HOST BINDINGS (world state, code store, kernel collections)     */
/*                                                                          */
/* The extern declarations for the C structures this file's Sail logic --   */
/* and host/kernel.sail, which follows -- drives. They live here, before    */
/* both host files, because host/state.sail and host/kernel.sail mutually   */
/* use each other's collections (single-pass typechecking needs every val   */
/* in scope before its first use). The `c:` extern binds ONLY the C         */
/* executable; every proof target (Coq/Lean/Lem) sees a bodyless axiom, so  */
/* each still stands for its abstract map / set / ordered-list model with   */
/* the cache/update and commit/revert laws. Sources: ffi/state_db.c         */
/* (accounts, storage, EIP-7928 BAL), ffi/transient_storage.c,              */
/* ffi/code_db.c (content-addressed code store), ffi/kernel_state.c (warm   */
/* sets, LOG series, selfdestruct/created sets, call-frame journal).        */
/* ======================================================================== */

/* ---- EIP-1153 transient storage (ffi/transient_storage.c): per-tx scratch
 * keyed by (address, slot), discarded at tx end, never in the state trie.
 * TSTORE frame-revert runs through the journal (JTran), not here. ---- */
val transient_reset = impure { c: "transient_storage_reset" } : unit -> unit
val transient_store = impure { c: "transient_storage_write" } : (address, word, word) -> unit
val transient_load = impure { c: "transient_storage_read" } : (address, word) -> word

/* ---- write-set storage overlay (ffi/state_db.c storage_wset_*): the tx overlay +
 * block base. A working-set miss falls through to the base resolver (the alloc
 * MPT natively / the witness MPT in the guest). */
val storage_tx_cache = impure { c: "storage_tx_cache" } : (address, word, word) -> unit /* cache resolved base + mark read (EIP-7928 read record) */
val storage_tx_update = impure { c: "storage_tx_update" } : (address, word, word) -> unit /* current := v on the existing tx row (re-write; JStor undo) */

/* Per-layer entry views: the tx overlay and the block base each answer a
 * point query with option(StorageEntry) (crossing the boundary as a whole
 * union value, like JEntry -- built by ffi/journal_glue.c from state_db.c's
 * row tables). The layer-precedence semantics -- which layer wins, and what
 * the EIP-2200 tx-start original is -- live in SAIL (host/kernel.sail
 * k_sload / k_sstore), not in C. None = this layer never touched the
 * slot. A value's curr is the live word; orig is the layer-start word
 * (frozen from the base on the first write; a cached read has curr == orig =
 * the resolved base). Point lookups know their key, so only the VALUE
 * returns; enumerations return whole entries (StorageEntry below). */

val storage_tx_get = impure { c: "storage_tx_get" } : (address, word) -> option(StorageValue)
val storage_block_get = impure { c: "storage_block_get" } : (address, word) -> option(StorageValue)

/* Entries: key/value pairs in DOMAIN terms -- the key is (address, slot),
 * never a hash; secure-key hashing is a trie/table detail (C hashes for its
 * table keys, the spec hashes visibly where the trie needs it). Drain-one
 * pops are journal_pop-style and SIDE-EFFECT-FREE: each hands back one
 * entry; None = drained (the tx layer resets). The merge itself is SAIL
 * (host/kernel.sail k_tx_merge): inspect the popped entry, emit the
 * EIP-7928 records it warrants, and apply the block-layer propagation it
 * needs through the storage_block_* / acct_block_* hooks below. Every tx
 * row was read (storage_tx_cache is the only row creator, and every
 * SSTORE's k_sload precedes it), so an entry is either a net change or a
 * read -- no flag crosses. */


val storage_tx_pop = impure { c: "storage_tx_pop" } : unit -> option(StorageEntry)



val acct_tx_pop = impure { c: "acct_tx_pop" } : unit -> option(AcctEntry)

/* Block-layer propagation hooks (pure mechanism; Sail chose the action).
 * put = a net change: a fresh block row freezes orig as the pre-state, an
 * existing one keeps its own; curr always lands. cache = a read member:
 * a fresh row binds curr == orig = the value; an existing row is only
 * marked read -- never downgraded. The account pair mirrors it, keyed by
 * the address preimage (the block row keeps it as the BAL serialization
 * key). */
val storage_block_put = impure { c: "storage_block_put" } : (address, word, word, word) -> unit /* (a, slot, curr, orig) */
val storage_block_cache = impure { c: "storage_block_cache" } : (address, word, word) -> unit /* (a, slot, value) */
val acct_block_write = impure { c: "acct_block_write" } : (address, bits(64), word, hash, hash, bits(64), word, hash, hash) -> unit /* (a, curr n/b/sr/ch, orig n/b/sr/ch) */
val acct_block_cache = impure { c: "acct_block_cache" } : (address, bits(64), word, hash, hash) -> unit /* (a, n/b/sr/ch) */

/* EIP-6780 tx-end account deletion (k_account_delete). storage_tx_wipe pops
 * ONE of the address's tx rows, returning its slot -- Sail drains it,
 * recording each touched slot as an EIP-7928 read (reads survive deletion in
 * EELS; the account's changes are voided by the drop). storage_block_wipe
 * then drops the block rows: a same-block re-CREATE must not inherit stale
 * cached storage. The pre-state (node DB) is never mutated. */
val storage_tx_wipe = impure { c: "storage_tx_wipe" } : address -> option(word)
val storage_block_wipe = impure { c: "storage_block_wipe" } : address -> unit

/* whether the account changed (per-field diff vs the tx-start / pre-state
 * original) -- the merge and state-root inclusion rule, stated in Sail. */
function account_changed(c : Account, o : Account) -> bool =
    c.nonce != o.nonce | c.balance != o.balance | c.code_hash != o.code_hash | c.storage_root != o.storage_root

/* state-root enumeration: UNFILTERED block entries -- Sail decides which
 * are net changes vs the pre-state (lib/mpt.sail storage_updates /
 * state_updates_map filter on curr != orig); C only hands entries over. */
val storage_block_row_count = impure { c: "storage_block_row_count" } : address -> bits(64)
val storage_block_row = impure { c: "storage_block_row" } : (address, bits(64)) -> StorageEntry

val acct_block_row_count = impure { c: "acct_block_row_count" } : unit -> bits(64)
val acct_block_row = impure { c: "acct_block_row" } : bits(64) -> AcctEntry

/* EIP-7928 block-access-list record sinks (ffi/state_db.c): append one
 * record, tagged C-side with the ambient bal_set_index. Pure recorders --
 * all change/read DETECTION happens in Sail (k_tx_merge). */
val bal_storage_change = impure { c: "bal_note_storage_change" } : (address, word, word) -> unit
val bal_storage_read = impure { c: "bal_note_storage_read" } : (address, word) -> unit
val bal_balance_change = impure { c: "bal_note_balance_change" } : (address, word) -> unit
val bal_nonce_change = impure { c: "bal_note_nonce_change" } : (address, bits(64)) -> unit
val bal_code_change = impure { c: "bal_note_code_change" } : (address, hash) -> unit

val storage_wset_union_count = impure { c: "storage_wset_union_count" } : hash -> bits(64)
val storage_wset_union_slot = impure { c: "storage_wset_union_slot" } : (hash, bits(64)) -> word
val storage_wset_union_val = impure { c: "storage_wset_union_val" } : (hash, bits(64)) -> word

/* ---- write-set account overlay (ffi/state_db.c acct_wset_*): the per-tx
 * overlay + block base. A miss here is resolved by the base resolver /
 * stateless_account. storage_root is the pre-state anchor, not mutated. */
/* Per-layer account views, mirroring storage_tx_get / storage_block_get: each
 * layer answers a point query with option(Account), crossing the boundary as
 * one whole value (ffi/journal_glue.c over the acct row tables); the
 * tx-over-block precedence lives in Sail (account_lookup below). */
val acct_tx_get = impure { c: "acct_tx_get" } : address -> option(Account)
val acct_block_get = impure { c: "acct_block_get" } : address -> option(Account)
val acct_tx_update = impure { c: "acct_tx_update" } : (address, bits(64), word, hash, hash) -> unit /* cur := fields on the existing tx row (store_account; JAcct undo) */

/* witness base-resolver read-through: bind a resolved pre-state account as a
 * block-level read member */
val acct_tx_cache = impure { c: "acct_tx_cache" } : (address, bits(64), word, hash, hash) -> unit /* cache a resolved account as a tx read member (cur == orig) */

/* compute_root enumeration over acct_wset_block (fields mirror AcctRow):
 * witness = dirty rows (changed vs pre-state); native = cache-union-block rows */

/* ---- EIP-7928 block access list, recomputed from execution (ffi/state_db.c
 * bal_*). The write-set overlay merges harvest this block's changes/reads
 * (keyed by keccak(address), tagged with bal_set_index: 0 pre-exec syscalls,
 * i+1 for user tx i, N+1 post); bal_recompute_hash returns keccak(rlp(bal)). */
val bal_reset = impure { c: "bal_reset" } : unit -> unit
val bal_set_index = impure { c: "bal_set_index" } : bits(64) -> unit
val bal_recompute_hash = impure { c: "bal_recompute_hash" } : unit -> hash

/* ---- content-addressed code store (ffi/code_db.c), keyed by codeHash: each
 * entry holds the code bytes AND the JUMPDEST bitmap. Content-addressing makes
 * the store de-duplicated and append-only, so REVERT needs no code rollback.
 * ALL writes are reference-based: the witness harvest and EIP-7702
 * designations intern a byte-source slice (code_db_store_slice), CREATE
 * deploys intern the frame's returndata (code_intern_returndata). */
val code_db_stored_code_length = impure { c: "code_db_stored_code_length" } : hash -> bits(64)
val code_db_copy_stored_code_to_memory = impure { c: "code_db_copy_stored_code_to_memory" } : (
        hash,
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
val code_db_store_source = impure { c: "code_db_store_source" } : (bits(64), bits(64), bits(64)) -> unit
function code_db_store_slice(s : ByteSlice) -> unit =
    code_db_store_source(byte_source_kind(s.source), s.off, s.len)
val code_intern_returndata = impure { c: "code_intern_returndata" } : unit -> hash
val code_intern_delegation = impure { c: "code_intern_delegation" } : address -> hash

/* EIP-7702 delegation probe for codeHash `h`: bit 160 = "code is a designation
 * (0xef0100 ++ address)", bits 159..0 = the target address (zeros otherwise). */
val code_db_read_delegation = impure { c: "code_db_read_delegation" } : hash -> bits(168)

/* ---- kernel collections (ffi/kernel_state.c) that were Sail registers: the
 * EIP-2929 warm sets, the LOG series, the SELFDESTRUCT and EIP-6780 created
 * sets, and the call-frame journal (undo log). Each carries the dedup,
 * ordering, and call-frame revert laws the registers had. ---- */

/* EIP-2929 warm access sets: membership over address / (address, slot);
 * insert/remove pair per the journal (a revert un-warms a new entry). */
val warm_reset = impure { c: "warm_reset" } : unit -> unit
val warm_addr_contains = impure { c: "warm_addr_contains" } : address -> bool
val warm_addr_insert = impure { c: "warm_addr_insert" } : address -> unit
val warm_addr_remove = impure { c: "warm_addr_remove" } : address -> unit
val warm_slot_contains = impure { c: "warm_slot_contains" } : (address, word) -> bool
val warm_slot_insert = impure { c: "warm_slot_insert" } : (address, word) -> unit
val warm_slot_remove = impure { c: "warm_slot_remove" } : (address, word) -> unit

/* LOG series: an ordered list of records (emitter, 0..4 topics, data
 * bytes). ONE call appends a whole record -- the data crosses as a Bytes
 * segment (the memory span for LOG0..4, materialized words for the EIP-7708
 * system logs) and is snapshotted into the C store; ONE call reads the whole
 * ordered series back for the receipt. Both cross generated types via
 * ffi/hash_glue.c. The last record is dropped on a revert (JLog). */
val logs_tx_reset = impure { c: "logs_tx_reset" } : unit -> unit /* records only;
                                  the data arena persists for receipt-held slices */
val log_append = impure { c: "log_append_record" } : (address, list(word), Bytes) -> unit
val log_drop_last = impure { c: "log_drop_last" } : unit -> unit
val read_logs = impure { c: "logs_read_all" } : unit -> list(LogEntry)
val log_arena_byte = impure { c: "log_arena_byte" } : bits(64) -> bits(8) /* read_byte_slice's LogDataSource view */

/* SELFDESTRUCT set: ordered (push / drop-last on revert via JSelfD), with a
 * membership test and index enumeration for the EIP-6780 tx-end finalization. */
val selfdestr_reset = impure { c: "selfdestr_reset" } : unit -> unit
val selfdestr_push = impure { c: "selfdestr_push" } : address -> unit
val selfdestr_drop_last = impure { c: "selfdestr_drop_last" } : unit -> unit
val selfdestr_contains = impure { c: "selfdestr_contains" } : address -> bool
val selfdestr_count = impure { c: "selfdestr_count" } : unit -> bits(64)
val selfdestr_get = impure { c: "selfdestr_get" } : bits(64) -> word

/* EIP-6780 created-this-tx set: idempotent-insert membership set; NOT journaled
 * (the marker survives a frame revert, like EELS created_accounts). */
val created_reset = impure { c: "created_reset" } : unit -> unit
val created_insert = impure { c: "created_insert" } : address -> unit
val created_contains = impure { c: "created_contains" } : address -> bool

/* call-frame journal (undo log) lifecycle: reset clears the whole journal;
 * commit folds the newest frame's entries into the parent (drops its JCheck
 * checkpoint marker). The entry-level operations are journal_push /
 * journal_pop, declared after union JEntry below. */
val journal_reset = impure { c: "journal_reset" } : unit -> unit
val journal_commit = impure { c: "journal_commit" } : unit -> unit

/* ----------------------------- kernel state ----------------------------- */


/* the account working set is the C-backed eest_account (ffi/state_db.c), keyed by
 * keccak(address) -- no Sail assoc list. The EIP-2929 warm address/slot sets,
 * the LOG series, and the SELFDESTRUCT/created sets and journal are likewise
 * C-backed (ffi/kernel_state.c; the bindings above). */

/* Stateless storage resolver: the value of (a, s) in the witness pre-state,
 * walked from the account's authenticated storage root. Defined in lib/mpt.sail
 * (the witness reader, included later); declared here so the kernel's storage
 * reads can fall through to it on a working-set miss. */
val stateless_storage : (address, word) -> word

function storage_wset_union_rows(ak : bits(256)) -> int = unsigned(storage_wset_union_count(ak))
function storage_wset_union_row(ak : bits(256), j : int) -> (word, word) =
    (storage_wset_union_slot(ak, get_slice_int(64, j, 0)), storage_wset_union_val(ak, get_slice_int(64, j, 0)))

/* "has any live (nonzero) storage slot" -- the CREATE address-collision test
 * (EIP-158 account_has_storage). The cache-union-block sees the native seeded
 * pre-state and any block writes; for the witness the cache is empty, so this
 * catches block writes and the caller also tests acc.storage_root != EMPTY. */
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

register k_refund : i64 = 0 /* gas-refund counter (SSTORE clears; EIP-3529 cap applied at tx end) */

register k_parent_state_root : hash =
    sail_zeros(256) /* the authenticated pre-state anchor: the parent header's
                    state_root, set once per block (authenticate_parent_state) and ONLY
                    AFTER the witness header chain validates, so it never holds a root
                    from an unvalidated chain. Together with the keccak(node)-keyed
                    node-db this is the whole lazy-witness model -- "node database + one
                    authenticated root register": every lazy account/storage walk and
                    the post-state overlay anchor here, and each walk is itself a
                    hash-chain proof from this root. Per-block input, same lifetime as
                    the node-db: the between-run wipe (test_utils.c clear_memory ->
                    nodedb_reset) must NOT clear it mid-run. */

/* Witness ancestor-header hashes (fixed 256-slot C table, kernel_state.c),
 * indexed by DISTANCE: slot j holds keccak(header[n-1-j]), the
 * (j+1)-blocks-back ancestor, so BLOCKHASH offset o reads slot o-1. Captured
 * by the witness-header pass (index_witness_headers, host/block.sail); only
 * the first k_n_headers slots are readable (k_blockhash guards), so no
 * reset is needed between runs. */
val headerhash_set = impure { c: "headerhash_set" } : (bits(64), hash) -> unit
val headerhash_get = impure { c: "headerhash_get" } : bits(64) -> hash
register k_n_headers : int = 0

register k_fork : Fork = Amsterdam /* the active fork; the guest runs the
                                      newest, the runner sets it per case
                                      (k_fork_from_ssz) */


/* Is `n` (the low bytes of an address, as an int) a precompile at the active
 * fork? The precompile set GROWS by fork: Frontier has 0x01-0x04, Byzantium
 * adds 0x05-0x08, Istanbul adds 0x09, Cancun adds 0x0a POINT_EVALUATION,
 * Prague adds the EIP-2537 BLS12-381 set 0x0b-0x11
 * (11-17), and Osaka adds 0x100 P256VERIFY (EIP-7951). A CALL to an address
 * OUTSIDE the active set is an ordinary call to a (typically empty) account,
 * not a precompile. */
function is_precompile(n : int) -> bool = {
    let base = (n >= 1) & (n <= 4);
    let byzantium = (n >= 5) & (n <= 8) & (k_fork >= Byzantium);
    let istanbul = (n == 9) & (k_fork >= Istanbul);
    let kzg = (n == 10) & (k_fork >= Cancun);
    let bls = (n >= 11) & (n <= 17) & (k_fork >= Prague);
    let p256 = (n == 256) & (k_fork >= Osaka);
    base | byzantium | istanbul | kzg | bls | p256
}

/* The full consensus block header: the sole per-block environment. The EVM reads
 * its execution-observable subset through k_env / k_coinbase (kernel.sail), which
 * project + type-normalize these fields (int->word, address->word, chain_id from
 * chain config); block validation reads the full header (block hash, roots, output
 * checks). Set once per block from the SSZ payload (k_set_header in decode_stateless_input). */
register k_header : BlockHeader =
    struct {
        number = 0,
        timestamp = 0,
        gas_limit = 0,
        gas_used = 0,
        prev_randao = ZERO_WORD,
        base_fee = ZERO_WORD,
        blob_gas_used = sail_zeros(64),
        excess_blob_gas = sail_zeros(64),
        state_root = ZERO_WORD,
        receipts_root = ZERO_WORD,
        logs_bloom = [||],
        block_hash = ZERO_WORD,
        fee_recipient = ZERO_ADDR,
        parent_hash = ZERO_WORD,
        parent_beacon_block_root = ZERO_WORD,
        slot_number = sail_zeros(64),
    }
register k_tx : TxEnv = struct { origin = ZERO_ADDR, gas_price = ZERO_WORD, blob_hashes = [||] }

/* ----------------------------- assoc helpers ---------------------------- */

/* Stateless account materialization (defined in lib/mpt.sail, the witness reader,
 * included later): resolve `a` from the authenticated witness leaf store and bind
 * it (nonce/balance/storageRoot/codeHash) as Some(account); proven-absent in
 * the witness -> None. PURE: the caller does the caching. The witness IS the
 * base, so k_aload loads on first touch -- no eager seeding. */
val stateless_account : address -> option(Account)

/* internal (un-journaled) account accessor used inside the kernel functions,
 * exactly k_sload-shaped: this tx's layer, else the block layer, else the
 * pure witness resolver (stateless_account; None = proven-absent = empty).
 * Every resolution is cached HERE into the tx layer, so after any load the
 * account is live in the tx layer -- the invariant acct_tx_update
 * (store_account) relies on -- and the merge propagates read members to the
 * block layer for later txs and the BAL account list. Un-journaled: a frame
 * revert keeps the revealed pre-state and only undoes recorded mutations. */
function k_aload(a : address) -> Account = {
    match acct_tx_get(a) {
        Some(acc) => return acc,
        None() => (),
    };
    let acc : Account = match acct_block_get(a) {
        Some(acc) => acc,
        None() => match stateless_account(a) {
            Some(acc) => acc,
            None() => EMPTY_ACCOUNT, /* proven-absent reads as empty */
        },
    };
    acct_tx_cache(a, acc.nonce, acc.balance, acc.storage_root, acc.code_hash);
    acc
}

/* a mutation writes the per-tx account overlay (acct_wset); the first write this
 * tx freezes the tx-start original C-side. Reverts run through the JAcct journal
 * (apply_undo below restores the overlay current), not the resolver cache. */
function store_account(a : address, v : Account) -> unit =
    acct_tx_update(a, v.nonce, v.balance, v.storage_root, v.code_hash)

/* --- journal boundary: push / pop whole JEntry values ---
 * The one place a user-defined Sail type crosses the C boundary. The glue
 * (ffi/journal_glue.c, compiled against the generated model header) encodes a
 * pushed JEntry into the scalar journal store (ffi/kernel_state.c) and
 * decodes the top entry back out. journal_pop pops AND returns the top entry;
 * on an empty journal it returns JCheck -- a no-op undo that also terminates
 * the k_revert loop -- without popping anything, so no caller needs a
 * separate emptiness guard. Laws: LIFO -- pop after push(e) yields e;
 * journal_commit drops exactly the newest JCheck checkpoint marker. */
val journal_push = impure { c: "journal_push" } : JEntry -> unit
val journal_pop = impure { c: "journal_pop" } : unit -> JEntry

/* Apply one undo entry (k_revert replays these back to the checkpoint). */
function apply_undo(e : JEntry) -> unit = match e {
    JCheck()       => (),
    JAcct(a, old)  => store_account(a, old),
    JTran(a, s, v) => transient_store(a, s, v),
    JStor(a, s, p) => storage_tx_update(a, s, p),
    JWarmA(a)      => warm_addr_remove(a),
    JWarmS(a, s)   => warm_slot_remove(a, s),
    JLog()         => log_drop_last(),
    JRefund(old)   => k_refund = old,
    JSelfD()       => selfdestr_drop_last(),
}

