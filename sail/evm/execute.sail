
/*md
# Opcode semantics

The single-step transition function of the EVM (Yellow Paper §9): given
the decoded opcode, [execute][] charges its gas, consumes its stack
operands, and produces its result or effect. One match arm per opcode,
grouped by family (arithmetic, bitwise, keccak, environment, block,
stack/memory, storage, flow, push/dup/swap, log, system).

Handlers follow the state-passing convention (YP μ′ = Ξ(μ)): each takes
only the carried values it uses and returns the same values in the same order. Decoded
instruction data remains explicit where required. The registers behind the
state are read and written only at frame boundaries.

Pure compute is done here; every world effect is a kernel syscall
(`k_*`). All gas and policy stays in the EVM: it marks-and-prices access
via the kernel's returned warm bit (EIP-2929), decides whether an effect
happens, and issues the syscall only for real effects (a no-op `SSTORE`
charges gas but issues no host write). Memory-touching opcodes pay the
quadratic expansion cost via [memory_expansion_cost][] before acting.
Sub-calls and creates delegate to [run_call][] / [run_create][], which
install a child frame and save its parent continuation. The non-recursive
opcode bodies live in [execute_opcode][].
*/
/*! Lightweight result returned by non-terminal opcode handlers. Return and
    revert data stay on their specialized paths rather than inflating this
    result. */
function opcode_failed(result : OpcodeOutcome) -> bool = match result {
    Continue() => false,
    Failed(_) => true,
}

function opcode_frame_status(result : OpcodeOutcome) -> FrameStatus = match result {
    Continue() => Running(),
    Failed(kind) => Exceptional(kind),
}

/*md
## Helpers
*/
/*! The storage owner (YP I_a): `SLOAD`, `SSTORE`, `LOG`, and
    `SELFDESTRUCT` act on this account. */
function self_addr(message : Message) -> address = message.address

/*! EIP-214 write protection: any state-modifying opcode in a
    `STATICCALL` frame halts exceptionally on the carried gas.
    State-changing opcodes call this first; `true` means already
    halted. */
function guard_static(g : gas, is_static : bool) -> (gas, OpcodeOutcome) =
    if is_static then {
        (GAS_ZERO, Failed(WriteProtection))
    } else {
        (g, Continue())
    }

/*! `JUMP`/`JUMPI` target validity: the destination must be in code range
    and land on a `JUMPDEST` (the precomputed valid-destination set,
    YP §9.4.3); otherwise an invalid-jump exceptional halt. PUSH-data
    bytes that happen to equal `0x5b` are not valid. Returns the next
    program counter and the carried gas. */
function do_jump(pc_in : code_pointer, g : gas, frame_code : Code, destination_value : word) -> (code_pointer, gas, OpcodeOutcome) = {
    let code_length = frame_code_len(frame_code);
    if destination_value < code_length then {
        let destination : code_pointer = destination_value;
        let valid_destination = frame_jumpdest_valid(frame_code, destination);
        if valid_destination then {
            (destination, g, Continue())
        } else {
            (pc_in, GAS_ZERO, Failed(InvalidJump))
        }
    } else {
        (pc_in, GAS_ZERO, Failed(InvalidJump))
    }
}

/*! Establishes an opcode's local stack precondition before gas charging or
    any instruction side effect. The caller supplies the opcode's constant
    Yellow Paper `(delta, alpha)` values. */
function guard_stack(
    carried_sp : StackPointer,
    inputs : operand_stack_height,
    outputs : operand_stack_height,
) -> OpcodeOutcome = match validate_stack(carried_sp, inputs, outputs) {
    StackValid => Continue(),
    StackUnderflowFailure => Failed(StackUnderflow),
    StackOverflowFailure => Failed(StackOverflow),
}


/*! Pops `count` log topics from the stack into its bounded representation. */
function pop_log_topics(count : log_topic_count, sp_in : StackPointer) -> (LogTopics, StackPointer) = {
    var sp = sp_in;
    match count {
        0 => (LogTopics0(), sp),
        1 => {
            let t0 = read_stack_word(sp);
            sp = stack_top_retreat(sp, 1);
            (LogTopics1(t0), sp)
        },
        2 => {
            let t0 = read_stack_word(sp);
            sp = stack_top_retreat(sp, 1);
            let t1 = read_stack_word(sp);
            sp = stack_top_retreat(sp, 1);
            (LogTopics2((t0, t1)), sp)
        },
        3 => {
            let t0 = read_stack_word(sp);
            sp = stack_top_retreat(sp, 1);
            let t1 = read_stack_word(sp);
            sp = stack_top_retreat(sp, 1);
            let t2 = read_stack_word(sp);
            sp = stack_top_retreat(sp, 1);
            (LogTopics3((t0, t1, t2)), sp)
        },
        4 => {
            let t0 = read_stack_word(sp);
            sp = stack_top_retreat(sp, 1);
            let t1 = read_stack_word(sp);
            sp = stack_top_retreat(sp, 1);
            let t2 = read_stack_word(sp);
            sp = stack_top_retreat(sp, 1);
            let t3 = read_stack_word(sp);
            sp = stack_top_retreat(sp, 1);
            (LogTopics4((t0, t1, t2, t3)), sp)
        },
        _ => (LogTopics0(), sp),
    }
}

