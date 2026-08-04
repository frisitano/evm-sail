
/*md
# Opcode semantics

The single-step transition function of the EVM (Yellow Paper §9): given
the decoded opcode, [execute][] charges its gas, consumes its stack
operands, and produces its result or effect. One match arm per opcode,
grouped by family (arithmetic, bitwise, keccak, environment, block,
stack/memory, storage, flow, push/dup/swap, log, system).

Pure compute is done here; every world effect is a kernel syscall
(`k_*`). All gas and policy stays in the EVM: it marks-and-prices access
via the kernel's returned warm bit (EIP-2929), decides whether an effect
happens, and issues the syscall only for real effects (a no-op `SSTORE`
charges gas but issues no host write). Memory-touching opcodes pay the
quadratic expansion cost via [charge_memory_range][] before acting.
Sub-calls and creates delegate to [run_call][] / [run_create][], which
install a child frame and save its parent continuation. The non-recursive
opcode bodies live in [execute_opcode][].
*/
/*! The call-family runner (0 = `CALL`, 1 = `CALLCODE`,
    2 = `DELEGATECALL`, 3 = `STATICCALL`); defined with the
    interpreter. */
val run_call : CallKind -> unit
/*! The contract-creation runner; defined with the interpreter. */
val run_create : CreateKind -> unit

/*md
## Helpers
*/
/*! The storage owner (YP I_a): `SLOAD`, `SSTORE`, `LOG`, and
    `SELFDESTRUCT` act on this account. */
function self_addr() -> address = message.address

/*! EIP-214 write protection: any state-modifying opcode in a
    `STATICCALL` frame halts exceptionally. State-changing opcodes call
    this first; `true` means already halted. */
function guard_static() -> bool =
    if message.is_static then {
        exc_halt(WriteProtection);
        true
    } else {
        false
    }

/*! `JUMP`/`JUMPI` target validity: the destination must be in code range
    and land on a `JUMPDEST` (the precomputed valid-destination set,
    YP §9.4.3); otherwise an invalid-jump exceptional halt. PUSH-data
    bytes that happen to equal `0x5b` are not valid. */
function do_jump(destination_value : word) -> unit = {
    let code_length = frame_code_len();
    if destination_value < code_length then {
        let destination : code_pointer = destination_value;
        if frame_jumpdest_valid(destination) then {
            pc = destination
        } else {
            exc_halt(InvalidJump)
        }
    } else {
        exc_halt(InvalidJump)
    }
}

/*! Pops `count` log topics from the stack into its bounded representation. */
function pop_log_topics(count : log_topic_count) -> LogTopics =
    match count {
        0 => LogTopics0(),
        1 => LogTopics1(pop()),
        2 => {
            let t0 = pop();
            let t1 = pop();
            LogTopics2((t0, t1))
        },
        3 => {
            let t0 = pop();
            let t1 = pop();
            let t2 = pop();
            LogTopics3((t0, t1, t2))
        },
        4 => {
            let t0 = pop();
            let t1 = pop();
            let t2 = pop();
            let t3 = pop();
            LogTopics4((t0, t1, t2, t3))
        },
        _ => LogTopics0(),
    }

/*md
## The opcode bodies

Each semantic opcode constructor has a named body. The standard decoder
validates the instruction's stack effect before dispatching its `ast` to these
functions below. Optimized C applies the same validation at its raw-byte
dispatch boundary before calling a generated body, without materializing an
`ast`.
*/
/*! Implements `ADD`. */
function execute_add() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let a = pop();
    let b = pop();
    push(alu_add(a, b))
}

/*! Implements `MUL`. */
function execute_mul() -> unit = {
    if not_bool(charge(G_low)) then { return () };
    let a = pop();
    let b = pop();
    push(alu_mul(a, b))
}

/*! Implements `SUB`. */
function execute_sub() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let a = pop();
    let b = pop();
    push(alu_sub(a, b))
}

/*! Implements unsigned `DIV`. */
function execute_div() -> unit = {
    if not_bool(charge(G_low)) then { return () };
    let a = pop();
    let b = pop();
    push(alu_div(a, b))
}

/*! Implements signed `SDIV`. */
function execute_sdiv() -> unit = {
    if not_bool(charge(G_low)) then { return () };
    let a = pop();
    let b = pop();
    push(alu_sdiv(a, b))
}

