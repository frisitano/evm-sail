/* ======================================================================== */
/* THE INTERPRETER: FETCH/DECODE, RUN LOOP, AND THE MESSAGE-CALL MACHINERY  */
/*                                                                          */
/* The user-space EVM: it fetches and decodes bytecode (Yellow Paper        */
/* section 9), drives the step loop, and spawns sub-frames for the call and */
/* create opcodes. This module specifies that machinery in three layers.    */
/*                                                                          */
/* (1) Fetch/decode: read the opcode at pc, decode PUSH immediates inline,  */
/* and map every other byte to its AST node (an undefined byte decodes to   */
/* INVALID). Reading past the end of code yields STOP (YP: implicit halt).  */
/*                                                                          */
/* (2) Run loop: interpret() steps fetch-then-execute until the frame stops */
/* running (normal halt, REVERT, or exceptional halt).                      */
/*                                                                          */
/* (3) Message calls: run_call handles                                      */
/* CALL/CALLCODE/DELEGATECALL/STATICCALL (selected by `kind`) and           */
/* run_create handles CREATE/CREATE2. A sub-call saves the parent's user-   */
/* space frame registers, runs the child to a halt, then restores them. All */
/* The complete parent frame, including a kernel state checkpoint, is       */
/* suspended before child effects begin and restored on return. k_revert    */
/* rolls the world back atomically when the child fails. The applicable     */
/* rules are EIP-150 (63/64ths gas cap + stipend), EIP-214 (static-context  */
/* write protection), EIP-2929 (cold/warm access), and EIP-7702 (delegated- */
/* code execution).                                                         */
/* ======================================================================== */

/* EIP-150 / YP: a message call may nest at most 1024 frames deep. */
let DEPTH_LIMIT : frame_depth = 1024

/* ----------------------------- fetch/decode ----------------------------- */

/* Assemble the n-byte big-endian immediate following a PUSHn opcode into a
 * 256-bit word; bytes past the end of code read as zero. */
function read_push(n : code_length) -> word = slice_load_n(frame_code.bytes, pc, n)

function advance_pc(delta : code_length) -> unit = pc = pc + delta

/* Decode one non-PUSH opcode byte to its AST node. The three contiguous
 * families fold to an arity argument -- DUP1..16 (0x80..0x8f), SWAP1..16
 * (0x90..0x9f), LOG0..4 (0xa0..0xa4) -- and the remainder is a flat table.
 * Any byte with no defined opcode decodes to INVALID. */
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

/* Fetch and decode the opcode at pc, advancing pc past the opcode and any
 * immediate. Past the end of code the frame implicitly executes STOP (YP).
 * PUSH0..PUSH32 (0x5f..0x7f) carry an n-byte immediate consumed here; every
 * other byte is decoded by decode_simple. */
function fetch() -> ast = {
    if not_bool(pc < frame_code_len()) then {
        STOP()
    } else {
        let opcode : opcode = unsigned(slice_byte(frame_code.bytes, pc));
        advance_pc(BYTE_ONE);
        if (opcode == 95) & (k_fork < Shanghai) then {
            INVALID()
        } else {
            if 95 <= opcode & opcode <= 127 then {
                let encoded_size : push_width = opcode - 95;
                let size : code_length = ByteQuantity(encoded_size);
                let value = read_push(size);
                advance_pc(size);
                PUSH(encoded_size, value)
            } else {
                decode_simple(opcode)
            }
        }
    }
}

/* The step loop returns the frame's own output. STOP, SELFDESTRUCT, and
 * exceptional halts return the empty slice; RETURN and REVERT carry their
 * frozen memory slice in the halt value. */
function interpret() -> ByteSlice = {
    while is_running() do {
        let op = fetch();
        execute(op)
    };
    match frame_status {
        Halted(HaltReturn(output)) => output,
        Halted(HaltRevert(output)) => output,
        _ => EMPTY_SLICE,
    }
}

/* Did the just-finished frame end successfully? A normal halt succeeds; a
 * REVERT and any exceptional halt do not (their world effects are rolled
 * back and CALL/CREATE reports failure). */
