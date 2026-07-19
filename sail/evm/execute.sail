
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
quadratic expansion cost via [charge_memory_expansion][] before acting.
Sub-calls and creates delegate to [run_call][] / [run_create][], which
install a child frame and save its parent continuation. The non-recursive
opcode bodies live in [execute_opcode][].
*/
/*! The call-family runner (0 = `CALL`, 1 = `CALLCODE`,
    2 = `DELEGATECALL`, 3 = `STATICCALL`); defined with the
    interpreter. */
val run_call : CallKind -> unit
/*! The create runner (`true` = `CREATE2`); defined with the
    interpreter. */
val run_create : bool -> unit

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
function do_jump(dest_word : word) -> unit = {
    if word_ult(dest_word, word_of_quantity(frame_code_len())) then {
        match word_to_limb(dest_word) {
            Some(destination_bits) => {
                let destination = ByteQuantity(unsigned(destination_bits));
                if frame_jumpdest_valid(destination) then {
                    pc = destination
                } else {
                    exc_halt(InvalidJump)
                }
            },
            None() => exc_halt(InvalidJump),
        }
    } else {
        exc_halt(InvalidJump)
    }
}

/*! Pops `count` log topics from the stack. */
function pop_log_topics(count : log_topic_count) -> list(word) =
    match count {
        0 => [||],
        1 => [|pop()|],
        2 => {
            let t0 = pop();
            let t1 = pop();
            [|t0, t1|]
        },
        3 => {
            let t0 = pop();
            let t1 = pop();
            let t2 = pop();
            [|t0, t1, t2|]
        },
        4 => {
            let t0 = pop();
            let t1 = pop();
            let t2 = pop();
            let t3 = pop();
            [|t0, t1, t2, t3|]
        },
        _ => [||],
    }

/*md
## The dispatch
*/
/*! Executes the arithmetic, comparison, and bitwise opcode families. */
function execute_arithmetic(op : ast) -> unit = match op {
    ADD() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_add(a, b))
    },
    MUL() => {
        charge(G_low);
        let a = pop();
        let b = pop();
        push(alu_mul(a, b))
    },
    SUB() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_sub(a, b))
    },
    DIV() => {
        charge(G_low);
        let a = pop();
        let b = pop();
        push(alu_div(a, b))
    },
    SDIV() => {
        charge(G_low);
        let a = pop();
        let b = pop();
        push(alu_sdiv(a, b))
    },
    MOD() => {
        charge(G_low);
        let a = pop();
        let b = pop();
        push(alu_mod(a, b))
    },
    SMOD() => {
        charge(G_low);
        let a = pop();
        let b = pop();
        push(alu_smod(a, b))
    },
    ADDMOD() => {
        charge(G_mid);
        let a = pop();
        let b = pop();
        let n = pop();
        push(alu_addmod(a, b, n))
    },
    MULMOD() => {
        charge(G_mid);
        let a = pop();
        let b = pop();
        let n = pop();
        push(alu_mulmod(a, b, n))
    },
    EXP() => {
        let a = pop();
        let e = pop();
        charge(exp_gas(e));
        push(alu_exp(a, e))
    },
    SIGNEXTEND() => {
        charge(G_low);
        let bi = pop();
        let v = pop();
        push(alu_signextend(bi, v))
    },
    LT() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_lt(a, b))
    },
    GT() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_gt(a, b))
    },
    SLT() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_slt(a, b))
    },
    SGT() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_sgt(a, b))
    },
    EQ() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_eq(a, b))
    },
    ISZERO() => {
        charge(G_verylow);
        let a = pop();
        push(alu_iszero(a))
    },
    AND() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_and(a, b))
    },
    OR() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_or(a, b))
    },
    XOR() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_xor(a, b))
    },
    NOT() => {
        charge(G_verylow);
        let a = pop();
        push(alu_not(a))
    },
    BYTE() => {
        charge(G_verylow);
        let i = pop();
        let x = pop();
        push(alu_byte(i, x))
    },
    SHL() => {
        /* EIP-145 */
        charge(G_verylow);
        let s = pop();
        let v = pop();
        push(alu_shl(s, v))
    },
    SHR() => {
        /* EIP-145 */
        charge(G_verylow);
        let s = pop();
        let v = pop();
        push(alu_shr(s, v))
    },
    SAR() => {
        /* EIP-145 */
        charge(G_verylow);
        let s = pop();
        let v = pop();
        push(alu_sar(s, v))
    },
    CLZ() => {
        /* EIP-7939 */
        charge(G_low);
        let x = pop();
        push(alu_clz(x))
    },
    _ => assert(false),
}

