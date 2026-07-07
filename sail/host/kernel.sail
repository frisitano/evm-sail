/* ======================================================================== */
/* HOST KERNEL FUNCTIONS                                                    */
/*                                                                          */
/* The host's syscall surface (k_*): the ONLY interface through which the   */
/* transaction kernel (the EVM, user space) reads or mutates world state    */
/* (host/state.sail). Each function performs one effect (a balance          */
/* transfer, an SSTORE, a log emission, a snapshot/commit/revert) or        */
/* answers one query (a balance, a code hash, an EIP-2929 warm bit).        */
/*                                                                          */
/* The host is pure mechanism: it holds no gas counter and makes no policy  */
/* decision. The EVM decides WHETHER an effect happens and charges all gas  */
/* (evm/gas.sail); for example a no-op SSTORE is still gas-charged in the   */
/* kernel but performs no host write. State mutations are made revertible   */
/* at their enclosing call-frame boundary either by a journal undo entry    */
/* or, for persistent storage, by the overlay-layer stack (k_snapshot /     */
/* k_commit / k_revert).                                                    */
/* ======================================================================== */

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
function k_access_slot(a : address, s : word) -> bool = {
    let prior = warm_slot_contains(a, s);
    if not_bool(prior) then {
        warm_slot_insert(a, s);
        journal_push(JWarmS(a, s))
    };
    prior
}

/* --- persistent storage (SLOAD/SSTORE): the C cache/update store, whose update
 * layers are pushed/popped by k_snapshot / k_commit / k_revert --- */

/* k_sload: the slot's live value = the write-set overlay (this tx's write over
 * committed block writes) when present, else the pre-state resolver
 * stateless_storage. k_orig (EIP-2200 "original"): the slot at tx start = the
 * block base (committed prior txs) when present, else stateless_storage. The
 * overlay is writes-only, so a pure read never mutates it; storage reverts run
 * through the JStor journal.
 *
 * stateless_storage is the only backend-specific storage primitive (declared in
 * host/state.sail, defined per entry: host/base_native.sail = seeded cache k/v
 * read, host/base_witness.sail = authenticated MPT point-get). Everything above
 * it -- the overlay, the read-through, the journal -- is common. */
function k_sload(a : address, s : word) -> word =
    match storage_wset_get(a, s) {
        Some(v) => v,
        None()  => stateless_storage(a, s),
    }
function k_orig(a : address, s : word) -> word =
    match storage_wset_base_get(a, s) {
        Some(v) => v,
        None()  => stateless_storage(a, s),
    }

function k_sstore(a : address, s : word, v : word) -> unit = {
    let orig = k_orig(a, s);                                    /* EIP-2200 tx-start value */
    journal_push(JStor(a, s, storage_wset_prior(a, s, orig)));  /* undo -> value before this write */
    storage_wset_write(a, s, v, orig);                          /* orig adopted on first write this tx */
    store_account(a, load_account(a)) /* storage-root changes require a fresh account trie leaf */
}

/* un-journaled storage read (drivers/examples; same overlay+base view as SLOAD) */
function k_storage_get(a : address, s : word) -> word = k_sload(a, s)

/* EIP-1153 transient storage (TLOAD/TSTORE): per-tx scratch, discarded at tx end,
 * not part of the state trie. TSTORE journals the prior value for frame revert. */
function k_tload(a : address, s : word) -> word = {
    transient_load(a, s)
}
function k_tstore(a : address, s : word, v : word) -> unit = {
    journal_push(JTran(a, s, transient_load(a, s)));
    transient_store(a, s, v)
}

/* --- accounts (BALANCE / EXTCODEHASH / EXTCODESIZE / nonce, transfers) --- */
function k_get_balance(a : address) -> word = {
    load_account(a).balance
}
function k_get_nonce(a : address) -> int = {
    load_account(a).nonce
}

/* the raw codeHash used to key the code store (KECCAK_EMPTY for codeless accounts) */
function k_code_key(a : address) -> hash = load_account(a).code_hash

/* EXTCODEHASH (EIP-1052): the account's code binding. A truly empty
 * (nonexistent per EIP-161) account reads as 0, not KECCAK_EMPTY. */
