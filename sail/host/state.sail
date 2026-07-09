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
/* This is the privileged host's data; the transaction kernel (the EVM,     */
/* user space) reaches it ONLY through the host kernel functions in         */
/* host/kernel.sail. Accounts, storage, transient storage, the warm access  */
/* sets, logs, the SELFDESTRUCT/created sets, and the journal are all held  */
/* in C-backed structures (the host holds no Sail data buffers).            */
/* ======================================================================== */

/* ------------------------------ structs --------------------------------- */

/* Account state (Yellow Paper section 4.1, the account tuple sigma[a]).
 * storage_root is the account's PRE-STATE storage root -- the witness anchor
 * that stateless storage reads walk on a working-set miss. It is set when the
 * account is materialized from the witness and is NOT kept live during execution
 * (storage mutates in the overlay maps; the post-state root is computed on demand
 * in state_root.sail). Code BYTES are not held here -- the account carries only
 * codeHash, a content address into the code store; codeHash is itself observable
 * state (EXTCODEHASH, EIP-1052) and the binding a stateless witness checks code
 * against. */
struct Account = {
    nonce        : int,   /* sigma[a]_n: nonce (txs sent / contracts created)      */
    balance      : word,  /* sigma[a]_b: balance in wei                            */
    code_hash     : hash,  /* sigma[a]_c: keccak256(code); KECCAK_EMPTY if codeless */
    storage_root : hash,  /* sigma[a]_s: PRE-STATE storage root, the witness anchor */
}

/* keccak256 of the empty string: the codeHash of every codeless account */
let KECCAK_EMPTY : hash = 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470

/* keccak256(rlp("")) -- the root of an empty Merkle-Patricia trie: the storage
 * root of every account with no storage (EMPTY_ACCOUNT, freshly created). */
let EMPTY_TRIE_ROOT : hash = 0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421

/* The account table is the C-backed write-set overlay (keccak(address) -> row;
 * ffi/state_db.c acct_wset_*). Persistent storage is keyed by (keccak(address),
 * keccak(slot)) in the same overlay; transient storage remains C-backed because
 * it is an execution-local word table. Warm address/slot sets are C-backed too
 * (ffi/kernel_state.c). No Sail association lists for sigma. */

/* SlotKey (the BAL access-set key) is defined in host/io.sail (its bal_load
 * builder is the producer), included before this file. */

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

/* ---- transient storage (ffi/transient_storage.c): id 0 = EIP-1153. ---- */
val transient_storage_reset = impure { c: "transient_storage_reset" } : bits(64) -> unit
val transient_storage_write = impure { c: "transient_storage_write" } : (bits(64), address, word, word) -> unit
val transient_storage_read = impure { c: "transient_storage_read" } : (bits(64), address, word) -> word

/* ---- write-set storage overlay (ffi/state_db.c storage_wset_*): the tx overlay +
 * block base. A working-set miss falls through to the base resolver (the alloc
 * MPT natively / the witness MPT in the guest). */
val storage_wset_reset = impure { c: "storage_wset_reset" } : unit -> unit
val storage_wset_tx_clear = impure { c: "storage_wset_tx_clear" } : unit -> unit
val storage_wset_merge = impure { c: "storage_wset_merge" } : unit -> unit
val storage_wset_present = impure { c: "storage_wset_present" } : (address, word) -> bool /* tx-or-block holds a value */
val storage_wset_load = impure { c: "storage_wset_load" } : (address, word) -> word /* live value (0 on miss) */
val storage_wset_base_present = impure { c: "storage_wset_base_present" } : (address, word) -> bool /* block base holds a value */
val storage_wset_base_load = impure { c: "storage_wset_base_load" } : (address, word) -> word /* tx-start value (k_sload_orig) */
val storage_wset_touch = impure { c: "storage_wset_touch" } : (address, word) -> unit /* mark touched (BAL reads) */
val storage_wset_cache_read = impure { c: "storage_wset_cache_read" } : (address, word, word) -> unit /* cache resolved base + mark read */
val storage_wset_prior = impure { c: "storage_wset_prior" } : (address, word, word) -> word /* JStor payload: value a revert restores */
val storage_wset_write = impure { c: "storage_wset_write" } : (address, word, word, word) -> unit /* (a,s,v,orig): orig adopted on first write */
val storage_wset_restore = impure { c: "storage_wset_restore" } : (address, word, word) -> unit /* JStor undo: current := prior */
val storage_wset_warm = impure { c: "storage_wset_warm" } : (address, word) -> bool /* set warm; true iff was cold */
val storage_wset_is_warm = impure { c: "storage_wset_is_warm" } : (address, word) -> bool
val storage_wset_unwarm = impure { c: "storage_wset_unwarm" } : (address, word) -> unit /* JWarmS undo */
val storage_wset_wipe_addr = impure { c: "storage_wset_wipe_addr" } : address -> unit /* EIP-6780 tx-end delete */

