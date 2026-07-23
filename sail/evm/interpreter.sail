/* CALL/CALLCODE/DELEGATECALL/STATICCALL (selected by `kind`) and           */
/* The complete parent frame, including a kernel state checkpoint, is       */
/* suspended before child effects begin and restored on return. k_revert    */
/* rolls the world back atomically when the child fails. The applicable     */

/*md
# The interpreter

The user-space EVM: it fetches and decodes bytecode (Yellow Paper §9),
drives the step loop, and enters sub-frames for the call and create
opcodes. This module specifies that machinery in three layers:

1. **Fetch/decode** — read the opcode at `pc`, decode PUSH immediates
   inline, and map every other byte to its AST node (an undefined byte
   decodes to `INVALID`). Reading past the end of code yields `STOP`
   (YP: implicit halt).
2. **Run loop** — [interpret][] steps fetch-then-execute until the active
   frame stops. A completed child is resumed through its explicit
   [FrameContinuation][type-FrameContinuation]; a completed top-level
   frame ends interpretation.
3. **Message calls** — [run_call][] handles
   `CALL`/`CALLCODE`/`DELEGATECALL`/`STATICCALL` (multiplexed on `mode`)
   and [run_create][] handles `CREATE`/`CREATE2`. A sub-call saves the
   parent's user-space frame registers in [frame_stack][], installs the
   child as the active frame, and returns to the single run loop. When the
   child halts, that loop restores and resumes the parent. There is no
   recursive interpreter invocation. All world effects go through kernel syscalls:
   [k_state_checkpoint][] on entry, [k_transfer][] for value, and [k_revert][] on
   failure — the kernel rolls the world back atomically on a reverting
   child. The applicable rules are EIP-150 (63/64ths gas cap + stipend),
   EIP-214 (static-context write protection), EIP-2929 (cold/warm
   access), and EIP-7702 (delegated-code execution).
*/
/*md
## The decoder
*/
/*! Assembles an `n`-byte big-endian PUSH immediate from a local code cursor;
    bytes past the end of code read as zero. */
function read_push(code : CodeSlice, offset : code_pointer, n : push_width) -> word =
    slice_load_n(code, offset, n)

/* Decode one non-PUSH opcode byte to its AST node. The three contiguous
 * families fold to an arity argument -- DUP1..16 (0x80..0x8f), SWAP1..16
 * (0x90..0x9f), LOG0..4 (0xa0..0xa4) -- and the remainder is a flat table.
 * Any byte with no defined opcode decodes to INVALID. */
/*! Decodes one non-PUSH opcode byte to its AST node. The three
    contiguous families fold to an arity argument — `DUP1`–`DUP16`
    (0x80–0x8f), `SWAP1`–`SWAP16` (0x90–0x9f), `LOG0`–`LOG4` (0xa0–0xa4)
    — and the remainder is a flat table. Any byte with no defined opcode
    decodes to `INVALID`. */
