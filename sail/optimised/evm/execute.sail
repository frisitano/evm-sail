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
