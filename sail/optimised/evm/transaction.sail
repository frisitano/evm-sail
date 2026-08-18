/* Optimized C overrides for `sail/evm/transaction.sail`. */

/*
 * Successful transaction products are exact word-valued protocol quantities,
 * not modular EVM arithmetic. Keep the semantic helper but refine its
 * optimized implementation to test overflow entirely in the native word
 * domain.
 */
function validated_word_product(value, factor) = {
    if word_is_zero(value) | word_is_zero(factor) then {
        WORD_ZERO
    } else {
        let maximum_value = word_div(WORD_ALL_ONES, factor);
        let product_fits = word_ule(value, maximum_value);
        if product_fits then {
            word_mul(value, factor)
        } else {
            fatal_error(ExecutionInvalid)
        }
    }
}

/*
 * Transaction cost validation is another word-valued observation of the
 * exact blob fee. Evaluate the two admitted products and additions only after
 * their overflow guards, retaining the standard TransactionCosts boundary.
 */
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
    let blob_fee : word =
        if blob_gas == 0 then {
            WORD_ZERO
        } else {
            let blob_price = blob_base_fee(
                profile.fork,
                profile.blob_schedule,
                profile.excess_blob_gas_limit,
                excess_blob_gas,
            );
            if word_ule(blob_price, tx.max_blob_fee) then {
                validated_word_product(blob_price, blob_gas)
            } else {
                fatal_error(ExecutionInvalid)
            }
        };
    let execution_cap = validated_word_product(tx.max_fee, gas_limit);
    let blob_cap = validated_word_product(tx.max_blob_fee, blob_gas);
    let maximum_value = word_sub(WORD_ALL_ONES, execution_cap);
    let value_fits = word_ule(tx.value, maximum_value);
    let execution_and_value : word =
        if value_fits then word_add(execution_cap, tx.value) else fatal_error(ExecutionInvalid);
    let maximum_blob_cap = word_sub(WORD_ALL_ONES, execution_and_value);
    let blob_cap_fits = word_ule(blob_cap, maximum_blob_cap);
    let upfront : word =
        if blob_cap_fits then word_add(execution_and_value, blob_cap) else fatal_error(ExecutionInvalid);
    struct {
        intrinsic_execution = intrinsic.execution,
        intrinsic_state = intrinsic.state,
        calldata_floor = intrinsic.calldata_floor,
        blob_gas = blob_gas,
        blob_fee = blob_fee,
        upfront = upfront,
    }
}