/*! Implements unsigned `MOD`. */
function execute_mod() -> unit = {
    if not_bool(charge(G_low)) then { return () };
    let a = pop();
    let b = pop();
    push(alu_mod(a, b))
}

/*! Implements signed `SMOD`. */
function execute_smod() -> unit = {
    if not_bool(charge(G_low)) then { return () };
    let a = pop();
    let b = pop();
    push(alu_smod(a, b))
}

/*! Implements `ADDMOD`. */
function execute_addmod() -> unit = {
    if not_bool(charge(G_mid)) then { return () };
    let a = pop();
    let b = pop();
    let n = pop();
    push(alu_addmod(a, b, n))
}

/*! Implements `MULMOD`. */
function execute_mulmod() -> unit = {
    if not_bool(charge(G_mid)) then { return () };
    let a = pop();
    let b = pop();
    let n = pop();
    push(alu_mulmod(a, b, n))
}

/*! Implements `EXP`, including exponent-dependent gas. */
function execute_exp() -> unit = {
    let a = pop();
    let e = pop();
    if not_bool(charge(exp_gas(e))) then { return () };
    push(alu_exp(a, e))
}

/*! Implements `SIGNEXTEND`. */
function execute_signextend() -> unit = {
    if not_bool(charge(G_low)) then { return () };
    let bi = pop();
    let v = pop();
    push(alu_signextend(bi, v))
}

/*! Implements unsigned `LT`. */
function execute_lt() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let a = pop();
    let b = pop();
    push(alu_lt(a, b))
}

/*! Implements unsigned `GT`. */
function execute_gt() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let a = pop();
    let b = pop();
    push(alu_gt(a, b))
}

/*! Implements signed `SLT`. */
function execute_slt() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let a = pop();
    let b = pop();
    push(alu_slt(a, b))
}

/*! Implements signed `SGT`. */
function execute_sgt() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let a = pop();
    let b = pop();
    push(alu_sgt(a, b))
}

/*! Implements `EQ`. */
function execute_eq() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let a = pop();
    let b = pop();
    push(alu_eq(a, b))
}

/*! Implements `ISZERO`. */
function execute_iszero() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let a = pop();
    push(alu_iszero(a))
}

/*! Implements bitwise `AND`. */
function execute_and() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let a = pop();
    let b = pop();
    push(alu_and(a, b))
}

/*! Implements bitwise `OR`. */
function execute_or() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let a = pop();
    let b = pop();
    push(alu_or(a, b))
}

/*! Implements bitwise `XOR`. */
function execute_xor() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let a = pop();
    let b = pop();
    push(alu_xor(a, b))
}

/*! Implements bitwise `NOT`. */
function execute_not() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let a = pop();
    push(alu_not(a))
}

/*! Implements `BYTE`. */
function execute_byte() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let i = pop();
    let x = pop();
    push(alu_byte(i, x))
}

/*! Implements logical left shift `SHL` (EIP-145). */
function execute_shl() -> unit = {
    /* EIP-145 */
    if not_bool(charge(G_verylow)) then { return () };
    let s = pop();
    let v = pop();
    push(alu_shl(s, v))
}

/*! Implements logical right shift `SHR` (EIP-145). */
function execute_shr() -> unit = {
    /* EIP-145 */
    if not_bool(charge(G_verylow)) then { return () };
    let s = pop();
    let v = pop();
    push(alu_shr(s, v))
}

/*! Implements arithmetic right shift `SAR` (EIP-145). */
function execute_sar() -> unit = {
    /* EIP-145 */
    if not_bool(charge(G_verylow)) then { return () };
    let s = pop();
    let v = pop();
    push(alu_sar(s, v))
}

/*! Implements count-leading-zeroes `CLZ` (EIP-7939). */
function execute_clz() -> unit = {
    /* EIP-7939 */
    if not_bool(charge(G_low)) then { return () };
    let x = pop();
    push(alu_clz(x))
}

/*! Implements `KECCAK256` over an expanded memory range. */
function execute_keccak256() -> unit = {
    let offset_word = pop();
    let length_word = pop();
    charge_keccak_gas(length_word);
    let range = charge_memory_range(offset_word, length_word);
    if is_running() then {
        push(mem_keccak(range))
    }
}

/*! Implements `ADDRESS`. */
function execute_address() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(address_to_word(self_addr()))
}

/*! Implements `ORIGIN`. */
function execute_origin() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(k_env(F_Origin))
}

