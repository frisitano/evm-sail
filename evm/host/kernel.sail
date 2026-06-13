/* ======================================================================== */
/* HOST KERNEL FUNCTIONS                                                     */
/*                                                                           */
/* The host's kernel-function surface (`k_*`): the ONLY way the transaction  */
/* kernel (the EVM) touches world state. Each function performs one effect   */
/* or answers one query against host/state.sail. The host is pure mechanism: */
/* it holds no gas counter and makes no policy decisions -- the EVM decides  */
/* whether an effect happens (e.g. a no-op SSTORE charges gas in the         */
/* transaction kernel but performs no host write).                           */
/* ======================================================================== */

/* --- EIP-2929 access (mark warm, return PRIOR warm bit) --- */
function k_access_account(a : address) -> bool = {
    let prior = addr_in(k_warm_addr, a);
    if not_bool(prior) then {
        k_warm_addr = a :: k_warm_addr;
        jpush(JWarmA(a))
    };
    prior
}
function k_access_slot(a : address, s : word) -> bool = {
    let prior = word_nonzero(map_load(MAP_WARM_SLOT, a, s));
    if not_bool(prior) then {
        map_store(MAP_WARM_SLOT, a, s, word_of_int(1));
        jpush(JWarmS(a, s))
    };
    prior
}

/* --- storage (C overlay map; layers paired with snapshot/commit/revert) --- */

/* stateless guard: a slot the EVM touches must be in the witness working set
 * (seeded from the BAL slice + witness pre-values). Absent => the prover did not
 * provision it => fail closed -- host_map_present distinguishes a witness-proven 0
 * from never-provisioned. Off by default, so the native path is unchanged. */
function k_stateless_on() -> unit = k_stateless = true
function storage_present(a : address, s : word) -> unit =
    if k_stateless then {
        assert(map_present(MAP_STORAGE, a, s), "storage slot not in witness working set")
    }

function k_sload(a : address, s : word) -> word = {
    storage_present(a, s);
    map_load(MAP_STORAGE, a, s)
}
function k_sstore(a : address, s : word, v : word) -> unit = {
    storage_present(a, s);
    map_store(MAP_STORAGE, a, s, v) /* overlay write; revert = layer discard */
}

/* EIP-2200 original value = the BASE layer (the state at tx start; every prior
 * tx was pop_commit-merged into it). No per-tx snapshot copy needed. */
function k_orig(a : address, s : word) -> word = map_base_load(MAP_STORAGE, a, s)

/* unrecorded storage read (drivers/examples; same layered view as SLOAD) */
function k_storage_get(a : address, s : word) -> word = map_load(MAP_STORAGE, a, s)
function k_tload(a : address, s : word) -> word = {
    map_load(MAP_TRANSIENT, a, s)
}
function k_tstore(a : address, s : word, v : word) -> unit = {
    jpush(JTran(a, s, map_load(MAP_TRANSIENT, a, s)));
    map_store(MAP_TRANSIENT, a, s, v)
}

/* --- accounts --- */
function k_get_balance(a : address) -> word = {
    kget(a).acc_balance
}
function k_get_nonce(a : address) -> int = {
    kget(a).acc_nonce
}

/* the raw code key for store lookups (KECCAK_EMPTY for codeless accounts) */
function k_code_key(a : address) -> hash = kget(a).acc_codehash

/* EXTCODEHASH: a field read -- the hash IS the account's code binding.
 * A truly empty (nonexistent) account reads as 0 (EIP-1052). */
function k_get_codehash(a : address) -> hash = {
    let acc = kget(a);
    if (acc.acc_codehash == KECCAK_EMPTY) & acc.acc_nonce == 0 & word_is_zero(acc.acc_balance) then {
        word_of_int(0)
    } else {
        acc.acc_codehash
    }
}
function k_account_is_empty(a : address) -> bool = {
    /* EIP-161 "empty" account: nonce 0, balance 0, no code (unrecorded query) */
    let acc = kget(a);
    (acc.acc_codehash == KECCAK_EMPTY) & acc.acc_nonce == 0 & word_is_zero(acc.acc_balance)
}

