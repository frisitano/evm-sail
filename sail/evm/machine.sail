
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
register pc : code_pointer = 0
/*! The frame's remaining gas (YP μ_g) — the one gas counter. */
register gas_remaining : gas = GAS_ZERO /* THE gas counter (user-space) */
/*! The Amsterdam state-gas reservoir owned by the active frame. */
register state_gas_remaining : gas = GAS_ZERO
/*! State gas drawn from execution gas after the reservoir was exhausted. */
register state_gas_spilled : state_gas_spill = STATE_GAS_SPILL_ZERO
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

/* Suspended parents occupy a fixed protocol-sized arena. Popping or resetting
 * changes only the top pointer; stale entries are overwritten by later calls. */
/*! The continuation arena for nested calls and creates. */
register frame_stack : FrameStack = vector_init(DEFAULT_FRAME_CONTINUATION)
/*! The number of live continuations in [frame_stack][]. */
register frame_stack_top : frame_depth = 0

/*! Starts a top-level interpretation with no suspended parent. */
function frame_stack_reset() -> unit = frame_stack_top = 0

/*! Whether the active frame is the top-level frame. */
function frame_stack_is_empty() -> bool = frame_stack_top == 0

/*! Saves the action to perform when the newly-entered child finishes. */
function frame_stack_push(continuation : FrameContinuation) -> unit = {
    let top = frame_stack_top;
    if top < DEPTH_LIMIT then {
        frame_stack[top] = continuation;
        frame_stack_top = top + 1
    } else {
        assert(false)
    }
}

/*! Removes and returns the current child frame's parent continuation. */
function frame_stack_pop() -> FrameContinuation = {
    let top = frame_stack_top;
    if 0 < top then {
        let parent_top = top - 1;
        frame_stack_top = parent_top;
        frame_stack[parent_top]
    } else {
        assert(false);
        DEFAULT_FRAME_CONTINUATION
    }
}

/*! Adds two semantically bounded refund quantities and narrows their exact sum
    back to the accumulator range. The dependent signature preserves the input
    values and their bounds for proof extraction; optimized C specializes the
    same source body to the representations demanded by each caller. */
val validated_refund_add : forall 'left 'right,
                                -gas_refund_bound
                             <= 'left
                             &  'left
                             <= gas_refund_bound
                             &  -gas_refund_bound
                             <= 'right
                             &  'right
                             <= gas_refund_bound. (int('left), int('right)) -> gas_refund

function validated_refund_add(left, right) = {
    let total = left + right;
    if (-sizeof(gas_refund_bound) <= total) & (total <= sizeof(gas_refund_bound)) then {
        total
    } else {
        throw InvalidBlock(ExecutionInvalid)
    }
}

val record_refund : forall 'delta, -gas_refund_bound <= 'delta & 'delta <= gas_refund_bound. int('delta) -> unit
function record_refund(delta) =
    frame_refund = validated_refund_add(frame_refund, delta)

/*! The frame's executable code as one indivisible value: its byte slice
    plus the resolved `JUMPDEST` table produced when that byte sequence
    was indexed. Frame creators supply this value; the interpreter only
    reads it. Keeping the pair in one register makes mismatched
    bytes/analysis unrepresentable during frame save and restore. */
register frame_code : Code = EMPTY_CODE

/*! The frame code length in bytes (`CODESIZE`). */
function frame_code_len() -> code_length = {
    let code = frame_code.bytes;
    let length = code.len;
    length
}

/*! Whether `dest` is a valid `JUMPDEST` in the frame's code
    (YP §9.4.3). */
