/* Alternate optimized splice for `sail/evm/interpreter.sail`, selected by
 * `EVM_GENERATED_INTERP=on` in place of `evm/interpreter.sail` (which
 * replaces `interpret` with the hand-written C loop). This variant keeps
 * the GENERATED Sail interpreter loop and attaches `$[c_inline]` so that,
 * with the optimized compiler's --c-inline-attr (EVM_INLINE_ATTR=on), the
 * fetch/decode dispatch layers are inlined into `interpret` alongside the
 * `execute` layers annotated in `evm/execute.sail`. It is deliberately NOT
 * listed in `sail/optimised/manifest`.
 *
 * The `interpret` body below textually repeats the canonical run loop from
 * `sail/evm/interpreter.sail` MINUS the proof-only `call_tree_steps_remaining`
 * termination measure: that counter is an unbounded mathematical int that
 * exists only to supply a decreasing measure to total extraction targets,
 * and --c-require-bounded-int (implied by --c-optimized-model) rejects it.
 * Keep this loop in sync with the canonical definition.
 *
 * Every arm follows the state-passing convention: it computes the advanced
 * program counter, validates the instruction stack effect against the
 * carried cursor and gas, and threads the carried machine state through the
 * narrow handler exactly as the canonical fetch/execute pipeline does. */

$[c_inline]
val fetch : code_pointer -> (code_pointer, ast)

$[c_inline]
val decode_simple : opcode -> ast

/* Fused raw-byte step: the semantic composition of the canonical
 * fetch/decode/execute pipeline with the intermediate `ast` value compiled
 * out. Each arm performs, in canonical order, exactly what
 * fetch (pc advance past opcode and immediates, fork gates, past-end STOP)
 * followed by execute (opcode_stack_effect constants, validate_stack, the
 * narrow execute_* handler) performs for that byte. The three Amsterdam
 * deep-stack opcodes keep their constructor and route through the canonical
 * execute so their immediate-validity stack-effect rules stay single-source.
 * The canonical pipeline remains the definition for the spec build and
 * every extraction target. */
$[c_inline]
val step_fused : (code_pointer, StackTop, EvmMemorySlice, gas) -> (code_pointer, StackTop, EvmMemorySlice, gas)

