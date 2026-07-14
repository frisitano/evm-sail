/*md
# The operand stack

The per-frame EVM operand stack (YP §9.1, μ_s): up to 1024 words, LIFO.
The EVM layer ([push][] / [pop][] in the machine module) enforces the
depth bounds and charges nothing here — this is pure mechanism. Frames
form a stack of stacks: [stack_enter_frame][] on a sub-call,
[stack_leave_frame][] on return, [stack_reset][] per transaction. The
words live behind the host interface (`ffi/stack.c`) as per-frame flat
arrays for O(1) access;
proof targets see the `val`s as bodyless axioms over an abstract per-frame
list-of-words model.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

/*! Resets to a single empty operand stack (per-transaction reset). */
val stack_reset = impure { c: "stack_reset" } : unit -> unit
/*! Pushes a fresh, empty operand stack for a sub-call frame. */
val stack_enter_frame = impure { c: "stack_enter_frame" } : unit -> unit
/*! Pops the current frame's stack, restoring the caller's. */
val stack_leave_frame = impure { c: "stack_leave_frame" } : unit -> unit
/*! The number of words on the current frame's stack; the 1024-item
    bound is enforced by the EVM layer. */
val stack_depth = impure { c: "stack_depth" } : unit -> bits(64)
/*! Pushes a word (the depth bound is checked by the EVM layer). */
val stack_push_word = impure { c: "stack_push_word" } : word -> unit
/*! Pops and returns the top word (underflow is guarded by the EVM
    layer). */
val stack_pop_word = impure { c: "stack_pop_word" } : unit -> word /* pop AND return the top (caller guards underflow) */
/*! The word at depth `n` from the top, without popping (`DUPn`). */
val stack_peek_word = impure { c: "stack_peek_word" } : bits(64) -> word
/*! Overwrites the word at depth `n` from the top (`SWAPn`). */
val stack_set_word = impure { c: "stack_set_word" } : (bits(64), word) -> unit