function k_get_codehash(a : address) -> hash = {
    let acc = load_account(a);
    if (acc.code_hash == KECCAK_EMPTY) & acc.nonce == 0 & word_is_zero(acc.balance) then {
        word_of_int(0)
    } else {
        acc.code_hash
    }
}
function k_account_is_empty(a : address) -> bool = {
    /* EIP-161 "empty" account: nonce 0, balance 0, no code (unrecorded query) */
    let acc = load_account(a);
    (acc.code_hash == KECCAK_EMPTY) & acc.nonce == 0 & word_is_zero(acc.balance)
}

/* CREATE/CREATE2/create-tx address-collision test (unrecorded query): the
 * target is occupied if it has code OR a nonzero nonce OR any storage
 * (EELS account_has_code_or_nonce || account_has_storage). */
function k_account_occupied(a : address) -> bool = {
    let acc = load_account(a);
    let has_code : bool = acc.code_hash != KECCAK_EMPTY;
    let has_storage : bool = (acc.storage_root != EMPTY_TRIE_ROOT) | storage_has_live_addr(a);
    has_code | (acc.nonce != 0) | has_storage
}

function eip7708_system_address() -> address = 0xfffffffffffffffffffffffffffffffffffffffe
function eip7708_transfer_topic() -> word =
    0xddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef
function eip7708_burn_topic() -> word =
    0xcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5

function k_emit_transfer_log(src : address, dst : address, v : word) -> unit = {
    if (k_fork < FORK_AMSTERDAM) | word_is_zero(v) | (src == dst) then {
        ()
    } else {
        journal_push(JLog());
        emit_log(
            eip7708_system_address(),
            [|eip7708_transfer_topic(), address_to_word(src), address_to_word(dst)|],
            word_to_bytes32(v),
        )
    }
}

function k_emit_burn_log(a : address, v : word) -> unit = {
    if (k_fork < FORK_AMSTERDAM) | word_is_zero(v) then {
        ()
    } else {
        journal_push(JLog());
        emit_log(eip7708_system_address(), [|eip7708_burn_topic(), address_to_word(a)|], word_to_bytes32(v))
    }
}

/* Move `v` wei from src to dst (the value field of a CALL or a tx). Both
 * balances are journaled; the EVM checks sufficiency before calling. */
function k_transfer(src : address, dst : address, v : word) -> unit = {
    if word_is_zero(v) | (src == dst) then {
        ()
    } else {
        journal_push(JAcct(src, load_account(src)));
        journal_push(JAcct(dst, load_account(dst)));
        store_account(src, { load_account(src) with balance = alu_sub(load_account(src).balance, v) });
        store_account(dst, { load_account(dst) with balance = alu_add(load_account(dst).balance, v) });
        k_emit_transfer_log(src, dst, v)
    }
}

/* Increment an account's nonce (sender per tx, creator at CREATE/CREATE2). */
function k_bump_nonce(a : address) -> unit = {
    journal_push(JAcct(a, load_account(a)));
    store_account(a, { load_account(a) with nonce = load_account(a).nonce + 1 })
}

/* Set an account's code by binding its codeHash, journaling the prior account.
 * The code blob is interned straight from its host byte source (no Sail byte
 * loop on the build backend):
 *   k_deploy_returndata_code: a CREATE/CREATE2 deploy's returned code (the
 *     pending returndata buffer).
 *   k_set_delegation: the EIP-7702 designation 0xef0100 ++ target.
 *   k_clear_code: reset to codeless (KECCAK_EMPTY); no store entry is needed
 *     (an empty code blob is never interned; a missing key reads length 0). */
function k_deploy_returndata_code(a : address) -> unit = {
    journal_push(JAcct(a, load_account(a)));
    let h : hash = code_intern_returndata();
    store_account(a, { load_account(a) with code_hash = h })
}
function k_set_delegation(a : address, target : address) -> unit = {
    journal_push(JAcct(a, load_account(a)));
    let h : hash = code_intern_delegation(target);
    store_account(a, { load_account(a) with code_hash = h })
}
function k_clear_code(a : address) -> unit = {
    journal_push(JAcct(a, load_account(a)));
    store_account(a, { load_account(a) with code_hash = KECCAK_EMPTY })
}

/* ---- EIP-7702 delegation designation: the 23-byte code 0xef0100 ++ address
 * that an EOA's code is set to so its calls execute the designated contract ---- */
function addr_bytes(a : address) -> list(byte) = {
    let w = address_to_word(a);
    var out : list(byte) = [||];
    foreach (k from 0 to 19) {
        out = list_append(out, [|sail_shiftright(w, 8 * (19 - k))[7 .. 0]|])
    };
    out
}
function delegation_code(a : address) -> list(byte) = 0xef :: 0x01 :: 0x00 :: addr_bytes(a)

