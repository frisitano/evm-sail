
/*md
# Machine state

The user-space half of the machine: per-frame registers, operand stack,
gas counter, and the saved-frame stack for nested calls. It holds the gas
counter and all per-frame compute state (`pc`, stack, memory, code,
message, returndata). World state lives in the host kernel and is reached
only via `k_*` calls; world rollback on revert is a kernel snapshot, not
saved here.

## The frame registers

One [Message][type-Message] per active frame (YP "message call"
`I = (I_a, I_o, I_s, …)`); sub-calls save and restore these registers in
the interpreter.
*/
/*! The program counter (YP μ_pc). */
register pc : code_pointer = BYTE_ZERO
/*! The frame's remaining gas (YP μ_g) — the one gas counter. */
register gas_remaining : gas = GAS_ZERO /* THE gas counter (user-space) */
/*! The frame-local signed gas refund accumulated by state changes. */
register frame_refund : gas_refund = GAS_REFUND_ZERO
/*! The frame's running/halted/exceptional status. */
register frame_status : FrameStatus = Running()
/*! The frame's call message. */
register message : Message = DEFAULT_MESSAGE
/*! The call depth (YP I_e), bounded by `DEPTH_LIMIT`. */
register call_depth : frame_depth = 0

/* EIP-150 / YP: a message call may nest at most 1024 frames deep. */
/*! A message call may nest at most 1024 frames deep (EIP-150 / YP). */
let DEPTH_LIMIT : frame_depth = 1024

function record_refund(delta : gas_refund) -> unit =
    frame_refund = frame_refund + delta

/*! The frame's executable code as one indivisible value: its byte slice
    plus the resolved `JUMPDEST` table produced when that byte sequence
    was indexed. Frame creators supply this value; the interpreter only
    reads it. Keeping the pair in one register makes mismatched
    bytes/analysis unrepresentable during frame save and restore. */
register frame_code : Code = EMPTY_CODE

/*! The frame code length in bytes (`CODESIZE`). */
function frame_code_len() -> code_length = frame_code.bytes.len

/*! Whether `dest` is a valid `JUMPDEST` in the frame's code
    (YP §9.4.3). */
function frame_jumpdest_valid(dest : code_pointer) -> bool =
    jumpdest_ref_contains(frame_code.jumpdests, frame_code.bytes.len, dest)

/*md
## The operand stack
*/
/*! The 1024-element operand-stack limit (YP §9.1). */
let STACK_LIMIT : operand_stack_height = 1024

/*! Enters the exceptional halting state (YP §9.4.2), consuming all
    remaining gas — unlike `REVERT`, which refunds it. Every exceptional
    halt routes through here so the gas is always zeroed. */
function exc_halt(k : ExceptionKind) -> unit = {
    gas_remaining = GAS_ZERO;
    frame_status = Exceptional(k)
}

/*! The stack height as a Sail integer. */
function stack_height() -> operand_stack_height = {
    let height = stack_depth();
    assert(height <= STACK_LIMIT);
    height
}

/*! The `n`-th-from-top operand (`n = 0` is the top); zero out of
    range. */
function peek(n : stack_index) -> word = stack_peek_word(n)

/*! Pushes a word, entering `StackOverflow` at the 1024 limit. */
function push(w : word) -> unit =
    if STACK_LIMIT <= stack_height() then {
        exc_halt(StackOverflow)
    } else {
        stack_push_word(w)
    }

/*! Pops the top word, entering `StackUnderflow` (and yielding zero) on
    an empty stack. */
function pop() -> word =
    if stack_height() == 0 then {
        exc_halt(StackUnderflow);
        WORD_ZERO
    } else {
        stack_pop_word()
    }

/*! Overwrites the `n`-th-from-top operand (`SWAP`). */
function stack_set(n : stack_index, w : word) -> unit =
    stack_set_word(n, w)

/*! Whether the frame is still running. */
function is_running() -> bool = match frame_status {
    Running() => true,
    _ => false,
}

