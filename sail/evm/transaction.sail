
/*md
# The transaction state transition

The per-transaction step of the Ethereum state transition (Yellow Paper
§6): validate the transaction, charge upfront gas, run it as the
top-level message call, then settle gas, refunds, and the coinbase fee.

Validity and intrinsic gas are user-space policy; every world effect is a
kernel syscall, and a transaction-level snapshot/commit/revert bounds the
whole transaction's world state. The typed-envelope rules covered here:
EIP-2718 (typed transactions), EIP-1559 (fee market: base fee + priority
tip caps), EIP-2930 (access lists), EIP-3860 (initcode size/gas),
EIP-4844 (blob transactions + blob-gas burn), EIP-7702 (set-code
authorizations), EIP-7623 (calldata floor), EIP-3607 (no transactions
from an account with code), EIP-7825 (gas cap). Gas refunds (`SSTORE`
clears) are capped per EIP-3529.

| Name | Value | Description |
| ---- | ----- | ----------- |
| `G_transaction` | `21000` | Per-transaction base cost |
| `G_txcreate` | `32000` | Create-transaction surcharge |
| `G_txdatazero` | `4` | Per zero calldata byte (EIP-2028) |
| `G_txdatanonzero` | `16` | Per nonzero calldata byte (EIP-2028) |
| `G_access_list_address` | `2400` | Per access-list address (EIP-2930) |
| `G_access_list_storage_key` | `1900` | Per access-list storage key (EIP-2930) |
| `PER_AUTH_BASE` | `12500` | Per authorization (EIP-7702) |
| `PER_EMPTY_ACCOUNT` | `25000` | Per authorization of a new account (EIP-7702) |
*/
let G_transaction : int(21000) = 21000
let G_txcreate : int(32000) = 32000
let G_txdatazero : int(4) = 4
let G_txdatanonzero : int(16) = 16
let G_access_list_address : int(2400) = 2400 /* EIP-2930 ACCESS_LIST_ADDRESS_COST     */
let G_access_list_storage_key : int(1900) = 1900 /* EIP-2930 ACCESS_LIST_STORAGE_KEY_COST */
let PER_AUTH_BASE : int(12500) = 12500 /* EIP-7702 PER_AUTH_BASE_COST          */
let PER_EMPTY_ACCOUNT : int(25000) = 25000 /* EIP-7702 PER_EMPTY_ACCOUNT_COST      */
/*! The refund available when one EIP-7702 authorization targets an existing
    account. */
type authorization_refund_per_item : Int = 12500
/*! The bounded refund contributed by one EIP-7702 authorization. */
type authorization_item_refund = range(0, authorization_refund_per_item)
/*! The aggregate EIP-7702 refund admitted by one transaction. */
type authorization_refund = range(
    0,
    authorization_refund_per_item * transaction_length_bound,
)
let AMSTERDAM_TX_BASE : int(12000) = 12000
let AMSTERDAM_CREATE_ACCESS : int(11000) = 11000
let AMSTERDAM_COLD_ACCOUNT_ACCESS : int(3000) = 3000
let AMSTERDAM_TX_VALUE_COST : int(4244) = 4244
let AMSTERDAM_TRANSFER_LOG_COST : int(1756) = 1756
let AMSTERDAM_ACCESS_LIST_ADDRESS : int(3000) = 3000
let AMSTERDAM_ACCESS_LIST_SLOT : int(3000) = 3000
let AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR : int(1280) = 1280
let AMSTERDAM_ACCESS_LIST_SLOT_FLOOR : int(2048) = 2048
let AMSTERDAM_AUTH_BASE : int(7816) = 7816
let AMSTERDAM_CALLDATA_FLOOR_BYTE : int(64) = 64

/*! Intrinsic transaction charges split into Amsterdam execution gas, state
    gas, and the calldata floor. */
struct IntrinsicGasCost = {
    execution : gas_cost,
    state : gas_cost,
    calldata_floor : gas_cost,
}

/*! The intrinsic, blob, and upfront costs established during validation. */
struct TransactionCosts = {
    intrinsic_execution : gas_cost,
    intrinsic_state : gas_cost,
    calldata_floor : gas_cost,
    blob_gas : transaction_blob_gas,
    blob_fee : word,
    upfront : word,
}

/*! Reclassifies transaction initcode as executable after re-establishing the
    enclosing SSZ transaction-envelope bound. This bound is structural and is
    deliberately independent of the active protocol deployment limit. */
function transaction_initcode_slice(input : TransactionInputSlice) -> CodeSlice = {
    let input_slice = stateless_input_slice(input.bytes, input.len);
    code_db_intern_input(input_slice)
}

/*! Values established before entering the top-level transaction frame. */
struct TxUpfrontResult = {
    authorization_refund : authorization_refund,
    create_target_prestate_empty : bool,
}

/*! The EIP-2028 calldata cost: 4 gas per zero byte, 16 per nonzero. One
    native pass counts the nonzero bytes; zero bytes are the
    remainder. */
function calldata_cost(input : TransactionInputSlice) -> transaction_calldata_cost = {
    let nonzeroes = slice_count_nonzero(input);
    let input_len = input.len;
    if nonzeroes <= input_len then {
        let zeroes = input_len - nonzeroes;
        G_txdatazero * zeroes + G_txdatanonzero * nonzeroes
    } else {
        fatal_error(ExecutionInvalid)
    }
}

/*! The intrinsic gas of a transaction (YP §6.2, g_0): the 21000 base,
    calldata cost, access-list cost (EIP-2930), authorization cost
    (EIP-7702), and for creates the `G_txcreate` base plus EIP-3860
    initcode words. */
function legacy_intrinsic_gas(tx : Transaction) -> gas_cost = {
    let data_cost = calldata_cost(tx.input_src);
    let input = tx.input_src;
    let input_len = input.len;
    let address_cost = G_access_list_address * tx.access_list.address_count;
    let slot_cost = G_access_list_storage_key * tx.access_list.slot_count;
    let authorizations = tx.authorizations;
    let auth_cost = PER_EMPTY_ACCOUNT * authorizations.count;
    let common = data_cost + G_transaction + address_cost + slot_cost + auth_cost;
    if tx.is_create then {
        common + G_txcreate + transaction_initcode_gas(input_len)
    } else {
        common
    }
}

/*! Computes the pre-Amsterdam EIP-7623 calldata floor cost. */
function legacy_calldata_floor(input : TransactionInputSlice) -> transaction_calldata_floor_cost = {
    let nonzeroes = slice_count_nonzero(input);
    let input_len = input.len;
    if nonzeroes <= input_len then {
        let zeroes = input_len - nonzeroes;
        10 * zeroes + 40 * nonzeroes + G_transaction
    } else {
        fatal_error(ExecutionInvalid)
    }
}

/*! The maximum Amsterdam recipient-side intrinsic execution charge. */
type amsterdam_recipient_cost = range(0, 12756)

/*! Computes Amsterdam execution-gas charges for recipient access, value
    transfer, and contract creation. */
function amsterdam_recipient_execution_cost(tx : Transaction) -> amsterdam_recipient_cost = {
    let transfers_value = word_nonzero(tx.value);
    if tx.is_create then {
        if transfers_value then {
            AMSTERDAM_CREATE_ACCESS + AMSTERDAM_TRANSFER_LOG_COST
        } else {
            AMSTERDAM_CREATE_ACCESS
        }
    } else if tx.recipient != tx.sender then {
        if transfers_value then {
            AMSTERDAM_COLD_ACCOUNT_ACCESS + AMSTERDAM_TX_VALUE_COST + AMSTERDAM_TRANSFER_LOG_COST
        } else {
            AMSTERDAM_COLD_ACCOUNT_ACCESS
        }
    } else {
        0
    }
}

/*! Computes the fork-specific execution/state intrinsic costs and calldata
    floor. Amsterdam decomposes the transaction charge into the two gas
    dimensions introduced by EIP-2780. */