/*! Implements `CALLER`. */
function execute_caller() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(address_to_word(message.caller))
}

/*! Implements `CALLVALUE`. */
function execute_callvalue() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(message.value)
}

/*! Implements `GASPRICE`. */
function execute_gasprice() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(k_env(F_GasPrice))
}

/*! Implements `CALLDATASIZE`. */
function execute_calldatasize() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    let input = calldata;
    push(word_of_byte_count(region_slice_length(input)))
}

/*! Implements `CALLDATALOAD`. */
function execute_calldataload() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let offset_word = pop();
    if is_running() then {
        push(slice_load_word_offset(calldata, offset_word))
    }
}

/*! Implements `CALLDATACOPY`. */
function execute_calldatacopy() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let destination_word = pop();
    let source_word = pop();
    let length_word = pop();
    charge_copy_gas(length_word);
    let range = charge_memory_range(destination_word, length_word);
    if is_running() then {
        slice_copy_word_offset(calldata, range.off, source_word, range.len)
    }
}

/*! Implements `CODESIZE`. */
function execute_codesize() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(word_of_byte_count(frame_code_len()))
}

/*! Implements `CODECOPY`. */
function execute_codecopy() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let destination_word = pop();
    let source_word = pop();
    let length_word = pop();
    charge_copy_gas(length_word);
    let range = charge_memory_range(destination_word, length_word);
    if is_running() then {
        let code = frame_code;
        slice_copy_word_offset(code_bytes(code), range.off, source_word, range.len)
    }
}

/*! Implements `BALANCE`, including warm/cold account access. */
function execute_balance() -> unit = {
    let a = word_to_address(pop());
    let warm = k_account_is_warm(a);
    if not_bool(charge(account_cost(warm))) then { return () };
    k_account_mark_warm(a);
    push(k_get_balance(a))
}

/*! Implements `SELFBALANCE`. */
function execute_selfbalance() -> unit = {
    if not_bool(charge(G_low)) then { return () };
    push(k_get_balance(self_addr()))
}

/*! Implements `EXTCODESIZE`, including warm/cold account access. */
function execute_extcodesize() -> unit = {
    let a = word_to_address(pop());
    let warm = k_account_is_warm(a);
    if not_bool(charge(account_cost(warm) + external_code_read_cost())) then { return () };
    k_account_mark_warm(a);
    push(word_of_byte_count(k_get_code_size(a)))
}

/*! Implements `EXTCODECOPY`, including access and copy gas. */
function execute_extcodecopy() -> unit = {
    let a = word_to_address(pop());
    let destination_word = pop();
    let source_word = pop();
    let length_word = pop();
    let warm = k_account_is_warm(a);
    if not_bool(charge(account_cost(warm) + external_code_read_cost())) then { return () };
    charge_copy_gas(length_word);
    let range = charge_memory_range(destination_word, length_word);
    if is_running() then {
        k_account_mark_warm(a);
        k_code_copy(a, range.off, source_word, range.len)
    }
}

/*! Implements `EXTCODEHASH`, including warm/cold account access. */
function execute_extcodehash() -> unit = {
    let a = word_to_address(pop());
    let warm = k_account_is_warm(a);
    if not_bool(charge(account_cost(warm))) then { return () };
    k_account_mark_warm(a);
    push(hash_to_word(k_get_codehash(a)))
}

/*! Implements `RETURNDATASIZE`. */
function execute_returndatasize() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(word_of_byte_count(returndata_size()))
}

/*! Implements bounds-checked `RETURNDATACOPY`. */
function execute_returndatacopy() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let destination_word = pop();
    let source_word = pop();
    let length_word = pop();
    charge_copy_gas(length_word);
    let range = charge_memory_range(destination_word, length_word);
    if is_running() then {
        returndata_copy_words(range.off, source_word, length_word)
    }
}

/*! Implements `BLOCKHASH`. */
function execute_blockhash() -> unit = {
    if not_bool(charge(20)) then { return () };
    push(hash_to_word(k_blockhash(pop())))
}

/*! Implements `COINBASE`. */
function execute_coinbase() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(k_env(F_Coinbase))
}

/*! Implements `TIMESTAMP`. */
function execute_timestamp() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(k_env(F_Timestamp))
}

/*! Implements `NUMBER`. */
function execute_number() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(k_env(F_Number))
}

