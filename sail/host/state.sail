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
/* host/kernel.sail. Accounts, persistent storage, and transient storage    */
/* are held in C-backed structures; warm access sets, logs, and the journal */
/* remain Sail values.                                                      */
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

/* The account table is the C-backed acctmap (keccak(address) -> Account; see
 * the ffi/state_db.c declarations below). Persistent storage is keyed by
 * (keccak(address), keccak(slot)); transient storage remains C-backed because
 * it is an execution-local word table. Warm address/slot sets are Sail values.
 * No Sail association lists for sigma. */

/* SlotKey (the BAL access-set key) is defined in host/io.sail (its bal_load
 * builder is the producer), included before this file. */

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
    F_BlobBaseFee,
    F_Origin,
    F_GasPrice,
    F_SlotNumber, /* EIP-7843 SLOTNUM */
}

/* The block environment the EVM reads (BLOCKHASH..BLOBBASEFEE opcodes).
 * base_fee: EIP-1559; prev_randao: EIP-4399 (PREVRANDAO replaced DIFFICULTY);
 * blob_base_fee/excess_blob_gas: EIP-4844. */
struct BlockEnv = {
    number : word, timestamp : word, gas_limit : word, coinbase : address,
    base_fee : word, prev_randao : word, chain_id : word,
    blob_base_fee : word, excess_blob_gas : word, slot : word,
}

/* Per-transaction environment: ORIGIN/GASPRICE (YP I_o, I_p) + the EIP-4844
 * blob versioned hashes the BLOBHASH opcode reads. */
struct TxEnv = { origin : address, gas_price : word, blob_hashes : list(word) }

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
        nonce = 0,
        balance = ZERO_WORD,
        code_hash = KECCAK_EMPTY,
        storage_root = EMPTY_TRIE_ROOT,
    }

/* the account working set is the C-backed acctmap (ffi/state_db.c), keyed by
 * keccak(address) -- no Sail assoc list. */
register k_warm_addr : list(address) = [||]
register k_warm_slots : list(SlotKey) = [||]
register k_logs : list(LogEntry) = [||]

/* Store code `c` in the content-addressed code store under its keccak hash `h`.
 * If `h` is already present (identical bytes) the byte stream is skipped
 * (code_db_stream_code_begin returns 0) -- the store is append-only and de-duplicated. */
