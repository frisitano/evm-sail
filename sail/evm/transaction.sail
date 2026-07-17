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

let G_transaction : gas_constant = GasConstant(21000)
let G_txcreate : gas_constant = GasConstant(32000)
let G_txdatazero : gas_constant = GasConstant(4)
let G_txdatanonzero : gas_constant = GasConstant(16)
let G_access_list_address : gas_constant = GasConstant(2400) /* EIP-2930 ACCESS_LIST_ADDRESS_COST     */
let G_access_list_storage_key : gas_constant = GasConstant(1900) /* EIP-2930 ACCESS_LIST_STORAGE_KEY_COST */
let PER_AUTH_BASE : gas_constant = GasConstant(12500) /* EIP-7702 PER_AUTH_BASE_COST          */
let PER_EMPTY_ACCOUNT : gas_constant = GasConstant(25000) /* EIP-7702 PER_EMPTY_ACCOUNT_COST      */
struct TransactionCosts = {
    intrinsic : gas_cost,
    calldata_floor : gas_cost,
    blob_gas : blob_gas,
    blob_fee : word,
    upfront : word,
}

/* EIP-2028: 4 gas per zero byte, 16 per non-zero. The generic slice host
 * operation performs one native pass; zero bytes are the remainder. */
function calldata_cost(input : ByteSlice) -> gas_cost = {
    let ByteQuantity(nonzeroes) = slice_count_nonzero(input);
    let ByteQuantity(input_len) = input.len;
    let zeroes : nat = if nonzeroes <= input_len then {
            input_len - nonzeroes
        } else {
            throw InvalidBlock(ExecutionInvalid)
        };
    G_txdatazero * zeroes + G_txdatanonzero * nonzeroes
}

/* EIP-7623 calldata tokens: each zero byte = 1 token, each nonzero byte = 4. */
function calldata_tokens(input : ByteSlice) -> nat = {
    let ByteQuantity(nonzeroes) = slice_count_nonzero(input);
    let ByteQuantity(input_len) = input.len;
    let zeroes : nat = if nonzeroes <= input_len then {
            input_len - nonzeroes
        } else {
            throw InvalidBlock(ExecutionInvalid)
        };
    zeroes + 4 * nonzeroes
}

/* EIP-4844: every blob versioned hash must carry VERSIONED_HASH_VERSION_KZG (0x01). */
function blob_hashes_versioned(hashes : BlobHashes) -> bool = {
    var valid : bool = true;
    var remaining : blob_count = hashes.count;
    var offset : source_pointer = BYTE_ONE;

    /* Every supported fork admits at most nine blobs in one transaction. The
     * caller independently rejects a larger count, so this bounded scan never
     * needs a data-dependent loop. */
    foreach (i from 0 to 8) {
        if valid & (remaining != 0) then {
            valid = slice_byte(hashes.bytes, offset) == 0x01;
            remaining = protocol_quantity_decrement(remaining);
            if i < 8 then {
                offset = offset + BLOB_HASH_RLP_LENGTH
            }
        }
    };
    valid & (remaining == 0)
}

function intrinsic_gas(tx : Transaction) -> gas_cost = {
    let data_cost = calldata_cost(tx.input_src);
    let ByteQuantity(input_len) = tx.input_src.len;
    let address_cost = G_access_list_address * tx.access_list_address_count;
    let slot_cost = G_access_list_storage_key * tx.access_list_slot_count;
    let auth_cost = PER_EMPTY_ACCOUNT * tx.authorization_count;
    var total : gas_cost = data_cost + G_transaction;
    total = total + address_cost;
    total = total + slot_cost;
    total = total + auth_cost;
    if tx.is_create then {
        total = total + G_txcreate;
        total = total + initcode_gas(input_len)
    };
    total
}

function calldata_floor(input : ByteSlice) -> gas_cost =
    GasConstant(10) * calldata_tokens(input) + G_transaction

