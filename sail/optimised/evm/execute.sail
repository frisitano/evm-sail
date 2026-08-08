/* Optimized persistent-storage opcode paths for `sail/evm/execute.sail`. */

/* The complete optimized opcode implementations retain their dense account and
   storage handles entirely inside C. The Sail-visible contract remains the
   same unit-to-unit opcode semantics as the canonical definitions. */
val execute_sload_host = impure { c: "execute_sload" } : unit -> unit
val execute_sstore_host = impure { c: "execute_sstore" } : unit -> unit

function execute_sload() -> unit = execute_sload_host()
function execute_sstore() -> unit = execute_sstore_host()

/* Attribute-only val splices: fuse the generated step-dispatch layers.
 * With the optimized compiler's --c-inline-attr (EVM_INLINE_ATTR=on) the
 * bodies of these spec functions are inlined into their callers, collapsing
 * the stack-effect read, stack validation, and opcode switch into the one
 * interpreter loop. Without the flag the attribute is inert, and the spec
 * bodies in sail/evm/execute.sail remain the definitions in every build. */
$[c_inline]
val execute : ast -> unit

$[c_inline]
val opcode_stack_effect : ast -> (operand_stack_height, operand_stack_height)

$[c_inline]
val execute_opcode : ast -> unit

/* Attribute-only val splices for the fused generated interpreter: with
 * --c-inline-attr (EVM_INLINE_ATTR=on) the narrow per-opcode handlers inline
 * into the fused step's arms, removing the per-opcode call boundary that
 * dominates the generated loop's overhead. Inert in every other build; the
 * spec bodies in sail/evm/execute.sail remain the definitions everywhere. */
$[c_inline]
val execute_stop : unit -> unit
$[c_inline]
val execute_add : unit -> unit
$[c_inline]
val execute_mul : unit -> unit
$[c_inline]
val execute_sub : unit -> unit
$[c_inline]
val execute_div : unit -> unit
$[c_inline]
val execute_sdiv : unit -> unit
$[c_inline]
val execute_mod : unit -> unit
$[c_inline]
val execute_smod : unit -> unit
$[c_inline]
val execute_addmod : unit -> unit
$[c_inline]
val execute_mulmod : unit -> unit
$[c_inline]
val execute_exp : unit -> unit
$[c_inline]
val execute_signextend : unit -> unit
$[c_inline]
val execute_lt : unit -> unit
$[c_inline]
val execute_gt : unit -> unit
$[c_inline]
val execute_slt : unit -> unit
$[c_inline]
val execute_sgt : unit -> unit
$[c_inline]
val execute_eq : unit -> unit
$[c_inline]
val execute_iszero : unit -> unit
$[c_inline]
val execute_and : unit -> unit
$[c_inline]
val execute_or : unit -> unit
$[c_inline]
val execute_xor : unit -> unit
$[c_inline]
val execute_not : unit -> unit
$[c_inline]
val execute_byte : unit -> unit
$[c_inline]
val execute_shl : unit -> unit
$[c_inline]
val execute_shr : unit -> unit
$[c_inline]
val execute_sar : unit -> unit
$[c_inline]
val execute_clz : unit -> unit
$[c_inline]
val execute_keccak256 : unit -> unit
$[c_inline]
val execute_address : unit -> unit
$[c_inline]
val execute_caller : unit -> unit
$[c_inline]
val execute_callvalue : unit -> unit
$[c_inline]
val execute_calldataload : unit -> unit
$[c_inline]
val execute_calldatasize : unit -> unit
$[c_inline]
val execute_pop : unit -> unit
$[c_inline]
val execute_mload : unit -> unit
$[c_inline]
val execute_mstore : unit -> unit
$[c_inline]
val execute_mstore8 : unit -> unit
$[c_inline]
val execute_msize : unit -> unit
$[c_inline]
val execute_jump : unit -> unit
$[c_inline]
val execute_jumpi : unit -> unit
$[c_inline]
val execute_pc : unit -> unit
$[c_inline]
val execute_gas : unit -> unit
$[c_inline]
val execute_jumpdest : unit -> unit
$[c_inline]
val execute_push : (push_width, word) -> unit
$[c_inline]
val execute_dup : (stack_operation_index) -> unit
$[c_inline]
val execute_swap : (stack_operation_index) -> unit