/*md
## The opcode bodies

Each semantic opcode constructor has a named body. The standard decoder
validates the instruction's stack effect before dispatching its `ast` to these
functions below, so the bodies consume and produce operands unchecked.
Optimized C applies the same validation at its raw-byte dispatch boundary
before calling a generated body, without materializing an `ast`.
*/
/*! Implements `ADD`. */
function execute_add(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    let result = alu_add(a, b);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `MUL`. */
function execute_mul(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_low);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    let result = alu_mul(a, b);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `SUB`. */
function execute_sub(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    let result = alu_sub(a, b);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements unsigned `DIV`. */
function execute_div(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_low);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    let result = alu_div(a, b);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements signed `SDIV`. */
function execute_sdiv(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_low);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    let result = alu_sdiv(a, b);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements unsigned `MOD`. */
function execute_mod(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_low);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    let result = alu_mod(a, b);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements signed `SMOD`. */
function execute_smod(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_low);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    let result = alu_smod(a, b);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `ADDMOD`. */
function execute_addmod(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 3, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_mid);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let n = read_stack_word(sp_after);
    let result = alu_addmod(a, b, n);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `MULMOD`. */
function execute_mulmod(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 3, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_mid);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let n = read_stack_word(sp_after);
    let result = alu_mulmod(a, b, n);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `EXP`, including exponent-dependent gas. */
function execute_exp(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let e = read_stack_word(sp_after);
    let gas_cost = exp_gas(e);
    let (halt, gas_after_charge) = charge(gas_after, gas_cost);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let result = alu_exp(a, e);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `SIGNEXTEND`. */
function execute_signextend(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_low);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let bi = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let v = read_stack_word(sp_after);
    let result = alu_signextend(bi, v);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements unsigned `LT`. */
function execute_lt(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    let result = alu_lt(a, b);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements unsigned `GT`. */
function execute_gt(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    let result = alu_gt(a, b);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements signed `SLT`. */
function execute_slt(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    let result = alu_slt(a, b);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements signed `SGT`. */
function execute_sgt(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    let result = alu_sgt(a, b);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `EQ`. */
function execute_eq(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    let result = alu_eq(a, b);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `ISZERO`. */
function execute_iszero(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, carried_sp, Failed(OutOfGas))
    };
    let a = read_stack_word(carried_sp);
    let result = alu_iszero(a);
    write_stack_word(carried_sp, result);
    gas_after = gas_after_charge;
    (gas_after, carried_sp, Continue())
}

/*! Implements bitwise `AND`. */
function execute_and(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    let result = alu_and(a, b);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements bitwise `OR`. */
function execute_or(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    let result = alu_or(a, b);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements bitwise `XOR`. */
function execute_xor(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let a = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let b = read_stack_word(sp_after);
    let result = alu_xor(a, b);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements bitwise `NOT`. */
function execute_not(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, carried_sp, Failed(OutOfGas))
    };
    let a = read_stack_word(carried_sp);
    let result = alu_not(a);
    write_stack_word(carried_sp, result);
    gas_after = gas_after_charge;
    (gas_after, carried_sp, Continue())
}

/*! Implements `BYTE`. */
function execute_byte(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let i = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let x = read_stack_word(sp_after);
    let result = alu_byte(i, x);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements logical left shift `SHL` (EIP-145). */
function execute_shl(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
/* EIP-145 */
    let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let s = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let v = read_stack_word(sp_after);
    let result = alu_shl(s, v);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements logical right shift `SHR` (EIP-145). */
function execute_shr(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
/* EIP-145 */
    let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let s = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let v = read_stack_word(sp_after);
    let result = alu_shr(s, v);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements arithmetic right shift `SAR` (EIP-145). */
function execute_sar(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
/* EIP-145 */
    let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let s = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let v = read_stack_word(sp_after);
    let result = alu_sar(s, v);
    write_stack_word(sp_after, result);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements count-leading-zeroes `CLZ` (EIP-7939). */
function execute_clz(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
/* EIP-7939 */
    let (halt, gas_after_charge) = charge(gas_after, G_low);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, carried_sp, Failed(OutOfGas))
    };
    let x = read_stack_word(carried_sp);
    let result = alu_clz(x);
    write_stack_word(carried_sp, result);
    gas_after = gas_after_charge;
    (gas_after, carried_sp, Continue())
}

/*! Implements `KECCAK256` over an expanded memory range. */
function execute_keccak256(carried_gas : gas, carried_sp : StackPointer, carried_memory : EvmMemorySlice) -> (gas, StackPointer, EvmMemorySlice, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, carried_memory, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
    var memory_after : EvmMemorySlice = carried_memory;
let offset_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let length_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let (hashing_halt, gas_after_hashing) = charge_keccak_gas(gas_after, length_word);
    if hashing_halt then {
        gas_after = gas_after_hashing;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let required_size = memory_required_size(offset_word, length_word);
    let (expansion_halt, gas_after_expansion) = charge_memory_expansion(gas_after_hashing, memory_after, required_size);
    if expansion_halt then {
        gas_after = gas_after_expansion;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let access = memory_access(offset_word, length_word);
    let mem1 = expand_memory(memory_after, access.required_size);
    let (digest, mem2) = mem_keccak(mem1, access.range);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, digest);
    memory_after = mem2;
    gas_after = gas_after_expansion;
    (gas_after, sp_after, memory_after, Continue())
}

/*! Implements `ADDRESS`. */
function execute_address(carried_address : address, carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let address_word = address_to_word(carried_address);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, address_word);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `ORIGIN`. */
function execute_origin(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let origin = k_env(F_Origin);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, origin);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `CALLER`. */
function execute_caller(carried_caller : address, carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let caller = address_to_word(carried_caller);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, caller);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `CALLVALUE`. */
function execute_callvalue(carried_value : word, carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, carried_value);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `GASPRICE`. */
function execute_gasprice(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let gas_price = k_env(F_GasPrice);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, gas_price);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `CALLDATASIZE`. */
function execute_calldatasize(carried_calldata : CalldataSlice, carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let input = carried_calldata;
    let input_length = region_slice_length(input);
    let length_word = word_of_byte_count(input_length);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, length_word);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `CALLDATALOAD`. */
function execute_calldataload(carried_calldata : CalldataSlice, carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let offset_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let value = slice_load_word_offset(carried_calldata, offset_word);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, value);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `CALLDATACOPY`. */
function execute_calldatacopy(carried_calldata : CalldataSlice, carried_gas : gas, carried_sp : StackPointer, carried_memory : EvmMemorySlice) -> (gas, StackPointer, EvmMemorySlice, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 3, 0);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, carried_memory, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
    var memory_after : EvmMemorySlice = carried_memory;
let (base_halt, gas_after_base) = charge(gas_after, G_verylow);
    if base_halt then {
        gas_after = gas_after_base;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let destination_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let source_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let length_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let (copy_halt, gas_after_copy) = charge_copy_gas(gas_after_base, length_word);
    if copy_halt then {
        gas_after = gas_after_copy;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let required_size = memory_required_size(destination_word, length_word);
    let (expansion_halt, gas_after_expansion) = charge_memory_expansion(gas_after_copy, memory_after, required_size);
    if expansion_halt then {
        gas_after = gas_after_expansion;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let access = memory_access(destination_word, length_word);
    let mem1 = expand_memory(memory_after, access.required_size);
    let range = access.range;
    slice_copy_word_offset(carried_calldata, range.off, source_word, range.len);
    memory_after = mem1;
    gas_after = gas_after_expansion;
    (gas_after, sp_after, memory_after, Continue())
}

/*! Implements `CODESIZE`. */
function execute_codesize(carried_code : Code, carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let code_length = frame_code_len(carried_code);
    let length_word = word_of_byte_count(code_length);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, length_word);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `CODECOPY`. */
function execute_codecopy(carried_code : Code, carried_gas : gas, carried_sp : StackPointer, carried_memory : EvmMemorySlice) -> (gas, StackPointer, EvmMemorySlice, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 3, 0);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, carried_memory, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
    var memory_after : EvmMemorySlice = carried_memory;
let (base_halt, gas_after_base) = charge(gas_after, G_verylow);
    if base_halt then {
        gas_after = gas_after_base;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let destination_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let source_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let length_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let (copy_halt, gas_after_copy) = charge_copy_gas(gas_after_base, length_word);
    if copy_halt then {
        gas_after = gas_after_copy;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let required_size = memory_required_size(destination_word, length_word);
    let (expansion_halt, gas_after_expansion) = charge_memory_expansion(gas_after_copy, memory_after, required_size);
    if expansion_halt then {
        gas_after = gas_after_expansion;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let access = memory_access(destination_word, length_word);
    let mem1 = expand_memory(memory_after, access.required_size);
    let range = access.range;
    let code = carried_code;
    let bytes = code_bytes(code);
    slice_copy_word_offset(bytes, range.off, source_word, range.len);
    memory_after = mem1;
    gas_after = gas_after_expansion;
    (gas_after, sp_after, memory_after, Continue())
}

/*! Implements `BALANCE`, including warm/cold account access. */
function execute_balance(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let address_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let a = word_to_address(address_word);
    let warm = k_account_is_warm(a);
    let gas_cost = account_cost(warm);
    let (halt, gas_after_charge) = charge(gas_after, gas_cost);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    k_account_mark_warm(a);
    let balance = k_get_balance(a);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, balance);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `SELFBALANCE`. */
function execute_selfbalance(carried_address : address, carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_low);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let balance = k_get_balance(carried_address);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, balance);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `EXTCODESIZE`, including warm/cold account access. */
function execute_extcodesize(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let address_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let a = word_to_address(address_word);
    let warm = k_account_is_warm(a);
    let access_cost = account_cost(warm);
    let read_cost = external_code_read_cost();
    let (halt, gas_after_charge) = charge(gas_after, access_cost + read_cost);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    k_account_mark_warm(a);
    let code_size = k_get_code_size(a);
    let size_word = word_of_byte_count(code_size);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, size_word);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `EXTCODECOPY`, including access and copy gas. */
function execute_extcodecopy(carried_gas : gas, carried_sp : StackPointer, carried_memory : EvmMemorySlice) -> (gas, StackPointer, EvmMemorySlice, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 4, 0);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, carried_memory, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
    var memory_after : EvmMemorySlice = carried_memory;
let address_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let a = word_to_address(address_word);
    let destination_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let source_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let length_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let warm = k_account_is_warm(a);
    let access_cost = account_cost(warm);
    let read_cost = external_code_read_cost();
    let (access_halt, gas_after_access) = charge(gas_after, access_cost + read_cost);
    if access_halt then {
        gas_after = gas_after_access;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let (copy_halt, gas_after_copy) = charge_copy_gas(gas_after_access, length_word);
    if copy_halt then {
        gas_after = gas_after_copy;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let required_size = memory_required_size(destination_word, length_word);
    let (expansion_halt, gas_after_expansion) = charge_memory_expansion(gas_after_copy, memory_after, required_size);
    if expansion_halt then {
        gas_after = gas_after_expansion;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let access = memory_access(destination_word, length_word);
    let mem1 = expand_memory(memory_after, access.required_size);
    let range = access.range;
    k_account_mark_warm(a);
    k_code_copy(a, range.off, source_word, range.len);
    memory_after = mem1;
    gas_after = gas_after_expansion;
    (gas_after, sp_after, memory_after, Continue())
}

/*! Implements `EXTCODEHASH`, including warm/cold account access. */
function execute_extcodehash(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let address_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let a = word_to_address(address_word);
    let warm = k_account_is_warm(a);
    let gas_cost = account_cost(warm);
    let (halt, gas_after_charge) = charge(gas_after, gas_cost);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    k_account_mark_warm(a);
    let code_hash = k_get_codehash(a);
    let hash_word = hash_to_word(code_hash);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, hash_word);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `RETURNDATASIZE`. */
function execute_returndatasize(carried_returndata : OutputSlice, carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let return_data_size = returndata_size(carried_returndata);
    let size_word = word_of_byte_count(return_data_size);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, size_word);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements bounds-checked `RETURNDATACOPY`. */
function execute_returndatacopy(carried_returndata : OutputSlice, carried_gas : gas, carried_sp : StackPointer, carried_memory : EvmMemorySlice) -> (gas, StackPointer, EvmMemorySlice, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 3, 0);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, carried_memory, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
    var memory_after : EvmMemorySlice = carried_memory;
let (base_halt, gas_after_base) = charge(gas_after, G_verylow);
    if base_halt then {
        gas_after = gas_after_base;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let destination_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let source_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let length_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let (available as 'available) = returndata_size(carried_returndata);
    if source_word <= available then {
        let remaining = returndata_remaining(available, source_word);
        let bounded_source_offset : source_pointer = source_word;
        if length_word <= remaining then {
            let bounded_length : memory_length = length_word;
            let (copy_halt, gas_after_copy) = charge_copy_gas(gas_after_base, length_word);
            if copy_halt then {
                gas_after = gas_after_copy;
                return (gas_after, sp_after, memory_after, Failed(OutOfGas))
            };
            let required_size = memory_required_size(destination_word, length_word);
            let (expansion_halt, gas_after_expansion) = charge_memory_expansion(gas_after_copy, memory_after, required_size);
            if expansion_halt then {
                gas_after = gas_after_expansion;
                return (gas_after, sp_after, memory_after, Failed(OutOfGas))
            };
            let access = memory_access(destination_word, length_word);
            let mem1 = expand_memory(memory_after, access.required_size);
            returndata_copy(carried_returndata, access.range.off, bounded_source_offset, bounded_length);
            memory_after = mem1;
            gas_after = gas_after_expansion;
            (gas_after, sp_after, memory_after, Continue())
        } else {
            (GAS_ZERO, sp_after, memory_after, Failed(InvalidOpcode))
        }
    } else {
        (GAS_ZERO, sp_after, memory_after, Failed(InvalidOpcode))
    }
}

/*! Implements `BLOCKHASH`. */
function execute_blockhash(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, 20);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let block_number = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let block_hash = k_blockhash(block_number);
    let hash_word = hash_to_word(block_hash);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, hash_word);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `COINBASE`. */
function execute_coinbase(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let coinbase = k_env(F_Coinbase);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, coinbase);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `TIMESTAMP`. */
function execute_timestamp(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let timestamp = k_env(F_Timestamp);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, timestamp);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `NUMBER`. */
function execute_number(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let block_number = k_env(F_Number);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, block_number);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `SLOTNUM`. */
function execute_slotnum(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let slot_number = k_env(F_SlotNumber);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, slot_number);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `PREVRANDAO`. */
function execute_prevrandao(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let prev_randao = k_env(F_PrevRandao);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, prev_randao);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `GASLIMIT`. */
function execute_gaslimit(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let gas_limit = k_env(F_GasLimit);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, gas_limit);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `CHAINID`. */
function execute_chainid(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let chain_id = k_env(F_ChainId);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, chain_id);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `BASEFEE`. */
function execute_basefee(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let base_fee = k_env(F_BaseFee);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, base_fee);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `BLOBBASEFEE`. */
function execute_blobbasefee(
    blob_fee : word,
    carried_gas : gas,
    carried_sp : StackPointer,
) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, blob_fee);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `BLOBHASH`. */
function execute_blobhash(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let index = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let blob_hash = k_blobhash(index);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, blob_hash);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `POP`. */
function execute_pop(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 0);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let _ = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `MLOAD`. */
function execute_mload(carried_gas : gas, carried_sp : StackPointer, carried_memory : EvmMemorySlice) -> (gas, StackPointer, EvmMemorySlice, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, carried_memory, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
    var memory_after : EvmMemorySlice = carried_memory;
let (base_halt, gas_after_base) = charge(gas_after, G_verylow);
    if base_halt then {
        gas_after = gas_after_base;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let offset_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let word_size = u256(32);
    let required_size = memory_required_size(offset_word, word_size);
    let (expansion_halt, gas_after_expansion) = charge_memory_expansion(gas_after_base, memory_after, required_size);
    if expansion_halt then {
        gas_after = gas_after_expansion;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let access = memory_access(offset_word, word_size);
    let mem1 = expand_memory(memory_after, access.required_size);
    let value = mem_load(access.range.off);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, value);
    memory_after = mem1;
    gas_after = gas_after_expansion;
    (gas_after, sp_after, memory_after, Continue())
}

/*! Implements `MSTORE`. */
function execute_mstore(carried_gas : gas, carried_sp : StackPointer, carried_memory : EvmMemorySlice) -> (gas, StackPointer, EvmMemorySlice, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 0);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, carried_memory, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
    var memory_after : EvmMemorySlice = carried_memory;
let (base_halt, gas_after_base) = charge(gas_after, G_verylow);
    if base_halt then {
        gas_after = gas_after_base;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let offset_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let v = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let word_size = u256(32);
    let required_size = memory_required_size(offset_word, word_size);
    let (expansion_halt, gas_after_expansion) = charge_memory_expansion(gas_after_base, memory_after, required_size);
    if expansion_halt then {
        gas_after = gas_after_expansion;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let access = memory_access(offset_word, word_size);
    let mem1 = expand_memory(memory_after, access.required_size);
    mem_store(access.range.off, v);
    memory_after = mem1;
    gas_after = gas_after_expansion;
    (gas_after, sp_after, memory_after, Continue())
}

/*! Implements `MSTORE8`. */
function execute_mstore8(carried_gas : gas, carried_sp : StackPointer, carried_memory : EvmMemorySlice) -> (gas, StackPointer, EvmMemorySlice, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 0);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, carried_memory, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
    var memory_after : EvmMemorySlice = carried_memory;
let (base_halt, gas_after_base) = charge(gas_after, G_verylow);
    if base_halt then {
        gas_after = gas_after_base;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let offset_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let v = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let required_size = memory_required_size(offset_word, WORD_ONE);
    let (expansion_halt, gas_after_expansion) = charge_memory_expansion(gas_after_base, memory_after, required_size);
    if expansion_halt then {
        gas_after = gas_after_expansion;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let access = memory_access(offset_word, WORD_ONE);
    let mem1 = expand_memory(memory_after, access.required_size);
    mem_store_byte(access.range.off, v);
    memory_after = mem1;
    gas_after = gas_after_expansion;
    (gas_after, sp_after, memory_after, Continue())
}

/*! Implements `MSIZE`. */
function execute_msize(carried_gas : gas, carried_sp : StackPointer, carried_memory : EvmMemorySlice) -> (gas, StackPointer, EvmMemorySlice, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, carried_memory, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, carried_memory, Failed(OutOfGas))
    };
    let high_water = memory_high_water(carried_memory);
    let words = memory_word_count(high_water);
    let size = word_of_nat_byte_count(words * 32);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, size);
    gas_after = gas_after_charge;
    (gas_after, sp_after, carried_memory, Continue())
}

/*! Implements overlapping memory copy `MCOPY` (EIP-5656). */
function execute_mcopy(carried_gas : gas, carried_sp : StackPointer, carried_memory : EvmMemorySlice) -> (gas, StackPointer, EvmMemorySlice, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 3, 0);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, carried_memory, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
    var memory_after : EvmMemorySlice = carried_memory;
/* EIP-5656 */
    let (base_halt, gas_after_base) = charge(gas_after, G_verylow);
    if base_halt then {
        gas_after = gas_after_base;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let destination_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let source_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let length_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let (copy_halt, gas_after_copy) = charge_copy_gas(gas_after_base, length_word);
    if copy_halt then {
        gas_after = gas_after_copy;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let destination_required = memory_required_size(destination_word, length_word);
    let source_required = memory_required_size(source_word, length_word);
    let required_size =
        if destination_required < source_required then source_required else destination_required;
    let (expansion_halt, gas_after_expansion) = charge_memory_expansion(gas_after_copy, memory_after, required_size);
    if expansion_halt then {
        gas_after = gas_after_expansion;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let destination = memory_access(destination_word, length_word);
    let source = memory_access(source_word, length_word);
    let materialized_required_size =
        if destination.required_size < source.required_size then source.required_size else destination.required_size;
    let mem1 = expand_memory(memory_after, materialized_required_size);
    mem_mcopy(destination.range.off, source.range.off, destination.range.len);
    memory_after = mem1;
    gas_after = gas_after_expansion;
    (gas_after, sp_after, memory_after, Continue())
}

/*! The storage-owner identity carried by the interpreter. Canonical backends
    retain the semantic address; optimized C refines this to the account row
    and its storage range/generation. */
type AccountId = range(0, 2 ^ 32 - 1)
type StorageId = range(0, 2 ^ 32 - 1)
type StorageCount = range(0, 2 ^ 32 - 1)
type StorageGeneration = range(0, 2 ^ 32 - 1)

struct AccountExecutionContext = {
    address : address,
}

function account_execution_context(address : address) -> AccountExecutionContext =
    struct { address = address }

function refresh_account_execution_context(
    context : AccountExecutionContext,
    previous_address : address,
    next_address : address,
) -> AccountExecutionContext =
    if previous_address == next_address then context else account_execution_context(next_address)

/*! Implements `SLOAD`, including warm/cold access gas. */
function execute_sload(context : AccountExecutionContext, carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
/* EIP-2929: cold (2100) vs warm (100) by the slot's accessed-set bit */
    let s = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let warm = k_slot_is_warm(context.address, s);
    let gas_cost = sload_cost(warm);
    let (halt, gas_after_charge) = charge(gas_after, gas_cost);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    k_slot_mark_warm(context.address, s);
    let entry = k_sload(context.address, s);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, entry.curr);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `SSTORE`, including fork-specific gas and refunds. */
val execute_sstore : (AccountExecutionContext, Fork, bool, gas, state_gas, state_gas_spill, gas_refund, StackPointer) ->
    (gas, state_gas, state_gas_spill, gas_refund, StackPointer, OpcodeOutcome)
function execute_sstore(context, fork, carried_is_static, carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp) = {
    let stack_status = guard_stack(carried_sp, 2, 0);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_state_gas, carried_state_spill, carried_refund, carried_sp, stack_status)
    };
    let (_, guard_result) = guard_static(carried_gas, carried_is_static);
    if opcode_failed(guard_result) then {
        return (carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, guard_result)
    };
    if (fork < Amsterdam) & (carried_gas <= G_callstipend) then {
        return (carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, Failed(OutOfGas))
    };
    let s = read_stack_word(carried_sp);
    let sp_after_slot = stack_top_retreat(carried_sp, 1);
    let v = read_stack_word(sp_after_slot);
    let sp_after = stack_top_retreat(sp_after_slot, 1);
    let warm = k_slot_is_warm(context.address, s);
    let cold = not_bool(warm);
    if fork >= Amsterdam then {
        let sentry_cost = sstore_sentry_cost(cold);
        let (sentry_halt, _) = check_execution_gas(carried_gas, sentry_cost);
        if sentry_halt then {
            return (carried_gas, carried_state_gas, carried_state_spill, carried_refund, sp_after, Failed(OutOfGas))
        }
    };
    k_slot_mark_warm(context.address, s);
    let entry = k_sload(context.address, s);
    let costs = sstore_costs(entry.orig, entry.curr, v, cold);
    let (gas_after_refund_credit, state_gas_after_credit, state_spill_after_credit) =
        if costs.state_credit != 0 then
            credit_state_gas_refund(carried_gas, carried_state_gas, carried_state_spill, costs.state_credit)
        else
            (carried_gas, carried_state_gas, carried_state_spill);
    let (execution_halt, gas_after_execution) = charge(gas_after_refund_credit, costs.execution);
    if execution_halt then {
        return (gas_after_execution, state_gas_after_credit, state_spill_after_credit, carried_refund, sp_after, Failed(OutOfGas))
    };
    let (state_halt, gas_after_state_charge, state_gas_after_charge, state_spill_after_charge) =
        charge_state_gas(gas_after_execution, state_gas_after_credit, state_spill_after_credit, costs.state_charge);
    if state_halt then {
        return (gas_after_state_charge, state_gas_after_charge, state_spill_after_charge, carried_refund, sp_after, Failed(OutOfGas))
    };
    let refund_after =
        if not_bool(costs.refund == GAS_REFUND_ZERO) then record_refund(carried_refund, costs.refund)
        else carried_refund;
    if entry.curr != v then {
        k_sstore(context.address, s, struct { curr = v, orig = entry.orig })
    };
    (gas_after_state_charge, state_gas_after_charge, state_spill_after_charge, refund_after, sp_after, Continue())
}

/*! Implements transient-storage load `TLOAD` (EIP-1153). */
function execute_tload(carried_address : address, carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
/* EIP-1153 */
    let (halt, gas_after_charge) = charge(gas_after, G_warm_access);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let s = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let value = k_tload(carried_address, s);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, value);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements transient-storage write `TSTORE` (EIP-1153). */
function execute_tstore(carried_address : address, carried_is_static : bool, carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 0);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
/* EIP-1153 */
    let (gas_after_static_guard, status_after_guard) = guard_static(gas_after, carried_is_static);
    if opcode_failed(status_after_guard) then {
        gas_after = gas_after_static_guard;
        return (gas_after, sp_after, status_after_guard)
    };
    let (halt, gas_after_charge) = charge(gas_after_static_guard, G_warm_access);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let s = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let v = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    k_tstore(carried_address, s, v);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements unconditional `JUMP`. */
function execute_jump(carried_pc : code_pointer, carried_gas : gas, carried_code : Code, carried_sp : StackPointer) -> (code_pointer, gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 0);
    if opcode_failed(stack_status) then {
        return (carried_pc, GAS_ZERO, carried_sp, stack_status)
    };
    var pc_after : code_pointer = carried_pc;
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_mid);
    if halt then {
        gas_after = gas_after_charge;
        return (pc_after, gas_after, sp_after, Failed(OutOfGas))
    };
    let dest = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let (next_pc, g2, jump_status) = do_jump(pc_after, gas_after_charge, carried_code, dest);
    pc_after = next_pc;
    gas_after = g2;
    (pc_after, gas_after, sp_after, jump_status)
}

/*! Implements conditional `JUMPI`. */
function execute_jumpi(carried_pc : code_pointer, carried_gas : gas, carried_code : Code, carried_sp : StackPointer) -> (code_pointer, gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 0);
    if opcode_failed(stack_status) then {
        return (carried_pc, GAS_ZERO, carried_sp, stack_status)
    };
    var pc_after : code_pointer = carried_pc;
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_high);
    if halt then {
        gas_after = gas_after_charge;
        return (pc_after, gas_after, sp_after, Failed(OutOfGas))
    };
    let dest = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let cond = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let condition_is_zero = word_is_zero(cond);
    if condition_is_zero then {
        gas_after = gas_after_charge;
        return (pc_after, gas_after, sp_after, Continue())
    };
    let (next_pc, g2, jump_status) = do_jump(pc_after, gas_after_charge, carried_code, dest);
    pc_after = next_pc;
    gas_after = g2;
    (pc_after, gas_after, sp_after, jump_status)
}

/*! Implements `PC`, returning the current opcode position from the
    carried, already-advanced program counter. */
function execute_pc(carried_pc : code_pointer, carried_gas : gas, carried_sp : StackPointer) -> (code_pointer, gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (carried_pc, GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (carried_pc, gas_after, sp_after, Failed(OutOfGas))
    };
    let next_pc = word_of_byte_count(carried_pc);
    let opcode_pc = alu_sub(next_pc, WORD_ONE);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, opcode_pc);
    gas_after = gas_after_charge;
    (carried_pc, gas_after, sp_after, Continue())
}

/*! Implements `GAS`, returning the carried gas remaining after its own
    charge. */
function execute_gas(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_base);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, word_of_nat_byte_count(gas_after_charge));
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements `JUMPDEST`. */
function execute_jumpdest(carried_gas : gas) -> (gas, OpcodeOutcome) = {
    var gas_after : gas = carried_gas;
let (halt, next_gas) = charge(gas_after, G_jumpdest);
    if halt then {
        gas_after = next_gas;
        return (gas_after, Failed(OutOfGas))
    };
    gas_after = next_gas;
    (gas_after, Continue())
}

/*! Implements the `PUSH0` through `PUSH32` family. */
function execute_push(carried_gas : gas, carried_sp : StackPointer, n : push_width, v : word) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
if n == 0 then {
        let (halt, gas_after_charge) = charge(gas_after, G_base);
        if halt then {
            gas_after = gas_after_charge;
            return (gas_after, sp_after, Failed(OutOfGas))
        };
        sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, v);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
    } else {
        let (halt, gas_after_charge) = charge(gas_after, G_verylow);
        if halt then {
            gas_after = gas_after_charge;
            return (gas_after, sp_after, Failed(OutOfGas))
        };
        sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, v);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
    }
}

/*! Implements the `DUP1` through `DUP16` family. */
function execute_dup(carried_gas : gas, carried_sp : StackPointer, n : stack_operation_index) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, n, n + 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, sp_after, Failed(OutOfGas))
    };
    let value = stack_slot_read(sp_after, n - 1);
    sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, value);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
}

/*! Implements the `SWAP1` through `SWAP16` family. */
function execute_swap(carried_gas : gas, carried_sp : StackPointer, n : stack_operation_index) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, n + 1, n + 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas_after : gas = carried_gas;
let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, carried_sp, Failed(OutOfGas))
    };
    let top_value = read_stack_word(carried_sp);
    let other = stack_slot_read(carried_sp, n);
    stack_set(carried_sp, 0, other);
    stack_set(carried_sp, n, top_value);
    gas_after = gas_after_charge;
    (gas_after, carried_sp, Continue())
}