/* EIP-7702: resolve whether addr's stored code is a delegation designation,
 * probed by codeHash in ONE host call (run on every CALL-family target). Returns
 * (is_delegation, designated_target): bit 160 of the result is the flag, the low
 * 160 bits the target the call should execute against. */
/* A non-empty codeHash whose bytes are ABSENT from the witness code store is a
 * deficient proof (the prover omitted code execution needs): flag the block
 * invalid (EIP-8025 witness_validation_codes), rather than silently treating the
 * account as codeless. KECCAK_EMPTY accounts are legitimately codeless. */
function note_code_resolved(h : hash) -> unit =
    if (h != KECCAK_EMPTY) & (unsigned(code_db_stored_code_length(h)) == 0) then {
        k_witness_bad = true
    }

function k_deleg_target(a : address) -> (bool, address) = {
    let h : hash = k_code_key(a);

    /* NB: do NOT flag a missing code here -- delegation is probed on the warming
       path (e.g. EIP-7702 auth validation) for accounts EELS may legitimately not
       witness (an auth skipped for an invalid chain id), which must not invalidate
       the block. Missing EXECUTED/queried code is flagged in k_get_code_size /
       k_code_copy instead. */
    let r : bits(168) = code_db_read_delegation(h);
    (r[160] == bitone, r[159 .. 0])
}

/* EXTCODESIZE / EXTCODECOPY: the length of, and a copy from, the code of the
 * account at `a`, resolved by codeHash through the C code store. */
function k_get_code_size(a : address) -> int = {
    let h : hash = k_code_key(a);
    note_code_resolved(h);
    unsigned(code_db_stored_code_length(h))
}
function k_code_copy(a : address, dst : int, off : int, len : int) -> unit = {
    let h : hash = k_code_key(a);
    note_code_resolved(h);

    /* clamp a beyond-2^64 source offset to a beyond-any-code sentinel: a past-end
     * read zero-fills the destination (EELS buffer_read semantics) */
    let o = if off > 16777216 then { 16777216 } else { off };
    code_db_copy_stored_code_to_memory(h, get_slice_int(64, dst, 0), get_slice_int(64, o, 0), get_slice_int(64, len, 0))
}

/* --- pre-state seeding (driver / runner; not part of execution) --- */

/* Seed an account from full code bytes (intern them in the code store). */
function k_seed_account(a : address, nonce : int, bal : word, c : list(byte)) -> unit = {
    let h : hash = code_hash_and_store(c);
    account_seed(a, struct { nonce = nonce, balance = bal, code_hash = h, storage_root = EMPTY_TRIE_ROOT })
}

/* seed an account whose code is already in the store (a stateless witness input
 * span): the codeHash and pre-state storage root are given, no bytes are streamed. */
function k_seed_account_hashed(a : address, nonce : int, bal : word, sroot : hash, codehash : hash) -> unit =
    account_seed(a, struct { nonce = nonce, balance = bal, code_hash = codehash, storage_root = sroot })
function k_seed_storage(a : address, s : word, v : word) -> unit =
    storage_seed(a, s, v) /* authenticated pre-state/cache material */

/* single-account balance credit/debit (gas fees, the coinbase priority-fee
 * payment, withdrawals); both journal the prior account for frame revert. */
function k_add_balance(a : address, v : word) -> unit = {
    if word_is_zero(v) then {
        ()
    } else {
        journal_push(JAcct(a, load_account(a)));
        store_account(a, { load_account(a) with balance = alu_add(load_account(a).balance, v) })
    }
}
function k_sub_balance(a : address, v : word) -> unit = {
    if word_is_zero(v) then {
        ()
    } else {
        journal_push(JAcct(a, load_account(a)));
        store_account(a, { load_account(a) with balance = alu_sub(load_account(a).balance, v) })
    }
}

/* --- block / tx environment reads (NUMBER, TIMESTAMP, COINBASE, BASEFEE,
 * CHAINID, GASLIMIT, PREVRANDAO, BLOBBASEFEE, ORIGIN, GASPRICE) --- */
