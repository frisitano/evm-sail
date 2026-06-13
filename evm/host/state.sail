/* ======================================================================== */
/* HOST STATE                                                                */
/*                                                                           */
/* The host's privileged world state: accounts, the C-backed overlay maps    */
/* (persistent storage, transient storage, the EIP-2929 warm-slot set), the  */
/* warm-address set, logs, refunds, the journal (undo log), and the block/tx */
/* environment. The transaction kernel (the EVM) reaches this state ONLY     */
/* through the host kernel functions in host/kernel.sail.                    */
/* ======================================================================== */

/* ------------------------------ structs --------------------------------- */

/* Account state (Yellow Paper sec. 4.1, the tuple sigma[a]). storageRoot is
 * omitted: storage lives in the overlay maps and its root is computed on demand
 * (state_root.sail). Code BYTES are not held here -- the account carries only
 * codeHash, a content address into the code store (host_code.c); codeHash is
 * itself observable state (EXTCODEHASH, EIP-1052) and the binding a stateless
 * witness checks code against. */
struct Account = {
    acc_nonce    : int,   /* sigma[a]_n: nonce (txs sent / contracts created)      */
    acc_balance  : word,  /* sigma[a]_b: balance in wei                            */
    acc_codehash : hash,  /* sigma[a]_c: keccak256(code); KECCAK_EMPTY if codeless */
}

/* keccak256 of the empty string: the codeHash of every codeless account */
let KECCAK_EMPTY : hash = 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470

/* Host bookkeeping (not protocol state): the account table is a Sail assoc list
 * (storage/transient/warm sets are C-backed maps; see below). */
struct AccountEntry = { ae_addr : address, ae_acc : Account }
struct StorageEntry = { se_addr : address, se_slot : word, se_val : word }

/* SlotKey (the BAL access-set key) is defined in host/io.sail (its bal_load
 * builder is the producer), included before this file. */

/* A LOG record (YP sec. 4.4.1): emitting address, 0..4 topics, data bytes. */
struct LogEntry     = { le_addr : address, le_topics : list(word), le_data : list(byte) }

enum EnvField = {
    F_Number,
    F_Timestamp,
    F_Coinbase,
    F_BaseFee,
    F_ChainId,
    F_GasLimit,
    F_PrevRandao,
    F_BlobBaseFee,
    F_Origin,
    F_GasPrice,
}

/* The block environment the EVM reads (BLOCKHASH..BLOBBASEFEE opcodes).
 * be_base_fee: EIP-1559; be_prev_randao: EIP-4399 (PREVRANDAO replaced DIFFICULTY);
 * be_blob_base_fee/be_excess_blob_gas: EIP-4844. */
struct BlockEnv = {
    be_number : word, be_timestamp : word, be_gas_limit : word, be_coinbase : address,
    be_base_fee : word, be_prev_randao : word, be_chain_id : word,
    be_blob_base_fee : word, be_excess_blob_gas : word,
}

/* Per-transaction environment: ORIGIN/GASPRICE (YP I_o, I_p) + the EIP-4844
 * blob versioned hashes the BLOBHASH opcode reads. */
struct TxEnv = { txe_origin : address, txe_gas_price : word, txe_blob_hashes : list(word) }

/* Everything a reverting frame must roll back atomically. */
/* Journal-based revert: instead of deep-copying the whole world per call frame
 * (O(state) per snapshot), each mutation records a small undo entry; k_snapshot
 * pushes a checkpoint marker, k_revert replays undos back to it, k_commit drops
 * the marker (the frame's entries fold into the parent). O(changes), not O(state). */
union JEntry = {
  JCheck  : unit,                     /* call-frame checkpoint marker            */
  JAcct   : (address, Account),       /* restore an account to its prior value   */
  JStor   : (address, word, word),    /* restore storage slot to its prior value */
  JTran   : (address, word, word),    /* restore transient slot                  */
  JWarmA  : address,                  /* un-warm a newly-accessed address        */
  JWarmS  : (address, word),          /* un-warm a newly-accessed slot           */
  JLog    : unit,                     /* drop the most recent log                */
  JRefund : int,                      /* restore the refund counter              */
  JSelfD  : unit                      /* drop the most recent selfdestruct       */
}

