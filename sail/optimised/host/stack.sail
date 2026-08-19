/* Native-word-pointer refinement of the canonical operand-stack host surface.
 * Every operation receives the complete cursor so generated C never converts
 * its `u256 *` storage field through an integer carrier. */

val stack_reset_host = impure { c: "stack_reset" } : unit -> StackPointer
val operand_stack_push_empty_frame_host = impure { c: "operand_stack_push_empty_frame" } : unit -> StackPointer
val operand_stack_pop_frame = impure { c: "operand_stack_pop_frame" } : unit -> unit
val stack_top_height_host = impure { c: "stack_top_height" } : StackPointer -> operand_stack_height
val stack_slot_read_host = impure { c: "stack_slot_read" } : (StackPointer, stack_index) -> word
val stack_slot_write_host = impure { c: "stack_slot_write" } : (StackPointer, stack_index, word) -> unit
val stack_slot_write_next_host = impure { c: "stack_slot_write_next" } : (StackPointer, word) -> unit
val stack_top_advance_host = impure { c: "stack_top_advance" } : (StackPointer, stack_slot_count) -> StackPointer
val stack_top_retreat_host = impure { c: "stack_top_retreat" } : (StackPointer, stack_slot_count) -> StackPointer

$[c_inline]
function stack_reset() -> StackPointer = stack_reset_host()

$[c_inline]
function operand_stack_push_empty_frame() -> StackPointer =
    operand_stack_push_empty_frame_host()

$[c_inline]
function stack_top_height(top : StackPointer) -> operand_stack_height =
    top.height

$[c_inline]
function stack_slot_read(top : StackPointer, index : stack_index) -> word =
    stack_slot_read_host(top, index)

$[c_inline]
function stack_slot_write(top : StackPointer, index : stack_index, value : word) -> unit =
    stack_slot_write_host(top, index, value)

$[c_inline]
function stack_slot_write_next(top : StackPointer, value : word) -> unit =
    stack_slot_write_next_host(top, value)

$[c_inline]
function stack_top_advance(top : StackPointer, count : stack_slot_count) -> StackPointer =
    stack_top_advance_host(top, count)

$[c_inline]
function stack_top_retreat(top : StackPointer, count : stack_slot_count) -> StackPointer =
    stack_top_retreat_host(top, count)
