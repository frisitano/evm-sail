
/*md
# Opcode semantics

The single-step transition function of the EVM (Yellow Paper §9): given
the decoded opcode, [execute][] charges its gas, consumes its stack
operands, and produces its result or effect. One match arm per opcode,
grouped by family (arithmetic, bitwise, keccak, environment, block,
stack/memory, storage, flow, push/dup/swap, log, system).

Handlers follow the state-passing convention (YP μ′ = Ξ(μ)): each takes
the machine state it uses — the carried gas, the operand-stack cursor,
the memory cursor for the memory family, and the program counter for the
flow family — by value and returns it updated. The registers behind these
values are read and written only at frame boundaries.

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
/*! The call-family runner (0 = `CALL`, 1 = `CALLCODE`,
    2 = `DELEGATECALL`, 3 = `STATICCALL`); defined with the
    interpreter. */
val run_call : (CallKind, code_pointer, StackTop, EvmMemorySlice, gas) -> (code_pointer, StackTop, EvmMemorySlice, gas)
/*! The contract-creation runner; defined with the interpreter. */
val run_create : (CreateKind, code_pointer, StackTop, EvmMemorySlice, gas) -> (
        code_pointer,
        StackTop,
        EvmMemorySlice,
        gas,
    )

/*md
## Helpers
*/
/*! The storage owner (YP I_a): `SLOAD`, `SSTORE`, `LOG`, and
    `SELFDESTRUCT` act on this account. */
function self_addr() -> address = message.address

/*! EIP-214 write protection: any state-modifying opcode in a
    `STATICCALL` frame halts exceptionally on the carried gas.
    State-changing opcodes call this first; `true` means already
    halted. */
function guard_static(g : gas) -> (bool, gas) =
    if message.is_static then {
        (true, exc_halt(g, WriteProtection))
    } else {
        (false, g)
    }

/*! `JUMP`/`JUMPI` target validity: the destination must be in code range
    and land on a `JUMPDEST` (the precomputed valid-destination set,
    YP §9.4.3); otherwise an invalid-jump exceptional halt. PUSH-data
    bytes that happen to equal `0x5b` are not valid. Returns the next
    program counter and the carried gas. */
function do_jump(pc_in : code_pointer, g : gas, destination_value : word) -> (code_pointer, gas) = {
    let code_length = frame_code_len();
    if destination_value < code_length then {
        let destination : code_pointer = destination_value;
        let valid_destination = frame_jumpdest_valid(destination);
        if valid_destination then {
            (destination, g)
        } else {
            (pc_in, exc_halt(g, InvalidJump))
        }
    } else {
        (pc_in, exc_halt(g, InvalidJump))
    }
}

/*! Pops `count` log topics from the stack into its bounded representation. */
function pop_log_topics(count : log_topic_count, top : StackTop) -> (LogTopics, StackTop) =
    match count {
        0 => (LogTopics0(), top),
        1 => {
            let (t0, top1) = pop(top);
            (LogTopics1(t0), top1)
        },
        2 => {
            let (t0, top1) = pop(top);
            let (t1, top2) = pop(top1);
            (LogTopics2((t0, t1)), top2)
        },
        3 => {
            let (t0, top1) = pop(top);
            let (t1, top2) = pop(top1);
            let (t2, top3) = pop(top2);
            (LogTopics3((t0, t1, t2)), top3)
        },
        4 => {
            let (t0, top1) = pop(top);
            let (t1, top2) = pop(top1);
            let (t2, top3) = pop(top2);
            let (t3, top4) = pop(top3);
            (LogTopics4((t0, t1, t2, t3)), top4)
        },
        _ => (LogTopics0(), top),
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
function execute_add(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let result = alu_add(a, b);
    (push(top2, result), g1)
}

/*! Implements `MUL`. */
function execute_mul(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_low);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let result = alu_mul(a, b);
    (push(top2, result), g1)
}

/*! Implements `SUB`. */
function execute_sub(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let result = alu_sub(a, b);
    (push(top2, result), g1)
}

/*! Implements unsigned `DIV`. */
function execute_div(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_low);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let result = alu_div(a, b);
    (push(top2, result), g1)
}

/*! Implements signed `SDIV`. */
function execute_sdiv(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_low);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let result = alu_sdiv(a, b);
    (push(top2, result), g1)
}

/*! Implements unsigned `MOD`. */
function execute_mod(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_low);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let result = alu_mod(a, b);
    (push(top2, result), g1)
}

/*! Implements signed `SMOD`. */
function execute_smod(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_low);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let result = alu_smod(a, b);
    (push(top2, result), g1)
}

/*! Implements `ADDMOD`. */
function execute_addmod(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_mid);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let (n, top3) = pop(top2);
    let result = alu_addmod(a, b, n);
    (push(top3, result), g1)
}

/*! Implements `MULMOD`. */
function execute_mulmod(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_mid);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let (n, top3) = pop(top2);
    let result = alu_mulmod(a, b, n);
    (push(top3, result), g1)
}

/*! Implements `EXP`, including exponent-dependent gas. */
function execute_exp(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (a, top1) = pop(top);
    let (e, top2) = pop(top1);
    let gas_cost = exp_gas(e);
    let (gas_charged, g1) = charge(g, gas_cost);
    if not_bool(gas_charged) then {
        return (top2, g1)
    };
    let result = alu_exp(a, e);
    (push(top2, result), g1)
}

/*! Implements `SIGNEXTEND`. */
function execute_signextend(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_low);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (bi, top1) = pop(top);
    let (v, top2) = pop(top1);
    let result = alu_signextend(bi, v);
    (push(top2, result), g1)
}

/*! Implements unsigned `LT`. */
function execute_lt(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let result = alu_lt(a, b);
    (push(top2, result), g1)
}

/*! Implements unsigned `GT`. */
function execute_gt(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let result = alu_gt(a, b);
    (push(top2, result), g1)
}

/*! Implements signed `SLT`. */
function execute_slt(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let result = alu_slt(a, b);
    (push(top2, result), g1)
}

/*! Implements signed `SGT`. */
function execute_sgt(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let result = alu_sgt(a, b);
    (push(top2, result), g1)
}

/*! Implements `EQ`. */
function execute_eq(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let result = alu_eq(a, b);
    (push(top2, result), g1)
}

/*! Implements `ISZERO`. */
function execute_iszero(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let result = alu_iszero(a);
    (push(top1, result), g1)
}

/*! Implements bitwise `AND`. */
function execute_and(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let result = alu_and(a, b);
    (push(top2, result), g1)
}

/*! Implements bitwise `OR`. */
function execute_or(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let result = alu_or(a, b);
    (push(top2, result), g1)
}

/*! Implements bitwise `XOR`. */
function execute_xor(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let (b, top2) = pop(top1);
    let result = alu_xor(a, b);
    (push(top2, result), g1)
}

/*! Implements bitwise `NOT`. */
function execute_not(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (a, top1) = pop(top);
    let result = alu_not(a);
    (push(top1, result), g1)
}

/*! Implements `BYTE`. */
function execute_byte(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (i, top1) = pop(top);
    let (x, top2) = pop(top1);
    let result = alu_byte(i, x);
    (push(top2, result), g1)
}