/*! Implements immediate deep-stack duplication `DUPN`. */
function execute_dupn(carried_gas : gas, carried_sp : StackPointer, immediate : byte) -> (gas, StackPointer, OpcodeOutcome) = {
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
    let valid_immediate = deep_stack_immediate_valid(immediate);
    if not_bool(valid_immediate) then {
        return (gas_after, sp_after, Failed(InvalidOpcode))
    } else {
        let n = decode_single_stack_index(immediate);
        let stack_status = guard_stack(carried_sp, n, n + 1);
        if opcode_failed(stack_status) then {
            return (GAS_ZERO, carried_sp, stack_status)
        };
        let (halt, gas_after_charge) = charge(gas_after, G_verylow);
        if halt then {
            gas_after = gas_after_charge;
            return (gas_after, sp_after, Failed(OutOfGas))
        };
        let value = stack_slot_read(sp_after, n - 1);
        sp_after = stack_top_advance(sp_after, 1);
    write_stack_word(sp_after, value);
    gas_after = gas_after_charge;
    (gas_after, sp_after, Continue())
    }
}

/*! Implements immediate deep-stack exchange `SWAPN`. */
function execute_swapn(carried_gas : gas, carried_sp : StackPointer, immediate : byte) -> (gas, StackPointer, OpcodeOutcome) = {
    var gas_after : gas = carried_gas;
    let valid_immediate = deep_stack_immediate_valid(immediate);
    if not_bool(valid_immediate) then {
        return (gas_after, carried_sp, Failed(InvalidOpcode))
    } else {
        let n = decode_single_stack_index(immediate);
        let stack_status = guard_stack(carried_sp, n + 1, n + 1);
        if opcode_failed(stack_status) then {
            return (GAS_ZERO, carried_sp, stack_status)
        };
        let (halt, gas_after_charge) = charge(gas_after, G_verylow);
        if halt then {
            gas_after = gas_after_charge;
            return (gas_after, carried_sp, Failed(OutOfGas))
        };
        let top_value = read_stack_word(carried_sp);
        let other = stack_slot_read(carried_sp, n);
        stack_set(carried_sp, 0, other);
        stack_set(carried_sp, n, top_value);
        gas_after = gas_after_charge;
        (gas_after, carried_sp, Continue())
    }
}

