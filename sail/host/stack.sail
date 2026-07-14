/*md
# The operand stack

The per-frame EVM operand stack (YP §9.1, μ_s): up to 1024 words, LIFO.
The EVM layer ([push][] / [pop][] in the machine module) enforces the
depth bounds and charges nothing here — this is pure mechanism. Frames
form a stack of stacks: [stack_enter_frame][] on a sub-call,
[stack_leave_frame][] on return, [stack_reset][] per transaction. The
words live in C (`ffi/stack.c`) as per-frame flat arrays for O(1) access;
proof targets see the `val`s as bodyless axioms over an abstract per-frame
list-of-words model.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

val stack_reset = impure { c: "stack_reset" } : unit -> unit
val stack_enter_frame = impure { c: "stack_enter_frame" } : unit -> unit
val stack_leave_frame = impure { c: "stack_leave_frame" } : unit -> unit
val stack_depth = impure { c: "stack_depth" } : unit -> bits(64)
val stack_push_word = impure { c: "stack_push_word" } : word -> unit
val stack_pop_word = impure { c: "stack_pop_word" } : unit -> word /* pop AND return the top (caller guards underflow) */
val stack_peek_word = impure { c: "stack_peek_word" } : bits(64) -> word
val stack_set_word = impure { c: "stack_set_word" } : (bits(64), word) -> unit
