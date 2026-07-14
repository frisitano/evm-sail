/*md
# Validation debug (optional)

Default build variant discarding the optional validation debug
records.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

function validation_debug_reset() -> unit = ()

function validation_debug_record(_scope : bits(8), _reason : BlockError) -> unit = ()
