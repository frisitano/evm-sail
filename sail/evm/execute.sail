
/*md
# Opcode semantics

The interpreter [interpret][] dispatches each decoded opcode to a handler
that charges its gas, consumes its stack operands, and produces its result
or effect. The handlers are grouped by family (arithmetic, bitwise, keccak,
environment, block, stack/memory, storage, flow, push/dup/swap, log, system).

Handlers follow the state-passing convention (YP μ′ = Ξ(μ)): each takes
only the carried values it uses and returns the same values in the same order. Decoded
instruction data remains explicit where required. The registers behind the
state are read and written only at frame boundaries.

Pure compute is done here; every world effect is a kernel syscall
(`k_*`). All gas and policy stays in the EVM: it marks-and-prices access
via the kernel's returned warm bit (EIP-2929), decides whether an effect
happens, and issues the syscall only for real effects (a no-op `SSTORE`
charges gas but issues no host write). Memory-touching opcodes pay the
quadratic expansion cost via [memory_expansion_gas_cost][] before acting.
Sub-calls and creates delegate to [run_call][] / [run_create][], which
install a child frame and save its parent continuation. The non-recursive
opcode bodies are the `execute_*` functions below.
*/
/*! Converts a non-terminal opcode result into the corresponding frame status. */
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
function do_jump(
    pc_in : code_pointer,
    g : gas,
    frame_code : Code,
    destination_value : word,
) -> (
    (code_pointer, gas, OpcodeOutcome)
) = {
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
) -> (
    OpcodeOutcome
) = {
    let stack_validation = validate_stack(carried_sp, inputs, outputs);
    match stack_validation {
        StackValid => Continue(),
        StackUnderflowFailure => Failed(StackUnderflow),
        StackOverflowFailure => Failed(StackOverflow),
    }
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
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_verylow then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    let result = alu_add(a, b);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements `MUL`. */
function execute_mul(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_low then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_low;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    let result = alu_mul(a, b);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements `SUB`. */
function execute_sub(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_verylow then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    let result = alu_sub(a, b);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements unsigned `DIV`. */
function execute_div(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_low then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_low;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    let result = alu_div(a, b);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements signed `SDIV`. */
function execute_sdiv(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_low then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_low;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    let result = alu_sdiv(a, b);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements unsigned `MOD`. */
function execute_mod(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_low then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_low;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    let result = alu_mod(a, b);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements signed `SMOD`. */
function execute_smod(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_low then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_low;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    let result = alu_smod(a, b);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements `ADDMOD`. */
function execute_addmod(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 3, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_mid then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_mid;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let n = read_stack_word(sp);
    let result = alu_addmod(a, b, n);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements `MULMOD`. */
function execute_mulmod(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 3, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_mid then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_mid;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let n = read_stack_word(sp);
    let result = alu_mulmod(a, b, n);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements `EXP`, including exponent-dependent gas. */
function execute_exp(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let e = read_stack_word(sp);
    let gas_cost = exp_gas(e);
    if carried_gas < gas_cost then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - gas_cost;
    let result = alu_exp(a, e);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements `SIGNEXTEND`. */
function execute_signextend(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_low then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_low;
    let bi = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let v = read_stack_word(sp);
    let result = alu_signextend(bi, v);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements unsigned `LT`. */
function execute_lt(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_verylow then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    let result = alu_lt(a, b);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements unsigned `GT`. */
function execute_gt(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_verylow then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    let result = alu_gt(a, b);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements signed `SLT`. */
function execute_slt(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_verylow then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    let result = alu_slt(a, b);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements signed `SGT`. */
function execute_sgt(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_verylow then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    let result = alu_sgt(a, b);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements `EQ`. */
function execute_eq(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_verylow then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    let result = alu_eq(a, b);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements `ISZERO`. */
function execute_iszero(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    if carried_gas < G_verylow then {
        return (GAS_ZERO, carried_sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let a = read_stack_word(carried_sp);
    let result = alu_iszero(a);
    write_stack_word(carried_sp, result);
    (gas, carried_sp, Continue())
}

/*! Implements bitwise `AND`. */
function execute_and(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_verylow then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    let result = alu_and(a, b);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements bitwise `OR`. */
function execute_or(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_verylow then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    let result = alu_or(a, b);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements bitwise `XOR`. */
function execute_xor(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_verylow then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let a = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let b = read_stack_word(sp);
    let result = alu_xor(a, b);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements bitwise `NOT`. */
function execute_not(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    if carried_gas < G_verylow then {
        return (GAS_ZERO, carried_sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let a = read_stack_word(carried_sp);
    let result = alu_not(a);
    write_stack_word(carried_sp, result);
    (gas, carried_sp, Continue())
}

/*! Implements `BYTE`. */
function execute_byte(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_verylow then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let i = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let x = read_stack_word(sp);
    let result = alu_byte(i, x);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements logical left shift `SHL` (EIP-145). */
function execute_shl(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;

    /* EIP-145 */
    if carried_gas < G_verylow then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let s = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let v = read_stack_word(sp);
    let result = alu_shl(s, v);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements logical right shift `SHR` (EIP-145). */
function execute_shr(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;

    /* EIP-145 */
    if carried_gas < G_verylow then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let s = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let v = read_stack_word(sp);
    let result = alu_shr(s, v);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements arithmetic right shift `SAR` (EIP-145). */
function execute_sar(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;

    /* EIP-145 */
    if carried_gas < G_verylow then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let s = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let v = read_stack_word(sp);
    let result = alu_sar(s, v);
    write_stack_word(sp, result);
    (gas, sp, Continue())
}

/*! Implements count-leading-zeroes `CLZ` (EIP-7939). */
function execute_clz(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };

    /* EIP-7939 */
    if carried_gas < G_low then {
        return (GAS_ZERO, carried_sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_low;
    let x = read_stack_word(carried_sp);
    let result = alu_clz(x);
    write_stack_word(carried_sp, result);
    (gas, carried_sp, Continue())
}

/*! Implements `KECCAK256` over an expanded memory range. */
function execute_keccak256(
    memory_base : memory_base,
    carried_gas : gas,
    carried_sp : StackPointer,
    carried_memory_height : memory_height,
) -> (
    (gas, StackPointer, memory_height, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 2, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, carried_memory_height, stack_status)
    };
    var gas : gas = carried_gas;
    var sp : StackPointer = carried_sp;
    var memory : memory_height = carried_memory_height;
    let offset_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let length_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let keccak_cost = keccak_gas_cost(length_word, gas);
    if not_bool(keccak_cost.affordable) then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, keccak_cost.cost);
    let requested_height = memory_requested_height(offset_word, length_word);
    let expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
    if not_bool(expansion_cost.affordable) then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, expansion_cost.cost);
    let access = memory_access(offset_word, length_word);
    memory = expand_memory(memory_base, memory, access.requested_height);
    let digest = mem_keccak(memory_base, memory, access.range);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, digest);
    (gas, sp, memory, Continue())
}

/*! Implements `ADDRESS`. */
function execute_address(
    carried_address : address,
    carried_gas : gas,
    carried_sp : StackPointer,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let address_word = address_to_word(carried_address);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, address_word);
    (gas, sp, Continue())
}

/*! Implements `ORIGIN`. */
function execute_origin(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let origin = k_env(F_Origin);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, origin);
    (gas, sp, Continue())
}

/*! Implements `CALLER`. */
function execute_caller(
    carried_caller : address,
    carried_gas : gas,
    carried_sp : StackPointer,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let caller = address_to_word(carried_caller);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, caller);
    (gas, sp, Continue())
}

/*! Implements `CALLVALUE`. */
function execute_callvalue(
    carried_value : word,
    carried_gas : gas,
    carried_sp : StackPointer,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, carried_value);
    (gas, sp, Continue())
}

/*! Implements `GASPRICE`. */
function execute_gasprice(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let gas_price = k_env(F_GasPrice);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, gas_price);
    (gas, sp, Continue())
}

/*! Implements `CALLDATASIZE`. */
function execute_calldatasize(
    carried_calldata : CalldataSlice,
    carried_gas : gas,
    carried_sp : StackPointer,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let input = carried_calldata;
    let input_length = region_slice_length(input);
    let length_word = word_of_byte_count(input_length);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, length_word);
    (gas, sp, Continue())
}

/*! Implements `CALLDATALOAD`. */
function execute_calldataload(
    carried_calldata : CalldataSlice,
    carried_gas : gas,
    carried_sp : StackPointer,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_verylow then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let offset_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let value = slice_load_word_offset(carried_calldata, offset_word);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, value);
    (gas, sp, Continue())
}

/*! Implements `CALLDATACOPY`. */
function execute_calldatacopy(
    carried_calldata : CalldataSlice,
    memory_base : memory_base,
    carried_gas : gas,
    carried_sp : StackPointer,
    carried_memory_height : memory_height,
) -> (
    (gas, StackPointer, memory_height, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 3, 0);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, carried_memory_height, stack_status)
    };
    var gas : gas = carried_gas;
    var sp : StackPointer = carried_sp;
    var memory : memory_height = carried_memory_height;
    if gas < G_verylow then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, G_verylow);
    let destination_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let source_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let length_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let copy_cost = copy_gas_cost(length_word, gas);
    if not_bool(copy_cost.affordable) then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, copy_cost.cost);
    let requested_height = memory_requested_height(destination_word, length_word);
    let expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
    if not_bool(expansion_cost.affordable) then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, expansion_cost.cost);
    let access = memory_access(destination_word, length_word);
    memory = expand_memory(memory_base, memory, access.requested_height);
    let range = access.range;
    let destination = memory_absolute(memory_base, range.off);
    slice_copy_word_offset(carried_calldata, destination, source_word, range.len);
    (gas, sp, memory, Continue())
}

/*! Implements `CODESIZE`. */
function execute_codesize(
    carried_code : Code,
    carried_gas : gas,
    carried_sp : StackPointer,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let code_length = frame_code_len(carried_code);
    let length_word = word_of_byte_count(code_length);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, length_word);
    (gas, sp, Continue())
}

/*! Implements `CODECOPY`. */
function execute_codecopy(
    carried_code : Code,
    memory_base : memory_base,
    carried_gas : gas,
    carried_sp : StackPointer,
    carried_memory_height : memory_height,
) -> (
    (gas, StackPointer, memory_height, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 3, 0);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, carried_memory_height, stack_status)
    };
    var gas : gas = carried_gas;
    var sp : StackPointer = carried_sp;
    var memory : memory_height = carried_memory_height;
    if gas < G_verylow then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, G_verylow);
    let destination_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let source_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let length_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let copy_cost = copy_gas_cost(length_word, gas);
    if not_bool(copy_cost.affordable) then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, copy_cost.cost);
    let requested_height = memory_requested_height(destination_word, length_word);
    let expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
    if not_bool(expansion_cost.affordable) then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, expansion_cost.cost);
    let access = memory_access(destination_word, length_word);
    memory = expand_memory(memory_base, memory, access.requested_height);
    let range = access.range;
    let code = carried_code;
    let bytes = code_bytes(code);
    let destination = memory_absolute(memory_base, range.off);
    slice_copy_word_offset(bytes, destination, source_word, range.len);
    (gas, sp, memory, Continue())
}

/*! Implements `BALANCE`, including warm/cold account access. */
function execute_balance(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    let address_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let a = word_to_address(address_word);
    let warm = k_account_is_warm(a);
    let gas_cost = account_cost(warm);
    if carried_gas < gas_cost then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - gas_cost;
    k_account_mark_warm(a);
    let balance = k_get_balance(a);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, balance);
    (gas, sp, Continue())
}

/*! Implements `SELFBALANCE`. */
function execute_selfbalance(
    carried_address : address,
    carried_gas : gas,
    carried_sp : StackPointer,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_low then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_low;
    let balance = k_get_balance(carried_address);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, balance);
    (gas, sp, Continue())
}

/*! Implements `EXTCODESIZE`, including warm/cold account access. */
function execute_extcodesize(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    let address_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let a = word_to_address(address_word);
    let warm = k_account_is_warm(a);
    let access_cost = account_cost(warm);
    let read_cost = external_code_read_cost();
    if carried_gas < access_cost + read_cost then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = gas_sub(carried_gas, access_cost + read_cost);
    k_account_mark_warm(a);
    let code_size = k_get_code_size(a);
    let size_word = word_of_byte_count(code_size);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, size_word);
    (gas, sp, Continue())
}

/*! Implements `EXTCODECOPY`, including access and copy gas. */
function execute_extcodecopy(
    memory_base : memory_base,
    carried_gas : gas,
    carried_sp : StackPointer,
    carried_memory_height : memory_height,
) -> (
    (gas, StackPointer, memory_height, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 4, 0);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, carried_memory_height, stack_status)
    };
    var gas : gas = carried_gas;
    var sp : StackPointer = carried_sp;
    var memory : memory_height = carried_memory_height;
    let address_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let a = word_to_address(address_word);
    let destination_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let source_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let length_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let warm = k_account_is_warm(a);
    let access_cost = account_cost(warm);
    let read_cost = external_code_read_cost();
    if gas < access_cost + read_cost then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, access_cost + read_cost);
    let copy_cost = copy_gas_cost(length_word, gas);
    if not_bool(copy_cost.affordable) then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, copy_cost.cost);
    let requested_height = memory_requested_height(destination_word, length_word);
    let expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
    if not_bool(expansion_cost.affordable) then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, expansion_cost.cost);
    let access = memory_access(destination_word, length_word);
    memory = expand_memory(memory_base, memory, access.requested_height);
    let range = access.range;
    k_account_mark_warm(a);
    let destination = memory_absolute(memory_base, range.off);
    k_code_copy(a, destination, source_word, range.len);
    (gas, sp, memory, Continue())
}

