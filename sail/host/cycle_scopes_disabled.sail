/*md
# Cycle scopes (disabled)

Build variant stubbing the instrumentation scopes.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

function cycle_scope_start(_scope : bits(8)) -> unit = ()
function cycle_scope_end(_scope : bits(8)) -> unit = ()
