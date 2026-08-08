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
 * Keep this loop in sync with the canonical definition. */

$[c_inline]
val fetch : unit -> ast

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
val step_fused : unit -> unit

function step_fused() -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let current = pc;
    let analyzed = frame_code;
    let code = code_bytes(analyzed);
    let code_length = code.len;
    let past_end = not_bool(current < code_length);
    if past_end then {
        if validate_stack(0, 0) then execute_stop()
    } else {
        let opcode_byte = slice_byte(code, current);
        let opcode : opcode = unsigned(opcode_byte);
        let immediate_offset : code_pointer = current + 1;
        match opcode {
            0 => { pc = immediate_offset; if validate_stack(0, 0) then execute_stop() },
            1 => { pc = immediate_offset; if validate_stack(2, 1) then execute_add() },
            2 => { pc = immediate_offset; if validate_stack(2, 1) then execute_mul() },
            3 => { pc = immediate_offset; if validate_stack(2, 1) then execute_sub() },
            4 => { pc = immediate_offset; if validate_stack(2, 1) then execute_div() },
            5 => { pc = immediate_offset; if validate_stack(2, 1) then execute_sdiv() },
            6 => { pc = immediate_offset; if validate_stack(2, 1) then execute_mod() },
            7 => { pc = immediate_offset; if validate_stack(2, 1) then execute_smod() },
            8 => { pc = immediate_offset; if validate_stack(3, 1) then execute_addmod() },
            9 => { pc = immediate_offset; if validate_stack(3, 1) then execute_mulmod() },
            10 => { pc = immediate_offset; if validate_stack(2, 1) then execute_exp() },
            11 => { pc = immediate_offset; if validate_stack(2, 1) then execute_signextend() },
            16 => { pc = immediate_offset; if validate_stack(2, 1) then execute_lt() },
            17 => { pc = immediate_offset; if validate_stack(2, 1) then execute_gt() },
            18 => { pc = immediate_offset; if validate_stack(2, 1) then execute_slt() },
            19 => { pc = immediate_offset; if validate_stack(2, 1) then execute_sgt() },
            20 => { pc = immediate_offset; if validate_stack(2, 1) then execute_eq() },
            21 => { pc = immediate_offset; if validate_stack(1, 1) then execute_iszero() },
            22 => { pc = immediate_offset; if validate_stack(2, 1) then execute_and() },
            23 => { pc = immediate_offset; if validate_stack(2, 1) then execute_or() },
            24 => { pc = immediate_offset; if validate_stack(2, 1) then execute_xor() },
            25 => { pc = immediate_offset; if validate_stack(1, 1) then execute_not() },
            26 => { pc = immediate_offset; if validate_stack(2, 1) then execute_byte() },
            27 => { pc = immediate_offset; if validate_stack(2, 1) then execute_shl() },
            28 => { pc = immediate_offset; if validate_stack(2, 1) then execute_shr() },
            29 => { pc = immediate_offset; if validate_stack(2, 1) then execute_sar() },
            30 => {
                pc = immediate_offset;
                if profile.fork >= Osaka then {
                    if validate_stack(1, 1) then execute_clz()
                } else {
                    if validate_stack(0, 0) then execute_invalid()
                }
            },
            32 => { pc = immediate_offset; if validate_stack(2, 1) then execute_keccak256() },
            48 => { pc = immediate_offset; if validate_stack(0, 1) then execute_address() },
            49 => { pc = immediate_offset; if validate_stack(1, 1) then execute_balance() },
            50 => { pc = immediate_offset; if validate_stack(0, 1) then execute_origin() },
            51 => { pc = immediate_offset; if validate_stack(0, 1) then execute_caller() },
            52 => { pc = immediate_offset; if validate_stack(0, 1) then execute_callvalue() },
            53 => { pc = immediate_offset; if validate_stack(1, 1) then execute_calldataload() },
            54 => { pc = immediate_offset; if validate_stack(0, 1) then execute_calldatasize() },
            55 => { pc = immediate_offset; if validate_stack(3, 0) then execute_calldatacopy() },
            56 => { pc = immediate_offset; if validate_stack(0, 1) then execute_codesize() },
            57 => { pc = immediate_offset; if validate_stack(3, 0) then execute_codecopy() },
            58 => { pc = immediate_offset; if validate_stack(0, 1) then execute_gasprice() },
            59 => { pc = immediate_offset; if validate_stack(1, 1) then execute_extcodesize() },
            60 => { pc = immediate_offset; if validate_stack(4, 0) then execute_extcodecopy() },
            61 => { pc = immediate_offset; if validate_stack(0, 1) then execute_returndatasize() },
            62 => { pc = immediate_offset; if validate_stack(3, 0) then execute_returndatacopy() },
            63 => { pc = immediate_offset; if validate_stack(1, 1) then execute_extcodehash() },
            64 => { pc = immediate_offset; if validate_stack(1, 1) then execute_blockhash() },
            65 => { pc = immediate_offset; if validate_stack(0, 1) then execute_coinbase() },
            66 => { pc = immediate_offset; if validate_stack(0, 1) then execute_timestamp() },
            67 => { pc = immediate_offset; if validate_stack(0, 1) then execute_number() },
            68 => { pc = immediate_offset; if validate_stack(0, 1) then execute_prevrandao() },
            69 => { pc = immediate_offset; if validate_stack(0, 1) then execute_gaslimit() },
            70 => { pc = immediate_offset; if validate_stack(0, 1) then execute_chainid() },
            71 => { pc = immediate_offset; if validate_stack(0, 1) then execute_selfbalance() },
            72 => {
                pc = immediate_offset;
                if profile.fork >= London then {
                    if validate_stack(0, 1) then execute_basefee()
                } else {
                    if validate_stack(0, 0) then execute_invalid()
                }
            },
            73 => {
                pc = immediate_offset;
                if profile.fork >= Cancun then {
                    if validate_stack(1, 1) then execute_blobhash()
                } else {
                    if validate_stack(0, 0) then execute_invalid()
                }
            },
            74 => {
                pc = immediate_offset;
                if profile.fork >= Cancun then {
                    if validate_stack(0, 1) then execute_blobbasefee()
                } else {
                    if validate_stack(0, 0) then execute_invalid()
                }
            },
            75 => {
                pc = immediate_offset;
                if profile.fork >= Amsterdam then {
                    if validate_stack(0, 1) then execute_slotnum()
                } else {
                    if validate_stack(0, 0) then execute_invalid()
                }
            },
            80 => { pc = immediate_offset; if validate_stack(1, 0) then execute_pop() },
            81 => { pc = immediate_offset; if validate_stack(1, 1) then execute_mload() },
            82 => { pc = immediate_offset; if validate_stack(2, 0) then execute_mstore() },
            83 => { pc = immediate_offset; if validate_stack(2, 0) then execute_mstore8() },
            84 => { pc = immediate_offset; if validate_stack(1, 1) then execute_sload() },
            85 => { pc = immediate_offset; if validate_stack(2, 0) then execute_sstore() },
            86 => { pc = immediate_offset; if validate_stack(1, 0) then execute_jump() },
            87 => { pc = immediate_offset; if validate_stack(2, 0) then execute_jumpi() },
            88 => { pc = immediate_offset; if validate_stack(0, 1) then execute_pc() },
            89 => { pc = immediate_offset; if validate_stack(0, 1) then execute_msize() },
            90 => { pc = immediate_offset; if validate_stack(0, 1) then execute_gas() },
            91 => { pc = immediate_offset; if validate_stack(0, 0) then execute_jumpdest() },
            92 => {
                pc = immediate_offset;
                if profile.fork >= Cancun then {
                    if validate_stack(1, 1) then execute_tload()
                } else {
                    if validate_stack(0, 0) then execute_invalid()
                }
            },
            93 => {
                pc = immediate_offset;
                if profile.fork >= Cancun then {
                    if validate_stack(2, 0) then execute_tstore()
                } else {
                    if validate_stack(0, 0) then execute_invalid()
                }
            },
            94 => {
                pc = immediate_offset;
                if profile.fork >= Cancun then {
                    if validate_stack(3, 0) then execute_mcopy()
                } else {
                    if validate_stack(0, 0) then execute_invalid()
                }
            },
            95 => {
                pc = immediate_offset;
                if profile.fork < Shanghai then {
                    if validate_stack(0, 0) then execute_invalid()
                } else {
                    if validate_stack(0, 1) then execute_push(0, read_push(code, immediate_offset, 0))
                }
            },
            96 => { let value = read_push(code, immediate_offset, 1); pc = current + 2; if validate_stack(0, 1) then execute_push(1, value) },
            97 => { let value = read_push(code, immediate_offset, 2); pc = current + 3; if validate_stack(0, 1) then execute_push(2, value) },
            98 => { let value = read_push(code, immediate_offset, 3); pc = current + 4; if validate_stack(0, 1) then execute_push(3, value) },
            99 => { let value = read_push(code, immediate_offset, 4); pc = current + 5; if validate_stack(0, 1) then execute_push(4, value) },
            100 => { let value = read_push(code, immediate_offset, 5); pc = current + 6; if validate_stack(0, 1) then execute_push(5, value) },
            101 => { let value = read_push(code, immediate_offset, 6); pc = current + 7; if validate_stack(0, 1) then execute_push(6, value) },
            102 => { let value = read_push(code, immediate_offset, 7); pc = current + 8; if validate_stack(0, 1) then execute_push(7, value) },
            103 => { let value = read_push(code, immediate_offset, 8); pc = current + 9; if validate_stack(0, 1) then execute_push(8, value) },
            104 => { let value = read_push(code, immediate_offset, 9); pc = current + 10; if validate_stack(0, 1) then execute_push(9, value) },
            105 => { let value = read_push(code, immediate_offset, 10); pc = current + 11; if validate_stack(0, 1) then execute_push(10, value) },
            106 => { let value = read_push(code, immediate_offset, 11); pc = current + 12; if validate_stack(0, 1) then execute_push(11, value) },
            107 => { let value = read_push(code, immediate_offset, 12); pc = current + 13; if validate_stack(0, 1) then execute_push(12, value) },
            108 => { let value = read_push(code, immediate_offset, 13); pc = current + 14; if validate_stack(0, 1) then execute_push(13, value) },
            109 => { let value = read_push(code, immediate_offset, 14); pc = current + 15; if validate_stack(0, 1) then execute_push(14, value) },
            110 => { let value = read_push(code, immediate_offset, 15); pc = current + 16; if validate_stack(0, 1) then execute_push(15, value) },
            111 => { let value = read_push(code, immediate_offset, 16); pc = current + 17; if validate_stack(0, 1) then execute_push(16, value) },
            112 => { let value = read_push(code, immediate_offset, 17); pc = current + 18; if validate_stack(0, 1) then execute_push(17, value) },
            113 => { let value = read_push(code, immediate_offset, 18); pc = current + 19; if validate_stack(0, 1) then execute_push(18, value) },
            114 => { let value = read_push(code, immediate_offset, 19); pc = current + 20; if validate_stack(0, 1) then execute_push(19, value) },
            115 => { let value = read_push(code, immediate_offset, 20); pc = current + 21; if validate_stack(0, 1) then execute_push(20, value) },
            116 => { let value = read_push(code, immediate_offset, 21); pc = current + 22; if validate_stack(0, 1) then execute_push(21, value) },
            117 => { let value = read_push(code, immediate_offset, 22); pc = current + 23; if validate_stack(0, 1) then execute_push(22, value) },
            118 => { let value = read_push(code, immediate_offset, 23); pc = current + 24; if validate_stack(0, 1) then execute_push(23, value) },
            119 => { let value = read_push(code, immediate_offset, 24); pc = current + 25; if validate_stack(0, 1) then execute_push(24, value) },
            120 => { let value = read_push(code, immediate_offset, 25); pc = current + 26; if validate_stack(0, 1) then execute_push(25, value) },
            121 => { let value = read_push(code, immediate_offset, 26); pc = current + 27; if validate_stack(0, 1) then execute_push(26, value) },
            122 => { let value = read_push(code, immediate_offset, 27); pc = current + 28; if validate_stack(0, 1) then execute_push(27, value) },
            123 => { let value = read_push(code, immediate_offset, 28); pc = current + 29; if validate_stack(0, 1) then execute_push(28, value) },
            124 => { let value = read_push(code, immediate_offset, 29); pc = current + 30; if validate_stack(0, 1) then execute_push(29, value) },
            125 => { let value = read_push(code, immediate_offset, 30); pc = current + 31; if validate_stack(0, 1) then execute_push(30, value) },
            126 => { let value = read_push(code, immediate_offset, 31); pc = current + 32; if validate_stack(0, 1) then execute_push(31, value) },
            127 => { let value = read_push(code, immediate_offset, 32); pc = current + 33; if validate_stack(0, 1) then execute_push(32, value) },
            128 => { pc = immediate_offset; if validate_stack(1, 2) then execute_dup(1) },
            129 => { pc = immediate_offset; if validate_stack(2, 3) then execute_dup(2) },
            130 => { pc = immediate_offset; if validate_stack(3, 4) then execute_dup(3) },
            131 => { pc = immediate_offset; if validate_stack(4, 5) then execute_dup(4) },
            132 => { pc = immediate_offset; if validate_stack(5, 6) then execute_dup(5) },
            133 => { pc = immediate_offset; if validate_stack(6, 7) then execute_dup(6) },
            134 => { pc = immediate_offset; if validate_stack(7, 8) then execute_dup(7) },
            135 => { pc = immediate_offset; if validate_stack(8, 9) then execute_dup(8) },
            136 => { pc = immediate_offset; if validate_stack(9, 10) then execute_dup(9) },
            137 => { pc = immediate_offset; if validate_stack(10, 11) then execute_dup(10) },
            138 => { pc = immediate_offset; if validate_stack(11, 12) then execute_dup(11) },
            139 => { pc = immediate_offset; if validate_stack(12, 13) then execute_dup(12) },
            140 => { pc = immediate_offset; if validate_stack(13, 14) then execute_dup(13) },
            141 => { pc = immediate_offset; if validate_stack(14, 15) then execute_dup(14) },
            142 => { pc = immediate_offset; if validate_stack(15, 16) then execute_dup(15) },
            143 => { pc = immediate_offset; if validate_stack(16, 17) then execute_dup(16) },
            144 => { pc = immediate_offset; if validate_stack(2, 2) then execute_swap(1) },
            145 => { pc = immediate_offset; if validate_stack(3, 3) then execute_swap(2) },
            146 => { pc = immediate_offset; if validate_stack(4, 4) then execute_swap(3) },
            147 => { pc = immediate_offset; if validate_stack(5, 5) then execute_swap(4) },
            148 => { pc = immediate_offset; if validate_stack(6, 6) then execute_swap(5) },
            149 => { pc = immediate_offset; if validate_stack(7, 7) then execute_swap(6) },
            150 => { pc = immediate_offset; if validate_stack(8, 8) then execute_swap(7) },
            151 => { pc = immediate_offset; if validate_stack(9, 9) then execute_swap(8) },
            152 => { pc = immediate_offset; if validate_stack(10, 10) then execute_swap(9) },
            153 => { pc = immediate_offset; if validate_stack(11, 11) then execute_swap(10) },
            154 => { pc = immediate_offset; if validate_stack(12, 12) then execute_swap(11) },
            155 => { pc = immediate_offset; if validate_stack(13, 13) then execute_swap(12) },
            156 => { pc = immediate_offset; if validate_stack(14, 14) then execute_swap(13) },
            157 => { pc = immediate_offset; if validate_stack(15, 15) then execute_swap(14) },
            158 => { pc = immediate_offset; if validate_stack(16, 16) then execute_swap(15) },
            159 => { pc = immediate_offset; if validate_stack(17, 17) then execute_swap(16) },
            160 => { pc = immediate_offset; if validate_stack(2, 0) then execute_log(0) },
            161 => { pc = immediate_offset; if validate_stack(3, 0) then execute_log(1) },
            162 => { pc = immediate_offset; if validate_stack(4, 0) then execute_log(2) },
            163 => { pc = immediate_offset; if validate_stack(5, 0) then execute_log(3) },
            164 => { pc = immediate_offset; if validate_stack(6, 0) then execute_log(4) },
            230 => if profile.fork >= Amsterdam then {
                let immediate = slice_byte(code, immediate_offset);
                let immediate_valid = deep_stack_operation_immediate_valid(DeepStackDuplicate, immediate);
                pc = if immediate_valid then current + 2 else immediate_offset;
                execute(DUPN(immediate))
            } else {
                pc = immediate_offset;
                if validate_stack(0, 0) then execute_invalid()
            },
            231 => if profile.fork >= Amsterdam then {
                let immediate = slice_byte(code, immediate_offset);
                let immediate_valid = deep_stack_operation_immediate_valid(DeepStackSwap, immediate);
                pc = if immediate_valid then current + 2 else immediate_offset;
                execute(SWAPN(immediate))
            } else {
                pc = immediate_offset;
                if validate_stack(0, 0) then execute_invalid()
            },
            232 => if profile.fork >= Amsterdam then {
                let immediate = slice_byte(code, immediate_offset);
                let immediate_valid = deep_stack_operation_immediate_valid(DeepStackExchange, immediate);
                pc = if immediate_valid then current + 2 else immediate_offset;
                execute(EXCHANGE(immediate))
            } else {
                pc = immediate_offset;
                if validate_stack(0, 0) then execute_invalid()
            },
            240 => { pc = immediate_offset; if validate_stack(3, 1) then execute_create() },
            241 => { pc = immediate_offset; if validate_stack(7, 1) then execute_call() },
            242 => { pc = immediate_offset; if validate_stack(7, 1) then execute_callcode() },
            243 => { pc = immediate_offset; if validate_stack(2, 0) then execute_return() },
            244 => { pc = immediate_offset; if validate_stack(6, 1) then execute_delegatecall() },
            245 => { pc = immediate_offset; if validate_stack(4, 1) then execute_create2() },
            250 => { pc = immediate_offset; if validate_stack(6, 1) then execute_staticcall() },
            253 => { pc = immediate_offset; if validate_stack(2, 0) then execute_revert() },
            255 => { pc = immediate_offset; if validate_stack(1, 0) then execute_selfdestruct() },
            _ => { pc = immediate_offset; if validate_stack(0, 0) then execute_invalid() },
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
            step_fused()
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