function transaction_costs(tx : Transaction, blob_price : word) -> option(TransactionCosts) = {
    let intrinsic = intrinsic_gas(tx);
    let floor = calldata_floor(tx.input_src);
    let blob_gas_value = tx.blob_hashes.count * GAS_PER_BLOB;
    let blob_gas : blob_gas = if blob_gas_value <= 2 ^ 64 - 1 then {
            blob_gas_value
        } else {
            return None()
        };
    let blob_fee : word = if blob_gas == 0 then {
            ZERO_WORD
        } else {
            match word_checked_mul(blob_price, blob_gas) {
                Some(amount) => amount,
                None() => return None(),
            }
        };
    let gas_cap : word = match word_checked_mul(tx.max_fee, tx.gas_limit) {
        Some(amount) => amount,
        None() => return None(),
    };
    let blob_cap : word = match word_checked_mul(tx.max_blob_fee, blob_gas) {
        Some(amount) => amount,
        None() => return None(),
    };
    let gas_and_value : word = match word_checked_add(gas_cap, tx.value) {
        Some(amount) => amount,
        None() => return None(),
    };
    let upfront : word = match word_checked_add(gas_and_value, blob_cap) {
        Some(amount) => amount,
        None() => return None(),
    };
    Some(
        struct {
            intrinsic = intrinsic,
            calldata_floor = floor,
            blob_gas = blob_gas,
            blob_fee = blob_fee,
            upfront = upfront,
        },
    )
}

/* These arithmetic relations were established by transaction validation or
 * an immediately preceding guard. Rechecking at the use site keeps the
 * boundary total and turns an internal invariant violation into an invalid
 * block instead of silently substituting a value. */
function validated_word_product(value : word, factor : gas) -> word =
    match word_checked_mul(value, factor) {
        Some(product) => product,
        None() => throw InvalidBlock(ExecutionInvalid),
    }

function validated_gas_add(Gas(left) : gas, Gas(right) : gas) -> gas =
    if right <= (2 ^ 63 - 1) - left then {
        Gas(left + right)
    } else {
        throw InvalidBlock(ExecutionInvalid)
    }

function validated_gas_sub_gas(Gas(left) : gas, Gas(right) : gas) -> gas =
    if right <= left then {
        Gas(left - right)
    } else {
        throw InvalidBlock(ExecutionInvalid)
    }

function validated_gas_sub_cost(Gas(left) : gas, GasCost(right) : gas_cost) -> gas =
    if right <= left then {
        Gas(left - right)
    } else {
        throw InvalidBlock(ExecutionInvalid)
    }

overload validated_gas_sub = {validated_gas_sub_gas, validated_gas_sub_cost}

/* EIP-7702: apply one authorization (validate against current state, set/clear the
 * delegation, bump the authority nonce; refund if the authority already existed). */
function process_auth(au : Authorization) -> gas_refund = {
    var refund = GAS_REFUND_ZERO;
    let authority = au.authority;
    let chain_ok = word_is_zero(au.chain_id) | (au.chain_id == word_of_nat(k_chain_id));

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
            let existed = k_account_exists(authority);
            if au.address == ZERO_ADDR then {
                k_clear_code(authority)
                /* clear delegation */
            } else {
                k_set_delegation(authority, au.address)
            };
            k_bump_nonce(authority);
            if existed then {
                refund = gas_cost_to_refund(PER_EMPTY_ACCOUNT - PER_AUTH_BASE)
            }
        }
    };
    refund
}

function process_auth_list(xs : list(Authorization)) -> gas_refund =
    match xs {
        [||] => GAS_REFUND_ZERO,
        a :: r => process_auth(a) + process_auth_list(r),
    }

function warm_access_list_addresses(xs : list(address)) -> unit =
    match xs {
        [||] => (),
        a :: r => {
            let _ = k_access_account(a);
            warm_access_list_addresses(r)
        },
    }
