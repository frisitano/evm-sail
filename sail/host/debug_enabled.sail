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
/*! The instrumentation scope that was active when the failure was
    recorded. */
register validation_failure_scope : bits(8) = SCOPE_STATELESS_VALIDATION
/*! The block-validation error recorded by the last failure. */
register validation_failure_reason : BlockError = InvalidConfig

/*! Clears the recorded failure at the start of a validation attempt. */
function validation_debug_reset() -> unit = {
    validation_failure_present = false
}

/*! Records the failing validation stage and reason for host
    inspection; a later record overwrites an earlier one. */
function validation_debug_record(scope : bits(8), reason : BlockError) -> unit = {
    validation_failure_present = true;
    validation_failure_scope = scope;
    validation_failure_reason = reason
}