/*! Implements logical left shift `SHL` (EIP-145). */
function execute_shl(top : StackTop, g : gas) -> (StackTop, gas) = {
    /* EIP-145 */
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (s, top1) = pop(top);
    let (v, top2) = pop(top1);
    let result = alu_shl(s, v);
    (push(top2, result), g1)
}

/*! Implements logical right shift `SHR` (EIP-145). */
function execute_shr(top : StackTop, g : gas) -> (StackTop, gas) = {
    /* EIP-145 */
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (s, top1) = pop(top);
    let (v, top2) = pop(top1);
    let result = alu_shr(s, v);
    (push(top2, result), g1)
}

/*! Implements arithmetic right shift `SAR` (EIP-145). */
function execute_sar(top : StackTop, g : gas) -> (StackTop, gas) = {
    /* EIP-145 */
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (s, top1) = pop(top);
    let (v, top2) = pop(top1);
    let result = alu_sar(s, v);
    (push(top2, result), g1)
}

/*! Implements count-leading-zeroes `CLZ` (EIP-7939). */
function execute_clz(top : StackTop, g : gas) -> (StackTop, gas) = {
    /* EIP-7939 */
    let (gas_charged, g1) = charge(g, G_low);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (x, top1) = pop(top);
    let result = alu_clz(x);
    (push(top1, result), g1)
}

/*! Implements `KECCAK256` over an expanded memory range. */
function execute_keccak256(top : StackTop, mem : EvmMemorySlice, g : gas) -> (StackTop, EvmMemorySlice, gas) = {
    let (offset_word, top1) = pop(top);
    let (length_word, top2) = pop(top1);
    let (hashing_gas_charged, g1) = charge_keccak_gas(g, length_word);
    if not_bool(hashing_gas_charged) then {
        return (top2, mem, g1)
    };
    let required_size = memory_required_size(offset_word, length_word);
    let expansion_cost = memory_expansion_cost(mem, required_size);
    let (expansion_gas_charged, g2) = charge(g1, expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return (top2, mem, g2)
    };
    let access = memory_access(offset_word, length_word);
    let mem1 = expand_memory(mem, access.required_size);
    let (digest, mem2) = mem_keccak(mem1, access.range);
    (push(top2, digest), mem2, g2)
}

/*! Implements `ADDRESS`. */
function execute_address(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let address = self_addr();
    let address_word = address_to_word(address);
    (push(top, address_word), g1)
}

/*! Implements `ORIGIN`. */
function execute_origin(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let origin = k_env(F_Origin);
    (push(top, origin), g1)
}

/*! Implements `CALLER`. */
function execute_caller(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let caller = address_to_word(message.caller);
    (push(top, caller), g1)
}

/*! Implements `CALLVALUE`. */
function execute_callvalue(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    (push(top, message.value), g1)
}

/*! Implements `GASPRICE`. */
function execute_gasprice(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let gas_price = k_env(F_GasPrice);
    (push(top, gas_price), g1)
}

/*! Implements `CALLDATASIZE`. */
function execute_calldatasize(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let input = calldata;
    let input_length = region_slice_length(input);
    let length_word = word_of_byte_count(input_length);
    (push(top, length_word), g1)
}

/*! Implements `CALLDATALOAD`. */
function execute_calldataload(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (offset_word, top1) = pop(top);
    let value = slice_load_word_offset(calldata, offset_word);
    (push(top1, value), g1)
}

/*! Implements `CALLDATACOPY`. */
function execute_calldatacopy(top : StackTop, mem : EvmMemorySlice, g : gas) -> (StackTop, EvmMemorySlice, gas) = {
    let (base_gas_charged, g1) = charge(g, G_verylow);
    if not_bool(base_gas_charged) then {
        return (top, mem, g1)
    };
    let (destination_word, top1) = pop(top);
    let (source_word, top2) = pop(top1);
    let (length_word, top3) = pop(top2);
    let (copy_gas_charged, g2) = charge_copy_gas(g1, length_word);
    if not_bool(copy_gas_charged) then {
        return (top3, mem, g2)
    };
    let required_size = memory_required_size(destination_word, length_word);
    let expansion_cost = memory_expansion_cost(mem, required_size);
    let (expansion_gas_charged, g3) = charge(g2, expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return (top3, mem, g3)
    };
    let access = memory_access(destination_word, length_word);
    let mem1 = expand_memory(mem, access.required_size);
    let range = access.range;
    slice_copy_word_offset(calldata, range.off, source_word, range.len);
    (top3, mem1, g3)
}

/*! Implements `CODESIZE`. */
function execute_codesize(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let code_length = frame_code_len();
    let length_word = word_of_byte_count(code_length);
    (push(top, length_word), g1)
}

/*! Implements `CODECOPY`. */
function execute_codecopy(top : StackTop, mem : EvmMemorySlice, g : gas) -> (StackTop, EvmMemorySlice, gas) = {
    let (base_gas_charged, g1) = charge(g, G_verylow);
    if not_bool(base_gas_charged) then {
        return (top, mem, g1)
    };
    let (destination_word, top1) = pop(top);
    let (source_word, top2) = pop(top1);
    let (length_word, top3) = pop(top2);
    let (copy_gas_charged, g2) = charge_copy_gas(g1, length_word);
    if not_bool(copy_gas_charged) then {
        return (top3, mem, g2)
    };
    let required_size = memory_required_size(destination_word, length_word);
    let expansion_cost = memory_expansion_cost(mem, required_size);
    let (expansion_gas_charged, g3) = charge(g2, expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return (top3, mem, g3)
    };
    let access = memory_access(destination_word, length_word);
    let mem1 = expand_memory(mem, access.required_size);
    let range = access.range;
    let code = frame_code;
    let bytes = code_bytes(code);
    slice_copy_word_offset(bytes, range.off, source_word, range.len);
    (top3, mem1, g3)
}

/*! Implements `BALANCE`, including warm/cold account access. */
function execute_balance(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (address_word, top1) = pop(top);
    let a = word_to_address(address_word);
    let warm = k_account_is_warm(a);
    let gas_cost = account_cost(warm);
    let (gas_charged, g1) = charge(g, gas_cost);
    if not_bool(gas_charged) then {
        return (top1, g1)
    };
    k_account_mark_warm(a);
    let balance = k_get_balance(a);
    (push(top1, balance), g1)
}

/*! Implements `SELFBALANCE`. */
function execute_selfbalance(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_low);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let address = self_addr();
    let balance = k_get_balance(address);
    (push(top, balance), g1)
}

/*! Implements `EXTCODESIZE`, including warm/cold account access. */
function execute_extcodesize(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (address_word, top1) = pop(top);
    let a = word_to_address(address_word);
    let warm = k_account_is_warm(a);
    let access_cost = account_cost(warm);
    let read_cost = external_code_read_cost();
    let (gas_charged, g1) = charge(g, access_cost + read_cost);
    if not_bool(gas_charged) then {
        return (top1, g1)
    };
    k_account_mark_warm(a);
    let code_size = k_get_code_size(a);
    let size_word = word_of_byte_count(code_size);
    (push(top1, size_word), g1)
}

