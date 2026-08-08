/* Optimized C overrides for `sail/evm/machine.sail`. */

/* The optimized host returns the newly-entered memory frame as its native
 * pointer-bearing slice. Canonical and proof backends retain the semantic
 * arena coordinate exposed by `mem_frame_enter`. */
val mem_frame_enter_slice = impure { c: "mem_frame_enter_slice" } : unit -> EvmMemorySlice

function memory_frame_enter() -> EvmMemorySlice = {
    let parent = evm_memory;
    evm_memory = mem_frame_enter_slice();
    parent
}

/* Canonical callers establish a shared admitted-gas budget. Reify that proof
 * obligation at the few credit boundaries so production uint64 addition can
 * never wrap if an internal invariant is violated. */
function conserved_gas_add(left, right) =
    if right <= sizeof(block_gas_limit_bound) - left then {
        left + right
    } else {
        fatal_error(ExecutionInvalid)
    }

/* Optimized `gas` is a uint64 range, so reduction modulo 2^256 is the
 * identity. Avoid materializing the 257-bit word modulus solely to widen a
 * live-gas value before pushing it. */
function push_gas(top : StackTop, value : gas) -> StackTop =
    push_word(top, u256(value))

/* Attribute-only val splice: with --c-inline-attr (EVM_INLINE_ATTR=on) the
 * stack precondition folds into each fused interpreter arm, where its
 * constant (delta, alpha) arguments reduce it to a bounds compare. Inert in
 * every other build. */
$[c_inline]
val validate_stack : (gas, StackTop, operand_stack_height, operand_stack_height) -> (bool, gas)
