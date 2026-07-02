/* ======================================================================== */
/* TRANSACTION STATE TRANSITION                                             */
/*                                                                          */
/* The per-transaction step of the Ethereum state transition (Yellow Paper  */
/* section 6): validate the transaction, charge upfront gas, run it as the  */
/* top-level message call, then settle gas/refunds and the coinbase fee.    */
/*                                                                          */
/* Validity and intrinsic gas are user-space policy; every world effect is  */
/* a kernel syscall, and a tx-level snapshot/commit/revert bounds the whole */
/* transaction's world state. The typed-envelope rules covered here:        */
/* EIP-2718 (typed tx), EIP-1559 (fee market: base fee + priority tip       */
/* caps), EIP-2930 (access list), EIP-3860 (initcode size/gas), EIP-4844    */
/* (blob tx + blob-gas burn), EIP-7702 (set-code authorizations), EIP-7623  */
/* (calldata floor), EIP-3607 (no tx from an account with code), EIP-7825   */
/* (gas cap). Gas refunds (SSTORE clears, etc.) are capped per EIP-3529.    */
/* ======================================================================== */

/* An EIP-7702 set-code authorization tuple. The runner pre-recovers the
 * authority and validates the signature (s <= n/2, y_parity, r); valid_sig
 * records that result, and process_auth applies the chain-id/nonce/code checks. */
struct Authorization = {
    valid_sig : bool, authority : address, address : address,
    nonce : int, chain_id : int,
}

/* A transaction. Covers the EIP-2718 typed envelopes 0..4: legacy, EIP-2930
 * (access list), EIP-1559 (fee market), EIP-4844 (blob), EIP-7702 (set code);
 * the type-specific fields below are validity-relevant per their EIP. */
struct Transaction = {
  sender : address, nonce : int, gas_price : word, priority_fee : word,
  gas_limit : gas, is_create : bool, recipient : address, value : word,
  input_index : int,                /* transaction index: its input bytes live in memory (txin) */
  access_list_addresses : list(address),   /* EIP-2930 access-list addresses (one per entry) */
  access_list_slots  : list(SlotKey),   /* EIP-2930 access-list (address, slot) pairs      */
  is_access_list : bool,      /* type-1 (EIP-2930) envelope, even if the list is empty */
  is_dynamic_fee : bool,      /* EIP-1559-style fee-market envelope (types 2/3/4) */
  blob_count : int,           /* EIP-4844 number of blob versioned hashes         */
  max_fee : word,             /* EIP-1559 max_fee_per_gas (cap, for validity)     */
  max_blob_fee : word,        /* EIP-4844 max_fee_per_blob_gas (cap, for validity)*/
  max_priority_fee : word,        /* EIP-1559 max_priority_fee_per_gas (cap, for validity) */
  is_set_code : bool,          /* type-4 (EIP-7702) envelope (even with 0 tuples)  */
  authorizations : list(Authorization), /* EIP-7702 set-code authorization tuples      */
  is_blob : bool,             /* EIP-4844 type-3 marker (blob validity rules)     */
  blob_hashes : list(word)    /* EIP-4844 versioned hashes (BLOBHASH operands)    */
}

/* success: the top-level frame returned/stopped without reverting or
 * exceptionally halting (used for create-tx code deposit, log inclusion).
 * valid: the tx passed transaction VALIDITY (nonce/balance/intrinsic/floor/
 * fee/blob/7702 rules) and was applied to the block. A valid tx that REVERTS
 * or runs out of gas during execution still has valid = true -- it is a
 * legitimate block member (gas charged, nonce bumped). Block validity (the
 * stateless guest's successful_validation) folds valid, NOT success:
 * only an INVALID/inapplicable tx makes the containing block invalid. */
/* block_gas: EIP-7778 block-gas accounting WITHOUT the EIP-3529 refund
 * (= max(gas_used_before_refund, calldata_floor)). The block driver sums this
 * for the block gas-availability check and the header gas_used; it differs from
 * gas_used (the receipt's refunded gas) precisely by the refund. */
struct Receipt = { success : bool, valid : bool, gas_used : gas, block_gas : gas, logs : list(LogEntry) }

