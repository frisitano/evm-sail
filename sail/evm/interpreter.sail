/* ======================================================================== */
/* THE INTERPRETER: FETCH/DECODE, RUN LOOP, AND THE MESSAGE-CALL MACHINERY   */
/*                                                                          */
/* The user-space EVM: it fetches and decodes bytecode (Yellow Paper section */
/* 9), drives the step loop, and spawns sub-frames for the call and create   */
/* opcodes. This module specifies that machinery in three layers.            */
/*                                                                          */
/* (1) Fetch/decode: read the opcode at pc, decode PUSH immediates inline,   */
/* and map every other byte to its AST node (an undefined byte decodes to    */
/* INVALID). Reading past the end of code yields STOP (YP: implicit halt).   */
/*                                                                          */
/* (2) Run loop: interpret() steps fetch-then-execute until the frame stops  */
/* running (normal halt, REVERT, or exceptional halt).                       */
/*                                                                          */
/* (3) Message calls: run_call handles CALL/CALLCODE/DELEGATECALL/STATICCALL */
/* (multiplexed on `mode`) and run_create handles CREATE/CREATE2. A sub-call */
/* saves the parent's user-space frame registers, runs the child to a halt,  */
/* then restores them. All WORLD effects go through EL-kernel syscalls:      */
/* k_snapshot on entry, k_transfer for value, and k_commit / k_revert on     */
/* exit -- the kernel rolls the world back atomically on a reverting child.  */
/* The applicable rules are EIP-150 (63/64ths gas cap + stipend), EIP-214    */
/* (static-context write protection), EIP-2929 (cold/warm access), and       */
/* EIP-7702 (delegated-code execution).                                      */
/* ======================================================================== */

/* EIP-150 / YP: a message call may nest at most 1024 frames deep. */
let DEPTH_LIMIT : int = 1024

/* ----------------------------- fetch/decode ----------------------------- */

/* Assemble the n-byte big-endian immediate following a PUSHn opcode into a
 * 256-bit word; bytes past the end of code read as zero (code_byte). */
function read_push(n : int) -> word = {
    var acc : word = WORD_ZERO;
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(code_byte(pc + k), 256))
    };
    acc
}

/* Decode one non-PUSH opcode byte to its AST node. The three contiguous
 * families fold to an arity argument -- DUP1..16 (0x80..0x8f), SWAP1..16
 * (0x90..0x9f), LOG0..4 (0xa0..0xa4) -- and the remainder is a flat table.
 * Any byte with no defined opcode decodes to INVALID. */