/*! Implements `EXTCODECOPY`, including access and copy gas. */
function execute_extcodecopy(top : StackTop, mem : EvmMemorySlice, g : gas) -> (StackTop, EvmMemorySlice, gas) = {
    let (address_word, top1) = pop(top);
    let a = word_to_address(address_word);
    let (destination_word, top2) = pop(top1);
    let (source_word, top3) = pop(top2);
    let (length_word, top4) = pop(top3);
    let warm = k_account_is_warm(a);
    let access_cost = account_cost(warm);
    let read_cost = external_code_read_cost();
    let (access_gas_charged, g1) = charge(g, access_cost + read_cost);
    if not_bool(access_gas_charged) then {
        return (top4, mem, g1)
    };
    let (copy_gas_charged, g2) = charge_copy_gas(g1, length_word);
    if not_bool(copy_gas_charged) then {
        return (top4, mem, g2)
    };
    let required_size = memory_required_size(destination_word, length_word);
    let expansion_cost = memory_expansion_cost(mem, required_size);
    let (expansion_gas_charged, g3) = charge(g2, expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return (top4, mem, g3)
    };
    let access = memory_access(destination_word, length_word);
    let mem1 = expand_memory(mem, access.required_size);
    let range = access.range;
    k_account_mark_warm(a);
    k_code_copy(a, range.off, source_word, range.len);
    (top4, mem1, g3)
}

/*! Implements `EXTCODEHASH`, including warm/cold account access. */
function execute_extcodehash(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (address_word, top1) = pop(top);
    let a = word_to_address(address_word);
    let warm = k_account_is_warm(a);
    let gas_cost = account_cost(warm);
    let (gas_charged, g1) = charge(g, gas_cost);
    if not_bool(gas_charged) then {
        return (top1, g1)
    };
    k_account_mark_warm(a);
    let code_hash = k_get_codehash(a);
    let hash_word = hash_to_word(code_hash);
    (push(top1, hash_word), g1)
}

/*! Implements `RETURNDATASIZE`. */
function execute_returndatasize(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let return_data_size = returndata_size();
    let size_word = word_of_byte_count(return_data_size);
    (push(top, size_word), g1)
}

/*! Implements bounds-checked `RETURNDATACOPY`. */
function execute_returndatacopy(top : StackTop, mem : EvmMemorySlice, g : gas) -> (StackTop, EvmMemorySlice, gas) = {
    let (base_gas_charged, g1) = charge(g, G_verylow);
    if not_bool(base_gas_charged) then {
        return (top, mem, g1)
    };
    let (destination_word, top1) = pop(top);
    let (source_word, top2) = pop(top1);
    let (length_word, top3) = pop(top2);
    let (copy_gas_charged, g2) = charge_copy_gas(g1, length_word);
    if not_bool(copy_gas_charged) then {
        return (top3, mem, g2)
    };
    let required_size = memory_required_size(destination_word, length_word);
    let expansion_cost = memory_expansion_cost(mem, required_size);
    let (expansion_gas_charged, g3) = charge(g2, expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return (top3, mem, g3)
    };
    let access = memory_access(destination_word, length_word);
    let mem1 = expand_memory(mem, access.required_size);
    let range = access.range;
    let g4 = returndata_copy_words(g3, range.off, source_word, length_word);
    (top3, mem1, g4)
}

/*! Implements `BLOCKHASH`. */
function execute_blockhash(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, 20);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (block_number, top1) = pop(top);
    let block_hash = k_blockhash(block_number);
    let hash_word = hash_to_word(block_hash);
    (push(top1, hash_word), g1)
}

/*! Implements `COINBASE`. */
function execute_coinbase(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let coinbase = k_env(F_Coinbase);
    (push(top, coinbase), g1)
}

/*! Implements `TIMESTAMP`. */
function execute_timestamp(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let timestamp = k_env(F_Timestamp);
    (push(top, timestamp), g1)
}

/*! Implements `NUMBER`. */
function execute_number(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let block_number = k_env(F_Number);
    (push(top, block_number), g1)
}

/*! Implements `SLOTNUM`. */
function execute_slotnum(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let slot_number = k_env(F_SlotNumber);
    (push(top, slot_number), g1)
}

/*! Implements `PREVRANDAO`. */
function execute_prevrandao(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let prev_randao = k_env(F_PrevRandao);
    (push(top, prev_randao), g1)
}

/*! Implements `GASLIMIT`. */
function execute_gaslimit(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let gas_limit = k_env(F_GasLimit);
    (push(top, gas_limit), g1)
}

/*! Implements `CHAINID`. */
function execute_chainid(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let chain_id = k_env(F_ChainId);
    (push(top, chain_id), g1)
}

/*! Implements `BASEFEE`. */
function execute_basefee(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let base_fee = k_env(F_BaseFee);
    (push(top, base_fee), g1)
}

/*! Implements `BLOBBASEFEE`. */
function execute_blobbasefee(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let execution_profile = k_execution_profile;
    let base_fee = blob_base_fee(execution_profile.protocol, k_header.excess_blob_gas);
    (push(top, base_fee), g1)
}

/*! Implements `BLOBHASH`. */
function execute_blobhash(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (index, top1) = pop(top);
    let blob_hash = k_blobhash(index);
    (push(top1, blob_hash), g1)
}

/*! Implements `POP`. */
function execute_pop(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (_, top1) = pop(top);
    (top1, g1)
}

/*! Implements `MLOAD`. */
function execute_mload(top : StackTop, mem : EvmMemorySlice, g : gas) -> (StackTop, EvmMemorySlice, gas) = {
    let (base_gas_charged, g1) = charge(g, G_verylow);
    if not_bool(base_gas_charged) then {
        return (top, mem, g1)
    };
    let (offset_word, top1) = pop(top);
    let word_size = u256(32);
    let required_size = memory_required_size(offset_word, word_size);
    let expansion_cost = memory_expansion_cost(mem, required_size);
    let (expansion_gas_charged, g2) = charge(g1, expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return (top1, mem, g2)
    };
    let access = memory_access(offset_word, word_size);
    let mem1 = expand_memory(mem, access.required_size);
    let value = mem_load(access.range.off);
    (push(top1, value), mem1, g2)
}

/*! Implements `MSTORE`. */
function execute_mstore(top : StackTop, mem : EvmMemorySlice, g : gas) -> (StackTop, EvmMemorySlice, gas) = {
    let (base_gas_charged, g1) = charge(g, G_verylow);
    if not_bool(base_gas_charged) then {
        return (top, mem, g1)
    };
    let (offset_word, top1) = pop(top);
    let (v, top2) = pop(top1);
    let word_size = u256(32);
    let required_size = memory_required_size(offset_word, word_size);
    let expansion_cost = memory_expansion_cost(mem, required_size);
    let (expansion_gas_charged, g2) = charge(g1, expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return (top2, mem, g2)
    };
    let access = memory_access(offset_word, word_size);
    let mem1 = expand_memory(mem, access.required_size);
    mem_store(access.range.off, v);
    (top2, mem1, g2)
}

/*! Implements `MSTORE8`. */
function execute_mstore8(top : StackTop, mem : EvmMemorySlice, g : gas) -> (StackTop, EvmMemorySlice, gas) = {
    let (base_gas_charged, g1) = charge(g, G_verylow);
    if not_bool(base_gas_charged) then {
        return (top, mem, g1)
    };
    let (offset_word, top1) = pop(top);
    let (v, top2) = pop(top1);
    let required_size = memory_required_size(offset_word, WORD_ONE);
    let expansion_cost = memory_expansion_cost(mem, required_size);
    let (expansion_gas_charged, g2) = charge(g1, expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return (top2, mem, g2)
    };
    let access = memory_access(offset_word, WORD_ONE);
    let mem1 = expand_memory(mem, access.required_size);
    mem_store_byte(access.range.off, v);
    (top2, mem1, g2)
}