let G_transaction : gas = 21000
let G_txcreate : gas = 32000
let G_txdatazero : gas = 4
let G_txdatanonzero : gas = 16
let G_access_list_address : gas = 2400 /* EIP-2930 ACCESS_LIST_ADDRESS_COST     */
let G_access_list_storage_key : gas = 1900 /* EIP-2930 ACCESS_LIST_STORAGE_KEY_COST */
let PER_AUTH_BASE : gas = 12500 /* EIP-7702 PER_AUTH_BASE_COST          */
let PER_EMPTY_ACCOUNT : gas = 25000 /* EIP-7702 PER_EMPTY_ACCOUNT_COST      */

function calldata_cost() -> gas = {
    /* EIP-2028: 4 gas per zero byte, 16 per non-zero, over the loaded tx input */
    let n = txdata_size();
    var g : int = 0;
    foreach (i from 0 to (n - 1)) {
        g = g + (if txdata_byte(i) == 0x00 then G_txdatazero else G_txdatanonzero)
    };
    to_gas(g)
}

/* EIP-7623 calldata tokens: each zero byte = 1 token, each nonzero byte = 4. */
function calldata_tokens() -> int = {
    let n = txdata_size();
    var t : int = 0;
    foreach (i from 0 to (n - 1)) {
        t = t + (if txdata_byte(i) == 0x00 then 1 else 4)
    };
    t
}

/* EIP-4844: every blob versioned hash must carry VERSIONED_HASH_VERSION_KZG (0x01). */
function blob_hashes_versioned(xs : list(word)) -> bool =
    match xs {
        [||]   => true,
        h :: r => if h[255 .. 248] == 0x01 then {
            blob_hashes_versioned(r)
        } else {
            false
        },
    }

/* intrinsic gas: tx input already loaded into the C buffer (txdata_load), so
 * calldata cost and the EIP-3860 initcode words read it via txdata_size. */
function intrinsic_gas(tx : Transaction) -> gas = {
    let access_list_gas =   G_access_list_address
                          * list_len(tx.access_list_addresses)
                          + G_access_list_storage_key
                          * list_len(tx.access_list_slots);
    let auth = PER_EMPTY_ACCOUNT * list_len(tx.authorizations); /* EIP-7702 */
    let base = G_transaction + calldata_cost() + access_list_gas + auth;
    to_gas(if tx.is_create then base + G_txcreate + initcode_gas(txdata_size()) else base)
}

/* EIP-7702: apply one authorization (validate against current state, set/clear the
 * delegation, bump the authority nonce; refund if the authority already existed). */
function process_auth(au : Authorization) -> unit = {
    let authority = au.authority;
    let chain_ok = (au.chain_id == 0) | (au.chain_id == unsigned(k_block.chain_id));

    /* EELS validates sig + chain id from the tuple ALONE before reading the
     * authority's account; a tuple rejected there (e.g. wrong chain id) touches
     * NO state, so its authority need not be witnessed -- reading it eagerly would
     * over-access the witness. Gate every authority-state read on sig & chain_ok. */
    if au.valid_sig & chain_ok then {
        /* step 5: warm the authority BEFORE the code/nonce checks -- a tuple later
         * skipped for code/nonce still warms its authority. */
        let _ = k_access_account(authority);
        let (is_deleg, _) = k_deleg_target(authority);
        let code_ok = (k_code_key(authority) == KECCAK_EMPTY) | is_deleg;
        let nonce_ok = k_get_nonce(authority) == au.nonce;
        if code_ok & nonce_ok then {
            let existed = not_bool(k_account_is_empty(authority));
            if unsigned(address_to_word(au.address)) == 0 then {
                k_set_code(authority, [||])
                /* clear delegation */
            } else {
                k_set_code(authority, delegation_code(au.address))
            };
            k_bump_nonce(authority);
            if existed then {
                k_refund_add(PER_EMPTY_ACCOUNT - PER_AUTH_BASE)
            }
        }
    }
}
function process_auth_list(xs : list(Authorization)) -> unit =
    match xs {
        [||] => (),
        a :: r => {
            process_auth(a);
            process_auth_list(r)
        },
    }

function warm_access_list_addresses(xs : list(address)) -> unit =
    match xs {
        [||] => (),
        a :: r => {
            let _ = k_access_account(a);
            warm_access_list_addresses(r)
        },
    }
function warm_access_list_slots(xs : list(SlotKey)) -> unit =
    match xs {
        [||] => (),
        k :: r => {
            let _ = k_access_slot(k.address, k.slot);
            warm_access_list_slots(r)
        },
    }

/* EIP-2929 + EIP-2930 pre-warm sender, target, precompiles, and the access
 * list. EIP-3651 additionally warms coinbase from Shanghai onward. */