function warm_access_list_slots(xs : list(StorageKey)) -> unit =
    match xs {
        [||] => (),
        k :: r => {
            let _ = k_slot_is_warm(k.addr, k.slot);
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
    if k_fork >= Shanghai then {
        let _ = k_access_account(k_coinbase())
    };

    /* pre-warm the active fork's precompile set only (EIP-2929 seeds
       accessed_addresses with PRE_COMPILED_CONTRACTS, which GROWS by fork:
       Cancun 0x01-0x0a, Prague +BLS 0x0b-0x11, Osaka +0x100 P256VERIFY). */
    let precompile_addresses : vector(17, precompile_id) = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17];
    foreach (i from 0 to 16) {
        let p = precompile_addresses[i];
        if is_precompile(p) then {
            let _ = k_access_account(word_to_address(word_of_precompile_id(p)))
        }
    };
    if is_precompile(256) then {
        let _ = k_access_account(word_to_address(word_of_precompile_id(256))) /* P256VERIFY */
    };
    warm_access_list_addresses(tx.access_list_addresses);
    warm_access_list_slots(tx.access_list_slots)
}

function invalid_receipt() -> Receipt =
    struct {
        tx_type = LegacyTx,
        success = false,
        valid = false,
        gas_used = GAS_ZERO,
        block_gas = GAS_ZERO,
        logs = [||],
    }

/* Effective fee for a typed (EIP-1559) tx: the gas price actually paid is
 * min(max_fee_per_gas, base_fee + max_priority_fee), and the priority tip paid to
 * the coinbase is that price minus the base fee. Legacy / 2930 txs carry a single
 * gas_price, passed here as max_fee = max_priority = gas_price so this recovers
 * (gas_price, gas_price - base_fee). The priority is clamped at 0 so an invalid
 * sub-base-fee price (rejected later by the validity check) never underflows. */
function eff_gas_price_for(base_fee : word, max_fee : word, max_priority_fee : word) -> (word, word) = {
    let price : word = match word_checked_add(base_fee, max_priority_fee) {
        Some(uncapped) => if word_ult(max_fee, uncapped) then { max_fee } else { uncapped },
        None() => max_fee,
    };
    let priority = if word_ule(base_fee, price) then {
            sub_bits(price, base_fee)
        } else {
            ZERO_WORD
        };
    (price, priority)
}