/* compute_root enumeration over the block base, per account keccak(addr):
 * witness = dirty rows (net change vs MPT); native = cache-union-block rows */
val storage_wset_block_dirty_count = impure { c: "storage_wset_block_dirty_count" } : hash -> bits(64)
val storage_wset_block_dirty_slot = impure { c: "storage_wset_block_dirty_slot" } : (hash, bits(64)) -> word
val storage_wset_block_dirty_val = impure { c: "storage_wset_block_dirty_val" } : (hash, bits(64)) -> word
val storage_wset_union_count = impure { c: "storage_wset_union_count" } : hash -> bits(64)
val storage_wset_union_slot = impure { c: "storage_wset_union_slot" } : (hash, bits(64)) -> word
val storage_wset_union_val = impure { c: "storage_wset_union_val" } : (hash, bits(64)) -> word

/* ---- write-set account overlay (ffi/state_db.c acct_wset_*): the per-tx
 * overlay + block base. A miss here is resolved by the base resolver /
 * stateless_account_load. storage_root is the pre-state anchor, not mutated. */
val acct_wset_reset = impure { c: "acct_wset_reset" } : unit -> unit
val acct_wset_tx_clear = impure { c: "acct_wset_tx_clear" } : unit -> unit
val acct_wset_merge = impure { c: "acct_wset_merge" } : unit -> unit
val acct_wset_wipe_addr = impure { c: "acct_wset_wipe_addr" } : address -> unit
val acct_wset_present = impure { c: "acct_wset_present" } : address -> bool
val acct_wset_nonce = impure { c: "acct_wset_nonce" } : address -> bits(64)
val acct_wset_bal = impure { c: "acct_wset_bal" } : address -> word
val acct_wset_sroot = impure { c: "acct_wset_sroot" } : address -> hash
val acct_wset_chash = impure { c: "acct_wset_chash" } : address -> hash
val acct_wset_write = impure { c: "acct_wset_write" } : (address, bits(64), word, hash, hash) -> unit
val acct_wset_restore = impure { c: "acct_wset_restore" } : (address, bits(64), word, hash, hash) -> unit

/* witness base-resolver read-through: bind a resolved pre-state account as a
 * block-level read member */
val acct_wset_seed_read = impure { c: "acct_wset_seed_read" } : (address, bits(64), word, hash, hash) -> unit
val acct_wset_mark_base_exists = impure { c: "acct_wset_mark_base_exists" } : address -> unit

/* compute_root enumeration over acct_wset_block (fields mirror AcctRow):
 * witness = dirty rows (changed vs pre-state); native = cache-union-block rows */