/*! Executes hashing and transaction/environment data access opcodes. */
function execute_environment(op : ast) -> unit = match op {
    KECCAK256() => {
        let offset_word = pop();
        let length_word = pop();
        if is_running() then {
            let (offset, length) = word_byte_range_or_oog(offset_word, length_word);
            if is_running() then {
                charge_keccak_gas(length);
                charge_memory_expansion(offset_word, length_word)
            };
            if is_running() then {
                push(mem_keccak(offset, length))
            }
        }
    },
    ADDRESS() => {
        charge(G_base);
        push(address_to_word(self_addr()))
    },
    ORIGIN() => {
        charge(G_base);
        push(k_env(F_Origin))
    },
    CALLER() => {
        charge(G_base);
        push(address_to_word(message.caller))
    },
    CALLVALUE() => {
        charge(G_base);
        push(message.value)
    },
    GASPRICE() => {
        charge(G_base);
        push(k_env(F_GasPrice))
    },
    CALLDATASIZE() => {
        charge(G_base);
        push(word_of_quantity(calldata.len))
    },
    CALLDATALOAD() => {
        charge(G_verylow);
        let offset_word = pop();
        if is_running() then {
            push(slice_load_word_offset(calldata, offset_word))
        }
    },
    CALLDATACOPY() => {
        charge(G_verylow);
        let destination_word = pop();
        let source_word = pop();
        let length_word = pop();
        if is_running() then {
            let (destination, length) = word_byte_range_or_oog(destination_word, length_word);
            if is_running() then {
                charge_copy_gas(length);
                charge_memory_expansion(destination_word, length_word)
            };
            if is_running() then {
                slice_copy_word_offset(calldata, destination, source_word, length)
            }
        }
    },
    CODESIZE() => {
        charge(G_base);
        push(word_of_quantity(frame_code_len()))
    },
    CODECOPY() => {
        charge(G_verylow);
        let destination_word = pop();
        let source_word = pop();
        let length_word = pop();
        if is_running() then {
            let (destination, length) = word_byte_range_or_oog(destination_word, length_word);
            if is_running() then {
                charge_copy_gas(length);
                charge_memory_expansion(destination_word, length_word)
            };
            if is_running() then {
                slice_copy_word_offset(frame_code.bytes, destination, source_word, length)
            }
        }
    },
    BALANCE() => {
        let a = word_to_address(pop());
        let warm = k_access_account(a);
        charge(account_cost(warm));
        if is_running() then {
            push(k_get_balance(a))
        }
    },
    SELFBALANCE() => {
        charge(G_low);
        push(k_get_balance(self_addr()))
    },
    EXTCODESIZE() => {
        let a = word_to_address(pop());
        let warm = k_access_account(a);
        charge(account_cost(warm) + external_code_read_cost());
        if is_running() then {
            push(word_of_quantity(k_get_code_size(a)))
        }
    },
    EXTCODECOPY() => {
        let a = word_to_address(pop());
        let destination_word = pop();
        let source_word = pop();
        let length_word = pop();
        let warm = k_access_account(a);
        charge(account_cost(warm) + external_code_read_cost());
        if is_running() then {
            let (destination, length) = word_byte_range_or_oog(destination_word, length_word);
            if is_running() then {
                charge_copy_gas(length);
                charge_memory_expansion(destination_word, length_word)
            };
            if is_running() then {
                k_code_copy(a, destination, source_word, length)
            }
        }
    },
    EXTCODEHASH() => {
        let a = word_to_address(pop());
        let warm = k_access_account(a);
        charge(account_cost(warm));
        if is_running() then {
            push(hash_to_word(k_get_codehash(a)))
        }
    },
    RETURNDATASIZE() => {
        charge(G_base);
        push(word_of_quantity(returndata_size()))
    },
    RETURNDATACOPY() => {
        charge(G_verylow);
        let destination_word = pop();
        let source_word = pop();
        let length_word = pop();
        if is_running() then {
            let (destination, length) = word_byte_range_or_oog(destination_word, length_word);
            if is_running() then {
                charge_copy_gas(length);
                charge_memory_expansion(destination_word, length_word)
            };
            if is_running() then {
                returndata_copy_words(destination, source_word, length_word)
            }
        }
    },
    _ => assert(false),
}