function code_db_store_code(h : hash, c : list(byte)) -> unit = {
    if code_db_stream_code_begin(h[255 .. 192], h[191 .. 128], h[127 .. 64], h[63 .. 0]) == 0x0000000000000000 then {
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

function storage_select(a : address, s : word) -> unit = {
    let ah : hash = keccak256_address(a);
    let sh : hash = keccak256_word(s);
    storage_map_key(
        s[255 .. 192],
        s[191 .. 128],
        s[127 .. 64],
        s[63 .. 0],
        ah[255 .. 192],
        ah[191 .. 128],
        ah[127 .. 64],
        ah[63 .. 0],
        sh[255 .. 192],
        sh[191 .. 128],
        sh[127 .. 64],
        sh[63 .. 0],
    )
}

function storage_load(a : address, s : word) -> word = {
    storage_select(a, s);
    append(
        storage_map_word(get_slice_int(64, 3, 0)),
        append(
            storage_map_word(get_slice_int(64, 2, 0)),
            append(storage_map_word(get_slice_int(64, 1, 0)), storage_map_word(get_slice_int(64, 0, 0))),
        ),
    )
}

function storage_store(a : address, s : word, v : word) -> unit = {
    storage_select(a, s);
    storage_map_store(v[255 .. 192], v[191 .. 128], v[127 .. 64], v[63 .. 0])
}

function storage_seed(a : address, s : word, v : word) -> unit = {
    storage_select(a, s);
    storage_map_seed(v[255 .. 192], v[191 .. 128], v[127 .. 64], v[63 .. 0])
}

/* value in the BASE layer only (the state at tx start -- EIP-2200 original) */
function storage_base_load(a : address, s : word) -> word = {
    storage_select(a, s);
    append(
        storage_map_base_word(get_slice_int(64, 3, 0)),
        append(
            storage_map_base_word(get_slice_int(64, 2, 0)),
            append(storage_map_base_word(get_slice_int(64, 1, 0)), storage_map_base_word(get_slice_int(64, 0, 0))),
        ),
    )
}

/* present in ANY layer (an explicit 0 is present; fail-closed stateless test) */
function storage_present(a : address, s : word) -> bool = {
    storage_select(a, s);
    storage_map_present()
}

/* present in the BASE layer only (EIP-2200 "original" presence; a committed base
 * 0 is present, a never-committed slot is absent and resolved from the witness) */
function storage_base_present(a : address, s : word) -> bool = {
    storage_select(a, s);
    storage_map_base_present()
}

function storage_wipe_addr(a : address) -> unit = {
    let ah : hash = keccak256_address(a);
    storage_map_wipe_acct_hash(ah[255 .. 192], ah[191 .. 128], ah[127 .. 64], ah[63 .. 0])
}

function transient_reset() -> unit = transient_storage_reset(get_slice_int(64, 0, 0))

function transient_select(a : address, s : word) -> unit = {
    let aw : word = sail_zero_extend(a, 256);
    transient_storage_key(
        get_slice_int(64, 0, 0),
        aw[191 .. 128],
        aw[127 .. 64],
        aw[63 .. 0],
        s[255 .. 192],
        s[191 .. 128],
        s[127 .. 64],
        s[63 .. 0],
    )
}
function transient_load(a : address, s : word) -> word = {
    transient_select(a, s);
    append(
        transient_storage_read_word(get_slice_int(64, 3, 0)),
        append(
            transient_storage_read_word(get_slice_int(64, 2, 0)),
            append(
                transient_storage_read_word(get_slice_int(64, 1, 0)),
                transient_storage_read_word(get_slice_int(64, 0, 0)),
            ),
        ),
    )
}
function transient_store(a : address, s : word, v : word) -> unit = {
    transient_select(a, s);
    transient_storage_write(v[255 .. 192], v[191 .. 128], v[127 .. 64], v[63 .. 0])
}

/* Stateless storage resolver: the value of (a, s) in the witness pre-state,
 * walked from the account's authenticated storage root. Defined in lib/mpt.sail
 * (the witness reader, included later); declared here so the kernel's storage
 * reads can fall through to it on a working-set miss. */
val stateless_storage : (address, word) -> word

/* flattened storage enumeration (dump_state / state root; cold path) */
function storage_count() -> int = unsigned(storage_map_count())
function storage_update_count() -> int = unsigned(storage_map_update_count())
function storage_selected_slot() -> word =
    append(
        storage_map_it_slot(get_slice_int(64, 3, 0)),
        append(
            storage_map_it_slot(get_slice_int(64, 2, 0)),
            append(storage_map_it_slot(get_slice_int(64, 1, 0)), storage_map_it_slot(get_slice_int(64, 0, 0))),
        ),
    )
function storage_selected_val() -> word =
    append(
        storage_map_it_val(get_slice_int(64, 3, 0)),
        append(
            storage_map_it_val(get_slice_int(64, 2, 0)),
            append(storage_map_it_val(get_slice_int(64, 1, 0)), storage_map_it_val(get_slice_int(64, 0, 0))),
        ),
    )
function storage_selected_hashed_entry() -> (bits(256), word, word) = {
    let ak : bits(256) = append(
        storage_map_it_acct_hash(get_slice_int(64, 3, 0)),
        append(
            storage_map_it_acct_hash(get_slice_int(64, 2, 0)),
            append(storage_map_it_acct_hash(get_slice_int(64, 1, 0)), storage_map_it_acct_hash(get_slice_int(64, 0, 0))),
        ),
    );
    (ak, storage_selected_slot(), storage_selected_val())
}
function storage_hashed_entry(j : int) -> (bits(256), word, word) = {
    storage_map_at(get_slice_int(64, j, 0));
    storage_selected_hashed_entry()
}
function storage_hashed_update_entry(j : int) -> (bits(256), word, word) = {
    storage_map_update_at(get_slice_int(64, j, 0));
    storage_selected_hashed_entry()
}

function storage_has_live_acct_hash(ak : bits(256)) -> bool = {
    let n = storage_count();
    var j : int = 0;
    var found : bool = false;
    while (j < n) & not_bool(found) do {
        let (eak, _, ev) = storage_hashed_entry(j);
        if (eak == ak) & word_nonzero(ev) then {
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

register k_block : BlockEnv =
    struct {
        number = ZERO_WORD,
        timestamp = ZERO_WORD,
        gas_limit = ZERO_WORD,
        coinbase = ZERO_ADDR,
        base_fee = ZERO_WORD,
        prev_randao = ZERO_WORD,
        chain_id = ZERO_WORD,
        blob_base_fee = ZERO_WORD,
        excess_blob_gas = ZERO_WORD,
        slot = ZERO_WORD,
    }
register k_tx : TxEnv = struct { origin = ZERO_ADDR, gas_price = ZERO_WORD, blob_hashes = [||] }

/* ----------------------------- assoc helpers ---------------------------- */

/* select the acctmap key for address `a` (caches it for the accessors below) */
function acctmap_select_hashed(h : bits(256)) -> unit =
    acctmap_key(h[255 .. 192], h[191 .. 128], h[127 .. 64], h[63 .. 0])
function acctmap_select(a : address) -> unit = acctmap_select_hashed(keccak256_address(a))

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
function account_lookup(a : address) -> Account = {
    acctmap_select(a);
    if acctmap_present() then {
        struct {
            nonce = unsigned(acctmap_nonce()),
            balance = acctmap_read_bal(),
            code_hash = acctmap_read_chash(),
            storage_root = acctmap_read_sroot(),
        }
    } else {
        EMPTY_ACCOUNT
    }
}

function acctmap_write_account(a : address, v : Account, update : bool) -> unit = {
    let h = keccak256_address(a);
    acctmap_select_hashed(h);
    if update then {
        acctmap_store(
            h[255 .. 192],
            h[191 .. 128],
            h[127 .. 64],
            h[63 .. 0],
            get_slice_int(64, v.nonce, 0),
            v.balance[255 .. 192],
            v.balance[191 .. 128],
            v.balance[127 .. 64],
            v.balance[63 .. 0],
            v.storage_root[255 .. 192],
            v.storage_root[191 .. 128],
            v.storage_root[127 .. 64],
            v.storage_root[63 .. 0],
            v.code_hash[255 .. 192],
            v.code_hash[191 .. 128],
            v.code_hash[127 .. 64],
            v.code_hash[63 .. 0],
        )
    } else {
        acctmap_seed(
            h[255 .. 192],
            h[191 .. 128],
            h[127 .. 64],
            h[63 .. 0],
            get_slice_int(64, v.nonce, 0),
            v.balance[255 .. 192],
            v.balance[191 .. 128],
            v.balance[127 .. 64],
            v.balance[63 .. 0],
            v.storage_root[255 .. 192],
            v.storage_root[191 .. 128],
            v.storage_root[127 .. 64],
            v.storage_root[63 .. 0],
            v.code_hash[255 .. 192],
            v.code_hash[191 .. 128],
            v.code_hash[127 .. 64],
            v.code_hash[63 .. 0],
        )
    }
}

/* bind `a` -> v in the acctmap as authenticated pre-state/cache material. */
function account_seed(a : address, v : Account) -> unit = acctmap_write_account(a, v, false)

/* bind `a` -> v in the acctmap as an execution mutation. */
function account_update(a : address, v : Account) -> unit = acctmap_write_account(a, v, true)
function address_list_contains(xs : list(address), a : address) -> bool =
    match xs {
        [||] => false,
        x :: r => if x == a then {
            true
        } else {
            address_list_contains(r, a)
        },
    }
function slot_list_contains(xs : list(SlotKey), a : address, s : word) -> bool =
    match xs {
        [||] => false,
        x :: r => if x.address == a & x.slot == s then {
            true
        } else {
            slot_list_contains(r, a, s)
        },
    }

/* whether `a` has been materialized into the acctmap (distinct from account_lookup,
 * which returns EMPTY_ACCOUNT for both absent and not-yet-loaded). */
function account_loaded(a : address) -> bool = {
    acctmap_select(a);
    acctmap_present()
}

/* Stateless account materialization (defined in lib/mpt.sail, the witness reader,
 * included later): resolve `a` from the authenticated witness leaf store and bind
 * it (nonce/balance/storageRoot/codeHash); absent in the witness -> empty. The
 * witness IS the base, so load_account loads on first touch -- no eager seeding. */
val stateless_account_load : address -> unit

/* internal (un-journaled) account accessors used inside the kernel functions.
 * In stateless mode the witness backs the base layer: a not-yet-loaded account
 * is materialized from the witness on first access (un-journaled, so a frame
 * revert keeps the revealed pre-state and only undoes recorded mutations). */
function load_account(a : address) -> Account = {
    if k_stateless & not_bool(account_loaded(a)) then {
        stateless_account_load(a)
    };
    account_lookup(a)
}
function store_account(a : address, v : Account) -> unit = account_update(a, v)

/* --- journal (undo log) helpers --- */
function journal_push(e : JEntry) -> unit = k_journal = e :: k_journal
function remove_address_once(xs : list(address), a : address) -> list(address) =
    match xs {
        [||] => [||],
        x :: r => if x == a then {
            r
        } else {
            x :: remove_address_once(r, a)
        },
    }
function remove_slot_once(xs : list(SlotKey), a : address, s : word) -> list(SlotKey) =
    match xs {
        [||] => [||],
        x :: r => if x.address == a & x.slot == s then {
            r
        } else {
            x :: remove_slot_once(r, a, s)
        },
    }
function drop_last_log(xs : list(LogEntry)) -> list(LogEntry) =
    match xs {
        [||] => [||],
        _ :: [||] => [||],
        x :: r => x :: drop_last_log(r),
    }
function drop_last_address(xs : list(address)) -> list(address) =
    match xs {
        [||] => [||],
        _ :: [||] => [||],
        x :: r => x :: drop_last_address(r),
    }

/* Apply one undo entry (k_revert replays these back to the checkpoint). */
function apply_undo(e : JEntry) -> unit = match e {
    JCheck()       => (),
    JAcct(a, old)  => store_account(a, old),
    JStor(_, _, _) => (), /* persistent-storage undo is the overlay layer's pop_discard */
    JTran(a, s, v) => transient_store(a, s, v),
    JWarmA(a)      => k_warm_addr = remove_address_once(k_warm_addr, a),
    JWarmS(a, s)   => k_warm_slots = remove_slot_once(k_warm_slots, a, s),
    JLog()         => k_logs = drop_last_log(k_logs),
    JRefund(old)   => k_refund = old,
    JSelfD()       => k_selfdestr = drop_last_address(k_selfdestr),
}