/*! Implements immediate pairwise deep-stack `EXCHANGE`. */
function execute_exchange(carried_gas : gas, carried_sp : StackPointer, immediate : byte) -> (gas, StackPointer, OpcodeOutcome) = {
    var gas_after : gas = carried_gas;
    let valid_immediate = exchange_immediate_valid(immediate);
    if not_bool(valid_immediate) then {
        return (gas_after, carried_sp, Failed(InvalidOpcode))
    };
    let (n, m) = decode_exchange_stack_indices(immediate);
    let stack_status = guard_stack(carried_sp, m + 1, m + 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    let (halt, gas_after_charge) = charge(gas_after, G_verylow);
    if halt then {
        gas_after = gas_after_charge;
        return (gas_after, carried_sp, Failed(OutOfGas))
    };
    let first = stack_slot_read(carried_sp, n);
    let second = stack_slot_read(carried_sp, m);
    stack_set(carried_sp, n, second);
    stack_set(carried_sp, m, first);
    gas_after = gas_after_charge;
    (gas_after, carried_sp, Continue())
}

/*! Implements the `LOG0` through `LOG4` family. */
function execute_log(carried_address : address, carried_is_static : bool, carried_gas : gas, carried_sp : StackPointer, carried_memory : EvmMemorySlice, n : log_topic_count) -> (gas, StackPointer, EvmMemorySlice, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, n + 2, 0);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, carried_memory, stack_status)
    };
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
    var memory_after : EvmMemorySlice = carried_memory;