/*! Executes block-context opcodes. */
function execute_block(op : ast) -> unit = match op {
    BLOCKHASH() => {
        charge(GasConstant(20));
        push(hash_to_word(k_blockhash(pop())))
    },
    COINBASE() => {
        charge(G_base);
        push(k_env(F_Coinbase))
    },
    TIMESTAMP() => {
        charge(G_base);
        push(k_env(F_Timestamp))
    },
    NUMBER() => {
        charge(G_base);
        push(k_env(F_Number))
    },
    SLOTNUM() => {
        charge(G_base);
        push(k_env(F_SlotNumber))
    },
    PREVRANDAO() => {
        charge(G_base);
        push(k_env(F_PrevRandao))
    },
    GASLIMIT() => {
        charge(G_base);
        push(k_env(F_GasLimit))
    },
    CHAINID() => {
        charge(G_base);
        push(k_env(F_ChainId))
    },
    BASEFEE() => {
        charge(G_base);
        push(k_env(F_BaseFee))
    },
    BLOBBASEFEE() => {
        charge(G_base);
        push(blob_base_fee(k_header.excess_blob_gas))
    },
    BLOBHASH() => {
        charge(G_verylow);
        push(k_blobhash(pop()))
    },
    _ => assert(false),
}

/*! Executes stack and linear-memory opcodes. */
function execute_memory(op : ast) -> unit = match op {
    POP() => {
        charge(G_base);
        let _ = pop()
    },
    MLOAD() => {
        charge(G_verylow);
        let offset_word = pop();
        charge_memory_expansion(offset_word, word_of_protocol_quantity(32));
        if is_running() then {
            let offset = word_to_byte_quantity_or_oog(offset_word);
            if is_running() then {
                push(mem_load(offset))
            }
        }
    },
    MSTORE() => {
        charge(G_verylow);
        let offset_word = pop();
        let v = pop();
        charge_memory_expansion(offset_word, word_of_protocol_quantity(32));
        if is_running() then {
            let offset = word_to_byte_quantity_or_oog(offset_word);
            if is_running() then {
                mem_store(offset, v)
            }
        }
    },
    MSTORE8() => {
        charge(G_verylow);
        let offset_word = pop();
        let v = pop();
        charge_memory_expansion(offset_word, WORD_ONE);
        if is_running() then {
            let offset = word_to_byte_quantity_or_oog(offset_word);
            if is_running() then {
                mem_store_byte(offset, v)
            }
        }
    },
    MSIZE() => {
        charge(G_base);
        push(word_of_quantity(evm_memory_size()))
    },
    MCOPY() => {
        /* EIP-5656 */
        charge(G_verylow);
        let destination_word = pop();
        let source_word = pop();
        let length_word = pop();
        let required_size = maximum_memory_required_size(destination_word, length_word, source_word, length_word);
        if is_running() then {
            let (destination, length) = word_byte_range_or_oog(destination_word, length_word);
            let (source, _) = word_byte_range_or_oog(source_word, length_word);
            if is_running() then {
                charge_copy_gas(length);
                charge(memory_expansion_gas(required_size))
            };
            if is_running() then {
                expand_memory(required_size)
            };
            if is_running() then {
                mem_mcopy(destination, source, length)
            }
        }
    },
    _ => assert(false),
}