function intrinsic_gas(tx : Transaction) -> IntrinsicGasCost = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if profile.fork < Amsterdam then {
        struct { execution = legacy_intrinsic_gas(tx), state = 0, calldata_floor = legacy_calldata_floor(tx.input_src) }
    } else {
        let input = tx.input_src;
        let recipient = amsterdam_recipient_execution_cost(tx);
        let address_count = tx.access_list.address_count;
        let slot_count = tx.access_list.slot_count;
        let access_execution =   AMSTERDAM_ACCESS_LIST_ADDRESS
                               * address_count
                               + AMSTERDAM_ACCESS_LIST_SLOT
                               * slot_count
                               + AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR
                               * address_count
                               + AMSTERDAM_ACCESS_LIST_SLOT_FLOOR
                               * slot_count;
        let authorizations = tx.authorizations;
        let authorization_execution = AMSTERDAM_AUTH_BASE * authorizations.count;
        let create_execution =
            if tx.is_create then transaction_initcode_gas(input.len) else 0;
        let execution =   calldata_cost(tx.input_src)
                        + AMSTERDAM_TX_BASE
                        + recipient
                        + access_execution
                        + authorization_execution
                        + create_execution;
        let input_length = input.len;
        let floor =   AMSTERDAM_CALLDATA_FLOOR_BYTE
                    * input_length
                    + AMSTERDAM_TX_BASE
                    + recipient
                    + AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR
                    * address_count
                    + AMSTERDAM_ACCESS_LIST_SLOT_FLOOR
                    * slot_count;
        struct { execution = execution, state = 0, calldata_floor = floor }
    }
}

/*! Exact blob-fee product. The result is not independently capped: its type
    is the formula over the exact blob price and the transaction's bounded
    blob-gas quantity. */
val transaction_blob_fee : forall 'price 'blob_gas,
                                0
                             <= 'price
                             &  'price
                             <  2 ^ 256
                             &  0
                             <= 'blob_gas
                             &  'blob_gas
                             <= prague_blob_max_count
                             *  gas_per_blob_value. (int('price), int('blob_gas)) -> int('price * 'blob_gas)
function transaction_blob_fee(blob_price, blob_gas) =
    blob_price * blob_gas

/*! Exact maximum upfront debit required by transaction validation. Every
    upper bound comes from the participating protocol quantity; the result is
    narrowed only after the formula has been compared with the word boundary. */