function k_env(f : EnvField) -> word = {
    match f {
        F_Number => k_block.number,
        F_Timestamp => k_block.timestamp,
        F_Coinbase => address_to_word(k_block.coinbase),
        F_BaseFee => k_block.base_fee,
        F_ChainId => k_block.chain_id,
        F_GasLimit => k_block.gas_limit,
        F_PrevRandao => k_block.prev_randao,
        F_BlobBaseFee => k_block.blob_base_fee,
        F_Origin => address_to_word(k_tx.origin),
        F_GasPrice => k_tx.gas_price,
        F_SlotNumber => k_block.slot,
    }
}
function k_coinbase() -> address = k_block.coinbase

/* BLOCKHASH: hash of one of the 256 most recent ancestor blocks. Base-layer
 * contract (per-entry): the witness backend hashes the authenticated witness
 * header at that depth (EIP-2935); the native backend has no ancestors in a
 * state-test fixture and models it as 0. */
val k_blockhash : word -> hash

/* BLOBHASH (EIP-4844): the i-th versioned hash of the current tx, 0 past the end. */
function k_blobhash(i : int) -> word = {
    list_nth(k_tx.blob_hashes, i, word_of_int(0))
}

/* --- address derivation primitives (CREATE / CREATE2) --- */
function k_create_addr(a : address, nonce : int) -> address = create_address(a, nonce)
function k_create2_addr(a : address, salt : word, inithash : hash) -> address =
    create2_address(a, salt, inithash)

/* --- logs (LOG0..LOG4) + the gas-refund counter --- */
function k_log(a : address, topics : list(word), data : list(byte)) -> unit = {
    journal_push(JLog());
    emit_log(a, topics, data)
}
function k_refund_add(delta : int) -> unit = {
    journal_push(JRefund(k_refund));
    k_refund = k_refund + delta
}

/* Record that `a` issued SELFDESTRUCT this tx (the balance transfer to the
 * beneficiary is a separate k_transfer). The account is only actually deleted
 * at tx end, and only under the EIP-6780 condition (k_finalize_selfdestructs). */
function k_selfdestruct(a : address) -> unit = {
    journal_push(JSelfD());
    selfdestr_push(a)
}

/* --- EIP-6780 created-this-tx tracking --- */
/* Mark an address as CREATEd in the current tx. Like EELS created_accounts the
 * marker is NOT removed when the creating frame reverts (harmless: the account
 * cannot selfdestruct later without being re-created, which re-marks it). */
function k_mark_created(a : address) -> unit = created_insert(a)
function k_was_created(a : address) -> bool = created_contains(a)

/* EIP-6780 (selfdestruct of a created-this-tx account): the originator's
 * balance is forced to 0 after the transfer, so a self-beneficiary transfer
 * burns the ether (journaled; a frame revert restores it). */
function k_zero_balance(a : address) -> unit = {
    if word_is_zero(load_account(a).balance) then {
        ()
    } else {
        journal_push(JAcct(a, load_account(a)));
        store_account(a, { load_account(a) with balance = ZERO_WORD })
    }
}

/* delete an account outright: drop it from the acctmap and wipe its storage.
 * Used by the EIP-6780 tx-end finalization (created AND selfdestructed in the
 * same tx). Unrecorded: the tx-end reduction is host bookkeeping. */
function k_account_delete(a : address) -> unit = {
    /* Set the account EMPTY in place (do NOT acctmap_remove it). Removing it would
       let a later load_account(a) RE-MATERIALIZE the account from the pre-state witness
       leaf -- wrong for an account CREATEd+SELFDESTRUCTed and then re-CREATEd later
       in the same block (the stale pre-state balance would resurface). Kept empty
       in the acctmap, a later read sees the deletion, the post-state walk drops it
       (account_empty) AND marks it touched (sb_touched) so its pre-state leaf is not
       reused -- subsuming the old k_deleted mechanism. */
    store_account(a, EMPTY_ACCOUNT);
    storage_wipe_addr(a)
}

function address_insert_sorted_unique(xs : list(address), a : address) -> list(address) =
    match xs {
        [||] => [|a|],
        x :: r => if x == a then {
            xs
        } else {
            if unsigned(address_to_word(a)) < unsigned(address_to_word(x)) then {
                a :: xs
            } else {
                x :: address_insert_sorted_unique(r, a)
            }
        },
    }

function address_sort_unique(xs : list(address)) -> list(address) =
    match xs {
        [||] => [||],
        x :: r => address_insert_sorted_unique(address_sort_unique(r), x),
    }

/* read the C-backed SELFDESTRUCT set out into a Sail list (cold path: tx-end
 * finalization sorts + dedups it). Order is irrelevant -- sort_unique reorders. */