/* CREATE/CREATE2/create-tx address-collision test (unrecorded query): the
 * target is occupied if it has code OR a nonzero nonce OR any storage
 * (EELS account_has_code_or_nonce || account_has_storage). */
function k_account_occupied(a : address) -> bool = {
    let acc = kget(a);
    let has_code : bool = acc.acc_codehash != KECCAK_EMPTY;
    has_code | (acc.acc_nonce != 0) | map_has_addr(MAP_STORAGE, a)
}
function k_transfer(src : address, dst : address, v : word) -> unit = {
    jpush(JAcct(src, kget(src)));
    jpush(JAcct(dst, kget(dst)));
    kset(src, { kget(src) with acc_balance = alu_sub(kget(src).acc_balance, v) });
    kset(dst, { kget(dst) with acc_balance = alu_add(kget(dst).acc_balance, v) })
}
function k_bump_nonce(a : address) -> unit = {
    jpush(JAcct(a, kget(a)));
    kset(a, { kget(a) with acc_nonce = kget(a).acc_nonce + 1 })
}
function k_set_code(a : address, c : list(byte)) -> unit = {
    jpush(JAcct(a, kget(a)));
    let h : hash = keccak256(c);
    cs_store_code(h, c);
    kset(a, { kget(a) with acc_codehash = h })
}

/* ---- EIP-7702 delegation designation (0xef0100 || address) ---- */
function addr_bytes(a : address) -> list(byte) = {
    let w = address_to_word(a);
    var out : list(byte) = [||];
    foreach (k from 0 to 19) {
        out = list_append(out, [|sail_shiftright(w, 8 * (19 - k))[7 .. 0]|])
    };
    out
}
function delegation_code(a : address) -> list(byte) = 0xef :: 0x01 :: 0x00 :: addr_bytes(a)

