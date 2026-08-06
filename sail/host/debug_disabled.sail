/*md
# Validation debug (optional)

Default build variant discarding the optional validation debug
records.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

/*! Clears the recorded failure at the start of a validation attempt. */
function validation_debug_reset() -> unit = ()

/*! Discards the optional block gas split in production builds. */
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
    _actual,
    _expected,
    _execution,
    _state,
) -> unit = ()

/*! Records the failing validation stage and reason for host
    inspection; a later record overwrites an earlier one. */
function validation_debug_record(_stage : validation_stage, _reason : FatalError) -> unit = ()

function debug_account_storage_root(_a : address) -> hash = EMPTY_TRIE_ROOT