/*! Implements `SLOTNUM`. */
function execute_slotnum() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(k_env(F_SlotNumber))
}

/*! Implements `PREVRANDAO`. */
function execute_prevrandao() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(k_env(F_PrevRandao))
}

/*! Implements `GASLIMIT`. */
function execute_gaslimit() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(k_env(F_GasLimit))
}

/*! Implements `CHAINID`. */
function execute_chainid() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(k_env(F_ChainId))
}

/*! Implements `BASEFEE`. */
function execute_basefee() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(k_env(F_BaseFee))
}

/*! Implements `BLOBBASEFEE`. */
function execute_blobbasefee() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    let execution_profile = k_execution_profile;
    push(blob_base_fee(execution_profile.protocol, k_header.excess_blob_gas))
}

/*! Implements `BLOBHASH`. */
function execute_blobhash() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    push(k_blobhash(pop()))
}

/*! Implements `POP`. */
function execute_pop() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    let _ = pop()
}

/*! Implements `MLOAD`. */
function execute_mload() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let offset_word = pop();
    let range = charge_memory_range(offset_word, u256(32));
    if is_running() then {
        push(mem_load(range.off))
    }
}

/*! Implements `MSTORE`. */
function execute_mstore() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let offset_word = pop();
    let v = pop();
    let range = charge_memory_range(offset_word, u256(32));
    if is_running() then {
        mem_store(range.off, v)
    }
}

/*! Implements `MSTORE8`. */
function execute_mstore8() -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let offset_word = pop();
    let v = pop();
    let range = charge_memory_range(offset_word, WORD_ONE);
    if is_running() then {
        mem_store_byte(range.off, v)
    }
}

/*! Implements `MSIZE`. */
function execute_msize() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(word_of_nat_byte_count(memory_word_count(evm_memory_high_water()) * 32))
}

/*! Implements overlapping memory copy `MCOPY` (EIP-5656). */
function execute_mcopy() -> unit = {
    /* EIP-5656 */
    if not_bool(charge(G_verylow)) then { return () };
    let destination_word = pop();
    let source_word = pop();
    let length_word = pop();
    charge_copy_gas(length_word);
    if is_running() then {
        let available = gas_remaining;
        let destination = memory_access(destination_word, length_word);
        let source = memory_access(source_word, length_word);
        let required_size =
            if destination.required_size < source.required_size then {
                source.required_size
            } else {
                destination.required_size
            };
        let expansion = memory_expansion(required_size, available);
        if not_bool(charge(expansion.cost)) then { return () };
        apply_memory_expansion(expansion);
        mem_mcopy(
            destination.range.off,
            source.range.off,
            destination.range.len,
        )
    }
}

/*! Implements `SLOAD`, including warm/cold access gas. */
function execute_sload() -> unit = {
    /* EIP-2929: cold (2100) vs warm (100) by the slot's accessed-set bit */
    let s = pop();
    let warm = k_slot_is_warm(self_addr(), s);
    if not_bool(charge(sload_cost(warm))) then { return () };
    k_slot_mark_warm(self_addr(), s);
    push(k_sload(self_addr(), s).curr)
}

/*! Implements `SSTORE`, including fork-specific gas and refunds. */
function execute_sstore() -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if guard_static() then {
        ()
    } else {
        if (profile.fork < Amsterdam) & (gas_remaining <= G_callstipend) then {
            exc_halt(OutOfGas)
        } else {
            let s = pop();
            let v = pop();
            if is_running() then {
                let warm = k_slot_is_warm(self_addr(), s);
                let cold = not_bool(warm);
                if profile.fork >= Amsterdam then {
                    let access_cost = amsterdam_storage_access_cost(cold);
                    let sentry_cost = if access_cost < G_sstore_sentry then { G_sstore_sentry } else { access_cost };
                    check_execution_gas(sentry_cost)
                };
                if not_bool(is_running()) then {
                    return ()
                };
                k_slot_mark_warm(self_addr(), s);
                let entry = k_sload(self_addr(), s);
                let costs = sstore_costs(entry.orig, entry.curr, v, cold);
                if costs.state_credit != 0 then {
                    credit_state_gas_refund(costs.state_credit)
                };
                if not_bool(charge(costs.execution)) then { return () };
                charge_state_gas(costs.state_charge);
                if not_bool(costs.refund == GAS_REFUND_ZERO) then {
                    record_refund(costs.refund)
                };
                if is_running() & (entry.curr != v) then {
                    k_sstore(self_addr(), s, struct { curr = v, orig = entry.orig })
                }
            }
        }
    }
}