function step_fused(current, top, mem, g) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let analyzed = frame_code;
    let code = code_bytes(analyzed);
    let code_length = code.len;
    let past_end = not_bool(current < code_length);
    if past_end then {
        let (stop_valid, g1) = validate_stack(g, top, 0, 0);
        if stop_valid then {
            execute_stop()
        };
        (current, top, mem, g1)
    } else {
        let opcode_byte = slice_byte(code, current);
        let opcode : opcode = unsigned(opcode_byte);
        let immediate_offset : code_pointer = current + 1;
        match opcode {
            0 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 0);
                if stack_valid then {
                    execute_stop()
                };
                (immediate_offset, top, mem, g1)
            },
            1 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_add(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            2 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_mul(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            3 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_sub(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            4 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_div(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            5 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_sdiv(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            6 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_mod(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            7 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_smod(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            8 => {
                let (stack_valid, g1) = validate_stack(g, top, 3, 1);
                if stack_valid then {
                    let (top1, g2) = execute_addmod(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            9 => {
                let (stack_valid, g1) = validate_stack(g, top, 3, 1);
                if stack_valid then {
                    let (top1, g2) = execute_mulmod(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            10 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_exp(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            11 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_signextend(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            16 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_lt(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            17 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_gt(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            18 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_slt(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            19 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_sgt(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            20 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_eq(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            21 => {
                let (stack_valid, g1) = validate_stack(g, top, 1, 1);
                if stack_valid then {
                    let (top1, g2) = execute_iszero(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            22 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_and(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            23 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_or(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            24 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_xor(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            25 => {
                let (stack_valid, g1) = validate_stack(g, top, 1, 1);
                if stack_valid then {
                    let (top1, g2) = execute_not(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            26 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_byte(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            27 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_shl(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            28 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_shr(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            29 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, g2) = execute_sar(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            30 => {
                if profile.fork >= Osaka then {
                    let (stack_valid, g1) = validate_stack(g, top, 1, 1);
                    if stack_valid then {
                        let (top1, g2) = execute_clz(top, g1);
                        (immediate_offset, top1, mem, g2)
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                } else {
                    let (stack_valid, g1) = validate_stack(g, top, 0, 0);
                    if stack_valid then {
                        (immediate_offset, top, mem, execute_invalid(g1))
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                }
            },
            32 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 1);
                if stack_valid then {
                    let (top1, mem1, g2) = execute_keccak256(top, mem, g1);
                    (immediate_offset, top1, mem1, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            48 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_address(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            49 => {
                let (stack_valid, g1) = validate_stack(g, top, 1, 1);
                if stack_valid then {
                    let (top1, g2) = execute_balance(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            50 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_origin(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            51 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_caller(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            52 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_callvalue(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            53 => {
                let (stack_valid, g1) = validate_stack(g, top, 1, 1);
                if stack_valid then {
                    let (top1, g2) = execute_calldataload(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            54 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_calldatasize(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            55 => {
                let (stack_valid, g1) = validate_stack(g, top, 3, 0);
                if stack_valid then {
                    let (top1, mem1, g2) = execute_calldatacopy(top, mem, g1);
                    (immediate_offset, top1, mem1, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            56 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_codesize(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            57 => {
                let (stack_valid, g1) = validate_stack(g, top, 3, 0);
                if stack_valid then {
                    let (top1, mem1, g2) = execute_codecopy(top, mem, g1);
                    (immediate_offset, top1, mem1, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            58 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_gasprice(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            59 => {
                let (stack_valid, g1) = validate_stack(g, top, 1, 1);
                if stack_valid then {
                    let (top1, g2) = execute_extcodesize(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            60 => {
                let (stack_valid, g1) = validate_stack(g, top, 4, 0);
                if stack_valid then {
                    let (top1, mem1, g2) = execute_extcodecopy(top, mem, g1);
                    (immediate_offset, top1, mem1, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            61 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_returndatasize(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            62 => {
                let (stack_valid, g1) = validate_stack(g, top, 3, 0);
                if stack_valid then {
                    let (top1, mem1, g2) = execute_returndatacopy(top, mem, g1);
                    (immediate_offset, top1, mem1, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            63 => {
                let (stack_valid, g1) = validate_stack(g, top, 1, 1);
                if stack_valid then {
                    let (top1, g2) = execute_extcodehash(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            64 => {
                let (stack_valid, g1) = validate_stack(g, top, 1, 1);
                if stack_valid then {
                    let (top1, g2) = execute_blockhash(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            65 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_coinbase(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            66 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_timestamp(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            67 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_number(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            68 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_prevrandao(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            69 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_gaslimit(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            70 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_chainid(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            71 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_selfbalance(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            72 => {
                if profile.fork >= London then {
                    let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                    if stack_valid then {
                        let (top1, g2) = execute_basefee(top, g1);
                        (immediate_offset, top1, mem, g2)
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                } else {
                    let (stack_valid, g1) = validate_stack(g, top, 0, 0);
                    if stack_valid then {
                        (immediate_offset, top, mem, execute_invalid(g1))
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                }
            },
            73 => {
                if profile.fork >= Cancun then {
                    let (stack_valid, g1) = validate_stack(g, top, 1, 1);
                    if stack_valid then {
                        let (top1, g2) = execute_blobhash(top, g1);
                        (immediate_offset, top1, mem, g2)
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                } else {
                    let (stack_valid, g1) = validate_stack(g, top, 0, 0);
                    if stack_valid then {
                        (immediate_offset, top, mem, execute_invalid(g1))
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                }
            },
            74 => {
                if profile.fork >= Cancun then {
                    let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                    if stack_valid then {
                        let (top1, g2) = execute_blobbasefee(top, g1);
                        (immediate_offset, top1, mem, g2)
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                } else {
                    let (stack_valid, g1) = validate_stack(g, top, 0, 0);
                    if stack_valid then {
                        (immediate_offset, top, mem, execute_invalid(g1))
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                }
            },
            75 => {
                if profile.fork >= Amsterdam then {
                    let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                    if stack_valid then {
                        let (top1, g2) = execute_slotnum(top, g1);
                        (immediate_offset, top1, mem, g2)
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                } else {
                    let (stack_valid, g1) = validate_stack(g, top, 0, 0);
                    if stack_valid then {
                        (immediate_offset, top, mem, execute_invalid(g1))
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                }
            },
            80 => {
                let (stack_valid, g1) = validate_stack(g, top, 1, 0);
                if stack_valid then {
                    let (top1, g2) = execute_pop(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            81 => {
                let (stack_valid, g1) = validate_stack(g, top, 1, 1);
                if stack_valid then {
                    let (top1, mem1, g2) = execute_mload(top, mem, g1);
                    (immediate_offset, top1, mem1, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            82 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 0);
                if stack_valid then {
                    let (top1, mem1, g2) = execute_mstore(top, mem, g1);
                    (immediate_offset, top1, mem1, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            83 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 0);
                if stack_valid then {
                    let (top1, mem1, g2) = execute_mstore8(top, mem, g1);
                    (immediate_offset, top1, mem1, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            84 => {
                let (stack_valid, g1) = validate_stack(g, top, 1, 1);
                if stack_valid then {
                    let (top1, g2) = execute_sload(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            85 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 0);
                if stack_valid then {
                    let (top1, g2) = execute_sstore(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            86 => {
                let (stack_valid, g1) = validate_stack(g, top, 1, 0);
                if stack_valid then {
                    let (pc1, top1, g2) = execute_jump(immediate_offset, top, g1);
                    (pc1, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            87 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 0);
                if stack_valid then {
                    let (pc1, top1, g2) = execute_jumpi(immediate_offset, top, g1);
                    (pc1, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            88 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_pc(immediate_offset, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            89 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, mem1, g2) = execute_msize(top, mem, g1);
                    (immediate_offset, top1, mem1, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            90 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_gas(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            91 => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 0);
                if stack_valid then {
                    (immediate_offset, top, mem, execute_jumpdest(g1))
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            92 => {
                if profile.fork >= Cancun then {
                    let (stack_valid, g1) = validate_stack(g, top, 1, 1);
                    if stack_valid then {
                        let (top1, g2) = execute_tload(top, g1);
                        (immediate_offset, top1, mem, g2)
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                } else {
                    let (stack_valid, g1) = validate_stack(g, top, 0, 0);
                    if stack_valid then {
                        (immediate_offset, top, mem, execute_invalid(g1))
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                }
            },
            93 => {
                if profile.fork >= Cancun then {
                    let (stack_valid, g1) = validate_stack(g, top, 2, 0);
                    if stack_valid then {
                        let (top1, g2) = execute_tstore(top, g1);
                        (immediate_offset, top1, mem, g2)
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                } else {
                    let (stack_valid, g1) = validate_stack(g, top, 0, 0);
                    if stack_valid then {
                        (immediate_offset, top, mem, execute_invalid(g1))
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                }
            },
            94 => {
                if profile.fork >= Cancun then {
                    let (stack_valid, g1) = validate_stack(g, top, 3, 0);
                    if stack_valid then {
                        let (top1, mem1, g2) = execute_mcopy(top, mem, g1);
                        (immediate_offset, top1, mem1, g2)
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                } else {
                    let (stack_valid, g1) = validate_stack(g, top, 0, 0);
                    if stack_valid then {
                        (immediate_offset, top, mem, execute_invalid(g1))
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                }
            },
            95 => {
                if profile.fork < Shanghai then {
                    let (stack_valid, g1) = validate_stack(g, top, 0, 0);
                    if stack_valid then {
                        (immediate_offset, top, mem, execute_invalid(g1))
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                } else {
                    let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                    if stack_valid then {
                        let (top1, g2) = execute_push(0, read_push(code, immediate_offset, 0), top, g1);
                        (immediate_offset, top1, mem, g2)
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                }
            },
            96 => {
                let value = read_push(code, immediate_offset, 1);
                let after_immediate : code_pointer = current + 2;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(1, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            97 => {
                let value = read_push(code, immediate_offset, 2);
                let after_immediate : code_pointer = current + 3;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(2, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            98 => {
                let value = read_push(code, immediate_offset, 3);
                let after_immediate : code_pointer = current + 4;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(3, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            99 => {
                let value = read_push(code, immediate_offset, 4);
                let after_immediate : code_pointer = current + 5;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(4, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            100 => {
                let value = read_push(code, immediate_offset, 5);
                let after_immediate : code_pointer = current + 6;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(5, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            101 => {
                let value = read_push(code, immediate_offset, 6);
                let after_immediate : code_pointer = current + 7;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(6, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            102 => {
                let value = read_push(code, immediate_offset, 7);
                let after_immediate : code_pointer = current + 8;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(7, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            103 => {
                let value = read_push(code, immediate_offset, 8);
                let after_immediate : code_pointer = current + 9;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(8, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            104 => {
                let value = read_push(code, immediate_offset, 9);
                let after_immediate : code_pointer = current + 10;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(9, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            105 => {
                let value = read_push(code, immediate_offset, 10);
                let after_immediate : code_pointer = current + 11;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(10, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            106 => {
                let value = read_push(code, immediate_offset, 11);
                let after_immediate : code_pointer = current + 12;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(11, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            107 => {
                let value = read_push(code, immediate_offset, 12);
                let after_immediate : code_pointer = current + 13;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(12, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            108 => {
                let value = read_push(code, immediate_offset, 13);
                let after_immediate : code_pointer = current + 14;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(13, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            109 => {
                let value = read_push(code, immediate_offset, 14);
                let after_immediate : code_pointer = current + 15;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(14, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            110 => {
                let value = read_push(code, immediate_offset, 15);
                let after_immediate : code_pointer = current + 16;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(15, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            111 => {
                let value = read_push(code, immediate_offset, 16);
                let after_immediate : code_pointer = current + 17;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(16, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            112 => {
                let value = read_push(code, immediate_offset, 17);
                let after_immediate : code_pointer = current + 18;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(17, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            113 => {
                let value = read_push(code, immediate_offset, 18);
                let after_immediate : code_pointer = current + 19;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(18, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            114 => {
                let value = read_push(code, immediate_offset, 19);
                let after_immediate : code_pointer = current + 20;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(19, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            115 => {
                let value = read_push(code, immediate_offset, 20);
                let after_immediate : code_pointer = current + 21;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(20, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            116 => {
                let value = read_push(code, immediate_offset, 21);
                let after_immediate : code_pointer = current + 22;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(21, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            117 => {
                let value = read_push(code, immediate_offset, 22);
                let after_immediate : code_pointer = current + 23;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(22, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            118 => {
                let value = read_push(code, immediate_offset, 23);
                let after_immediate : code_pointer = current + 24;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(23, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            119 => {
                let value = read_push(code, immediate_offset, 24);
                let after_immediate : code_pointer = current + 25;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(24, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            120 => {
                let value = read_push(code, immediate_offset, 25);
                let after_immediate : code_pointer = current + 26;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(25, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            121 => {
                let value = read_push(code, immediate_offset, 26);
                let after_immediate : code_pointer = current + 27;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(26, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            122 => {
                let value = read_push(code, immediate_offset, 27);
                let after_immediate : code_pointer = current + 28;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(27, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            123 => {
                let value = read_push(code, immediate_offset, 28);
                let after_immediate : code_pointer = current + 29;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(28, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            124 => {
                let value = read_push(code, immediate_offset, 29);
                let after_immediate : code_pointer = current + 30;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(29, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            125 => {
                let value = read_push(code, immediate_offset, 30);
                let after_immediate : code_pointer = current + 31;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(30, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            126 => {
                let value = read_push(code, immediate_offset, 31);
                let after_immediate : code_pointer = current + 32;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(31, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            127 => {
                let value = read_push(code, immediate_offset, 32);
                let after_immediate : code_pointer = current + 33;
                let (stack_valid, g1) = validate_stack(g, top, 0, 1);
                if stack_valid then {
                    let (top1, g2) = execute_push(32, value, top, g1);
                    (after_immediate, top1, mem, g2)
                } else {
                    (after_immediate, top, mem, g1)
                }
            },
            128 => {
                let (stack_valid, g1) = validate_stack(g, top, 1, 2);
                if stack_valid then {
                    let (top1, g2) = execute_dup(1, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            129 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 3);
                if stack_valid then {
                    let (top1, g2) = execute_dup(2, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            130 => {
                let (stack_valid, g1) = validate_stack(g, top, 3, 4);
                if stack_valid then {
                    let (top1, g2) = execute_dup(3, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            131 => {
                let (stack_valid, g1) = validate_stack(g, top, 4, 5);
                if stack_valid then {
                    let (top1, g2) = execute_dup(4, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            132 => {
                let (stack_valid, g1) = validate_stack(g, top, 5, 6);
                if stack_valid then {
                    let (top1, g2) = execute_dup(5, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            133 => {
                let (stack_valid, g1) = validate_stack(g, top, 6, 7);
                if stack_valid then {
                    let (top1, g2) = execute_dup(6, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            134 => {
                let (stack_valid, g1) = validate_stack(g, top, 7, 8);
                if stack_valid then {
                    let (top1, g2) = execute_dup(7, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            135 => {
                let (stack_valid, g1) = validate_stack(g, top, 8, 9);
                if stack_valid then {
                    let (top1, g2) = execute_dup(8, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            136 => {
                let (stack_valid, g1) = validate_stack(g, top, 9, 10);
                if stack_valid then {
                    let (top1, g2) = execute_dup(9, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            137 => {
                let (stack_valid, g1) = validate_stack(g, top, 10, 11);
                if stack_valid then {
                    let (top1, g2) = execute_dup(10, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            138 => {
                let (stack_valid, g1) = validate_stack(g, top, 11, 12);
                if stack_valid then {
                    let (top1, g2) = execute_dup(11, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            139 => {
                let (stack_valid, g1) = validate_stack(g, top, 12, 13);
                if stack_valid then {
                    let (top1, g2) = execute_dup(12, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            140 => {
                let (stack_valid, g1) = validate_stack(g, top, 13, 14);
                if stack_valid then {
                    let (top1, g2) = execute_dup(13, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            141 => {
                let (stack_valid, g1) = validate_stack(g, top, 14, 15);
                if stack_valid then {
                    let (top1, g2) = execute_dup(14, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            142 => {
                let (stack_valid, g1) = validate_stack(g, top, 15, 16);
                if stack_valid then {
                    let (top1, g2) = execute_dup(15, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            143 => {
                let (stack_valid, g1) = validate_stack(g, top, 16, 17);
                if stack_valid then {
                    let (top1, g2) = execute_dup(16, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            144 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 2);
                if stack_valid then {
                    let (top1, g2) = execute_swap(1, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            145 => {
                let (stack_valid, g1) = validate_stack(g, top, 3, 3);
                if stack_valid then {
                    let (top1, g2) = execute_swap(2, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            146 => {
                let (stack_valid, g1) = validate_stack(g, top, 4, 4);
                if stack_valid then {
                    let (top1, g2) = execute_swap(3, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            147 => {
                let (stack_valid, g1) = validate_stack(g, top, 5, 5);
                if stack_valid then {
                    let (top1, g2) = execute_swap(4, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            148 => {
                let (stack_valid, g1) = validate_stack(g, top, 6, 6);
                if stack_valid then {
                    let (top1, g2) = execute_swap(5, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            149 => {
                let (stack_valid, g1) = validate_stack(g, top, 7, 7);
                if stack_valid then {
                    let (top1, g2) = execute_swap(6, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            150 => {
                let (stack_valid, g1) = validate_stack(g, top, 8, 8);
                if stack_valid then {
                    let (top1, g2) = execute_swap(7, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            151 => {
                let (stack_valid, g1) = validate_stack(g, top, 9, 9);
                if stack_valid then {
                    let (top1, g2) = execute_swap(8, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            152 => {
                let (stack_valid, g1) = validate_stack(g, top, 10, 10);
                if stack_valid then {
                    let (top1, g2) = execute_swap(9, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            153 => {
                let (stack_valid, g1) = validate_stack(g, top, 11, 11);
                if stack_valid then {
                    let (top1, g2) = execute_swap(10, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            154 => {
                let (stack_valid, g1) = validate_stack(g, top, 12, 12);
                if stack_valid then {
                    let (top1, g2) = execute_swap(11, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            155 => {
                let (stack_valid, g1) = validate_stack(g, top, 13, 13);
                if stack_valid then {
                    let (top1, g2) = execute_swap(12, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            156 => {
                let (stack_valid, g1) = validate_stack(g, top, 14, 14);
                if stack_valid then {
                    let (top1, g2) = execute_swap(13, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            157 => {
                let (stack_valid, g1) = validate_stack(g, top, 15, 15);
                if stack_valid then {
                    let (top1, g2) = execute_swap(14, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            158 => {
                let (stack_valid, g1) = validate_stack(g, top, 16, 16);
                if stack_valid then {
                    let (top1, g2) = execute_swap(15, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            159 => {
                let (stack_valid, g1) = validate_stack(g, top, 17, 17);
                if stack_valid then {
                    let (top1, g2) = execute_swap(16, top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            160 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 0);
                if stack_valid then {
                    let (top1, mem1, g2) = execute_log(0, top, mem, g1);
                    (immediate_offset, top1, mem1, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            161 => {
                let (stack_valid, g1) = validate_stack(g, top, 3, 0);
                if stack_valid then {
                    let (top1, mem1, g2) = execute_log(1, top, mem, g1);
                    (immediate_offset, top1, mem1, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            162 => {
                let (stack_valid, g1) = validate_stack(g, top, 4, 0);
                if stack_valid then {
                    let (top1, mem1, g2) = execute_log(2, top, mem, g1);
                    (immediate_offset, top1, mem1, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            163 => {
                let (stack_valid, g1) = validate_stack(g, top, 5, 0);
                if stack_valid then {
                    let (top1, mem1, g2) = execute_log(3, top, mem, g1);
                    (immediate_offset, top1, mem1, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            164 => {
                let (stack_valid, g1) = validate_stack(g, top, 6, 0);
                if stack_valid then {
                    let (top1, mem1, g2) = execute_log(4, top, mem, g1);
                    (immediate_offset, top1, mem1, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            230 => {
                if profile.fork >= Amsterdam then {
                    let immediate = slice_byte(code, immediate_offset);
                    let immediate_valid = deep_stack_operation_immediate_valid(DeepStackDuplicate, immediate);
                    let after_instruction : code_pointer =
                        if immediate_valid then current + 2 else immediate_offset;
                    execute(DUPN(immediate), after_instruction, top, mem, g)
                } else {
                    let (stack_valid, g1) = validate_stack(g, top, 0, 0);
                    if stack_valid then {
                        (immediate_offset, top, mem, execute_invalid(g1))
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                }
            },
            231 => {
                if profile.fork >= Amsterdam then {
                    let immediate = slice_byte(code, immediate_offset);
                    let immediate_valid = deep_stack_operation_immediate_valid(DeepStackSwap, immediate);
                    let after_instruction : code_pointer =
                        if immediate_valid then current + 2 else immediate_offset;
                    execute(SWAPN(immediate), after_instruction, top, mem, g)
                } else {
                    let (stack_valid, g1) = validate_stack(g, top, 0, 0);
                    if stack_valid then {
                        (immediate_offset, top, mem, execute_invalid(g1))
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                }
            },
            232 => {
                if profile.fork >= Amsterdam then {
                    let immediate = slice_byte(code, immediate_offset);
                    let immediate_valid = deep_stack_operation_immediate_valid(DeepStackExchange, immediate);
                    let after_instruction : code_pointer =
                        if immediate_valid then current + 2 else immediate_offset;
                    execute(EXCHANGE(immediate), after_instruction, top, mem, g)
                } else {
                    let (stack_valid, g1) = validate_stack(g, top, 0, 0);
                    if stack_valid then {
                        (immediate_offset, top, mem, execute_invalid(g1))
                    } else {
                        (immediate_offset, top, mem, g1)
                    }
                }
            },
            240 => {
                let (stack_valid, g1) = validate_stack(g, top, 3, 1);
                if stack_valid then {
                    execute_create(immediate_offset, top, mem, g1)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            241 => {
                let (stack_valid, g1) = validate_stack(g, top, 7, 1);
                if stack_valid then {
                    execute_call(immediate_offset, top, mem, g1)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            242 => {
                let (stack_valid, g1) = validate_stack(g, top, 7, 1);
                if stack_valid then {
                    execute_callcode(immediate_offset, top, mem, g1)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            243 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 0);
                if stack_valid then {
                    let (top1, mem1, g2) = execute_return(top, mem, g1);
                    (immediate_offset, top1, mem1, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            244 => {
                let (stack_valid, g1) = validate_stack(g, top, 6, 1);
                if stack_valid then {
                    execute_delegatecall(immediate_offset, top, mem, g1)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            245 => {
                let (stack_valid, g1) = validate_stack(g, top, 4, 1);
                if stack_valid then {
                    execute_create2(immediate_offset, top, mem, g1)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            250 => {
                let (stack_valid, g1) = validate_stack(g, top, 6, 1);
                if stack_valid then {
                    execute_staticcall(immediate_offset, top, mem, g1)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            253 => {
                let (stack_valid, g1) = validate_stack(g, top, 2, 0);
                if stack_valid then {
                    let (top1, mem1, g2) = execute_revert(top, mem, g1);
                    (immediate_offset, top1, mem1, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            255 => {
                let (stack_valid, g1) = validate_stack(g, top, 1, 0);
                if stack_valid then {
                    let (top1, g2) = execute_selfdestruct(top, g1);
                    (immediate_offset, top1, mem, g2)
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
            _ => {
                let (stack_valid, g1) = validate_stack(g, top, 0, 0);
                if stack_valid then {
                    (immediate_offset, top, mem, execute_invalid(g1))
                } else {
                    (immediate_offset, top, mem, g1)
                }
            },
        }
    }
}

function interpret() -> OutputSlice = {
    frame_stack_reset();
    var interpreting : bool = true;
    var result : OutputSlice = EMPTY_OUTPUT_SLICE;
    while interpreting do {
        let running = is_running();
        if running then {
            let (next_pc, next_top, next_mem, next_gas) = step_fused(pc, stack_top, evm_memory, gas_remaining);
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
        }
    };
    result
}