function read_selfdestr_rev(i : int, n : int, acc : list(address)) -> list(address) =
    if i >= n then {
        acc
    } else {
        read_selfdestr_rev(i + 1, n, selfdestr_get(get_slice_int(64, i, 0))[159 .. 0] :: acc)
    }
function read_selfdestr() -> list(address) = read_selfdestr_rev(0, unsigned(selfdestr_count()), [||])

/* SELFDESTRUCT tx-end finalization. Before Cancun, every surviving
 * SELFDESTRUCT deletes the originator. Cancun+ applies EIP-6780: an account is
 * actually DELETED only if it was both CREATEd and SELFDESTRUCTed in this same
 * tx (EELS: destroy_account per tx_output.accounts_to_delete). The balance
 * already moved at SELFDESTRUCT time. Call AFTER the tx-level commit, so only
 * surviving (committed) selfdestructs are in k_selfdestr. */
function k_finalize_selfdestructs() -> unit = {
    var xs : list(address) = address_sort_unique(read_selfdestr());
    var more : bool = true;
    while more do {
        match xs {
            a :: r => {
                if (k_fork < FORK_CANCUN) | k_was_created(a) then {
                    k_emit_burn_log(a, load_account(a).balance);
                    k_account_delete(a)
                };
                xs = r
            },
            [||]   => more = false,
        }
    }
}

/* --- call-frame snapshot / commit / revert: the EVM brackets every CALL- and
 * CREATE-family sub-call with a snapshot, then commits on success or reverts on
 * REVERT / exceptional halt, giving each frame's effects all-or-nothing atomicity --- */

/* Open a checkpoint: push a journal marker. Storage writes are reverted via
 * the JStor journal, so there is no separate storage overlay layer to push. */
function k_snapshot() -> unit = {
    journal_push(JCheck())
}

/* full world wipe between independent blocks/fixtures (driver/test only) */
function k_world_reset() -> unit = {
    acctmap_reset();
    storage_map_reset();     /* native base cache */
    storage_wset_reset();    /* write-set overlay (tx + block) */
    transient_reset();
    warm_reset();
    logs_reset();
    k_refund = 0;
    selfdestr_reset();
    created_reset();
    journal_reset()
}

/* --- tx-lifecycle / driver helpers (system layer, not EVM user space) --- */
function k_set_block(env : BlockEnv) -> unit = k_block = env
function k_set_tx(env : TxEnv) -> unit = k_tx = env
function k_tx_reset() -> unit = {
    /* per-tx: clear ephemeral state, snapshot tx-original storage */
    warm_reset();
    transient_reset();
    logs_reset();
    k_refund = 0;
    selfdestr_reset();
    created_reset();
    journal_reset()
    /* no per-tx original-storage snapshot: the overlay's base layer IS tx-start state */
}

/* commit: drop the most recent checkpoint; the frame's undo entries fold into
 * the parent frame (so a later parent revert also undoes them). The C journal
 * splices out the most recent checkpoint marker, keeping all other entries. */
function k_commit() -> unit = {
    journal_commit() /* the frame's JStor undos fold into the parent; no overlay merge */
}

/* Merge a completed TOP-LEVEL boundary's net storage writes (a transaction or a
 * system call) from the per-tx overlay into the block base, then clear the
 * overlay. Called once after the boundary's OUTERMOST commit-or-revert -- never
 * per call frame (a frame's writes must stay tx-atomic until the tx settles).
 * Only dirty (net-change) rows reach the base; a fully-reverted boundary leaves
 * none, so this just clears the overlay. The block base is the compute_root
 * cursor (host/base_*.sail storage_updates). */
function k_tx_merge() -> unit = storage_wset_merge()

/* revert: replay undo entries back to (and dropping) the most recent checkpoint.
 * Decode the top entry into a JEntry, drop it, then dispatch: stop on JCheck,
 * otherwise apply_undo. Stop on an empty journal. */
function k_revert() -> unit = {
    /* storage undos are JStor entries replayed below; no overlay layer to drop */
    var done : bool = false;
    while not_bool(done) do {
        let tag = journal_top_tag();
        if tag == JT_EMPTY then {
            done = true
        } else {
            let e = journal_decode_top(tag);
            journal_drop_top();
            match e {
                JCheck() => done = true,
                _        => apply_undo(e),
            }
        }
    }
}