/*! Implements `EXTCODEHASH`, including warm/cold account access. */
function execute_extcodehash(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    let address_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let a = word_to_address(address_word);
    let warm = k_account_is_warm(a);
    let gas_cost = account_cost(warm);
    if carried_gas < gas_cost then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - gas_cost;
    k_account_mark_warm(a);
    let code_hash = k_get_codehash(a);
    let hash_word = hash_to_word(code_hash);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, hash_word);
    (gas, sp, Continue())
}

/*! Implements `RETURNDATASIZE`. */
function execute_returndatasize(
    carried_returndata : OutputSlice,
    carried_gas : gas,
    carried_sp : StackPointer,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let return_data_size = returndata_size(carried_returndata);
    let size_word = word_of_byte_count(return_data_size);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, size_word);
    (gas, sp, Continue())
}

/*! Implements bounds-checked `RETURNDATACOPY`. */
function execute_returndatacopy(
    carried_returndata : OutputSlice,
    memory_base : memory_base,
    carried_gas : gas,
    carried_sp : StackPointer,
    carried_memory_height : memory_height,
) -> (
    (gas, StackPointer, memory_height, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 3, 0);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, carried_memory_height, stack_status)
    };
    var gas : gas = carried_gas;
    var sp : StackPointer = carried_sp;
    var memory : memory_height = carried_memory_height;
    if gas < G_verylow then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, G_verylow);
    let destination_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let source_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let length_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let available = returndata_size(carried_returndata);
    if source_word <= available then {
        let remaining = returndata_remaining(available, source_word);
        if length_word <= remaining then {
            let bounded_length : memory_length = length_word;
            let copy_cost = copy_gas_cost(length_word, gas);
            if not_bool(copy_cost.affordable) then {
                return (GAS_ZERO, sp, memory, Failed(OutOfGas))
            };
            gas = gas_sub(gas, copy_cost.cost);
            let requested_height = memory_requested_height(destination_word, length_word);
            let expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
            if not_bool(expansion_cost.affordable) then {
                return (GAS_ZERO, sp, memory, Failed(OutOfGas))
            };
            gas = gas_sub(gas, expansion_cost.cost);
            let access = memory_access(destination_word, length_word);
            memory = expand_memory(memory_base, memory, access.requested_height);
            let destination = memory_absolute(memory_base, access.range.off);
            let bounded_source_offset : source_pointer = source_word;
            returndata_copy(carried_returndata, destination, bounded_source_offset, bounded_length);
            (gas, sp, memory, Continue())
        } else {
            (GAS_ZERO, sp, memory, Failed(InvalidOpcode))
        }
    } else {
        (GAS_ZERO, sp, memory, Failed(InvalidOpcode))
    }
}