function prewarm(tx : Transaction) -> unit = {
    let _ = k_access_account(tx.sender);
    let _ = if tx.is_create then {
            false
        } else {
            k_access_account(tx.recipient)
        };
    if k_fork >= FORK_SHANGHAI then {
        let _ = k_access_account(k_coinbase())
    };

    /* pre-warm the active fork's precompile set only (EIP-2929 seeds
       accessed_addresses with PRE_COMPILED_CONTRACTS, which GROWS by fork:
       Cancun 0x01-0x0a, Prague +BLS 0x0b-0x11, Osaka +0x100 P256VERIFY). */
    foreach (p from 1 to 17) {
        if is_precompile(p) then {
            let _ = k_access_account(word_to_address(word_of_int(p)))
        }
    };
    if is_precompile(256) then {
        let _ = k_access_account(word_to_address(word_of_int(256))) /* 0x100 P256VERIFY */
    };
    warm_access_list_addresses(tx.access_list_addresses);
    warm_access_list_slots(tx.access_list_slots)
}

struct TxValidity = {
    valid : bool,
    sender : address,
    nonce_before : int,
    intrinsic_gas : gas,
    blob_fee : int,
    input_len : int,
}

struct TxFrameResult = {
    success : bool,
    gas_remaining : gas,
}

function invalid_receipt() -> Receipt =
    struct { success = false, valid = false, gas_used = 0, block_gas = 0, logs = [||] }

function check_transaction_validity(tx : Transaction, dlen : int) -> TxValidity = {
    let sender = tx.sender;
    let nonce_before = k_get_nonce(sender);
    let intrinsic = intrinsic_gas(tx);
    let blob_fee = tx.blob_count * GAS_PER_BLOB * unsigned(k_block.blob_base_fee);

    /* Transaction validity: an invalid tx is REJECTED with no state change at all
       (no nonce bump, no gas charge). The upfront-balance test uses the fee CAPS
       (max_fee_per_gas, max_fee_per_blob_gas), not the effective prices. */
    let blob_gas = tx.blob_count * GAS_PER_BLOB;
    let upfront = tx.gas_limit * unsigned(tx.max_fee) + unsigned(tx.value) + blob_gas * unsigned(tx.max_blob_fee);
    let (sender_deleg, _) = k_deleg_target(sender);

    /* EIP-4844 / EIP-7691 / EIP-7594 blob-count bounds: Cancun max 6 per block,
       Prague max 9 per block, Osaka adds MAX_BLOBS_PER_TX = 6. A type-3 tx must
       carry >= 1 blob, must not create, and every versioned hash must be 0x01-tagged. */
    let max_blobs : int = if k_fork >= FORK_OSAKA then { 6 } else { if k_fork >= FORK_PRAGUE then { 9 } else { 6 } };
    let blob_ok : bool = if tx.is_blob then {
              (k_fork >= FORK_CANCUN)
            & (tx.blob_count >= 1)
            & (tx.blob_count <= max_blobs)
            & not_bool(tx.is_create)
            & blob_hashes_versioned(tx.blob_hashes)
        } else {
            true
        };

    /* EIP-7623 (Prague+): the calldata floor is ALSO a validity bound -- a tx whose
       gas limit cannot cover 21000 + 10*tokens is rejected outright. */
    let floor_ok : bool = (k_fork < FORK_PRAGUE) | (tx.gas_limit >= G_transaction + 10 * calldata_tokens());
    let valid =   (unsigned(k_get_balance(sender)) >= upfront)
                & /* covers upfront cost      */
                  ((k_code_key(sender) == KECCAK_EMPTY) | sender_deleg)
                & /* EIP-3607 (7702 exempts delegations) */
                  (tx.gas_limit >= intrinsic)
                & /* gas covers intrinsic     */
                  floor_ok
                & /* EIP-7623 floor coverage  */
                  (unsigned(tx.max_fee) >= unsigned(k_block.base_fee))
                & /* >= base fee */
                  ((tx.blob_count == 0) |
                      /* blob fee cap >= price    */
                      (unsigned(tx.max_blob_fee) >= unsigned(k_block.blob_base_fee)))
                & blob_ok
                & (not_bool(tx.is_create) | (dlen <= max_initcode_size()))
                & /* EIP-3860 */
                  ((k_fork < FORK_OSAKA) | (tx.gas_limit <= 16777216))
                & /* EIP-7825: gas cap 2^24 */
                  (unsigned(tx.max_priority_fee) <= unsigned(tx.max_fee))
                & /* EIP-1559: prio cap <= fee cap */
                  (not_bool(tx.is_access_list) | (k_fork >= FORK_BERLIN))
                & /* EIP-2930 type-1 tx is Berlin+ */
                  (not_bool(tx.is_dynamic_fee) | (k_fork >= FORK_LONDON))
                & /* EIP-1559 fee-market txs are London+ */
                  (not_bool(tx.is_set_code) | not_bool(tx.is_create))
                & /* EIP-7702: type-4 has to != nil */
                  (not_bool(tx.is_set_code) | (list_len(tx.authorizations) > 0))
                & /* EIP-7702: non-empty auth list */
                  (not_bool(tx.is_set_code) | (k_fork >= FORK_PRAGUE))
                & /* EIP-7702 is Prague+: a type-4 tx is invalid on an earlier fork */
                  (nonce_before < 18446744073709551615)
                & /* EIP-2681: nonce < 2^64-1 (incrementable) */
                  (tx.gas_limit <= unsigned(k_block.gas_limit)); /* tx gas <= block gas limit */

    struct {
        valid = valid,
        sender = sender,
        nonce_before = nonce_before,
        intrinsic_gas = intrinsic,
        blob_fee = blob_fee,
        input_len = dlen,
    }
}