/*! Implements `MSIZE`. */
function execute_msize(top : StackTop, mem : EvmMemorySlice, g : gas) -> (StackTop, EvmMemorySlice, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, mem, g1)
    };
    let high_water = memory_high_water(mem);
    let words = memory_word_count(high_water);
    let size = word_of_nat_byte_count(words * 32);
    (push(top, size), mem, g1)
}

/*! Implements overlapping memory copy `MCOPY` (EIP-5656). */
function execute_mcopy(top : StackTop, mem : EvmMemorySlice, g : gas) -> (StackTop, EvmMemorySlice, gas) = {
    /* EIP-5656 */
    let (base_gas_charged, g1) = charge(g, G_verylow);
    if not_bool(base_gas_charged) then {
        return (top, mem, g1)
    };
    let (destination_word, top1) = pop(top);
    let (source_word, top2) = pop(top1);
    let (length_word, top3) = pop(top2);
    let (copy_gas_charged, g2) = charge_copy_gas(g1, length_word);
    if not_bool(copy_gas_charged) then {
        return (top3, mem, g2)
    };
    let destination_required = memory_required_size(destination_word, length_word);
    let source_required = memory_required_size(source_word, length_word);
    let required_size =
        if destination_required < source_required then source_required else destination_required;
    let expansion_cost = memory_expansion_cost(mem, required_size);
    let (expansion_gas_charged, g3) = charge(g2, expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return (top3, mem, g3)
    };
    let destination = memory_access(destination_word, length_word);
    let source = memory_access(source_word, length_word);
    let materialized_required_size =
        if destination.required_size < source.required_size then source.required_size else destination.required_size;
    let mem1 = expand_memory(mem, materialized_required_size);
    mem_mcopy(destination.range.off, source.range.off, destination.range.len);
    (top3, mem1, g3)
}

/*! Implements `SLOAD`, including warm/cold access gas. */
function execute_sload(top : StackTop, g : gas) -> (StackTop, gas) = {
    /* EIP-2929: cold (2100) vs warm (100) by the slot's accessed-set bit */
    let (s, top1) = pop(top);
    let address = self_addr();
    let warm = k_slot_is_warm(address, s);
    let gas_cost = sload_cost(warm);
    let (gas_charged, g1) = charge(g, gas_cost);
    if not_bool(gas_charged) then {
        return (top1, g1)
    };
    k_slot_mark_warm(address, s);
    let entry = k_sload(address, s);
    (push(top1, entry.curr), g1)
}

/*! Implements `SSTORE`, including fork-specific gas and refunds. */
function execute_sstore(top : StackTop, g : gas) -> (StackTop, gas) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let (write_protected, g0) = guard_static(g);
    if write_protected then {
        return (top, g0)
    };
    if (profile.fork < Amsterdam) & (g0 <= G_callstipend) then {
        return (top, exc_halt(g0, OutOfGas))
    };
    let (s, top1) = pop(top);
    let (v, top2) = pop(top1);
    let address = self_addr();
    let warm = k_slot_is_warm(address, s);
    let cold = not_bool(warm);
    if profile.fork >= Amsterdam then {
        let sentry_cost = sstore_sentry_cost(cold);
        let (sentry_affordable, sentry_gas) = check_execution_gas(g0, sentry_cost);
        if not_bool(sentry_affordable) then {
            return (top2, sentry_gas)
        }
    };
    k_slot_mark_warm(address, s);
    let entry = k_sload(address, s);
    let costs = sstore_costs(entry.orig, entry.curr, v, cold);
    let g1 : gas =
        if costs.state_credit != 0 then credit_state_gas_refund(g0, costs.state_credit) else g0;
    let (execution_gas_charged, g2) = charge(g1, costs.execution);
    if not_bool(execution_gas_charged) then {
        return (top2, g2)
    };
    let (state_gas_charged, g3) = charge_state_gas(g2, costs.state_charge);
    if not_bool(state_gas_charged) then {
        return (top2, g3)
    };
    if not_bool(costs.refund == GAS_REFUND_ZERO) then {
        record_refund(costs.refund)
    };
    if entry.curr != v then {
        k_sstore(address, s, struct { curr = v, orig = entry.orig })
    };
    (top2, g3)
}

/*! Implements transient-storage load `TLOAD` (EIP-1153). */
function execute_tload(top : StackTop, g : gas) -> (StackTop, gas) = {
    /* EIP-1153 */
    let (gas_charged, g1) = charge(g, G_warm_access);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (s, top1) = pop(top);
    let address = self_addr();
    let value = k_tload(address, s);
    (push(top1, value), g1)
}

/*! Implements transient-storage write `TSTORE` (EIP-1153). */
function execute_tstore(top : StackTop, g : gas) -> (StackTop, gas) = {
    /* EIP-1153 */
    let (write_protected, g0) = guard_static(g);
    if write_protected then {
        return (top, g0)
    };
    let (gas_charged, g1) = charge(g0, G_warm_access);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (s, top1) = pop(top);
    let (v, top2) = pop(top1);
    let address = self_addr();
    k_tstore(address, s, v);
    (top2, g1)
}

/*! Implements unconditional `JUMP`. */
function execute_jump(pc_in : code_pointer, top : StackTop, g : gas) -> (code_pointer, StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_mid);
    if not_bool(gas_charged) then {
        return (pc_in, top, g1)
    };
    let (dest, top1) = pop(top);
    let (next_pc, g2) = do_jump(pc_in, g1, dest);
    (next_pc, top1, g2)
}

/*! Implements conditional `JUMPI`. */
function execute_jumpi(pc_in : code_pointer, top : StackTop, g : gas) -> (code_pointer, StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_high);
    if not_bool(gas_charged) then {
        return (pc_in, top, g1)
    };
    let (dest, top1) = pop(top);
    let (cond, top2) = pop(top1);
    let condition_is_zero = word_is_zero(cond);
    if condition_is_zero then {
        return (pc_in, top2, g1)
    };
    let (next_pc, g2) = do_jump(pc_in, g1, dest);
    (next_pc, top2, g2)
}

/*! Implements `PC`, returning the current opcode position from the
    carried, already-advanced program counter. */
function execute_pc(pc_in : code_pointer, top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let next_pc = word_of_byte_count(pc_in);
    let opcode_pc = alu_sub(next_pc, WORD_ONE);
    (push(top, opcode_pc), g1)
}

/*! Implements `GAS`, returning the carried gas remaining after its own
    charge. */
function execute_gas(top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_base);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    (push_gas(top, g1), g1)
}

/*! Implements `JUMPDEST`. */
function execute_jumpdest(g : gas) -> gas = {
    let (_, g1) = charge(g, G_jumpdest);
    g1
}

/*! Implements the `PUSH0` through `PUSH32` family. */
function execute_push(n : push_width, v : word, top : StackTop, g : gas) -> (StackTop, gas) = {
    if n == 0 then {
        let (gas_charged, g1) = charge(g, G_base);
        if not_bool(gas_charged) then {
            return (top, g1)
        };
        (push(top, v), g1)
    } else {
        let (gas_charged, g1) = charge(g, G_verylow);
        if not_bool(gas_charged) then {
            return (top, g1)
        };
        (push(top, v), g1)
    }
}

