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
/* host/kernel.sail. Large maps (storage, transient storage, warm slots)    */
/* are held in C overlays for O(1) access (host/io.sail FFI); the account   */
/* table and the small sets are Sail association lists.                     */
/* ======================================================================== */

/* ------------------------------ structs --------------------------------- */

/* Account state (Yellow Paper section 4.1, the account tuple sigma[a]).
 * acc_storage_root is the account's PRE-STATE storage root -- the witness anchor
 * that stateless storage reads walk on a working-set miss. It is set when the
 * account is materialized from the witness and is NOT kept live during execution
 * (storage mutates in the overlay maps; the post-state root is computed on demand
 * in state_root.sail). Code BYTES are not held here -- the account carries only
 * codeHash, a content address into the code store; codeHash is itself observable
 * state (EXTCODEHASH, EIP-1052) and the binding a stateless witness checks code
 * against. */
struct Account = {
    acc_nonce        : int,   /* sigma[a]_n: nonce (txs sent / contracts created)      */
    acc_balance      : word,  /* sigma[a]_b: balance in wei                            */
    acc_codehash     : hash,  /* sigma[a]_c: keccak256(code); KECCAK_EMPTY if codeless */
    acc_storage_root : hash,  /* sigma[a]_s: PRE-STATE storage root, the witness anchor */
}

/* keccak256 of the empty string: the codeHash of every codeless account */
let KECCAK_EMPTY : hash = 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470

/* keccak256(rlp("")) -- the root of an empty Merkle-Patricia trie: the storage
 * root of every account with no storage (EMPTY_ACCOUNT, freshly created). */
let EMPTY_TRIE_ROOT : hash = 0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421

/* The account table is the C-backed acctmap (raw address -> Account; see the
 * ffi/host_acctmap.c declarations below); storage/transient/warm sets are the
 * C-backed (address, slot) maps. No Sail association lists for sigma. */

/* SlotKey (the BAL access-set key) is defined in host/io.sail (its bal_load
 * builder is the producer), included before this file. */

/* A LOG record (Yellow Paper section 4.4.1, the log series): emitting address,
 * 0..4 topics (LOG0..LOG4), and the data bytes. */
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
    F_SlotNumber, /* EIP-7843 SLOTNUM */
}

/* The block environment the EVM reads (BLOCKHASH..BLOBBASEFEE opcodes).
 * be_base_fee: EIP-1559; be_prev_randao: EIP-4399 (PREVRANDAO replaced DIFFICULTY);
 * be_blob_base_fee/be_excess_blob_gas: EIP-4844. */
struct BlockEnv = {
    be_number : word, be_timestamp : word, be_gas_limit : word, be_coinbase : address,
    be_base_fee : word, be_prev_randao : word, be_chain_id : word,
    be_blob_base_fee : word, be_excess_blob_gas : word, be_slot : word,
}

/* Per-transaction environment: ORIGIN/GASPRICE (YP I_o, I_p) + the EIP-4844
 * blob versioned hashes the BLOBHASH opcode reads. */
struct TxEnv = { txe_origin : address, txe_gas_price : word, txe_blob_hashes : list(word) }

/* Journal (undo-log) entry: one reversible state mutation. A call frame's
 * effects must roll back atomically on REVERT or an exceptional halt; rather
 * than snapshot the whole world per frame (O(state)), each mutation records a
 * small undo entry. k_snapshot pushes a JCheck checkpoint marker, k_revert
 * replays undos back to it, k_commit drops the marker so the frame's entries
 * fold into the parent (a later parent revert still undoes them). This is the
 * EELS journal model: O(changes), not O(state). */
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
let EMPTY_ACCOUNT : Account = struct {
        acc_nonce = 0,
        acc_balance = ZERO_WORD,
        acc_codehash = KECCAK_EMPTY,
        acc_storage_root = EMPTY_TRIE_ROOT,
    }

/* the account working set is the C-backed acctmap (ffi/host_acctmap.c), keyed by
 * raw address -- no Sail assoc list. */
register k_warm_addr : list(address) = [||]
register k_logs : list(LogEntry) = [||]