/*! Implements `BLOCKHASH`. */
function execute_blockhash(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < 20 then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - 20;
    let block_number = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let block_hash = k_blockhash(block_number);
    let hash_word = hash_to_word(block_hash);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, hash_word);
    (gas, sp, Continue())
}

/*! Implements `COINBASE`. */
function execute_coinbase(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let coinbase = k_env(F_Coinbase);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, coinbase);
    (gas, sp, Continue())
}

/*! Implements `TIMESTAMP`. */
function execute_timestamp(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let timestamp = k_env(F_Timestamp);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, timestamp);
    (gas, sp, Continue())
}

/*! Implements `NUMBER`. */
function execute_number(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let block_number = k_env(F_Number);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, block_number);
    (gas, sp, Continue())
}

/*! Implements `SLOTNUM`. */
function execute_slotnum(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let slot_number = k_env(F_SlotNumber);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, slot_number);
    (gas, sp, Continue())
}

/*! Implements `PREVRANDAO`. */
function execute_prevrandao(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let prev_randao = k_env(F_PrevRandao);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, prev_randao);
    (gas, sp, Continue())
}

/*! Implements `GASLIMIT`. */
function execute_gaslimit(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let gas_limit = k_env(F_GasLimit);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, gas_limit);
    (gas, sp, Continue())
}