function frame_succeeded() -> bool =
    match frame_status {
        Halted(HaltRevert(_)) => false,
        Halted(_) => true,
        Running() => true,
        Exceptional(_) => false,
    }

/* Select the code a frame actually executes (EIP-7702). A delegated account
 * runs the code at its delegation target, following exactly ONE hop; a
 * delegation whose target is a precompile (or has no code) executes as empty
 * code. An undelegated account runs its own code. */
function executable_code(target : address, dele : bool, dtgt : address) -> Code =
    if dele then {
        match word_to_precompile_id(address_to_word(dtgt)) {
            Some(number) if is_precompile(number) => EMPTY_CODE,
            _ => code_db_resolve(k_code_key(dtgt)),
        }
    } else {
        code_db_resolve(k_code_key(target))
    }

/* ------------------------------ run_call -------------------------------- */
function call_takes_value(kind : CallKind) -> bool =
    match kind {
        Call => true,
        CallCode => true,
        _ => false,
    }

function call_transfers_value(kind : CallKind) -> bool =
    match kind {
        Call => true,
        _ => false,
    }

function call_uses_target_address(kind : CallKind) -> bool =
    match kind {
        Call => true,
        StaticCall => true,
        _ => false,
    }

function call_is_delegate(kind : CallKind) -> bool =
    match kind {
        DelegateCall => true,
        _ => false,
    }

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
function run_call(kind : CallKind) -> unit = {
    let caller = self_addr();
    let gas_request = pop();
    let target = word_to_address(pop());
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
       (exc_halt zeroes the gas). The execution-specs unwind the whole op via
       an exception; Sail has no unwind, so bail out here before any gas is
       re-added below. */
    if not_bool(is_running()) then {
        return ()
    };

    /* Memory is expanded once to cover the larger input/output extent. A
       zero-sized operand requires no memory, irrespective of its offset. */
    let required_memory_size = maximum_memory_required_size(args_off_word, args_len_word, ret_off_word, ret_len_word);

    /* memory-expansion cost is folded into the single message-call charge
       below: the execution-specs charge message_call_gas.cost together with
       extend_memory.cost, then grow memory only if the frame did not run out
       of gas. */
    let mem_gas = memory_expansion_gas(required_memory_size);

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
    let transfer_cost : gas_constant = if word_nonzero(value) then { G_callvalue } else { GAS_CONSTANT_ZERO };
    let static_cost = target_cost + transfer_cost + mem_gas;

    /* "check static gas before state access" (execution-specs generic_call): the
       target's access + value-transfer + memory cost is checked BEFORE reading
       any of the target's state. A frame that cannot afford it halts here, having
       touched no account -- so the stateless witness need not prove the target.
       (Resolving the EIP-7702 delegation or the empty-account check below reads
       the target's account, which an OOG-before-access tx must never do.) */
    if not_bool(is_running()) | gas_remaining < static_cost then {
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
    let access_cost = target_cost + delegation_cost;
    let create_cost : gas_constant = if word_nonzero(value) & call_transfers_value(kind) & k_account_is_empty(target) then {
            G_newaccount
        } else {
            GAS_CONSTANT_ZERO
        };
    let extra_gas = access_cost + transfer_cost + create_cost;
    let required = extra_gas + mem_gas;
    let stipend : gas = if word_nonzero(value) then { G_callstipend } else { GAS_ZERO };

    if not_bool(is_running()) then {
        return ()
    };

    /* Forwarded child gas, mirroring the execution-specs
       calculate_message_call_gas. If the frame cannot afford extra_gas +
       mem_gas, charging that required component already forces the same
       exceptional halt, so no transaction-controlled request value needs to
       be narrowed. Otherwise the EIP-150 63/64ths cap applies to the gas left
       after subtracting mem_gas and extra_gas. */
    let avail = gas_remaining;
    let base_child : gas = if avail < required then {
            GAS_ZERO
        } else {
            let available_after_cost = gas_sub_or_oog(avail, required);
            call_gas_cap_word(available_after_cost, gas_request)
        };

    /* one combined charge: cost (= base_child + extra_gas) + mem_gas. A frame
       that cannot cover this exceptionally halts (all gas consumed) BEFORE the
       child runs and BEFORE the stipend is granted. */
    let total_charge = gas_to_cost(base_child) + required;
    charge(total_charge);
    if not_bool(is_running()) then {
        return ()
    };

    /* gas is now covered, so EELS proceeds to prepare_message, which loads the
       delegate's CODE -- BEFORE the depth/balance guards below. Resolve it now
       (flagging a missing-code witness deficiency) so an insufficient-balance
       call to a 7702-delegated target still detects an omitted delegate-code
       proof, matching EELS; an OOG call returned above and never reaches here. */
    if tg_deleg then {
        let _ = code_db_resolve(k_code_key(tg_target));

        /* EELS reads get_account(code_address) on the resolved delegate, so the
           delegate is touched (kept in the BAL account set). */
        let _ = k_aload(tg_target)
    };
    expand_charged_memory(required_memory_size);
    if not_bool(is_running()) then {
        return ()
    };
    let Gas(base_child_amount) = base_child;
    let Gas(stipend_amount) = stipend;
    let child_gas : gas = if stipend_amount <= (2 ^ 63 - 1) - base_child_amount then {
            Gas(base_child_amount + stipend_amount)
        } else {
            throw InvalidBlock(ExecutionInvalid)
        };
    if not_bool(is_running()) then {
        return ()
    };

    /* EELS reads get_account(code_address).code_hash for every call once gas is
     * charged, before the depth/balance guards -- so the call target is always a
     * state access (kept in the BAL account set), including precompiles and calls
     * that then fail the depth/balance guard below. */
    let _ = k_aload(target);

    /* call-failure guards (checked AFTER gas is charged): the depth ceiling
       (EIP-150, 1024 frames) and, for a value-bearing CALL/CALLCODE, the
       caller's balance covering the transfer. */
    let depth_ok = call_depth < DEPTH_LIMIT;
    let balance_ok : bool = if call_takes_value(kind) & word_nonzero(value) then {
            word_ule(value, k_get_balance(caller))
        } else {
            true
        };

    if not_bool(depth_ok) | not_bool(balance_ok) then {
        /* the call does not happen: the child gas (including stipend) is
           refunded to the caller and CALL pushes 0. */
        returndata_clear();
        refund_gas(child_gas);
        push(WORD_ZERO)
    } else {
        match word_to_precompile_id(address_to_word(target)) {
            Some(precompile_number) if is_precompile(precompile_number) => {
                /* PRECOMPILE call. The precompile set is fork-gated (the highest is
                   0x100 P256VERIFY, EIP-7951/Osaka); an address outside the active
                   set is an ordinary code call. The precompile's input is the
                   child message's calldata: mem[args_off .. args_off+args_len).
                   Gas inspects the same memory source before execution. */
                let input = memory_byte_slice(
                    ByteQuantity(unsigned(args_off_word)),
                    ByteQuantity(unsigned(args_len_word)),
                );

                /* Gas is checked before execution: an unpayable/OOG precompile call
                 * must not execute (a BLAKE2F request may carry an enormous round
                 * count). Failure consumes all child gas and exposes no returndata. */
                match precompile_gas(precompile_number, input) {
                    Some(used) if used <= child_gas => {
                        let result = run_precompile_slice(precompile_number, input);
                        if result.success then {
                            returndata = result.output;

                            /* the value transfer is part of the successful call */
                            if call_transfers_value(kind) & word_nonzero(value) then {
                                k_transfer(caller, target, value)
                            };
                            returndata_copy_prefix(
                                ByteQuantity(unsigned(ret_off_word)),
                                ByteQuantity(unsigned(ret_len_word)),
                            );
                            let unused = gas_sub_or_oog(child_gas, used);
                            if is_running() then {
                                refund_gas(unused);
                                push(WORD_ONE)
                            }
                        } else {
                            returndata_clear();
                            push(WORD_ZERO)
                        }
                    },
                    _ => {
                        returndata_clear();
                        push(WORD_ZERO)
                    },
                }
            },
            _ => {
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
                        memory_byte_slice(ByteQuantity(unsigned(args_off_word)), ByteQuantity(unsigned(args_len_word)))
                    } else {
                        EMPTY_SLICE
                    };
                let checkpoint = suspend_frame();
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
                        is_static = child_static,
                        depth = frame_depth_increment(checkpoint.call_depth),
                    };
                calldata_install(child_calldata);
                pc = BYTE_ZERO;
                gas_remaining = child_gas;
                frame_status = Running();
                returndata_clear();
                frame_code = child_code;
                call_depth = frame_depth_increment(checkpoint.call_depth);
                frame_refund = GAS_REFUND_ZERO;

                returndata = interpret();

                let succeeded = frame_succeeded();
                let child_left = gas_remaining;
                let child_refund = frame_refund;

                /* leave the child: restore every saved parent register, then credit
                   the parent with the child's unspent gas. */
                restore_frame(checkpoint);
                refund_gas(child_left);

                /* copy returndata into the caller's output region on both success
                   AND revert (execution-specs generic_call), writing only
                   min(ret_len, |output|) bytes and leaving the rest untouched. An
                   exceptionally-halted child has empty output, so nothing is
                   written. The child's world effects remain on success and are
                   rolled back on failure. */
                returndata_copy_prefix(ByteQuantity(unsigned(ret_off_word)), ByteQuantity(unsigned(ret_len_word)));
                if succeeded then {
                    record_refund(child_refund);
                    push(WORD_ONE)
                } else {
                    k_revert(checkpoint.state);
                    push(WORD_ZERO)
                }
            },
        }
    }
}