val frame_jumpdest_valid : forall 'dest, source_valid_length('dest). int('dest) -> bool
function frame_jumpdest_valid(dest) = {
    let code = frame_code.bytes;
    let length = code.len;
    jumpdest_ref_contains(frame_code.jumpdests, length, dest)
}

/*md
## The operand stack
*/
/*! The 1024-element operand-stack limit (YP §9.1). */
let STACK_LIMIT : operand_stack_height = 1024

/* Every caller establishes that both operands belong to one admitted gas
 * budget. The canonical model records the exact arithmetic; the production C
 * splice checks its fixed-width representation only at these credit points. */
/*! Adds gas whose shared-budget conservation invariant is already known. */
val conserved_gas_add : forall 'left 'right, live_gas_valid('left) & live_gas_valid('right). (int('left), int('right)) -> gas
function conserved_gas_add(left, right) = left + right

/*! Returns any Amsterdam state-gas spill to the execution-gas pool and
    restores the frame's state-gas reservoir. */
function refill_frame_state_gas() -> unit = {
    if k_fork >= Amsterdam then {
        gas_remaining = conserved_gas_add(gas_remaining, state_gas_spilled);
        state_gas_remaining = message.state_gas_reservoir;
        state_gas_spilled = STATE_GAS_SPILL_ZERO
    }
}

/*! Computes the signed state gas consumed by the current frame. */
function frame_state_gas_used() -> frame_state_gas_delta = {
    let entry = message.state_gas_reservoir;
    let remaining = state_gas_remaining;
    let spilled = state_gas_spilled;
    entry - remaining + spilled
}

/*! Enters the exceptional halting state (YP §9.4.2), consuming all
    remaining execution gas after consolidating the frame's state gas. */
function exc_halt(k : ExceptionKind) -> unit = {
    refill_frame_state_gas();
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
function push_word(w : word) -> unit =
    if STACK_LIMIT <= stack_height() then {
        exc_halt(StackOverflow)
    } else {
        stack_push_word(w)
    }

/*! Pushes the mathematical live-gas quantity as an EVM word. Transaction
    validation establishes that every reachable value is below the word
    modulus; the total canonical definition retains modular behavior outside
    that invariant. */
function push_gas(value : gas) -> unit =
    push_word(U256(tmod_nat(value, sizeof(word_modulus))))

overload push = {push_word, push_gas}

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
function returndata_size() -> source_pointer = {
    let data = returndata;
    data.len
}

/*! One returndata byte. */
function returndata_byte(index : source_pointer) -> byte =
    slice_byte(returndata, index)

/*! `RETURNDATACOPY` into frame memory (bounds pre-checked by the
    opcode). */
val returndata_copy : forall 'dst 'off 'len,
                        host_valid_access('dst) & source_valid_length('off) & host_valid_access('len). (
        int('dst),
        int('off),
        int('len),
    ) -> unit
function returndata_copy(dst, off, len) =
    slice_copy(returndata, dst, off, len)

/*! Copies `min(want, size)` returndata bytes — the `CALL`-family output
    write-back. */
function returndata_copy_prefix(dst : memory_pointer, want : memory_length) -> unit = {
    let wanted = want;
    let available = returndata_size();
    let copy_length : memory_length = if wanted < available then { wanted } else { available };
    slice_copy(returndata, dst, 0, copy_length)
}

/* EIP-211 validates the source range even when its length is zero. This
 * operation is deliberately separate from memory-range charging: source
 * failure is an out-of-bounds exceptional halt, not memory OOG. */
/*md
## The frame memory

The EVM's view over frame memory, a region of the host interface. Every
access is guarded by [is_running][]: an unaffordable expansion sets the halt
status before [memory_expand_to][] is reached, so a post-out-of-gas write to a
huge offset (for example, `MSTORE` at 2^32) cannot grow the backing buffer.
Successful writes raise the high-water mark that `MSIZE` and
memory-expansion gas read.
*/
/*! Validates word-sized returndata bounds and copies the requested range. */
val returndata_remaining : forall 'available 'offset,
                             source_valid_length('available) & 0 <= 'offset & 'offset <= 'available. (
        int('available),
        int('offset),
    ) -> range(0, 'available)
function returndata_remaining(available, offset) = available - offset

val validated_returndata_copy : forall 'source_offset 'length,
                                  0 <= 'source_offset & 'source_offset < 2 ^ 256 & 0 <= 'length & 'length < 2 ^ 256. (
        memory_pointer,
        int('source_offset),
        int('length),
    ) -> unit
/*! Copies a returndata range after validating its word-sized offset and length. */
function validated_returndata_copy(dst, source_offset, length) = {
    let (available as 'available) = returndata_size();
    if source_offset <= available then {
        let remaining = returndata_remaining(available, source_offset);
        let bounded_source_offset : source_pointer = source_offset;
        if length <= remaining then {
            let bounded_length : memory_length = length;
            returndata_copy(dst, bounded_source_offset, bounded_length)
        } else {
            exc_halt(InvalidOpcode)
        }
    } else {
        exc_halt(InvalidOpcode)
    }
}

function returndata_copy_words(dst : memory_pointer, source_offset : word, length : word) -> unit =
    validated_returndata_copy(dst, source_offset, length)

/* ----------------------- frame memory (EVM view) ------------------------ */
/* Each frame owns one absolute slice of the shared host arena. Relative EVM
 * offsets are represented by subslicing this value, so calldata captured
 * from a suspended parent and an immediate memory read have the same source
 * representation. The slice length is the exact highest accessed byte; gas
 * and `MSIZE` round that watermark to EVM words when required. */
/*! The active frame's established memory range in the shared memory arena. */
register evm_memory : ByteSlice = byte_slice(EvmMemorySource, 0, 0)

/*! Returns the active frame's exact byte high-water mark. */
function evm_memory_high_water() -> memory_length = {
    let memory = evm_memory;
    let length = memory.len;
    length
}

/*! Clears all frame-memory storage and resets the active range. */
function memory_reset() -> unit = {
    mem_clear();
    evm_memory = byte_slice(EvmMemorySource, 0, 0)
}

/*! Expands the active frame memory to an already-charged size and returns the
    resulting high-water-mark witness. */
val memory_expand_to : forall 'required, source_valid_range(0, 'required). int('required) -> ByteSliceLength('required)

function memory_expand_to(new_size) = {
    let memory = evm_memory;
    if memory.len < new_size then {
        let expanded = mem_expand(new_size);
        evm_memory = expanded;
        expanded
    } else {
        sub_slice(memory, 0, new_size)
    }
}

/*! Returns an already-expanded memory range. The successful expansion value
    retains the high-water containment fact that would be lost by reading the
    existential [evm_memory][] register again. */
val memory_byte_slice : forall 'off 'len, memory_valid_range('off, 'len). (int('off), int('len)) -> ByteSlice

function memory_byte_slice(off, len) =
    if len == 0 then {
        EMPTY_SLICE
    } else {
        sub_slice(memory_expand_to(off + len), off, len)
    }

/*! Returns expanded memory as executable initcode, retaining the cursor
    headroom established by the successful memory-expansion range. */
val memory_code_slice : forall 'off 'len, memory_valid_range('off, 'len). (int('off), int('len)) -> CodeSlice

function memory_code_slice(off, len) =
    if len == 0 then {
        EMPTY_CODE_SLICE
    } else {
        code_slice(sub_slice(memory_expand_to(off + len), off, len))
    }

/* Entering a child returns the saved parent pointer. Restoring that value is
 * the complete Sail-side memory-frame unwind; no second size register exists. */
/*! Saves the parent range and enters an empty child-memory frame. */
function memory_frame_enter() -> ByteSlice = {
    let parent = evm_memory;
    let base : source_pointer = mem_frame_enter();
    evm_memory = byte_slice(EvmMemorySource, base, 0);
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
    let saved_state_gas = state_gas_remaining;
    let saved_state_spill = state_gas_spilled;
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
        state_gas_remaining = saved_state_gas,
        state_gas_spilled = saved_state_spill,
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
    state_gas_remaining = checkpoint.state_gas_remaining;
    state_gas_spilled = checkpoint.state_gas_spilled;
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
    mem_set_byte(off, word_low_byte(w))

/*! `MCOPY` (EIP-5656): overlapping-safe memory-to-memory copy. */
function mem_mcopy(dst : memory_pointer, src : memory_pointer, len : memory_length) -> unit =
    if len != 0 then {
        mem_move(dst, src, len)
    }

/*! `CODECOPY`: copies the frame's own code into memory, zero-padded past
    the end. */
function mem_codecopy(dst : memory_pointer, off : source_pointer, len : memory_length) -> unit =
    slice_copy(frame_code.bytes, dst, off, len)

/*! `KECCAK256` over the memory range `[off, off+len)`: semantically, the
    caller reads the addressed bytes and hashes that byte list. */
function mem_keccak(range : MemoryRange) -> word =
    hash_to_word(keccak256(memory_byte_slice(range.off, range.len)))
