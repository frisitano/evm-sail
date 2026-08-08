/*md
# The interpreter

The user-space EVM: it fetches and decodes bytecode (Yellow Paper §9),
drives the step loop, and enters sub-frames for the call and create
opcodes. This module specifies that machinery in three layers:

1. **Fetch/decode** — read the opcode at the carried program counter,
   decode PUSH immediates inline, and map every other byte to its AST
   node (an undefined byte decodes to `INVALID`). Reading past the end of
   code yields `STOP` (YP: implicit halt).
2. **Run loop** — [interpret][] steps fetch-then-execute until the active
   frame stops, threading the carried machine state (program counter,
   gas, operand-stack cursor, memory cursor) through every step: each
   step's arguments are supplied from the frame registers and the
   returned state is assigned back, so no handler ever reads or writes
   those registers. Popping a pending
   [FrameContinuation][type-FrameContinuation] resumes a completed child;
   [Empty][type-FrameContinuation] marks completion of the top-level
   frame.
3. **Message calls** — [run_call][] handles
   `CALL`/`CALLCODE`/`DELEGATECALL`/`STATICCALL` (multiplexed on `mode`)
   and [run_create][] handles `CREATE`/`CREATE2`. A sub-call publishes
   the parent's carried state to the frame registers, saves them through
   [frame_stack_push][], installs the child as the active frame, and
   returns the child's carried state to the single run loop. When the
   child halts, that loop restores and resumes the parent. There is no
   recursive interpreter invocation. All world effects go through kernel
   syscalls: [k_journal_checkpoint][] on entry, [k_transfer][] for value,
   and [k_journal_revert][] on failure — the kernel rolls the world back
   atomically on a reverting child. The applicable rules are EIP-150
   (63/64ths gas cap + stipend),
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
function decode_simple(opcode : opcode) -> ast = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if 128 <= opcode & opcode <= 143 then {
        DUP(opcode - 127)
    } else if 144 <= opcode & opcode <= 159 then {
        SWAP(opcode - 143)
    } else if 160 <= opcode & opcode <= 164 then {
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
            30 => if profile.fork >= Osaka then {
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
            72 => if profile.fork >= London then {
                BASEFEE()
            } else {
                INVALID()
            },
            73 => if profile.fork >= Cancun then {
                BLOBHASH()
            } else {
                INVALID()
            },
            74 => if profile.fork >= Cancun then {
                BLOBBASEFEE()
            } else {
                INVALID()
            },
            75 => if profile.fork >= Amsterdam then {
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
            92 => if profile.fork >= Cancun then {
                TLOAD()
            } else {
                INVALID()
            },
            93 => if profile.fork >= Cancun then {
                TSTORE()
            } else {
                INVALID()
            },
            94 => if profile.fork >= Cancun then {
                MCOPY()
            } else {
                INVALID()
            },
            240 => opcode_CREATE(),
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

/*! Fetches and decodes the opcode at the carried program counter,
    returning the counter advanced past the opcode and any immediate.
    Past the end of code the frame implicitly executes `STOP` (YP).
    `PUSH0`–`PUSH32` (0x5f–0x7f) carry an `n`-byte immediate; Amsterdam's
    `DUPN`/`SWAPN`/`EXCHANGE` carry one byte, zero-padded at end of code.
    Every other byte decodes via [decode_simple][]. */
function fetch(current : code_pointer) -> (code_pointer, ast) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let analyzed = frame_code;
    let code = code_bytes(analyzed);
    let code_length = code.len;
    let past_end = not_bool(current < code_length);
    if past_end then {
        (current, STOP())
    } else {
        let opcode_byte = slice_byte(code, current);
        let opcode : opcode = unsigned(opcode_byte);
        let immediate_offset = current + 1;
        let decoded : (code_pointer, ast) =
            if (opcode == 95) & (profile.fork < Shanghai)
            then (immediate_offset, INVALID())
            else if 95 <= opcode & opcode <= 127 then {
                let size : push_width = opcode - 95;
                let value = read_push(code, immediate_offset, size);
                let after_immediate : code_pointer = current + 1 + size;
                (after_immediate, PUSH(size, value))
            } else if profile.fork >= Amsterdam then {
                let deep_operation = deep_stack_operation(opcode);
                match deep_operation {
                    NotDeepStackOperation => (immediate_offset, decode_simple(opcode)),
                    operation => {
                        let immediate = slice_byte(code, immediate_offset);
                        let immediate_valid = deep_stack_operation_immediate_valid(operation, immediate);
                        let after_instruction : code_pointer =
                            if immediate_valid then current + 2 else current + 1;
                        let instruction : ast = match operation {
                            DeepStackDuplicate => DUPN(immediate),
                            DeepStackSwap => SWAPN(immediate),
                            DeepStackExchange => EXCHANGE(immediate),
                            NotDeepStackOperation => decode_simple(opcode),
                        };
                        (after_instruction, instruction)
                    },
                }
            } else {
                (immediate_offset, decode_simple(opcode))
            };
        decoded
    }
}

