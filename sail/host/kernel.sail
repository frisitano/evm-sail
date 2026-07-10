/* ======================================================================== */
/* HOST KERNEL FUNCTIONS                                                    */
/*                                                                          */
/* The host's syscall surface (k_*): the ONLY interface through which the   */
/* EVM (user space) reads or mutates world state                            */
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
 * stateless_storage is the base storage primitive (declared in host/state.sail,
 * defined in lib/mpt.sail as an authenticated MPT point-get -- one walk for
 * both the witness and the harness-built alloc trie). Everything above it --
 * the overlay, the read-through, the journal -- is common. */
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

/* --- accounts (BALANCE / EXTCODEHASH / EXTCODESIZE / nonce, transfers) --- */
function k_get_balance(a : address) -> word = {
    k_aload(a).balance
}
function k_get_nonce(a : address) -> u64 = {
    k_aload(a).nonce
}

/* the raw codeHash used to key the code store (KECCAK_EMPTY for codeless accounts) */
function k_code_key(a : address) -> hash = k_aload(a).code_hash

/* EXTCODEHASH (EIP-1052): the account's code binding. A truly empty
 * (nonexistent per EIP-161) account reads as 0, not KECCAK_EMPTY. */
function k_get_codehash(a : address) -> hash = {
    let acc = k_aload(a);
    if (acc.code_hash == KECCAK_EMPTY) & acc.nonce == sail_zeros(64) & word_is_zero(acc.balance) then {
        word_of_int(0)
    } else {
        acc.code_hash
    }
}
function k_account_is_empty(a : address) -> bool = {
    /* EIP-161 "empty" account: nonce 0, balance 0, no code (unrecorded query) */
    let acc = k_aload(a);
    (acc.code_hash == KECCAK_EMPTY) & acc.nonce == sail_zeros(64) & word_is_zero(acc.balance)
}

/* CREATE/CREATE2/create-tx address-collision test (unrecorded query): the
 * target is occupied if it has code OR a nonzero nonce OR any storage
 * (EELS account_has_code_or_nonce || account_has_storage). */
function k_account_occupied(a : address) -> bool = {
    let acc = k_aload(a);
    let has_code : bool = acc.code_hash != KECCAK_EMPTY;
    let has_storage : bool = (acc.storage_root != EMPTY_TRIE_ROOT) | storage_has_live_addr(a);
    has_code | (acc.nonce != sail_zeros(64)) | has_storage
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
    /* EELS move_ether reads (touches) both accounts unconditionally -- even a
     * zero or self transfer -- so both stay in the account set (BAL). */
    let _ = k_aload(src);
    let _ = k_aload(dst);
    if word_is_zero(v) | (src == dst) then {
        ()
    } else {
        journal_push(JAcct(src, k_aload(src)));
        journal_push(JAcct(dst, k_aload(dst)));
        store_account(src, { k_aload(src) with balance = alu_sub(k_aload(src).balance, v) });
        store_account(dst, { k_aload(dst) with balance = alu_add(k_aload(dst).balance, v) });
        k_emit_transfer_log(src, dst, v)
    }
}

/* Increment an account's nonce (sender per tx, creator at CREATE/CREATE2).
 * The u64 increment cannot wrap: EIP-2681 guards every path that reaches a
 * bump (tx validity rejects a sender at 2^64-1; CREATE and EIP-7702
 * authorization processing skip at the cap before incrementing). */