function decode_simple(opcode : opcode) -> ast =
    if 128 <= opcode & opcode <= 143 then {
        DUP(opcode - 127)
    } else {
        if 144 <= opcode & opcode <= 159 then {
            SWAP(opcode - 143)
        } else {
            if 160 <= opcode & opcode <= 164 then {
                LOG(opcode - 160)
            } else {
                match opcode {
                    0 => STOP(),
                    1 => ADD(),
                    2 => MUL(),
                    3 => SUB(),
                    4 => DIV(),
                    5 => SDIV(),
                    6 => MOD(),
                    7 => SMOD(),
                    8 => ADDMOD(),
                    9 => MULMOD(),
                    10 => EXP(),
                    11 => SIGNEXTEND(),
                    16 => LT(),
                    17 => GT(),
                    18 => SLT(),
                    19 => SGT(),
                    20 => EQ(),
                    21 => ISZERO(),
                    22 => AND(),
                    23 => OR(),
                    24 => XOR(),
                    25 => NOT(),
                    26 => BYTE(),
                    27 => SHL(),
                    28 => SHR(),
                    29 => SAR(),
                    30 => if k_fork >= Osaka then {
                        CLZ()
                    } else {
                        INVALID()
                    }, /* EIP-7939: Osaka+ */
                    32 => KECCAK256(),
                    48 => ADDRESS(),
                    49 => BALANCE(),
                    50 => ORIGIN(),
                    51 => CALLER(),
                    52 => CALLVALUE(),
                    53 => CALLDATALOAD(),
                    54 => CALLDATASIZE(),
                    55 => CALLDATACOPY(),
                    56 => CODESIZE(),
                    57 => CODECOPY(),
                    58 => GASPRICE(),
                    59 => EXTCODESIZE(),
                    60 => EXTCODECOPY(),
                    61 => RETURNDATASIZE(),
                    62 => RETURNDATACOPY(),
                    63 => EXTCODEHASH(),
                    64 => BLOCKHASH(),
                    65 => COINBASE(),
                    66 => TIMESTAMP(),
                    67 => NUMBER(),
                    68 => PREVRANDAO(),
                    69 => GASLIMIT(),
                    70 => CHAINID(),
                    71 => SELFBALANCE(),
                    72 => if k_fork >= London then {
                        BASEFEE()
                    } else {
                        INVALID()
                    },
                    73 => if k_fork >= Cancun then {
                        BLOBHASH()
                    } else {
                        INVALID()
                    },
                    74 => if k_fork >= Cancun then {
                        BLOBBASEFEE()
                    } else {
                        INVALID()
                    },
                    75 => if k_fork >= Amsterdam then {
                        SLOTNUM()
                    } else {
                        INVALID()
                    }, /* EIP-7843: Amsterdam+ (0x4b is undefined earlier) */
                    80 => POP(),
                    81 => MLOAD(),
                    82 => MSTORE(),
                    83 => MSTORE8(),
                    84 => SLOAD(),
                    85 => SSTORE(),
                    86 => JUMP(),
                    87 => JUMPI(),
                    88 => PC(),
                    89 => MSIZE(),
                    90 => GAS(),
                    91 => JUMPDEST(),
                    92 => if k_fork >= Cancun then {
                        TLOAD()
                    } else {
                        INVALID()
                    },
                    93 => if k_fork >= Cancun then {
                        TSTORE()
                    } else {
                        INVALID()
                    },
                    94 => if k_fork >= Cancun then {
                        MCOPY()
                    } else {
                        INVALID()
                    },
                    240 => CREATE(),
                    241 => CALL(),
                    242 => CALLCODE(),
                    243 => RETURN(),
                    244 => DELEGATECALL(),
                    245 => CREATE2(),
                    250 => STATICCALL(),
                    253 => REVERT(),
                    255 => SELFDESTRUCT(),
                    _ => INVALID(),
                }
            }
        }
    }

/*! Fetches and decodes the opcode at `pc`, advancing `pc` past the
    opcode and any immediate. Past the end of code the frame implicitly
    executes `STOP` (YP). `PUSH0`–`PUSH32` (0x5f–0x7f) carry an `n`-byte
    immediate; Amsterdam's `DUPN`/`SWAPN`/`EXCHANGE` carry one byte,
    zero-padded at end of code. Every other byte decodes via
    [decode_simple][]. */
function fetch() -> ast = {
    let current = pc;
    let code = frame_code.bytes;
    let code_length = code.len;
    if not_bool(current < code_length) then {
        STOP()
    } else {
        let opcode : opcode = unsigned(slice_byte(code, current));
        let immediate_offset = current + 1;
        let decoded : (code_pointer, ast) = if (opcode == 95) & (k_fork < Shanghai) then {
                (immediate_offset, INVALID())
            } else {
                if 95 <= opcode & opcode <= 127 then {
                    let size : push_width = opcode - 95;
                    let value = read_push(code, immediate_offset, size);
                    let after_immediate : code_pointer = current + 1 + size;
                    (after_immediate, PUSH(size, value))
                } else {
                    if (k_fork >= Amsterdam) & (230 <= opcode) & (opcode <= 232) then {
                        let immediate = slice_byte(code, immediate_offset);
                        let immediate_valid : bool = match opcode {
                            230 => deep_stack_immediate_valid(immediate),
                            231 => deep_stack_immediate_valid(immediate),
                            232 => exchange_immediate_valid(immediate),
                            _ => false,
                        };
                        let after_instruction : code_pointer = if immediate_valid then {
                                current + 2
                            } else {
                                current + 1
                            };
                        let instruction : ast = match opcode {
                            230 => DUPN(immediate),
                            231 => SWAPN(immediate),
                            232 => EXCHANGE(immediate),
                            _ => INVALID(),
                        };
                        (after_instruction, instruction)
                    } else {
                        (immediate_offset, decode_simple(opcode))
                    }
                }
            };
        let (next_pc, instruction) = decoded;
        pc = next_pc;
        instruction
    }
}

