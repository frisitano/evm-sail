/* Optimized C overrides for `sail/evm/machine.sail`. */

/* The optimized host returns the newly-entered memory frame as its native
 * pointer-bearing slice. Canonical and proof backends retain the semantic
 * arena coordinate exposed by `mem_frame_enter`. */
val mem_frame_enter_slice = impure { c: "mem_frame_enter_slice" } : unit -> EvmMemorySlice

function memory_frame_enter() -> EvmMemorySlice = {
    mem_frame_enter_slice()
}

/* Attribute-only val splice: with --c-inline-attr (EVM_INLINE_ATTR=on) the
 * handler-local stack precondition folds into each fused opcode arm, where its
 * handler's constant (delta, alpha) arguments reduce it to a bounds compare. Inert in
 * every other build. */
$[c_inline]
val validate_stack : (StackPointer, operand_stack_height, operand_stack_height) -> StackValidation

$[c_inline]
val stack_height : StackPointer -> operand_stack_height

$[c_inline]
val read_stack_word : StackPointer -> word

$[c_inline]
val write_stack_word : (StackPointer, word) -> unit

$[c_inline]
val stack_set : (StackPointer, stack_index, word) -> unit