val acct_wset_block_dirty_count = impure { c: "acct_wset_block_dirty_count" } : unit -> bits(64)
val acct_wset_block_dirty_hkey = impure { c: "acct_wset_block_dirty_hkey" } : bits(64) -> hash
val acct_wset_block_dirty_nonce = impure { c: "acct_wset_block_dirty_nonce" } : bits(64) -> bits(64)
val acct_wset_block_dirty_bal = impure { c: "acct_wset_block_dirty_bal" } : bits(64) -> word
val acct_wset_block_dirty_sroot = impure { c: "acct_wset_block_dirty_sroot" } : bits(64) -> hash
val acct_wset_block_dirty_chash = impure { c: "acct_wset_block_dirty_chash" } : bits(64) -> hash
val acct_wset_block_dirty_base_exists = impure { c: "acct_wset_block_dirty_base_exists" } : bits(64) -> bool
val acct_wset_union_count = impure { c: "acct_wset_union_count" } : unit -> bits(64)
val acct_wset_union_hkey = impure { c: "acct_wset_union_hkey" } : bits(64) -> hash
val acct_wset_union_nonce = impure { c: "acct_wset_union_nonce" } : bits(64) -> bits(64)
val acct_wset_union_bal = impure { c: "acct_wset_union_bal" } : bits(64) -> word
val acct_wset_union_sroot = impure { c: "acct_wset_union_sroot" } : bits(64) -> hash
val acct_wset_union_chash = impure { c: "acct_wset_union_chash" } : bits(64) -> hash
val acct_wset_union_base_exists = impure { c: "acct_wset_union_base_exists" } : bits(64) -> bool

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
 * Streamed writes (begin/byte) seed native pre-state code; execution-path writes
 * (CREATE deploys, EIP-7702 designations) and the witness path intern a blob
 * directly from a host byte source (code_intern_* / code_db_store_slice). */
val code_db_stream_code_begin = impure { c: "code_db_stream_code_begin" } : hash -> bits(64)
val code_db_stream_code_byte = impure { c: "code_db_stream_code_byte" } : bits(8) -> unit
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

/* LOG series: an ordered list of records (emitter, 0..4 topics, data bytes),
 * streamed (log_begin then topics/data), the last dropped on a revert (JLog). */
val logs_reset = impure { c: "logs_reset" } : unit -> unit
val log_begin = impure { c: "log_begin" } : address -> unit
val log_add_topic = impure { c: "log_add_topic" } : word -> unit
val log_add_data_byte = impure { c: "log_add_data_byte" } : bits(8) -> unit
val log_drop_last = impure { c: "log_drop_last" } : unit -> unit
val log_count = impure { c: "log_count" } : unit -> bits(64)
val log_addr = impure { c: "log_addr" } : bits(64) -> word
val log_topic_count = impure { c: "log_topic_count" } : bits(64) -> bits(64)
val log_topic = impure { c: "log_topic" } : (bits(64), bits(64)) -> word
val log_data_len = impure { c: "log_data_len" } : bits(64) -> bits(64)
val log_data_byte = impure { c: "log_data_byte" } : (bits(64), bits(64)) -> bits(8)

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

/* call-frame journal (undo log): a stack of undo entries. push_* append one;
 * commit folds the frame's entries into the parent; revert reads the top entry's
 * tag + payload (top_*), the caller dispatches its undo, then drop_top pops back
 * to and including the checkpoint. top_tag returns JT_EMPTY when empty. */
val journal_reset = impure { c: "journal_reset" } : unit -> unit
val journal_push_check = impure { c: "journal_push_check" } : unit -> unit
val journal_push_acct = impure { c: "journal_push_acct" } : (address, bits(64), word, hash, hash) -> unit
val journal_push_tran = impure { c: "journal_push_tran" } : (address, word, word) -> unit
val journal_push_stor = impure { c: "journal_push_stor" } : (address, word, word) -> unit
val journal_push_warma = impure { c: "journal_push_warma" } : address -> unit
val journal_push_warms = impure { c: "journal_push_warms" } : (address, word) -> unit
val journal_push_log = impure { c: "journal_push_log" } : unit -> unit
val journal_push_refund = impure { c: "journal_push_refund" } : bits(64) -> unit
val journal_push_selfd = impure { c: "journal_push_selfd" } : unit -> unit
val journal_commit = impure { c: "journal_commit" } : unit -> unit
val journal_top_tag = impure { c: "journal_top_tag" } : unit -> bits(64)
val journal_drop_top = impure { c: "journal_drop_top" } : unit -> unit
val journal_top_addr = impure { c: "journal_top_addr" } : unit -> word
val journal_top_nonce = impure { c: "journal_top_nonce" } : unit -> bits(64)
val journal_top_balance = impure { c: "journal_top_balance" } : unit -> word
val journal_top_sroot = impure { c: "journal_top_sroot" } : unit -> hash
val journal_top_chash = impure { c: "journal_top_chash" } : unit -> hash
val journal_top_slot = impure { c: "journal_top_slot" } : unit -> word
val journal_top_val = impure { c: "journal_top_val" } : unit -> word
val journal_top_refund = impure { c: "journal_top_refund" } : unit -> bits(64)