/* ----------------------------- run_create ------------------------------- */
/* CREATE (is2 = false) and CREATE2 (is2 = true, EIP-1014). Both deploy a new
 * contract by running the initcode supplied in memory; the new address is
 * derived from (creator, nonce) for CREATE or (creator, salt, initcode hash)
 * for CREATE2. Operand layout: value, offset, length, [salt for CREATE2].
 * Pushes the new address on success, 0 on failure. */
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

    let initcode_size : nat = unsigned(len_word);

    /* EIP-214: CREATE/CREATE2 modifies state and is forbidden in a static
       context -- it raises WriteProtection (an exceptional halt consuming all
       remaining gas), checked before any charge or child frame. */
    if guard_static() then {
        return ()
    };

    /* up-front charges: memory expansion over the initcode, the CREATE base
       cost (G_create), the EIP-3860 per-initcode-word cost, and -- for CREATE2
       only -- the keccak word cost of hashing the initcode for the address. */
    charge_memory_expansion(off_word, len_word);
    charge(G_create);
    charge(initcode_gas(initcode_size));
    if is2 then {
        charge(G_keccak_word * memory_word_count(initcode_size))
    };

    /* an out-of-gas during those charges is an exceptional halt (all gas
       consumed); bail before any nonce bump or child allocation. */
    if not_bool(is_running()) then {
        return ()
    };

    /* EIP-3860: initcode longer than MAX_INITCODE_SIZE is rejected outright as
       an exceptional halt. */
    if not_bool(initcode_size_allowed(initcode_size)) then {
        exc_halt(InitCodeTooLarge)
    } else {
        /* derive the new contract address: keccak(0xff, creator, salt,
           keccak(initcode)) for CREATE2 (EIP-1014), else rlp(creator, nonce). */
        let nonce = k_get_nonce(creator);
        let new_addr : address = if is2 & is_running() then {
                k_create2_addr(creator, salt, mem_keccak(ByteQuantity(unsigned(off_word)), ByteQuantity(initcode_size)))
            } else {
                k_create_addr(creator, nonce)
            };

        /* reserve all-but-one-64th of the remaining gas for the initcode frame
           (EIP-150); the retained 1/64th stays with the creating frame. */
        let avail = gas_remaining;
        let retained_gas = gas_quotient(avail, 64);
        let child_gas = gas_sub_or_oog(avail, retained_gas);
        if not_bool(is_running()) then {
            return ()
        };
        gas_remaining = retained_gas;

        /* EIP-214 (execution-specs generic_create): a CREATE in a static
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
        let depth_ok = call_depth < DEPTH_LIMIT;
        let balance_ok : bool = word_ule(value, k_get_balance(creator));
        let nonce_ok : bool = nonce != 2 ^ 64 - 1; /* EIP-2681 */

        if not_bool(depth_ok) | not_bool(balance_ok) | not_bool(nonce_ok) then {
            returndata_clear();
            refund_gas(child_gas);
            push(WORD_ZERO)
        } else {
            /* warm the new address and bump the creator nonce BEFORE the child
             * snapshot: both persist even if the initcode reverts (the
             * execution-specs run them in the parent, before
             * process_create_message). */
            let _ = k_access_account(new_addr);
            let occupied : bool = k_account_occupied(new_addr);
            returndata_clear();
            k_bump_nonce(creator);
            if occupied then {
                /* address collision (the target already has code, a nonzero
                 * nonce, or storage): no initcode runs, 0 is pushed, and the
                 * reserved child gas is NOT refunded (it stays deducted). */
                push(WORD_ZERO)
            } else {
                /* Freeze the initcode while parent memory is active, then
                   suspend the complete parent before mutating the new account.
                   The child frame has no calldata. */
                let initcode = if is_running() then {
                        memory_byte_slice(ByteQuantity(unsigned(off_word)), ByteQuantity(initcode_size))
                    } else {
                        EMPTY_SLICE
                    };
                let child_code = code_db_resolve(code_db_insert(initcode));
                let checkpoint = suspend_frame();
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
                        is_static = checkpoint.message.is_static,
                        depth = frame_depth_increment(checkpoint.call_depth),
                    };
                calldata_install(EMPTY_SLICE);
                pc = BYTE_ZERO;
                gas_remaining = child_gas;
                frame_status = Running();
                returndata_clear();
                frame_code = child_code;
                call_depth = frame_depth_increment(checkpoint.call_depth);
                frame_refund = GAS_REFUND_ZERO;

                returndata = interpret();

                /* the initcode's RETURN value is the deployed code; its length
                   sets the EIP-170 size check and the per-byte deposit cost. */
                let succeeded = frame_succeeded();
                let child_left = gas_remaining;
                let child_refund = frame_refund;
                let dep_len = returndata_size();

                /* deployed-code validity: within the EIP-170 size limit and,
                   from London onward, not beginning with 0xEF (EIP-3541, the
                   reserved EOF prefix). */
                let code_ok =    dep_len
                              <= max_code_size()
                              &  (  (k_fork < London)
                                 | (dep_len == BYTE_ZERO)
                                 | (slice_byte(returndata, BYTE_ZERO) != 0xef));
                let deposit = G_codedeposit * dep_len;

                /* leave the initcode frame: restore the parent's registers. */
                restore_frame(checkpoint);
                refund_gas(child_left);

                /* finalize: an initcode that succeeded AND whose code is valid
                   AND can pay its G_codedeposit cost deploys the code; the new
                   address is pushed. */
                if succeeded & code_ok & deposit <= child_left then {
                    gas_remaining = gas_sub_or_oog(gas_remaining, deposit);
                    if is_running() then {
                        record_refund(child_refund);
                        k_deploy_code(new_addr, returndata);
                        push(address_to_word(new_addr))
                    }
                } else {
                    /* otherwise the create fails and is rolled back, pushing 0.
                     * A deposit-stage failure (0xEF prefix, oversize code, or
                     * deposit OOG) is an exceptional halt that consumes ALL the
                     * child gas (process_create_message sets gas_left = 0), so
                     * the parent's gas is reset to its pre-reservation value; a
                     * reverting/failing initcode instead keeps child_left as
                     * already folded in above. */
                    if succeeded then {
                        gas_remaining = checkpoint.gas_remaining
                    };
                    k_revert(checkpoint.state);
                    push(WORD_ZERO)
                };

                /* CREATE exposes REVERT output, but clears successful initcode
                 * output after it has been consumed as deployed code. */
                if succeeded then {
                    returndata_clear()
                }
            }
        }
    }
}