/*! Implements the `DUP1` through `DUP16` family. */
function execute_dup(n : stack_operation_index, top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let value = peek(top, n - 1);
    (push(top, value), g1)
}

/*! Implements the `SWAP1` through `SWAP16` family. */
function execute_swap(n : stack_operation_index, top : StackTop, g : gas) -> (StackTop, gas) = {
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let top_value = peek(top, 0);
    let other = peek(top, n);
    stack_set(top, 0, other);
    stack_set(top, n, top_value);
    (top, g1)
}

/*! Implements immediate deep-stack duplication `DUPN`. */
function execute_dupn(immediate : byte, top : StackTop, g : gas) -> (StackTop, gas) = {
    let valid_immediate = deep_stack_immediate_valid(immediate);
    if not_bool(valid_immediate) then {
        (top, exc_halt(g, InvalidOpcode))
    } else {
        let (gas_charged, g1) = charge(g, G_verylow);
        if not_bool(gas_charged) then {
            return (top, g1)
        };
        let n = decode_single_stack_index(immediate);
        let value = peek(top, n - 1);
        (push(top, value), g1)
    }
}

/*! Implements immediate deep-stack exchange `SWAPN`. */
function execute_swapn(immediate : byte, top : StackTop, g : gas) -> (StackTop, gas) = {
    let valid_immediate = deep_stack_immediate_valid(immediate);
    if not_bool(valid_immediate) then {
        (top, exc_halt(g, InvalidOpcode))
    } else {
        let (gas_charged, g1) = charge(g, G_verylow);
        if not_bool(gas_charged) then {
            return (top, g1)
        };
        let n = decode_single_stack_index(immediate);
        let top_value = peek(top, 0);
        let other = peek(top, n);
        stack_set(top, 0, other);
        stack_set(top, n, top_value);
        (top, g1)
    }
}

/*! Implements immediate pairwise deep-stack `EXCHANGE`. */
function execute_exchange(immediate : byte, top : StackTop, g : gas) -> (StackTop, gas) = {
    let valid_immediate = exchange_immediate_valid(immediate);
    if not_bool(valid_immediate) then {
        return (top, exc_halt(g, InvalidOpcode))
    };
    let (gas_charged, g1) = charge(g, G_verylow);
    if not_bool(gas_charged) then {
        return (top, g1)
    };
    let (n, m) = decode_exchange_stack_indices(immediate);
    let first = peek(top, n);
    let second = peek(top, m);
    stack_set(top, n, second);
    stack_set(top, m, first);
    (top, g1)
}

/*! Implements the `LOG0` through `LOG4` family. */
function execute_log(
    n : log_topic_count,
    top : StackTop,
    mem : EvmMemorySlice,
    g : gas,
) -> (
    (StackTop, EvmMemorySlice, gas)
) = {
    let (write_protected, g0) = guard_static(g);
    if write_protected then {
        return (top, mem, g0)
    };
    let (offset_word, top1) = pop(top);
    let (length_word, top2) = pop(top1);
    let (topics, top3) = pop_log_topics(n, top2);
    let (log_gas_charged, g1) = charge_log_gas(g0, n, length_word);
    if not_bool(log_gas_charged) then {
        return (top3, mem, g1)
    };
    let required_size = memory_required_size(offset_word, length_word);
    let expansion_cost = memory_expansion_cost(mem, required_size);
    let (expansion_gas_charged, g2) = charge(g1, expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return (top3, mem, g2)
    };
    let access = memory_access(offset_word, length_word);
    let mem1 = expand_memory(mem, access.required_size);
    let range = access.range;
    let (data, mem2) = active_memory_slice(mem1, range.off, range.len);
    let address = self_addr();
    let memory_slice = evm_memory_slice(data.bytes, data.len);
    let log_data = LogDataMemory(memory_slice);
    k_log(address, topics, log_data);
    (top3, mem2, g2)
}

/*! Implements normal `STOP`. */
function execute_stop() -> unit = {
    let reason = HaltStop();
    frame_status = Halted(reason)
}

/*! Implements successful `RETURN`. */
function execute_return(top : StackTop, mem : EvmMemorySlice, g : gas) -> (StackTop, EvmMemorySlice, gas) = {
    let (offset_word, top1) = pop(top);
    let (length_word, top2) = pop(top1);
    let required_size = memory_required_size(offset_word, length_word);
    let expansion_cost = memory_expansion_cost(mem, required_size);
    let (expansion_gas_charged, g1) = charge(g, expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return (top2, mem, g1)
    };
    let access = memory_access(offset_word, length_word);
    let mem1 = expand_memory(mem, access.required_size);
    let range = access.range;
    let (data, mem2) = active_memory_slice(mem1, range.off, range.len);
    let output = freeze_output(data);
    let reason = HaltReturn(output);
    frame_status = Halted(reason);
    (top2, mem2, g1)
}

/*! Implements rollback-carrying `REVERT` (EIP-140). */
function execute_revert(top : StackTop, mem : EvmMemorySlice, g : gas) -> (StackTop, EvmMemorySlice, gas) = {
    /* EIP-140 */
    let (offset_word, top1) = pop(top);
    let (length_word, top2) = pop(top1);
    let required_size = memory_required_size(offset_word, length_word);
    let expansion_cost = memory_expansion_cost(mem, required_size);
    let (expansion_gas_charged, g1) = charge(g, expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return (top2, mem, g1)
    };
    let access = memory_access(offset_word, length_word);
    let mem1 = expand_memory(mem, access.required_size);
    let range = access.range;
    let g2 = refill_frame_state_gas(g1);
    let (data, mem2) = active_memory_slice(mem1, range.off, range.len);
    let output = freeze_output(data);
    let reason = HaltRevert(output);
    frame_status = Halted(reason);
    (top2, mem2, g2)
}

/*! Implements exceptional invalid-opcode termination: all carried gas is
    consumed before the exceptional halt. */
function execute_invalid(g : gas) -> gas = {
    let consumed = gas_sub(g, g);
    exc_halt(consumed, InvalidOpcode)
}

