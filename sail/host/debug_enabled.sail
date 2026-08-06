/*md
# Validation debug (enabled)

Build variant recording the failing validation stage and reason.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

/*! Whether a validation failure has been recorded for the current
    block. */
register validation_failure_present : bool = false
/*! The validation stage that was active when the failure was recorded. */
register validation_failure_scope : validation_stage = 0
/*! The block-validation error recorded by the last failure. */
register validation_failure_reason : FatalError = InvalidConfig
/*! Whether the latest completed block execution recorded its gas totals. */
register validation_debug_gas_present : bool = false
/*! Header gas computed from the execution and state reservoirs. */
register validation_debug_header_gas_actual : gas = GAS_ZERO
/*! Header gas committed by the payload being validated. */
register validation_debug_header_gas_expected : gas = GAS_ZERO
/*! Cumulative regular execution gas consumed by the block. */
register validation_debug_execution_gas : gas = GAS_ZERO
/*! Cumulative Amsterdam state gas consumed by the block. */
register validation_debug_state_gas : gas = GAS_ZERO

/*! Clears the recorded failure at the start of a validation attempt. */
function validation_debug_reset() -> unit = {
    validation_failure_present = false;
    validation_debug_gas_present = false
}

/*! Retains the block gas split for native failure diagnostics. */
val validation_debug_capture_block_gas : forall 'actual 'expected 'execution 'state,
       0 <= 'actual
    &  'actual <= block_gas_limit_bound
    &  0 <= 'expected
    &  'expected <= block_gas_limit_bound
    &  0 <= 'execution
    &  'execution <= block_gas_limit_bound
    &  0 <= 'state
    &  'state <= block_gas_limit_bound.
    (int('actual), int('expected), int('execution), int('state)) -> unit
function validation_debug_capture_block_gas(
    actual,
    expected,
    execution,
    state,
) -> unit = {
    validation_debug_gas_present = true;
    validation_debug_header_gas_actual = actual;
    validation_debug_header_gas_expected = expected;
    validation_debug_execution_gas = execution;
    validation_debug_state_gas = state
}

/*! Records the failing validation stage and reason for host
    inspection; a later record overwrites an earlier one. */
function validation_debug_record(stage : validation_stage, reason : FatalError) -> unit = {
    validation_failure_present = true;
    validation_failure_scope = stage;
    validation_failure_reason = reason
}

/*! Rebuilds the storage root represented by the cached storage of `a`. */
function debug_account_storage_root(a : address) -> hash = {
    storage_block_iter_begin(a);
    let account = acct_block_get(a);
    if account.found then {
        let account = account.account;
        trie_root(
            if account.storage_cleared then {
                EMPTY_TRIE_ROOT
            } else {
                account.info.storage_root
            },
            StorageTrieUpdates(a),
        ).root
    } else {
        EMPTY_TRIE_ROOT
    }
}