/*! Implements `CHAINID`. */
function execute_chainid(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let chain_id = k_env(F_ChainId);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, chain_id);
    (gas, sp, Continue())
}

/*! Implements `BASEFEE`. */
function execute_basefee(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let base_fee = k_env(F_BaseFee);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, base_fee);
    (gas, sp, Continue())
}

/*! Implements `BLOBBASEFEE`. */
function execute_blobbasefee(
    blob_fee : word,
    carried_gas : gas,
    carried_sp : StackPointer,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, blob_fee);
    (gas, sp, Continue())
}

/*! Implements `BLOBHASH`. */
function execute_blobhash(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_verylow then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let index = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let blob_hash = k_blobhash(index);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, blob_hash);
    (gas, sp, Continue())
}

/*! Implements `POP`. */
function execute_pop(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 1, 0);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let _ = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    (gas, sp, Continue())
}

/*! Implements `MLOAD`. */
function execute_mload(
    memory_base : memory_base,
    carried_gas : gas,
    carried_sp : StackPointer,
    carried_memory_height : memory_height,
) -> (
    (gas, StackPointer, memory_height, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, carried_memory_height, stack_status)
    };
    var gas : gas = carried_gas;
    var sp : StackPointer = carried_sp;
    var memory : memory_height = carried_memory_height;
    if gas < G_verylow then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, G_verylow);
    let offset_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let word_size = u256(32);
    let requested_height = memory_requested_height(offset_word, word_size);
    let expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
    if not_bool(expansion_cost.affordable) then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, expansion_cost.cost);
    let access = memory_access(offset_word, word_size);
    memory = expand_memory(memory_base, memory, access.requested_height);
    let value = mem_load(memory_base, access.range.off);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, value);
    (gas, sp, memory, Continue())
}

/*! Implements `MSTORE`. */
function execute_mstore(
    memory_base : memory_base,
    carried_gas : gas,
    carried_sp : StackPointer,
    carried_memory_height : memory_height,
) -> (
    (gas, StackPointer, memory_height, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 2, 0);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, carried_memory_height, stack_status)
    };
    var gas : gas = carried_gas;
    var sp : StackPointer = carried_sp;
    var memory : memory_height = carried_memory_height;
    if gas < G_verylow then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, G_verylow);
    let offset_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let v = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let word_size = u256(32);
    let requested_height = memory_requested_height(offset_word, word_size);
    let expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
    if not_bool(expansion_cost.affordable) then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, expansion_cost.cost);
    let access = memory_access(offset_word, word_size);
    memory = expand_memory(memory_base, memory, access.requested_height);
    mem_store(memory_base, access.range.off, v);
    (gas, sp, memory, Continue())
}

/*! Implements `MSTORE8`. */
function execute_mstore8(
    memory_base : memory_base,
    carried_gas : gas,
    carried_sp : StackPointer,
    carried_memory_height : memory_height,
) -> (
    (gas, StackPointer, memory_height, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 2, 0);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, carried_memory_height, stack_status)
    };
    var gas : gas = carried_gas;
    var sp : StackPointer = carried_sp;
    var memory : memory_height = carried_memory_height;
    if gas < G_verylow then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, G_verylow);
    let offset_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let v = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let requested_height = memory_requested_height(offset_word, WORD_ONE);
    let expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
    if not_bool(expansion_cost.affordable) then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, expansion_cost.cost);
    let access = memory_access(offset_word, WORD_ONE);
    memory = expand_memory(memory_base, memory, access.requested_height);
    mem_store_byte(memory_base, access.range.off, v);
    (gas, sp, memory, Continue())
}

/*! Implements `MSIZE`. */
function execute_msize(
    carried_gas : gas,
    carried_sp : StackPointer,
    carried_memory_height : memory_height,
) -> (
    (gas, StackPointer, memory_height, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, carried_memory_height, stack_status)
    };
    var sp : StackPointer = carried_sp;
    if carried_gas < G_base then {
        return (GAS_ZERO, sp, carried_memory_height, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let high_water = memory_high_water(carried_memory_height);
    let words = memory_word_count(high_water);
    let size = word_of_nat_byte_count(words * 32);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, size);
    (gas, sp, carried_memory_height, Continue())
}

/*! Implements overlapping memory copy `MCOPY` (EIP-5656). */
function execute_mcopy(
    memory_base : memory_base,
    carried_gas : gas,
    carried_sp : StackPointer,
    carried_memory_height : memory_height,
) -> (
    (gas, StackPointer, memory_height, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 3, 0);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, carried_memory_height, stack_status)
    };
    var gas : gas = carried_gas;
    var sp : StackPointer = carried_sp;
    var memory : memory_height = carried_memory_height;

    /* EIP-5656 */
    if gas < G_verylow then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, G_verylow);
    let destination_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let source_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let length_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let copy_cost = copy_gas_cost(length_word, gas);
    if not_bool(copy_cost.affordable) then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, copy_cost.cost);
    let destination_requested_height = memory_requested_height(destination_word, length_word);
    let source_requested_height = memory_requested_height(source_word, length_word);
    let requested_height =
        if destination_requested_height < source_requested_height
        then source_requested_height
        else destination_requested_height;
    let expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
    if not_bool(expansion_cost.affordable) then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, expansion_cost.cost);
    let destination = memory_access(destination_word, length_word);
    let source = memory_access(source_word, length_word);
    let materialized_required_size =
        if destination.requested_height < source.requested_height
        then source.requested_height
        else destination.requested_height;
    memory = expand_memory(memory_base, memory, materialized_required_size);
    mem_mcopy(memory_base, destination.range.off, source.range.off, destination.range.len);
    (gas, sp, memory, Continue())
}