val cs_deleg = impure { _: "cs_deleg" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(168)

/* EIP-7702 delegation of addr's stored code, probed by codeHash in ONE host
 * call (runs on every CALL-family target): bit 160 = is a delegation
 * designation, low 160 bits = the designated target. */
function k_deleg_target(a : address) -> (bool, address) = {
    let h : hash = k_code_key(a);
    let r : bits(168) = cs_deleg(h[255 .. 192], h[191 .. 128], h[127 .. 64], h[63 .. 0]);
    (r[160] == bitone, r[159 .. 0])
}

/* EXTCODESIZE / EXTCODECOPY via the execution mirror (the semantic code db
 * defines the value; walking a code list is O(|code|) per opcode) */
function k_get_code_size(a : address) -> int = {
    let h : hash = k_code_key(a);
    unsigned(cs_len(h[255 .. 192], h[191 .. 128], h[127 .. 64], h[63 .. 0]))
}
function k_code_copy(a : address, dst : int, off : int, len : int) -> unit = {
    let h : hash = k_code_key(a);

    /* clamp a beyond-2^64 offset to a beyond-any-code sentinel (reads zeros) */
    let o = if off > 16777216 then { 16777216 } else { off };
    cs_to_mem(
        h[255 .. 192],
        h[191 .. 128],
        h[127 .. 64],
        h[63 .. 0],
        get_slice_int(64, dst, 0),
        get_slice_int(64, o, 0),
        get_slice_int(64, len, 0),
    )
}

/* used only by the tx driver to seed pre-state (no trace entry) */
function k_seed_account(a : address, nonce : int, bal : word, c : list(byte)) -> unit = {
    let h : hash = keccak256(c);
    cs_store_code(h, c);
    kset(a, struct { acc_nonce = nonce, acc_balance = bal, acc_codehash = h })
}

/* seed an account whose code is already in the store (a stateless witness view):
 * the codeHash is given, no bytes are streamed. */
function k_seed_account_hashed(a : address, nonce : int, bal : word, codehash : hash) -> unit =
    kset(a, struct { acc_nonce = nonce, acc_balance = bal, acc_codehash = codehash })
function k_seed_storage(a : address, s : word, v : word) -> unit =
    map_store(MAP_STORAGE, a, s, v) /* seeds the base layer (only layer live here) */

/* single-account balance deltas (gas fees / withdrawals); recorded as a
 * transfer to/from the zero-address fee sink so the flow is in the trace. */
function k_add_balance(a : address, v : word) -> unit = {
    jpush(JAcct(a, kget(a)));
    kset(a, { kget(a) with acc_balance = alu_add(kget(a).acc_balance, v) })
}
function k_sub_balance(a : address, v : word) -> unit = {
    jpush(JAcct(a, kget(a)));
    kset(a, { kget(a) with acc_balance = alu_sub(kget(a).acc_balance, v) })
}

/* --- environment --- */
function k_env(f : EnvField) -> word = {
    match f {
        F_Number => k_block.be_number,
        F_Timestamp => k_block.be_timestamp,
        F_Coinbase => address_to_word(k_block.be_coinbase),
        F_BaseFee => k_block.be_base_fee,
        F_ChainId => k_block.be_chain_id,
        F_GasLimit => k_block.be_gas_limit,
        F_PrevRandao => k_block.be_prev_randao,
        F_BlobBaseFee => k_block.be_blob_base_fee,
        F_Origin => address_to_word(k_tx.txe_origin),
        F_GasPrice => k_tx.txe_gas_price,
    }
}
function k_coinbase() -> address = k_block.be_coinbase
function k_blockhash(n : word) -> hash = {
    let _ = n;
    word_of_int(0)
}
function k_blobhash(i : int) -> word = {
    list_nth(k_tx.txe_blob_hashes, i, word_of_int(0))
}

/* --- primitives --- */
function k_create_addr(a : address, nonce : int) -> address = create_address(a, nonce)
function k_create2_addr(a : address, salt : word, inithash : hash) -> address =
    create2_address(a, salt, inithash)

/* precompile syscall: the input was STAGED by the EVM (a snapshot of the
 * caller's mem[args .. args+len) -- the child message's calldata -- or the tx
 * input). Output stays in the accelerator buffer; the EVM adopts it as
 * returndata. Gas is computed by the EVM (policy, evm/gas.sail). */
function k_precompile(num : int) -> bool =
    if is_precompile(num) then {
        run_precompile_staged()
    } else {
        false
    }

/* --- logs + refund --- */
function k_log(a : address, topics : list(word), data : list(byte)) -> unit = {
    jpush(JLog());
    k_logs = list_append(k_logs, [|struct { le_addr = a, le_topics = topics, le_data = data }|])
}
function k_refund_add(delta : int) -> unit = {
    jpush(JRefund(k_refund));
    k_refund = k_refund + delta
}
function k_selfdestruct(a : address) -> unit = {
    jpush(JSelfD());
    k_selfdestr = list_append(k_selfdestr, [|a|])
}

/* --- EIP-6780 created-this-tx tracking --- */
/* Mark an address as CREATEd in the current tx. Like EELS created_accounts the
 * marker is NOT removed when the creating frame reverts (harmless: the account
 * cannot selfdestruct later without being re-created, which re-marks it). */
function k_mark_created(a : address) -> unit =
    if addr_in(k_created, a) then {
        ()
    } else {
        k_created = a :: k_created
    }
function k_was_created(a : address) -> bool = addr_in(k_created, a)

/* EIP-6780 (selfdestruct of a created-this-tx account): the originator's
 * balance is forced to 0 after the transfer, so a self-beneficiary transfer
 * burns the ether (journaled; a frame revert restores it). */
function k_zero_balance(a : address) -> unit = {
    jpush(JAcct(a, kget(a)));
    kset(a, { kget(a) with acc_balance = ZERO_WORD })
}

/* delete an account outright: drop it from the account table and wipe its
 * storage. Used by the EIP-6780 tx-end finalization (created AND selfdestructed
 * in the same tx). Unrecorded: the tx-end reduction is host bookkeeping. */
function acc_remove(es : list(AccountEntry), a : address) -> list(AccountEntry) =
    match es {
        [||] => [||],
        e :: r => if e.ae_addr == a then {
            acc_remove(r, a)
        } else {
            e :: acc_remove(r, a)
        },
    }
function k_account_delete(a : address) -> unit = {
    k_accounts = acc_remove(k_accounts, a);
    map_wipe_addr(MAP_STORAGE, a)
}

/* EIP-6780 tx-end finalization: an account is actually DELETED only if it was
 * both CREATEd and SELFDESTRUCTed in this same tx (EELS: destroy_account per
 * tx_output.accounts_to_delete). The balance already moved at SELFDESTRUCT
 * time. Call AFTER the tx-level commit, so only surviving (committed)
 * selfdestructs are in k_selfdestr; a pre-existing selfdestructed account is
 * left in place (code/nonce/storage kept). */
function k_finalize_selfdestructs() -> unit = {
    var xs : list(address) = k_selfdestr;
    var more : bool = true;
    while more do {
        match xs {
            a :: r => {
                if k_was_created(a) then {
                    k_account_delete(a)
                };
                xs = r
            },
            [||]   => more = false,
        }
    }
}

/* --- world snapshots (atomic revert across a frame) --- */
function k_snapshot() -> unit = {
    jpush(JCheck());
    host_map_push(get_slice_int(64, MAP_STORAGE, 0)) /* open the frame's storage overlay */
}

/* full world wipe between independent blocks/fixtures (driver/test only) */
function k_world_reset() -> unit = {
    k_accounts = [||];
    host_map_reset(get_slice_int(64, MAP_STORAGE, 0));
    host_map_reset(get_slice_int(64, MAP_TRANSIENT, 0));
    k_warm_addr = [||];
    host_map_reset(get_slice_int(64, MAP_WARM_SLOT, 0));
    k_logs = [||];
    k_refund = 0;
    k_selfdestr = [||];
    k_created = [||];
    k_journal = [||];
    k_stateless = false
}

/* --- tx-lifecycle / driver helpers (system layer, not EVM user space) --- */
function k_set_block(env : BlockEnv) -> unit = k_block = env
function k_set_tx(env : TxEnv) -> unit = k_tx = env
function k_tx_reset() -> unit = {
    /* per-tx: clear ephemeral state, snapshot tx-original storage */
    k_warm_addr = [||];
    host_map_reset(get_slice_int(64, MAP_WARM_SLOT, 0));
    host_map_reset(get_slice_int(64, MAP_TRANSIENT, 0));
    k_logs = [||];
    k_refund = 0;
    k_selfdestr = [||];
    k_created = [||];
    k_journal = [||]
    /* no per-tx original-storage snapshot: the overlay's base layer IS tx-start state */
}

/* commit: drop the most recent checkpoint; the frame's undo entries fold into
 * the parent frame (so a later parent revert also undoes them). */
function k_commit() -> unit = {
    host_map_pop_commit(get_slice_int(64, MAP_STORAGE, 0)); /* merge the frame's storage overlay */
    var pre : list(JEntry) = [||]; /* frame's entries before the checkpoint (reversed) */
    var j : list(JEntry) = k_journal;
    var done : bool = false;
    while not_bool(done) do {
        match j {
            [||]          => done = true,
            JCheck() :: r => {
                j = r;
                done = true
            },
            e :: r        => {
                pre = e :: pre;
                j = r
            },
        }
    };
    k_journal = rev_onto(pre, j)
}

/* revert: replay undo entries back to (and dropping) the most recent checkpoint. */
function k_revert() -> unit = {
    host_map_pop_discard(get_slice_int(64, MAP_STORAGE, 0)); /* drop the frame's storage overlay */
    var j : list(JEntry) = k_journal;
    var done : bool = false;
    while not_bool(done) do {
        match j {
            [||]          => done = true,
            JCheck() :: r => {
                j = r;
                done = true
            },
            e :: r        => {
                apply_undo(e);
                j = r
            },
        }
    };
    k_journal = j
}