/*md
## The run loop
*/
val resume_frame : (FrameContinuation, ByteSlice) -> unit

/*! Returns the active frame's halt output. */
function frame_output() -> ByteSlice =
    match frame_status {
        Halted(HaltReturn(output)) => output,
        Halted(HaltRevert(output)) => output,
        _ => EMPTY_SLICE,
    }

/*! The non-recursive step loop for one complete call tree. It executes the
    active frame, resumes suspended parents from [frame_stack][] as children
    halt, and returns the top-level frame's output. `STOP`, `SELFDESTRUCT`,
    and exceptional halts return the empty slice; `RETURN` and `REVERT` carry
    their frozen memory slice in the halt value. */
function interpret() -> ByteSlice = {
    frame_stack_reset();
    var interpreting : bool = true;
    var result : ByteSlice = EMPTY_SLICE;

    /* Each opcode step consumes gas; every child completion corresponds to an
     * earlier gas-consuming CALL or CREATE step. */
    while interpreting termination_measure(2 ^ 64) do {
        if is_running() then {
            execute(fetch())
        } else {
            let output = frame_output();
            if frame_stack_is_empty() then {
                result = output;
                interpreting = false
            } else {
                resume_frame(frame_stack_pop(), output)
            }
        }
    };
    result
}

/*! Whether the just-finished frame ended successfully: a normal halt
    succeeds; a `REVERT` and any exceptional halt do not (their world
    effects are rolled back and `CALL`/`CREATE` reports failure). */
function frame_succeeded() -> bool =
    match frame_status {
        Halted(HaltRevert(_)) => false,
        Halted(_) => true,
        Running() => true,
        Exceptional(_) => false,
    }

/*! Selects the code a frame actually executes (EIP-7702). A delegated
    account runs the code at its delegation target, following exactly one
    hop; a delegation whose target is a precompile (or has no code)
    executes as empty code. An undelegated account runs its own code. */
function executable_code(target : address, dele : bool, dtgt : address) -> Code =
    if dele then {
        let delegate_code = code_db_resolve(k_code_key(dtgt));
        if precompile_number(dtgt) != 0 then { EMPTY_CODE } else { delegate_code }
    } else {
        code_db_resolve(k_code_key(target))
    }

/*md
## The message calls
*/
/*! Whether a call kind consumes an explicit value stack operand. */
function call_takes_value(kind : CallKind) -> bool =
    match kind {
        Call => true,
        CallCode => true,
        _ => false,
    }

/*! Whether a call kind transfers value between distinct accounts. */
function call_transfers_value(kind : CallKind) -> bool =
    match kind {
        Call => true,
        _ => false,
    }

/*! Whether a call kind executes with the target as its storage address. */
function call_uses_target_address(kind : CallKind) -> bool =
    match kind {
        Call => true,
        StaticCall => true,
        _ => false,
    }

/*! Whether a call kind inherits the parent caller and call value. */
function call_is_delegate(kind : CallKind) -> bool =
    match kind {
        DelegateCall => true,
        _ => false,
    }

/*! Whether a call kind introduces a read-only execution context. */
function call_is_static(kind : CallKind) -> bool =
    match kind {
        StaticCall => true,
        _ => false,
    }

/* The four call opcodes share one implementation selected by `kind`.
 * CALL enters `target` and may transfer value; CALLCODE executes the target
 * code in the caller's account; DELEGATECALL inherits caller and value;
 * STATICCALL forces a static context and zero value.
 * Operand layout (top of stack first): gas, target, [value for CALL/CALLCODE],
 * argsOffset, argsLen, retOffset, retLen. Pushes 1 on success, 0 on failure. */
/*! The four call opcodes, multiplexed on `mode`.

    - `0` — `CALL`: a new frame at `target`, may transfer value.
    - `1` — `CALLCODE`: runs the target's code in the caller's account,
      may transfer.
    - `2` — `DELEGATECALL` (EIP-7): runs the target's code in the
      caller's account, inheriting the parent's caller/value/static
      context.
    - `3` — `STATICCALL` (EIP-214): `CALL` with value 0 and a forced
      static context.

    Operand layout (top of stack first): `gas`, `target`, `value` (for
    `CALL`/`CALLCODE`), `argsOffset`, `argsLen`, `retOffset`, `retLen`.
    Pushes 1 on success, 0 on failure. */