/*! The storage-owner identity carried by the interpreter. Canonical backends
    retain the semantic address; optimized C refines this to the account row
    and its storage range/generation. */
type AccountId = range(0, 2 ^ 32 - 1)
/*! A stable row identifier in an account's optimized storage table. */
type StorageId = range(0, 2 ^ 32 - 1)
/*! The number of storage rows belonging to an optimized account row. */
type StorageCount = range(0, 2 ^ 32 - 1)
/*! A generation token that invalidates storage rows after an account clear. */
type StorageGeneration = range(0, 2 ^ 32 - 1)

/*! The semantic account identity carried while executing one frame. */
struct AccountExecutionContext = {
    address : address,
}

function account_execution_context(address : address) -> AccountExecutionContext =
    struct { address = address }

/*! Reuses the carried account context when the frame address is unchanged and
    rebuilds it when execution enters a different account. */
function refresh_account_execution_context(
    context : AccountExecutionContext,
    previous_address : address,
    next_address : address,
) -> (
    AccountExecutionContext
) =
    if previous_address == next_address then {
        context
    } else {
        account_execution_context(next_address)
    }

/*! Implements `SLOAD`, including warm/cold access gas. */
function execute_sload(
    context : AccountExecutionContext,
    carried_gas : gas,
    carried_sp : StackPointer,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;

    /* EIP-2929: cold (2100) vs warm (100) by the slot's accessed-set bit */
    let s = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let warm = k_slot_is_warm(context.address, s);
    let gas_cost = sload_cost(warm);
    if carried_gas < gas_cost then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - gas_cost;
    k_slot_mark_warm(context.address, s);
    let entry = k_sload(context.address, s);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, entry.curr);
    (gas, sp, Continue())
}

/*! Implements `SSTORE`, including fork-specific gas and refunds. */
val execute_sstore : (AccountExecutionContext, Fork, bool, gas, state_gas, state_gas_spill, gas_refund, StackPointer) -> (
        gas,
        state_gas,
        state_gas_spill,
        gas_refund,
        StackPointer,
        OpcodeOutcome,
    )
function execute_sstore(
    context,
    fork,
    carried_is_static,
    carried_gas,
    carried_state_gas,
    carried_state_spill,
    carried_refund,
    carried_sp,
) = {
    let stack_status = guard_stack(carried_sp, 2, 0);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_state_gas, carried_state_spill, carried_refund, carried_sp, stack_status)
    };
    var gas : gas = carried_gas;
    var state_gas : state_gas = carried_state_gas;
    var state_spill : state_gas_spill = carried_state_spill;
    var refund : gas_refund = carried_refund;
    var sp : StackPointer = carried_sp;
    var halt : bool = false;
    var status : OpcodeOutcome = Continue();
    (gas, status) = guard_static(gas, carried_is_static);
    if match status {
        Failed(_) => true,
        _ => false,
    } then {
        return (gas, state_gas, state_spill, refund, sp, status)
    };
    if (fork < Amsterdam) & (gas <= G_callstipend) then {
        return (gas, state_gas, state_spill, refund, sp, Failed(OutOfGas))
    };
    let s = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let v = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let warm = k_slot_is_warm(context.address, s);
    let cold = not_bool(warm);
    if fork >= Amsterdam then {
        let sentry_cost = sstore_sentry_cost(cold);
        if gas < sentry_cost then {
            return (GAS_ZERO, state_gas, state_spill, refund, sp, Failed(OutOfGas))
        }
    };
    k_slot_mark_warm(context.address, s);
    let entry = k_sload(context.address, s);
    let costs = sstore_costs(entry.orig, entry.curr, v, cold);
    if costs.state_credit != 0 then {
        (gas, state_gas, state_spill) = credit_state_gas_refund(gas, state_gas, state_spill, costs.state_credit)
    };
    if gas < costs.execution then {
        return (GAS_ZERO, state_gas, state_spill, refund, sp, Failed(OutOfGas))
    };
    gas = gas_sub(gas, costs.execution);
    (halt, gas, state_gas, state_spill) = charge_state_gas(gas, state_gas, state_spill, costs.state_charge);
    if halt then {
        return (gas, state_gas, state_spill, refund, sp, Failed(OutOfGas))
    };
    if not_bool(costs.refund == GAS_REFUND_ZERO) then {
        refund = record_refund(refund, costs.refund)
    };
    if entry.curr != v then {
        k_sstore(context.address, s, struct { curr = v, orig = entry.orig })
    };
    (gas, state_gas, state_spill, refund, sp, Continue())
}

/*! Implements transient-storage load `TLOAD` (EIP-1153). */
function execute_tload(
    carried_address : address,
    carried_gas : gas,
    carried_sp : StackPointer,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var sp : StackPointer = carried_sp;

    /* EIP-1153 */
    if carried_gas < G_warm_access then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_warm_access;
    let s = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let value = k_tload(carried_address, s);
    sp = stack_top_advance(sp, 1);
    write_stack_word(sp, value);
    (gas, sp, Continue())
}