let (gas_after_static_guard, status_after_guard) = guard_static(gas_after, carried_is_static);
    if opcode_failed(status_after_guard) then {
        gas_after = gas_after_static_guard;
        return (gas_after, sp_after, memory_after, status_after_guard)
    };
    let offset_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let length_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let (topics, next_sp_1) = pop_log_topics(n, sp_after);
    sp_after = next_sp_1;
    let (log_halt, gas_after_log) = charge_log_gas(gas_after_static_guard, n, length_word);
    if log_halt then {
        gas_after = gas_after_log;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let required_size = memory_required_size(offset_word, length_word);
    let (expansion_halt, gas_after_expansion) = charge_memory_expansion(gas_after_log, memory_after, required_size);
    if expansion_halt then {
        gas_after = gas_after_expansion;
        return (gas_after, sp_after, memory_after, Failed(OutOfGas))
    };
    let access = memory_access(offset_word, length_word);
    let mem1 = expand_memory(memory_after, access.required_size);
    let range = access.range;
    let (data, mem2) = active_memory_slice(mem1, range.off, range.len);
    let memory_slice = evm_memory_slice(data.bytes, data.len);
    let log_data = LogDataMemory(memory_slice);
    k_log(carried_address, topics, log_data);
    memory_after = mem2;
    gas_after = gas_after_expansion;
    (gas_after, sp_after, memory_after, Continue())
}