/*! Implements transient-storage load `TLOAD` (EIP-1153). */
function execute_tload() -> unit = {
    /* EIP-1153 */
    if not_bool(charge(G_warm_access)) then { return () };
    let s = pop();
    push(k_tload(self_addr(), s))
}

/*! Implements transient-storage write `TSTORE` (EIP-1153). */
function execute_tstore() -> unit = {
    /* EIP-1153 */
    if guard_static() then {
        ()
    } else {
        if not_bool(charge(G_warm_access)) then { return () };
        let s = pop();
        let v = pop();
        k_tstore(self_addr(), s, v)
    }
}

/*! Implements unconditional `JUMP`. */
function execute_jump() -> unit = {
    if not_bool(charge(G_mid)) then { return () };
    let dest = pop();
    do_jump(dest)
}

/*! Implements conditional `JUMPI`. */
function execute_jumpi() -> unit = {
    if not_bool(charge(G_high)) then { return () };
    let dest = pop();
    let cond = pop();
    if word_is_zero(cond) then {
        ()
    } else {
        do_jump(dest)
    }
}

/*! Implements `PC`, returning the current opcode position. */
function execute_pc() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push(alu_sub(word_of_byte_count(pc), WORD_ONE))
}

/*! Implements `GAS`, returning gas remaining after its own charge. */
function execute_gas() -> unit = {
    if not_bool(charge(G_base)) then { return () };
    push_gas(gas_remaining)
}

/*! Implements `JUMPDEST`. */
function execute_jumpdest() -> unit = {
    if not_bool(charge(G_jumpdest)) then { return () }
}

/*! Implements the `PUSH0` through `PUSH32` family. */
function execute_push(n : push_width, v : word) -> unit = {
    if n == 0 then {
        if not_bool(charge(G_base)) then { return () }
    } else {
        if not_bool(charge(G_verylow)) then { return () }
    };
    push(v)
}

/*! Implements the `DUP1` through `DUP16` family. */
function execute_dup(n : stack_operation_index) -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    push(peek(n - 1))
}

/*! Implements the `SWAP1` through `SWAP16` family. */
function execute_swap(n : stack_operation_index) -> unit = {
    if not_bool(charge(G_verylow)) then { return () };
    let top = peek(0);
    let other = peek(n);
    stack_set(0, other);
    stack_set(n, top)
}

/*! Implements immediate deep-stack duplication `DUPN`. */
function execute_dupn(immediate : byte) -> unit = {
    if not_bool(deep_stack_immediate_valid(immediate)) then {
        exc_halt(InvalidOpcode)
    } else {
        if not_bool(charge(G_verylow)) then { return () };
        let n = decode_single_stack_index(immediate);
        push(peek(n - 1))
    }
}

/*! Implements immediate deep-stack exchange `SWAPN`. */
function execute_swapn(immediate : byte) -> unit = {
    if not_bool(deep_stack_immediate_valid(immediate)) then {
        exc_halt(InvalidOpcode)
    } else {
        if not_bool(charge(G_verylow)) then { return () };
        let n = decode_single_stack_index(immediate);
        let top = peek(0);
        let other = peek(n);
        stack_set(0, other);
        stack_set(n, top)
    }
}

/*! Implements immediate pairwise deep-stack `EXCHANGE`. */
function execute_exchange(immediate : byte) -> unit = {
    if not_bool(exchange_immediate_valid(immediate)) then {
        exc_halt(InvalidOpcode)
    } else {
        if not_bool(charge(G_verylow)) then { return () };
        let (n, m) = decode_exchange_stack_indices(immediate);
        let first = peek(n);
        let second = peek(m);
        stack_set(n, second);
        stack_set(m, first)
    }
}

/*! Implements the `LOG0` through `LOG4` family. */
function execute_log(n : log_topic_count) -> unit = {
    if guard_static() then {
        ()
    } else {
        let offset_word = pop();
        let length_word = pop();
        let topics = pop_log_topics(n);
        charge_log_gas(n, length_word);
        let range = charge_memory_range(offset_word, length_word);
        if is_running() then {
            let data = active_memory_slice(range.off, range.len);
            k_log(self_addr(), topics, LogDataMemory(evm_memory_slice(data.bytes, data.len)))
        }
    }
}