/*! Implements transient-storage write `TSTORE` (EIP-1153). */
function execute_tstore(
    carried_address : address,
    carried_is_static : bool,
    carried_gas : gas,
    carried_sp : StackPointer,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 2, 0);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    var gas : gas = carried_gas;
    var status : OpcodeOutcome = Continue();
    var sp : StackPointer = carried_sp;

    /* EIP-1153 */
    (gas, status) = guard_static(gas, carried_is_static);
    if match status {
        Failed(_) => true,
        _ => false,
    } then {
        return (gas, sp, status)
    };
    if gas < G_warm_access then {
        return (GAS_ZERO, sp, Failed(OutOfGas))
    };
    gas = gas_sub(gas, G_warm_access);
    let s = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let v = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    k_tstore(carried_address, s, v);
    (gas, sp, Continue())
}

/*! Implements unconditional `JUMP`. */
function execute_jump(
    carried_code : Code,
    carried_pc : code_pointer,
    carried_gas : gas,
    carried_sp : StackPointer,
) -> (
    (code_pointer, gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 1, 0);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (carried_pc, GAS_ZERO, carried_sp, stack_status)
    };
    var gas : gas = carried_gas;
    var pc : code_pointer = carried_pc;
    var status : OpcodeOutcome = Continue();
    if gas < G_mid then {
        return (pc, GAS_ZERO, carried_sp, Failed(OutOfGas))
    };
    gas = gas_sub(gas, G_mid);
    let dest = read_stack_word(carried_sp);
    let sp = stack_top_retreat(carried_sp, 1);
    (pc, gas, status) = do_jump(pc, gas, carried_code, dest);
    (pc, gas, sp, status)
}

/*! Implements conditional `JUMPI`. */
function execute_jumpi(
    carried_code : Code,
    carried_pc : code_pointer,
    carried_gas : gas,
    carried_sp : StackPointer,
) -> (
    (code_pointer, gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 2, 0);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (carried_pc, GAS_ZERO, carried_sp, stack_status)
    };
    var gas : gas = carried_gas;
    var pc : code_pointer = carried_pc;
    var status : OpcodeOutcome = Continue();
    var sp : StackPointer = carried_sp;
    if gas < G_high then {
        return (pc, GAS_ZERO, sp, Failed(OutOfGas))
    };
    gas = gas_sub(gas, G_high);
    let dest = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let cond = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let condition_is_zero = word_is_zero(cond);
    if condition_is_zero then {
        return (pc, gas, sp, status)
    };
    (pc, gas, status) = do_jump(pc, gas, carried_code, dest);
    (pc, gas, sp, status)
}

/*! Implements `PC`, returning the current opcode position from the
    carried, already-advanced program counter. */