function run_call(kind : CallKind) -> unit = {
    let caller = self_addr();
    let gas_request = pop();
    let target_word = pop();
    let target = word_to_address(target_word);
    let value = if call_takes_value(kind) then {
            pop()
        } else {
            WORD_ZERO
        };
    let args_off_word = pop();
    let args_len_word = pop();
    let ret_off_word = pop();
    let ret_len_word = pop();

    /* A stack underflow during the operand pops exceptionally halts the frame
       (exc_halt zeroes the gas). The protocol unwinds the whole op on
       an exception; Sail has no unwind, so bail out here before any gas is
       re-added below. */
    if not_bool(is_running()) then {
        return ()
    };

    /* EIP-214: a value-bearing CALL inside a static context is a write and
       raises WriteInStaticContext -- an exceptional halt that consumes all the
       caller frame's gas. CALLCODE/DELEGATECALL/STATICCALL never trigger it
       (CALLCODE has no static guard in the spec; the others force value = 0). */
    if call_transfers_value(kind) & word_nonzero(value) & message.is_static then {
        exc_halt(WriteProtection);
        return ()
    };

    /* EIP-2929 access: warm the target (a set update, NOT a state read) and take
       its cold/warm cost. */
    let warm = k_access_account(target);
    let target_cost : gas_constant = account_cost(warm);
    let transfer_cost : gas_constant = if word_nonzero(value) then {
            call_value_cost()
        } else {
            GAS_CONSTANT_ZERO
        };

    /* Narrow the two word-addressed ranges at the point where memory OOG is
       sequenced. The planner itself halts if the requested high-water mark is
       outside the executable memory domain. */
    let available = gas_remaining;
    let memory = memory_pair_expansion(args_off_word, args_len_word, ret_off_word, ret_len_word, available);
    if not_bool(is_running()) then {
        return ()
    };
    let memory_cost = memory.cost;

    /* Static gas is checked before any state access: the
       target's access + value-transfer + memory cost is checked BEFORE reading
       any of the target's state. A frame that cannot afford it halts here, having
       touched no account -- so the stateless witness need not prove the target.
       (Resolving the EIP-7702 delegation or the empty-account check below reads
       the target's account, which an OOG-before-access tx must never do.) */
    let static_base : gas_cost = target_cost + transfer_cost;
    let before_static = gas_remaining;
    if before_static < static_base then {
        exc_halt(OutOfGas);
        return ()
    };
    let after_static_base : gas = before_static - static_base;
    if after_static_base < memory_cost then {
        exc_halt(OutOfGas);
        return ()
    };

    /* STATE ACCESS (only now that the static gas is covered): the EIP-7702
       delegation designation (reads the target's code) + new-account component. */
    let (tg_deleg, tg_target) = k_deleg_target(target);
    let delegation_cost : gas_constant = if tg_deleg then {
            let dw = k_access_account(tg_target);
            account_cost(dw)
        } else {
            GAS_CONSTANT_ZERO
        };
    let new_account_charged =    k_fork
                              >= Amsterdam
                              &  word_nonzero(value)
                              &  call_transfers_value(kind)
                              &  k_account_is_empty(target);
    let create_cost : gas_constant = if   k_fork
                                        < Amsterdam
                                        & word_nonzero(value)
                                        & call_transfers_value(kind)
                                        & k_account_is_empty(target) then {
            G_newaccount
        } else {
            GAS_CONSTANT_ZERO
        };
    let additional_cost : gas_cost = delegation_cost + create_cost;
    let before_required = gas_remaining;
    if before_required < static_base then {
        exc_halt(OutOfGas);
        return ()
    };
    let after_required_base : gas = before_required - static_base;
    if after_required_base < memory_cost then {
        exc_halt(OutOfGas);
        return ()
    };
    let after_static_cost : gas = after_required_base - memory_cost;
    if after_static_cost < additional_cost then {
        exc_halt(OutOfGas);
        return ()
    };
    let after_required : gas = after_static_cost - additional_cost;
    let required : gas_cost = if after_required <= before_required then {
            before_required - after_required
        } else {
            throw InvalidBlock(ExecutionInvalid)
        };
    let stipend : gas = if word_nonzero(value) then { G_callstipend } else { GAS_ZERO };

    /* Forwarded child gas, per the EIP-150
       calculate_message_call_gas. If the frame cannot even afford
       extra gas plus memory expansion, the call is charged the FULL requested gas (not the
       63/64-capped value), which forces the out-of-gas exceptional halt below.
       Otherwise the EIP-150 63/64ths cap applies to the gas left after
       subtracting the memory and extra costs. */
    var base_child : gas = GAS_ZERO;
    if k_fork >= Amsterdam then {
        /* Amsterdam charges execution work, then state growth, before
           applying EIP-150 to the execution gas left for the child. */
        charge(required);
        if new_account_charged then {
            charge_state_gas(G_amsterdam_state_new_account)
        };
        if is_running() then {
            base_child = call_gas_cap_word(gas_remaining, gas_request);
            charge(base_child)
        }
    } else {
        let avail = gas_remaining;
        base_child =
            if avail < required then {
                GAS_ZERO
            } else {
                let available_after_cost = gas_sub_or_oog(avail, required);
                call_gas_cap_word(available_after_cost, gas_request)
            };

        /* Legacy message-call charging combines the child allocation with
           access, value-transfer, new-account, and memory costs. */
        charge(required);
        if is_running() then {
            charge(base_child)
        }
    };
    if not_bool(is_running()) then {
        return ()
    };

    /* gas is now covered; message preparation loads the
       delegate's CODE -- BEFORE the depth/balance guards below. Resolve it now
       (flagging a missing-code witness deficiency) so an insufficient-balance
       call to a 7702-delegated target still detects an omitted delegate-code
       proof; an OOG call returned above and never reaches here. */
    if tg_deleg then {
        let _ = code_db_resolve(k_code_key(tg_target));

        /* The resolved delegate's account is read for its code, so the
           delegate is touched (kept in the BAL account set). */
        let _ = k_aload(tg_target)
    };
    let (args, ret) = apply_memory_pair_expansion(memory);
    let child_gas : gas = conserved_gas_add(base_child, stipend);

    /* The target's code hash is read for every call once gas is
     * charged, before the depth/balance guards -- so the call target is always a
     * state access (kept in the BAL account set), including precompiles and calls
     * that then fail the depth/balance guard below. */
    let _ = k_aload(target);

    /* call-failure guards (checked AFTER gas is charged): the depth ceiling
       (EIP-150, 1024 frames) and, for a value-bearing CALL/CALLCODE, the
       caller's balance covering the transfer. */
    if (call_depth >= DEPTH_LIMIT) |
        (call_takes_value(kind) & word_nonzero(value) & not_bool(word_ule(value, k_get_balance(caller)))) then {
        /* the call does not happen: the child gas (including stipend) is
           refunded to the caller and CALL pushes 0. */
        returndata_clear();
        refund_gas(child_gas);
        if new_account_charged then {
            credit_state_gas_refund(G_amsterdam_state_new_account)
        };
        push(WORD_ZERO)
    } else {
        let selected_precompile = precompile_number(target);
        if selected_precompile != 0 then {
            let number : precompile_id = selected_precompile;

            /* PRECOMPILE call. The precompile set is fork-gated (the highest is
               0x100 P256VERIFY, EIP-7951/Osaka); an address outside the active
               set is an ordinary code call. The precompile's input is the
               child message's calldata: mem[args_off .. args_off+args_len).
               Gas inspects the same memory source before execution. */
            let input = memory_byte_slice(args.off, args.len);

            /* Gas is checked before execution: an unpayable/OOG precompile call
             * must not execute (a BLAKE2F request may carry an enormous round
             * count). Failure consumes all child gas and exposes no returndata. */
            match precompile_gas(number, input, child_gas) {
                Some(used) => {
                    let result = run_precompile_slice(number, input);
                    if result.success then {
                        returndata = result.output;

                        /* the value transfer is part of the successful call */
                        if call_transfers_value(kind) & word_nonzero(value) then {
                            k_transfer(caller, target, value)
                        };
                        returndata_copy_prefix(ret.off, ret.len);
                        let unused = gas_sub_or_oog(child_gas, used);
                        if is_running() then {
                            refund_gas(unused);
                            push(WORD_ONE)
                        }
                    } else {
                        returndata_clear();
                        if new_account_charged then {
                            credit_state_gas_refund(G_amsterdam_state_new_account)
                        };
                        push(WORD_ZERO)
                    }
                },
                _ => {
                    returndata_clear();
                    if new_account_charged then {
                        credit_state_gas_refund(G_amsterdam_state_new_account)
                    };
                    push(WORD_ZERO)
                },
            }
        } else {
            /* CODE call. Snapshot the world (so a reverting child can be rolled
               back), then perform the CALL value transfer up front. */
            let child_code = executable_code(target, tg_deleg, tg_target);

            /* the child message context, by call mode:
               - address: target for CALL/STATICCALL; the CALLER's own address
                 for CALLCODE/DELEGATECALL (they run target's code in place);
               - caller/value: inherited from the parent for DELEGATECALL,
                 else the immediate caller and the call's value;
               - static: forced for STATICCALL, else inherited (a static frame
                 stays static for all of its sub-calls). */
            let child_addr : address = if call_uses_target_address(kind) then { target } else { caller };
            let child_caller : address = if call_is_delegate(kind) then {
                    message.caller
                } else {
                    caller
                };
            let child_value : word = if call_is_delegate(kind) then {
                    message.value
                } else {
                    value
                };
            let child_static : bool = if call_is_static(kind) then {
                    true
                } else {
                    message.is_static
                };
            let child_calldata = if is_running() then {
                    memory_byte_slice(args.off, args.len)
                } else {
                    EMPTY_SLICE
                };
            let child_state_gas = state_gas_remaining;
            let checkpoint = { suspend_frame() with state_gas_remaining = GAS_ZERO };
            frame_stack_push(
                ResumeCall(
                    struct {
                        checkpoint = checkpoint,
                        return_offset = ret.off,
                        return_length = ret.len,
                        new_account_charged = new_account_charged,
                    },
                ),
            );
            if call_transfers_value(kind) & word_nonzero(value) then {
                k_transfer(caller, target, value)
            };

            /* enter the child frame: pc at 0, its own gas/stack/memory, code
               selected by the EIP-7702 delegation rule, depth bumped. */
            message =
                struct {
                    caller = child_caller,
                    address = child_addr,
                    code_address = target,
                    value = child_value,
                    state_gas_reservoir = child_state_gas,
                    is_static = child_static,
                    depth = frame_depth_increment(checkpoint.call_depth),
                };
            calldata_install(child_calldata);
            pc = 0;
            gas_remaining = child_gas;
            state_gas_remaining = child_state_gas;
            state_gas_spilled = STATE_GAS_SPILL_ZERO;
            frame_status = Running();
            returndata_clear();
            frame_code = child_code;
            call_depth = frame_depth_increment(checkpoint.call_depth);
            frame_refund = GAS_REFUND_ZERO
        }
    }
}