/*! Implements normal `STOP`. */
function execute_stop() -> unit = frame_status = Halted(HaltStop())

/*! Implements successful `RETURN`. */
function execute_return() -> unit = {
    let offset_word = pop();
    let length_word = pop();
    let range = charge_memory_range(offset_word, length_word);
    if is_running() then {
        frame_status = Halted(HaltReturn(freeze_output(active_memory_slice(range.off, range.len))))
    }
}

/*! Implements rollback-carrying `REVERT` (EIP-140). */
function execute_revert() -> unit = {
    /* EIP-140 */
    let offset_word = pop();
    let length_word = pop();
    let range = charge_memory_range(offset_word, length_word);
    if is_running() then {
        refill_frame_state_gas();
        frame_status = Halted(HaltRevert(freeze_output(active_memory_slice(range.off, range.len))))
    }
}

/*! Implements exceptional invalid-opcode termination. */
function execute_invalid() -> unit = {
    gas_remaining = GAS_ZERO;
    exc_halt(InvalidOpcode)
}

/*! Implements `SELFDESTRUCT` with fork-specific semantics. */
function execute_selfdestruct() -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if guard_static() then {
        ()
    } else {
        let beneficiary = word_to_address(pop());
        if profile.fork >= Amsterdam then {
            let warm = k_account_is_warm(beneficiary);
            let access_cost = 0 + G_selfdestruct + (if warm then { G_zero } else { G_amsterdam_cold_account_access });
            check_execution_gas(access_cost);
            if is_running() then {
                k_account_mark_warm(beneficiary);
                let bal = k_get_balance(self_addr());
                let creates_account = word_nonzero(bal) & k_account_is_empty(beneficiary);
                let execution_cost = if creates_account then {
                        access_cost + G_amsterdam_account_write
                    } else {
                        access_cost
                    };
                if not_bool(charge(execution_cost)) then { return () };
                if creates_account then {
                    charge_state_gas(G_amsterdam_state_new_account)
                };
                if is_running() then {
                    k_transfer(self_addr(), beneficiary, bal);
                    if k_was_created(self_addr()) then {
                        k_selfdestruct(self_addr())
                    };
                    frame_status = Halted(HaltSelfDestruct())
                }
            }
        } else {
            let bal = k_get_balance(self_addr());
            let warm = k_account_is_warm(beneficiary);
            if not_bool(charge(G_selfdestruct)) then { return () };
            if not_bool(warm) then {
                if not_bool(charge(G_cold_account)) then { return () }
            };
            k_account_mark_warm(beneficiary);
            if word_nonzero(bal) & k_account_is_empty(beneficiary) then {
                if not_bool(charge(G_newaccount)) then { return () }
            };
            if is_running() then {
                let first_selfdestruct = not_bool(k_is_selfdestructed(self_addr()));
                if (profile.fork < London) & first_selfdestruct then {
                    record_refund(R_selfdestruct_pre_london)
                };
                k_transfer(self_addr(), beneficiary, bal);
                if profile.fork < Cancun then {
                    k_zero_balance(self_addr());
                    k_selfdestruct(self_addr())
                } else {
                    if k_was_created(self_addr()) then {
                        k_zero_balance(self_addr());
                        k_selfdestruct(self_addr())
                    }
                };
                frame_status = Halted(HaltSelfDestruct())
            }
        }
    }
}

/*! Implements `CREATE` through the common create runner. */
function execute_create() -> unit = run_create(CreateByNonce)
/*! Implements `CREATE2` through the common create runner. */
function execute_create2() -> unit = run_create(CreateBySalt)
/*! Implements `CALL` through the common call runner. */
function execute_call() -> unit = run_call(Call)
/*! Implements `CALLCODE` through the common call runner. */
function execute_callcode() -> unit = run_call(CallCode)
/*! Implements `DELEGATECALL` through the common call runner. */
function execute_delegatecall() -> unit = run_call(DelegateCall)
/*! Implements `STATICCALL` through the common call runner. */
function execute_staticcall() -> unit = run_call(StaticCall)

/*! The Yellow Paper stack effect `(delta, alpha)` for every decoded
    instruction. Invalid deep-stack immediates have no stack effect because
    their opcode body raises `InvalidOpcode` before accessing the stack. */
