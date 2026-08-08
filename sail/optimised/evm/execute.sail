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
 * spec bodies in sail/evm/execute.sail remain the definitions for these
 * names everywhere. */
$[c_inline]
val execute_stop : unit -> unit
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

/* In-place operand windows for the pure ALU opcode bodies.
 *
 * The canonical bodies compose `pop`/`push`, threading an intermediate
 * cursor through every operand; each of those steps lowers to a generated
 * (word, cursor) tuple copy. These overrides keep the same carried-state
 * signatures but address the operand window directly: read the operands
 * from their slots (slot 0 is the canonical first pop), apply the
 * instruction's net cursor movement once, and overwrite the deepest
 * consumed slot with the result. `validate_stack` has already established
 * each instruction's stack depth before `execute` dispatches to a body, so
 * direct slot addressing below the carried cursor is safe. Charge amounts
 * and the early return on a failed charge are unchanged, and a retreated
 * cursor leaves deeper slots untouched exactly as `pop` does, so these
 * bodies are observably identical to the canonical composition. The
 * `$[c_inline]` vals carry the same fusion attribute as the other
 * per-opcode handlers above. */
$[c_inline]
val execute_add : (StackTop, gas) -> (StackTop, gas)
function execute_add(top, g) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_add(a, b));
    (top1, g1)
}

$[c_inline]
val execute_mul : (StackTop, gas) -> (StackTop, gas)
function execute_mul(top, g) = {
    let (gas_charged, g1) = charge(g, G_low);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_mul(a, b));
    (top1, g1)
}

$[c_inline]
val execute_sub : (StackTop, gas) -> (StackTop, gas)
function execute_sub(top, g) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_sub(a, b));
    (top1, g1)
}

$[c_inline]
val execute_div : (StackTop, gas) -> (StackTop, gas)
function execute_div(top, g) = {
    let (gas_charged, g1) = charge(g, G_low);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_div(a, b));
    (top1, g1)
}

$[c_inline]
val execute_sdiv : (StackTop, gas) -> (StackTop, gas)
function execute_sdiv(top, g) = {
    let (gas_charged, g1) = charge(g, G_low);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_sdiv(a, b));
    (top1, g1)
}

$[c_inline]
val execute_mod : (StackTop, gas) -> (StackTop, gas)
function execute_mod(top, g) = {
    let (gas_charged, g1) = charge(g, G_low);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_mod(a, b));
    (top1, g1)
}

$[c_inline]
val execute_smod : (StackTop, gas) -> (StackTop, gas)
function execute_smod(top, g) = {
    let (gas_charged, g1) = charge(g, G_low);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_smod(a, b));
    (top1, g1)
}

$[c_inline]
val execute_addmod : (StackTop, gas) -> (StackTop, gas)
function execute_addmod(top, g) = {
    let (gas_charged, g1) = charge(g, G_mid);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let n = stack_slot_read(top, 2);
    let top1 = stack_top_retreat(top, 2);
    stack_slot_write(top1, 0, alu_addmod(a, b, n));
    (top1, g1)
}

$[c_inline]
val execute_mulmod : (StackTop, gas) -> (StackTop, gas)
function execute_mulmod(top, g) = {
    let (gas_charged, g1) = charge(g, G_mid);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let n = stack_slot_read(top, 2);
    let top1 = stack_top_retreat(top, 2);
    stack_slot_write(top1, 0, alu_mulmod(a, b, n));
    (top1, g1)
}

/* EXP mirrors the canonical pop-then-charge order: both operands are
 * consumed before the exponent-dependent charge, so a failed charge still
 * retreats the cursor over both. */
$[c_inline]
val execute_exp : (StackTop, gas) -> (StackTop, gas)
function execute_exp(top, g) = {
    let a = stack_slot_read(top, 0);
    let e = stack_slot_read(top, 1);
    let gas_cost = exp_gas(e);
    let (gas_charged, g1) = charge(g, gas_cost);
    if not_bool(gas_charged) then {
        return (stack_top_retreat(top, 2), g1)
    };
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_exp(a, e));
    (top1, g1)
}

$[c_inline]
val execute_signextend : (StackTop, gas) -> (StackTop, gas)
function execute_signextend(top, g) = {
    let (gas_charged, g1) = charge(g, G_low);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let bi = stack_slot_read(top, 0);
    let v = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_signextend(bi, v));
    (top1, g1)
}

$[c_inline]
val execute_lt : (StackTop, gas) -> (StackTop, gas)
function execute_lt(top, g) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_lt(a, b));
    (top1, g1)
}

$[c_inline]
val execute_gt : (StackTop, gas) -> (StackTop, gas)
function execute_gt(top, g) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_gt(a, b));
    (top1, g1)
}

$[c_inline]
val execute_slt : (StackTop, gas) -> (StackTop, gas)
function execute_slt(top, g) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_slt(a, b));
    (top1, g1)
}

$[c_inline]
val execute_sgt : (StackTop, gas) -> (StackTop, gas)
function execute_sgt(top, g) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_sgt(a, b));
    (top1, g1)
}

$[c_inline]
val execute_eq : (StackTop, gas) -> (StackTop, gas)
function execute_eq(top, g) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_eq(a, b));
    (top1, g1)
}

$[c_inline]
val execute_iszero : (StackTop, gas) -> (StackTop, gas)
function execute_iszero(top, g) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    stack_slot_write(top, 0, alu_iszero(a));
    (top, g1)
}

$[c_inline]
val execute_and : (StackTop, gas) -> (StackTop, gas)
function execute_and(top, g) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_and(a, b));
    (top1, g1)
}

$[c_inline]
val execute_or : (StackTop, gas) -> (StackTop, gas)
function execute_or(top, g) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_or(a, b));
    (top1, g1)
}

$[c_inline]
val execute_xor : (StackTop, gas) -> (StackTop, gas)
function execute_xor(top, g) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    let b = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_xor(a, b));
    (top1, g1)
}

$[c_inline]
val execute_not : (StackTop, gas) -> (StackTop, gas)
function execute_not(top, g) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let a = stack_slot_read(top, 0);
    stack_slot_write(top, 0, alu_not(a));
    (top, g1)
}

$[c_inline]
val execute_byte : (StackTop, gas) -> (StackTop, gas)
function execute_byte(top, g) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let i = stack_slot_read(top, 0);
    let x = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_byte(i, x));
    (top1, g1)
}

$[c_inline]
val execute_shl : (StackTop, gas) -> (StackTop, gas)
function execute_shl(top, g) = {
    /* EIP-145 */
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let s = stack_slot_read(top, 0);
    let v = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_shl(s, v));
    (top1, g1)
}

$[c_inline]
val execute_shr : (StackTop, gas) -> (StackTop, gas)
function execute_shr(top, g) = {
    /* EIP-145 */
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let s = stack_slot_read(top, 0);
    let v = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_shr(s, v));
    (top1, g1)
}

$[c_inline]
val execute_sar : (StackTop, gas) -> (StackTop, gas)
function execute_sar(top, g) = {
    /* EIP-145 */
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let s = stack_slot_read(top, 0);
    let v = stack_slot_read(top, 1);
    let top1 = stack_top_retreat(top, 1);
    stack_slot_write(top1, 0, alu_sar(s, v));
    (top1, g1)
}

$[c_inline]
val execute_clz : (StackTop, gas) -> (StackTop, gas)
function execute_clz(top, g) = {
    /* EIP-7939 */
    let (gas_charged, g1) = charge(g, G_low);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let x = stack_slot_read(top, 0);
    stack_slot_write(top, 0, alu_clz(x));
    (top, g1)
}