/* journal entry tags, shared with ffi/kernel_state.c (MUST stay in sync). */
let JT_EMPTY : bits(64) = 0x0000000000000000
let JT_CHECK : bits(64) = 0x0000000000000001
let JT_ACCT : bits(64) = 0x0000000000000002
let JT_TRAN : bits(64) = 0x0000000000000003
let JT_WARMA : bits(64) = 0x0000000000000004
let JT_WARMS : bits(64) = 0x0000000000000005
let JT_LOG : bits(64) = 0x0000000000000006
let JT_REFUND : bits(64) = 0x0000000000000007
let JT_SELFD : bits(64) = 0x0000000000000008
let JT_STOR : bits(64) = 0x0000000000000009

/* A LOG record (Yellow Paper section 4.4.1, the log series): emitting address,
 * 0..4 topics (LOG0..LOG4), and the data bytes. */
struct LogEntry     = { address : address, topics : list(word), data : list(byte) }

enum EnvField = {
    F_Number,
    F_Timestamp,
    F_Coinbase,
    F_BaseFee,
    F_ChainId,
    F_GasLimit,
    F_PrevRandao,
    F_Origin,
    F_GasPrice,
    F_SlotNumber, /* EIP-7843 SLOTNUM */
}

/* BLOBBASEFEE (EIP-4844) is NOT an EnvField: it is derived from the header's
 * excess_blob_gas via the gas-schedule blob_base_fee(), so it is computed at the
 * EVM layer (the opcode + transaction.sail), never in the host kernel. */

/* Per-transaction environment: ORIGIN/GASPRICE (YP I_o, I_p) + the EIP-4844
 * blob versioned hashes the BLOBHASH opcode reads. */
struct TxEnv = { origin : address, gas_price : word, blob_hashes : list(word) }

/* Journal (undo-log) entry: one reversible state mutation. A call frame's
 * effects must roll back atomically on REVERT or an exceptional halt; rather
 * than snapshot the whole world per frame (O(state)), each mutation records a
 * small undo entry. k_snapshot pushes a JCheck checkpoint marker, k_revert
 * replays undos back to it, k_commit drops the marker so the frame's entries
 * fold into the parent (a later parent revert still undoes them). This is the
 * EELS journal model: O(changes), not O(state).
 *
 * JEntry is the kernel-facing type: the kernel journals a mutation as
 * `journal_push(JAcct(..))` and k_revert dispatches undos with `match e`. The
 * entries themselves are NOT held in a Sail list -- the journal lives in the
 * C-backed store (ffi/kernel_state.c; the journal_* bindings above). A thin
 * adapter (journal_push / journal_decode_top, below) encodes one JEntry into
 * the scalar push ops and decodes the top C entry back into a JEntry, so a
 * JEntry is only ever an ephemeral view -- built for a push, decoded for a
 * revert -- never a stored buffer. */
union JEntry = {
  JCheck  : unit,                     /* call-frame checkpoint marker            */
  JAcct   : (address, Account),       /* restore an account to its prior value   */
  JTran   : (address, word, word),    /* restore transient slot                  */
  JStor   : (address, word, word),    /* restore a storage slot's current value  */
  JWarmA  : address,                  /* un-warm a newly-accessed address        */
  JWarmS  : (address, word),          /* un-warm a newly-accessed slot           */
  JLog    : unit,                     /* drop the most recent log                */
  JRefund : int,                      /* restore the refund counter              */
  JSelfD  : unit                      /* drop the most recent selfdestruct       */
}

/* ----------------------------- kernel state ----------------------------- */

let ZERO_WORD : word = 0x0000000000000000000000000000000000000000000000000000000000000000
let ZERO_ADDR : address = 0x0000000000000000000000000000000000000000
let EMPTY_ACCOUNT : Account = struct {
        nonce = 0,
        balance = ZERO_WORD,
        code_hash = KECCAK_EMPTY,
        storage_root = EMPTY_TRIE_ROOT,
    }

