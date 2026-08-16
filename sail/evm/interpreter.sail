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

/*! Reports whether an opcode byte is defined by the active fork. Undefined
    opcode bytes remain available here and decode to `INVALID`; this predicate
    contains only fork-dependent availability so every interpreter can share
    the same deployment rules without duplicating them in its dispatch. */
function opcode_available(opcode : opcode, fork : Fork) -> bool =
    match opcode {
        30 => fork >= Osaka,
        72 => fork >= London,
        73 => fork >= Cancun,
        74 => fork >= Cancun,
        75 => fork >= Amsterdam,
        92 => fork >= Cancun,
        93 => fork >= Cancun,
        94 => fork >= Cancun,
        95 => fork >= Shanghai,
        230 => fork >= Amsterdam,
        231 => fork >= Amsterdam,
        232 => fork >= Amsterdam,
        _ => true,
    }

/*! Decodes one PUSH immediate and returns the semantic program counter after
    all encoded immediate bytes. Missing code bytes contribute zero to the
    value but still belong to the instruction encoding. */
function decode_push_immediate(
    frame_code : Code,
    immediate_offset : code_scan_position,
    width : push_width,
) -> (
    (code_pointer, word)
) = {
    let bytes = code_bytes(frame_code);
    let value = read_push(bytes, immediate_offset, width);
    (immediate_offset + width, value)
}

/*! Decodes the immediate of an Amsterdam deep-stack instruction. A valid
    immediate advances the counter; an invalid immediate remains unconsumed so
    the handler can report `InvalidOpcode` with the canonical instruction
    boundary. Reads beyond code are zero-padded. */
function decode_deep_immediate(
    frame_code : Code,
    immediate_offset : code_scan_position,
    operation : DeepStackOperation,
) -> (
    (code_pointer, byte)
) = {
    let bytes = code_bytes(frame_code);
    let immediate = slice_byte(bytes, immediate_offset);
    let immediate_valid = deep_stack_operation_immediate_valid(operation, immediate);
    let next_pc : code_pointer =
        if immediate_valid then immediate_offset + 1 else immediate_offset;
    (next_pc, immediate)
}

/*! Executes an encoded PUSH instruction from its immediate cursor. This is
    the shared semantic boundary used by raw-byte interpreters: decoding, PC
    progression, stack validation, gas charging, and the stack effect remain
    generated from Sail. */
function execute_push_encoded(
    frame_code : Code,
    opcode : opcode,
    immediate_offset : code_scan_position,
    execution_gas : gas,
    sp : StackPointer,
) -> (
    (code_pointer, gas, StackPointer, OpcodeOutcome)
) = {
    if 95 <= opcode & opcode <= 127 then {
        let width : push_width = opcode - 95;
        let (next_pc, value) = decode_push_immediate(frame_code, immediate_offset, width);
        let (gas_after, sp_after, status_after) = execute_push(execution_gas, sp, width, value);
        (next_pc, gas_after, sp_after, status_after)
    } else {
        let (gas_after, status_after) = execute_invalid(execution_gas);
        (immediate_offset, gas_after, sp, status_after)
    }
}

/*! Executes one opcode from the `DUP1` through `DUP16` family. The raw-byte
    interpreter routes the family here without reproducing its index
    relationship. */
function execute_dup_encoded(
    opcode : opcode,
    execution_gas : gas,
    sp : StackPointer,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    if 128 <= opcode & opcode <= 143 then {
        execute_dup(execution_gas, sp, opcode - 127)
    } else {
        let (gas_after, status_after) = execute_invalid(execution_gas);
        (gas_after, sp, status_after)
    }
}

/*! Executes one opcode from the `SWAP1` through `SWAP16` family. */
function execute_swap_encoded(
    opcode : opcode,
    execution_gas : gas,
    sp : StackPointer,
) -> (
    (gas, StackPointer, OpcodeOutcome)
) = {
    if 144 <= opcode & opcode <= 159 then {
        execute_swap(execution_gas, sp, opcode - 143)
    } else {
        let (gas_after, status_after) = execute_invalid(execution_gas);
        (gas_after, sp, status_after)
    }
}

/*! Executes one opcode from the `LOG0` through `LOG4` family. */
function execute_log_encoded(
    carried_address : address,
    carried_is_static : bool,
    execution_gas : gas,
    sp : StackPointer,
    memory : EvmMemorySlice,
    opcode : opcode,
) -> (
    (gas, StackPointer, EvmMemorySlice, OpcodeOutcome)
) = {
    if 160 <= opcode & opcode <= 164 then {
        execute_log(carried_address, carried_is_static, execution_gas, sp, memory, opcode - 160)
    } else {
        let (gas_after, status_after) = execute_invalid(execution_gas);
        (gas_after, sp, memory, status_after)
    }
}

/*! Executes an encoded Amsterdam deep-stack instruction from its immediate
    cursor. Opcode classification and immediate validity are specification
    semantics; the raw-byte interpreter supplies only the opcode byte. */
function execute_deep_stack_encoded(
    frame_code : Code,
    opcode : opcode,
    immediate_offset : code_scan_position,
    execution_gas : gas,
    sp : StackPointer,
) -> (
    (code_pointer, gas, StackPointer, OpcodeOutcome)
) = {
    let operation = deep_stack_operation(opcode);
    let (next_pc, immediate) = decode_deep_immediate(frame_code, immediate_offset, operation);
    let result : (gas, StackPointer, OpcodeOutcome) = match operation {
        DeepStackDuplicate => execute_dupn(execution_gas, sp, immediate),
        DeepStackSwap => execute_swapn(execution_gas, sp, immediate),
        DeepStackExchange => execute_exchange(execution_gas, sp, immediate),
        NotDeepStackOperation => {
            let (gas_after, status_after) = execute_invalid(execution_gas);
            (gas_after, sp, status_after)
        },
    };
    let (gas_after, sp_after, status_after) = result;
    (next_pc, gas_after, sp_after, status_after)
}

/* Decode one non-PUSH opcode byte to its AST node. The three contiguous
 * families fold to an arity argument -- DUP1..16 (0x80..0x8f), SWAP1..16
 * (0x90..0x9f), LOG0..4 (0xa0..0xa4) -- and the remainder is a flat table.
 * Any byte with no defined opcode decodes to INVALID. */
/*! Decodes one non-PUSH opcode byte to its AST node. The three
    contiguous families fold to an arity argument — `DUP1`–`DUP16`
    (0x80–0x8f), `SWAP1`–`SWAP16` (0x90–0x9f), `LOG0`–`LOG4` (0xa0–0xa4)
    — and the remainder is a flat table. Any byte with no defined opcode
    decodes to `INVALID`. */