function execute_pc(
    carried_pc : code_pointer,
    carried_gas : gas,
    carried_sp : StackPointer,
) -> (
    (code_pointer, gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (carried_pc, GAS_ZERO, carried_sp, stack_status)
    };
    if carried_gas < G_base then {
        return (carried_pc, GAS_ZERO, carried_sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let next_pc = word_of_byte_count(carried_pc);
    let opcode_pc = alu_sub(next_pc, WORD_ONE);
    let sp = stack_top_advance(carried_sp, 1);
    write_stack_word(sp, opcode_pc);
    (carried_pc, gas, sp, Continue())
}

/*! Implements `GAS`, returning the carried gas remaining after its own
    charge. */
function execute_gas(carried_gas : gas, carried_sp : StackPointer) -> (gas, StackPointer, OpcodeOutcome) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    if carried_gas < G_base then {
        return (GAS_ZERO, carried_sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_base;
    let gas_word = word_of_nat_byte_count(gas);
    let sp = stack_top_advance(carried_sp, 1);
    write_stack_word(sp, gas_word);
    (gas, sp, Continue())
}

/*! Implements `JUMPDEST`. */
function execute_jumpdest(carried_gas : gas) -> (gas, OpcodeOutcome) = {
    if carried_gas < G_jumpdest then {
        return (GAS_ZERO, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_jumpdest;
    (gas, Continue())
}

/*! Implements the `PUSH0` through `PUSH32` family. */
function execute_push(
    carried_gas : gas,
    carried_sp : StackPointer,
    n : push_width,
    v : word,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, 0, 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    let cost =
        if n == 0 then G_base else G_verylow;
    if carried_gas < cost then {
        return (GAS_ZERO, carried_sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - cost;
    let sp = stack_top_advance(carried_sp, 1);
    write_stack_word(sp, v);
    (gas, sp, Continue())
}

/*! Implements the `DUP1` through `DUP16` family. */
function execute_dup(
    carried_gas : gas,
    carried_sp : StackPointer,
    n : stack_operation_index,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, n, n + 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    if carried_gas < G_verylow then {
        return (GAS_ZERO, carried_sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let value = stack_slot_read(carried_sp, n - 1);
    let sp = stack_top_advance(carried_sp, 1);
    write_stack_word(sp, value);
    (gas, sp, Continue())
}

/*! Implements the `SWAP1` through `SWAP16` family. */
function execute_swap(
    carried_gas : gas,
    carried_sp : StackPointer,
    n : stack_operation_index,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, n + 1, n + 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    if carried_gas < G_verylow then {
        return (GAS_ZERO, carried_sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let top_value = read_stack_word(carried_sp);
    let other = stack_slot_read(carried_sp, n);
    stack_set(carried_sp, 0, other);
    stack_set(carried_sp, n, top_value);
    (gas, carried_sp, Continue())
}

/*! Implements immediate deep-stack duplication `DUPN`. */
function execute_dupn(
    carried_gas : gas,
    carried_sp : StackPointer,
    immediate : byte,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let valid_immediate = deep_stack_immediate_valid(immediate);
    if not_bool(valid_immediate) then {
        return (carried_gas, carried_sp, Failed(InvalidOpcode))
    };
    let n = decode_single_stack_index(immediate);
    let stack_status = guard_stack(carried_sp, n, n + 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    if carried_gas < G_verylow then {
        return (GAS_ZERO, carried_sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let value = stack_slot_read(carried_sp, n - 1);
    let sp = stack_top_advance(carried_sp, 1);
    write_stack_word(sp, value);
    (gas, sp, Continue())
}

/*! Implements immediate deep-stack exchange `SWAPN`. */
function execute_swapn(
    carried_gas : gas,
    carried_sp : StackPointer,
    immediate : byte,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let valid_immediate = deep_stack_immediate_valid(immediate);
    if not_bool(valid_immediate) then {
        return (carried_gas, carried_sp, Failed(InvalidOpcode))
    };
    let n = decode_single_stack_index(immediate);
    let stack_status = guard_stack(carried_sp, n + 1, n + 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    if carried_gas < G_verylow then {
        return (GAS_ZERO, carried_sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let top_value = read_stack_word(carried_sp);
    let other = stack_slot_read(carried_sp, n);
    stack_set(carried_sp, 0, other);
    stack_set(carried_sp, n, top_value);
    (gas, carried_sp, Continue())
}

/*! Implements immediate pairwise deep-stack `EXCHANGE`. */
function execute_exchange(
    carried_gas : gas,
    carried_sp : StackPointer,
    immediate : byte,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    let valid_immediate = exchange_immediate_valid(immediate);
    if not_bool(valid_immediate) then {
        return (carried_gas, carried_sp, Failed(InvalidOpcode))
    };
    let (n, m) = decode_exchange_stack_indices(immediate);
    let stack_status = guard_stack(carried_sp, m + 1, m + 1);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    if carried_gas < G_verylow then {
        return (GAS_ZERO, carried_sp, Failed(OutOfGas))
    };
    let gas : gas = carried_gas - G_verylow;
    let first = stack_slot_read(carried_sp, n);
    let second = stack_slot_read(carried_sp, m);
    stack_set(carried_sp, n, second);
    stack_set(carried_sp, m, first);
    (gas, carried_sp, Continue())
}

/*! Implements the `LOG0` through `LOG4` family. */
function execute_log(
    carried_address : address,
    carried_is_static : bool,
    memory_base : memory_base,
    n : log_topic_count,
    carried_gas : gas,
    carried_sp : StackPointer,
    carried_memory_height : memory_height,
) -> (
    (gas, StackPointer, memory_height, OpcodeOutcome)
) = {
    let stack_status = guard_stack(carried_sp, n + 2, 0);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, carried_memory_height, stack_status)
    };
    var gas : gas = carried_gas;
    var status : OpcodeOutcome = Continue();
    var sp : StackPointer = carried_sp;
    var memory : memory_height = carried_memory_height;
    var topics : LogTopics = LogTopics0();
    (gas, status) = guard_static(gas, carried_is_static);
    if match status {
        Failed(_) => true,
        _ => false,
    } then {
        return (gas, sp, memory, status)
    };
    let offset_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let length_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    (topics, sp) = pop_log_topics(n, sp);
    let log_cost = log_gas_cost(n, length_word, gas);
    if not_bool(log_cost.affordable) then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, log_cost.cost);
    let requested_height = memory_requested_height(offset_word, length_word);
    let expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
    if not_bool(expansion_cost.affordable) then {
        return (GAS_ZERO, sp, memory, Failed(OutOfGas))
    };
    gas = gas_sub(gas, expansion_cost.cost);
    let access = memory_access(offset_word, length_word);
    memory = expand_memory(memory_base, memory, access.requested_height);
    let range = access.range;
    let data = active_memory_slice(memory_base, memory, range.off, range.len);
    let memory_slice = evm_memory_slice(data.bytes, data.len);
    let log_data = LogDataMemory(memory_slice);
    k_log(carried_address, topics, log_data);
    (gas, sp, memory, Continue())
}

/*! Implements normal `STOP`. */
function execute_stop() -> FrameStatus = {
    let reason = HaltStop();
    Halted(reason)
}

/*! Implements successful `RETURN`. */
function execute_return(
    memory_base : memory_base,
    carried_gas : gas,
    carried_sp : StackPointer,
    carried_memory_height : memory_height,
) -> (
    (gas, StackPointer, memory_height, FrameStatus)
) = {
    let stack_status = guard_stack(carried_sp, 2, 0);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (GAS_ZERO, carried_sp, carried_memory_height, opcode_frame_status(stack_status))
    };
    var gas : gas = carried_gas;
    var sp : StackPointer = carried_sp;
    var memory : memory_height = carried_memory_height;
    let offset_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let length_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let requested_height = memory_requested_height(offset_word, length_word);
    let expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
    if not_bool(expansion_cost.affordable) then {
        return (GAS_ZERO, sp, memory, Exceptional(OutOfGas))
    };
    gas = gas_sub(gas, expansion_cost.cost);
    let access = memory_access(offset_word, length_word);
    memory = expand_memory(memory_base, memory, access.requested_height);
    let range = access.range;
    let data = active_memory_slice(memory_base, memory, range.off, range.len);
    let output = freeze_output(data);
    let reason = HaltReturn(output);
    (gas, sp, memory, Halted(reason))
}

/*! Implements rollback-carrying `REVERT` (EIP-140). */
val execute_revert : (state_gas, memory_base, gas, state_gas, state_gas_spill, StackPointer, memory_height) -> (
        gas,
        state_gas,
        state_gas_spill,
        StackPointer,
        memory_height,
        FrameStatus,
    )
function execute_revert(
    carried_state_gas_reservoir,
    memory_base,
    carried_gas,
    carried_state_gas,
    carried_state_spill,
    carried_sp,
    carried_memory_height,
) = {
    let stack_status = guard_stack(carried_sp, 2, 0);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (
            GAS_ZERO,
            carried_state_gas,
            carried_state_spill,
            carried_sp,
            carried_memory_height,
            opcode_frame_status(stack_status),
        )
    };
    var gas : gas = carried_gas;
    var state_gas : state_gas = carried_state_gas;
    var state_spill : state_gas_spill = carried_state_spill;
    var sp : StackPointer = carried_sp;
    var memory : memory_height = carried_memory_height;

    /* EIP-140 */
    let offset_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let length_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let requested_height = memory_requested_height(offset_word, length_word);
    let expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
    if not_bool(expansion_cost.affordable) then {
        return (GAS_ZERO, carried_state_gas, carried_state_spill, sp, memory, Exceptional(OutOfGas))
    };
    gas = gas_sub(gas, expansion_cost.cost);
    let access = memory_access(offset_word, length_word);
    memory = expand_memory(memory_base, memory, access.requested_height);
    let range = access.range;
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if profile.fork >= Amsterdam then {
        gas = conserved_gas_add(gas, state_spill);
        state_gas = carried_state_gas_reservoir;
        state_spill = STATE_GAS_SPILL_ZERO
    };
    let data = active_memory_slice(memory_base, memory, range.off, range.len);
    let output = freeze_output(data);
    let reason = HaltRevert(output);
    (gas, state_gas, state_spill, sp, memory, Halted(reason))
}

/*! Reports invalid-opcode termination to the interpreter's exceptional-halt
    boundary. */
function execute_invalid(carried_gas : gas) -> (gas, OpcodeOutcome) = {
    (carried_gas, Failed(InvalidOpcode))
}

/*! Implements `SELFDESTRUCT` with fork-specific semantics. */
val execute_selfdestruct : (address, Fork, bool, gas, state_gas, state_gas_spill, gas_refund, StackPointer) -> (
        gas,
        state_gas,
        state_gas_spill,
        gas_refund,
        StackPointer,
        FrameStatus,
    )
function execute_selfdestruct(
    carried_address,
    fork,
    carried_is_static,
    carried_gas,
    carried_state_gas,
    carried_state_spill,
    carried_refund,
    carried_sp,
) = {
    let stack_status = guard_stack(carried_sp, 1, 0);
    if match stack_status {
        Failed(_) => true,
        _ => false,
    } then {
        return (
            GAS_ZERO,
            carried_state_gas,
            carried_state_spill,
            carried_refund,
            carried_sp,
            opcode_frame_status(stack_status),
        )
    };
    var gas : gas = carried_gas;
    var state_gas : state_gas = carried_state_gas;
    var state_spill : state_gas_spill = carried_state_spill;
    var refund : gas_refund = carried_refund;
    var sp : StackPointer = carried_sp;
    var halt : bool = false;
    var status : OpcodeOutcome = Continue();
    (gas, status) = guard_static(gas, carried_is_static);
    if match status {
        Failed(_) => true,
        _ => false,
    } then {
        return (gas, state_gas, state_spill, refund, sp, opcode_frame_status(status))
    };
    let beneficiary_word = read_stack_word(sp);
    sp = stack_top_retreat(sp, 1);
    let beneficiary = word_to_address(beneficiary_word);
    let halt_reason = HaltSelfDestruct();
    let halt_status = Halted(halt_reason);
    if fork >= Amsterdam then {
        let warm = k_account_is_warm(beneficiary);
        let cold_access_cost =
            if warm then G_zero else G_amsterdam_cold_account_access;
        let access_cost = G_selfdestruct + cold_access_cost;
        if gas < access_cost then {
            return (GAS_ZERO, state_gas, state_spill, refund, sp, Exceptional(OutOfGas))
        };
        k_account_mark_warm(beneficiary);
        let bal = k_get_balance(carried_address);
        let nonzero_balance = word_nonzero(bal);
        let beneficiary_empty = k_account_is_empty(beneficiary);
        let creates_account = nonzero_balance & beneficiary_empty;
        let execution_cost =
            if creates_account then access_cost + G_amsterdam_account_write else access_cost;
        if gas < execution_cost then {
            return (GAS_ZERO, state_gas, state_spill, refund, sp, Exceptional(OutOfGas))
        };
        gas = gas_sub(gas, execution_cost);
        if creates_account then {
            (halt, gas, state_gas, state_spill) = charge_state_gas(
                gas,
                state_gas,
                state_spill,
                G_amsterdam_state_new_account,
            )
        };
        if halt then {
            return (gas, state_gas, state_spill, refund, sp, Exceptional(OutOfGas))
        };
        k_transfer(carried_address, beneficiary, bal);
        let created = k_was_created(carried_address);
        if created then {
            k_selfdestruct(carried_address)
        };
        (gas, state_gas, state_spill, refund, sp, halt_status)
    } else {
        let bal = k_get_balance(carried_address);
        let warm = k_account_is_warm(beneficiary);
        if gas < G_selfdestruct then {
            return (GAS_ZERO, state_gas, state_spill, refund, sp, Exceptional(OutOfGas))
        };
        gas = gas_sub(gas, G_selfdestruct);
        if not_bool(warm) then {
            if gas < G_cold_account then {
                return (GAS_ZERO, state_gas, state_spill, refund, sp, Exceptional(OutOfGas))
            };
            gas = gas_sub(gas, G_cold_account)
        };
        k_account_mark_warm(beneficiary);
        let nonzero_balance = word_nonzero(bal);
        let beneficiary_empty = k_account_is_empty(beneficiary);
        if nonzero_balance & beneficiary_empty then {
            if gas < G_newaccount then {
                return (GAS_ZERO, state_gas, state_spill, refund, sp, Exceptional(OutOfGas))
            };
            gas = gas_sub(gas, G_newaccount)
        };
        let is_selfdestructed = k_is_selfdestructed(carried_address);
        let first_selfdestruct = not_bool(is_selfdestructed);
        if (fork < London) & first_selfdestruct then {
            refund = record_refund(refund, R_selfdestruct_pre_london)
        };
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
        (gas, state_gas, state_spill, refund, sp, halt_status)
    }
}