val transaction_upfront_cost : forall 'max_fee 'gas_limit 'value 'max_blob_fee 'blob_gas,
                                    0
                                 <= 'max_fee
                                 &  'max_fee
                                 <  2 ^ 256
                                 &  0
                                 <= 'gas_limit
                                 &  'gas_limit
                                 <= block_gas_limit_bound
                                 &  0
                                 <= 'value
                                 &  'value
                                 <  2 ^ 256
                                 &  0
                                 <= 'max_blob_fee
                                 &  'max_blob_fee
                                 <  2 ^ 256
                                 &  0
                                 <= 'blob_gas
                                 &  'blob_gas
                                 <= prague_blob_max_count
                                 *  gas_per_blob_value. (
        int('max_fee),
        int('gas_limit),
        int('value),
        int('max_blob_fee),
        int('blob_gas),
    ) -> int('max_fee * 'gas_limit + 'value + 'max_blob_fee * 'blob_gas)
function transaction_upfront_cost(max_fee, gas_limit, value, max_blob_fee, blob_gas) =
    max_fee * gas_limit + value + max_blob_fee * blob_gas

/*! Computes transaction costs as mathematical naturals, narrowing only the
    externally observable word-valued results. */
function transaction_costs(
    profile : ProtocolProfile,
    tx : Transaction,
    gas_limit : block_gas_limit,
    excess_blob_gas : excess_blob_gas,
) -> (
    TransactionCosts
) = {
    let intrinsic = intrinsic_gas(tx);
    let blob_gas : transaction_blob_gas = sizeof(gas_per_blob_value) * tx.blob_hashes.count;
    let blob_fee_value : nat =
        if blob_gas == 0 then {
            0
        } else {
            let blob_price = blob_base_fee(
                profile.fork,
                profile.blob_schedule,
                profile.excess_blob_gas_limit,
                excess_blob_gas,
            );
            if blob_price <= tx.max_blob_fee then {
                transaction_blob_fee(blob_price, blob_gas)
            } else {
                fatal_error(ExecutionInvalid)
            }
        };
    let upfront_value = transaction_upfront_cost(tx.max_fee, gas_limit, tx.value, tx.max_blob_fee, blob_gas);
    if (blob_fee_value < sizeof(word_modulus)) & (upfront_value < sizeof(word_modulus)) then {
        struct {
            intrinsic_execution = intrinsic.execution,
            intrinsic_state = intrinsic.state,
            calldata_floor = intrinsic.calldata_floor,
            blob_gas = blob_gas,
            blob_fee = protocol_word(blob_fee_value),
            upfront = protocol_word(upfront_value),
        }
    } else {
        fatal_error(ExecutionInvalid)
    }
}

/* These arithmetic relations were established by transaction validation or
 * an immediately preceding guard. Rechecking at the use site keeps the
 * boundary total and turns an internal invariant violation into an invalid
 * block instead of silently substituting a value. */
/*! Reifies a word product whose validity was established by transaction checks. */
val validated_word_product : forall 'factor, 0 <= 'factor & 'factor < 2 ^ 256. (word, int('factor)) -> word
function validated_word_product(value, factor) = {
    let product = value * factor;
    if product < sizeof(word_modulus) then {
        protocol_word(product)
    } else {
        fatal_error(ExecutionInvalid)
    }
}

/*! Captures the two live top-level gas reservoirs at the frame boundary and
    retains their consumed execution/state split. Mutable frame registers
    cannot themselves carry a dependent conservation relation, so this is the
    single boundary that re-establishes it for settlement and block
    accumulation. */
val tx_frame_gas_snapshot : forall 'limit 'regular 'intrinsic_execution 'intrinsic_state 'calldata_floor 'initial_execution 'initial_state 'execution 'state,
                                transaction_initial_gas_relation(
                                    'limit,
                                    'regular,
                                    'intrinsic_execution,
                                    'intrinsic_state,
                                    'calldata_floor,
                                    'initial_execution,
                                    'initial_state,
                                )
                              & live_gas_valid('execution)
                              & live_gas_valid('state). (
        TransactionInitialGasFields(
            'limit,
            'regular,
            'intrinsic_execution,
            'intrinsic_state,
            'calldata_floor,
            'initial_execution,
            'initial_state,
        ),
        int('execution),
        int('state),
        state_gas_delta,
    ) -> TxFrameGasSnapshotForLimits('limit, 'regular)

function tx_frame_gas_snapshot(initial, execution, state, state_delta) = {
    let limit = initial.admitted_limit;
    let regular = initial.regular_limit;
    if execution <= limit then {
        let room = limit - execution;
        if state <= room then {
            let (remaining as 'remaining) = execution + state;
            let spent = limit - remaining;
            let raw_state_used : transaction_state_gas_delta = initial.intrinsic_state + state_delta;
            if raw_state_used <= 0 then {
                if spent <= regular then {
                    tx_frame_gas_snapshot_fields(limit, regular, initial.calldata_floor, remaining, 0)
                } else {
                    fatal_error(ExecutionInvalid)
                }
            } else {
                let positive_state_used : transaction_state_gas_used = raw_state_used;
                if positive_state_used <= spent then {
                    /* The comparison is the semantic boundary that narrows the
                       signed transaction delta back into the admitted reservoir.
                       Name that refinement explicitly so optimized lowering can
                       convert directly to the concrete gas width instead of first
                       materializing the wider positive range of the signed sum. */
                    let (bounded_state_used as 'bounded_state_used) : range(0, 'limit - 'remaining) = positive_state_used;
                    if spent - bounded_state_used <= regular then {
                        tx_frame_gas_snapshot_fields(
                            limit,
                            regular,
                            initial.calldata_floor,
                            remaining,
                            bounded_state_used,
                        )
                    } else {
                        fatal_error(ExecutionInvalid)
                    }
                } else {
                    fatal_error(ExecutionInvalid)
                }
            }
        } else {
            fatal_error(ExecutionInvalid)
        }
    } else {
        fatal_error(ExecutionInvalid)
    }
}

/*! Constructs the proof-carrying allowance once admission has established
    that the decoded total fits the execution profile. */
val transaction_gas_allowance_fields : forall 'value 'total_limit 'regular_limit,
                                            0
                                         <= 'value
                                         &  0
                                         <= 'regular_limit
                                         &  'regular_limit
                                         <= 'total_limit
                                         &  'value
                                         <= 'total_limit
                                         &  'total_limit
                                         <= block_gas_limit_bound. (int('value), int('total_limit), int('regular_limit)) -> TransactionGasAllowanceFields(
        'value,
        if 'value < 'regular_limit then 'value else 'regular_limit,
    )
function transaction_gas_allowance_fields(value, _total_limit, regular_limit) = {
    let regular =
        if value < regular_limit then value else regular_limit;
    struct { total = value, regular = regular }
}

/*! Couples a decoded transaction gas value to the total and regular limits
    derived once from the current execution profile. Transaction gas is not a
    finite enumeration: these are relational bounds established after the
    header-dependent admission check. */
val transaction_gas_allowance : forall 'value 'total_limit 'regular_limit,
                                     0
                                  <= 'value
                                  &  0
                                  <= 'regular_limit
                                  &  'regular_limit
                                  <= 'total_limit
                                  &  'total_limit
                                  <= block_gas_limit_bound. (int('value), int('total_limit), int('regular_limit)) -> TransactionGasAllowance
function transaction_gas_allowance(value, total_limit, regular_limit) =
    if total_limit < value then {
        fatal_error(ExecutionInvalid)
    } else {
        transaction_gas_allowance_fields(value, total_limit, regular_limit)
    }

/*! Splits the post-intrinsic transaction reservoir into ordinary execution
    gas and Amsterdam state gas. The same equation covers earlier forks:
    their profile makes the regular allowance equal the total allowance, so
    the state reservoir is exactly zero. */
val transaction_initial_gas : forall 'total 'regular 'intrinsic_execution 'intrinsic_state 'calldata_floor,
                                   0
                                <= 'regular
                                &  'regular
                                <= 'total
                                &  'total
                                <= block_gas_limit_bound
                                &  0
                                <= 'intrinsic_execution
                                &  0
                                <= 'intrinsic_state
                                &  0
                                <= 'calldata_floor. (
        TransactionGasAllowanceFields('total, 'regular),
        int('intrinsic_execution),
        int('intrinsic_state),
        int('calldata_floor),
    ) -> TransactionInitialGasForLimits('total, 'regular)
function transaction_initial_gas(allowance, intrinsic_execution, intrinsic_state, calldata_floor) = {
    if allowance.total < intrinsic_execution then {
        fatal_error(ExecutionInvalid)
    } else {
        let after_execution = allowance.total - intrinsic_execution;
        if after_execution < intrinsic_state then {
            fatal_error(ExecutionInvalid)
        } else if (allowance.regular < intrinsic_execution) | (allowance.regular < calldata_floor) then {
            fatal_error(ExecutionInvalid)
        } else {
            let available = after_execution - intrinsic_state;
            let regular_budget = allowance.regular - intrinsic_execution;
            if available < regular_budget then {
                transaction_initial_gas_fields(
                    allowance.total,
                    allowance.regular,
                    intrinsic_execution,
                    intrinsic_state,
                    calldata_floor,
                    available,
                    0,
                )
            } else {
                transaction_initial_gas_fields(
                    allowance.total,
                    allowance.regular,
                    intrinsic_execution,
                    intrinsic_state,
                    calldata_floor,
                    regular_budget,
                    available - regular_budget,
                )
            }
        }
    }
}

/*! Applies one EIP-7702 authorization: validates it against current
    state, sets or clears the delegation, bumps the authority nonce, and
    refunds if the authority already existed. The signature and chain id
    are validated from the tuple alone before the authority's account is
    read — a tuple rejected there touches no state, so its authority need
    not be witnessed; every authority-state read is gated on those
    checks. The authority is warmed before the code/nonce
    checks, so a tuple later skipped still warms it. */
function process_auth(au : Authorization) -> authorization_item_refund = {
    var refund : authorization_item_refund = 0;
    let authority = au.authority;

    /* Sig + chain id are validated from the tuple ALONE before reading the
     * authority's account; a tuple rejected there (e.g. wrong chain id) touches
     * NO state, so its authority need not be witnessed -- reading it eagerly would
     * over-access the witness. Gate every authority-state read on signature and
     * chain-id acceptance. */
    let chain_id_is_zero = word_is_zero(au.chain_id);
    let expected_chain_id = word_of_chain_identifier(k_chain_id);
    let chain_id_matches = au.chain_id == expected_chain_id;
    if au.valid_sig & (chain_id_is_zero | chain_id_matches) then {
        /* step 5: warm the authority BEFORE the code/nonce checks -- a tuple later
         * skipped for code/nonce still warms its authority. */
        k_account_mark_warm(authority);
        let (is_deleg, _) = k_deleg_target(authority);
        let code_key = k_code_key(authority);
        let nonce = k_get_nonce(authority);
        if ((code_key == KECCAK_EMPTY) | is_deleg) & (nonce == au.nonce) then {
            let existed = k_account_exists(authority);
            if au.address == ZERO_ADDRESS then {
                k_clear_code(authority)
                /* clear delegation */
            } else {
                k_set_delegation(authority, au.address)
            };
            k_bump_nonce(authority);
            if existed then {
                refund = PER_EMPTY_ACCOUNT - PER_AUTH_BASE
            }
        }
    };
    refund
}

/*! Adds one authorization refund to the transaction-wide accumulator. The
    decoded authorization count proves this guard unreachable in valid input;
    spelling it at the narrowing boundary keeps proof extraction independent
    of the Coq backend's treatment of existential range indices. */
function authorization_refund_add(
    item : authorization_item_refund,
    accumulated : authorization_refund,
) -> (
    authorization_refund
) = {
    let bound = sizeof(authorization_refund_per_item) * sizeof(transaction_length_bound);
    if accumulated <= bound - item then {
        item + accumulated
    } else {
        fatal_error(ExecutionInvalid)
    }
}

/*! Applies prepared authorizations in wire order. */
val process_auth_cursor : (PreparedAuthorizationList, prepared_authorization_count) -> authorization_refund
function process_auth_cursor(authorizations, count) =
    if count == 0 then {
        0
    } else {
        let authorization = prepared_authorization_head(authorizations);
        let remaining = prepared_authorization_tail(authorizations, count);
        let item_refund = process_auth(authorization);
        let remaining_refund = process_auth_cursor(remaining, count - 1);
        authorization_refund_add(item_refund, remaining_refund)
    }
termination_measure process_auth_cursor(authorizations, count) = count

/*! Applies a prepared authorization collection in order. */
function process_auth_list(authorizations : PreparedAuthorizationList) -> authorization_refund =
    process_auth_cursor(authorizations, authorizations.count)

/*! Applies one Amsterdam authorization and charges its state-dependent
    execution-gas and state-gas components. Tuple-local signature and chain
    checks precede all authority-state reads; a valid tuple warms its authority
    before checking code and nonce, as required by EIP-7702. */
function process_amsterdam_auth(
    au : Authorization,
    sender : address,
    current_target : address,
    transfers_value : bool,
    carried_gas : gas,
    carried_state_gas : state_gas,
    carried_state_spill : state_gas_spill,
) -> (
    (bool, gas, state_gas, state_gas_spill)
) = {
    var gas_after : gas = carried_gas;
    var state_gas_after : state_gas = carried_state_gas;
    var state_spill_after : state_gas_spill = carried_state_spill;
    let authority = au.authority;

    let chain_id_is_zero = word_is_zero(au.chain_id);
    let expected_chain_id = word_of_chain_identifier(k_chain_id);
    let chain_id_matches = au.chain_id == expected_chain_id;
    if au.valid_sig & (chain_id_is_zero | chain_id_matches) then {
        k_account_mark_warm(authority);
        let (currently_delegated, _) = k_deleg_target(authority);

        let code_key = k_code_key(authority);
        let nonce = k_get_nonce(authority);
        if ((code_key == KECCAK_EMPTY) | currently_delegated) & (nonce == au.nonce) then {
            let seen = authorization_tracker_seen(authority);
            let delegated_before_tx =
                if seen then authorization_tracker_originally_delegated(authority) else currently_delegated;
            let already_written = seen | (authority == sender) | (transfers_value & (authority == current_target));

            let account_exists = k_account_exists(authority);
            let account_missing = not_bool(account_exists);
            if account_missing then {
                let (state_gas_halt, next_gas, next_state_gas, next_state_spill) = charge_state_gas(
                    gas_after,
                    state_gas_after,
                    state_spill_after,
                    G_amsterdam_state_new_account,
                );
                gas_after = next_gas;
                state_gas_after = next_state_gas;
                state_spill_after = next_state_spill;
                if state_gas_halt then {
                    return (false, gas_after, state_gas_after, state_spill_after)
                }
            };
            let requires_account_write = not_bool(already_written);
            if requires_account_write then {
                if gas_after < G_amsterdam_account_write then {
                    return (false, GAS_ZERO, state_gas_after, state_spill_after)
                };
                gas_after = gas_sub(gas_after, G_amsterdam_account_write)
            };
            let not_delegated_before_tx = not_bool(delegated_before_tx);
            let delegation_set = authorization_tracker_delegation_set(authority);
            let delegation_not_set = not_bool(delegation_set);
            let creates_delegation = au.address != ZERO_ADDRESS;
            if creates_delegation & not_delegated_before_tx & delegation_not_set then {
                let (auth_state_gas_halt, auth_gas, auth_state_gas, auth_state_spill) = charge_state_gas(
                    gas_after,
                    state_gas_after,
                    state_spill_after,
                    G_amsterdam_state_auth_base,
                );
                gas_after = auth_gas;
                state_gas_after = auth_state_gas;
                state_spill_after = auth_state_spill;
                if auth_state_gas_halt then {
                    return (false, gas_after, state_gas_after, state_spill_after)
                }
            };

            if au.address == ZERO_ADDRESS then {
                k_clear_code(authority)
            } else {
                k_set_delegation(authority, au.address)
            };
            k_bump_nonce(authority);
            let unseen = not_bool(seen);
            let originally_delegated = unseen & currently_delegated;
            authorization_tracker_commit(authority, originally_delegated, creates_delegation)
        }
    };
    (true, gas_after, state_gas_after, state_spill_after)
}

/*! Applies Amsterdam authorizations in wire order, stopping immediately when
    one of their execution-gas or state-gas charges exhausts the transaction. */
val process_amsterdam_auth_cursor : (
        PreparedAuthorizationList,
        prepared_authorization_count,
        address,
        address,
        bool,
        gas,
        state_gas,
        state_gas_spill,
    ) -> (bool, gas, state_gas, state_gas_spill)
function process_amsterdam_auth_cursor(
    authorizations,
    count,
    sender,
    current_target,
    transfers_value,
    gas,
    state_gas,
    state_spill,
) =
    if count == 0 then {
        (true, gas, state_gas, state_spill)
    } else {
        let authorization = prepared_authorization_head(authorizations);
        let remaining = prepared_authorization_tail(authorizations, count);
        let (processed, gas_after, state_gas_after, state_spill_after) = process_amsterdam_auth(
            authorization,
            sender,
            current_target,
            transfers_value,
            gas,
            state_gas,
            state_spill,
        );
        if processed then {
            process_amsterdam_auth_cursor(
                remaining,
                count - 1,
                sender,
                current_target,
                transfers_value,
                gas_after,
                state_gas_after,
                state_spill_after,
            )
        } else {
            (false, gas_after, state_gas_after, state_spill_after)
        }
    }
termination_measure process_amsterdam_auth_cursor(authorizations, count, sender, current_target, transfers_value, gas, state_gas, state_spill) = count

/*! Warms the storage keys of one source-backed access-list entry. */
val warm_access_list_keys : forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
        RlpCursor('source_off, 'source_len),
        address,
    ) -> unit
function warm_access_list_keys(cursor, addr) = {
    if cursor.len == 0 then {
        return ()
    };
    let key = rlp_decode_item(cursor);
    let next = rlp_cursor_advance(cursor, key.source.len);
    let slot = rlp_decode_word(key);
    k_prewarm_slot(addr, slot);
    warm_access_list_keys(next, addr)
}
termination_measure warm_access_list_keys(cursor, addr) = cursor.len

/*! Warms every account and storage key in a source-backed access list. */
val warm_access_list : forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). RlpCursor(
        'source_off,
        'source_len,
    ) -> unit
function warm_access_list(cursor) = {
    if cursor.len == 0 then {
        return ()
    };
    let entry = rlp_decode_item(cursor);
    let next = rlp_cursor_advance(cursor, entry.source.len);
    let fields = rlp_decode_list(entry);
    let addr_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, addr_f.source.len);
    let keys_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, keys_f.source.len);
    rlp_cursor_expect_end(fields);
    let addr_word = rlp_decode_word(addr_f);
    let addr = word_to_address(addr_word);
    k_account_mark_warm(addr);
    let keys = rlp_decode_list(keys_f);
    warm_access_list_keys(keys, addr);
    warm_access_list(next)
}
termination_measure warm_access_list(cursor) = cursor.len

/*! Pre-warms the accessed-address set (EIP-2929): the sender, the call
    target, and the access list (EIP-2930); EIP-3651 additionally warms the
    coinbase from Shanghai onward. Active precompiles are a fork-derived warm
    class in [k_account_is_warm][] and therefore need no table entries. */
function prewarm(tx : Transaction) -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    k_account_mark_warm(tx.sender);
    if tx.is_create then {
        ()
    } else {
        k_account_mark_warm(tx.recipient)
    };
    if profile.fork >= Shanghai then {
        let coinbase = k_coinbase();
        k_account_mark_warm(coinbase)
    };

    let access_list : StatelessInputSlice = tx.access_list.encoded;
    warm_access_list(access_list)
}

/*! The EIP-1559 effective fee: the gas price actually paid is
    `min(max_fee, base_fee + max_priority_fee)`, and the priority tip
    paid to the coinbase is that price minus the base fee. Legacy and
    EIP-2930 transactions carry a single `gas_price`, passed as
    `max_fee = max_priority = gas_price`, so this recovers
    `(gas_price, gas_price − base_fee)`. The priority is clamped at 0 so
    an invalid sub-base-fee price (rejected later by validity) never
    underflows. */
function eff_gas_price_for(base_fee : word, max_fee : word, max_priority_fee : word) -> (word, word) = {
    let max_fee_below_base = word_ule(max_fee, base_fee);
    let price : word =
        if max_fee_below_base then {
            max_fee
        } else {
            let available_priority = word_sub(max_fee, base_fee);
            let priority_within_cap = word_ule(max_priority_fee, available_priority);
            if priority_within_cap then {
                word_add(base_fee, max_priority_fee)
            } else {
                max_fee
            }
        };
    let base_fee_covered = word_ule(base_fee, price);
    let priority =
        if base_fee_covered then word_sub(price, base_fee) else ZERO_WORD;
    (price, priority)
}

/*! Transaction validity (YP §6.2). First
    authenticates the witnessed public key against the signature over the
    signing hash — a forged key or bad `v`/`r`/`s` makes the whole block
    invalid regardless of the verdict — then applies the per-envelope
    validity rules (nonce, balance, intrinsic gas, fee caps, blob rules,
    EIP-3607, EIP-7825) and derives the effective prices. */
val check_transaction_validity : forall 'total 'regular,
                                   0 <= 'regular & 'regular <= 'total & 'total <= block_gas_limit_bound. (
        Transaction,
        TransactionGasAllowanceFields('total, 'regular),
    ) -> TxValidityForLimits('total, 'regular)
function check_transaction_validity(tx, allowance) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let tx_semantics = tx_type_semantics(tx.tx_type);

    /* Sender authentication: `v` selects the recovered signer, which must be
       the address derived from the witnessed public key. A forged key or bad
       v/r/s makes the whole BLOCK invalid, regardless of the validity verdict
       below. */
    let parity = tx_signature_parity(k_chain_id, tx_semantics.signature, tx.sig_v);
    let authenticated = tx_auth_valid(tx.sender, tx.signing_hash, parity, tx.sig_r, tx.sig_s);
    let invalid_signature = not_bool(authenticated);
    if invalid_signature then {
        fatal_error(InvalidSignature)
    };
    let gas_limit = allowance.total;

    /* effective gas price (EIP-1559) */
    let (eff_gas_price, eff_priority_fee) = eff_gas_price_for(k_header.base_fee, tx.max_fee, tx.max_priority_fee);
    let sender = tx.sender;
    let input = tx.input_src;
    let input_len = input.len;
    let nonce_before = k_get_nonce(sender);
    let costs = transaction_costs(profile, tx, gas_limit, k_header.excess_blob_gas);
    let expected_nonce = word_of_account_nonce(nonce_before);
    if tx.nonce != expected_nonce then {
        fatal_error(ExecutionInvalid)
    };

    /* Transaction validity: an invalid tx is REJECTED with no state change at all
       (no nonce bump, no gas charge). The upfront-balance test uses the fee CAPS
       (max_fee_per_gas, max_fee_per_blob_gas), not the effective prices. */
    let (sender_deleg, _) = k_deleg_target(sender);

    /* EIP-4844 / EIP-7691 / EIP-7594: the profile-indexed RLP decoder has
       already bounded the count and validated each `0x01` version byte while
       consuming the hash-list cursor. A type-3 transaction must still carry
       at least one blob and must not create a contract. */
    if tx_semantics.blob then {
        if (profile.fork < Cancun) | (tx.blob_hashes.count == 0) | tx.is_create then {
            fatal_error(ExecutionInvalid)
        }
    };

    /* EIP-7623 (Prague+): the calldata floor is ALSO a validity bound -- a tx whose
       gas limit cannot cover 21000 + 10*tokens is rejected outright. */
    if (profile.fork >= Prague) & (gas_limit < costs.calldata_floor) then {
        fatal_error(ExecutionInvalid)
    };
    let sender_balance = k_get_balance(sender);
    let upfront_affordable = word_ule(costs.upfront, sender_balance);
    let insufficient_balance = not_bool(upfront_affordable);
    if insufficient_balance then {
        fatal_error(ExecutionInvalid)
    };
    let sender_code_key = k_code_key(sender);
    let valid_sender_code = (sender_code_key == KECCAK_EMPTY) | sender_deleg;
    let invalid_sender_code = not_bool(valid_sender_code);
    if invalid_sender_code then {
        fatal_error(ExecutionInvalid)
    };
    if allowance.regular < costs.calldata_floor then {
        fatal_error(ExecutionInvalid)
    };
    let base_fee_affordable = word_ule(k_header.base_fee, tx.max_fee);
    let base_fee_exceeds_cap = not_bool(base_fee_affordable);
    if base_fee_exceeds_cap then {
        fatal_error(ExecutionInvalid)
    };
    let valid_initcode_size = initcode_size_allowed(input_len);
    let invalid_initcode_size = not_bool(valid_initcode_size);
    if tx.is_create & invalid_initcode_size then {
        fatal_error(ExecutionInvalid)
    };
    let valid_priority_fee = word_ule(tx.max_priority_fee, tx.max_fee);
    let invalid_priority_fee = not_bool(valid_priority_fee);
    if invalid_priority_fee then {
        fatal_error(ExecutionInvalid)
    };
    if profile.fork < tx_semantics.minimum_fork then {
        fatal_error(ExecutionInvalid)
    };
    if tx_semantics.set_code & tx.is_create then {
        fatal_error(ExecutionInvalid)
    };
    let authorizations = tx.authorizations;
    if tx_semantics.set_code & (authorizations.count == 0) then {
        fatal_error(ExecutionInvalid)
    };
    let typed_signature = tx_semantics.signature == TypedSignature;
    if typed_signature & (tx.chain_id != k_chain_id) then {
        fatal_error(ExecutionInvalid)
    };
    if nonce_before == sizeof(account_nonce_bound) then {
        fatal_error(ExecutionInvalid)
    };
    let initial_gas = transaction_initial_gas(
        allowance,
        costs.intrinsic_execution,
        costs.intrinsic_state,
        costs.calldata_floor,
    );
    tx_validity_fields(sender, nonce_before, initial_gas, costs.blob_fee, eff_gas_price, eff_priority_fee)
}

/*! The upfront effects, taken before the execution snapshot so they persist
    across a dispatched-frame revert: charge the full execution/state gas
    limit and the EIP-4844 blob-gas fee, bump the sender nonce, and prewarm the
    transaction access set. Before Amsterdam, EIP-7702 authorizations are also
    applied here; Amsterdam applies them inside the separately reversible
    top-frame preparation phase. */
function apply_transaction_upfront_effects(
    tx : Transaction,
    v : TxValidity,
    authorizations : PreparedAuthorizationList,
) -> (
    TxUpfrontResult
) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let initial_gas = v.gas;
    let create_target_prestate_empty : bool =
        if (profile.fork >= Amsterdam) & tx.is_create then {
            let create_target = k_create_addr(v.sender, v.nonce_before);
            let target_exists = k_account_exists(create_target);
            not_bool(target_exists)
        } else {
            false
        };

    /* upfront gas + nonce bump (persist across revert; taken before the snapshot) */
    let gas_debit = validated_word_product(v.gas_price, initial_gas.admitted_limit);
    k_sub_balance(v.sender, gas_debit);

    /* EIP-4844: burn the blob fee upfront (blob_gas_used * blob_gas_price), no refund */
    let has_blob_fee = word_nonzero(v.blob_fee);
    if has_blob_fee then {
        k_sub_balance(v.sender, v.blob_fee)
    };
    k_bump_nonce(v.sender);
    prewarm(tx);
    let authorization_refund =
        if profile.fork < Amsterdam then process_auth_list(authorizations) else 0;
    struct { authorization_refund = authorization_refund, create_target_prestate_empty = create_target_prestate_empty }
}

/*! Resets the user-space machine for the transaction's top-level frame,
    funding it with `gas_limit − intrinsic`. */
function enter_transaction_frame(v : TxValidity) -> (
    (gas, state_gas, state_gas_spill, gas_refund, StackPointer, memory_base, memory_height)
) = {
    let initial_gas = v.gas;
    let stack = stack_reset();
    (
        initial_gas.execution_remaining,
        initial_gas.state_remaining,
        STATE_GAS_SPILL_ZERO,
        GAS_REFUND_ZERO,
        stack,
        MEMORY_BASE_ZERO,
        MEMORY_HEIGHT_ZERO,
    )
}

/*! The outcome of top-level dispatch preparation: whether the frame is
    ready to run, and whether a call recipient delegated, which disables
    direct precompile dispatch. */
struct TransactionPreparation = {
    ready : bool,
    delegated : bool,
}

/*! Charges Amsterdam's state-dependent top-level dispatch costs and installs
    the code selected for execution. This phase deliberately performs no
    revertible account mutation: its state-gas charges are therefore refilled
    if the subsequently dispatched frame fails. */
function prepare_amsterdam_transaction_dispatch(
    tx : Transaction,
    v : TxValidity,
    upfront : TxUpfrontResult,
    carried_gas : gas,
    carried_state_gas : state_gas,
    carried_state_spill : state_gas_spill,
) -> (
    (TransactionPreparation, gas, state_gas, state_gas_spill, address, address, Code, CalldataSlice)
) = {
    var gas_after : gas = carried_gas;
    var state_gas_after : state_gas = carried_state_gas;
    var state_spill_after : state_gas_spill = carried_state_spill;
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let current_target =
        if tx.is_create then k_create_addr(v.sender, v.nonce_before) else tx.recipient;

    if tx.is_create then {
        if upfront.create_target_prestate_empty then {
            let (state_gas_halt, next_gas, next_state_gas, next_state_spill) = charge_state_gas(
                gas_after,
                state_gas_after,
                state_spill_after,
                G_amsterdam_state_new_account,
            );
            gas_after = next_gas;
            state_gas_after = next_state_gas;
            state_spill_after = next_state_spill;
            if state_gas_halt then {
                return (
                    struct { ready = false, delegated = false },
                    gas_after,
                    state_gas_after,
                    state_spill_after,
                    current_target,
                    current_target,
                    EMPTY_CODE,
                    EMPTY_CALLDATA,
                )
            }
        };
        let initcode = transaction_initcode_slice(tx.input_src);
        let code_id = code_db_insert(initcode, profile.fork);
        let code = code_db_resolve(code_id);
        (
            struct { ready = true, delegated = false },
            gas_after,
            state_gas_after,
            state_spill_after,
            current_target,
            current_target,
            code,
            EMPTY_CALLDATA,
        )
    } else {
        let calldata = InputCalldata(tx.input_src);
        let transfers_value = word_nonzero(tx.value);
        let recipient_empty = k_account_is_empty(tx.recipient);
        if transfers_value & recipient_empty then {
            let (state_gas_halt, next_gas, next_state_gas, next_state_spill) = charge_state_gas(
                gas_after,
                state_gas_after,
                state_spill_after,
                G_amsterdam_state_new_account,
            );
            gas_after = next_gas;
            state_gas_after = next_state_gas;
            state_spill_after = next_state_spill;
            if state_gas_halt then {
                return (
                    struct { ready = false, delegated = false },
                    gas_after,
                    state_gas_after,
                    state_spill_after,
                    current_target,
                    current_target,
                    EMPTY_CODE,
                    calldata,
                )
            }
        };

        let (delegated, delegate) = k_deleg_target(tx.recipient);
        if delegated then {
            let warm = k_account_is_warm(delegate);
            let access_cost = account_cost(warm);
            if gas_after < access_cost then {
                return (
                    struct { ready = false, delegated = false },
                    GAS_ZERO,
                    state_gas_after,
                    state_spill_after,
                    current_target,
                    current_target,
                    EMPTY_CODE,
                    calldata,
                )
            };
            gas_after = gas_sub(gas_after, access_cost);
            k_account_mark_warm(delegate)
        };
        let code_address =
            if delegated then delegate else current_target;
        let code = executable_code(tx.recipient, delegated, delegate);
        (
            struct { ready = true, delegated = delegated },
            gas_after,
            state_gas_after,
            state_spill_after,
            current_target,
            code_address,
            code,
            calldata,
        )
    }
}

/*! Runs a create transaction's top-level frame: derives the new address
    from `(sender, nonce_before)`, fails outright on an address collision
    (all gas consumed, no initcode runs — EIP-684/EIP-7610), and
    otherwise deploys via the initcode path. */
function run_create_transaction_frame(
    tx : Transaction,
    sender : address,
    nonce_before : account_nonce,
    carried_gas : gas,
    carried_state_gas : state_gas,
    carried_state_spill : state_gas_spill,
    carried_refund : gas_refund,
    carried_stack : StackPointer,
    carried_memory_base : memory_base,
    carried_memory_height : memory_height,
    carried_code : Code,
    carried_calldata : CalldataSlice,
    state_gas_reservoir : state_gas,
) -> (
    (gas, state_gas, state_gas_spill, gas_refund, FrameStatus, OutputSlice)
) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let new_addr = k_create_addr(sender, nonce_before);
    var gas_after : gas = carried_gas;
    var state_gas_after : state_gas = carried_state_gas;
    var state_spill_after : state_gas_spill = carried_state_spill;
    var refund_after : gas_refund = carried_refund;
    var status_after : FrameStatus = Running();
    var output_after : OutputSlice = EMPTY_OUTPUT_SLICE;
    k_account_mark_warm(new_addr);

    /* create-tx address collision (code, nonzero nonce, or storage at the
     * target): the tx fails outright consuming ALL gas, no initcode runs
     * (EIP-684/EIP-7610; gas_left = 0). */
    let occupied = k_account_occupied(new_addr);
    if occupied then {
        gas_after = GAS_ZERO;
        let exceptional = exceptional_state(state_gas_after, state_spill_after, state_gas_reservoir, AddressCollision);
        state_gas_after = exceptional.state_gas_remaining;
        state_spill_after = exceptional.state_gas_spilled;
        status_after = exceptional.status
    } else {
        k_mark_created(new_addr); /* EIP-6780: created this tx */
        k_clear_storage(new_addr);
        k_bump_nonce(new_addr);
        let transfers_value = word_nonzero(tx.value);
        if transfers_value then {
            k_transfer(sender, new_addr, tx.value)
        };
        var frame_code : Code = carried_code;
        var frame_calldata : CalldataSlice = carried_calldata;
        if profile.fork < Amsterdam then {
            let initcode = transaction_initcode_slice(tx.input_src);
            let code_id = code_db_insert(initcode, profile.fork);
            frame_code = code_db_resolve(code_id);
            frame_calldata = EMPTY_CALLDATA
        };
        (gas_after, state_gas_after, state_spill_after, refund_after, status_after, output_after) = interpret(
            gas_after,
            state_gas_after,
            state_spill_after,
            refund_after,
            carried_stack,
            carried_memory_base,
            carried_memory_height,
            sender,
            new_addr,
            new_addr,
            tx.value,
            state_gas_reservoir,
            false,
            0,
            frame_code,
            frame_calldata,
        );
        let initcode_succeeded = frame_succeeded(status_after);
        if initcode_succeeded then {
            let deployed_output = output_after;
            let dep_len = deployed_output.len;
            let deployed_length = dep_len;
            let valid_deployed_size = deployed_code_size_allowed(deployed_length);
            let valid_prefix =
                if (profile.fork < London) | (deployed_length == 0) then {
                    true
                } else {
                    let first_byte = slice_byte(deployed_output, 0);
                    first_byte != 0xef
                };
            if valid_deployed_size & valid_prefix then {
                let deployment_charge = code_deployment_execution_cost(dep_len, gas_after);
                if deployment_charge.affordable then {
                    let execution_deposit = deployment_charge.cost;
                    gas_after = gas_sub(gas_after, execution_deposit);
                    let state_deposit = code_deployment_state_cost(dep_len);
                    var deployment_halt : bool = false;
                    (deployment_halt, gas_after, state_gas_after, state_spill_after) = charge_state_gas(
                        gas_after,
                        state_gas_after,
                        state_spill_after,
                        state_deposit,
                    );
                    if deployment_halt then {
                        gas_after = GAS_ZERO;
                        let exceptional = exceptional_state(
                            state_gas_after,
                            state_spill_after,
                            state_gas_reservoir,
                            OutOfGas,
                        );
                        state_gas_after = exceptional.state_gas_remaining;
                        state_spill_after = exceptional.state_gas_spilled;
                        status_after = exceptional.status
                    };
                    let deployment_succeeded = frame_succeeded(status_after);
                    if deployment_succeeded then {
                        let stored_code = code_db_intern_output(deployed_output);
                        k_deploy_code(new_addr, stored_code)
                    }
                } else if profile.fork < Homestead then {
                    /* Frontier consumes the remaining gas and keeps the
                     * created account with empty code. */
                    gas_after = GAS_ZERO;
                    k_deploy_code(new_addr, EMPTY_CODE_SLICE)
                } else {
                    gas_after = GAS_ZERO;
                    let exceptional = exceptional_state(
                        state_gas_after,
                        state_spill_after,
                        state_gas_reservoir,
                        OutOfGas,
                    );
                    state_gas_after = exceptional.state_gas_remaining;
                    state_spill_after = exceptional.state_gas_spilled;
                    status_after = exceptional.status
                }
            } else {
                gas_after = GAS_ZERO;
                let exceptional = exceptional_state(state_gas_after, state_spill_after, state_gas_reservoir, OutOfGas);
                state_gas_after = exceptional.state_gas_remaining;
                state_spill_after = exceptional.state_gas_spilled;
                status_after = exceptional.status
            } /* failed deploy */
        }
    };
    (gas_after, state_gas_after, state_spill_after, refund_after, status_after, output_after)
}

/*! Runs a call transaction's top-level frame: transfers value, then either
    runs a direct recipient precompile or interprets the selected code. At
    Amsterdam the preparation phase has already resolved and charged a
    recipient delegation; a delegated recipient never dispatches a precompile
    directly. */
function run_call_transaction_frame(
    tx : Transaction,
    sender : address,
    delegated : bool,
    carried_gas : gas,
    carried_state_gas : state_gas,
    carried_state_spill : state_gas_spill,
    carried_refund : gas_refund,
    carried_stack : StackPointer,
    carried_memory_base : memory_base,
    carried_memory_height : memory_height,
    carried_code_address : address,
    carried_code : Code,
    carried_calldata : CalldataSlice,
    state_gas_reservoir : state_gas,
) -> (
    (gas, state_gas, state_gas_spill, gas_refund, FrameStatus, OutputSlice)
) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    var gas_after : gas = carried_gas;
    var state_gas_after : state_gas = carried_state_gas;
    var state_spill_after : state_gas_spill = carried_state_spill;
    var refund_after : gas_refund = carried_refund;
    var status_after : FrameStatus = Running();
    var output_after : OutputSlice = EMPTY_OUTPUT_SLICE;
    var code_address : address = carried_code_address;
    var frame_code : Code = carried_code;
    var frame_calldata : CalldataSlice = carried_calldata;

    /* The recipient account is read during message setup (for its code) on
     * every call-transaction, so it is always a state access -- keep it in the
    * account set (BAL), including a tx sent directly to a precompile. */
    let _ = k_aload(tx.recipient);
    let transfers_value = word_nonzero(tx.value);
    if transfers_value then {
        k_transfer(sender, tx.recipient, tx.value)
    };
    let selected_precompile = precompile_id_for_address(tx.recipient);
    let direct_recipient = not_bool(delegated);
    if direct_recipient & (selected_precompile != NotPrecompile) then {
        /* tx directly to a precompile: run it as the top-level frame, gas-checked
           FIRST (an OOG precompile must not execute). Failure or OOG is an
           exceptional halt (all gas consumed, value transfer reverted). */
        let input_src : StatelessInputSlice = tx.input_src;
        let precompile_input = InputCalldata(input_src);
        let precompile_charge = precompile_gas(selected_precompile, precompile_input, gas_after);
        if precompile_charge.affordable then {
            let used = precompile_charge.cost;
            let result = run_precompile_slice(selected_precompile, precompile_input);
            if result.success then {
                gas_after = gas_sub(gas_after, used);
                output_after = result.output;
                let halt_reason = HaltReturn(result.output);
                status_after = Halted(halt_reason)
            } else {
                gas_after = GAS_ZERO;
                let exceptional = exceptional_state(state_gas_after, state_spill_after, state_gas_reservoir, OutOfGas);
                state_gas_after = exceptional.state_gas_remaining;
                state_spill_after = exceptional.state_gas_spilled;
                status_after = exceptional.status
            }
        } else {
            gas_after = GAS_ZERO;
            let exceptional = exceptional_state(state_gas_after, state_spill_after, state_gas_reservoir, OutOfGas);
            state_gas_after = exceptional.state_gas_remaining;
            state_spill_after = exceptional.state_gas_spilled;
            status_after = exceptional.status
        }
    } else {
        if profile.fork < Amsterdam then {
            frame_calldata = InputCalldata(tx.input_src);
            code_address = tx.recipient;

            /* EIP-7702: before Amsterdam the transaction-level delegate is
               warmed but has no separate access charge. */
            let (tx_deleg, tx_dtgt) = k_deleg_target(tx.recipient);
            if tx_deleg then {
                k_account_mark_warm(tx_dtgt);
                let _ = k_aload(tx_dtgt);
                ()
            };
            if tx_deleg then {
                code_address = tx_dtgt
            };
            frame_code = executable_code(tx.recipient, tx_deleg, tx_dtgt)
        };
        (gas_after, state_gas_after, state_spill_after, refund_after, status_after, output_after) = interpret(
            gas_after,
            state_gas_after,
            state_spill_after,
            refund_after,
            carried_stack,
            carried_memory_base,
            carried_memory_height,
            sender,
            tx.recipient,
            code_address,
            tx.value,
            state_gas_reservoir,
            false,
            0,
            frame_code,
            frame_calldata,
        )
    };
    (gas_after, state_gas_after, state_spill_after, refund_after, status_after, output_after)
}

/*! Runs a pre-Amsterdam top-level frame under one execution snapshot. */
val run_legacy_transaction_frame : forall 'limit 'regular,
                                     0 <= 'regular & 'regular <= 'limit & 'limit <= block_gas_limit_bound. (
        Transaction,
        TxValidityForLimits('limit, 'regular),
    ) -> TxFrameResultFields('limit, 'regular)
function run_legacy_transaction_frame(tx, v) = {
    let initial_gas = v.gas;
    k_journal_checkpoint();
    let (
        initial_execution_gas,
        initial_state_gas,
        initial_state_spill,
        initial_refund,
        initial_stack,
        initial_memory_base,
        initial_memory_height,
    ) = enter_transaction_frame(v);
    let state_gas_reservoir = initial_state_gas;
    let (gas_after, state_gas_after, state_spill_after, refund_after, status_after, _) =
        if tx.is_create
        then run_create_transaction_frame(
            tx,
            v.sender,
            v.nonce_before,
            initial_execution_gas,
            initial_state_gas,
            initial_state_spill,
            initial_refund,
            initial_stack,
            initial_memory_base,
            initial_memory_height,
            EMPTY_CODE,
            EMPTY_CALLDATA,
            state_gas_reservoir,
        )
        else run_call_transaction_frame(
            tx,
            v.sender,
            false,
            initial_execution_gas,
            initial_state_gas,
            initial_state_spill,
            initial_refund,
            initial_stack,
            initial_memory_base,
            initial_memory_height,
            tx.recipient,
            EMPTY_CODE,
            EMPTY_CALLDATA,
            state_gas_reservoir,
        );

    let success = frame_succeeded(status_after);
    let failed = not_bool(success);
    if failed then {
        k_journal_revert()
    } else {
        k_journal_commit()
    };
    let state_delta = frame_state_gas_used(state_gas_reservoir, state_gas_after, state_spill_after);
    let retained_refund =
        if success then refund_after else GAS_REFUND_ZERO;
    struct {
        success = success,
        gas = tx_frame_gas_snapshot(initial_gas, gas_after, state_gas_after, state_delta),
        refund = retained_refund,
    }
}

/*! Runs an Amsterdam top-level frame with two rollback boundaries. The
    preparation checkpoint contains EIP-7702 authorization writes and is
    reverted only when preparation itself runs out of gas. Once preparation
    succeeds, the execution checkpoint lets a failed call/create revert while
    retaining valid authorization writes and their state-gas charge. */
val run_amsterdam_transaction_frame : forall 'limit 'regular,
                                        0 <= 'regular & 'regular <= 'limit & 'limit <= block_gas_limit_bound. (
        Transaction,
        TxValidityForLimits('limit, 'regular),
        TxUpfrontResult,
        PreparedAuthorizationList,
    ) -> TxFrameResultFields('limit, 'regular)
function run_amsterdam_transaction_frame(tx, v, upfront, authorizations) = {
    let (
        entered_gas,
        entered_state_gas,
        entered_state_spill,
        entered_refund,
        entered_stack,
        entered_memory_base,
        entered_memory,
    ) = enter_transaction_frame(v);
    var gas_after : gas = entered_gas;
    var state_gas_after : state_gas = entered_state_gas;
    var state_spill_after : state_gas_spill = entered_state_spill;
    var refund_after : gas_refund = entered_refund;
    var status_after : FrameStatus = Running();
    var output_after : OutputSlice = EMPTY_OUTPUT_SLICE;
    let initial_gas = v.gas;
    k_journal_checkpoint();
    let preparation_reservoir = state_gas_after;
    let current_target =
        if tx.is_create then k_create_addr(v.sender, v.nonce_before) else tx.recipient;

    authorization_tracker_reset(authorizations.count);
    let transfers_value = word_nonzero(tx.value);
    var preparation_ready : bool = false;
    (preparation_ready, gas_after, state_gas_after, state_spill_after) = process_amsterdam_auth_cursor(
        authorizations,
        authorizations.count,
        v.sender,
        current_target,
        transfers_value,
        gas_after,
        state_gas_after,
        state_spill_after,
    );

    var authorization_state_gas : frame_state_gas_delta = FRAME_STATE_GAS_DELTA_ZERO;
    var delegated : bool = false;
    var execution_reservoir : state_gas = state_gas_after;
    var prepared_code_address : address = current_target;
    var prepared_code : Code = EMPTY_CODE;
    var prepared_calldata : CalldataSlice = EMPTY_CALLDATA;
    if preparation_ready then {
        authorization_state_gas = frame_state_gas_used(preparation_reservoir, state_gas_after, state_spill_after);
        execution_reservoir = state_gas_after;
        state_spill_after = STATE_GAS_SPILL_ZERO;
        let (preparation, prepared_gas, prepared_state_gas, prepared_state_spill, _, code_address, code, calldata) = prepare_amsterdam_transaction_dispatch(
            tx,
            v,
            upfront,
            gas_after,
            state_gas_after,
            state_spill_after,
        );
        gas_after = prepared_gas;
        state_gas_after = prepared_state_gas;
        state_spill_after = prepared_state_spill;
        preparation_ready = preparation.ready;
        delegated = preparation.delegated;
        prepared_code_address = code_address;
        prepared_code = code;
        prepared_calldata = calldata
    };

    let preparation_failed = not_bool(preparation_ready);
    if preparation_failed then {
        k_journal_revert();
        state_gas_after = preparation_reservoir;
        state_spill_after = STATE_GAS_SPILL_ZERO;
        return struct {
            success = false,
            gas = tx_frame_gas_snapshot(initial_gas, GAS_ZERO, STATE_GAS_ZERO, FRAME_STATE_GAS_DELTA_ZERO),
            refund = GAS_REFUND_ZERO,
        }
    };

    k_journal_checkpoint();
    if tx.is_create then {
        (gas_after, state_gas_after, state_spill_after, refund_after, status_after, output_after) = run_create_transaction_frame(
            tx,
            v.sender,
            v.nonce_before,
            gas_after,
            state_gas_after,
            state_spill_after,
            refund_after,
            entered_stack,
            entered_memory_base,
            entered_memory,
            prepared_code,
            prepared_calldata,
            execution_reservoir,
        )
    } else {
        (gas_after, state_gas_after, state_spill_after, refund_after, status_after, output_after) = run_call_transaction_frame(
            tx,
            v.sender,
            delegated,
            gas_after,
            state_gas_after,
            state_spill_after,
            refund_after,
            entered_stack,
            entered_memory_base,
            entered_memory,
            prepared_code_address,
            prepared_code,
            prepared_calldata,
            execution_reservoir,
        )
    };

    let success = frame_succeeded(status_after);
    let failed = not_bool(success);
    if failed then {
        k_journal_revert()
    } else {
        k_journal_commit()
    };

    /* Authorization writes live in the preparation scope and survive an EVM
       execution failure once preparation itself has completed. */
    k_journal_commit();
    let state_delta = authorization_state_gas +
            frame_state_gas_used(execution_reservoir, state_gas_after, state_spill_after);
    let retained_refund =
        if success then refund_after else GAS_REFUND_ZERO;
    struct {
        success = success,
        gas = tx_frame_gas_snapshot(initial_gas, gas_after, state_gas_after, state_delta),
        refund = retained_refund,
    }
}

/*! Runs the fork-appropriate top-level transaction frame. */
val run_transaction_frame : forall 'limit 'regular,
                              0 <= 'regular & 'regular <= 'limit & 'limit <= block_gas_limit_bound. (
        Transaction,
        TxValidityForLimits('limit, 'regular),
        TxUpfrontResult,
        PreparedAuthorizationList,
    ) -> TxFrameResultFields('limit, 'regular)
function run_transaction_frame(tx, v, upfront, authorizations) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if profile.fork >= Amsterdam then {
        run_amsterdam_transaction_frame(tx, v, upfront, authorizations)
    } else {
        run_legacy_transaction_frame(tx, v)
    }
}

/*! Applies the capped refund to a bounded frame-gas snapshot. Retaining the
    admitted limit in the result type makes the post-refund conservation proof
    available to settlement and to bounded-integer specialization. */
val remaining_gas_after_refund : forall 'limit 'total 'remaining 'cap,
                                      -gas_refund_bound
                                   <= 'total
                                   &  'total
                                   <= gas_refund_bound
                                   +  authorization_refund_per_item
                                   *  transaction_length_bound
                                   &  0
                                   <= 'remaining
                                   &  'remaining
                                   <= 'limit
                                   &  'limit
                                   <= block_gas_limit_bound
                                   &  0
                                   <= 'cap
                                   &  'cap
                                   <= 'limit
                                   -  'remaining. (int('limit), int('total), int('remaining), int('cap)) -> {'result,
                                                                                                                'remaining
                                                                                                             <= 'result
                                                                                                             &  'result
                                                                                                             <= 'limit.
                                                                                                             int(
                                                                                                                 'result,
                                                                                                             )}

function remaining_gas_after_refund(_limit, total, remaining, cap) = {
    let refund =
        if total <= 0 then 0 else if total <= cap then total else cap;
    remaining + refund
}

/*! Settlement (YP §6.2 g*, A_r): applies the capped refund
    (`gas_used/5` from London, `gas_used/2` before), the EIP-7623
    calldata floor (Prague+), and the EIP-7778 block-gas rule (Amsterdam+:
    the block charges the unrefunded, floored gas); returns unused gas to
    the sender, pays the coinbase the priority fee, merges the
    transaction into the block layer, and emits the receipt. */
val settle_transaction : forall 'limit 'regular, 0 <= 'regular & 'regular <= 'limit & 'limit <= block_gas_limit_bound. (
        Transaction,
        TxValidityForLimits('limit, 'regular),
        authorization_refund,
        TxFrameResultFields('limit, 'regular),
    ) -> ReceiptWithin('limit, 'regular)
function settle_transaction(tx, v, authorization_refund, fr) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;

    /* Refund cap: gas_used / 2 before London, gas_used / 5 after EIP-3529. */
    let gas_snapshot = fr.gas;
    let gas_limit = gas_snapshot.admitted_limit;
    let gas_left = gas_snapshot.remaining;
    let gas_used0 = gas_limit - gas_left;
    let refund_quotient = profile.refund_divisor;
    let refund_cap = gas_used0 / refund_quotient;
    let total_refund = authorization_refund + fr.refund;
    let gas_left = remaining_gas_after_refund(gas_limit, total_refund, gas_left, refund_cap);
    let gas_used1 = gas_limit - gas_left;

    /* EIP-7623 (Prague+): a tx pays at least the calldata floor 21000 + 10*tokens */
    let floor =
        if profile.fork >= Prague then gas_snapshot.calldata_floor else 0;
    let gas_used : range(0, 'limit) =
        if gas_used1 < floor then floor else gas_used1;
    let gas_left = gas_limit - gas_used;

    let tx_state_gas = gas_snapshot.state_used;
    let unrefunded_execution_gas : range(0, 'regular) = gas_limit - gas_snapshot.remaining - tx_state_gas;

    /* Retain the unrefunded regular-gas contribution for Amsterdam block
       accounting. Earlier forks accumulate `gas_used` instead; keeping this
       auxiliary field under the regular allowance in every receipt preserves
       one uniform dependent type. */
    let execution_gas : range(0, 'regular) =
        if unrefunded_execution_gas < floor then floor else unrefunded_execution_gas;

    /* This auxiliary field retains the state-reservoir contribution even on
       earlier profiles, where it is necessarily zero. Keeping the conserved
       split in the receipt type proves that its cumulative gas can never
       exceed the two block reservoirs. */
    let state_gas : range(0, 'limit) = tx_state_gas;

    /* return unused gas to sender; pay coinbase the priority fee */
    let sender_refund = validated_word_product(v.gas_price, gas_left);
    k_add_balance(v.sender, sender_refund);
    let coinbase = k_coinbase();
    let priority_payment = validated_word_product(v.priority_fee, gas_used);
    k_add_balance(coinbase, priority_payment);

    k_tx_merge();

    let logs = read_logs();
    let (gas_used_value as 'gas_used) = gas_used;
    let (execution_gas_value as 'execution_gas) = execution_gas;
    let (state_gas_value as 'state_gas) = state_gas;
    if gas_used_value <= execution_gas_value + state_gas_value then {
        receipt_within(
            gas_limit,
            gas_snapshot.regular_limit,
            tx.tx_type,
            fr.success,
            gas_used_value,
            execution_gas_value,
            state_gas_value,
            logs,
        )
    } else {
        /* A violation would mean the frame snapshot failed to conserve the
           two admitted gas reservoirs. This is the one transaction boundary
           that validates that internal invariant; block and receipt
           accumulation consume its dependent proof without rechecking it. */
        fatal_error(ExecutionInvalid)
    }
}

/*! The complete per-transaction step: reset, validate (an invalid
    transaction throws before state changes), apply upfront effects, run the
    frame, and settle. */
val process_transaction : forall 'total 'regular, 0 <= 'regular & 'regular <= 'total & 'total <= block_gas_limit_bound. (
        Transaction,
        TransactionGasAllowanceFields('total, 'regular),
    ) -> ReceiptWithin('total, 'regular)
function process_transaction(tx, allowance) = {
    k_tx_reset();
    let validity = check_transaction_validity(tx, allowance);
    let authorizations = prepare_authorizations(tx.authorizations);
    let environment = tx_env(tx.sender, validity.gas_price, tx.blob_hashes);
    k_set_tx(environment);
    let upfront = apply_transaction_upfront_effects(tx, validity, authorizations);
    let frame_result = run_transaction_frame(tx, validity, upfront, authorizations);
    let receipt = settle_transaction(tx, validity, upfront.authorization_refund, frame_result);
    receipt
}