/*! The frame's calldata (YP I_d), as a reference: the transaction input
    slice at the top level, a minted slice of the suspended caller's
    memory for sub-calls, the empty slice for initcode frames.
    Saved/restored across sub-calls like the other frame registers. */
register calldata : ByteSlice = EMPTY_SLICE

/*! Installs the frame's calldata reference. */
function calldata_install(data : ByteSlice) -> unit = calldata = data

/*! The output of the frame's latest child call (EIP-211's returndata
    buffer). The active frame's own output is carried by its
    `RETURN`/`REVERT` halt value and returned from [interpret][], rather
    than stored in another register. */
register returndata : ByteSlice = EMPTY_SLICE

/*! Clears the returndata buffer (a new sub-call begins). */
function returndata_clear() -> unit = returndata = EMPTY_SLICE

/*! `RETURNDATASIZE`. */
function returndata_size() -> byte_length = returndata.len

/*! One returndata byte. */
function returndata_byte(index : source_pointer) -> byte =
    slice_byte(returndata, index)

/*! `RETURNDATACOPY` into frame memory (bounds pre-checked by the
    opcode). */
function returndata_copy(dst : memory_pointer, off : source_pointer, len : memory_length) -> unit =
    slice_copy(returndata, dst, off, len)

/*! Copies `min(want, size)` returndata bytes — the `CALL`-family output
    write-back. */
function returndata_copy_prefix(dst : memory_pointer, want : memory_length) -> unit = {
    let have = returndata_size();
    slice_copy(returndata, dst, BYTE_ZERO, if want < have then { want } else { have })
}

/* EIP-211 validates the source range even when its length is zero. This
 * operation is deliberately separate from memory-range charging: source
 * failure is an out-of-bounds exceptional halt, not memory OOG. */
/*md
## The frame memory

The EVM's view over frame memory, a region of the host interface. Every
access is guarded by
[is_running][] — a failed expansion charge sets the halt status without
aborting the opcode handler, and a post-out-of-gas write to a huge offset
(e.g. `MSTORE` at 2^32) must not grow the backing buffer — and writes
raise the
high-water mark that `MSIZE` and memory-expansion gas read.
*/
/*! Validates word-sized returndata bounds and copies the requested range. */
function returndata_copy_words(dst : memory_pointer, source_offset : word, length : word) -> unit = {
    let available = word_of_quantity(returndata_size());
    if word_ule(source_offset, available) & word_ule(length, sub_bits(available, source_offset)) then {
        returndata_copy(dst, ByteQuantity(unsigned(source_offset)), ByteQuantity(unsigned(length)))
    } else {
        exc_halt(InvalidOpcode)
    }
}

/* ----------------------- frame memory (EVM view) ------------------------ */
/* Each frame owns one absolute slice of the shared host arena. Relative EVM
 * offsets are represented by subslicing this value, so calldata captured
 * from a suspended parent and an immediate memory read have the same source
 * representation. The slice length is the word-aligned MSIZE watermark. */
/*! The active frame's word-aligned memory range in the shared memory arena. */
register evm_memory : ByteSlice = byte_slice(EvmMemorySource, BYTE_ZERO, BYTE_ZERO)

function evm_memory_size() -> memory_length = evm_memory.len

/*! A slice into frame memory. */
function memory_byte_slice(off : memory_pointer, len : memory_length) -> ByteSlice =
    if len == BYTE_ZERO then {
        EMPTY_SLICE
    } else {
        sub_slice(evm_memory, off, len)
    }

/*! Clears all frame-memory storage and resets the active range. */
function memory_reset() -> unit = {
    mem_clear();
    evm_memory = byte_slice(EvmMemorySource, BYTE_ZERO, BYTE_ZERO)
}

/*! Expands the active frame memory to an already-charged size. */
function memory_expand_to(new_size : memory_length) -> unit =
    if evm_memory_size() < new_size then {
        if mem_expand(new_size) then {
            evm_memory = byte_slice(EvmMemorySource, evm_memory.off, new_size)
        } else {
            exc_halt(OutOfGas)
        }
    }

