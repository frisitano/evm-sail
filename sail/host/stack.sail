
/*md
# The operand stack

The per-frame EVM operand stack (YP §9.1, μ_s): up to 1024 words, LIFO.
The words live behind the host interface (the corresponding `stack.c` in
`ffi/spec/` or `ffi/optimized/`) as per-frame flat arrays for O(1) access;
proof targets see the `val`s as bodyless axioms over an abstract per-frame
list-of-words model.

The active frame's position is the abstract cursor token
[StackTop][type-StackTop], threaded by value through the interpreter and
every opcode handler in the state-passing convention and held in the
`stack_top` frame register at frame boundaries. This module keeps
ownership of the word storage and its workspace binding — only the cursor
changes custody. Frames form a stack of stacks:
[operand_stack_push_empty_frame][] on a sub-call returns the child's
empty cursor, [operand_stack_pop_frame][] releases it on return (the
parent's cursor is restored from its frame checkpoint), and
[stack_reset][] returns the base frame's empty cursor per transaction.

The EVM layer ([push_word][] / [pop][] in the machine module) enforces the
depth bounds through [validate_stack][] before a handler body runs and
charges nothing here — this is pure mechanism, so the slot and cursor
axioms perform no re-checks.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/
/*! Resets to a single empty operand stack (per-transaction reset) and
    returns the base frame's empty cursor. */
val stack_reset = impure { c: "stack_reset" } : unit -> StackTop
/*! Pushes a fresh, empty operand stack for a sub-call frame and returns
    its empty cursor. */
val operand_stack_push_empty_frame = impure { c: "operand_stack_push_empty_frame" } : unit -> StackTop
/*! Pops the current frame's stack storage, restoring the caller's; the
    caller's cursor is restored from its frame checkpoint. */
val operand_stack_pop_frame = impure { c: "operand_stack_pop_frame" } : unit -> unit
/*! The number of words below a cursor; the 1024-item bound is enforced by
    the EVM layer. */
val stack_top_height = impure { c: "stack_top_height" } : StackTop -> operand_stack_height
/*! The word `index` slots below the cursor's top (`index = 0` is the
    top). The EVM layer has validated `index < height`. */
val stack_slot_read = impure { c: "stack_slot_read" } : (StackTop, stack_index) -> word
/*! Overwrites the word `index` slots below the cursor's top (`SWAPn`). */
val stack_slot_write = impure { c: "stack_slot_write" } : (StackTop, stack_index, word) -> unit
/*! Advances the cursor over `count` newly pushed slots. */
val stack_top_advance = impure { c: "stack_top_advance" } : (StackTop, stack_slot_count) -> StackTop
/*! Retreats the cursor over `count` popped slots. */
val stack_top_retreat = impure { c: "stack_top_retreat" } : (StackTop, stack_slot_count) -> StackTop