/*! Implements normal `STOP`. */
function execute_stop() -> FrameStatus = {
    var status_after : FrameStatus = Running();
    let reason = HaltStop();
    status_after = Halted(reason);
    status_after
}

/*! Implements successful `RETURN`. */
function execute_return(carried_gas : gas, carried_sp : StackPointer, carried_memory : EvmMemorySlice) -> (gas, StackPointer, EvmMemorySlice, FrameStatus) = {
    let stack_status = guard_stack(carried_sp, 2, 0);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, carried_memory, opcode_frame_status(stack_status))
    };
    var status_after : FrameStatus = Running();
    var gas_after : gas = carried_gas;
    var sp_after : StackPointer = carried_sp;
    var memory_after : EvmMemorySlice = carried_memory;
let offset_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let length_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let required_size = memory_required_size(offset_word, length_word);
    let (expansion_halt, gas_after_expansion) = charge_memory_expansion(gas_after, memory_after, required_size);
    if expansion_halt then {
        status_after = Exceptional(OutOfGas);
        gas_after = gas_after_expansion;
        return (gas_after, sp_after, memory_after, status_after)
    };
    let access = memory_access(offset_word, length_word);
    let mem1 = expand_memory(memory_after, access.required_size);
    let range = access.range;
    let (data, mem2) = active_memory_slice(mem1, range.off, range.len);
    let output = freeze_output(data);
    let reason = HaltReturn(output);
    status_after = Halted(reason);
    memory_after = mem2;
    gas_after = gas_after_expansion;
    (gas_after, sp_after, memory_after, status_after)
}