/* the account working set is the C-backed eest_account (ffi/state_db.c), keyed by
 * keccak(address) -- no Sail assoc list. The EIP-2929 warm address/slot sets,
 * the LOG series, and the SELFDESTRUCT/created sets and journal are likewise
 * C-backed (ffi/kernel_state.c; the bindings above). */

/* Store code `c` in the content-addressed code store under its keccak hash `h`.
 * If `h` is already present (identical bytes) the byte stream is skipped
 * (code_db_stream_code_begin returns 0) -- the store is append-only and de-duplicated. */
function code_db_store_code(h : hash, c : list(byte)) -> unit = {
    if code_db_stream_code_begin(h) == 0x0000000000000000 then {
        ()
    } else {
        var rest : list(byte) = c;
        var more : bool = true;
        while more do {
            match rest {
                b :: t => {
                    code_db_stream_code_byte(b);
                    rest = t
                },
                [||] => more = false,
            }
        }
    }
}

/* Store a witness code blob into the content-addressed code store. The spec
 * backend materializes the witness bytes; the build backend may hash/copy the
 * witness slice directly. */
val code_db_store_witness_code : witness_slice -> unit

/* Write-set overlay lookups as option(word): Some(v) = the working set holds a
 * value for the slot (a stored 0 is Some(0x..0), distinct from an absent slot),
 * None = miss -> the caller resolves the pre-state via stateless_storage. This is
 * why the overlay cannot just return a bare word: 0 is a valid value, so presence
 * is a separate signal. The present pass and the load share the memoized slot
 * keccak (storage_secure_key), so the pair is one keccak + a second cheap get.
 * storage_wset_get is the live view (this tx over committed block writes = SLOAD);
 * storage_wset_base_get is the block base only (EIP-2200 tx-start original). */
function storage_wset_get(a : address, s : word) -> option(word) =
    if storage_wset_present(a, s) then {
        Some(storage_wset_load(a, s))
    } else {
        None()
    }
function storage_wset_base_get(a : address, s : word) -> option(word) =
    if storage_wset_base_present(a, s) then {
        Some(storage_wset_base_load(a, s))
    } else {
        None()
    }

/* wipe an address's storage in the write-set overlay (tx + block). Used by the
 * EIP-6780 tx-end account deletion; without the overlay wipe a created+
 * selfdestructed account's writes would still merge into the block base, and a
 * same-block re-CREATE would inherit stale storage. The pre-state (node DB) is
 * not mutated -- a deleted account's storage_root goes empty in the overlay. */
function storage_wipe_addr(a : address) -> unit =
    storage_wset_wipe_addr(a)

function transient_reset() -> unit = transient_storage_reset(get_slice_int(64, 0, 0))

function transient_load(a : address, s : word) -> word = transient_storage_read(get_slice_int(64, 0, 0), a, s)

function transient_store(a : address, s : word, v : word) -> unit =
    transient_storage_write(get_slice_int(64, 0, 0), a, s, v)

/* Stateless storage resolver: the value of (a, s) in the witness pre-state,
 * walked from the account's authenticated storage root. Defined in lib/mpt.sail
 * (the witness reader, included later); declared here so the kernel's storage
 * reads can fall through to it on a working-set miss. */
val stateless_storage : (address, word) -> word

/* write-set block enumeration for compute_root (host/base_*.sail storage_updates):
 * dirty rows = the net change over the authenticated MPT (witness); the
 * cache-union-block = the full post-state over the empty anchor (native). */
function storage_wset_dirty_rows(ak : bits(256)) -> int = unsigned(storage_wset_block_dirty_count(ak))
function storage_wset_dirty_row(ak : bits(256), j : int) -> (word, word) =
    (
        storage_wset_block_dirty_slot(ak, get_slice_int(64, j, 0)),
        storage_wset_block_dirty_val(ak, get_slice_int(64, j, 0)),
    )
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

/* one enumerated account row (dump_state / state root; cold path). Rows come
 * from the account write-set block base -- acct_wset_dirty_row (witness) and
 * acct_wset_union_row (native) below. */
struct AcctRow = {
    row_hkey : hash,
    row_nonce : bits(64),
    row_bal : word,
    row_sroot : hash,
    row_chash : hash,
    row_base_exists : bool,
}