/*! Implements `SELFDESTRUCT` with fork-specific semantics. */
function execute_selfdestruct(top : StackTop, g : gas) -> (StackTop, gas) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let (write_protected, g0) = guard_static(g);
    if write_protected then {
        return (top, g0)
    };
    let (beneficiary_word, top1) = pop(top);
    let beneficiary = word_to_address(beneficiary_word);
    let address = self_addr();
    if profile.fork >= Amsterdam then {
        let warm = k_account_is_warm(beneficiary);
        let access_cost = 0 + G_selfdestruct + (if warm then G_zero else G_amsterdam_cold_account_access);
        let (execution_affordable, sentry_gas) = check_execution_gas(g0, access_cost);
        if not_bool(execution_affordable) then {
            return (top1, sentry_gas)
        };
        k_account_mark_warm(beneficiary);
        let bal = k_get_balance(address);
        let nonzero_balance = word_nonzero(bal);
        let beneficiary_empty = k_account_is_empty(beneficiary);
        let creates_account = nonzero_balance & beneficiary_empty;
        let execution_cost =
            if creates_account then access_cost + G_amsterdam_account_write else access_cost;
        let (execution_gas_charged, g1) = charge(g0, execution_cost);
        if not_bool(execution_gas_charged) then {
            return (top1, g1)
        };
        var g2 : gas = g1;
        if creates_account then {
            let (state_gas_charged, state_gas) = charge_state_gas(g1, G_amsterdam_state_new_account);
            g2 = state_gas;
            if not_bool(state_gas_charged) then {
                return (top1, state_gas)
            }
        };
        k_transfer(address, beneficiary, bal);
        let created = k_was_created(address);
        if created then {
            k_selfdestruct(address)
        };
        let reason = HaltSelfDestruct();
        frame_status = Halted(reason);
        (top1, g2)
    } else {
        let bal = k_get_balance(address);
        let warm = k_account_is_warm(beneficiary);
        let (selfdestruct_gas_charged, g1) = charge(g0, G_selfdestruct);
        if not_bool(selfdestruct_gas_charged) then {
            return (top1, g1)
        };
        var g2 : gas = g1;
        if not_bool(warm) then {
            let (cold_account_gas_charged, cold_gas) = charge(g1, G_cold_account);
            g2 = cold_gas;
            if not_bool(cold_account_gas_charged) then {
                return (top1, cold_gas)
            }
        };
        k_account_mark_warm(beneficiary);
        let nonzero_balance = word_nonzero(bal);
        let beneficiary_empty = k_account_is_empty(beneficiary);
        var g3 : gas = g2;
        if nonzero_balance & beneficiary_empty then {
            let (new_account_gas_charged, new_account_gas) = charge(g2, G_newaccount);
            g3 = new_account_gas;
            if not_bool(new_account_gas_charged) then {
                return (top1, new_account_gas)
            }
        };
        let is_selfdestructed = k_is_selfdestructed(address);
        let first_selfdestruct = not_bool(is_selfdestructed);
        if (profile.fork < London) & first_selfdestruct then {
            record_refund(R_selfdestruct_pre_london)
        };
        k_transfer(address, beneficiary, bal);
        if profile.fork < Cancun then {
            k_zero_balance(address);
            k_selfdestruct(address)
        } else {
            let created = k_was_created(address);
            if created then {
                k_zero_balance(address);
                k_selfdestruct(address)
            }
        };
        let reason = HaltSelfDestruct();
        frame_status = Halted(reason);
        (top1, g3)
    }
}

/*! Implements `CREATE` through the common create runner. */
function execute_create(
    pc_in : code_pointer,
    top : StackTop,
    mem : EvmMemorySlice,
    g : gas,
) -> (
    (code_pointer, StackTop, EvmMemorySlice, gas)
) =
    run_create(CreateByNonce, pc_in, top, mem, g)
/*! Implements `CREATE2` through the common create runner. */
function execute_create2(
    pc_in : code_pointer,
    top : StackTop,
    mem : EvmMemorySlice,
    g : gas,
) -> (
    (code_pointer, StackTop, EvmMemorySlice, gas)
) =
    run_create(CreateBySalt, pc_in, top, mem, g)
/*! Implements `CALL` through the common call runner. */
function execute_call(
    pc_in : code_pointer,
    top : StackTop,
    mem : EvmMemorySlice,
    g : gas,
) -> (
    (code_pointer, StackTop, EvmMemorySlice, gas)
) =
    run_call(Call, pc_in, top, mem, g)
/*! Implements `CALLCODE` through the common call runner. */
function execute_callcode(
    pc_in : code_pointer,
    top : StackTop,
    mem : EvmMemorySlice,
    g : gas,
) -> (
    (code_pointer, StackTop, EvmMemorySlice, gas)
) =
    run_call(CallCode, pc_in, top, mem, g)
/*! Implements `DELEGATECALL` through the common call runner. */
function execute_delegatecall(
    pc_in : code_pointer,
    top : StackTop,
    mem : EvmMemorySlice,
    g : gas,
) -> (
    (code_pointer, StackTop, EvmMemorySlice, gas)
) =
    run_call(DelegateCall, pc_in, top, mem, g)
/*! Implements `STATICCALL` through the common call runner. */
function execute_staticcall(
    pc_in : code_pointer,
    top : StackTop,
    mem : EvmMemorySlice,
    g : gas,
) -> (
    (code_pointer, StackTop, EvmMemorySlice, gas)
) =
    run_call(StaticCall, pc_in, top, mem, g)

/*! The Yellow Paper stack effect `(delta, alpha)` for every decoded
    instruction. Invalid deep-stack immediates have no stack effect because
    their opcode body raises `InvalidOpcode` before accessing the stack. */
function opcode_stack_effect(op : ast) -> (operand_stack_height, operand_stack_height) = match op {
    STOP() => (0, 0),
    ADD() => (2, 1),
    MUL() => (2, 1),
    SUB() => (2, 1),
    DIV() => (2, 1),
    SDIV() => (2, 1),
    MOD() => (2, 1),
    SMOD() => (2, 1),
    ADDMOD() => (3, 1),
    MULMOD() => (3, 1),
    EXP() => (2, 1),
    SIGNEXTEND() => (2, 1),
    LT() => (2, 1),
    GT() => (2, 1),
    SLT() => (2, 1),
    SGT() => (2, 1),
    EQ() => (2, 1),
    ISZERO() => (1, 1),
    AND() => (2, 1),
    OR() => (2, 1),
    XOR() => (2, 1),
    NOT() => (1, 1),
    BYTE() => (2, 1),
    SHL() => (2, 1),
    SHR() => (2, 1),
    SAR() => (2, 1),
    CLZ() => (1, 1),
    KECCAK256() => (2, 1),
    ADDRESS() => (0, 1),
    BALANCE() => (1, 1),
    ORIGIN() => (0, 1),
    CALLER() => (0, 1),
    CALLVALUE() => (0, 1),
    CALLDATALOAD() => (1, 1),
    CALLDATASIZE() => (0, 1),
    CALLDATACOPY() => (3, 0),
    CODESIZE() => (0, 1),
    CODECOPY() => (3, 0),
    GASPRICE() => (0, 1),
    EXTCODESIZE() => (1, 1),
    EXTCODECOPY() => (4, 0),
    RETURNDATASIZE() => (0, 1),
    RETURNDATACOPY() => (3, 0),
    EXTCODEHASH() => (1, 1),
    BLOCKHASH() => (1, 1),
    COINBASE() => (0, 1),
    TIMESTAMP() => (0, 1),
    NUMBER() => (0, 1),
    PREVRANDAO() => (0, 1),
    GASLIMIT() => (0, 1),
    CHAINID() => (0, 1),
    SELFBALANCE() => (0, 1),
    BASEFEE() => (0, 1),
    BLOBHASH() => (1, 1),
    BLOBBASEFEE() => (0, 1),
    SLOTNUM() => (0, 1),
    POP() => (1, 0),
    MLOAD() => (1, 1),
    MSTORE() => (2, 0),
    MSTORE8() => (2, 0),
    SLOAD() => (1, 1),
    SSTORE() => (2, 0),
    JUMP() => (1, 0),
    JUMPI() => (2, 0),
    PC() => (0, 1),
    MSIZE() => (0, 1),
    GAS() => (0, 1),
    JUMPDEST() => (0, 0),
    TLOAD() => (1, 1),
    TSTORE() => (2, 0),
    MCOPY() => (3, 0),
    PUSH(_, _) => (0, 1),
    DUP(n) => (n, n + 1),
    SWAP(n) => (n + 1, n + 1),
    DUPN(immediate) => {
        let valid_immediate = deep_stack_immediate_valid(immediate);
        if valid_immediate then {
            let n = decode_single_stack_index(immediate);
            (n, n + 1)
        } else {
            (0, 0)
        }
    },
    SWAPN(immediate) => {
        let valid_immediate = deep_stack_immediate_valid(immediate);
        if valid_immediate then {
            let n = decode_single_stack_index(immediate);
            (n + 1, n + 1)
        } else {
            (0, 0)
        }
    },
    EXCHANGE(immediate) => {
        let valid_immediate = exchange_immediate_valid(immediate);
        if valid_immediate then {
            let (_, higher) = decode_exchange_stack_indices(immediate);
            (higher + 1, higher + 1)
        } else {
            (0, 0)
        }
    },
    LOG(n) => (n + 2, 0),
    opcode_CREATE() => (3, 1),
    CALL() => (7, 1),
    CALLCODE() => (7, 1),
    RETURN() => (2, 0),
    DELEGATECALL() => (6, 1),
    CREATE2() => (4, 1),
    STATICCALL() => (6, 1),
    REVERT() => (2, 0),
    INVALID() => (0, 0),
    SELFDESTRUCT() => (1, 0),
}