/*! Implements rollback-carrying `REVERT` (EIP-140). */
val execute_revert :
    (gas, state_gas, state_gas_spill, state_gas, StackPointer, EvmMemorySlice) ->
    (gas, state_gas, state_gas_spill, StackPointer, EvmMemorySlice, FrameStatus)
function execute_revert(carried_gas, carried_state_gas, carried_state_spill, carried_state_gas_reservoir, carried_sp, carried_memory) = {
    let stack_status = guard_stack(carried_sp, 2, 0);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_state_gas, carried_state_spill, carried_sp, carried_memory, opcode_frame_status(stack_status))
    };
    var status_after : FrameStatus = Running();
    var sp_after : StackPointer = carried_sp;
    var memory_after : EvmMemorySlice = carried_memory;
/* EIP-140 */
    let offset_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let length_word = read_stack_word(sp_after);
    sp_after = stack_top_retreat(sp_after, 1);
    let required_size = memory_required_size(offset_word, length_word);
    let (expansion_halt, gas_after_expansion) = charge_memory_expansion(carried_gas, memory_after, required_size);
    if expansion_halt then {
        status_after = Exceptional(OutOfGas);
        return (gas_after_expansion, carried_state_gas, carried_state_spill, sp_after, memory_after, status_after)
    };
    let access = memory_access(offset_word, length_word);
    let mem1 = expand_memory(memory_after, access.required_size);
    let range = access.range;
    let (g2, state_gas_after, state_spill_after) =
        refill_frame_state_gas(gas_after_expansion, carried_state_gas, carried_state_spill, carried_state_gas_reservoir);
    let (data, mem2) = active_memory_slice(mem1, range.off, range.len);
    let output = freeze_output(data);
    let reason = HaltRevert(output);
    status_after = Halted(reason);
    memory_after = mem2;
    (g2, state_gas_after, state_spill_after, sp_after, memory_after, status_after)
}