function opcode_stack_effect(op : ast) -> (operand_stack_height, operand_stack_height) = match op {
    STOP() => (0, 0),
    ADD() => (2, 1), MUL() => (2, 1), SUB() => (2, 1),
    DIV() => (2, 1), SDIV() => (2, 1), MOD() => (2, 1), SMOD() => (2, 1),
    ADDMOD() => (3, 1), MULMOD() => (3, 1), EXP() => (2, 1),
    SIGNEXTEND() => (2, 1),
    LT() => (2, 1), GT() => (2, 1), SLT() => (2, 1), SGT() => (2, 1),
    EQ() => (2, 1), ISZERO() => (1, 1),
    AND() => (2, 1), OR() => (2, 1), XOR() => (2, 1), NOT() => (1, 1),
    BYTE() => (2, 1), SHL() => (2, 1), SHR() => (2, 1), SAR() => (2, 1),
    CLZ() => (1, 1),
    KECCAK256() => (2, 1),
    ADDRESS() => (0, 1), BALANCE() => (1, 1), ORIGIN() => (0, 1),
    CALLER() => (0, 1), CALLVALUE() => (0, 1), CALLDATALOAD() => (1, 1),
    CALLDATASIZE() => (0, 1), CALLDATACOPY() => (3, 0),
    CODESIZE() => (0, 1), CODECOPY() => (3, 0), GASPRICE() => (0, 1),
    EXTCODESIZE() => (1, 1), EXTCODECOPY() => (4, 0),
    RETURNDATASIZE() => (0, 1), RETURNDATACOPY() => (3, 0),
    EXTCODEHASH() => (1, 1),
    BLOCKHASH() => (1, 1), COINBASE() => (0, 1), TIMESTAMP() => (0, 1),
    NUMBER() => (0, 1), PREVRANDAO() => (0, 1), GASLIMIT() => (0, 1),
    CHAINID() => (0, 1), SELFBALANCE() => (0, 1), BASEFEE() => (0, 1),
    BLOBHASH() => (1, 1), BLOBBASEFEE() => (0, 1), SLOTNUM() => (0, 1),
    POP() => (1, 0), MLOAD() => (1, 1), MSTORE() => (2, 0),
    MSTORE8() => (2, 0), SLOAD() => (1, 1), SSTORE() => (2, 0),
    JUMP() => (1, 0), JUMPI() => (2, 0), PC() => (0, 1), MSIZE() => (0, 1),
    GAS() => (0, 1), JUMPDEST() => (0, 0), TLOAD() => (1, 1),
    TSTORE() => (2, 0), MCOPY() => (3, 0),
    PUSH(_, _) => (0, 1),
    DUP(n) => (n, n + 1),
    SWAP(n) => (n + 1, n + 1),
    DUPN(immediate) => if deep_stack_immediate_valid(immediate) then {
            let n = decode_single_stack_index(immediate);
            (n, n + 1)
        } else {
            (0, 0)
        },
    SWAPN(immediate) => if deep_stack_immediate_valid(immediate) then {
            let n = decode_single_stack_index(immediate);
            (n + 1, n + 1)
        } else {
            (0, 0)
        },
    EXCHANGE(immediate) => if exchange_immediate_valid(immediate) then {
            let (_, higher) = decode_exchange_stack_indices(immediate);
            (higher + 1, higher + 1)
        } else {
            (0, 0)
        },
    LOG(n) => (n + 2, 0),
    opcode_CREATE() => (3, 1), CALL() => (7, 1), CALLCODE() => (7, 1),
    RETURN() => (2, 0), DELEGATECALL() => (6, 1), CREATE2() => (4, 1),
    STATICCALL() => (6, 1), REVERT() => (2, 0), INVALID() => (0, 0),
    SELFDESTRUCT() => (1, 0),
}

/*! Standard decoded-opcode dispatcher. Each arm invokes the same narrow
    semantic function used directly by the optimized raw-byte interpreter. */