function apply_transaction_upfront_effects(tx : Transaction, v : TxValidity) -> unit = {
    /* upfront gas + nonce bump (persist across revert; taken before the snapshot) */
    k_sub_balance(v.sender, word_of_int(tx.gas_limit * unsigned(tx.gas_price)));

    /* EIP-4844: burn the blob fee upfront (blob_gas_used * blob_gas_price), no refund */
    if v.blob_fee > 0 then {
        k_sub_balance(v.sender, word_of_int(v.blob_fee))
    };
    k_bump_nonce(v.sender);
    prewarm(tx);
    process_auth_list(tx.authorizations); /* EIP-7702: set delegations before execution (persist) */
    k_snapshot()
}

function enter_transaction_frame(tx : Transaction, intrinsic : gas) -> unit = {
    pc = 0;
    call_depth = 0;
    gas_remaining = to_gas(tx.gas_limit - intrinsic);
    stack_clear();
    mem_clear();
    hr_reset();
    memory_size = 0;
    frame_status = Running()
}

function run_create_transaction_frame(tx : Transaction, sender : address, nonce_before : int) -> unit = {
    let new_addr = k_create_addr(sender, nonce_before);
    let _ = k_access_account(new_addr);

    /* create-tx address collision (code, nonzero nonce, or storage at the
     * target): the tx fails outright consuming ALL gas, no initcode runs
     * (EELS process_message_call -> AddressCollision, gas_left = 0). */
    if k_account_occupied(new_addr) then {
        exc_halt(AddressCollision)
    } else {
        k_mark_created(new_addr); /* EIP-6780: created this tx */
        k_bump_nonce(new_addr);
        if word_nonzero(tx.value) then {
            k_transfer(sender, new_addr, tx.value)
        };
        message =
            struct {
                caller = sender,
                address = new_addr,
                code_address = new_addr,
                value = tx.value,
                data_len = 0,
                is_static = false,
                depth = 0,
            };
        frame_code_txd();
        interpret();
        if frame_succeeded() then {
            let dep_len = unsigned(hr_pending_len());
            let deposit = G_codedeposit * dep_len;
            let code_ok = (dep_len <= max_code_size()) &
                    ((dep_len == 0) | (hr_pending_byte(get_slice_int(64, 0, 0)) != 0xef)); /* EIP-3541 */
            if code_ok & (gas_remaining >= deposit) then {
                gas_remaining = to_gas(gas_remaining - deposit);
                k_set_code(new_addr, pending_output_list());
                hr_discard()
            } else {
                hr_discard();
                exc_halt(OutOfGas)
            } /* failed deploy */
        }
    }
}