/* ----------------------------- kernel state ----------------------------- */

let ZERO_WORD : word = 0x0000000000000000000000000000000000000000000000000000000000000000
let ZERO_ADDR : address = 0x0000000000000000000000000000000000000000
let EMPTY_ACCOUNT : Account = struct { acc_nonce = 0, acc_balance = ZERO_WORD, acc_codehash = KECCAK_EMPTY }

register k_accounts : list(AccountEntry) = [||]
register k_warm_addr : list(address) = [||]
register k_logs : list(LogEntry) = [||]

/* ---- C-backed (address, slot) maps (ffi/host_map.c): O(1) per access ----
 * id 0 = EIP-1153 transient storage, id 1 = the EIP-2929 warm-slot set.
 * Sail assoc lists made these O(n) per access -- O(n^2) over a TSTORE loop or a
 * large access-list prewarm. "Absent" == "value 0" for both (TLOAD of an unset
 * slot is 0; un-warm = 0), so journal undo just stores the prior value. */
val host_map_reset = impure { _: "host_map_reset" } : bits(64) -> unit
val host_map_push = impure { _: "host_map_push" } : bits(64) -> unit
val host_map_pop_commit = impure { _: "host_map_pop_commit" } : bits(64) -> unit
val host_map_pop_discard = impure { _: "host_map_pop_discard" } : bits(64) -> unit
val host_map_key = impure { _: "host_map_key" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
val host_map_store = impure { _: "host_map_store" } : (bits(64), bits(64), bits(64), bits(64)) -> unit
val host_map_word = impure { _: "host_map_word" } : bits(64) -> bits(64)
val host_map_base_word = impure { _: "host_map_base_word" } : bits(64) -> bits(64)
val host_map_present = impure { _: "host_map_present" } : unit -> bits(64)
val host_map_has_addr = impure { _: "host_map_has_addr" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(64)
val host_map_wipe_addr = impure { _: "host_map_wipe_addr" } : (bits(64), bits(64), bits(64), bits(64)) -> unit
val host_map_count = impure { _: "host_map_count" } : bits(64) -> bits(64)
val host_map_sel = impure { _: "host_map_sel" } : bits(64) -> unit
val host_map_it_key = impure { _: "host_map_it_key" } : bits(64) -> bits(64)
val host_map_it_val = impure { _: "host_map_it_val" } : bits(64) -> bits(64)

/* ---- C code store (ffi/host_code.c): contract code, content-addressed --------
 * Keyed by codeHash (not address): each entry holds the code bytes AND the
 * JUMPDEST bitmap (built once per code WRITE -- seeding, CREATE deploys, EIP-7702
 * delegations). Content-addressing makes code de-duplicated and append-only, so
 * REVERT needs no code rollback: restoring the account's codeHash restores its
 * code. An account resolves its code by codeHash (k_code_key). */
val cs_reset = impure { _: "cs_reset" } : unit -> unit
val cs_begin = impure { _: "cs_begin" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(64)
val cs_len = impure { _: "cs_len" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(64)
val cs_to_mem = impure { _: "cs_to_mem" } : (bits(64), bits(64), bits(64), bits(64), bits(64), bits(64), bits(64)) -> unit
val cs_byte = impure { _: "cs_byte" } : bits(8) -> unit

/* store `c` in the content-addressed code store under its keccak `h`; if the
 * hash is already present (same bytes) the stream is skipped (cs_begin == 0). */
function cs_store_code(h : hash, c : list(byte)) -> unit = {
    if cs_begin(h[255 .. 192], h[191 .. 128], h[127 .. 64], h[63 .. 0]) == 0x0000000000000000 then {
        ()
    } else {
        var rest : list(byte) = c;
        var more : bool = true;
        while more do {
            match rest {
                b :: t => {
                    cs_byte(b);
                    rest = t
                },
                [||] => more = false,
            }
        }
    }
}

let MAP_TRANSIENT : int = 0 /* EIP-1153 transient storage (TLOAD/TSTORE)   */
let MAP_WARM_SLOT : int = 1 /* EIP-2929 warm storage-slot set              */
let MAP_STORAGE : int = 2 /* persistent storage: OVERLAY layers (push at snapshot,
                             pop_commit/pop_discard at commit/revert); the base
                             layer = tx-start state = EIP-2200 originals. */

function map_select(id : int, a : address, s : word) -> unit = {
    let aw : word = sail_zero_extend(a, 256);
    host_map_key(
        get_slice_int(64, id, 0),
        aw[191 .. 128],
        aw[127 .. 64],
        aw[63 .. 0],
        s[255 .. 192],
        s[191 .. 128],
        s[127 .. 64],
        s[63 .. 0],
    )
}
function map_load(id : int, a : address, s : word) -> word = {
    map_select(id, a, s);
    append(
        host_map_word(get_slice_int(64, 3, 0)),
        append(
            host_map_word(get_slice_int(64, 2, 0)),
            append(host_map_word(get_slice_int(64, 1, 0)), host_map_word(get_slice_int(64, 0, 0))),
        ),
    )
}
function map_store(id : int, a : address, s : word, v : word) -> unit = {
    map_select(id, a, s);
    host_map_store(v[255 .. 192], v[191 .. 128], v[127 .. 64], v[63 .. 0])
}

/* value in the BASE layer only (the state at tx start -- EIP-2200 original) */
function map_base_load(id : int, a : address, s : word) -> word = {
    map_select(id, a, s);
    append(
        host_map_base_word(get_slice_int(64, 3, 0)),
        append(
            host_map_base_word(get_slice_int(64, 2, 0)),
            append(host_map_base_word(get_slice_int(64, 1, 0)), host_map_base_word(get_slice_int(64, 0, 0))),
        ),
    )
}

/* present in ANY layer (an explicit 0 is present; fail-closed stateless test) */
function map_present(id : int, a : address, s : word) -> bool = {
    map_select(id, a, s);
    unsigned(host_map_present()) == 1
}
function map_has_addr(id : int, a : address) -> bool = {
    let aw : word = sail_zero_extend(a, 256);
    unsigned(host_map_has_addr(get_slice_int(64, id, 0), aw[191 .. 128], aw[127 .. 64], aw[63 .. 0])) == 1
}
function map_wipe_addr(id : int, a : address) -> unit = {
    let aw : word = sail_zero_extend(a, 256);
    host_map_wipe_addr(get_slice_int(64, id, 0), aw[191 .. 128], aw[127 .. 64], aw[63 .. 0])
}

/* flattened storage enumeration (dump_state / state root; cold path) */
function storage_count() -> int = unsigned(host_map_count(get_slice_int(64, MAP_STORAGE, 0)))
function storage_entry(j : int) -> (address, word, word) = {
    host_map_sel(get_slice_int(64, j, 0));
    let aw : word = sail_zero_extend(
        append(
            host_map_it_key(get_slice_int(64, 0, 0)),
            append(host_map_it_key(get_slice_int(64, 1, 0)), host_map_it_key(get_slice_int(64, 2, 0))),
        ),
        256,
    );
    let s : word = append(
        host_map_it_key(get_slice_int(64, 3, 0)),
        append(
            host_map_it_key(get_slice_int(64, 4, 0)),
            append(host_map_it_key(get_slice_int(64, 5, 0)), host_map_it_key(get_slice_int(64, 6, 0))),
        ),
    );
    let v : word = append(
        host_map_it_val(get_slice_int(64, 3, 0)),
        append(
            host_map_it_val(get_slice_int(64, 2, 0)),
            append(host_map_it_val(get_slice_int(64, 1, 0)), host_map_it_val(get_slice_int(64, 0, 0))),
        ),
    );
    (word_to_address(aw), s, v)
}
register k_refund : int = 0
register k_selfdestr : list(address) = [||]
register k_created : list(address) =
    [||] /* accounts CREATEd this tx (EIP-6780).
            NOT journaled: like EELS created_accounts,
            the marker survives a frame revert. */

register k_journal : list(JEntry) = [||] /* undo log (call-frame revert) */
register k_stateless : bool = false /* stateless mode: storage reads must hit the witness working set */
register k_fork : int = 99 /* active fork level (higher = newer); set by the runner */

/* fork levels for fork-gated rules (chronological; only thresholds matter) */
let FORK_PRAGUE : int = 2 /* EIP-7623 calldata floor              */
let FORK_OSAKA : int = 3 /* EIP-7883 modexp gas, EIP-7825 cap    */

register k_block : BlockEnv =
    struct {
        be_number = ZERO_WORD,
        be_timestamp = ZERO_WORD,
        be_gas_limit = ZERO_WORD,
        be_coinbase = ZERO_ADDR,
        be_base_fee = ZERO_WORD,
        be_prev_randao = ZERO_WORD,
        be_chain_id = ZERO_WORD,
        be_blob_base_fee = ZERO_WORD,
        be_excess_blob_gas = ZERO_WORD,
    }
register k_tx : TxEnv = struct { txe_origin = ZERO_ADDR, txe_gas_price = ZERO_WORD, txe_blob_hashes = [||] }

/* ----------------------------- assoc helpers ---------------------------- */

function acc_lookup(es : list(AccountEntry), a : address) -> Account =
    match es {
        [||] => EMPTY_ACCOUNT,
        e :: r => if e.ae_addr == a then {
            e.ae_acc
        } else {
            acc_lookup(r, a)
        },
    }
function acc_update(es : list(AccountEntry), a : address, v : Account) -> list(AccountEntry) =
    match es {
        [||] => [|struct { ae_addr = a, ae_acc = v }|],
        e :: r => if e.ae_addr == a then {
            struct { ae_addr = a, ae_acc = v } :: r
        } else {
            e :: acc_update(r, a, v)
        },
    }
function addr_in(xs : list(address), a : address) -> bool =
    match xs {
        [||] => false,
        x :: r => if x == a then {
            true
        } else {
            addr_in(r, a)
        },
    }
function slot_in(xs : list(SlotKey), a : address, s : word) -> bool =
    match xs {
        [||] => false,
        x :: r => if x.sk_addr == a & x.sk_slot == s then {
            true
        } else {
            slot_in(r, a, s)
        },
    }

/* internal (unrecorded) account accessors used by syscall handlers */
function kget(a : address) -> Account = acc_lookup(k_accounts, a)
function kset(a : address, v : Account) -> unit = k_accounts = acc_update(k_accounts, a, v)

/* --- journal (undo log) helpers --- */
function jpush(e : JEntry) -> unit = k_journal = e :: k_journal
function jremove_addr(xs : list(address), a : address) -> list(address) =
    match xs {
        [||] => [||],
        x :: r => if x == a then {
            r
        } else {
            x :: jremove_addr(r, a)
        },
    }
function jremove_slot(xs : list(SlotKey), a : address, s : word) -> list(SlotKey) =
    match xs {
        [||] => [||],
        x :: r => if x.sk_addr == a & x.sk_slot == s then {
            r
        } else {
            x :: jremove_slot(r, a, s)
        },
    }
function jdrop_last_log(xs : list(LogEntry)) -> list(LogEntry) =
    match xs {
        [||] => [||],
        _ :: [||] => [||],
        x :: r => x :: jdrop_last_log(r),
    }
function jdrop_last_addr(xs : list(address)) -> list(address) =
    match xs {
        [||] => [||],
        _ :: [||] => [||],
        x :: r => x :: jdrop_last_addr(r),
    }
function apply_undo(e : JEntry) -> unit = match e {
    JCheck()       => (),
    JAcct(a, old)  => kset(a, old),
    JStor(_, _, _) => (), /* storage undo is the overlay's pop_discard now */
    JTran(a, s, v) => map_store(MAP_TRANSIENT, a, s, v),
    JWarmA(a)      => k_warm_addr = jremove_addr(k_warm_addr, a),
    JWarmS(a, s)   => map_store(MAP_WARM_SLOT, a, s, ZERO_WORD),
    JLog()         => k_logs = jdrop_last_log(k_logs),
    JRefund(old)   => k_refund = old,
    JSelfD()       => k_selfdestr = jdrop_last_addr(k_selfdestr),
}