/* ---- C-backed (address, slot) maps (ffi/host_map.c): O(1) per access ----
 * id 0 = EIP-1153 transient storage, id 1 = the EIP-2929 warm-slot set, id 2 =
 * persistent storage (see MAP_* below). For every map "absent" == "value 0"
 * (TLOAD of an unset slot is 0; a cold slot's warm bit is 0), so a journal undo
 * is just a store of the prior value. */
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
val host_map_base_present = impure { _: "host_map_base_present" } : unit -> bits(64)
val host_map_has_addr = impure { _: "host_map_has_addr" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(64)
val host_map_wipe_addr = impure { _: "host_map_wipe_addr" } : (bits(64), bits(64), bits(64), bits(64)) -> unit
val host_map_count = impure { _: "host_map_count" } : bits(64) -> bits(64)
val host_map_sel = impure { _: "host_map_sel" } : bits(64) -> unit
val host_map_it_key = impure { _: "host_map_it_key" } : bits(64) -> bits(64)
val host_map_it_val = impure { _: "host_map_it_val" } : bits(64) -> bits(64)

/* ---- C-backed raw-address -> account map (ffi/host_acctmap.c): the kernel
 * account working set (k_accounts), keyed by the RAW address so reads/writes
 * never hash. O(1)-amortized open addressing; the witness pre-state is the base
 * it sits over (materialized once per account, then cached here). An account is
 * (nonce, balance, storage_root, code_hash): nonce one word, the 256-bit fields
 * four big-endian words each (w3 most significant). Point access selects the key
 * once (acctmap_key) then the accessors resolve it; iteration feeds the post-state
 * root. ---- */
val acctmap_reset = impure { _: "acctmap_reset" } : unit -> unit
val acctmap_key = impure { _: "acctmap_key" } : (bits(64), bits(64), bits(64)) -> unit
val acctmap_present = impure { _: "acctmap_present" } : unit -> bits(64)
val acctmap_store = impure { _: "acctmap_store" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
val acctmap_nonce = impure { _: "acctmap_nonce" } : unit -> bits(64)
val acctmap_bal = impure { _: "acctmap_bal" } : bits(64) -> bits(64)
val acctmap_sroot = impure { _: "acctmap_sroot" } : bits(64) -> bits(64)
val acctmap_chash = impure { _: "acctmap_chash" } : bits(64) -> bits(64)
val acctmap_remove = impure { _: "acctmap_remove" } : (bits(64), bits(64), bits(64)) -> unit
val acctmap_count = impure { _: "acctmap_count" } : unit -> bits(64)
val acctmap_at = impure { _: "acctmap_at" } : bits(64) -> unit
val acctmap_at_key = impure { _: "acctmap_at_key" } : bits(64) -> bits(64)
val acctmap_at_nonce = impure { _: "acctmap_at_nonce" } : unit -> bits(64)
val acctmap_at_bal = impure { _: "acctmap_at_bal" } : bits(64) -> bits(64)
val acctmap_at_chash = impure { _: "acctmap_at_chash" } : bits(64) -> bits(64)

/* ---- C code store (ffi/host_code.c): contract code, content-addressed --------
 * Keyed by codeHash (not address): each entry holds the code bytes AND the
 * JUMPDEST validity bitmap (built once per code WRITE -- pre-state seeding,
 * CREATE deploys, EIP-7702 delegations). Content-addressing makes the store
 * de-duplicated and append-only, so REVERT needs no code rollback: restoring an
 * account's codeHash restores its code. An account resolves its code by codeHash
 * (k_code_key). */
val cs_reset = impure { _: "cs_reset" } : unit -> unit
val cs_begin = impure { _: "cs_begin" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(64)
val cs_len = impure { _: "cs_len" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(64)
val cs_to_mem = impure { _: "cs_to_mem" } : (bits(64), bits(64), bits(64), bits(64), bits(64), bits(64), bits(64)) -> unit
val cs_byte = impure { _: "cs_byte" } : bits(8) -> unit

/* Store code `c` in the content-addressed code store under its keccak hash `h`.
 * If `h` is already present (identical bytes) the byte stream is skipped
 * (cs_begin returns 0) -- the store is append-only and de-duplicated. */
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
let MAP_STORAGE : int = 2 /* persistent storage, kept as a stack of OVERLAY layers
                             (push at k_snapshot, pop_commit/pop_discard at frame
                             commit/revert); the base layer is tx-start state, i.e.
                             the EIP-2200 "original" value of every slot. */

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

/* present in the BASE layer only (EIP-2200 "original" presence; a committed base
 * 0 is present, a never-committed slot is absent and resolved from the witness) */
function map_base_present(id : int, a : address, s : word) -> bool = {
    map_select(id, a, s);
    unsigned(host_map_base_present()) == 1
}

/* Stateless storage resolver: the value of (a, s) in the witness pre-state,
 * walked from the account's authenticated storage root. Defined in lib/mpt.sail
 * (the witness reader, included later); declared here so the kernel's storage
 * reads can fall through to it on a working-set miss. */
val stateless_storage : (address, word) -> word
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
register k_refund : int = 0 /* gas-refund counter (SSTORE clears; EIP-3529 cap applied at tx end) */
register k_selfdestr : list(address) = [||] /* addresses SELFDESTRUCTed this tx */

register k_created : list(address) =
    [||] /* accounts CREATEd this tx (EIP-6780).
            NOT journaled: like EELS created_accounts,
            the marker survives a frame revert. */

register k_journal : list(JEntry) = [||] /* undo log (call-frame revert) */
register k_stateless : bool = false /* stateless mode: storage reads must hit the witness working set */
register k_witness_bad : bool =
    false /* witness proof deficient/inconsistent: validation FAILS (valid=false)
           gracefully instead of aborting (EIP-8025 optional-proofs tests) */

register k_fork : int = 99 /* active fork level (higher = newer); set by the runner */

/* fork levels for fork-gated rules (chronological; only thresholds matter) */
let FORK_CANCUN : int = 1 /* EIP-1153/4844; precompiles 0x01-0x0a */
let FORK_PRAGUE : int = 2 /* EIP-7623 calldata floor; +BLS 0x0b-0x11 */
let FORK_OSAKA : int = 3 /* EIP-7883 modexp gas, EIP-7825 cap; +0x100 */
let FORK_AMSTERDAM : int = 4 /* EIP-7954 code/initcode size bump (32768/65536) */

/* Is `n` (the low bytes of an address, as an int) a precompile at the active
 * fork? The precompile set GROWS by fork: Cancun has 0x01-0x0a (1-10);
 * Prague adds the EIP-2537 BLS12-381 set 0x0b-0x11 (11-17); Osaka adds
 * 0x100 P256VERIFY (EIP-7951). A CALL to an address OUTSIDE the active set is
 * an ordinary call to a (typically empty) account, not a precompile. */
function is_precompile(n : int) -> bool = {
    let bls = (n >= 11) & (n <= 17) & (k_fork >= FORK_PRAGUE);
    let p256 = (n == 256) & (k_fork >= FORK_OSAKA);
    ((n >= 1) & (n <= 10)) | bls | p256
}

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
        be_slot = ZERO_WORD,
    }
register k_tx : TxEnv = struct { txe_origin = ZERO_ADDR, txe_gas_price = ZERO_WORD, txe_blob_hashes = [||] }

/* ----------------------------- assoc helpers ---------------------------- */

/* select the acctmap key for address `a` (caches it for the accessors below) */
function acctmap_select(a : address) -> unit = {
    let aw : word = sail_zero_extend(a, 256);
    acctmap_key(aw[191 .. 128], aw[127 .. 64], aw[63 .. 0])
}

/* the three 256-bit account fields at the currently selected acctmap key
 * (word 3 most significant), reassembled from the per-word C accessors. */
function acctmap_read_bal() -> word =
    append(
        acctmap_bal(get_slice_int(64, 3, 0)),
        append(
            acctmap_bal(get_slice_int(64, 2, 0)),
            append(acctmap_bal(get_slice_int(64, 1, 0)), acctmap_bal(get_slice_int(64, 0, 0))),
        ),
    )
function acctmap_read_sroot() -> word =
    append(
        acctmap_sroot(get_slice_int(64, 3, 0)),
        append(
            acctmap_sroot(get_slice_int(64, 2, 0)),
            append(acctmap_sroot(get_slice_int(64, 1, 0)), acctmap_sroot(get_slice_int(64, 0, 0))),
        ),
    )
function acctmap_read_chash() -> word =
    append(
        acctmap_chash(get_slice_int(64, 3, 0)),
        append(
            acctmap_chash(get_slice_int(64, 2, 0)),
            append(acctmap_chash(get_slice_int(64, 1, 0)), acctmap_chash(get_slice_int(64, 0, 0))),
        ),
    )

/* the account bound to `a` (EMPTY_ACCOUNT if absent), read from the acctmap */
function acc_lookup(a : address) -> Account = {
    acctmap_select(a);
    if unsigned(acctmap_present()) == 1 then {
        struct {
            acc_nonce = unsigned(acctmap_nonce()),
            acc_balance = acctmap_read_bal(),
            acc_codehash = acctmap_read_chash(),
            acc_storage_root = acctmap_read_sroot(),
        }
    } else {
        EMPTY_ACCOUNT
    }
}

/* bind `a` -> v in the acctmap (overwrites any prior value) */
function acc_update(a : address, v : Account) -> unit = {
    acctmap_select(a);
    acctmap_store(
        get_slice_int(64, v.acc_nonce, 0),
        v.acc_balance[255 .. 192],
        v.acc_balance[191 .. 128],
        v.acc_balance[127 .. 64],
        v.acc_balance[63 .. 0],
        v.acc_storage_root[255 .. 192],
        v.acc_storage_root[191 .. 128],
        v.acc_storage_root[127 .. 64],
        v.acc_storage_root[63 .. 0],
        v.acc_codehash[255 .. 192],
        v.acc_codehash[191 .. 128],
        v.acc_codehash[127 .. 64],
        v.acc_codehash[63 .. 0],
    )
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

/* whether `a` has been materialized into the acctmap (distinct from acc_lookup,
 * which returns EMPTY_ACCOUNT for both absent and not-yet-loaded). */
function acc_has(a : address) -> bool = {
    acctmap_select(a);
    unsigned(acctmap_present()) == 1
}

/* Stateless account materialization (defined in lib/mpt.sail, the witness reader,
 * included later): resolve `a` from the authenticated witness leaf store and bind
 * it (nonce/balance/storageRoot/codeHash); absent in the witness -> empty. The
 * witness IS the base, so kget loads on first touch -- no eager seeding. */
val stateless_account_load : address -> unit

/* internal (un-journaled) account accessors used inside the kernel functions.
 * In stateless mode the witness backs the base layer: a not-yet-loaded account
 * is materialized from the witness on first access (un-journaled, so a frame
 * revert keeps the revealed pre-state and only undoes recorded mutations). */
function kget(a : address) -> Account = {
    if k_stateless & not_bool(acc_has(a)) then {
        stateless_account_load(a)
    };
    acc_lookup(a)
}
function kset(a : address, v : Account) -> unit = acc_update(a, v)

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

/* Apply one undo entry (k_revert replays these back to the checkpoint). */
function apply_undo(e : JEntry) -> unit = match e {
    JCheck()       => (),
    JAcct(a, old)  => kset(a, old),
    JStor(_, _, _) => (), /* persistent-storage undo is the overlay layer's pop_discard */
    JTran(a, s, v) => map_store(MAP_TRANSIENT, a, s, v),
    JWarmA(a)      => k_warm_addr = jremove_addr(k_warm_addr, a),
    JWarmS(a, s)   => map_store(MAP_WARM_SLOT, a, s, ZERO_WORD),
    JLog()         => k_logs = jdrop_last_log(k_logs),
    JRefund(old)   => k_refund = old,
    JSelfD()       => k_selfdestr = jdrop_last_addr(k_selfdestr),
}