/*! Reports invalid-opcode termination to the interpreter's exceptional-halt
    boundary. */
function execute_invalid(carried_gas : gas) -> (gas, OpcodeOutcome) = {
    (carried_gas, Failed(InvalidOpcode))
}

/*! Implements `SELFDESTRUCT` with fork-specific semantics. */
val execute_selfdestruct :
    (address, Fork, bool, gas, state_gas, state_gas_spill, gas_refund, StackPointer) ->
    (gas, state_gas, state_gas_spill, gas_refund, StackPointer, FrameStatus)
function execute_selfdestruct(carried_address, fork, carried_is_static, carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp) = {
    let stack_status = guard_stack(carried_sp, 1, 0);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_state_gas, carried_state_spill, carried_refund, carried_sp, opcode_frame_status(stack_status))
    };
    let (_, guard_result) = guard_static(carried_gas, carried_is_static);
    if opcode_failed(guard_result) then {
        return (carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, opcode_frame_status(guard_result))
    };
    let beneficiary_word = read_stack_word(carried_sp);
    let sp_after = stack_top_retreat(carried_sp, 1);
    let beneficiary = word_to_address(beneficiary_word);
    if fork >= Amsterdam then {
        let warm = k_account_is_warm(beneficiary);
        let access_cost = 0 + G_selfdestruct + (if warm then G_zero else G_amsterdam_cold_account_access);
        let (sentry_halt, sentry_gas) = check_execution_gas(carried_gas, access_cost);
        if sentry_halt then {
            return (sentry_gas, carried_state_gas, carried_state_spill, carried_refund, sp_after, Exceptional(OutOfGas))
        };
        k_account_mark_warm(beneficiary);
        let bal = k_get_balance(carried_address);
        let nonzero_balance = word_nonzero(bal);
        let beneficiary_empty = k_account_is_empty(beneficiary);
        let creates_account = nonzero_balance & beneficiary_empty;
        let execution_cost =
            if creates_account then access_cost + G_amsterdam_account_write else access_cost;
        let (execution_halt, gas_after_execution) = charge(carried_gas, execution_cost);
        if execution_halt then {
            return (gas_after_execution, carried_state_gas, carried_state_spill, carried_refund, sp_after, Exceptional(OutOfGas))
        };
        let (state_halt, gas_after_all_charges, state_gas_after, state_spill_after) =
            if creates_account then
                charge_state_gas(gas_after_execution, carried_state_gas, carried_state_spill, G_amsterdam_state_new_account)
            else
                (false, gas_after_execution, carried_state_gas, carried_state_spill);
        if state_halt then {
            return (gas_after_all_charges, state_gas_after, state_spill_after, carried_refund, sp_after, Exceptional(OutOfGas))
        };
        k_transfer(carried_address, beneficiary, bal);
        let created = k_was_created(carried_address);
        if created then {
            k_selfdestruct(carried_address)
        };
        (gas_after_all_charges, state_gas_after, state_spill_after, carried_refund, sp_after, Halted(HaltSelfDestruct()))
    } else {
        let bal = k_get_balance(carried_address);
        let warm = k_account_is_warm(beneficiary);
        let (selfdestruct_halt, gas_after_execution) = charge(carried_gas, G_selfdestruct);
        if selfdestruct_halt then {
            return (gas_after_execution, carried_state_gas, carried_state_spill, carried_refund, sp_after, Exceptional(OutOfGas))
        };
        let (cold_account_halt, gas_after_access) =
            if not_bool(warm) then charge(gas_after_execution, G_cold_account)
            else (false, gas_after_execution);
        if cold_account_halt then {
            return (gas_after_access, carried_state_gas, carried_state_spill, carried_refund, sp_after, Exceptional(OutOfGas))
        };
        k_account_mark_warm(beneficiary);
        let nonzero_balance = word_nonzero(bal);
        let beneficiary_empty = k_account_is_empty(beneficiary);
        let (new_account_halt, gas_after_all_charges) =
            if nonzero_balance & beneficiary_empty then charge(gas_after_access, G_newaccount)
            else (false, gas_after_access);
        if new_account_halt then {
            return (gas_after_all_charges, carried_state_gas, carried_state_spill, carried_refund, sp_after, Exceptional(OutOfGas))
        };
        let is_selfdestructed = k_is_selfdestructed(carried_address);
        let first_selfdestruct = not_bool(is_selfdestructed);
        let refund_after =
            if (fork < London) & first_selfdestruct then
                record_refund(carried_refund, R_selfdestruct_pre_london)
            else carried_refund;
        k_transfer(carried_address, beneficiary, bal);
        if fork < Cancun then {
            k_zero_balance(carried_address);
            k_selfdestruct(carried_address)
        } else {
            let created = k_was_created(carried_address);
            if created then {
                k_zero_balance(carried_address);
                k_selfdestruct(carried_address)
            }
        };
        (gas_after_all_charges, carried_state_gas, carried_state_spill, refund_after, sp_after, Halted(HaltSelfDestruct()))
    }
}