function decode_simple(b : int) -> ast =
    if b >= 128 & b <= 143 then {
        DUP(b - 127)
    } else {
        if b >= 144 & b <= 159 then {
            SWAP(b - 143)
        } else {
            if b >= 160 & b <= 164 then {
                LOG(b - 160)
            } else {
                match b {
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
                    30 => CLZ(),
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
                    72 => BASEFEE(),
                    73 => BLOBHASH(),
                    74 => BLOBBASEFEE(),
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
                    92 => TLOAD(),
                    93 => TSTORE(),
                    94 => MCOPY(),
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
    if pc >= code_len then {
        pc = pc + 1;
        STOP()
    } else {
        let b = unsigned(code_byte(pc));
        pc = pc + 1;
        if b >= 95 & b <= 127 then {
            let n = b - 95;
            let v = read_push(n);
            pc = pc + n;
            PUSH(n, v)
        } else {
            decode_simple(b)
        }
    }
}

/* The step loop: fetch then execute until the frame stops running -- a normal
 * halt (STOP/RETURN), a REVERT, or an exceptional halt all clear is_running. */
function interpret() -> unit =
    while is_running() do {
        let op = fetch();
        execute(op)
    }

/* Did the just-finished frame end successfully? A normal halt succeeds; a
 * REVERT and any exceptional halt do not (their world effects are rolled
 * back and CALL/CREATE reports failure). */
function frame_succeeded() -> bool =
    match frame_status {
        Halted(HaltRevert) => false,
        Halted(_) => true,
        Running() => true,
        Exceptional(_) => false,
    }

/* Select the code a frame actually executes (EIP-7702). A delegated account
 * runs the code at its delegation target, following exactly ONE hop; a
 * delegation whose target is a precompile (or has no code) executes as empty
 * code. An undelegated account runs its own code. */
function frame_code_exec(target : address, dele : bool, dtgt : address) -> unit =
    if dele then {
        let n = unsigned(address_to_word(dtgt));
        if is_precompile(n) then {
            frame_code_empty()
        } else {
            frame_code_addr(dtgt)
        }
    } else {
        frame_code_addr(target)
    }

/* ------------------------------ run_call -------------------------------- */
/* The four call opcodes, multiplexed on `mode`:
 *   0 = CALL          new frame at `target`, may transfer value
 *   1 = CALLCODE      run target's code in the CALLER's account, may transfer
 *   2 = DELEGATECALL  run target's code in the caller's account, inheriting
 *                     the parent's caller/value/static context (EIP-7)
 *   3 = STATICCALL    like CALL with value 0 and a forced static context
 *                     (EIP-214)
 * Operand layout (top of stack first): gas, target, [value for CALL/CALLCODE],
 * argsOffset, argsLen, retOffset, retLen. Pushes 1 on success, 0 on failure. */
function run_call(mode : int) -> unit = {
    let caller = self_addr();
    let gas_req = pop_int();
    let target = word_to_address(pop());
    let value = if mode == 0 | mode == 1 then {
            pop()
        } else {
            WORD_ZERO
        };
    let args_off = pop_int();
    let args_len = pop_int();
    let ret_off = pop_int();
    let ret_len = pop_int();

    /* A stack underflow during the operand pops exceptionally halts the frame
       (exc_halt zeroes the gas). The execution-specs unwind the whole op via
       an exception; Sail has no unwind, so bail out here before any gas is
       re-added below. */
    if not_bool(is_running()) then {
        return ()
    };

    /* memory is expanded once to cover the larger of the input-args range and
       the return-data region (a zero-length range touches no memory) */
    let m_args = if args_len > 0 then {
            args_off + args_len
        } else {
            0
        };
    let m_ret = if ret_len > 0 then {
            ret_off + ret_len
        } else {
            0
        };
    let m_max = if m_args > m_ret then { m_args } else { m_ret };

    /* memory-expansion cost is folded into the single message-call charge
       below: the execution-specs charge message_call_gas.cost together with
       extend_memory.cost, then grow memory only if the frame did not run out
       of gas. */
    let mem_gas = if m_max > 0 then {
            expansion_gas(m_max)
        } else {
            0
        };

    /* EIP-214: a value-bearing CALL (mode 0) inside a static context is a write and
       raises WriteInStaticContext -- an exceptional halt that consumes all the
       caller frame's gas. CALLCODE/DELEGATECALL/STATICCALL never trigger it
       (CALLCODE has no static guard in the spec; the others force value = 0). */
    if (mode == 0) & word_nonzero(value) & message.msg_is_static then {
        exc_halt(WriteProtection);
        return ()
    };

    /* EIP-2929 access (marks the target warm as a side effect) + EIP-7702
       delegation access form the access component of extra_gas. */
    let warm = k_access_account(target);
    var access_cost : int = account_cost(warm);
    let (tg_deleg, tg_target) = k_deleg_target(target);
    if tg_deleg then {
        let dw = k_access_account(tg_target);
        access_cost = access_cost + account_cost(dw)
    };

    /* value-transfer + new-account components of extra_gas */
    let transfer_cost : int = if word_nonzero(value) then { G_callvalue } else { 0 };
    let create_cost : int = if word_nonzero(value) & (mode == 0) & k_account_is_empty(target) then {
            G_newaccount
        } else {
            0
        };
    let extra_gas : int = access_cost + transfer_cost + create_cost;
    let stipend : int = if word_nonzero(value) then { G_callstipend } else { 0 };

    /* Forwarded child gas, mirroring the execution-specs
       calculate_message_call_gas. If the frame cannot even afford
       extra_gas + mem_gas, the call is charged the FULL requested gas (not the
       63/64-capped value), which forces the out-of-gas exceptional halt below.
       Otherwise the EIP-150 63/64ths cap applies to the gas left after
       subtracting mem_gas and extra_gas. */
    let avail = gas_remaining;
    let base_child : int = if avail < extra_gas + mem_gas then {
            gas_req
        } else {
            call_gas_cap(to_gas(avail - mem_gas - extra_gas), gas_req)
        };

    /* one combined charge: cost (= base_child + extra_gas) + mem_gas. A frame
       that cannot cover this exceptionally halts (all gas consumed) BEFORE the
       child runs and BEFORE the stipend is granted. */
    charge(to_gas(base_child + extra_gas + mem_gas));
    if not_bool(is_running()) then {
        return ()
    };
    if m_max > memory_size then {
        memory_size = m_max
    };
    let child_gas = base_child + stipend;

    /* call-failure guards (checked AFTER gas is charged): the depth ceiling
       (EIP-150, 1024 frames) and, for a value-bearing CALL/CALLCODE, the
       caller's balance covering the transfer. */
    let an = unsigned(target);
    let depth_ok : bool = call_depth + 1 <= DEPTH_LIMIT;
    let balance_ok : bool = if (mode == 0 | mode == 1) & word_nonzero(value) then {
            unsigned(k_get_balance(caller)) >= unsigned(value)
        } else {
            true
        };

    if not_bool(depth_ok) | not_bool(balance_ok) then {
        /* the call does not happen: the child gas (including stipend) is
           refunded to the caller and CALL pushes 0. */
        hr_clear();
        gas_remaining = to_gas(gas_remaining + child_gas);
        push(WORD_ZERO)
    } else {
        if is_precompile(an) then {
            /* PRECOMPILE call. The precompile set is fork-gated (the highest is
               0x100 P256VERIFY, EIP-7951/Osaka); an address outside the active
               set is an ordinary code call. The precompile's input is the
               child message's calldata, a snapshot of
               mem[args_off .. args_off+args_len) staged host-side in one copy;
               precompile_gas inspects the staged bytes O(1). */
            let in_len = if is_running() then { args_len } else { 0 };
            stage_precompile_input(an, args_off, in_len);
            let used = precompile_gas(an, in_len);

            /* gas check FIRST: an out-of-gas precompile call must not execute (e.g. a
               blake2f with a huge round count would otherwise run before failing). */
            let ok : bool = if used <= child_gas then {
                    k_precompile(an)
                } else {
                    false
                };
            if ok & (used <= child_gas) then {
                hr_capture_acc();
                hr_adopt();

                /* the value transfer is part of the (successful) call */
                if (mode == 0) & word_nonzero(value) then {
                    k_transfer(caller, target, value)
                };
                returndata_out_region(ret_off, ret_len);
                gas_remaining = to_gas(gas_remaining + (child_gas - used));
                push(WORD_ONE)
            } else {
                /* precompile failure (bad input) or OOG is a non-revert error: it consumes
                   ALL the call's gas (no refund of child_gas), CALL returns 0, and the
                   RETURNDATA buffer is EMPTY (no output escapes a failed precompile). */
                hr_clear();
                push(WORD_ZERO)
            }
        } else {
            /* CODE call. Snapshot the world (so a reverting child can be rolled
               back), then perform the CALL value transfer up front. */
            k_snapshot();
            if (mode == 0) & word_nonzero(value) then {
                k_transfer(caller, target, value)
            };

            /* save the parent's user-space frame registers to restore on
               return (Sail has no native call stack for these). */
            let s_pc = pc;
            let s_gas = gas_remaining;
            let s_msg = message;
            let s_status = frame_status;
            let s_msize = memory_size;
            let s_depth = call_depth;

            /* the child message context, by call mode:
               - address: target for CALL/STATICCALL; the CALLER's own address
                 for CALLCODE/DELEGATECALL (they run target's code in place);
               - caller/value: inherited from the parent for DELEGATECALL,
                 else the immediate caller and the call's value;
               - static: forced for STATICCALL, else inherited (a static frame
                 stays static for all of its sub-calls). */
            let child_addr : address = if mode == 0 | mode == 3 then { target } else { caller };
            let child_caller : address = if mode == 2 then {
                    s_msg.msg_caller
                } else {
                    caller
                };
            let child_value : word = if mode == 2 then {
                    s_msg.msg_value
                } else {
                    value
                };
            let child_static : bool = if mode == 3 then {
                    true
                } else {
                    s_msg.msg_is_static
                };

            message =
                struct {
                    msg_caller = child_caller,
                    msg_address = child_addr,
                    msg_code_address = target,
                    msg_data_len = if is_running() then args_len else 0,
                    msg_value = child_value,
                    msg_is_static = child_static,
                    msg_depth = s_depth + 1,
                };
            if is_running() then {
                calldata_set_next(args_off, args_len)
            } else {
                cd_set_empty()
            };

            /* enter the child frame: pc at 0, its own gas/stack/memory, code
               selected by the EIP-7702 delegation rule, depth bumped. */
            pc = 0;
            gas_remaining = to_gas(child_gas);
            stack_frame_enter();
            frame_status = Running();
            mem_frame_enter();
            hr_clear();
            memory_size = 0;
            frame_code_exec(target, tg_deleg, tg_target); /* EIP-7702 delegation */
            call_depth = call_depth + 1;

            interpret();

            let succeeded = frame_succeeded();
            let child_left = gas_remaining;

            /* leave the child: restore every saved parent register, then credit
               the parent with the child's unspent gas. */
            pc = s_pc;
            stack_frame_leave();
            message = s_msg;
            frame_status = s_status;
            mem_frame_leave();
            frame_code_resync();
            memory_size = s_msize;
            call_depth = call_depth - 1;
            hr_adopt();
            gas_remaining = to_gas(s_gas + child_left);

            /* copy returndata into the caller's output region on both success
               AND revert (execution-specs generic_call), writing only
               min(ret_len, |output|) bytes and leaving the rest untouched. An
               exceptionally-halted child has empty output, so nothing is
               written. The kernel then commits the child's world effects on
               success, or rolls them back on failure. */
            returndata_out_region(ret_off, ret_len);
            if succeeded then {
                k_commit();
                push(WORD_ONE)
            } else {
                k_revert();
                push(WORD_ZERO)
            }
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
    let off = pop_int();
    let len = pop_int();
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

    /* up-front charges: memory expansion over the initcode, the CREATE base
       cost (G_create), the EIP-3860 per-initcode-word cost, and -- for CREATE2
       only -- the keccak word cost of hashing the initcode for the address. */
    charge_expansion(off, len);
    charge(G_create);
    charge(initcode_gas(len));
    if is2 then {
        charge(to_gas(G_keccak_word * ediv_int(len + 31, 32)))
    };

    /* an out-of-gas during those charges is an exceptional halt (all gas
       consumed); bail before any nonce bump or child allocation. */
    if not_bool(is_running()) then {
        return ()
    };

    /* EIP-3860: initcode longer than MAX_INITCODE_SIZE is rejected outright as
       an exceptional halt. */
    if len > MAX_INITCODE_SIZE then {
        exc_halt(InitCodeTooLarge)
    } else {
        /* derive the new contract address: keccak(0xff, creator, salt,
           keccak(initcode)) for CREATE2 (EIP-1014), else rlp(creator, nonce). */
        let nonce = k_get_nonce(creator);
        let new_addr : address = if is2 & is_running() then {
                k_create2_addr(creator, salt, mem_keccak(off, len))
            } else {
                k_create_addr(creator, nonce)
            };

        /* reserve all-but-one-64th of the remaining gas for the initcode frame
           (EIP-150); the retained 1/64th stays with the creating frame. */
        let avail = gas_remaining;
        let child_gas = avail - ediv_int(avail, 64);
        gas_remaining = to_gas(avail - child_gas);

        /* EIP-214 (execution-specs generic_create): a CREATE in a static
         * context (e.g. inside a STATICCALL frame) is a write and raises
         * WriteInStaticContext after the child gas is reserved -- an
         * exceptional halt that consumes all gas, bumps no nonce, runs no
         * initcode. */
        if message.msg_is_static then {
            exc_halt(WriteProtection);
            return ()
        };

        /* early-abort guards (child gas refunded, 0 pushed, NO nonce bump): the
           depth ceiling, the creator's balance covering the endowment, and the
           creator nonce not already at the 2^64-1 ceiling (it must be
           incrementable). */
        let depth_ok : bool = call_depth + 1 <= DEPTH_LIMIT;
        let balance_ok : bool = unsigned(k_get_balance(creator)) >= unsigned(value);
        let nonce_ok : bool = nonce < 18446744073709551615;

        if not_bool(depth_ok) | not_bool(balance_ok) | not_bool(nonce_ok) then {
            hr_clear();
            gas_remaining = to_gas(gas_remaining + child_gas);
            push(WORD_ZERO)
        } else {
            /* warm the new address and bump the creator nonce BEFORE the child
             * snapshot: both persist even if the initcode reverts (the
             * execution-specs run them in the parent, before
             * process_create_message). */
            let _ = k_access_account(new_addr);
            let occupied : bool = k_account_occupied(new_addr);
            hr_clear();
            k_bump_nonce(creator);
            if occupied then {
                /* address collision (the target already has code, a nonzero
                 * nonce, or storage): no initcode runs, 0 is pushed, and the
                 * reserved child gas is NOT refunded (it stays deducted). */
                push(WORD_ZERO)
            } else {
                /* enter the new account: snapshot, mark it created this tx,
                   set its initial nonce to 1, and endow it with `value`. */
                k_snapshot();
                k_mark_created(new_addr); /* EIP-6780: created this tx */
                k_bump_nonce(new_addr); /* EIP-161: a created contract starts at nonce 1 */
                k_transfer(creator, new_addr, value);

                /* save the parent's frame registers (restored on return). */
                let s_pc = pc;
                let s_gas = gas_remaining;
                let s_msg = message;
                let s_status = frame_status;
                let s_msize = memory_size;
                let s_depth = call_depth;

                message =
                    struct {
                        msg_caller = creator,
                        msg_address = new_addr,
                        msg_code_address = new_addr,
                        msg_value = value,
                        msg_data_len = 0,
                        msg_is_static = s_msg.msg_is_static,
                        msg_depth = s_depth + 1,
                    };

                /* the initcode frame has no calldata; its executing code is the
                   initcode copied from memory[off, off+len). */
                cd_set_empty();
                let ic_len = if is_running() then {
                        frame_code_pend_mem(off, len)
                    } else {
                        0
                    };
                pc = 0;
                gas_remaining = to_gas(child_gas);
                stack_frame_enter();
                frame_status = Running();
                mem_frame_enter();
                hr_clear();
                memory_size = 0;
                code_len = ic_len;
                if is_running() & (len == 0) then {
                    frame_code_empty()
                };
                call_depth = call_depth + 1;

                interpret();

                /* the initcode's RETURN value is the deployed code; its length
                   sets the EIP-170 size check and the per-byte deposit cost. */
                let succeeded = frame_succeeded();
                let child_left = gas_remaining;
                let dep_len = unsigned(hr_pending_len());
                let deposit = G_codedeposit * dep_len;

                /* deployed-code validity: within the EIP-170 size limit and not
                   beginning with 0xEF (EIP-3541, the reserved EOF prefix). */
                let code_ok = (dep_len <= MAX_CODE_SIZE) &
                        ((dep_len == 0) | (hr_pending_byte(get_slice_int(64, 0, 0)) != 0xef));

                /* leave the initcode frame: restore the parent's registers. */
                pc = s_pc;
                stack_frame_leave();
                message = s_msg;
                frame_status = s_status;
                mem_frame_leave();
                frame_code_resync();
                memory_size = s_msize;
                call_depth = call_depth - 1;

                /* a successful create yields EMPTY returndata; a revert exposes its data */
                if succeeded then {
                    hr_clear()
                } else {
                    hr_adopt()
                };
                gas_remaining = to_gas(s_gas + child_left);

                /* finalize: an initcode that succeeded AND whose code is valid
                   AND can pay its G_codedeposit cost deploys the code and
                   commits; the new address is pushed. */
                if succeeded & code_ok & (child_left >= deposit) then {
                    gas_remaining = to_gas(gas_remaining - deposit);
                    k_set_code(new_addr, pending_output_list());
                    hr_discard();
                    k_commit();
                    push(address_to_word(new_addr))
                } else {
                    /* otherwise the create fails and is rolled back, pushing 0.
                     * A deposit-stage failure (0xEF prefix, oversize code, or
                     * deposit OOG) is an exceptional halt that consumes ALL the
                     * child gas (process_create_message sets gas_left = 0), so
                     * the parent's gas is reset to its pre-reservation value; a
                     * reverting/failing initcode instead keeps child_left as
                     * already folded in above. */
                    if succeeded then {
                        gas_remaining = s_gas
                    };
                    hr_discard();
                    k_revert();
                    push(WORD_ZERO)
                }
            }
        }
    }
}