function run_call_transaction_frame(tx : Transaction, sender : address, dlen : int) -> unit = {
    if word_nonzero(tx.value) then {
        k_transfer(sender, tx.recipient, tx.value)
    };
    let an = unsigned(tx.recipient);
    if is_precompile(an) then {
        /* tx directly to a precompile: run it as the top-level frame, gas-checked
           FIRST (an OOG precompile must not execute). Failure or OOG is an
           exceptional halt (all gas consumed, value transfer reverted). */
        stage_precompile_txd(an);
        let used = precompile_gas(an, dlen);
        let ok : bool = if used <= gas_remaining then {
                k_precompile(an)
            } else {
                false
            };
        if ok & (used <= gas_remaining) then {
            gas_remaining = to_gas(gas_remaining - used);
            hr_capture_acc();
            hr_adopt();
            frame_status = Halted(HaltReturn)
        } else {
            exc_halt(OutOfGas)
        }
    } else {
        cd_set_tx();
        message =
            struct {
                caller = sender,
                address = tx.recipient,
                code_address = tx.recipient,
                value = tx.value,
                data_len = dlen,
                is_static = false,
                depth = 0,
            };

        /* EIP-7702: at the tx level the delegate is warmed but not separately charged */
        let (tx_deleg, tx_dtgt) = k_deleg_target(tx.recipient);
        if tx_deleg then {
            let _ = k_access_account(tx_dtgt);
            ()
        };
        frame_code_exec(tx.recipient, tx_deleg, tx_dtgt); /* follow a delegation designation */
        interpret()
    }
}

function run_transaction_frame(tx : Transaction, v : TxValidity) -> TxFrameResult = {
    enter_transaction_frame(tx, v.intrinsic_gas);
    if tx.is_create then {
        run_create_transaction_frame(tx, v.sender, v.nonce_before)
    } else {
        run_call_transaction_frame(tx, v.sender, v.input_len)
    };

    let success = frame_succeeded();
    if success then {
        k_commit()
    } else {
        k_revert()
    };
    struct { success = success, gas_remaining = gas_remaining }
}

function settle_transaction(tx : Transaction, sender : address, fr : TxFrameResult) -> Receipt = {
    /* Refund cap: gas_used / 2 before London, gas_used / 5 after EIP-3529. */
    let gas_left0 = fr.gas_remaining;
    let gas_used0 = tx.gas_limit - gas_left0;
    let refund_quotient = if k_fork >= FORK_LONDON then { 5 } else { 2 };
    let refund_cap = ediv_int(gas_used0, refund_quotient);
    let refund = if k_refund < refund_cap then { k_refund } else { refund_cap };
    let refund_pos = if refund < 0 then { 0 } else { refund };
    let gas_left1 = gas_left0 + refund_pos;
    let gas_used1 = tx.gas_limit - gas_left1;

    /* EIP-7623 (Prague+): a tx pays at least the calldata floor 21000 + 10*tokens */
    let floor : int = if k_fork >= FORK_PRAGUE then {
            G_transaction + 10 * calldata_tokens()
        } else {
            0
        };
    let gas_used = if gas_used1 < floor then { floor } else { gas_used1 };
    let gas_left = tx.gas_limit - gas_used;

    /* EIP-7778: the block charges the UNREFUNDED gas (floored), not the receipt's
       refunded gas -- this is what bounds the block gas limit and the header. */
    let block_gas = if gas_used0 < floor then { floor } else { gas_used0 };

    /* return unused gas to sender; pay coinbase the priority fee */
    k_add_balance(sender, word_of_int(gas_left * unsigned(tx.gas_price)));
    k_add_balance(k_coinbase(), word_of_int(gas_used * unsigned(tx.priority_fee)));

    /* EIP-6780 tx-end reduction: delete accounts created AND selfdestructed in
     * this tx (EELS destroys tx_output.accounts_to_delete after the fee
     * transfers; a failed tx has no surviving selfdestructs to apply). */
    if fr.success then {
        k_finalize_selfdestructs()
    };

    struct {
        success = fr.success,
        valid = true,
        gas_used = to_gas(gas_used),
        block_gas = to_gas(block_gas),
        logs = k_logs,
    }
}

function process_transaction(tx : Transaction) -> Receipt = {
    k_tx_reset();
    k_set_tx(struct { origin = tx.sender, gas_price = tx.gas_price, blob_hashes = tx.blob_hashes });
    hr_reset();

    /* select this tx's input (already loaded into memory at block setup); gas,
       validity, calldata and initcode all read it from there by index. */
    let dlen = txdata_activate(tx.input_index);
    let validity = check_transaction_validity(tx, dlen);
    if not_bool(validity.valid) then {
        return invalid_receipt()
    };
    apply_transaction_upfront_effects(tx, validity);
    let frame_result = run_transaction_frame(tx, validity);
    settle_transaction(tx, validity.sender, frame_result)
}
