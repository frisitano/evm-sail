/* Optimized persistent-storage opcode paths for `sail/evm/execute.sail`. */

/* The complete optimized opcode implementations retain their dense account and
   storage handles entirely inside C. The Sail-visible contract remains the
   same state-passing opcode semantics as the canonical definitions: the C
   operations consume their operands relative to the carried stack cursor and
   return the carried gas, while the thin canonical wrappers apply the net
   cursor movement (SLOAD replaces its popped slot; SSTORE consumes two). */
val execute_sload_host = impure { c: "execute_sload" } : (StackTop, gas) -> gas
val execute_sstore_host = impure { c: "execute_sstore" } : (StackTop, gas) -> gas

function execute_sload(top : StackTop, g : gas) -> (StackTop, gas) = {
    let charged = execute_sload_host(top, g);
    (top, charged)
}

function execute_sstore(top : StackTop, g : gas) -> (StackTop, gas) = {
    let charged = execute_sstore_host(top, g);
    (stack_top_retreat(top, 2), charged)
}

/* Attribute-only val splices: fuse the generated step-dispatch layers.
 * With the optimized compiler's --c-inline-attr (EVM_INLINE_ATTR=on) the
 * bodies of these spec functions are inlined into their callers, collapsing
 * the stack-effect read, stack validation, and opcode switch into the one
 * interpreter loop. Without the flag the attribute is inert, and the spec
 * bodies in sail/evm/execute.sail remain the definitions in every build. */
$[c_inline]
val execute : (ast, code_pointer, StackTop, EvmMemorySlice, gas) -> (code_pointer, StackTop, EvmMemorySlice, gas)

$[c_inline]
val opcode_stack_effect : ast -> (operand_stack_height, operand_stack_height)

$[c_inline]
val execute_opcode : (ast, code_pointer, StackTop, EvmMemorySlice, gas) -> (code_pointer, StackTop, EvmMemorySlice, gas)

/* Attribute-only val splices for the fused generated interpreter: with
 * --c-inline-attr (EVM_INLINE_ATTR=on) the narrow per-opcode handlers inline
 * into the fused step's arms, removing the per-opcode call boundary that
 * dominates the generated loop's overhead. Inert in every other build; the
 * spec bodies in sail/evm/execute.sail remain the definitions everywhere. */
$[c_inline]
val execute_stop : unit -> unit
$[c_inline]
val execute_add : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_mul : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_sub : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_div : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_sdiv : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_mod : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_smod : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_addmod : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_mulmod : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_exp : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_signextend : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_lt : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_gt : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_slt : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_sgt : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_eq : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_iszero : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_and : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_or : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_xor : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_not : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_byte : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_shl : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_shr : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_sar : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_clz : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_keccak256 : (StackTop, EvmMemorySlice, gas) -> (StackTop, EvmMemorySlice, gas)
$[c_inline]
val execute_address : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_caller : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_callvalue : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_calldataload : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_calldatasize : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_pop : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_mload : (StackTop, EvmMemorySlice, gas) -> (StackTop, EvmMemorySlice, gas)
$[c_inline]
val execute_mstore : (StackTop, EvmMemorySlice, gas) -> (StackTop, EvmMemorySlice, gas)
$[c_inline]
val execute_mstore8 : (StackTop, EvmMemorySlice, gas) -> (StackTop, EvmMemorySlice, gas)
$[c_inline]
val execute_msize : (StackTop, EvmMemorySlice, gas) -> (StackTop, EvmMemorySlice, gas)
$[c_inline]
val execute_jump : (code_pointer, StackTop, gas) -> (code_pointer, StackTop, gas)
$[c_inline]
val execute_jumpi : (code_pointer, StackTop, gas) -> (code_pointer, StackTop, gas)
$[c_inline]
val execute_pc : (code_pointer, StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_gas : (StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_jumpdest : gas -> gas
$[c_inline]
val execute_push : (push_width, word, StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_dup : (stack_operation_index, StackTop, gas) -> (StackTop, gas)
$[c_inline]
val execute_swap : (stack_operation_index, StackTop, gas) -> (StackTop, gas)