/*! `CREATE` (`is2 = false`) and `CREATE2` (`is2 = true`, EIP-1014).
    Both deploy a new contract by running the initcode supplied in
    memory; the new address derives from `(creator, nonce)` for `CREATE`
    or `(creator, salt, keccak256(initcode))` for `CREATE2`. Operand
    layout: `value`, `offset`, `length`, `salt` (for `CREATE2`). Pushes
    the new address on success, 0 on failure. */
function run_create(is2 : bool) -> unit = {
    let creator = self_addr();
    let value = pop();
    let off_word = pop();
    let len_word = pop();
    let salt = if is2 then {
            pop()
        } else {
            WORD_ZERO
        };

    /* a stack underflow during the operand pops exceptionally halts the frame
       (exc_halt zeroes the gas); bail out before any child gas is allocated. */
    if not_bool(is_running()) then {
        return ()
    };

    /* EIP-214: CREATE/CREATE2 modifies state and is forbidden in a static
       context -- it raises WriteProtection (an exceptional halt consuming all
       remaining gas), checked before any charge or child frame. */
    if guard_static() then {
        return ()
    };
    let initcode = charge_memory_range(off_word, len_word);
    if not_bool(is_running()) then {
        return ()
    };

    /* up-front charges: memory expansion over the initcode, the CREATE base
       cost (G_create), the EIP-3860 per-initcode-word cost, and -- for CREATE2
       only -- the keccak word cost of hashing the initcode for the address. */
    charge(create_access_cost());
    if k_fork >= Shanghai then {
        charge_word_scaled_gas(G_initcode_word, memory_word_count_word(len_word))
    };
    if is2 then {
        charge_word_scaled_gas(G_keccak_word, memory_word_count_word(len_word))
    };

    /* an out-of-gas during those charges is an exceptional halt (all gas
       consumed); bail before any nonce bump or child allocation. */
    if not_bool(is_running()) then {
        return ()
    };

    /* EIP-3860: initcode longer than MAX_INITCODE_SIZE is rejected outright as
       an exceptional halt. */
    if not_bool(initcode_size_allowed(initcode.len)) then {
        exc_halt(InitCodeTooLarge)
    } else {
        /* derive the new contract address: keccak(0xff, creator, salt,
           keccak(initcode)) for CREATE2 (EIP-1014), else rlp(creator, nonce). */
        let nonce = k_get_nonce(creator);
        let new_addr : address = if is2 & is_running() then {
                k_create2_addr(creator, salt, word_to_hash(mem_keccak(initcode)))
            } else {
                k_create_addr(creator, nonce)
            };

        /* Before Amsterdam the child allocation is computed before the
           early guards (and returned if they fail). Amsterdam first charges
           any account-growth state gas, because a spill reduces the execution
           gas to which the EIP-150 cap applies. */
        var child_gas : gas = GAS_ZERO;
        if k_fork < Amsterdam then {
            let avail = gas_remaining;
            let retained_gas : gas = avail / 64;
            child_gas = gas_sub_or_oog(avail, retained_gas);
            gas_remaining = retained_gas
        };
        if not_bool(is_running()) then {
            return ()
        };

        /* EIP-214: a CREATE in a static
         * context (e.g. inside a STATICCALL frame) is a write and raises
         * WriteInStaticContext after the child gas is reserved -- an
         * exceptional halt that consumes all gas, bumps no nonce, runs no
         * initcode. */
        if message.is_static then {
            exc_halt(WriteProtection);
            return ()
        };

        /* early-abort guards (child gas refunded, 0 pushed, NO nonce bump): the
           depth ceiling, the creator's balance covering the endowment, and the
           creator nonce not already at the 2^64-1 ceiling (it must be
           incrementable). */
        if   (call_depth >= DEPTH_LIMIT)
           | not_bool(word_ule(value, k_get_balance(creator)))
           | (nonce == sizeof(account_nonce_bound)) /* EIP-2681 */
        then {
            returndata_clear();
            if k_fork < Amsterdam then {
                refund_gas(child_gas)
            };
            push(WORD_ZERO)
        } else {
            /* warm the new address and bump the creator nonce BEFORE the child
             * snapshot: both persist even if the initcode reverts (the
             * the protocol runs them in the parent, before
             * process_create_message). */
            let _ = k_access_account(new_addr);
            let new_account_charged = (k_fork >= Amsterdam) & k_account_is_empty(new_addr);
            if new_account_charged then {
                charge_state_gas(G_amsterdam_state_new_account)
            };
            if not_bool(is_running()) then {
                return ()
            };
            if k_fork >= Amsterdam then {
                let avail = gas_remaining;
                let retained_gas : gas = avail / 64;
                child_gas = gas_sub_or_oog(avail, retained_gas);
                gas_remaining = retained_gas
            };
            if not_bool(is_running()) then {
                return ()
            };
            let occupied : bool = k_account_occupied(new_addr);
            returndata_clear();
            k_bump_nonce(creator);
            if occupied then {
                /* address collision (the target already has code, a nonzero
                 * nonce, or storage): no initcode runs, 0 is pushed, and the
                 * reserved child gas is NOT refunded (it stays deducted). */
                if new_account_charged then {
                    credit_state_gas_refund(G_amsterdam_state_new_account)
                };
                push(WORD_ZERO)
            } else {
                /* Freeze the initcode while parent memory is active, then
                   suspend the complete parent before mutating the new account.
                   The child frame has no calldata. */
                let initcode = if is_running() then {
                        memory_code_slice(initcode.off, initcode.len)
                    } else {
                        EMPTY_CODE_SLICE
                    };
                let child_code = code_db_resolve(code_db_insert(initcode, k_fork));
                let child_state_gas = state_gas_remaining;
                let checkpoint = { suspend_frame() with state_gas_remaining = GAS_ZERO };
                frame_stack_push(
                    ResumeCreate(
                        struct {
                            checkpoint = checkpoint,
                            address = new_addr,
                            new_account_charged = new_account_charged,
                        },
                    ),
                );
                k_mark_created(new_addr); /* EIP-6780: created this tx */
                k_clear_storage(new_addr);
                k_bump_nonce(new_addr); /* EIP-161: a created contract starts at nonce 1 */
                k_transfer(creator, new_addr, value);
                message =
                    struct {
                        caller = creator,
                        address = new_addr,
                        code_address = new_addr,
                        value = value,
                        state_gas_reservoir = child_state_gas,
                        is_static = checkpoint.message.is_static,
                        depth = frame_depth_increment(checkpoint.call_depth),
                    };
                calldata_install(EMPTY_SLICE);
                pc = 0;
                gas_remaining = child_gas;
                state_gas_remaining = child_state_gas;
                state_gas_spilled = STATE_GAS_SPILL_ZERO;
                frame_status = Running();
                returndata_clear();
                frame_code = child_code;
                call_depth = frame_depth_increment(checkpoint.call_depth);
                frame_refund = GAS_REFUND_ZERO
            }
        }
    }
}