/*! Executes persistent and transient storage opcodes. */
function execute_storage(op : ast) -> unit = match op {
    SLOAD() => {
        /* EIP-2929: cold (2100) vs warm (100) by the slot's accessed-set bit */
        let s = pop();
        let warm = k_slot_is_warm(self_addr(), s);
        charge(sload_cost(warm));
        if is_running() then {
            push(k_sload(self_addr(), s).curr)
        }
    },
    SSTORE() => {
        if guard_static() then {
            ()
        } else {
            if (k_fork < Amsterdam) & (gas_remaining <= G_callstipend) then {
                exc_halt(OutOfGas)
            } else {
                let s = pop();
                let v = pop();
                if is_running() then {
                    let warm = k_slot_is_warm(self_addr(), s);
                    let cold = not_bool(warm);
                    if k_fork >= Amsterdam then {
                        let access_cost = amsterdam_storage_access_cost(cold);
                        let sentry_cost = if access_cost < G_sstore_sentry then { G_sstore_sentry } else { access_cost };
                        check_execution_gas(sentry_cost)
                    };
                    if not_bool(is_running()) then {
                        return ()
                    };
                    let entry = k_sload(self_addr(), s);
                    let costs = sstore_costs(entry.orig, entry.curr, v, cold);
                    if costs.state_credit != GAS_COST_ZERO then {
                        credit_state_gas_refund(costs.state_credit)
                    };
                    charge(costs.execution);
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
    },
    TLOAD() => {
        /* EIP-1153 */
        charge(G_warm_access);
        let s = pop();
        push(k_tload(self_addr(), s))
    },
    TSTORE() => {
        /* EIP-1153 */
        if guard_static() then {
            ()
        } else {
            charge(G_warm_access);
            let s = pop();
            let v = pop();
            k_tstore(self_addr(), s, v)
        }
    },
    _ => assert(false),
}

/*! Executes control-flow and stack-shuffling opcodes. */
function execute_control(op : ast) -> unit = match op {
    JUMP() => {
        charge(G_mid);
        let dest = pop();
        do_jump(dest)
    },
    JUMPI() => {
        charge(G_high);
        let dest = pop();
        let cond = pop();
        if word_is_zero(cond) then {
            ()
        } else {
            do_jump(dest)
        }
    },
    PC() => {
        charge(G_base);
        push(alu_sub(word_of_quantity(pc), WORD_ONE))
    },
    GAS() => {
        charge(G_base);
        push(word_of_gas(gas_remaining))
    },
    JUMPDEST() => charge(G_jumpdest),
    PUSH(n, v) => {
        if n == 0 then {
            charge(G_base)
        } else {
            charge(G_verylow)
        };
        push(v)
    },
    DUP(n) => {
        charge(G_verylow);
        if stack_height() < n then {
            exc_halt(StackUnderflow)
        } else {
            push(peek(n - 1))
        }
    },
    SWAP(n) => {
        charge(G_verylow);
        if stack_height() < n + 1 then {
            exc_halt(StackUnderflow)
        } else {
            let top = peek(0);
            let other = peek(n);
            stack_set(0, other);
            stack_set(n, top)
        }
    },
    DUPN(immediate) => {
        charge(G_verylow);
        if is_running() then {
            if not_bool(deep_stack_immediate_valid(immediate)) then {
                exc_halt(InvalidOpcode)
            } else {
                let n = decode_single_stack_index(immediate);
                if stack_height() < n then {
                    exc_halt(StackUnderflow)
                } else {
                    push(peek(n - 1))
                }
            }
        }
    },
    SWAPN(immediate) => {
        charge(G_verylow);
        if is_running() then {
            if not_bool(deep_stack_immediate_valid(immediate)) then {
                exc_halt(InvalidOpcode)
            } else {
                let n = decode_single_stack_index(immediate);
                if stack_height() < n + 1 then {
                    exc_halt(StackUnderflow)
                } else {
                    let top = peek(0);
                    let other = peek(n);
                    stack_set(0, other);
                    stack_set(n, top)
                }
            }
        }
    },
    EXCHANGE(immediate) => {
        charge(G_verylow);
        if is_running() then {
            if not_bool(exchange_immediate_valid(immediate)) then {
                exc_halt(InvalidOpcode)
            } else {
                let (n, m) = decode_exchange_stack_indices(immediate);
                if stack_height() < m + 1 then {
                    exc_halt(StackUnderflow)
                } else {
                    let first = peek(n);
                    let second = peek(m);
                    stack_set(n, second);
                    stack_set(m, first)
                }
            }
        }
    },
    _ => assert(false),
}

/*! Executes the LOG family. */
function execute_log(op : ast) -> unit = match op {
    LOG(n) => {
        if guard_static() then {
            ()
        } else {
            let offset_word = pop();
            let length_word = pop();
            let topics = pop_log_topics(n);
            if is_running() then {
                let (offset, length) = word_byte_range_or_oog(offset_word, length_word);
                if is_running() then {
                    charge_log_gas(n, length);
                    charge_memory_expansion(offset_word, length_word)
                };
                if is_running() then {
                    k_log(self_addr(), topics, BytesSlice(memory_byte_slice(offset, length)))
                }
            }
        }
    },
    _ => assert(false),
}

/*! Executes frame-halting opcodes other than call and create. */
function execute_halt(op : ast) -> unit = match op {
    STOP() => frame_status = Halted(HaltStop()),
    RETURN() => {
        let offset_word = pop();
        let length_word = pop();
        charge_memory_expansion(offset_word, length_word);
        if is_running() then {
            let (offset, length) = word_byte_range_or_oog(offset_word, length_word);
            if is_running() then {
                frame_status = Halted(HaltReturn(freeze_output(memory_byte_slice(offset, length))))
            }
        }
    },
    REVERT() => {
        /* EIP-140 */
        let offset_word = pop();
        let length_word = pop();
        charge_memory_expansion(offset_word, length_word);
        if is_running() then {
            let (offset, length) = word_byte_range_or_oog(offset_word, length_word);
            if is_running() then {
                refill_frame_state_gas();
                frame_status = Halted(HaltRevert(freeze_output(memory_byte_slice(offset, length))))
            }
        }
    },
    INVALID() => {
        gas_remaining = GAS_ZERO;
        exc_halt(InvalidOpcode)
    },
    SELFDESTRUCT() => {
        if guard_static() then {
            ()
        } else {
            let beneficiary = word_to_address(pop());
            if k_fork >= Amsterdam then {
                let warm = k_access_account(beneficiary);
                let access_cost =   GAS_COST_ZERO
                                  + G_selfdestruct
                                  + (if warm then { G_zero } else { G_amsterdam_cold_account_access });
                check_execution_gas(access_cost);
                if is_running() then {
                    let bal = k_get_balance(self_addr());
                    let creates_account = word_nonzero(bal) & k_account_is_empty(beneficiary);
                    let execution_cost = if creates_account then {
                            access_cost + G_amsterdam_account_write
                        } else {
                            access_cost
                        };
                    charge(execution_cost);
                    if is_running() & creates_account then {
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
                let warm = k_access_account(beneficiary);
                charge(G_selfdestruct);
                if not_bool(warm) then {
                    charge(G_cold_account)
                };
                if is_running() & word_nonzero(bal) & k_account_is_empty(beneficiary) then {
                    charge(G_newaccount)
                };
                if is_running() then {
                    let first_selfdestruct = not_bool(k_is_selfdestructed(self_addr()));
                    if (k_fork < London) & first_selfdestruct then {
                        record_refund(gas_constant_to_refund(R_selfdestruct_pre_london))
                    };
                    k_transfer(self_addr(), beneficiary, bal);
                    if k_fork < Cancun then {
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
    },
    _ => assert(false),
}

/*! Executes every opcode except the call and create families. The public
    [execute][] dispatcher handles those six frame-entering cases directly
    and only calls this function for the remaining constructors. */
function execute_opcode(op : ast) -> unit = match op {
    STOP() => execute_halt(op),
    /* ---- arithmetic (pure) ---- */
    ADD() => execute_arithmetic(op),
    MUL() => execute_arithmetic(op),
    SUB() => execute_arithmetic(op),
    DIV() => execute_arithmetic(op),
    SDIV() => execute_arithmetic(op),
    MOD() => execute_arithmetic(op),
    SMOD() => execute_arithmetic(op),
    ADDMOD() => execute_arithmetic(op),
    MULMOD() => execute_arithmetic(op),
    EXP() => execute_arithmetic(op),
    SIGNEXTEND() => execute_arithmetic(op),
    /* ---- comparison / bitwise (pure) ---- */
    LT() => execute_arithmetic(op),
    GT() => execute_arithmetic(op),
    SLT() => execute_arithmetic(op),
    SGT() => execute_arithmetic(op),
    EQ() => execute_arithmetic(op),
    ISZERO() => execute_arithmetic(op),
    AND() => execute_arithmetic(op),
    OR() => execute_arithmetic(op),
    XOR() => execute_arithmetic(op),
    NOT() => execute_arithmetic(op),
    BYTE() => execute_arithmetic(op),
    SHL() => execute_arithmetic(op),
    SHR() => execute_arithmetic(op),
    SAR() => execute_arithmetic(op),
    CLZ() => execute_arithmetic(op),
    /* ---- keccak: hash via kernel primitive ---- */
    KECCAK256() => execute_environment(op),
    /* ---- environment / account (kernel reads, EVM prices them) ---- */
    ADDRESS() => execute_environment(op),
    ORIGIN() => execute_environment(op),
    CALLER() => execute_environment(op),
    CALLVALUE() => execute_environment(op),
    GASPRICE() => execute_environment(op),
    CALLDATASIZE() => execute_environment(op),
    CALLDATALOAD() => execute_environment(op),
    CALLDATACOPY() => execute_environment(op),
    CODESIZE() => execute_environment(op),
    CODECOPY() => execute_environment(op),
    BALANCE() => execute_environment(op),
    SELFBALANCE() => execute_environment(op),
    EXTCODESIZE() => execute_environment(op),
    EXTCODECOPY() => execute_environment(op),
    EXTCODEHASH() => execute_environment(op),
    RETURNDATASIZE() => execute_environment(op),
    RETURNDATACOPY() => execute_environment(op),
    /* ---- block (kernel reads) ---- */
    BLOCKHASH() => execute_block(op),
    COINBASE() => execute_block(op),
    TIMESTAMP() => execute_block(op),
    NUMBER() => execute_block(op),
    SLOTNUM() => execute_block(op),
    PREVRANDAO() => execute_block(op),
    GASLIMIT() => execute_block(op),
    CHAINID() => execute_block(op),
    BASEFEE() => execute_block(op),
    BLOBBASEFEE() => execute_block(op),
    BLOBHASH() => execute_block(op),
    /* ---- stack / memory (pure, user-space) ---- */
    POP() => execute_memory(op),
    MLOAD() => execute_memory(op),
    MSTORE() => execute_memory(op),
    MSTORE8() => execute_memory(op),
    MSIZE() => execute_memory(op),
    MCOPY() => execute_memory(op),
    /* ---- storage: gas/decision in EVM, effect in kernel ---- */
    SLOAD() => execute_storage(op),
    SSTORE() => execute_storage(op),
    TLOAD() => execute_storage(op),
    TSTORE() => execute_storage(op),
    /* ---- flow (pure over own code) ---- */
    JUMP() => execute_control(op),
    JUMPI() => execute_control(op),
    PC() => execute_control(op),
    GAS() => execute_control(op),
    JUMPDEST() => execute_control(op),
    /* ---- push / dup / swap (pure) ---- */
    PUSH(_, _) => execute_control(op),
    DUP(_) => execute_control(op),
    SWAP(_) => execute_control(op),
    DUPN(_) => execute_control(op),
    SWAPN(_) => execute_control(op),
    EXCHANGE(_) => execute_control(op),
    /* ---- log: EVM checks/prices, kernel records ---- */
    LOG(_) => execute_log(op),
    /* ---- system ---- */
    CREATE() => assert(false),
    CREATE2() => assert(false),
    CALL() => assert(false),
    CALLCODE() => assert(false),
    DELEGATECALL() => assert(false),
    STATICCALL() => assert(false),
    RETURN() => execute_halt(op),
    REVERT() => execute_halt(op),
    INVALID() => execute_halt(op),
    SELFDESTRUCT() => execute_halt(op),
}

/*! The single-step transition function: call/create opcodes install a child
    frame; every other opcode executes in [execute_opcode][]. */
function execute(op : ast) -> unit = match op {
    CREATE() => run_create(false),
    CREATE2() => /* EIP-1014 */ run_create(true),
    CALL() => run_call(Call),
    CALLCODE() => run_call(CallCode),
    DELEGATECALL() => /* EIP-7 */ run_call(DelegateCall),
    STATICCALL() => /* EIP-214 */ run_call(StaticCall),
    _ => execute_opcode(op),
}
