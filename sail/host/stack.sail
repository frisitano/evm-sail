
/*md
# The operand stack

The per-frame EVM operand stack (YP §9.1, μ_s): up to 1024 words, LIFO.
The words live behind the host interface (the corresponding `stack.c` in
`extractions/c/spec/contract/` or `extractions/c/optimised/contract/`) as per-frame flat arrays for O(1) access;
proof targets see the `val`s as bodyless axioms over an abstract per-frame
list-of-words model.

The active frame's position is the abstract cursor token
[StackPointer][type-StackPointer], threaded by value through the interpreter and
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
val stack_reset_host = impure { c: "stack_reset" } : unit -> bits(64)
/*! Pushes a fresh, empty operand stack for a sub-call frame and returns
    its empty cursor. */
val operand_stack_push_empty_frame_host = impure { c: "operand_stack_push_empty_frame" } : unit -> bits(64)
/*! Pops the current frame's stack storage, restoring the caller's; the
    caller's cursor is restored from its frame checkpoint. */
val operand_stack_pop_frame = impure { c: "operand_stack_pop_frame" } : unit -> unit
/*! The number of words below a cursor; the 1024-item bound is enforced by
    the EVM layer. */
val stack_top_height_host = impure { c: "stack_top_height" } : bits(64) -> operand_stack_height
/*! The word `index` slots below the cursor's top (`index = 0` is the
    top). The EVM layer has validated `index < height`. */
val stack_slot_read_host = impure { c: "stack_slot_read" } : (bits(64), stack_index) -> word
/*! Overwrites the word `index` slots below the cursor's top (`SWAPn`). */
val stack_slot_write_host = impure { c: "stack_slot_write" } : (bits(64), stack_index, word) -> unit
/*! Writes the next free slot without changing the cursor. */
val stack_slot_write_next_host = impure { c: "stack_slot_write_next" } : (bits(64), word) -> unit
/*! Advances the cursor over `count` newly pushed slots. */
val stack_top_advance_host = impure { c: "stack_top_advance" } : (bits(64), stack_slot_count) -> bits(64)
/*! Retreats the cursor over `count` popped slots. */
val stack_top_retreat_host = impure { c: "stack_top_retreat" } : (bits(64), stack_slot_count) -> bits(64)

function stack_reset() -> StackPointer =
    struct { storage = stack_reset_host(), height = 0 }

function operand_stack_push_empty_frame() -> StackPointer =
    struct { storage = operand_stack_push_empty_frame_host(), height = 0 }

function stack_top_height(top : StackPointer) -> operand_stack_height =
    top.height

function stack_slot_read(top : StackPointer, index : stack_index) -> word =
    stack_slot_read_host(top.storage, index)

function stack_slot_write(top : StackPointer, index : stack_index, value : word) -> unit =
    stack_slot_write_host(top.storage, index, value)

function stack_slot_write_next(top : StackPointer, value : word) -> unit =
    stack_slot_write_next_host(top.storage, value)

function stack_top_advance(top : StackPointer, count : stack_slot_count) -> StackPointer = {
    let storage = stack_top_advance_host(top.storage, count);
    struct { storage = storage, height = stack_top_height_host(storage) }
}

function stack_top_retreat(top : StackPointer, count : stack_slot_count) -> StackPointer = {
    let storage = stack_top_retreat_host(top.storage, count);
    struct { storage = storage, height = stack_top_height_host(storage) }
}