/* account write-set block enumeration for compute_root (host/base_*.sail
 * state_updates_map): dirty rows = the net change vs pre-state (witness); the
 * cache-union-block = the full post-state accounts (native). */
function acct_wset_dirty_row(j : int) -> AcctRow = {
    let idx = get_slice_int(64, j, 0);
    struct {
        row_hkey = acct_wset_block_dirty_hkey(idx),
        row_nonce = acct_wset_block_dirty_nonce(idx),
        row_bal = acct_wset_block_dirty_bal(idx),
        row_sroot = acct_wset_block_dirty_sroot(idx),
        row_chash = acct_wset_block_dirty_chash(idx),
        row_base_exists = acct_wset_block_dirty_base_exists(idx),
    }
}
register k_refund : int = 0 /* gas-refund counter (SSTORE clears; EIP-3529 cap applied at tx end) */

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

/* number of authenticated ancestor headers available to BLOCKHASH (k_blockhash,
 * host/base.sail). Both entries publish it from the SSZ witness header chain in
 * authenticate_parent_state() (k_n_headers = wit_n_headers()): the guest carries
 * its real ancestor chain; the runner a single dummy parent (count 1). A request
 * beyond the witness is a deficient proof -> throw InvalidBlock(WitnessDeficient).
 * Data, not a per-backend k_blockhash definition. */
register k_n_headers : int = 0

register k_fork : int = 99 /* active fork level (higher = newer); set by the runner */

/* fork levels for fork-gated rules (chronological; only thresholds matter) */
let FORK_FRONTIER : int = 0
let FORK_HOMESTEAD : int = 1
let FORK_BYZANTIUM : int = 2
let FORK_CONSTANTINOPLE : int = 3
let FORK_ISTANBUL : int = 4
let FORK_BERLIN : int = 5 /* EIP-2929/2930 access lists + warm/cold access costs */
let FORK_LONDON : int = 6 /* EIP-1559 fee market + EIP-3529 refund reduction */
let FORK_PARIS : int = 7
let FORK_SHANGHAI : int = 8 /* EIP-3651 warm coinbase, EIP-3855 PUSH0, EIP-3860 initcode */
let FORK_CANCUN : int = 9 /* EIP-1153/4844; precompiles 0x01-0x0a */
let FORK_PRAGUE : int = 10 /* EIP-7623 calldata floor; +BLS 0x0b-0x11 */
let FORK_OSAKA : int = 11 /* EIP-7883 modexp gas, EIP-7825 cap; +0x100 */
let FORK_AMSTERDAM : int = 12 /* EIP-7954 code/initcode size bump (32768/65536) */

/* Is `n` (the low bytes of an address, as an int) a precompile at the active
 * fork? The precompile set GROWS by fork: Frontier has 0x01-0x04, Byzantium
 * adds 0x05-0x08, Istanbul adds 0x09, Cancun adds 0x0a POINT_EVALUATION,
 * Prague adds the EIP-2537 BLS12-381 set 0x0b-0x11
 * (11-17), and Osaka adds 0x100 P256VERIFY (EIP-7951). A CALL to an address
 * OUTSIDE the active set is an ordinary call to a (typically empty) account,
 * not a precompile. */
function is_precompile(n : int) -> bool = {
    let base = (n >= 1) & (n <= 4);
    let byzantium = (n >= 5) & (n <= 8) & (k_fork >= FORK_BYZANTIUM);
    let istanbul = (n == 9) & (k_fork >= FORK_ISTANBUL);
    let kzg = (n == 10) & (k_fork >= FORK_CANCUN);
    let bls = (n >= 11) & (n <= 17) & (k_fork >= FORK_PRAGUE);
    let p256 = (n == 256) & (k_fork >= FORK_OSAKA);
    base | byzantium | istanbul | kzg | bls | p256
}

/* The full consensus block header: the sole per-block environment. The EVM reads
 * its execution-observable subset through k_env / k_coinbase (kernel.sail), which
 * project + type-normalize these fields (int->word, address->word, chain_id from
 * chain config); block validation reads the full header (block hash, roots, output
 * checks). Set once per block from the SSZ payload (k_set_header in decode_block_ssz). */