function k_bump_nonce(a : address) -> unit = {
    journal_push(JAcct(a, k_aload(a)));
    store_account(a, { k_aload(a) with nonce = get_slice_int(64, unsigned(k_aload(a).nonce) + 1, 0) })
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
    journal_push(JAcct(a, k_aload(a)));
    let h : hash = code_intern_returndata();
    store_account(a, { k_aload(a) with code_hash = h })
}
function k_set_delegation(a : address, target : address) -> unit = {
    journal_push(JAcct(a, k_aload(a)));
    let h : hash = code_intern_delegation(target);
    store_account(a, { k_aload(a) with code_hash = h })
}
function k_clear_code(a : address) -> unit = {
    journal_push(JAcct(a, k_aload(a)));
    store_account(a, { k_aload(a) with code_hash = KECCAK_EMPTY })
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
        throw InvalidBlock(WitnessDeficient)
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

/* single-account balance credit/debit (gas fees, the coinbase priority-fee
 * payment, withdrawals); both journal the prior account for frame revert. */
function k_add_balance(a : address, v : word) -> unit = {
    /* EELS modify_state always reads (touches) the account, even for a zero
     * delta, so it stays in the account set (BAL). Only the write is skipped
     * at zero -- a zero delta is not a net balance change. */
    let cur = k_aload(a);
    if not_bool(word_is_zero(v)) then {
        journal_push(JAcct(a, cur));
        store_account(a, { cur with balance = alu_add(cur.balance, v) })
    }
}
function k_sub_balance(a : address, v : word) -> unit = {
    let cur = k_aload(a);
    if not_bool(word_is_zero(v)) then {
        journal_push(JAcct(a, cur));
        store_account(a, { cur with balance = alu_sub(cur.balance, v) })
    }
}

/* --- block / tx environment reads (NUMBER, TIMESTAMP, COINBASE, BASEFEE,
 * CHAINID, GASLIMIT, PREVRANDAO, BLOBBASEFEE, ORIGIN, GASPRICE) --- */
function k_env(f : EnvField) -> word = {
    match f {
        F_Number => word_of_int(k_header.number),
        F_Timestamp => word_of_int(k_header.timestamp),
        F_Coinbase => address_to_word(k_header.fee_recipient),
        F_BaseFee => k_header.base_fee,
        F_ChainId => word_of_int(cc_chain_id()),
        F_GasLimit => word_of_int(k_header.gas_limit),
        F_PrevRandao => k_header.prev_randao,
        F_Origin => address_to_word(k_tx.origin),
        F_GasPrice => k_tx.gas_price,
        F_SlotNumber => sail_zero_extend(k_header.slot_number, 256),
    }
}
function k_coinbase() -> address = k_header.fee_recipient

/* BLOCKHASH / EIP-2935: the hash of the ancestor `offset` blocks back (1..256)
 * is keccak of the witness header at that depth. Headers run oldest..newest
 * with the parent (offset 1) LAST, so the ancestor at `offset` is
 * header[k_n_headers - offset]. k_n_headers is the authenticated witness
 * header count (authenticate_parent_state -> wit_n_headers()): the guest
 * carries its real ancestor chain; the native runner carries a single dummy
 * parent (count 1), so only BLOCKHASH of the immediate parent resolves and
 * anything deeper is idx < 0. A requested header beyond the witness is a
 * deficient proof -> throw InvalidBlock(WitnessDeficient), caught once at the
 * top (run_one). */
function k_blockhash(n : word) -> hash = {
    let req = unsigned(n);
    let offset = k_header.number - req;
    if offset < 1 | offset > 256 then {
        word_of_int(0)
    } else {
        let idx = k_n_headers - offset;
        if idx < 0 then {
            throw InvalidBlock(WitnessDeficient)
        } else {
            witness_keccak(ssz_witness_header_slice(idx))
        }
    }
}

/* BLOBHASH (EIP-4844): the i-th versioned hash of the current tx, 0 past the end. */
function k_blobhash(i : int) -> word = {
    list_nth(k_tx.blob_hashes, i, word_of_int(0))
}

/* --- address derivation primitives (CREATE / CREATE2) --- */
function k_create_addr(a : address, nonce : u64) -> address = create_address(a, nonce)
function k_create2_addr(a : address, salt : word, inithash : hash) -> address =
    create2_address(a, salt, inithash)

/* --- logs (LOG0..LOG4) + the gas-refund counter --- */
function k_log(a : address, topics : list(word), data : list(byte)) -> unit = {
    journal_push(JLog());
    emit_log(a, topics, data)
}
function k_refund_add(delta : int) -> unit = {
    journal_push(JRefund(k_refund));
    k_refund = to_i64(k_refund + delta)
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
    if word_is_zero(k_aload(a).balance) then {
        ()
    } else {
        journal_push(JAcct(a, k_aload(a)));
        store_account(a, { k_aload(a) with balance = ZERO_WORD })
    }
}

/* delete an account outright: drop it from the eest_account and wipe its storage.
 * Used by the EIP-6780 tx-end finalization (created AND selfdestructed in the
 * same tx). Unrecorded: the tx-end reduction is host bookkeeping. */
function k_account_delete(a : address) -> unit = {
    /* Set the account EMPTY in place (do NOT eest_account_remove it). Removing it would
       let a later k_aload(a) RE-MATERIALIZE the account from the pre-state witness
       leaf -- wrong for an account CREATEd+SELFDESTRUCTed and then re-CREATEd later
       in the same block (the stale pre-state balance would resurface). Kept empty
       in the eest_account, a later read sees the deletion, the post-state walk drops it
       (account_empty) AND marks it touched (sb_touched) so its pre-state leaf is not
       reused -- subsuming the old k_deleted mechanism. */
    store_account(a, EMPTY_ACCOUNT);
    /* EIP-7928: storage reads survive account deletion (EELS) -- drain the
       address's tx rows, recording each touched slot as a read (every tx row
       was read; its changes are voided by the drop), then drop the block rows
       so a same-block re-CREATE cannot inherit stale cached storage. */
    var more : bool = true;
    while more do {
        match storage_tx_wipe(a) {
            Some(slot) => bal_storage_read(a, slot),
            None() => more = false,
        }
    };
    storage_block_wipe(a)
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
                    k_emit_burn_log(a, k_aload(a).balance);
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

/* The full world wipe between independent blocks/fixtures is a DRIVER/TEST
 * concern, not EVM semantics, so it lives in the harness (test_utils.c
 * evmsail_clear_memory) rather than the model. The production guest never needs
 * it (fresh process, zeroed FFI statics); the in-process ctypes harnesses call
 * clear_memory between runs. k_world_reset was removed accordingly. */

/* --- tx-lifecycle / driver helpers (system layer, not EVM user space) --- */
function k_set_header(h : BlockHeader) -> unit = k_header = h
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
 * cursor (lib/mpt.sail storage_updates). */
function k_tx_merge() -> unit = {
    /* THE MERGE, stated here: drain each tx layer (pop is side-effect-free;
       None = drained and the layer resets), and per row decide BOTH the
       EIP-7928 records (tagged C-side with the ambient bal_set_index) and
       the block-layer propagation. Storage: a net change (curr != orig) is
       a storage_change with the tx-final value and a block put; else the
       row is a read (every tx row was read: storage_tx_cache is the only
       row creator) -- a storage_read record and a block read-cache; a
       net-zero SSTORE is a read, matching EELS.
       Accounts: per-field diffs vs the tx-start original make the records;
       a changed account writes the block layer, an unchanged one merges as
       a read member (it must still reach the BAL account list). */
    var more : bool = true;
    while more do {
        match storage_tx_pop() {
            Some(e) => if e.value.curr != e.value.orig then {
                bal_storage_change(e.key.addr, e.key.slot, e.value.curr);
                storage_block_put(e.key.addr, e.key.slot, e.value.curr, e.value.orig)
            } else {
                bal_storage_read(e.key.addr, e.key.slot);
                storage_block_cache(e.key.addr, e.key.slot, e.value.curr)
            },
            None() => more = false,
        }
    };
    more = true;
    while more do {
        match acct_tx_pop() {
            Some(e) => {
                if e.value.curr.nonce != e.value.orig.nonce then {
                    bal_nonce_change(e.addr, e.value.curr.nonce)
                };
                if e.value.curr.balance != e.value.orig.balance then {
                    bal_balance_change(e.addr, e.value.curr.balance)
                };
                if e.value.curr.code_hash != e.value.orig.code_hash then {
                    bal_code_change(e.addr, e.value.curr.code_hash)
                };
                if account_changed(e.value.curr, e.value.orig) then {
                    acct_block_write(
                        e.addr,
                        e.value.curr.nonce,
                        e.value.curr.balance,
                        e.value.curr.storage_root,
                        e.value.curr.code_hash,
                        e.value.orig.nonce,
                        e.value.orig.balance,
                        e.value.orig.storage_root,
                        e.value.orig.code_hash,
                    )
                } else {
                    acct_block_cache(
                        e.addr,
                        e.value.curr.nonce,
                        e.value.curr.balance,
                        e.value.curr.storage_root,
                        e.value.curr.code_hash,
                    )
                }
            },
            None() => more = false,
        }
    }
}

/* revert: replay undo entries back to (and dropping) the most recent checkpoint.
 * journal_pop decodes + drops the top entry: stop on JCheck (the checkpoint
 * marker, or an empty journal), otherwise apply_undo. */
function k_revert() -> unit = {
    /* storage undos are JStor entries replayed below; no overlay layer to drop */
    var done : bool = false;
    while not_bool(done) do {
        let e = journal_pop();
        match e {
            JCheck() => done = true,
            _        => apply_undo(e),
        }
    }
}