/*! Restores a message-call parent and applies the child's outcome. */
function resume_call(continuation : CallContinuation, output : ByteSlice) -> unit = {
    returndata = output;
    let checkpoint = continuation.checkpoint;
    let succeeded = frame_succeeded();
    let child_left = gas_remaining;
    let child_state_left = state_gas_remaining;
    let child_state_spill = state_gas_spilled;
    let child_refund = frame_refund;

    restore_frame(checkpoint);
    refund_gas(child_left);
    return_child_state_gas(child_state_left, child_state_spill);

    /* Both RETURN and REVERT copy their output; exceptional halts carry the
     * empty slice. Successful effects remain, while every failure reverts to
     * the saved kernel checkpoint. */
    returndata_copy_prefix(continuation.return_offset, continuation.return_length);
    if succeeded then {
        record_refund(child_refund);
        push(WORD_ONE)
    } else {
        k_revert(checkpoint.state);
        if continuation.new_account_charged then {
            credit_state_gas_refund(G_amsterdam_state_new_account)
        };
        push(WORD_ZERO)
    }
}

/*! Restores a create parent and either deploys or rolls back the child. */
function resume_create(continuation : CreateContinuation, output : ByteSlice) -> unit = {
    returndata = output;
    let checkpoint = continuation.checkpoint;
    let initcode_succeeded = frame_succeeded();
    let deployed_length = returndata_size();
    let deployed_size = deployed_length;
    var frontier_empty_deposit : bool = false;

    if initcode_succeeded then {
        if not_bool(deployed_code_size_allowed(deployed_size)) |
            ((k_fork >= London) & (deployed_size != 0) & (slice_byte(returndata, 0) == 0xef)) then {
            /* EIP-170 and EIP-3541 failures use the post-Homestead
             * exceptional creation path. Neither rule is active on
             * Frontier/Homestead in a way that reaches this branch. */
            exc_halt(OutOfGas)
        } else {
            match code_deployment_execution_cost(deployed_length, gas_remaining) {
                Some(execution_deposit) => {
                    gas_remaining = gas_sub_or_oog(gas_remaining, execution_deposit);
                    charge_deployment_state_gas(code_deployment_state_cost(deployed_length))
                },
                None() => {
                    if k_fork < Homestead then {
                        /* Frontier consumed the remaining child gas but kept the
                         * creation successful with empty deployed code. EIP-2
                         * changed this to exceptional failure at Homestead. */
                        gas_remaining = GAS_ZERO;
                        frontier_empty_deposit = true
                    } else {
                        exc_halt(OutOfGas)
                    }
                },
            }
        }
    };
    let deploy_succeeds = initcode_succeeded & frame_succeeded();
    let child_left = gas_remaining;
    let child_state_left = state_gas_remaining;
    let child_state_spill = state_gas_spilled;
    let child_refund = frame_refund;

    restore_frame(checkpoint);
    refund_gas(child_left);
    return_child_state_gas(child_state_left, child_state_spill);

    if deploy_succeeds then {
        record_refund(child_refund);
        let deployed_bytes : ByteSlice = if frontier_empty_deposit then { EMPTY_SLICE } else { returndata };
        let deployed_code = validated_code_slice(deployed_bytes);
        k_deploy_code(continuation.address, deployed_code);
        push(address_to_word(continuation.address))
    } else {
        k_revert(checkpoint.state);
        if continuation.new_account_charged then {
            credit_state_gas_refund(G_amsterdam_state_new_account)
        };
        push(WORD_ZERO)
    };

    /* Successful initcode output has become deployed code. REVERT output
     * remains visible to RETURNDATA opcodes in the parent. */
    if initcode_succeeded then {
        returndata_clear()
    }
}

/*! Applies the pending operation for one completed child frame. */
function resume_frame(continuation : FrameContinuation, output : ByteSlice) -> unit =
    match continuation {
        ResumeCall(call) => resume_call(call, output),
        ResumeCreate(create) => resume_create(create, output),
    }