register k_header : BlockHeader =
    struct {
        number = 0,
        timestamp = 0,
        gas_limit = 0,
        gas_used = 0,
        prev_randao = ZERO_WORD,
        base_fee = ZERO_WORD,
        blob_gas_used = 0,
        excess_blob_gas = 0,
        state_root = ZERO_WORD,
        receipts_root = ZERO_WORD,
        logs_bloom = [||],
        block_hash = ZERO_WORD,
        fee_recipient = ZERO_ADDR,
        parent_hash = ZERO_WORD,
        parent_beacon_block_root = ZERO_WORD,
        slot_number = 0,
    }
register k_tx : TxEnv = struct { origin = ZERO_ADDR, gas_price = ZERO_WORD, blob_hashes = [||] }

/* ----------------------------- assoc helpers ---------------------------- */

/* the account bound to `a` (EMPTY_ACCOUNT if absent), read from the account
 * working set. A base pre-state leaf is materialized into the working set as a
 * block-level read member on first touch (acct_wset_seed_read, in the base
 * resolver stateless_account_load), so the overlay is the single source. */
function account_lookup(a : address) -> Account = {
    if acct_wset_present(a) then {
        struct {
            nonce = unsigned(acct_wset_nonce(a)),
            balance = acct_wset_bal(a),
            code_hash = acct_wset_chash(a),
            storage_root = acct_wset_sroot(a),
        }
    } else {
        EMPTY_ACCOUNT
    }
}

/* whether `a` has been materialized as a working-set read member or written;
 * distinct from account_lookup, which returns EMPTY_ACCOUNT for absent and
 * not-yet-loaded alike. Gates the resolve-once in load_account. */
function account_loaded(a : address) -> bool = acct_wset_present(a)

/* Stateless account materialization (defined in lib/mpt.sail, the witness reader,
 * included later): resolve `a` from the authenticated witness leaf store and bind
 * it (nonce/balance/storageRoot/codeHash); absent in the witness -> empty. The
 * witness IS the base, so load_account loads on first touch -- no eager seeding. */
val stateless_account_load : address -> unit

/* internal (un-journaled) account accessor used inside the kernel functions.
 * Base-layer contract (defined per entry): the native backend reads the seeded
 * map directly (absent == EMPTY_ACCOUNT); the witness backend materializes a
 * not-yet-loaded account from the witness on first access (un-journaled, so a
 * frame revert keeps the revealed pre-state and only undoes recorded
 * mutations). */
val load_account : address -> Account

/* a mutation writes the per-tx account overlay (acct_wset); the first write this
 * tx freezes the tx-start original C-side. Reverts run through the JAcct journal
 * (apply_undo below restores the overlay current), not the resolver cache. */
function store_account(a : address, v : Account) -> unit =
    acct_wset_write(a, get_slice_int(64, v.nonce, 0), v.balance, v.storage_root, v.code_hash)

/* --- journal (undo log) adapter: JEntry <-> the C-backed scalar journal ---
 * The kernel works in JEntry; these functions are the only place a JEntry is
 * (en/de)coded into the scalar push / top-getter journal_* ops above.
 * No JEntry is ever stored -- push encodes one and returns; k_revert decodes
 * the top entry, matches on it, and drops it. */

/* encode one undo entry into the C journal (kernel side: journal_push(JAcct ..)) */
function journal_push(e : JEntry) -> unit = match e {
    JCheck()       => journal_push_check(),
    JAcct(a, old)  => journal_push_acct(a, get_slice_int(64, old.nonce, 0), old.balance, old.storage_root, old.code_hash),
    JTran(a, s, v) => journal_push_tran(a, s, v),
    JStor(a, s, p) => journal_push_stor(a, s, p),
    JWarmA(a)      => journal_push_warma(a),
    JWarmS(a, s)   => journal_push_warms(a, s),
    JLog()         => journal_push_log(),
    JRefund(old)   => journal_push_refund(get_slice_int(64, old, 0)),
    JSelfD()       => journal_push_selfd(),
}