/* Entering a child returns the saved parent pointer. Restoring that value is
 * the complete Sail-side memory-frame unwind; no second size register exists. */
/*! Saves the parent range and enters an empty child-memory frame. */
function memory_frame_enter() -> ByteSlice = {
    let parent = evm_memory;
    let base = mem_frame_enter();
    if base == MAX_BYTE_QUANTITY then {
        exc_halt(OutOfGas);
        evm_memory = byte_slice(EvmMemorySource, BYTE_ZERO, BYTE_ZERO)
    } else {
        evm_memory = byte_slice(EvmMemorySource, base, BYTE_ZERO)
    };
    parent
}

/*! Releases child memory and restores the saved parent range. */
function memory_frame_leave(parent : ByteSlice) -> unit = {
    mem_frame_leave();
    evm_memory = parent
}

/*! Captures parent execution state and enters child stack and memory frames. */
function suspend_frame() -> FrameCheckpoint = {
    let state = k_state_checkpoint();
    let saved_pc = pc;
    let saved_gas = gas_remaining;
    let saved_refund = frame_refund;
    let saved_status = frame_status;
    let saved_message = message;
    let saved_depth = call_depth;
    let saved_code = frame_code;
    let saved_calldata = calldata;
    stack_enter_frame();
    let saved_memory = memory_frame_enter();
    struct {
        state = state,
        pc = saved_pc,
        gas_remaining = saved_gas,
        refund = saved_refund,
        status = saved_status,
        message = saved_message,
        call_depth = saved_depth,
        code = saved_code,
        calldata = saved_calldata,
        memory = saved_memory,
    }
}

/*! Restores a suspended parent after nested execution completes. */
function restore_frame(checkpoint : FrameCheckpoint) -> unit = {
    stack_leave_frame();
    memory_frame_leave(checkpoint.memory);
    pc = checkpoint.pc;
    gas_remaining = checkpoint.gas_remaining;
    frame_refund = checkpoint.refund;
    frame_status = checkpoint.status;
    message = checkpoint.message;
    call_depth = checkpoint.call_depth;
    frame_code = checkpoint.code;
    calldata = checkpoint.calldata
}

/*! One memory byte; zero when the frame has halted. */
function mem_get_byte(off : memory_pointer) -> byte =
    if is_running() then {
        mem_read_byte(off)
    } else {
        0x00
    }

/*! Writes one memory byte and raises the high-water mark; a no-op when
    halted. */
function mem_set_byte(off : memory_pointer, v : byte) -> unit =
    if is_running() then {
        mem_write_byte(off, v)
    }

/*! `MLOAD`: the big-endian word at `off`; zero when halted. */
function mem_load(off : memory_pointer) -> word =
    if is_running() then {
        mem_load_word(off)
    } else {
        ZERO_WORD
    }

/*! `MSTORE`: writes the big-endian word at `off` and raises the
    high-water mark. */
function mem_store(off : memory_pointer, w : word) -> unit =
    if is_running() then {
        mem_store_word(off, w)
    }

/*! `MSTORE8`: writes the low byte of `w`. */
function mem_store_byte(off : memory_pointer, w : word) -> unit =
    mem_set_byte(off, w[7 .. 0])

/*! `MCOPY` (EIP-5656): overlapping-safe memory-to-memory copy. */
function mem_mcopy(dst : memory_pointer, src : memory_pointer, len : memory_length) -> unit =
    if len != BYTE_ZERO then {
        mem_move(dst, src, len)
    }

/*! `CODECOPY`: copies the frame's own code into memory, zero-padded past
    the end. */
function mem_codecopy(dst : memory_pointer, off : source_pointer, len : memory_length) -> unit =
    slice_copy(frame_code.bytes, dst, off, len)

/*! `KECCAK256` over the memory range `[off, off+len)`: semantically, the
    caller reads the addressed bytes and hashes that byte list. */
function mem_keccak(off : memory_pointer, len : memory_length) -> word =
    keccak256(memory_byte_slice(off, len))