function execute_opcode(op : ast) -> unit = match op {
    STOP() => execute_stop(),
    /* ---- arithmetic (pure) ---- */
    ADD() => execute_add(),
    MUL() => execute_mul(),
    SUB() => execute_sub(),
    DIV() => execute_div(),
    SDIV() => execute_sdiv(),
    MOD() => execute_mod(),
    SMOD() => execute_smod(),
    ADDMOD() => execute_addmod(),
    MULMOD() => execute_mulmod(),
    EXP() => execute_exp(),
    SIGNEXTEND() => execute_signextend(),
    /* ---- comparison / bitwise (pure) ---- */
    LT() => execute_lt(),
    GT() => execute_gt(),
    SLT() => execute_slt(),
    SGT() => execute_sgt(),
    EQ() => execute_eq(),
    ISZERO() => execute_iszero(),
    AND() => execute_and(),
    OR() => execute_or(),
    XOR() => execute_xor(),
    NOT() => execute_not(),
    BYTE() => execute_byte(),
    SHL() => execute_shl(),
    SHR() => execute_shr(),
    SAR() => execute_sar(),
    CLZ() => execute_clz(),
    /* ---- keccak: hash via kernel primitive ---- */
    KECCAK256() => execute_keccak256(),
    /* ---- environment / account (kernel reads, EVM prices them) ---- */
    ADDRESS() => execute_address(),
    ORIGIN() => execute_origin(),
    CALLER() => execute_caller(),
    CALLVALUE() => execute_callvalue(),
    GASPRICE() => execute_gasprice(),
    CALLDATASIZE() => execute_calldatasize(),
    CALLDATALOAD() => execute_calldataload(),
    CALLDATACOPY() => execute_calldatacopy(),
    CODESIZE() => execute_codesize(),
    CODECOPY() => execute_codecopy(),
    BALANCE() => execute_balance(),
    SELFBALANCE() => execute_selfbalance(),
    EXTCODESIZE() => execute_extcodesize(),
    EXTCODECOPY() => execute_extcodecopy(),
    EXTCODEHASH() => execute_extcodehash(),
    RETURNDATASIZE() => execute_returndatasize(),
    RETURNDATACOPY() => execute_returndatacopy(),
    /* ---- block (kernel reads) ---- */
    BLOCKHASH() => execute_blockhash(),
    COINBASE() => execute_coinbase(),
    TIMESTAMP() => execute_timestamp(),
    NUMBER() => execute_number(),
    SLOTNUM() => execute_slotnum(),
    PREVRANDAO() => execute_prevrandao(),
    GASLIMIT() => execute_gaslimit(),
    CHAINID() => execute_chainid(),
    BASEFEE() => execute_basefee(),
    BLOBBASEFEE() => execute_blobbasefee(),
    BLOBHASH() => execute_blobhash(),
    /* ---- stack / memory (pure, user-space) ---- */
    POP() => execute_pop(),
    MLOAD() => execute_mload(),
    MSTORE() => execute_mstore(),
    MSTORE8() => execute_mstore8(),
    MSIZE() => execute_msize(),
    MCOPY() => execute_mcopy(),
    /* ---- storage: gas/decision in EVM, effect in kernel ---- */
    SLOAD() => execute_sload(),
    SSTORE() => execute_sstore(),
    TLOAD() => execute_tload(),
    TSTORE() => execute_tstore(),
    /* ---- flow (pure over own code) ---- */
    JUMP() => execute_jump(),
    JUMPI() => execute_jumpi(),
    PC() => execute_pc(),
    GAS() => execute_gas(),
    JUMPDEST() => execute_jumpdest(),
    /* ---- push / dup / swap (pure) ---- */
    PUSH(n, v) => execute_push(n, v),
    DUP(n) => execute_dup(n),
    SWAP(n) => execute_swap(n),
    DUPN(immediate) => execute_dupn(immediate),
    SWAPN(immediate) => execute_swapn(immediate),
    EXCHANGE(immediate) => execute_exchange(immediate),
    /* ---- log: EVM checks/prices, kernel records ---- */
    LOG(n) => execute_log(n),
    /* ---- system ---- */
    opcode_CREATE() => execute_create(),
    CREATE2() => execute_create2(),
    CALL() => execute_call(),
    CALLCODE() => execute_callcode(),
    DELEGATECALL() => execute_delegatecall(),
    STATICCALL() => execute_staticcall(),
    RETURN() => execute_return(),
    REVERT() => execute_revert(),
    INVALID() => execute_invalid(),
    SELFDESTRUCT() => execute_selfdestruct(),
}

/*! The single-step transition function over the standard decoded AST. Stack
    validity is established before gas charging or any instruction side
    effect, as part of the Yellow Paper exceptional-halt predicate. */
function execute(op : ast) -> unit = {
    let (inputs, outputs) = opcode_stack_effect(op);
    if validate_stack(inputs, outputs) then {
        execute_opcode(op)
    }
}