function decode_simple(opcode : opcode, fork : Fork) -> ast = {
    let available = opcode_available(opcode, fork);
    if not_bool(available) then {
        INVALID()
    } else if 128 <= opcode & opcode <= 143 then {
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
            30 => CLZ(), /* EIP-7939: Osaka+ */
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
            75 => SLOTNUM(), /* EIP-7843: Amsterdam+ (0x4b is undefined earlier) */
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
function fetch(frame_code : Code, current : code_pointer, fork : Fork) -> (code_pointer, ast) = {
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
        let available = opcode_available(opcode, fork);
        let decoded : (code_pointer, ast) =
            if not_bool(available)
            then (immediate_offset, INVALID())
            else if 95 <= opcode & opcode <= 127 then {
                let size : push_width = opcode - 95;
                let (after_immediate, value) = decode_push_immediate(frame_code, immediate_offset, size);
                (after_immediate, PUSH(size, value))
            } else {
                let deep_operation = deep_stack_operation(opcode);
                match deep_operation {
                    NotDeepStackOperation => (immediate_offset, decode_simple(opcode, fork)),
                    operation => {
                        let (after_instruction, immediate) = decode_deep_immediate(
                            frame_code,
                            immediate_offset,
                            operation,
                        );
                        let instruction : ast = match operation {
                            DeepStackDuplicate => DUPN(immediate),
                            DeepStackSwap => SWAPN(immediate),
                            DeepStackExchange => EXCHANGE(immediate),
                            NotDeepStackOperation => decode_simple(opcode, fork),
                        };
                        (after_instruction, instruction)
                    },
                }
            };
        decoded
    }
}

/*! The operand-stack effect of one decoded instruction.  Validation belongs
    at the instruction boundary: once this effect has been admitted, the
    selected handler may read and write its operands without repeating the
    same precondition.  Invalid deep-stack immediates have no stack effect;
    their handlers report `InvalidOpcode` before touching the stack. */
/*md
## The run loop
*/
val run_call : (
        code_pointer,
        gas,
        state_gas,
        state_gas_spill,
        gas_refund,
        StackPointer,
        EvmMemorySlice,
        address,
        address,
        address,
        word,
        state_gas,
        bool,
        frame_depth,
        Code,
        CalldataSlice,
        OutputSlice,
        CallKind,
    ) -> (
        code_pointer,
        gas,
        state_gas,
        state_gas_spill,
        gas_refund,
        FrameStatus,
        StackPointer,
        EvmMemorySlice,
        address,
        address,
        address,
        word,
        state_gas,
        bool,
        frame_depth,
        Code,
        CalldataSlice,
        OutputSlice,
    )

val run_create : (
        code_pointer,
        gas,
        state_gas,
        state_gas_spill,
        gas_refund,
        StackPointer,
        EvmMemorySlice,
        address,
        address,
        address,
        word,
        state_gas,
        bool,
        frame_depth,
        Code,
        CalldataSlice,
        OutputSlice,
        CreateKind,
    ) -> (
        code_pointer,
        gas,
        state_gas,
        state_gas_spill,
        gas_refund,
        FrameStatus,
        StackPointer,
        EvmMemorySlice,
        address,
        address,
        address,
        word,
        state_gas,
        bool,
        frame_depth,
        Code,
        CalldataSlice,
        OutputSlice,
    )

val resume_frame : (FrameContinuation, OutputSlice, gas, state_gas, state_gas_spill, gas_refund, FrameStatus, state_gas) -> (
        code_pointer,
        gas,
        state_gas,
        state_gas_spill,
        gas_refund,
        FrameStatus,
        StackPointer,
        EvmMemorySlice,
        address,
        address,
        address,
        word,
        state_gas,
        bool,
        frame_depth,
        Code,
        CalldataSlice,
        OutputSlice,
    )

/*! Returns the active frame's halt output. */
function frame_output(frame_status : FrameStatus) -> OutputSlice =
    match frame_status {
        Halted(HaltReturn(output)) => output,
        Halted(HaltRevert(output)) => output,
        _ => EMPTY_OUTPUT_SLICE,
    }

/*! A decreasing bound for the non-recursive interpreter's complete call tree. */
type call_tree_steps = range(0, 3 * (2 * (2 ^ 64 - 1)) + 2)

/*! The non-recursive step loop for one complete call tree. It executes
    the active frame, resumes suspended parents through
    [frame_stack_pop][] as children halt, and returns the top-level
    frame's output. Each step's carried state is supplied from the frame
    registers and its returned state is assigned back; the handlers
    themselves never touch the registers. `STOP`, `SELFDESTRUCT`, and
    exceptional halts return the empty slice; `RETURN` and `REVERT` carry
    their frozen memory slice in the halt value. */
function interpret(
    initial_gas : gas,
    initial_state_gas : state_gas,
    initial_state_spill : state_gas_spill,
    initial_refund : gas_refund,
    initial_sp : StackPointer,
    initial_memory : EvmMemorySlice,
    initial_caller : address,
    initial_address : address,
    initial_code_address : address,
    initial_value : word,
    initial_state_gas_reservoir : state_gas,
    initial_is_static : bool,
    initial_depth : frame_depth,
    initial_code : Code,
    initial_calldata : CalldataSlice,
) -> (
    (gas, state_gas, state_gas_spill, gas_refund, FrameStatus, OutputSlice)
) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let fork = profile.fork;
    let blob_fee = blob_base_fee(fork, profile.blob_schedule, profile.excess_blob_gas_limit, k_header.excess_blob_gas);
    frame_stack_reset();
    var interpreting : bool = true;
    var result : OutputSlice = EMPTY_OUTPUT_SLICE;
    var carried_pc : code_pointer = 0;
    var carried_sp : StackPointer = initial_sp;
    var carried_memory : EvmMemorySlice = initial_memory;
    var carried_gas : gas = initial_gas;
    var carried_state_gas : state_gas = initial_state_gas;
    var carried_state_spill : state_gas_spill = initial_state_spill;
    var carried_refund : gas_refund = initial_refund;
    var carried_status : FrameStatus = Running();
    var carried_caller : address = initial_caller;
    var carried_address : address = initial_address;
    var carried_account_context : AccountExecutionContext = account_execution_context(initial_address);
    var carried_code_address : address = initial_code_address;
    var carried_value : word = initial_value;
    var carried_state_gas_reservoir : state_gas = initial_state_gas_reservoir;
    var carried_is_static : bool = initial_is_static;
    var carried_depth : frame_depth = initial_depth;
    var carried_code : Code = initial_code;
    var carried_calldata : CalldataSlice = initial_calldata;
    var carried_returndata : OutputSlice = EMPTY_OUTPUT_SLICE;

    let initial_call_tree_gas = initial_gas + initial_state_gas;
    var call_tree_steps_remaining : call_tree_steps = 3 * initial_call_tree_gas + 2;
    while interpreting termination_measure(call_tree_steps_remaining) do {
        let running = is_running(carried_status);
        if running then {
            let (fetched_pc, instruction) = fetch(carried_code, carried_pc, fork);
            carried_pc = fetched_pc;
            match instruction {
                opcode_CREATE() => {
                    let previous_address = carried_address;
                    (
                        carried_pc,
                        carried_gas,
                        carried_state_gas,
                        carried_state_spill,
                        carried_refund,
                        carried_status,
                        carried_sp,
                        carried_memory,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        carried_returndata,
                    ) = run_create(
                        carried_pc,
                        carried_gas,
                        carried_state_gas,
                        carried_state_spill,
                        carried_refund,
                        carried_sp,
                        carried_memory,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        carried_returndata,
                        CreateByNonce,
                    );
                    carried_account_context = refresh_account_execution_context(
                        carried_account_context,
                        previous_address,
                        carried_address,
                    )
                },
                CREATE2() => {
                    let previous_address = carried_address;
                    (
                        carried_pc,
                        carried_gas,
                        carried_state_gas,
                        carried_state_spill,
                        carried_refund,
                        carried_status,
                        carried_sp,
                        carried_memory,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        carried_returndata,
                    ) = run_create(
                        carried_pc,
                        carried_gas,
                        carried_state_gas,
                        carried_state_spill,
                        carried_refund,
                        carried_sp,
                        carried_memory,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        carried_returndata,
                        CreateBySalt,
                    );
                    carried_account_context = refresh_account_execution_context(
                        carried_account_context,
                        previous_address,
                        carried_address,
                    )
                },
                CALL() => {
                    let previous_address = carried_address;
                    (
                        carried_pc,
                        carried_gas,
                        carried_state_gas,
                        carried_state_spill,
                        carried_refund,
                        carried_status,
                        carried_sp,
                        carried_memory,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        carried_returndata,
                    ) = run_call(
                        carried_pc,
                        carried_gas,
                        carried_state_gas,
                        carried_state_spill,
                        carried_refund,
                        carried_sp,
                        carried_memory,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        carried_returndata,
                        Call,
                    );
                    carried_account_context = refresh_account_execution_context(
                        carried_account_context,
                        previous_address,
                        carried_address,
                    )
                },
                CALLCODE() => {
                    let previous_address = carried_address;
                    (
                        carried_pc,
                        carried_gas,
                        carried_state_gas,
                        carried_state_spill,
                        carried_refund,
                        carried_status,
                        carried_sp,
                        carried_memory,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        carried_returndata,
                    ) = run_call(
                        carried_pc,
                        carried_gas,
                        carried_state_gas,
                        carried_state_spill,
                        carried_refund,
                        carried_sp,
                        carried_memory,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        carried_returndata,
                        CallCode,
                    );
                    carried_account_context = refresh_account_execution_context(
                        carried_account_context,
                        previous_address,
                        carried_address,
                    )
                },
                DELEGATECALL() => {
                    let previous_address = carried_address;
                    (
                        carried_pc,
                        carried_gas,
                        carried_state_gas,
                        carried_state_spill,
                        carried_refund,
                        carried_status,
                        carried_sp,
                        carried_memory,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        carried_returndata,
                    ) = run_call(
                        carried_pc,
                        carried_gas,
                        carried_state_gas,
                        carried_state_spill,
                        carried_refund,
                        carried_sp,
                        carried_memory,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        carried_returndata,
                        DelegateCall,
                    );
                    carried_account_context = refresh_account_execution_context(
                        carried_account_context,
                        previous_address,
                        carried_address,
                    )
                },
                STATICCALL() => {
                    let previous_address = carried_address;
                    (
                        carried_pc,
                        carried_gas,
                        carried_state_gas,
                        carried_state_spill,
                        carried_refund,
                        carried_status,
                        carried_sp,
                        carried_memory,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        carried_returndata,
                    ) = run_call(
                        carried_pc,
                        carried_gas,
                        carried_state_gas,
                        carried_state_spill,
                        carried_refund,
                        carried_sp,
                        carried_memory,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        carried_returndata,
                        StaticCall,
                    );
                    carried_account_context = refresh_account_execution_context(
                        carried_account_context,
                        previous_address,
                        carried_address,
                    )
                },
                _ => {
                    let result :
                        (
                            code_pointer,
                            gas,
                            state_gas,
                            state_gas_spill,
                            gas_refund,
                            StackPointer,
                            EvmMemorySlice,
                            FrameStatus,
                        ) = match instruction {
                        STOP() => {
                            let status_after = execute_stop();
                            (
                                carried_pc,
                                carried_gas,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                carried_sp,
                                carried_memory,
                                status_after,
                            )
                        },
                        ADD() => {
                            let (gas_after, sp_after, status_after) = execute_add(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        MUL() => {
                            let (gas_after, sp_after, status_after) = execute_mul(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        SUB() => {
                            let (gas_after, sp_after, status_after) = execute_sub(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        DIV() => {
                            let (gas_after, sp_after, status_after) = execute_div(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        SDIV() => {
                            let (gas_after, sp_after, status_after) = execute_sdiv(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        MOD() => {
                            let (gas_after, sp_after, status_after) = execute_mod(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        SMOD() => {
                            let (gas_after, sp_after, status_after) = execute_smod(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        ADDMOD() => {
                            let (gas_after, sp_after, status_after) = execute_addmod(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        MULMOD() => {
                            let (gas_after, sp_after, status_after) = execute_mulmod(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        EXP() => {
                            let (gas_after, sp_after, status_after) = execute_exp(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        SIGNEXTEND() => {
                            let (gas_after, sp_after, status_after) = execute_signextend(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        LT() => {
                            let (gas_after, sp_after, status_after) = execute_lt(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        GT() => {
                            let (gas_after, sp_after, status_after) = execute_gt(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        SLT() => {
                            let (gas_after, sp_after, status_after) = execute_slt(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        SGT() => {
                            let (gas_after, sp_after, status_after) = execute_sgt(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        EQ() => {
                            let (gas_after, sp_after, status_after) = execute_eq(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        ISZERO() => {
                            let (gas_after, sp_after, status_after) = execute_iszero(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        AND() => {
                            let (gas_after, sp_after, status_after) = execute_and(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        OR() => {
                            let (gas_after, sp_after, status_after) = execute_or(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        XOR() => {
                            let (gas_after, sp_after, status_after) = execute_xor(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        NOT() => {
                            let (gas_after, sp_after, status_after) = execute_not(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        BYTE() => {
                            let (gas_after, sp_after, status_after) = execute_byte(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        SHL() => {
                            let (gas_after, sp_after, status_after) = execute_shl(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        SHR() => {
                            let (gas_after, sp_after, status_after) = execute_shr(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        SAR() => {
                            let (gas_after, sp_after, status_after) = execute_sar(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        CLZ() => {
                            let (gas_after, sp_after, status_after) = execute_clz(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        KECCAK256() => {
                            let (gas_after, sp_after, memory_after, status_after) = execute_keccak256(
                                carried_gas,
                                carried_sp,
                                carried_memory,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                memory_after,
                                opcode_frame_status(status_after),
                            )
                        },
                        ADDRESS() => {
                            let (gas_after, sp_after, status_after) = execute_address(
                                carried_address,
                                carried_gas,
                                carried_sp,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        BALANCE() => {
                            let (gas_after, sp_after, status_after) = execute_balance(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        ORIGIN() => {
                            let (gas_after, sp_after, status_after) = execute_origin(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        CALLER() => {
                            let (gas_after, sp_after, status_after) = execute_caller(
                                carried_caller,
                                carried_gas,
                                carried_sp,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        CALLVALUE() => {
                            let (gas_after, sp_after, status_after) = execute_callvalue(
                                carried_value,
                                carried_gas,
                                carried_sp,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        CALLDATALOAD() => {
                            let (gas_after, sp_after, status_after) = execute_calldataload(
                                carried_calldata,
                                carried_gas,
                                carried_sp,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        CALLDATASIZE() => {
                            let (gas_after, sp_after, status_after) = execute_calldatasize(
                                carried_calldata,
                                carried_gas,
                                carried_sp,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        CALLDATACOPY() => {
                            let (gas_after, sp_after, memory_after, status_after) = execute_calldatacopy(
                                carried_calldata,
                                carried_gas,
                                carried_sp,
                                carried_memory,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                memory_after,
                                opcode_frame_status(status_after),
                            )
                        },
                        CODESIZE() => {
                            let (gas_after, sp_after, status_after) = execute_codesize(
                                carried_code,
                                carried_gas,
                                carried_sp,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        CODECOPY() => {
                            let (gas_after, sp_after, memory_after, status_after) = execute_codecopy(
                                carried_code,
                                carried_gas,
                                carried_sp,
                                carried_memory,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                memory_after,
                                opcode_frame_status(status_after),
                            )
                        },
                        GASPRICE() => {
                            let (gas_after, sp_after, status_after) = execute_gasprice(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        EXTCODESIZE() => {
                            let (gas_after, sp_after, status_after) = execute_extcodesize(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        EXTCODECOPY() => {
                            let (gas_after, sp_after, memory_after, status_after) = execute_extcodecopy(
                                carried_gas,
                                carried_sp,
                                carried_memory,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                memory_after,
                                opcode_frame_status(status_after),
                            )
                        },
                        RETURNDATASIZE() => {
                            let (gas_after, sp_after, status_after) = execute_returndatasize(
                                carried_returndata,
                                carried_gas,
                                carried_sp,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        RETURNDATACOPY() => {
                            let (gas_after, sp_after, memory_after, status_after) = execute_returndatacopy(
                                carried_returndata,
                                carried_gas,
                                carried_sp,
                                carried_memory,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                memory_after,
                                opcode_frame_status(status_after),
                            )
                        },
                        EXTCODEHASH() => {
                            let (gas_after, sp_after, status_after) = execute_extcodehash(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        BLOCKHASH() => {
                            let (gas_after, sp_after, status_after) = execute_blockhash(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        COINBASE() => {
                            let (gas_after, sp_after, status_after) = execute_coinbase(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        TIMESTAMP() => {
                            let (gas_after, sp_after, status_after) = execute_timestamp(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        NUMBER() => {
                            let (gas_after, sp_after, status_after) = execute_number(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        SLOTNUM() => {
                            let (gas_after, sp_after, status_after) = execute_slotnum(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        PREVRANDAO() => {
                            let (gas_after, sp_after, status_after) = execute_prevrandao(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        GASLIMIT() => {
                            let (gas_after, sp_after, status_after) = execute_gaslimit(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        CHAINID() => {
                            let (gas_after, sp_after, status_after) = execute_chainid(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        SELFBALANCE() => {
                            let (gas_after, sp_after, status_after) = execute_selfbalance(
                                carried_address,
                                carried_gas,
                                carried_sp,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        BASEFEE() => {
                            let (gas_after, sp_after, status_after) = execute_basefee(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        BLOBHASH() => {
                            let (gas_after, sp_after, status_after) = execute_blobhash(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        BLOBBASEFEE() => {
                            let (gas_after, sp_after, status_after) = execute_blobbasefee(
                                blob_fee,
                                carried_gas,
                                carried_sp,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        POP() => {
                            let (gas_after, sp_after, status_after) = execute_pop(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        MLOAD() => {
                            let (gas_after, sp_after, memory_after, status_after) = execute_mload(
                                carried_gas,
                                carried_sp,
                                carried_memory,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                memory_after,
                                opcode_frame_status(status_after),
                            )
                        },
                        MSTORE() => {
                            let (gas_after, sp_after, memory_after, status_after) = execute_mstore(
                                carried_gas,
                                carried_sp,
                                carried_memory,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                memory_after,
                                opcode_frame_status(status_after),
                            )
                        },
                        MSTORE8() => {
                            let (gas_after, sp_after, memory_after, status_after) = execute_mstore8(
                                carried_gas,
                                carried_sp,
                                carried_memory,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                memory_after,
                                opcode_frame_status(status_after),
                            )
                        },
                        SLOAD() => {
                            let (gas_after, sp_after, status_after) = execute_sload(
                                carried_account_context,
                                carried_gas,
                                carried_sp,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        SSTORE() => {
                            let (gas_after, state_gas_after, state_spill_after, refund_after, sp_after, status_after) = execute_sstore(
                                carried_account_context,
                                fork,
                                carried_is_static,
                                carried_gas,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                carried_sp,
                            );
                            (
                                carried_pc,
                                gas_after,
                                state_gas_after,
                                state_spill_after,
                                refund_after,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        JUMP() => {
                            let (pc_after, gas_after, sp_after, status_after) = execute_jump(
                                carried_code,
                                carried_pc,
                                carried_gas,
                                carried_sp,
                            );
                            (
                                pc_after,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        JUMPI() => {
                            let (pc_after, gas_after, sp_after, status_after) = execute_jumpi(
                                carried_code,
                                carried_pc,
                                carried_gas,
                                carried_sp,
                            );
                            (
                                pc_after,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        PC() => {
                            let (pc_after, gas_after, sp_after, status_after) = execute_pc(
                                carried_pc,
                                carried_gas,
                                carried_sp,
                            );
                            (
                                pc_after,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        MSIZE() => {
                            let (gas_after, sp_after, memory_after, status_after) = execute_msize(
                                carried_gas,
                                carried_sp,
                                carried_memory,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                memory_after,
                                opcode_frame_status(status_after),
                            )
                        },
                        GAS() => {
                            let (gas_after, sp_after, status_after) = execute_gas(carried_gas, carried_sp);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        JUMPDEST() => {
                            let (gas_after, status_after) = execute_jumpdest(carried_gas);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                carried_sp,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        TLOAD() => {
                            let (gas_after, sp_after, status_after) = execute_tload(
                                carried_address,
                                carried_gas,
                                carried_sp,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        TSTORE() => {
                            let (gas_after, sp_after, status_after) = execute_tstore(
                                carried_address,
                                carried_is_static,
                                carried_gas,
                                carried_sp,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        MCOPY() => {
                            let (gas_after, sp_after, memory_after, status_after) = execute_mcopy(
                                carried_gas,
                                carried_sp,
                                carried_memory,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                memory_after,
                                opcode_frame_status(status_after),
                            )
                        },
                        PUSH(n, value) => {
                            let (gas_after, sp_after, status_after) = execute_push(carried_gas, carried_sp, n, value);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        DUP(n) => {
                            let (gas_after, sp_after, status_after) = execute_dup(carried_gas, carried_sp, n);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        SWAP(n) => {
                            let (gas_after, sp_after, status_after) = execute_swap(carried_gas, carried_sp, n);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        DUPN(immediate) => {
                            let (gas_after, sp_after, status_after) = execute_dupn(carried_gas, carried_sp, immediate);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        SWAPN(immediate) => {
                            let (gas_after, sp_after, status_after) = execute_swapn(carried_gas, carried_sp, immediate);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        EXCHANGE(immediate) => {
                            let (gas_after, sp_after, status_after) = execute_exchange(
                                carried_gas,
                                carried_sp,
                                immediate,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        LOG(n) => {
                            let (gas_after, sp_after, memory_after, status_after) = execute_log(
                                carried_address,
                                carried_is_static,
                                carried_gas,
                                carried_sp,
                                carried_memory,
                                n,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                memory_after,
                                opcode_frame_status(status_after),
                            )
                        },
                        opcode_CREATE() => fatal_error(ExecutionInvalid),
                        CREATE2() => fatal_error(ExecutionInvalid),
                        CALL() => fatal_error(ExecutionInvalid),
                        CALLCODE() => fatal_error(ExecutionInvalid),
                        DELEGATECALL() => fatal_error(ExecutionInvalid),
                        STATICCALL() => fatal_error(ExecutionInvalid),
                        RETURN() => {
                            let (gas_after, sp_after, memory_after, status_after) = execute_return(
                                carried_gas,
                                carried_sp,
                                carried_memory,
                            );
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                sp_after,
                                memory_after,
                                status_after,
                            )
                        },
                        REVERT() => {
                            let (gas_after, state_gas_after, state_spill_after, sp_after, memory_after, status_after) = execute_revert(
                                carried_state_gas_reservoir,
                                carried_gas,
                                carried_state_gas,
                                carried_state_spill,
                                carried_sp,
                                carried_memory,
                            );
                            (
                                carried_pc,
                                gas_after,
                                state_gas_after,
                                state_spill_after,
                                carried_refund,
                                sp_after,
                                memory_after,
                                status_after,
                            )
                        },
                        INVALID() => {
                            let (gas_after, status_after) = execute_invalid(carried_gas);
                            (
                                carried_pc,
                                gas_after,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                carried_sp,
                                carried_memory,
                                opcode_frame_status(status_after),
                            )
                        },
                        SELFDESTRUCT() => {
                            let (gas_after, state_gas_after, state_spill_after, refund_after, sp_after, status_after) = execute_selfdestruct(
                                carried_address,
                                fork,
                                carried_is_static,
                                carried_gas,
                                carried_state_gas,
                                carried_state_spill,
                                carried_refund,
                                carried_sp,
                            );
                            (
                                carried_pc,
                                gas_after,
                                state_gas_after,
                                state_spill_after,
                                refund_after,
                                sp_after,
                                carried_memory,
                                status_after,
                            )
                        },
                    };
                    (
                        carried_pc,
                        carried_gas,
                        carried_state_gas,
                        carried_state_spill,
                        carried_refund,
                        carried_sp,
                        carried_memory,
                        carried_status,
                    ) = match result {
                        (
                            pc_after,
                            _,
                            state_gas_after,
                            state_spill_after,
                            refund_after,
                            sp_after,
                            memory_after,
                            Exceptional(kind),
                        ) => {
                            let (state_gas_after, state_spill_after, status_after) = exceptional_state(
                                state_gas_after,
                                state_spill_after,
                                carried_state_gas_reservoir,
                                kind,
                            );
                            (
                                pc_after,
                                GAS_ZERO,
                                state_gas_after,
                                state_spill_after,
                                refund_after,
                                sp_after,
                                memory_after,
                                status_after,
                            )
                        },
                        _ => result,
                    }
                },
            }
        } else {
            let output = frame_output(carried_status);
            let continuation = frame_stack_pop();
            match continuation {
                Empty() => {
                    result = output;
                    interpreting = false
                },
                continuation => {
                    let previous_address = carried_address;
                    (
                        carried_pc,
                        carried_gas,
                        carried_state_gas,
                        carried_state_spill,
                        carried_refund,
                        carried_status,
                        carried_sp,
                        carried_memory,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        carried_returndata,
                    ) = resume_frame(
                        continuation,
                        output,
                        carried_gas,
                        carried_state_gas,
                        carried_state_spill,
                        carried_refund,
                        carried_status,
                        carried_state_gas_reservoir,
                    );
                    carried_account_context = refresh_account_execution_context(
                        carried_account_context,
                        previous_address,
                        carried_address,
                    )
                },
            }
        };
        let remaining_steps = call_tree_steps_remaining;
        call_tree_steps_remaining =
            if remaining_steps == 0 then {
                0
            } else {
                remaining_steps - 1
            }
    };
    (carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_status, result)
}

/*! Whether the just-finished frame ended successfully: a normal halt
    succeeds; a `REVERT` and any exceptional halt do not (their world
    effects are rolled back and `CALL`/`CREATE` reports failure). */
function frame_succeeded(frame_status : FrameStatus) -> bool =
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
function call_stack_inputs(kind : CallKind) -> operand_stack_height =
    match kind {
        Call => 7,
        CallCode => 7,
        DelegateCall => 6,
        StaticCall => 6,
    }

/*! Executes a message-call instruction through its non-entering failure paths
    or installs the child frame and returns its initial carried machine state. */
function run_call(
    carried_pc : code_pointer,
    carried_gas : gas,
    carried_state_gas : state_gas,
    carried_state_spill : state_gas_spill,
    carried_refund : gas_refund,
    carried_sp : StackPointer,
    carried_memory : EvmMemorySlice,
    carried_caller : address,
    carried_address : address,
    carried_code_address : address,
    carried_value : word,
    carried_state_gas_reservoir : state_gas,
    carried_is_static : bool,
    carried_depth : frame_depth,
    carried_code : Code,
    carried_calldata : CalldataSlice,
    carried_returndata : OutputSlice,
    kind : CallKind,
) -> (
    (
        code_pointer,
        gas,
        state_gas,
        state_gas_spill,
        gas_refund,
        FrameStatus,
        StackPointer,
        EvmMemorySlice,
        address,
        address,
        address,
        word,
        state_gas,
        bool,
        frame_depth,
        Code,
        CalldataSlice,
        OutputSlice,
    )
) = {
    let stack_inputs = call_stack_inputs(kind);
    let stack_status = guard_stack(carried_sp, stack_inputs, 1);
    match stack_status {
        Failed(halt_kind) => {
            let (state_gas_after, state_spill_after, status_after) = exceptional_state(
                carried_state_gas,
                carried_state_spill,
                carried_state_gas_reservoir,
                halt_kind,
            );
            (
                carried_pc,
                GAS_ZERO,
                state_gas_after,
                state_spill_after,
                carried_refund,
                status_after,
                carried_sp,
                carried_memory,
                carried_caller,
                carried_address,
                carried_code_address,
                carried_value,
                carried_state_gas_reservoir,
                carried_is_static,
                carried_depth,
                carried_code,
                carried_calldata,
                carried_returndata,
            )
        },
        Continue() => {
            let pc_after : code_pointer = carried_pc;
            var gas_after : gas = carried_gas;
            var state_gas_after : state_gas = carried_state_gas;
            var state_spill_after : state_gas_spill = carried_state_spill;
            var status_after : FrameStatus = Running();
            var sp_after : StackPointer = carried_sp;
            var memory_after : EvmMemorySlice = carried_memory;
            var returndata_after : OutputSlice = carried_returndata;
            let parent_message : Message = struct {
                    caller = carried_caller,
                    address = carried_address,
                    code_address = carried_code_address,
                    value = carried_value,
                    state_gas_reservoir = carried_state_gas_reservoir,
                    is_static = carried_is_static,
                    depth = carried_depth,
                };
            let semantics = call_semantics(kind);
            let execution_profile = k_execution_profile;
            let profile = execution_profile.protocol;
            let current_depth = carried_depth;
            let caller = carried_address;
            let gas_request = read_stack_word(sp_after);
            sp_after = stack_top_retreat(sp_after, 1);
            let target_word = read_stack_word(sp_after);
            sp_after = stack_top_retreat(sp_after, 1);
            let target = word_to_address(target_word);
            let (value, next_sp) : (word, StackPointer) =
                if semantics.takes_value then {
                    let value = read_stack_word(sp_after);
                    (value, stack_top_retreat(sp_after, 1))
                } else {
                    (WORD_ZERO, sp_after)
                };
            sp_after = next_sp;
            let value_nonzero = word_nonzero(value);
            let args_off_word = read_stack_word(sp_after);
            sp_after = stack_top_retreat(sp_after, 1);
            let args_len_word = read_stack_word(sp_after);
            sp_after = stack_top_retreat(sp_after, 1);
            let ret_off_word = read_stack_word(sp_after);
            sp_after = stack_top_retreat(sp_after, 1);
            let ret_len_word = read_stack_word(sp_after);
            sp_after = stack_top_retreat(sp_after, 1);

            /* EIP-214: a value-bearing CALL inside a static context is a write and
               raises WriteInStaticContext -- an exceptional halt that consumes all the
               caller frame's gas. CALLCODE/DELEGATECALL/STATICCALL never trigger it
               (CALLCODE has no static guard in the spec; the others force value = 0). */
            if semantics.transfers_value & value_nonzero & carried_is_static then {
                gas_after = GAS_ZERO;
                (state_gas_after, state_spill_after, status_after) = exceptional_state(
                    state_gas_after,
                    state_spill_after,
                    carried_state_gas_reservoir,
                    WriteProtection,
                );
                return (
                    pc_after,
                    gas_after,
                    state_gas_after,
                    state_spill_after,
                    carried_refund,
                    status_after,
                    sp_after,
                    memory_after,
                    carried_caller,
                    carried_address,
                    carried_code_address,
                    carried_value,
                    carried_state_gas_reservoir,
                    carried_is_static,
                    carried_depth,
                    carried_code,
                    carried_calldata,
                    returndata_after,
                )
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
            let expansion_cost = memory_expansion_gas_cost(memory_after, required_size, gas_after);
            if not_bool(expansion_cost.affordable) then {
                gas_after = GAS_ZERO;
                (state_gas_after, state_spill_after, status_after) = exceptional_state(
                    state_gas_after,
                    state_spill_after,
                    carried_state_gas_reservoir,
                    OutOfGas,
                );
                return (
                    pc_after,
                    gas_after,
                    state_gas_after,
                    state_spill_after,
                    carried_refund,
                    status_after,
                    sp_after,
                    memory_after,
                    carried_caller,
                    carried_address,
                    carried_code_address,
                    carried_value,
                    carried_state_gas_reservoir,
                    carried_is_static,
                    carried_depth,
                    carried_code,
                    carried_calldata,
                    returndata_after,
                )
            };
            gas_after = gas_sub(gas_after, expansion_cost.cost);

            /* Static gas is charged before any state access: the target's access and
               value-transfer cost is checked after memory gas and before reading any
               of the target's state. A frame that cannot afford it halts here, having
               touched no account -- so the stateless witness need not prove the target.
               (Resolving the EIP-7702 delegation or the empty-account check below reads
               the target's account, which an OOG-before-access tx must never do.) */
            let static_base : gas_cost = target_cost + transfer_cost;
            if gas_after < static_base then {
                gas_after = GAS_ZERO;
                (state_gas_after, state_spill_after, status_after) = exceptional_state(
                    state_gas_after,
                    state_spill_after,
                    carried_state_gas_reservoir,
                    OutOfGas,
                );
                return (
                    pc_after,
                    gas_after,
                    state_gas_after,
                    state_spill_after,
                    carried_refund,
                    status_after,
                    sp_after,
                    memory_after,
                    carried_caller,
                    carried_address,
                    carried_code_address,
                    carried_value,
                    carried_state_gas_reservoir,
                    carried_is_static,
                    carried_depth,
                    carried_code,
                    carried_calldata,
                    returndata_after,
                )
            };
            gas_after = gas_sub(gas_after, static_base);

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
            let new_account_charged =    profile.fork
                                      >= Amsterdam
                                      &  value_nonzero
                                      &  semantics.transfers_value
                                      &  target_empty;
            let create_cost : gas_constant =
                if profile.fork < Amsterdam & value_nonzero & semantics.transfers_value & target_empty
                then G_newaccount
                else GAS_CONSTANT_ZERO;
            let additional_cost : gas_cost = delegation_cost + create_cost;
            if gas_after < additional_cost then {
                gas_after = GAS_ZERO;
                (state_gas_after, state_spill_after, status_after) = exceptional_state(
                    state_gas_after,
                    state_spill_after,
                    carried_state_gas_reservoir,
                    OutOfGas,
                );
                return (
                    pc_after,
                    gas_after,
                    state_gas_after,
                    state_spill_after,
                    carried_refund,
                    status_after,
                    sp_after,
                    memory_after,
                    carried_caller,
                    carried_address,
                    carried_code_address,
                    carried_value,
                    carried_state_gas_reservoir,
                    carried_is_static,
                    carried_depth,
                    carried_code,
                    carried_calldata,
                    returndata_after,
                )
            };
            gas_after = gas_sub(gas_after, additional_cost);
            let stipend : gas =
                if value_nonzero then G_callstipend else GAS_ZERO;

            /* Forwarded child gas, per the EIP-150
               calculate_message_call_gas. The EIP-150 63/64ths cap applies to the gas
               left after the access, value-transfer, memory, and new-account costs. */
            var base_child : gas = GAS_ZERO;
            if profile.fork >= Amsterdam then {
                /* Amsterdam charges execution work, then state growth, before
                   applying EIP-150 to the execution gas left for the child. */
                if new_account_charged then {
                    let (state_gas_halt, next_gas, next_state_gas, next_state_spill) = charge_state_gas(
                        gas_after,
                        state_gas_after,
                        state_spill_after,
                        G_amsterdam_state_new_account,
                    );
                    gas_after = next_gas;
                    state_gas_after = next_state_gas;
                    state_spill_after = next_state_spill;
                    if state_gas_halt then {
                        gas_after = GAS_ZERO;
                        (state_gas_after, state_spill_after, status_after) = exceptional_state(
                            state_gas_after,
                            state_spill_after,
                            carried_state_gas_reservoir,
                            OutOfGas,
                        );
                        return (
                            pc_after,
                            gas_after,
                            state_gas_after,
                            state_spill_after,
                            carried_refund,
                            status_after,
                            sp_after,
                            memory_after,
                            carried_caller,
                            carried_address,
                            carried_code_address,
                            carried_value,
                            carried_state_gas_reservoir,
                            carried_is_static,
                            carried_depth,
                            carried_code,
                            carried_calldata,
                            returndata_after,
                        )
                    }
                };
                base_child = call_gas_cap_word(gas_after, gas_request);
                if gas_after < base_child then {
                    gas_after = GAS_ZERO;
                    (state_gas_after, state_spill_after, status_after) = exceptional_state(
                        state_gas_after,
                        state_spill_after,
                        carried_state_gas_reservoir,
                        OutOfGas,
                    );
                    return (
                        pc_after,
                        gas_after,
                        state_gas_after,
                        state_spill_after,
                        carried_refund,
                        status_after,
                        sp_after,
                        memory_after,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        returndata_after,
                    )
                };
                gas_after = gas_sub(gas_after, base_child)
            } else {
                base_child = call_gas_cap_word(gas_after, gas_request);
                if gas_after < base_child then {
                    gas_after = GAS_ZERO;
                    (state_gas_after, state_spill_after, status_after) = exceptional_state(
                        state_gas_after,
                        state_spill_after,
                        carried_state_gas_reservoir,
                        OutOfGas,
                    );
                    return (
                        pc_after,
                        gas_after,
                        state_gas_after,
                        state_spill_after,
                        carried_refund,
                        status_after,
                        sp_after,
                        memory_after,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        returndata_after,
                    )
                };
                gas_after = gas_sub(gas_after, base_child)
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
            let mem1 = expand_memory(memory_after, materialized_required_size);
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
                returndata_after = returndata_clear();
                gas_after = refund_gas(gas_after, child_gas);
                if new_account_charged then {
                    (gas_after, state_gas_after, state_spill_after) = credit_state_gas_refund(
                        gas_after,
                        state_gas_after,
                        state_spill_after,
                        G_amsterdam_state_new_account,
                    )
                };
                sp_after = stack_top_advance(sp_after, 1);
                write_stack_word(sp_after, WORD_ZERO);
                memory_after = mem1;
                (
                    pc_after,
                    gas_after,
                    state_gas_after,
                    state_spill_after,
                    carried_refund,
                    status_after,
                    sp_after,
                    memory_after,
                    carried_caller,
                    carried_address,
                    carried_code_address,
                    carried_value,
                    carried_state_gas_reservoir,
                    carried_is_static,
                    carried_depth,
                    carried_code,
                    carried_calldata,
                    returndata_after,
                )
            } else {
                let selected_precompile = precompile_id_for_address(target);
                if selected_precompile != NotPrecompile then {
                    /* PRECOMPILE call. The precompile set is fork-gated (the highest is
                       0x100 P256VERIFY, EIP-7951/Osaka); an address outside the active
                       set is an ordinary code call. The precompile's input is the
                       child message's calldata: memory_after[args_off .. args_off+args_len).
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
                            returndata_after = result.output;

                            /* the value transfer is part of the successful call */
                            if semantics.transfers_value & value_nonzero then {
                                k_transfer(caller, target, value)
                            };
                            returndata_copy_prefix(returndata_after, ret.off, ret.len);
                            let unused : gas = gas_sub(child_gas, used);
                            gas_after = refund_gas(gas_after, unused);
                            sp_after = stack_top_advance(sp_after, 1);
                            write_stack_word(sp_after, WORD_ONE);
                            memory_after = mem2;
                            (
                                pc_after,
                                gas_after,
                                state_gas_after,
                                state_spill_after,
                                carried_refund,
                                status_after,
                                sp_after,
                                memory_after,
                                carried_caller,
                                carried_address,
                                carried_code_address,
                                carried_value,
                                carried_state_gas_reservoir,
                                carried_is_static,
                                carried_depth,
                                carried_code,
                                carried_calldata,
                                returndata_after,
                            )
                        } else {
                            returndata_after = returndata_clear();
                            if new_account_charged then {
                                (gas_after, state_gas_after, state_spill_after) = credit_state_gas_refund(
                                    gas_after,
                                    state_gas_after,
                                    state_spill_after,
                                    G_amsterdam_state_new_account,
                                )
                            };
                            sp_after = stack_top_advance(sp_after, 1);
                            write_stack_word(sp_after, WORD_ZERO);
                            memory_after = mem2;
                            (
                                pc_after,
                                gas_after,
                                state_gas_after,
                                state_spill_after,
                                carried_refund,
                                status_after,
                                sp_after,
                                memory_after,
                                carried_caller,
                                carried_address,
                                carried_code_address,
                                carried_value,
                                carried_state_gas_reservoir,
                                carried_is_static,
                                carried_depth,
                                carried_code,
                                carried_calldata,
                                returndata_after,
                            )
                        }
                    } else {
                        returndata_after = returndata_clear();
                        if new_account_charged then {
                            (gas_after, state_gas_after, state_spill_after) = credit_state_gas_refund(
                                gas_after,
                                state_gas_after,
                                state_spill_after,
                                G_amsterdam_state_new_account,
                            )
                        };
                        sp_after = stack_top_advance(sp_after, 1);
                        write_stack_word(sp_after, WORD_ZERO);
                        memory_after = mem2;
                        (
                            pc_after,
                            gas_after,
                            state_gas_after,
                            state_spill_after,
                            carried_refund,
                            status_after,
                            sp_after,
                            memory_after,
                            carried_caller,
                            carried_address,
                            carried_code_address,
                            carried_value,
                            carried_state_gas_reservoir,
                            carried_is_static,
                            carried_depth,
                            carried_code,
                            carried_calldata,
                            returndata_after,
                        )
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
                        if semantics.inherits_caller_and_value then carried_caller else caller;
                    let child_value : word =
                        if semantics.inherits_caller_and_value then carried_value else value;
                    let child_static : bool =
                        if semantics.enters_static_context then true else carried_is_static;
                    let (bytes, mem2) = active_memory_slice(mem1, args.off, args.len);
                    let child_memory = evm_memory_slice(bytes.bytes, bytes.len);
                    let child_calldata = MemoryCalldata(child_memory);
                    let child_state_gas = state_gas_after;
                    let running = Running();
                    let (checkpoint, child_stack, child_frame_memory) = suspend_frame(
                        pc_after,
                        gas_after,
                        sp_after,
                        mem2,
                        STATE_GAS_ZERO,
                        state_spill_after,
                        carried_refund,
                        running,
                        parent_message,
                        carried_code,
                        carried_calldata,
                    );
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
                    let child_returndata = returndata_clear();
                    (
                        0,
                        child_gas,
                        child_state_gas,
                        STATE_GAS_SPILL_ZERO,
                        GAS_REFUND_ZERO,
                        running,
                        child_stack,
                        child_frame_memory,
                        child_caller,
                        child_addr,
                        target,
                        child_value,
                        child_state_gas,
                        child_static,
                        child_depth,
                        child_code,
                        child_calldata,
                        child_returndata,
                    )
                }
            }
        },
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
function create_stack_inputs(kind : CreateKind) -> operand_stack_height =
    match kind {
        CreateByNonce => 3,
        CreateBySalt => 4,
    }

/*! Executes a contract-creation instruction through its non-entering failure
    paths or installs the initcode child frame and returns its initial state. */
function run_create(
    carried_pc : code_pointer,
    carried_gas : gas,
    carried_state_gas : state_gas,
    carried_state_spill : state_gas_spill,
    carried_refund : gas_refund,
    carried_sp : StackPointer,
    carried_memory : EvmMemorySlice,
    carried_caller : address,
    carried_address : address,
    carried_code_address : address,
    carried_value : word,
    carried_state_gas_reservoir : state_gas,
    carried_is_static : bool,
    carried_depth : frame_depth,
    carried_code : Code,
    carried_calldata : CalldataSlice,
    carried_returndata : OutputSlice,
    kind : CreateKind,
) -> (
    (
        code_pointer,
        gas,
        state_gas,
        state_gas_spill,
        gas_refund,
        FrameStatus,
        StackPointer,
        EvmMemorySlice,
        address,
        address,
        address,
        word,
        state_gas,
        bool,
        frame_depth,
        Code,
        CalldataSlice,
        OutputSlice,
    )
) = {
    let stack_inputs = create_stack_inputs(kind);
    let stack_status = guard_stack(carried_sp, stack_inputs, 1);
    match stack_status {
        Failed(halt_kind) => {
            let (state_gas_after, state_spill_after, status_after) = exceptional_state(
                carried_state_gas,
                carried_state_spill,
                carried_state_gas_reservoir,
                halt_kind,
            );
            (
                carried_pc,
                GAS_ZERO,
                state_gas_after,
                state_spill_after,
                carried_refund,
                status_after,
                carried_sp,
                carried_memory,
                carried_caller,
                carried_address,
                carried_code_address,
                carried_value,
                carried_state_gas_reservoir,
                carried_is_static,
                carried_depth,
                carried_code,
                carried_calldata,
                carried_returndata,
            )
        },
        Continue() => {
            let pc_after : code_pointer = carried_pc;
            var gas_after : gas = carried_gas;
            var state_gas_after : state_gas = carried_state_gas;
            var state_spill_after : state_gas_spill = carried_state_spill;
            var status_after : FrameStatus = Running();
            var sp_after : StackPointer = carried_sp;
            var memory_after : EvmMemorySlice = carried_memory;
            var returndata_after : OutputSlice = carried_returndata;
            let parent_message : Message = struct {
                    caller = carried_caller,
                    address = carried_address,
                    code_address = carried_code_address,
                    value = carried_value,
                    state_gas_reservoir = carried_state_gas_reservoir,
                    is_static = carried_is_static,
                    depth = carried_depth,
                };
            let semantics = create_semantics(kind);
            let execution_profile = k_execution_profile;
            let profile = execution_profile.protocol;
            let current_depth = carried_depth;
            let creator = carried_address;
            let value = read_stack_word(sp_after);
            sp_after = stack_top_retreat(sp_after, 1);
            let off_word = read_stack_word(sp_after);
            sp_after = stack_top_retreat(sp_after, 1);
            let len_word = read_stack_word(sp_after);
            sp_after = stack_top_retreat(sp_after, 1);
            let (salt, next_sp) : (word, StackPointer) =
                if semantics.uses_salt then {
                    let salt = read_stack_word(sp_after);
                    (salt, stack_top_retreat(sp_after, 1))
                } else {
                    (WORD_ZERO, sp_after)
                };
            sp_after = next_sp;

            /* EIP-214: CREATE/CREATE2 modifies state and is forbidden in a static
               context -- it raises WriteProtection (an exceptional halt consuming all
               remaining gas), checked before any charge or child frame. */
            if carried_is_static then {
                gas_after = GAS_ZERO;
                (state_gas_after, state_spill_after, status_after) = exceptional_state(
                    state_gas_after,
                    state_spill_after,
                    carried_state_gas_reservoir,
                    WriteProtection,
                );
                return (
                    pc_after,
                    gas_after,
                    state_gas_after,
                    state_spill_after,
                    carried_refund,
                    status_after,
                    sp_after,
                    memory_after,
                    carried_caller,
                    carried_address,
                    carried_code_address,
                    carried_value,
                    carried_state_gas_reservoir,
                    carried_is_static,
                    carried_depth,
                    carried_code,
                    carried_calldata,
                    returndata_after,
                )
            };
            let required_size = memory_required_size(off_word, len_word);
            let expansion_cost = memory_expansion_gas_cost(memory_after, required_size, gas_after);
            if not_bool(expansion_cost.affordable) then {
                gas_after = GAS_ZERO;
                (state_gas_after, state_spill_after, status_after) = exceptional_state(
                    state_gas_after,
                    state_spill_after,
                    carried_state_gas_reservoir,
                    OutOfGas,
                );
                return (
                    pc_after,
                    gas_after,
                    state_gas_after,
                    state_spill_after,
                    carried_refund,
                    status_after,
                    sp_after,
                    memory_after,
                    carried_caller,
                    carried_address,
                    carried_code_address,
                    carried_value,
                    carried_state_gas_reservoir,
                    carried_is_static,
                    carried_depth,
                    carried_code,
                    carried_calldata,
                    returndata_after,
                )
            };
            gas_after = gas_sub(gas_after, expansion_cost.cost);
            let initcode_access = memory_access(off_word, len_word);
            let mem1 = expand_memory(memory_after, initcode_access.required_size);
            let initcode = initcode_access.range;

            /* Up-front charges: memory expansion over the initcode, the CREATE base
               cost (G_create), the EIP-3860 per-initcode-word cost, and -- for
               CREATE2 only -- the keccak word cost of hashing the initcode for the
               address. */
            let access_cost = create_access_cost();
            if gas_after < access_cost then {
                memory_after = mem1;
                gas_after = GAS_ZERO;
                (state_gas_after, state_spill_after, status_after) = exceptional_state(
                    state_gas_after,
                    state_spill_after,
                    carried_state_gas_reservoir,
                    OutOfGas,
                );
                return (
                    pc_after,
                    gas_after,
                    state_gas_after,
                    state_spill_after,
                    carried_refund,
                    status_after,
                    sp_after,
                    memory_after,
                    carried_caller,
                    carried_address,
                    carried_code_address,
                    carried_value,
                    carried_state_gas_reservoir,
                    carried_is_static,
                    carried_depth,
                    carried_code,
                    carried_calldata,
                    returndata_after,
                )
            };
            gas_after = gas_sub(gas_after, access_cost);
            let initcode_word_count = memory_word_count_word(len_word);
            if profile.fork >= Shanghai then {
                let initcode_cost = word_scaled_gas_cost(G_initcode_word, initcode_word_count, gas_after);
                if not_bool(initcode_cost.affordable) then {
                    memory_after = mem1;
                    gas_after = GAS_ZERO;
                    (state_gas_after, state_spill_after, status_after) = exceptional_state(
                        state_gas_after,
                        state_spill_after,
                        carried_state_gas_reservoir,
                        OutOfGas,
                    );
                    return (
                        pc_after,
                        gas_after,
                        state_gas_after,
                        state_spill_after,
                        carried_refund,
                        status_after,
                        sp_after,
                        memory_after,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        returndata_after,
                    )
                };
                gas_after = gas_sub(gas_after, initcode_cost.cost)
            };
            if semantics.uses_salt then {
                let hashing_cost = word_scaled_gas_cost(G_keccak_word, initcode_word_count, gas_after);
                if not_bool(hashing_cost.affordable) then {
                    memory_after = mem1;
                    gas_after = GAS_ZERO;
                    (state_gas_after, state_spill_after, status_after) = exceptional_state(
                        state_gas_after,
                        state_spill_after,
                        carried_state_gas_reservoir,
                        OutOfGas,
                    );
                    return (
                        pc_after,
                        gas_after,
                        state_gas_after,
                        state_spill_after,
                        carried_refund,
                        status_after,
                        sp_after,
                        memory_after,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        returndata_after,
                    )
                };
                gas_after = gas_sub(gas_after, hashing_cost.cost)
            };

            /* EIP-3860: initcode longer than MAX_INITCODE_SIZE is rejected outright as
               an exceptional halt. */
            let valid_initcode_size = initcode_size_allowed(initcode.len);
            let invalid_initcode_size = not_bool(valid_initcode_size);
            if invalid_initcode_size then {
                memory_after = mem1;
                gas_after = GAS_ZERO;
                (state_gas_after, state_spill_after, status_after) = exceptional_state(
                    state_gas_after,
                    state_spill_after,
                    carried_state_gas_reservoir,
                    InitCodeTooLarge,
                );
                (
                    pc_after,
                    gas_after,
                    state_gas_after,
                    state_spill_after,
                    carried_refund,
                    status_after,
                    sp_after,
                    memory_after,
                    carried_caller,
                    carried_address,
                    carried_code_address,
                    carried_value,
                    carried_state_gas_reservoir,
                    carried_is_static,
                    carried_depth,
                    carried_code,
                    carried_calldata,
                    returndata_after,
                )
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
                var child_gas : gas = GAS_ZERO;
                if profile.fork < Amsterdam then {
                    let avail = gas_after;
                    let retained_gas : gas = avail / 64;
                    child_gas = gas_sub(avail, retained_gas);
                    gas_after = retained_gas
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
                    returndata_after = returndata_clear();
                    gas_after =
                        if profile.fork < Amsterdam then {
                            refund_gas(gas_after, child_gas)
                        } else {
                            gas_after
                        };
                    sp_after = stack_top_advance(sp_after, 1);
                    write_stack_word(sp_after, WORD_ZERO);
                    memory_after = mem2;
                    (
                        pc_after,
                        gas_after,
                        state_gas_after,
                        state_spill_after,
                        carried_refund,
                        status_after,
                        sp_after,
                        memory_after,
                        carried_caller,
                        carried_address,
                        carried_code_address,
                        carried_value,
                        carried_state_gas_reservoir,
                        carried_is_static,
                        carried_depth,
                        carried_code,
                        carried_calldata,
                        returndata_after,
                    )
                } else {
                    /* warm the new address and bump the creator nonce BEFORE the child
                     * snapshot: both persist even if the initcode reverts (the
                     * the protocol runs them in the parent, before
                     * process_create_message). */
                    let child_depth : frame_depth = current_depth + 1;
                    k_account_mark_warm(new_addr);
                    let new_account_charged = (profile.fork >= Amsterdam) & k_account_is_empty(new_addr);
                    if new_account_charged then {
                        let (state_gas_halt, next_gas, next_state_gas, next_state_spill) = charge_state_gas(
                            gas_after,
                            state_gas_after,
                            state_spill_after,
                            G_amsterdam_state_new_account,
                        );
                        gas_after = next_gas;
                        state_gas_after = next_state_gas;
                        state_spill_after = next_state_spill;
                        if state_gas_halt then {
                            memory_after = mem2;
                            gas_after = GAS_ZERO;
                            (state_gas_after, state_spill_after, status_after) = exceptional_state(
                                state_gas_after,
                                state_spill_after,
                                carried_state_gas_reservoir,
                                OutOfGas,
                            );
                            return (
                                pc_after,
                                gas_after,
                                state_gas_after,
                                state_spill_after,
                                carried_refund,
                                status_after,
                                sp_after,
                                memory_after,
                                carried_caller,
                                carried_address,
                                carried_code_address,
                                carried_value,
                                carried_state_gas_reservoir,
                                carried_is_static,
                                carried_depth,
                                carried_code,
                                carried_calldata,
                                returndata_after,
                            )
                        }
                    };
                    if profile.fork >= Amsterdam then {
                        let avail = gas_after;
                        let retained_gas : gas = avail / 64;
                        child_gas = gas_sub(avail, retained_gas);
                        gas_after = retained_gas
                    };
                    let occupied : bool = k_account_occupied(new_addr);
                    returndata_after = returndata_clear();
                    k_bump_nonce(creator);
                    if occupied then {
                        /* address collision (the target already has code, a nonzero
                         * nonce, or storage): no initcode runs, 0 is pushed, and the
                         * reserved child gas is NOT refunded (it stays deducted). */
                        if new_account_charged then {
                            (gas_after, state_gas_after, state_spill_after) = credit_state_gas_refund(
                                gas_after,
                                state_gas_after,
                                state_spill_after,
                                G_amsterdam_state_new_account,
                            )
                        };
                        sp_after = stack_top_advance(sp_after, 1);
                        write_stack_word(sp_after, WORD_ZERO);
                        memory_after = mem2;
                        (
                            pc_after,
                            gas_after,
                            state_gas_after,
                            state_spill_after,
                            carried_refund,
                            status_after,
                            sp_after,
                            memory_after,
                            carried_caller,
                            carried_address,
                            carried_code_address,
                            carried_value,
                            carried_state_gas_reservoir,
                            carried_is_static,
                            carried_depth,
                            carried_code,
                            carried_calldata,
                            returndata_after,
                        )
                    } else {
                        /* Freeze the initcode while parent memory is active, then
                           suspend the complete parent before mutating the new account.
                           The child frame has no calldata. */
                        let (initcode_bytes, mem3) = memory_code_slice(mem2, initcode.off, initcode.len);
                        let child_code_id = code_db_insert(initcode_bytes, profile.fork);
                        let child_code = code_db_resolve(child_code_id);
                        let child_state_gas = state_gas_after;
                        let running = Running();
                        let (checkpoint, child_stack, child_frame_memory) = suspend_frame(
                            pc_after,
                            gas_after,
                            sp_after,
                            mem3,
                            STATE_GAS_ZERO,
                            state_spill_after,
                            carried_refund,
                            running,
                            parent_message,
                            carried_code,
                            carried_calldata,
                        );
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
                        let child_returndata = returndata_clear();
                        (
                            0,
                            child_gas,
                            child_state_gas,
                            STATE_GAS_SPILL_ZERO,
                            GAS_REFUND_ZERO,
                            running,
                            child_stack,
                            child_frame_memory,
                            creator,
                            new_addr,
                            new_addr,
                            value,
                            child_state_gas,
                            carried_is_static,
                            child_depth,
                            child_code,
                            EMPTY_CALLDATA,
                            child_returndata,
                        )
                    }
                }
            }
        },
    }
}

/*! Restores a message-call parent and applies the child's outcome. */
function resume_call(
    continuation : CallContinuation,
    output : OutputSlice,
    child_gas : gas,
    child_state_gas : state_gas,
    child_state_spill : state_gas_spill,
    child_refund : gas_refund,
    child_status : FrameStatus,
) -> (
    (
        code_pointer,
        gas,
        state_gas,
        state_gas_spill,
        gas_refund,
        FrameStatus,
        StackPointer,
        EvmMemorySlice,
        address,
        address,
        address,
        word,
        state_gas,
        bool,
        frame_depth,
        Code,
        CalldataSlice,
        OutputSlice,
    )
) = {
    let checkpoint = continuation.checkpoint;
    let succeeded = frame_succeeded(child_status);
    let (
        parent_pc,
        restored_gas,
        restored_sp,
        parent_memory,
        restored_state_gas,
        restored_state_spill,
        restored_refund,
        restored_status,
        parent_message,
        parent_code,
        parent_calldata,
    ) = restore_frame(checkpoint);
    var parent_gas = refund_gas(restored_gas, child_gas);
    var parent_state_gas : state_gas = restored_state_gas;
    var parent_state_spill : state_gas_spill = restored_state_spill;
    (parent_state_gas, parent_state_spill) = return_child_state_gas(
        parent_state_gas,
        parent_state_spill,
        child_state_gas,
        child_state_spill,
    );
    var parent_refund : gas_refund = restored_refund;
    var parent_sp : StackPointer = restored_sp;

    /* Both RETURN and REVERT copy their output; exceptional halts carry the
     * empty slice. Successful effects remain, while every failure reverts to
     * the saved kernel checkpoint. */
    returndata_copy_prefix(output, continuation.return_offset, continuation.return_length);
    if succeeded then {
        parent_refund = record_refund(parent_refund, child_refund);
        k_journal_commit();
        parent_sp = stack_top_advance(parent_sp, 1);
        write_stack_word(parent_sp, WORD_ONE)
    } else {
        k_journal_revert();
        if continuation.new_account_charged then {
            (parent_gas, parent_state_gas, parent_state_spill) = credit_state_gas_refund(
                parent_gas,
                parent_state_gas,
                parent_state_spill,
                G_amsterdam_state_new_account,
            )
        };
        parent_sp = stack_top_advance(parent_sp, 1);
        write_stack_word(parent_sp, WORD_ZERO)
    };
    (
        parent_pc,
        parent_gas,
        parent_state_gas,
        parent_state_spill,
        parent_refund,
        restored_status,
        parent_sp,
        parent_memory,
        parent_message.caller,
        parent_message.address,
        parent_message.code_address,
        parent_message.value,
        parent_message.state_gas_reservoir,
        parent_message.is_static,
        parent_message.depth,
        parent_code,
        parent_calldata,
        output,
    )
}

/*! Restores a create parent and either deploys or rolls back the child. */
function resume_create(
    continuation : CreateContinuation,
    output : OutputSlice,
    child_gas : gas,
    child_state_gas : state_gas,
    child_state_spill : state_gas_spill,
    child_refund : gas_refund,
    child_status : FrameStatus,
    child_state_gas_reservoir : state_gas,
) -> (
    (
        code_pointer,
        gas,
        state_gas,
        state_gas_spill,
        gas_refund,
        FrameStatus,
        StackPointer,
        EvmMemorySlice,
        address,
        address,
        address,
        word,
        state_gas,
        bool,
        frame_depth,
        Code,
        CalldataSlice,
        OutputSlice,
    )
) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let checkpoint = continuation.checkpoint;
    let initcode_succeeded = frame_succeeded(child_status);
    let deployed_length = returndata_size(output);
    let deployed_size = deployed_length;
    var frontier_empty_deposit : bool = false;
    var settled_child_gas : gas = child_gas;
    var settled_child_state_gas : state_gas = child_state_gas;
    var settled_child_state_spill : state_gas_spill = child_state_spill;
    var settled_child_status : FrameStatus = child_status;

    if initcode_succeeded then {
        let deployed_size_allowed = deployed_code_size_allowed(deployed_size);
        let invalid_deployed_size = not_bool(deployed_size_allowed);
        let prohibited_prefix =
            if deployed_size != 0 then {
                let first_byte = slice_byte(output, 0);
                first_byte == 0xef
            } else {
                false
            };
        if invalid_deployed_size | ((profile.fork >= London) & prohibited_prefix) then {
            /* EIP-170 and EIP-3541 failures use the post-Homestead
             * exceptional creation path. Neither rule is active on
             * Frontier/Homestead in a way that reaches this branch. */
            settled_child_gas = GAS_ZERO;
            (settled_child_state_gas, settled_child_state_spill, settled_child_status) = exceptional_state(
                settled_child_state_gas,
                settled_child_state_spill,
                child_state_gas_reservoir,
                OutOfGas,
            )
        } else {
            let deployment_charge = code_deployment_execution_cost(deployed_length, settled_child_gas);
            if deployment_charge.affordable then {
                let execution_deposit = deployment_charge.cost;
                settled_child_gas = gas_sub(settled_child_gas, execution_deposit);
                let state_deposit = code_deployment_state_cost(deployed_length);
                let (deployment_halt, deployment_gas, deployment_state_gas, deployment_state_spill) = charge_deployment_state_gas(
                    settled_child_gas,
                    settled_child_state_gas,
                    settled_child_state_spill,
                    state_deposit,
                );
                settled_child_gas = deployment_gas;
                settled_child_state_gas = deployment_state_gas;
                settled_child_state_spill = deployment_state_spill;
                if deployment_halt then {
                    settled_child_gas = GAS_ZERO;
                    (settled_child_state_gas, settled_child_state_spill, settled_child_status) = exceptional_state(
                        settled_child_state_gas,
                        settled_child_state_spill,
                        child_state_gas_reservoir,
                        OutOfGas,
                    )
                }
            } else if profile.fork < Homestead then {
                /* Frontier consumed the remaining child gas but kept the
                 * creation successful with empty deployed code. EIP-2
                 * changed this to exceptional failure at Homestead. */
                settled_child_gas = GAS_ZERO;
                frontier_empty_deposit = true
            } else {
                settled_child_gas = GAS_ZERO;
                (settled_child_state_gas, settled_child_state_spill, settled_child_status) = exceptional_state(
                    settled_child_state_gas,
                    settled_child_state_spill,
                    child_state_gas_reservoir,
                    OutOfGas,
                )
            }
        }
    };
    let deploy_succeeds = initcode_succeeded & frame_succeeded(settled_child_status);
    let (
        parent_pc,
        restored_gas,
        restored_sp,
        parent_memory,
        restored_state_gas,
        restored_state_spill,
        restored_refund,
        restored_status,
        parent_message,
        parent_code,
        parent_calldata,
    ) = restore_frame(checkpoint);
    var parent_gas = refund_gas(restored_gas, settled_child_gas);
    var parent_state_gas : state_gas = restored_state_gas;
    var parent_state_spill : state_gas_spill = restored_state_spill;
    (parent_state_gas, parent_state_spill) = return_child_state_gas(
        parent_state_gas,
        parent_state_spill,
        settled_child_state_gas,
        settled_child_state_spill,
    );
    var parent_refund : gas_refund = restored_refund;
    var parent_sp : StackPointer = restored_sp;

    if deploy_succeeds then {
        parent_refund = record_refund(parent_refund, child_refund);
        let deployed_bytes : OutputSlice =
            if frontier_empty_deposit then EMPTY_OUTPUT_SLICE else output;
        let deployed_code = code_db_intern_output(deployed_bytes);
        k_deploy_code(continuation.address, deployed_code);
        k_journal_commit();
        let deployed_address = address_to_word(continuation.address);
        parent_sp = stack_top_advance(parent_sp, 1);
        write_stack_word(parent_sp, deployed_address)
    } else {
        k_journal_revert();
        if continuation.new_account_charged then {
            (parent_gas, parent_state_gas, parent_state_spill) = credit_state_gas_refund(
                parent_gas,
                parent_state_gas,
                parent_state_spill,
                G_amsterdam_state_new_account,
            )
        };
        parent_sp = stack_top_advance(parent_sp, 1);
        write_stack_word(parent_sp, WORD_ZERO)
    };

    /* Successful initcode output has become deployed code. REVERT output
     * remains visible to RETURNDATA opcodes in the parent. */
    let parent_returndata =
        if initcode_succeeded then returndata_clear() else output;
    (
        parent_pc,
        parent_gas,
        parent_state_gas,
        parent_state_spill,
        parent_refund,
        restored_status,
        parent_sp,
        parent_memory,
        parent_message.caller,
        parent_message.address,
        parent_message.code_address,
        parent_message.value,
        parent_message.state_gas_reservoir,
        parent_message.is_static,
        parent_message.depth,
        parent_code,
        parent_calldata,
        parent_returndata,
    )
}

/*! Applies the pending operation for one completed child frame. */
function resume_frame(
    continuation : FrameContinuation,
    output : OutputSlice,
    child_gas : gas,
    child_state_gas : state_gas,
    child_state_spill : state_gas_spill,
    child_refund : gas_refund,
    child_status : FrameStatus,
    child_state_gas_reservoir : state_gas,
) -> (
    (
        code_pointer,
        gas,
        state_gas,
        state_gas_spill,
        gas_refund,
        FrameStatus,
        StackPointer,
        EvmMemorySlice,
        address,
        address,
        address,
        word,
        state_gas,
        bool,
        frame_depth,
        Code,
        CalldataSlice,
        OutputSlice,
    )
) =
    match continuation {
        Empty() => fatal_error(ExecutionInvalid),
        ResumeCall(call) => resume_call(
            call,
            output,
            child_gas,
            child_state_gas,
            child_state_spill,
            child_refund,
            child_status,
        ),
        ResumeCreate(create) => resume_create(
            create,
            output,
            child_gas,
            child_state_gas,
            child_state_spill,
            child_refund,
            child_status,
            child_state_gas_reservoir,
        ),
    }

/*! Selects the canonical frame-entering operation from its encoded opcode.
    The optimized threaded dispatcher therefore owns only label routing; CALL
    and CREATE classification remains part of the executable specification. */
function run_frame_entry_encoded(
    carried_pc : code_pointer,
    carried_gas : gas,
    carried_state_gas : state_gas,
    carried_state_spill : state_gas_spill,
    carried_refund : gas_refund,
    carried_sp : StackPointer,
    carried_memory : EvmMemorySlice,
    carried_caller : address,
    carried_address : address,
    carried_code_address : address,
    carried_value : word,
    carried_state_gas_reservoir : state_gas,
    carried_is_static : bool,
    carried_depth : frame_depth,
    carried_code : Code,
    carried_calldata : CalldataSlice,
    carried_returndata : OutputSlice,
    opcode : opcode,
) -> (
    (
        code_pointer,
        gas,
        state_gas,
        state_gas_spill,
        gas_refund,
        FrameStatus,
        StackPointer,
        EvmMemorySlice,
        address,
        address,
        address,
        word,
        state_gas,
        bool,
        frame_depth,
        Code,
        CalldataSlice,
        OutputSlice,
    )
) = {
    match opcode {
        240 => run_create(
            carried_pc,
            carried_gas,
            carried_state_gas,
            carried_state_spill,
            carried_refund,
            carried_sp,
            carried_memory,
            carried_caller,
            carried_address,
            carried_code_address,
            carried_value,
            carried_state_gas_reservoir,
            carried_is_static,
            carried_depth,
            carried_code,
            carried_calldata,
            carried_returndata,
            CreateByNonce,
        ),
        241 => run_call(
            carried_pc,
            carried_gas,
            carried_state_gas,
            carried_state_spill,
            carried_refund,
            carried_sp,
            carried_memory,
            carried_caller,
            carried_address,
            carried_code_address,
            carried_value,
            carried_state_gas_reservoir,
            carried_is_static,
            carried_depth,
            carried_code,
            carried_calldata,
            carried_returndata,
            Call,
        ),
        242 => run_call(
            carried_pc,
            carried_gas,
            carried_state_gas,
            carried_state_spill,
            carried_refund,
            carried_sp,
            carried_memory,
            carried_caller,
            carried_address,
            carried_code_address,
            carried_value,
            carried_state_gas_reservoir,
            carried_is_static,
            carried_depth,
            carried_code,
            carried_calldata,
            carried_returndata,
            CallCode,
        ),
        244 => run_call(
            carried_pc,
            carried_gas,
            carried_state_gas,
            carried_state_spill,
            carried_refund,
            carried_sp,
            carried_memory,
            carried_caller,
            carried_address,
            carried_code_address,
            carried_value,
            carried_state_gas_reservoir,
            carried_is_static,
            carried_depth,
            carried_code,
            carried_calldata,
            carried_returndata,
            DelegateCall,
        ),
        245 => run_create(
            carried_pc,
            carried_gas,
            carried_state_gas,
            carried_state_spill,
            carried_refund,
            carried_sp,
            carried_memory,
            carried_caller,
            carried_address,
            carried_code_address,
            carried_value,
            carried_state_gas_reservoir,
            carried_is_static,
            carried_depth,
            carried_code,
            carried_calldata,
            carried_returndata,
            CreateBySalt,
        ),
        250 => run_call(
            carried_pc,
            carried_gas,
            carried_state_gas,
            carried_state_spill,
            carried_refund,
            carried_sp,
            carried_memory,
            carried_caller,
            carried_address,
            carried_code_address,
            carried_value,
            carried_state_gas_reservoir,
            carried_is_static,
            carried_depth,
            carried_code,
            carried_calldata,
            carried_returndata,
            StaticCall,
        ),
        _ => {
            let (state_gas_after, state_spill_after, status_after) = exceptional_state(
                carried_state_gas,
                carried_state_spill,
                carried_state_gas_reservoir,
                InvalidOpcode,
            );
            (
                carried_pc,
                GAS_ZERO,
                state_gas_after,
                state_spill_after,
                carried_refund,
                status_after,
                carried_sp,
                carried_memory,
                carried_caller,
                carried_address,
                carried_code_address,
                carried_value,
                carried_state_gas_reservoir,
                carried_is_static,
                carried_depth,
                carried_code,
                carried_calldata,
                carried_returndata,
            )
        },
    }
}