/*md
## The run loop
*/
val resume_frame : (FrameContinuation, OutputSlice) -> unit

/*! Returns the active frame's halt output. */
function frame_output() -> OutputSlice =
    match frame_status {
        Halted(HaltReturn(output)) => output,
        Halted(HaltRevert(output)) => output,
        _ => EMPTY_OUTPUT_SLICE,
    }

/*! The non-recursive step loop for one complete call tree. It executes
    the active frame, resumes suspended parents through
    [frame_stack_pop][] as children halt, and returns the top-level
    frame's output. Each step's carried state is supplied from the frame
    registers and its returned state is assigned back; the handlers
    themselves never touch the registers. `STOP`, `SELFDESTRUCT`, and
    exceptional halts return the empty slice; `RETURN` and `REVERT` carry
    their frozen memory slice in the halt value. */
function interpret() -> OutputSlice = {
    frame_stack_reset();
    var interpreting : bool = true;
    var result : OutputSlice = EMPTY_OUTPUT_SLICE;

    /* Every continuing opcode consumes at least one unit from the combined
     * execution/state-gas budget. Each additional frame therefore corresponds
     * to an earlier charged CALL or CREATE, and adds at most one zero-cost
     * terminating opcode plus one unwind step. Consequently a complete call
     * tree takes at most 3 * initial gas + 2 interpreter transitions. This
     * proof-only counter does not gate execution; it supplies the decreasing
     * measure required by total extraction targets. */
    let initial_execution_gas = gas_remaining;
    let initial_state_gas = state_gas_remaining;
    let initial_call_tree_gas = initial_execution_gas + initial_state_gas;
    var call_tree_steps_remaining : int = 3 * initial_call_tree_gas + 2;
    while interpreting termination_measure(call_tree_steps_remaining) do {
        let running = is_running();
        if running then {
            let (fetched_pc, instruction) = fetch(pc);
            let (next_pc, next_top, next_mem, next_gas) = execute(
                instruction,
                fetched_pc,
                stack_top,
                evm_memory,
                gas_remaining,
            );
            pc = next_pc;
            stack_top = next_top;
            evm_memory = next_mem;
            gas_remaining = next_gas
        } else {
            let output = frame_output();
            let continuation = frame_stack_pop();
            match continuation {
                Empty() => {
                    result = output;
                    interpreting = false
                },
                continuation => resume_frame(continuation, output),
            }
        };
        call_tree_steps_remaining = call_tree_steps_remaining - 1
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
        let delegate_key = k_code_key(dtgt);
        let delegate_code = code_db_resolve(delegate_key);
        let delegate_precompile = precompile_id_for_address(dtgt);
        if delegate_precompile != NotPrecompile then {
            EMPTY_CODE
        } else {
            delegate_code
        }
    } else {
        let target_key = k_code_key(target);
        code_db_resolve(target_key)
    }

/*md
## The message calls
*/
/*! The behavior selected by one member of the closed CALL-family algebra.
    Interpreting `CallKind` once keeps operand decoding, value transfer, child
    identity, and static-context construction coupled instead of re-matching
    the tag independently at every use site. */
struct CallSemantics = {
    takes_value : bool,
    transfers_value : bool,
    uses_target_address : bool,
    inherits_caller_and_value : bool,
    enters_static_context : bool,
}

/*! Defunctionalizes each CALL-family opcode into the data consumed by the
    shared message-call interpreter. */
function call_semantics(kind : CallKind) -> CallSemantics =
    match kind {
        Call => struct {
            takes_value = true,
            transfers_value = true,
            uses_target_address = true,
            inherits_caller_and_value = false,
            enters_static_context = false,
        },
        CallCode => struct {
            takes_value = true,
            transfers_value = false,
            uses_target_address = false,
            inherits_caller_and_value = false,
            enters_static_context = false,
        },
        DelegateCall => struct {
            takes_value = false,
            transfers_value = false,
            uses_target_address = false,
            inherits_caller_and_value = true,
            enters_static_context = false,
        },
        StaticCall => struct {
            takes_value = false,
            transfers_value = false,
            uses_target_address = true,
            inherits_caller_and_value = false,
            enters_static_context = true,
        },
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
    Pushes 1 on success, 0 on failure. Takes the parent's carried machine
    state; returns the parent's updated state on the non-entering paths
    and the freshly installed child's state after a frame entry. */
function run_call(
    kind : CallKind,
    pc_in : code_pointer,
    top : StackTop,
    mem : EvmMemorySlice,
    g : gas,
) -> (
    (code_pointer, StackTop, EvmMemorySlice, gas)
) = {
    let semantics = call_semantics(kind);
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let current_depth = call_depth;
    let caller = self_addr();
    let (gas_request, top1) = pop(top);
    let (target_word, top2) = pop(top1);
    let target = word_to_address(target_word);
    let (value, top3) : (word, StackTop) =
        if semantics.takes_value then pop(top2) else (WORD_ZERO, top2);
    let value_nonzero = word_nonzero(value);
    let (args_off_word, top4) = pop(top3);
    let (args_len_word, top5) = pop(top4);
    let (ret_off_word, top6) = pop(top5);
    let (ret_len_word, top7) = pop(top6);

    /* EIP-214: a value-bearing CALL inside a static context is a write and
       raises WriteInStaticContext -- an exceptional halt that consumes all the
       caller frame's gas. CALLCODE/DELEGATECALL/STATICCALL never trigger it
       (CALLCODE has no static guard in the spec; the others force value = 0). */
    if semantics.transfers_value & value_nonzero & message.is_static then {
        return (pc_in, top7, mem, exc_halt(g, WriteProtection))
    };

    /* EIP-2929 access: inspect warmth without mutating state, then mark the
       target only after the access charge has been established as payable. */
    let warm = k_account_is_warm(target);
    let target_cost : gas_constant = account_cost(warm);
    let transfer_cost : gas_constant =
        if value_nonzero then call_value_cost() else GAS_CONSTANT_ZERO;

    /* Compute the mathematical endpoints before narrowing either host range.
       Optimized C saturates only its endpoint representation, to a charge
       which is greater than every representable live-gas value. */
    let args_required = memory_required_size(args_off_word, args_len_word);
    let ret_required = memory_required_size(ret_off_word, ret_len_word);
    let required_size =
        if args_required < ret_required then ret_required else args_required;
    let expansion_cost = memory_expansion_cost(mem, required_size);
    let (expansion_affordable, g1) = charge(g, expansion_cost);
    let expansion_out_of_gas = not_bool(expansion_affordable);
    if expansion_out_of_gas then {
        return (pc_in, top7, mem, g1)
    };

    /* Static gas is charged before any state access: the
       target's access + value-transfer cost is checked AFTER memory gas and BEFORE reading
       any of the target's state. A frame that cannot afford it halts here, having
       touched no account -- so the stateless witness need not prove the target.
       (Resolving the EIP-7702 delegation or the empty-account check below reads
       the target's account, which an OOG-before-access tx must never do.) */
    let static_base : gas_cost = target_cost + transfer_cost;
    let (static_base_affordable, g2) = charge(g1, static_base);
    let static_base_out_of_gas = not_bool(static_base_affordable);
    if static_base_out_of_gas then {
        return (pc_in, top7, mem, g2)
    };

    /* The target access happens once the static charge is affordable. Mark it
       before pricing EIP-7702's delegation target: a self-delegation therefore
       pays one cold target access followed by one warm delegation access. */
    k_account_mark_warm(target);

    /* STATE ACCESS (only now that the static gas is covered): the EIP-7702
       delegation designation (reads the target's code) + new-account component. */
    let (tg_deleg, tg_target) = k_deleg_target(target);
    let delegation_cost : gas_constant =
        if tg_deleg then {
            let dw = k_account_is_warm(tg_target);
            account_cost(dw)
        } else {
            GAS_CONSTANT_ZERO
        };
    let target_empty = k_account_is_empty(target);
    let new_account_charged = profile.fork >= Amsterdam & value_nonzero & semantics.transfers_value & target_empty;
    let create_cost : gas_constant =
        if profile.fork < Amsterdam & value_nonzero & semantics.transfers_value & target_empty
        then G_newaccount
        else GAS_CONSTANT_ZERO;
    let additional_cost : gas_cost = delegation_cost + create_cost;
    let (additional_cost_affordable, g3) = charge(g2, additional_cost);
    let additional_cost_out_of_gas = not_bool(additional_cost_affordable);
    if additional_cost_out_of_gas then {
        return (pc_in, top7, mem, g3)
    };
    let stipend : gas =
        if value_nonzero then G_callstipend else GAS_ZERO;

    /* Forwarded child gas, per the EIP-150
       calculate_message_call_gas. The EIP-150 63/64ths cap applies to the gas
       left after the access, value-transfer, memory, and new-account costs. */
    var g4 : gas = g3;
    var base_child : gas = GAS_ZERO;
    if profile.fork >= Amsterdam then {
        /* Amsterdam charges execution work, then state growth, before
           applying EIP-150 to the execution gas left for the child. */
        if new_account_charged then {
            let (state_gas_affordable, state_gas) = charge_state_gas(g3, G_amsterdam_state_new_account);
            g4 = state_gas;
            let state_gas_out_of_gas = not_bool(state_gas_affordable);
            if state_gas_out_of_gas then {
                return (pc_in, top7, mem, state_gas)
            }
        };
        base_child = call_gas_cap_word(g4, gas_request);
        let (child_gas_affordable, child_charged_gas) = charge(g4, base_child);
        g4 = child_charged_gas;
        let child_out_of_gas = not_bool(child_gas_affordable);
        if child_out_of_gas then {
            return (pc_in, top7, mem, child_charged_gas)
        }
    } else {
        base_child = call_gas_cap_word(g4, gas_request);
        let (child_gas_affordable, child_charged_gas) = charge(g4, base_child);
        g4 = child_charged_gas;
        let child_out_of_gas = not_bool(child_gas_affordable);
        if child_out_of_gas then {
            return (pc_in, top7, mem, child_charged_gas)
        }
    };
    if tg_deleg then {
        k_account_mark_warm(tg_target)
    };

    /* gas is now covered; message preparation loads the
       delegate's CODE -- BEFORE the depth/balance guards below. Resolve it now
       (flagging a missing-code witness deficiency) so an insufficient-balance
       call to a 7702-delegated target still detects an omitted delegate-code
       proof; an OOG call returned above and never reaches here. */
    if tg_deleg then {
        let delegate_key = k_code_key(tg_target);
        let _ = code_db_resolve(delegate_key);

        /* The resolved delegate's account is read for its code, so the
           delegate is touched (kept in the BAL account set). */
        let _ = k_aload(tg_target)
    };
    let args_access = memory_access(args_off_word, args_len_word);
    let ret_access = memory_access(ret_off_word, ret_len_word);
    let materialized_required_size =
        if args_access.required_size < ret_access.required_size
        then ret_access.required_size
        else args_access.required_size;
    let mem1 = expand_memory(mem, materialized_required_size);
    let args = args_access.range;
    let ret = ret_access.range;
    let child_gas : gas = conserved_gas_add(base_child, stipend);

    /* The target's code hash is read for every call once gas is
     * charged, before the depth/balance guards -- so the call target is always a
     * state access (kept in the BAL account set), including precompiles and calls
     * that then fail the depth/balance guard below. */
    let _ = k_aload(target);

    /* call-failure guards (checked AFTER gas is charged): the depth ceiling
       (EIP-150, 1024 frames) and, for a value-bearing CALL/CALLCODE, the
       caller's balance covering the transfer. */
    let insufficient_balance : bool =
        if semantics.takes_value & value_nonzero then {
            let caller_balance = k_get_balance(caller);
            let transfer_affordable = word_ule(value, caller_balance);
            not_bool(transfer_affordable)
        } else {
            false
        };
    let depth_limit = sizeof(call_depth_limit);
    if insufficient_balance | (current_depth == depth_limit) then {
        returndata_clear();
        let g5 = refund_gas(g4, child_gas);
        let g6 : gas =
            if new_account_charged then credit_state_gas_refund(g5, G_amsterdam_state_new_account) else g5;
        (pc_in, push(top7, WORD_ZERO), mem1, g6)
    } else {
        let selected_precompile = precompile_id_for_address(target);
        if selected_precompile != NotPrecompile then {
            /* PRECOMPILE call. The precompile set is fork-gated (the highest is
               0x100 P256VERIFY, EIP-7951/Osaka); an address outside the active
               set is an ordinary code call. The precompile's input is the
               child message's calldata: mem[args_off .. args_off+args_len).
               Gas inspects the same memory source before execution. */
            let (input_memory, mem2) = active_memory_slice(mem1, args.off, args.len);
            let input = MemoryCalldata(input_memory);

            /* Gas is checked before execution: an unpayable/OOG precompile call
             * must not execute (a BLAKE2F request may carry an enormous round
             * count). Failure consumes all child gas and exposes no returndata. */
            let precompile_charge = precompile_gas(selected_precompile, input, child_gas);
            if precompile_charge.affordable then {
                let used = precompile_charge.cost;
                let result = run_precompile_slice(selected_precompile, input);
                if result.success then {
                    returndata = result.output;

                    /* the value transfer is part of the successful call */
                    if semantics.transfers_value & value_nonzero then {
                        k_transfer(caller, target, value)
                    };
                    returndata_copy_prefix(ret.off, ret.len);
                    let unused : gas = gas_sub(child_gas, used);
                    let g5 = refund_gas(g4, unused);
                    (pc_in, push(top7, WORD_ONE), mem2, g5)
                } else {
                    returndata_clear();
                    let g5 : gas =
                        if new_account_charged then credit_state_gas_refund(g4, G_amsterdam_state_new_account) else g4;
                    (pc_in, push(top7, WORD_ZERO), mem2, g5)
                }
            } else {
                returndata_clear();
                let g5 : gas =
                    if new_account_charged then credit_state_gas_refund(g4, G_amsterdam_state_new_account) else g4;
                (pc_in, push(top7, WORD_ZERO), mem2, g5)
            }
        } else {
            /* CODE call. Snapshot the world (so a reverting child can be rolled
               back), then perform the CALL value transfer up front. */
            let child_depth : frame_depth = current_depth + 1;
            let child_code = executable_code(target, tg_deleg, tg_target);

            /* the child message context, by call mode:
               - address: target for CALL/STATICCALL; the CALLER's own address
                 for CALLCODE/DELEGATECALL (they run target's code in place);
               - caller/value: inherited from the parent for DELEGATECALL,
                 else the immediate caller and the call's value;
               - static: forced for STATICCALL, else inherited (a static frame
                 stays static for all of its sub-calls). */
            let child_addr : address =
                if semantics.uses_target_address then target else caller;
            let child_caller : address =
                if semantics.inherits_caller_and_value then message.caller else caller;
            let child_value : word =
                if semantics.inherits_caller_and_value then message.value else value;
            let child_static : bool =
                if semantics.enters_static_context then true else message.is_static;
            let (bytes, mem2) = active_memory_slice(mem1, args.off, args.len);
            let child_memory = evm_memory_slice(bytes.bytes, bytes.len);
            let child_calldata = MemoryCalldata(child_memory);
            let child_state_gas = state_gas_remaining;

            /* Publish the parent's carried state: suspend_frame reads the
               frame registers at this one authoritative boundary. */
            pc = pc_in;
            gas_remaining = g4;
            stack_top = top7;
            evm_memory = mem2;
            let checkpoint = { suspend_frame() with state_gas_remaining = GAS_ZERO };
            let call_continuation : CallContinuation = struct {
                    checkpoint = checkpoint,
                    return_offset = ret.off,
                    return_length = ret.len,
                    new_account_charged = new_account_charged,
                };
            let continuation = ResumeCall(call_continuation);
            frame_stack_push(continuation);
            if semantics.transfers_value & value_nonzero then {
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
                    depth = child_depth,
                };
            calldata_install(child_calldata);
            pc = 0;
            gas_remaining = child_gas;
            state_gas_remaining = child_state_gas;
            state_gas_spilled = STATE_GAS_SPILL_ZERO;
            frame_status = Running();
            returndata_clear();
            frame_code = child_code;
            call_depth = child_depth;
            frame_refund = GAS_REFUND_ZERO;
            (0, stack_top, evm_memory, child_gas)
        }
    }
}

/*! The behavior selected by one member of the closed CREATE-family algebra.
    Interpreting `CreateKind` once keeps operand decoding, hashing charges, and
    address derivation coupled rather than passing an unexplained boolean
    through the shared creation path. */
struct CreateSemantics = {
    uses_salt : bool,
}

/*! Defunctionalizes each CREATE-family opcode into the data consumed by the
    shared contract-creation interpreter. */
function create_semantics(kind : CreateKind) -> CreateSemantics =
    match kind {
        CreateByNonce => struct { uses_salt = false },
        CreateBySalt => struct { uses_salt = true },
    }

/*! `CREATE` (`CreateByNonce`) and `CREATE2` (`CreateBySalt`, EIP-1014).
    Both deploy a new contract by running the initcode supplied in
    memory; the new address derives from `(creator, nonce)` for `CREATE`
    or `(creator, salt, keccak256(initcode))` for `CREATE2`. Operand
    layout: `value`, `offset`, `length`, `salt` (for `CREATE2`). Pushes
    the new address on success, 0 on failure. Takes the parent's carried
    machine state; returns the parent's updated state on the non-entering
    paths and the freshly installed child's state after a frame entry. */
function run_create(
    kind : CreateKind,
    pc_in : code_pointer,
    top : StackTop,
    mem : EvmMemorySlice,
    g : gas,
) -> (
    (code_pointer, StackTop, EvmMemorySlice, gas)
) = {
    let semantics = create_semantics(kind);
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let current_depth = call_depth;
    let creator = self_addr();
    let (value, top1) = pop(top);
    let (off_word, top2) = pop(top1);
    let (len_word, top3) = pop(top2);
    let (salt, top4) : (word, StackTop) =
        if semantics.uses_salt then pop(top3) else (WORD_ZERO, top3);

    /* EIP-214: CREATE/CREATE2 modifies state and is forbidden in a static
       context -- it raises WriteProtection (an exceptional halt consuming all
       remaining gas), checked before any charge or child frame. */
    let (static_violation, g0) = guard_static(g);
    if static_violation then {
        return (pc_in, top4, mem, g0)
    };
    let required_size = memory_required_size(off_word, len_word);
    let expansion_cost = memory_expansion_cost(mem, required_size);
    let (expansion_affordable, g1) = charge(g0, expansion_cost);
    let expansion_out_of_gas = not_bool(expansion_affordable);
    if expansion_out_of_gas then {
        return (pc_in, top4, mem, g1)
    };
    let initcode_access = memory_access(off_word, len_word);
    let mem1 = expand_memory(mem, initcode_access.required_size);
    let initcode = initcode_access.range;

    /* up-front charges: memory expansion over the initcode, the CREATE base
       cost (G_create), the EIP-3860 per-initcode-word cost, and -- for CREATE2
       only -- the keccak word cost of hashing the initcode for the address. */
    let access_cost = create_access_cost();
    let (access_affordable, g2) = charge(g1, access_cost);
    let access_out_of_gas = not_bool(access_affordable);
    if access_out_of_gas then {
        return (pc_in, top4, mem1, g2)
    };
    var g3 : gas = g2;
    let initcode_word_count = memory_word_count_word(len_word);
    if profile.fork >= Shanghai then {
        let (initcode_charge_affordable, initcode_gas) = charge_word_scaled_gas(
            g3,
            G_initcode_word,
            initcode_word_count,
        );
        g3 = initcode_gas;
        let initcode_charge_out_of_gas = not_bool(initcode_charge_affordable);
        if initcode_charge_out_of_gas then {
            return (pc_in, top4, mem1, initcode_gas)
        }
    };
    if semantics.uses_salt then {
        let (hashing_affordable, hashing_gas) = charge_word_scaled_gas(g3, G_keccak_word, initcode_word_count);
        g3 = hashing_gas;
        let hashing_out_of_gas = not_bool(hashing_affordable);
        if hashing_out_of_gas then {
            return (pc_in, top4, mem1, hashing_gas)
        }
    };

    /* EIP-3860: initcode longer than MAX_INITCODE_SIZE is rejected outright as
       an exceptional halt. */
    let valid_initcode_size = initcode_size_allowed(initcode.len);
    let invalid_initcode_size = not_bool(valid_initcode_size);
    if invalid_initcode_size then {
        (pc_in, top4, mem1, exc_halt(g3, InitCodeTooLarge))
    } else {
        /* derive the new contract address: keccak(0xff, creator, salt,
           keccak(initcode)) for CREATE2 (EIP-1014), else rlp(creator, nonce). */
        let nonce = k_get_nonce(creator);
        var mem2 : EvmMemorySlice = mem1;
        let new_addr : address =
            if semantics.uses_salt then {
                let (initcode_digest_word, hashed_mem) = mem_keccak(mem1, initcode);
                mem2 = hashed_mem;
                let initcode_digest = word_to_hash(initcode_digest_word);
                k_create2_addr(creator, salt, initcode_digest)
            } else {
                k_create_addr(creator, nonce)
            };

        /* Before Amsterdam the child allocation is computed before the
           early guards (and returned if they fail). Amsterdam first charges
           any account-growth state gas, because a spill reduces the execution
           gas to which the EIP-150 cap applies. */
        var g4 : gas = g3;
        var child_gas : gas = GAS_ZERO;
        if profile.fork < Amsterdam then {
            let avail = g4;
            let retained_gas : gas = avail / 64;
            child_gas = gas_sub(avail, retained_gas);
            g4 = retained_gas
        };

        /* EIP-214: a CREATE in a static
         * context (e.g. inside a STATICCALL frame) is a write and raises
         * WriteInStaticContext after the child gas is reserved -- an
         * exceptional halt that consumes all gas, bumps no nonce, runs no
         * initcode. */
        if message.is_static then {
            return (pc_in, top4, mem2, exc_halt(g4, WriteProtection))
        };

        /* early-abort guards (child gas refunded, 0 pushed, NO nonce bump): the
           depth ceiling, the creator's balance covering the endowment, and the
           creator nonce not already at the 2^64-1 ceiling (it must be
           incrementable). */
        let creator_balance = k_get_balance(creator);
        let endowment_affordable = word_ule(value, creator_balance);
        let insufficient_balance = not_bool(endowment_affordable);
        let nonce_limit = sizeof(account_nonce_bound);
        let depth_limit = sizeof(call_depth_limit);
        if   insufficient_balance
           | (nonce == nonce_limit)
           | /* EIP-2681 */
             (current_depth == depth_limit) then {
            returndata_clear();
            let g5 : gas =
                if profile.fork < Amsterdam then refund_gas(g4, child_gas) else g4;
            (pc_in, push(top4, WORD_ZERO), mem2, g5)
        } else {
            /* warm the new address and bump the creator nonce BEFORE the child
             * snapshot: both persist even if the initcode reverts (the
             * the protocol runs them in the parent, before
             * process_create_message). */
            let child_depth : frame_depth = current_depth + 1;
            k_account_mark_warm(new_addr);
            let new_account_charged = (profile.fork >= Amsterdam) & k_account_is_empty(new_addr);
            if new_account_charged then {
                let (state_gas_affordable, state_gas) = charge_state_gas(g4, G_amsterdam_state_new_account);
                g4 = state_gas;
                let state_gas_out_of_gas = not_bool(state_gas_affordable);
                if state_gas_out_of_gas then {
                    return (pc_in, top4, mem2, state_gas)
                }
            };
            if profile.fork >= Amsterdam then {
                let avail = g4;
                let retained_gas : gas = avail / 64;
                child_gas = gas_sub(avail, retained_gas);
                g4 = retained_gas
            };
            let occupied : bool = k_account_occupied(new_addr);
            returndata_clear();
            k_bump_nonce(creator);
            if occupied then {
                /* address collision (the target already has code, a nonzero
                 * nonce, or storage): no initcode runs, 0 is pushed, and the
                 * reserved child gas is NOT refunded (it stays deducted). */
                let g5 : gas =
                    if new_account_charged then credit_state_gas_refund(g4, G_amsterdam_state_new_account) else g4;
                (pc_in, push(top4, WORD_ZERO), mem2, g5)
            } else {
                /* Freeze the initcode while parent memory is active, then
                   suspend the complete parent before mutating the new account.
                   The child frame has no calldata. */
                let (initcode_bytes, mem3) = memory_code_slice(mem2, initcode.off, initcode.len);
                let child_code_id = code_db_insert(initcode_bytes, profile.fork);
                let child_code = code_db_resolve(child_code_id);
                let child_state_gas = state_gas_remaining;

                /* Publish the parent's carried state: suspend_frame reads
                   the frame registers at this one authoritative boundary. */
                pc = pc_in;
                gas_remaining = g4;
                stack_top = top4;
                evm_memory = mem3;
                let checkpoint = { suspend_frame() with state_gas_remaining = GAS_ZERO };
                let create_continuation : CreateContinuation = struct {
                        checkpoint = checkpoint,
                        address = new_addr,
                        new_account_charged = new_account_charged,
                    };
                let continuation = ResumeCreate(create_continuation);
                frame_stack_push(continuation);
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
                        depth = child_depth,
                    };
                calldata_install(EMPTY_CALLDATA);
                pc = 0;
                gas_remaining = child_gas;
                state_gas_remaining = child_state_gas;
                state_gas_spilled = STATE_GAS_SPILL_ZERO;
                frame_status = Running();
                returndata_clear();
                frame_code = child_code;
                call_depth = child_depth;
                frame_refund = GAS_REFUND_ZERO;
                (0, stack_top, evm_memory, child_gas)
            }
        }
    }
}

/*! Restores a message-call parent and applies the child's outcome. The
    interpreter has published the child's final carried state to the frame
    registers; this boundary code works through them and republishes the
    parent's stack cursor after pushing the result. */
function resume_call(continuation : CallContinuation, output : OutputSlice) -> unit = {
    returndata = output;
    let checkpoint = continuation.checkpoint;
    let succeeded = frame_succeeded();
    let child_left = gas_remaining;
    let child_state_left = state_gas_remaining;
    let child_state_spill = state_gas_spilled;
    let child_refund = frame_refund;

    restore_frame(checkpoint);
    gas_remaining = refund_gas(gas_remaining, child_left);
    return_child_state_gas(child_state_left, child_state_spill);

    /* Both RETURN and REVERT copy their output; exceptional halts carry the
     * empty slice. Successful effects remain, while every failure reverts to
     * the saved kernel checkpoint. */
    returndata_copy_prefix(continuation.return_offset, continuation.return_length);
    if succeeded then {
        record_refund(child_refund);
        k_journal_commit();
        stack_top = push(stack_top, WORD_ONE)
    } else {
        k_journal_revert();
        if continuation.new_account_charged then {
            gas_remaining = credit_state_gas_refund(gas_remaining, G_amsterdam_state_new_account)
        };
        stack_top = push(stack_top, WORD_ZERO)
    }
}

/*! Restores a create parent and either deploys or rolls back the child. */
function resume_create(continuation : CreateContinuation, output : OutputSlice) -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    returndata = output;
    let checkpoint = continuation.checkpoint;
    let initcode_succeeded = frame_succeeded();
    let deployed_length = returndata_size();
    let deployed_size = deployed_length;
    var frontier_empty_deposit : bool = false;

    if initcode_succeeded then {
        let deployed_size_allowed = deployed_code_size_allowed(deployed_size);
        let invalid_deployed_size = not_bool(deployed_size_allowed);
        let prohibited_prefix =
            if deployed_size != 0 then {
                let first_byte = slice_byte(returndata, 0);
                first_byte == 0xef
            } else {
                false
            };
        if invalid_deployed_size | ((profile.fork >= London) & prohibited_prefix) then {
            /* EIP-170 and EIP-3541 failures use the post-Homestead
             * exceptional creation path. Neither rule is active on
             * Frontier/Homestead in a way that reaches this branch. */
            gas_remaining = exc_halt(gas_remaining, OutOfGas)
        } else {
            let deployment_charge = code_deployment_execution_cost(deployed_length, gas_remaining);
            if deployment_charge.affordable then {
                let execution_deposit = deployment_charge.cost;
                gas_remaining = gas_sub(gas_remaining, execution_deposit);
                let state_deposit = code_deployment_state_cost(deployed_length);
                let (_, deployment_gas) = charge_deployment_state_gas(gas_remaining, state_deposit);
                gas_remaining = deployment_gas
            } else if profile.fork < Homestead then {
                /* Frontier consumed the remaining child gas but kept the
                 * creation successful with empty deployed code. EIP-2
                 * changed this to exceptional failure at Homestead. */
                gas_remaining = GAS_ZERO;
                frontier_empty_deposit = true
            } else {
                gas_remaining = exc_halt(gas_remaining, OutOfGas)
            }
        }
    };
    let deploy_succeeds = initcode_succeeded & frame_succeeded();
    let child_left = gas_remaining;
    let child_state_left = state_gas_remaining;
    let child_state_spill = state_gas_spilled;
    let child_refund = frame_refund;

    restore_frame(checkpoint);
    gas_remaining = refund_gas(gas_remaining, child_left);
    return_child_state_gas(child_state_left, child_state_spill);

    if deploy_succeeds then {
        record_refund(child_refund);
        let deployed_bytes : OutputSlice =
            if frontier_empty_deposit then EMPTY_OUTPUT_SLICE else returndata;
        let deployed_code = code_db_intern_output(deployed_bytes);
        k_deploy_code(continuation.address, deployed_code);
        k_journal_commit();
        let deployed_address = address_to_word(continuation.address);
        stack_top = push(stack_top, deployed_address)
    } else {
        k_journal_revert();
        if continuation.new_account_charged then {
            gas_remaining = credit_state_gas_refund(gas_remaining, G_amsterdam_state_new_account)
        };
        stack_top = push(stack_top, WORD_ZERO)
    };

    /* Successful initcode output has become deployed code. REVERT output
     * remains visible to RETURNDATA opcodes in the parent. */
    if initcode_succeeded then {
        returndata_clear()
    }
}

/*! Applies the pending operation for one completed child frame. */
function resume_frame(continuation : FrameContinuation, output : OutputSlice) -> unit =
    match continuation {
        Empty() => fatal_error(ExecutionInvalid),
        ResumeCall(call) => resume_call(call, output),
        ResumeCreate(create) => resume_create(create, output),
    }