/* decode the journal's TOP entry into a JEntry (the payload getters are
 * tag-indexed views over that one C entry record). The caller guards this with
 * journal_top_tag() != JT_EMPTY, so the empty case never reaches here (it maps
 * to JCheck, a no-op undo, purely to keep the match total). */
function journal_decode_top(tag : bits(64)) -> JEntry =
    if tag == JT_ACCT then {
        JAcct(
            journal_top_addr()[159 .. 0],
            struct {
                nonce = unsigned(journal_top_nonce()),
                balance = journal_top_balance(),
                code_hash = journal_top_chash(),
                storage_root = journal_top_sroot(),
            },
        )
    } else {
        if tag == JT_TRAN then {
            JTran(journal_top_addr()[159 .. 0], journal_top_slot(), journal_top_val())
        } else {
            if tag == JT_STOR then {
                JStor(journal_top_addr()[159 .. 0], journal_top_slot(), journal_top_val())
            } else {
                if tag == JT_WARMA then {
                    JWarmA(journal_top_addr()[159 .. 0])
                } else {
                    if tag == JT_WARMS then {
                        JWarmS(journal_top_addr()[159 .. 0], journal_top_slot())
                    } else {
                        if tag == JT_LOG then {
                            JLog()
                        } else {
                            if tag == JT_REFUND then {
                                JRefund(signed(journal_top_refund()))
                            } else {
                                if tag == JT_SELFD then {
                                    JSelfD()
                                } else {
                                    JCheck() /* JT_CHECK / JT_EMPTY: no undo effect */
                                }
                            }
                        }
                    }
                }
            }
        }
    }

/* Apply one undo entry (k_revert replays these back to the checkpoint). */
function apply_undo(e : JEntry) -> unit = match e {
    JCheck()       => (),
    JAcct(a, old)  => store_account(a, old),
    JTran(a, s, v) => transient_store(a, s, v),
    JStor(a, s, p) => storage_wset_restore(a, s, p),
    JWarmA(a)      => warm_addr_remove(a),
    JWarmS(a, s)   => warm_slot_remove(a, s),
    JLog()         => log_drop_last(),
    JRefund(old)   => k_refund = old,
    JSelfD()       => selfdestr_drop_last(),
}

/* --- LOG series helpers: stream a record into the C log store, and read the
 * whole ordered list back out (into the receipt) via the count/getters. --- */
function log_stream_topics(ts : list(word)) -> unit =
    match ts {
        [||]   => (),
        t :: r => {
            log_add_topic(t);
            log_stream_topics(r)
        },
    }
function log_stream_data(bs : list(byte)) -> unit =
    match bs {
        [||]   => (),
        b :: r => {
            log_add_data_byte(b);
            log_stream_data(r)
        },
    }
function emit_log(a : address, topics : list(word), data : list(byte)) -> unit = {
    log_begin(a);
    log_stream_topics(topics);
    log_stream_data(data)
}

/* read one stored log's topics (in order) / data bytes (reversed, caller flips) */
function read_log_topics(i : bits(64), j : int, m : int, acc : list(word)) -> list(word) =
    if j >= m then {
        acc
    } else {
        read_log_topics(i, j + 1, m, acc ++ [|log_topic(i, get_slice_int(64, j, 0))|])
    }
function read_log_data_rev(i : bits(64), k : int, n : int, acc : list(byte)) -> list(byte) =
    if k >= n then {
        acc
    } else {
        read_log_data_rev(i, k + 1, n, log_data_byte(i, get_slice_int(64, k, 0)) :: acc)
    }
function read_log_entry(i : bits(64)) -> LogEntry =
    struct {
        address = log_addr(i)[159 .. 0],
        topics = read_log_topics(i, 0, unsigned(log_topic_count(i)), [||]),
        data = list_reverse(read_log_data_rev(i, 0, unsigned(log_data_len(i)), [||])),
    }
function read_logs_rev(i : int, n : int, acc : list(LogEntry)) -> list(LogEntry) =
    if i >= n then {
        acc
    } else {
        read_logs_rev(i + 1, n, read_log_entry(get_slice_int(64, i, 0)) :: acc)
    }
function read_logs() -> list(LogEntry) = list_reverse(read_logs_rev(0, unsigned(log_count()), [||]))