function check_transaction_validity(tx : Transaction) -> TxValidity = {
    /* EELS check_transaction: the witnessed public key must verify this tx's
       signature over the signing hash (recover_sender_from_public_key) -- a
       forged key or bad v/r/s makes the whole BLOCK invalid, regardless of the
       validity verdict below. */
    if not_bool(
        tx_sig_v_ok(k_chain_id, tx.tx_type, tx.sig_v) & tx_auth_ok(tx.pubkey, tx.signing_hash, tx.sig_r, tx.sig_s),
    ) then {
        throw InvalidBlock(InvalidSignature)
    };

    /* effective gas price (EELS: computed in check_transaction) */
    let (eff_gas_price, eff_priority_fee) = eff_gas_price_for(k_header.base_fee, tx.max_fee, tx.max_priority_fee);
    let sender = tx.sender;
    let ByteQuantity(input_len) = tx.input_src.len;
    let nonce_before = k_get_nonce(sender);
    let blob_price = blob_base_fee(k_header.excess_blob_gas);
    let checked_costs = transaction_costs(tx, blob_price);
    let costs_valid : bool = match checked_costs {
        Some(_) => true,
        None() => false,
    };
    let costs : TransactionCosts = match checked_costs {
        Some(costs) => costs,
        None() => struct {
            intrinsic = GAS_COST_ZERO,
            calldata_floor = GAS_COST_ZERO,
            blob_gas = 0,
            blob_fee = ZERO_WORD,
            upfront = ZERO_WORD,
        },
    };
    let nonce_ok : bool = match word_to_account_nonce(tx.nonce) {
        Some(nonce) => nonce == nonce_before,
        None() => false,
    };

    /* Transaction validity: an invalid tx is REJECTED with no state change at all
       (no nonce bump, no gas charge). The upfront-balance test uses the fee CAPS
       (max_fee_per_gas, max_fee_per_blob_gas), not the effective prices. */
    let (sender_deleg, _) = k_deleg_target(sender);

    /* EIP-4844 / EIP-7691 / EIP-7594 blob-count bounds: Cancun max 6 per block,
       Prague max 9 per block, Osaka adds MAX_BLOBS_PER_TX = 6. A type-3 tx must
       carry >= 1 blob, must not create, and every versioned hash must be 0x01-tagged. */
    let max_blobs : blob_count = if k_fork >= Osaka then { 6 } else { if k_fork >= Prague then { 9 } else { 6 } };
    let blob_ok : bool = if tx_is_blob(tx.tx_type) then {
              (k_fork >= Cancun)
            & (tx.blob_hashes.count != 0)
            & (tx.blob_hashes.count <= max_blobs)
            & not_bool(tx.is_create)
            & blob_hashes_versioned(tx.blob_hashes)
        } else {
            true
        };

    /* EIP-7623 (Prague+): the calldata floor is ALSO a validity bound -- a tx whose
       gas limit cannot cover 21000 + 10*tokens is rejected outright. */
    let floor_ok : bool = (k_fork < Prague) | costs.calldata_floor <= tx.gas_limit;
    let chain_id_ok : bool = match tx.tx_type {
        LegacyTx => true,
        _ => tx.chain_id == k_chain_id,
    };
    let balance_ok : bool = word_ule(costs.upfront, k_get_balance(sender));
    let sender_code_ok : bool = (k_code_key(sender) == KECCAK_EMPTY) | sender_deleg;
    let intrinsic_ok : bool = costs.intrinsic <= tx.gas_limit;
    let fee_cap_ok : bool = word_ule(k_header.base_fee, tx.max_fee);
    let blob_fee_cap_ok : bool = (tx.blob_hashes.count == 0) | word_ule(blob_price, tx.max_blob_fee);
    let initcode_ok : bool = not_bool(tx.is_create) | initcode_size_allowed(input_len);
    let priority_fee_ok : bool = word_ule(tx.max_priority_fee, tx.max_fee);
    let access_list_fork_ok : bool = not_bool(tx_is_access_list(tx.tx_type)) | (k_fork >= Berlin);
    let dynamic_fee_fork_ok : bool = not_bool(tx_is_dynamic_fee(tx.tx_type)) | (k_fork >= London);
    let set_code_target_ok : bool = not_bool(tx_is_set_code(tx.tx_type)) | not_bool(tx.is_create);
    let set_code_authorizations_ok : bool = not_bool(tx_is_set_code(tx.tx_type)) | (tx.authorization_count != 0);
    let set_code_fork_ok : bool = not_bool(tx_is_set_code(tx.tx_type)) | (k_fork >= Prague);
    let nonce_incrementable : bool = nonce_before != 2 ^ 64 - 1;
    let block_gas_ok : bool = tx.gas_limit <= k_header.gas_limit;
    let valid =   costs_valid
                & balance_ok
                & sender_code_ok
                & intrinsic_ok
                & floor_ok
                & fee_cap_ok
                & blob_fee_cap_ok
                & blob_ok
                & initcode_ok
                & priority_fee_ok
                & access_list_fork_ok
                & dynamic_fee_fork_ok
                & set_code_target_ok
                & set_code_authorizations_ok
                & set_code_fork_ok
                & chain_id_ok
                & nonce_ok
                & nonce_incrementable
                & block_gas_ok;

    struct {
        valid = valid,
        sender = sender,
        nonce_before = nonce_before,
        intrinsic_gas = costs.intrinsic,
        calldata_floor = costs.calldata_floor,
        blob_fee = costs.blob_fee,
        gas_price = eff_gas_price,
        priority_fee = eff_priority_fee,
    }
}