/*! Standard decoded-opcode dispatcher. Each arm invokes the same narrow
    semantic function used directly by the optimized raw-byte interpreter,
    threading exactly the machine state that family carries. */
function execute_opcode(
    op : ast,
    pc_in : code_pointer,
    top : StackTop,
    mem : EvmMemorySlice,
    g : gas,
) -> (
    (code_pointer, StackTop, EvmMemorySlice, gas)
) =
    match op {
        STOP() => {
            execute_stop();
            (pc_in, top, mem, g)
        },
        /* ---- arithmetic (pure) ---- */
        ADD() => {
            let (top1, g1) = execute_add(top, g);
            (pc_in, top1, mem, g1)
        },
        MUL() => {
            let (top1, g1) = execute_mul(top, g);
            (pc_in, top1, mem, g1)
        },
        SUB() => {
            let (top1, g1) = execute_sub(top, g);
            (pc_in, top1, mem, g1)
        },
        DIV() => {
            let (top1, g1) = execute_div(top, g);
            (pc_in, top1, mem, g1)
        },
        SDIV() => {
            let (top1, g1) = execute_sdiv(top, g);
            (pc_in, top1, mem, g1)
        },
        MOD() => {
            let (top1, g1) = execute_mod(top, g);
            (pc_in, top1, mem, g1)
        },
        SMOD() => {
            let (top1, g1) = execute_smod(top, g);
            (pc_in, top1, mem, g1)
        },
        ADDMOD() => {
            let (top1, g1) = execute_addmod(top, g);
            (pc_in, top1, mem, g1)
        },
        MULMOD() => {
            let (top1, g1) = execute_mulmod(top, g);
            (pc_in, top1, mem, g1)
        },
        EXP() => {
            let (top1, g1) = execute_exp(top, g);
            (pc_in, top1, mem, g1)
        },
        SIGNEXTEND() => {
            let (top1, g1) = execute_signextend(top, g);
            (pc_in, top1, mem, g1)
        },
        /* ---- comparison / bitwise (pure) ---- */
        LT() => {
            let (top1, g1) = execute_lt(top, g);
            (pc_in, top1, mem, g1)
        },
        GT() => {
            let (top1, g1) = execute_gt(top, g);
            (pc_in, top1, mem, g1)
        },
        SLT() => {
            let (top1, g1) = execute_slt(top, g);
            (pc_in, top1, mem, g1)
        },
        SGT() => {
            let (top1, g1) = execute_sgt(top, g);
            (pc_in, top1, mem, g1)
        },
        EQ() => {
            let (top1, g1) = execute_eq(top, g);
            (pc_in, top1, mem, g1)
        },
        ISZERO() => {
            let (top1, g1) = execute_iszero(top, g);
            (pc_in, top1, mem, g1)
        },
        AND() => {
            let (top1, g1) = execute_and(top, g);
            (pc_in, top1, mem, g1)
        },
        OR() => {
            let (top1, g1) = execute_or(top, g);
            (pc_in, top1, mem, g1)
        },
        XOR() => {
            let (top1, g1) = execute_xor(top, g);
            (pc_in, top1, mem, g1)
        },
        NOT() => {
            let (top1, g1) = execute_not(top, g);
            (pc_in, top1, mem, g1)
        },
        BYTE() => {
            let (top1, g1) = execute_byte(top, g);
            (pc_in, top1, mem, g1)
        },
        SHL() => {
            let (top1, g1) = execute_shl(top, g);
            (pc_in, top1, mem, g1)
        },
        SHR() => {
            let (top1, g1) = execute_shr(top, g);
            (pc_in, top1, mem, g1)
        },
        SAR() => {
            let (top1, g1) = execute_sar(top, g);
            (pc_in, top1, mem, g1)
        },
        CLZ() => {
            let (top1, g1) = execute_clz(top, g);
            (pc_in, top1, mem, g1)
        },
        /* ---- keccak: hash via kernel primitive ---- */
        KECCAK256() => {
            let (top1, mem1, g1) = execute_keccak256(top, mem, g);
            (pc_in, top1, mem1, g1)
        },
        /* ---- environment / account (kernel reads, EVM prices them) ---- */
        ADDRESS() => {
            let (top1, g1) = execute_address(top, g);
            (pc_in, top1, mem, g1)
        },
        ORIGIN() => {
            let (top1, g1) = execute_origin(top, g);
            (pc_in, top1, mem, g1)
        },
        CALLER() => {
            let (top1, g1) = execute_caller(top, g);
            (pc_in, top1, mem, g1)
        },
        CALLVALUE() => {
            let (top1, g1) = execute_callvalue(top, g);
            (pc_in, top1, mem, g1)
        },
        GASPRICE() => {
            let (top1, g1) = execute_gasprice(top, g);
            (pc_in, top1, mem, g1)
        },
        CALLDATASIZE() => {
            let (top1, g1) = execute_calldatasize(top, g);
            (pc_in, top1, mem, g1)
        },
        CALLDATALOAD() => {
            let (top1, g1) = execute_calldataload(top, g);
            (pc_in, top1, mem, g1)
        },
        CALLDATACOPY() => {
            let (top1, mem1, g1) = execute_calldatacopy(top, mem, g);
            (pc_in, top1, mem1, g1)
        },
        CODESIZE() => {
            let (top1, g1) = execute_codesize(top, g);
            (pc_in, top1, mem, g1)
        },
        CODECOPY() => {
            let (top1, mem1, g1) = execute_codecopy(top, mem, g);
            (pc_in, top1, mem1, g1)
        },
        BALANCE() => {
            let (top1, g1) = execute_balance(top, g);
            (pc_in, top1, mem, g1)
        },
        SELFBALANCE() => {
            let (top1, g1) = execute_selfbalance(top, g);
            (pc_in, top1, mem, g1)
        },
        EXTCODESIZE() => {
            let (top1, g1) = execute_extcodesize(top, g);
            (pc_in, top1, mem, g1)
        },
        EXTCODECOPY() => {
            let (top1, mem1, g1) = execute_extcodecopy(top, mem, g);
            (pc_in, top1, mem1, g1)
        },
        EXTCODEHASH() => {
            let (top1, g1) = execute_extcodehash(top, g);
            (pc_in, top1, mem, g1)
        },
        RETURNDATASIZE() => {
            let (top1, g1) = execute_returndatasize(top, g);
            (pc_in, top1, mem, g1)
        },
        RETURNDATACOPY() => {
            let (top1, mem1, g1) = execute_returndatacopy(top, mem, g);
            (pc_in, top1, mem1, g1)
        },
        /* ---- block (kernel reads) ---- */
        BLOCKHASH() => {
            let (top1, g1) = execute_blockhash(top, g);
            (pc_in, top1, mem, g1)
        },
        COINBASE() => {
            let (top1, g1) = execute_coinbase(top, g);
            (pc_in, top1, mem, g1)
        },
        TIMESTAMP() => {
            let (top1, g1) = execute_timestamp(top, g);
            (pc_in, top1, mem, g1)
        },
        NUMBER() => {
            let (top1, g1) = execute_number(top, g);
            (pc_in, top1, mem, g1)
        },
        SLOTNUM() => {
            let (top1, g1) = execute_slotnum(top, g);
            (pc_in, top1, mem, g1)
        },
        PREVRANDAO() => {
            let (top1, g1) = execute_prevrandao(top, g);
            (pc_in, top1, mem, g1)
        },
        GASLIMIT() => {
            let (top1, g1) = execute_gaslimit(top, g);
            (pc_in, top1, mem, g1)
        },
        CHAINID() => {
            let (top1, g1) = execute_chainid(top, g);
            (pc_in, top1, mem, g1)
        },
        BASEFEE() => {
            let (top1, g1) = execute_basefee(top, g);
            (pc_in, top1, mem, g1)
        },
        BLOBBASEFEE() => {
            let (top1, g1) = execute_blobbasefee(top, g);
            (pc_in, top1, mem, g1)
        },
        BLOBHASH() => {
            let (top1, g1) = execute_blobhash(top, g);
            (pc_in, top1, mem, g1)
        },
        /* ---- stack / memory (pure, user-space) ---- */
        POP() => {
            let (top1, g1) = execute_pop(top, g);
            (pc_in, top1, mem, g1)
        },
        MLOAD() => {
            let (top1, mem1, g1) = execute_mload(top, mem, g);
            (pc_in, top1, mem1, g1)
        },
        MSTORE() => {
            let (top1, mem1, g1) = execute_mstore(top, mem, g);
            (pc_in, top1, mem1, g1)
        },
        MSTORE8() => {
            let (top1, mem1, g1) = execute_mstore8(top, mem, g);
            (pc_in, top1, mem1, g1)
        },
        MSIZE() => {
            let (top1, mem1, g1) = execute_msize(top, mem, g);
            (pc_in, top1, mem1, g1)
        },
        MCOPY() => {
            let (top1, mem1, g1) = execute_mcopy(top, mem, g);
            (pc_in, top1, mem1, g1)
        },
        /* ---- storage: gas/decision in EVM, effect in kernel ---- */
        SLOAD() => {
            let (top1, g1) = execute_sload(top, g);
            (pc_in, top1, mem, g1)
        },
        SSTORE() => {
            let (top1, g1) = execute_sstore(top, g);
            (pc_in, top1, mem, g1)
        },
        TLOAD() => {
            let (top1, g1) = execute_tload(top, g);
            (pc_in, top1, mem, g1)
        },
        TSTORE() => {
            let (top1, g1) = execute_tstore(top, g);
            (pc_in, top1, mem, g1)
        },
        /* ---- flow (pure over own code) ---- */
        JUMP() => {
            let (pc1, top1, g1) = execute_jump(pc_in, top, g);
            (pc1, top1, mem, g1)
        },
        JUMPI() => {
            let (pc1, top1, g1) = execute_jumpi(pc_in, top, g);
            (pc1, top1, mem, g1)
        },
        PC() => {
            let (top1, g1) = execute_pc(pc_in, top, g);
            (pc_in, top1, mem, g1)
        },
        GAS() => {
            let (top1, g1) = execute_gas(top, g);
            (pc_in, top1, mem, g1)
        },
        JUMPDEST() => (pc_in, top, mem, execute_jumpdest(g)),
        /* ---- push / dup / swap (pure) ---- */
        PUSH(n, v) => {
            let (top1, g1) = execute_push(n, v, top, g);
            (pc_in, top1, mem, g1)
        },
        DUP(n) => {
            let (top1, g1) = execute_dup(n, top, g);
            (pc_in, top1, mem, g1)
        },
        SWAP(n) => {
            let (top1, g1) = execute_swap(n, top, g);
            (pc_in, top1, mem, g1)
        },
        DUPN(immediate) => {
            let (top1, g1) = execute_dupn(immediate, top, g);
            (pc_in, top1, mem, g1)
        },
        SWAPN(immediate) => {
            let (top1, g1) = execute_swapn(immediate, top, g);
            (pc_in, top1, mem, g1)
        },
        EXCHANGE(immediate) => {
            let (top1, g1) = execute_exchange(immediate, top, g);
            (pc_in, top1, mem, g1)
        },
        /* ---- log: EVM checks/prices, kernel records ---- */
        LOG(n) => {
            let (top1, mem1, g1) = execute_log(n, top, mem, g);
            (pc_in, top1, mem1, g1)
        },
        /* ---- system ---- */
        opcode_CREATE() => execute_create(pc_in, top, mem, g),
        CREATE2() => execute_create2(pc_in, top, mem, g),
        CALL() => execute_call(pc_in, top, mem, g),
        CALLCODE() => execute_callcode(pc_in, top, mem, g),
        DELEGATECALL() => execute_delegatecall(pc_in, top, mem, g),
        STATICCALL() => execute_staticcall(pc_in, top, mem, g),
        RETURN() => {
            let (top1, mem1, g1) = execute_return(top, mem, g);
            (pc_in, top1, mem1, g1)
        },
        REVERT() => {
            let (top1, mem1, g1) = execute_revert(top, mem, g);
            (pc_in, top1, mem1, g1)
        },
        INVALID() => (pc_in, top, mem, execute_invalid(g)),
        SELFDESTRUCT() => {
            let (top1, g1) = execute_selfdestruct(top, g);
            (pc_in, top1, mem, g1)
        },
    }

/*! The single-step transition function over the standard decoded AST. Stack
    validity is established before gas charging or any instruction side
    effect, as part of the Yellow Paper exceptional-halt predicate. */
function execute(
    op : ast,
    pc_in : code_pointer,
    top : StackTop,
    mem : EvmMemorySlice,
    g : gas,
) -> (
    (code_pointer, StackTop, EvmMemorySlice, gas)
) = {
    let (inputs, outputs) = opcode_stack_effect(op);
    let (stack_valid, g1) = validate_stack(g, top, inputs, outputs);
    if stack_valid then {
        execute_opcode(op, pc_in, top, mem, g1)
    } else {
        (pc_in, top, mem, g1)
    }
}
