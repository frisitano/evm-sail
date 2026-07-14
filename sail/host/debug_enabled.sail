/*md
# Validation debug (enabled)

Build variant recording the failing validation stage and reason.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

register validation_failure_present : bool = false
register validation_failure_scope : bits(8) = SCOPE_STATELESS_VALIDATION
register validation_failure_reason : BlockError = InvalidConfig

function validation_debug_reset() -> unit = {
    validation_failure_present = false
}

function validation_debug_record(scope : bits(8), reason : BlockError) -> unit = {
    validation_failure_present = true;
    validation_failure_scope = scope;
    validation_failure_reason = reason
}