function apply_transaction_upfront_effects(tx : Transaction, v : TxValidity) -> gas_refund = {
    /* upfront gas + nonce bump (persist across revert; taken before the snapshot) */
    k_sub_balance(v.sender, validated_word_product(v.gas_price, tx.gas_limit));

    /* EIP-4844: burn the blob fee upfront (blob_gas_used * blob_gas_price), no refund */
    if word_nonzero(v.blob_fee) then {
        k_sub_balance(v.sender, v.blob_fee)
    };
    k_bump_nonce(v.sender);
    prewarm(tx);
    process_auth_list(tx.authorizations) /* EIP-7702: set delegations before execution (persist) */
}

function enter_transaction_frame(tx : Transaction, intrinsic : gas_cost) -> unit = {
    pc = BYTE_ZERO;
    call_depth = 0;
    gas_remaining = validated_gas_sub(tx.gas_limit, intrinsic);
    stack_reset();
    memory_reset();
    returndata_clear();
    calldata = EMPTY_SLICE;
    frame_code = EMPTY_CODE;
    frame_refund = GAS_REFUND_ZERO;
    frame_status = Running()
}

function run_create_transaction_frame(tx : Transaction, sender : address, nonce_before : account_nonce) -> unit = {
    let new_addr = k_create_addr(sender, nonce_before);
    let _ = k_access_account(new_addr);

    /* create-tx address collision (code, nonzero nonce, or storage at the
     * target): the tx fails outright consuming ALL gas, no initcode runs
     * (EELS process_message_call -> AddressCollision, gas_left = 0). */
    if k_account_occupied(new_addr) then {
        exc_halt(AddressCollision)
    } else {
        k_mark_created(new_addr); /* EIP-6780: created this tx */
        k_clear_storage(new_addr);
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
                is_static = false,
                depth = 0,
            };
        frame_code = code_db_resolve(code_db_insert(tx.input_src));
        let deployed_code = interpret();
        if frame_succeeded() then {
            let dep_len = deployed_code.len;
            let code_ok =    dep_len
                          <= max_code_size()
                          &  ((dep_len == BYTE_ZERO) | (slice_byte(deployed_code, BYTE_ZERO) != 0xef)); /* EIP-3541 */
            if code_ok then {
                let deposit = G_codedeposit * dep_len;
                if deposit <= gas_remaining then {
                    gas_remaining = gas_sub_or_oog(gas_remaining, deposit);
                    k_deploy_code(new_addr, deployed_code)
                } else {
                    exc_halt(OutOfGas)
                }
            } else {
                exc_halt(OutOfGas)
            } /* failed deploy */
        }
    }
}

function run_call_transaction_frame(tx : Transaction, sender : address) -> unit = {
    /* EELS reads the recipient account during message setup (get_account for its
     * code) for every call-transaction, so it is always a state access -- keep it
     * in the account set (BAL), including a tx sent directly to a precompile. */
    let _ = k_aload(tx.recipient);
    if word_nonzero(tx.value) then {
        k_transfer(sender, tx.recipient, tx.value)
    };
    let precompile : option(precompile_id) = match word_to_precompile_id(address_to_word(tx.recipient)) {
        Some(number) if is_precompile(number) => Some(number),
        _ => None(),
    };
    match precompile {
        Some(number) => {
            /* tx directly to a precompile: run it as the top-level frame, gas-checked
               FIRST (an OOG precompile must not execute). Failure or OOG is an
               exceptional halt (all gas consumed, value transfer reverted). */
            match precompile_gas(number, tx.input_src) {
                Some(used) if used <= gas_remaining => {
                    let result = run_precompile_slice(number, tx.input_src);
                    if result.success then {
                        gas_remaining = gas_sub_or_oog(gas_remaining, used);
                        if is_running() then {
                            frame_status = Halted(HaltReturn(result.output))
                        }
                    } else {
                        exc_halt(OutOfGas)
                    }
                },
                _ => exc_halt(OutOfGas),
            }
        },
        None() => {
            calldata = tx.input_src;
            message =
                struct {
                    caller = sender,
                    address = tx.recipient,
                    code_address = tx.recipient,
                    value = tx.value,
                    is_static = false,
                    depth = 0,
                };

            /* EIP-7702: at the tx level the delegate is warmed but not separately charged */
            let (tx_deleg, tx_dtgt) = k_deleg_target(tx.recipient);
            if tx_deleg then {
                let _ = k_access_account(tx_dtgt);

                /* EELS reads get_account(code_address) on the resolved delegate, so
                   the delegate is touched (kept in the BAL account set). */
                let _ = k_aload(tx_dtgt);
                ()
            };
            frame_code = executable_code(tx.recipient, tx_deleg, tx_dtgt);
            let _ = interpret()
        },
    }
}

