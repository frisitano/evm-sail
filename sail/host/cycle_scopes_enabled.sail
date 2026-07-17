/*md
# Cycle scopes (enabled)

Build variant binding the instrumentation scopes to host counters.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

val cycle_scope_start = impure { c: "evmsail_cycle_scope_start" } : bits(8) -> unit
val cycle_scope_end = impure { c: "evmsail_cycle_scope_end" } : bits(8) -> unit