function run_transaction_frame(tx : Transaction, v : TxValidity) -> TxFrameResult = {
    let checkpoint = k_state_checkpoint();
    enter_transaction_frame(tx, v.intrinsic_gas);
    if tx.is_create then {
        run_create_transaction_frame(tx, v.sender, v.nonce_before)
    } else {
        run_call_transaction_frame(tx, v.sender)
    };

    let success = frame_succeeded();
    if not_bool(success) then {
        k_revert(checkpoint)
    };
    struct {
        success = success,
        gas_remaining = gas_remaining,
        refund = if success then { frame_refund } else { GAS_REFUND_ZERO },
    }
}

function settle_transaction(
    tx : Transaction,
    v : TxValidity,
    authorization_refund : gas_refund,
    fr : TxFrameResult,
) -> (
    Receipt
) = {
    /* Refund cap: gas_used / 2 before London, gas_used / 5 after EIP-3529. */
    let gas_left0 = fr.gas_remaining;
    let gas_used0 = validated_gas_sub(tx.gas_limit, gas_left0);
    let refund_quotient : gas_divisor = if k_fork >= London then { 5 } else { 2 };
    let refund_cap = gas_quotient(gas_used0, refund_quotient);
    let refund = capped_gas_refund(authorization_refund + fr.refund, refund_cap);
    let gas_left1 = validated_gas_add(gas_left0, refund);
    let gas_used1 = validated_gas_sub(tx.gas_limit, gas_left1);

    /* EIP-7623 (Prague+): a tx pays at least the calldata floor 21000 + 10*tokens */
    let floor : gas = if k_fork >= Prague then {
            let GasCost(floor_cost) = v.calldata_floor;
            let Gas(tx_limit) = tx.gas_limit;
            if floor_cost <= tx_limit then {
                Gas(floor_cost)
            } else {
                throw InvalidBlock(ExecutionInvalid)
            }
        } else {
            GAS_ZERO
        };
    let gas_used = if gas_used1 < floor then { floor } else { gas_used1 };
    let gas_left = validated_gas_sub(tx.gas_limit, gas_used);

    /* EIP-7778: the block charges the UNREFUNDED gas (floored), not the receipt's
       refunded gas -- this is what bounds the block gas limit and the header. */
    let block_gas = if k_fork >= Amsterdam then {
            if gas_used0 < floor then { floor } else { gas_used0 }
        } else {
            gas_used
        };

    /* return unused gas to sender; pay coinbase the priority fee */
    k_add_balance(v.sender, validated_word_product(v.gas_price, gas_left));
    k_add_balance(k_coinbase(), validated_word_product(v.priority_fee, gas_used));

    k_tx_merge();

    struct {
        tx_type = tx.tx_type,
        success = fr.success,
        valid = true,
        gas_used = gas_used,
        block_gas = block_gas,
        logs = read_logs(),
    }
}

function process_transaction(tx : Transaction) -> Receipt = {
    k_tx_reset();

    let validity = check_transaction_validity(tx);
    if not_bool(validity.valid) then {
        return invalid_receipt()
    };
    k_set_tx(struct { origin = tx.sender, gas_price = validity.gas_price, blob_hashes = tx.blob_hashes });
    let authorization_refund = apply_transaction_upfront_effects(tx, validity);
    let frame_result = run_transaction_frame(tx, validity);
    settle_transaction(tx, validity, authorization_refund, frame_result)
}
