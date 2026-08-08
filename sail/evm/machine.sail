
/*md
# Machine state

The user-space half of the machine: per-frame registers, operand stack,
gas counter, and the saved-frame stack for nested calls. World state lives
in the host kernel and is reached only via `k_*` calls; world rollback on
revert is a kernel snapshot, not saved here.

## The state-passing convention

Hot machine state flows through handler signatures instead of registers,
mirroring the Yellow Paper's state-passing transition μ′ = Ξ(μ): the
remaining gas (μ_g), the program counter (μ_pc), the operand-stack cursor
(μ_s), and the frame-memory extent (μ_i) are carried by value from the
interpreter loop into each opcode handler and returned updated. No
handler reads or writes these registers: the canonical loop supplies each
step's arguments from the registers and assigns the returned state back,
while frame-boundary code — frame save/suspend ([suspend_frame][]),
resume ([restore_frame][]), and the transaction wrapper — synchronizes
them explicitly. The optimized interpreter carries the same values in
locals and touches the registers only at those frame boundaries.

## The frame registers

One [Message][type-Message] per active frame (YP "message call"
`I = (I_a, I_o, I_s, …)`); sub-calls save and restore these registers in
the interpreter.
*/
/*! The program counter (YP μ_pc), authoritative at frame boundaries. */
register pc : code_pointer = 0
/*! The frame's remaining gas (YP μ_g), authoritative at frame
    boundaries. */
register gas_remaining : gas = GAS_ZERO
/*! The frame's operand-stack cursor (YP μ_s), authoritative at frame
    boundaries. */
register stack_top : StackTop = 0x0000000000000000
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
/*! The call depth (YP I_e), bounded by `call_depth_limit`. */
register call_depth : frame_depth = 0

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
        fatal_error(ExecutionInvalid)
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
    let code = frame_code;
    let length = code.len;
    length
}

/*! Whether `dest` is a valid `JUMPDEST` in the frame's code
    (YP §9.4.3). */
val frame_jumpdest_valid : forall 'dest, source_valid_length('dest). int('dest) -> bool
function frame_jumpdest_valid(dest) = {
    let code = frame_code;
    let length = code.len;
    jumpdest_ref_contains(code.jumpdests, length, dest)
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

/*! Returns any Amsterdam state-gas spill to the carried execution gas and
    restores the frame's state-gas reservoir. */
function refill_frame_state_gas(g : gas) -> gas = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if profile.fork >= Amsterdam then {
        let refilled = conserved_gas_add(g, state_gas_spilled);
        state_gas_remaining = message.state_gas_reservoir;
        state_gas_spilled = STATE_GAS_SPILL_ZERO;
        refilled
    } else {
        g
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
    remaining execution gas after consolidating the frame's state gas.
    Takes the carried gas, performs the state-gas refill against it in the
    canonical read-then-zero order, and returns the frame's zeroed gas. */
function exc_halt(g : gas, k : ExceptionKind) -> gas = {
    let _ = refill_frame_state_gas(g);
    frame_status = Exceptional(k);
    GAS_ZERO
}

/*! The stack height below a carried cursor. */
function stack_height(top : StackTop) -> operand_stack_height = stack_top_height(top)

/*! Checks the Yellow Paper stack precondition for one instruction before it
    charges gas or performs side effects. `inputs` is the instruction's
    required stack height (delta) and `outputs` is the height it contributes
    after consuming those inputs (alpha). This is the single stack-bounds
    guard: handler bodies consume and produce operands unchecked behind it. */
function validate_stack(
    g : gas,
    top : StackTop,
    inputs : operand_stack_height,
    outputs : operand_stack_height,
) -> (
    (bool, gas)
) = {
    let height = stack_height(top);
    if height < inputs then {
        (false, exc_halt(g, StackUnderflow))
    } else if STACK_LIMIT < height - inputs + outputs then {
        (false, exc_halt(g, StackOverflow))
    } else {
        (true, g)
    }
}

/*! The `n`-th-from-top operand (`n = 0` is the top), below a validated
    cursor. */
function peek(top : StackTop, n : stack_index) -> word = stack_slot_read(top, n)

/*! Pushes a word below a validated cursor and returns the new cursor. */
function push_word(top : StackTop, w : word) -> StackTop = {
    let pushed = stack_top_advance(top, 1);
    stack_slot_write(pushed, 0, w);
    pushed
}

/*! Pushes the mathematical live-gas quantity as an EVM word. Transaction
    validation establishes that every reachable value is below the word
    modulus; the total canonical definition retains modular behavior outside
    that invariant. */
function push_gas(top : StackTop, value : gas) -> StackTop = {
    let reduced = tmod_nat(value, sizeof(word_modulus));
    let gas_word = u256(reduced);
    push_word(top, gas_word)
}

overload push = {push_word, push_gas}

/*! Pops the top word below a validated cursor, returning the word and the
    new cursor. */
function pop(top : StackTop) -> (word, StackTop) = {
    let value = stack_slot_read(top, 0);
    (value, stack_top_retreat(top, 1))
}

/*! Overwrites the `n`-th-from-top operand (`SWAP`); the cursor is
    unchanged. */
function stack_set(top : StackTop, n : stack_index, w : word) -> unit =
    stack_slot_write(top, n, w)

/*! Whether the frame is still running. */
function is_running() -> bool = match frame_status {
    Running() => true,
    _ => false,
}

/*! The frame's calldata (YP I_d), as a reference: the transaction input
    slice at the top level, a minted slice of the suspended caller's
    memory for sub-calls, the empty slice for initcode frames.
    Saved/restored across sub-calls like the other frame registers. */
register calldata : CalldataSlice = EMPTY_CALLDATA

/*! Installs the frame's calldata reference. */
function calldata_install(data : CalldataSlice) -> unit = calldata = data

/*! The output of the frame's latest child call (EIP-211's returndata
    buffer). The active frame's own output is carried by its
    `RETURN`/`REVERT` halt value and returned from [interpret][], rather
    than stored in another register. */
register returndata : OutputSlice = EMPTY_OUTPUT_SLICE

/*! Clears the returndata buffer (a new sub-call begins). */
function returndata_clear() -> unit = returndata = EMPTY_OUTPUT_SLICE

/*! `RETURNDATASIZE`. */
function returndata_size() -> source_pointer = {
    let data = returndata;
    data.len
}

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
    let copy_length : memory_length =
        if wanted < available then wanted else available;
    slice_copy(returndata, dst, 0, copy_length)
}

/* EIP-211 validates the source range even when its length is zero. This
 * operation is deliberately separate from memory-range charging: source
 * failure is an out-of-bounds exceptional halt, not memory OOG. */
/*md
## The frame memory

The EVM's view over frame memory, a region of the host interface. The
active frame's established extent is the carried memory cursor
[EvmMemorySlice][type-EvmMemorySlice]: its base and byte high-water mark
flow through the memory-family handlers by value, with the
[evm_memory][] register authoritative only at frame boundaries. Every
access is guarded
by gas charging against the carried cursor: an unaffordable expansion sets
the halt status before [memory_expand_to][] is reached, so a
post-out-of-gas write to a huge offset (for example, `MSTORE` at 2^32)
cannot grow the backing buffer. Successful expansions return the raised
high-water mark that `MSIZE` and memory-expansion gas read.
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
        gas,
        memory_pointer,
        int('source_offset),
        int('length),
    ) -> gas
/*! Copies a returndata range after validating its word-sized offset and
    length, entering an exceptional halt on the carried gas when the source
    range is out of bounds. */
function validated_returndata_copy(g, dst, source_offset, length) = {
    let (available as 'available) = returndata_size();
    if source_offset <= available then {
        let remaining = returndata_remaining(available, source_offset);
        let bounded_source_offset : source_pointer = source_offset;
        if length <= remaining then {
            let bounded_length : memory_length = length;
            returndata_copy(dst, bounded_source_offset, bounded_length);
            g
        } else {
            exc_halt(g, InvalidOpcode)
        }
    } else {
        exc_halt(g, InvalidOpcode)
    }
}

function returndata_copy_words(g : gas, dst : memory_pointer, source_offset : word, length : word) -> gas =
    validated_returndata_copy(g, dst, source_offset, length)

/* ----------------------- frame memory (EVM view) ------------------------ */
/* Each frame owns one absolute slice of the shared host arena. Relative EVM
 * offsets are represented by subslicing this value, so calldata captured
 * from a suspended parent and an immediate memory read have the same source
 * representation. The slice length is the exact highest accessed byte; gas
 * and `MSIZE` round that watermark to EVM words when required. */
/*! The active frame's established memory range, authoritative at frame
    boundaries; in-loop the carried cursor is the only truth. */
register evm_memory : EvmMemorySlice = EMPTY_EVM_MEMORY_SLICE

/*! Returns a carried cursor's exact byte high-water mark. */
function memory_high_water(mem : EvmMemorySlice) -> memory_length = {
    let memory = mem;
    let length = memory.len;
    length
}

/*! Clears all frame-memory storage and resets the active range. */
function memory_reset() -> unit = {
    mem_clear();
    evm_memory = EMPTY_EVM_MEMORY_SLICE
}

/*! Expands the carried frame memory to an already-charged size, returning
    the resulting high-water-mark witness and the updated cursor. */
val memory_expand_to : forall 'required, source_valid_range(0, 'required). (EvmMemorySlice, int('required)) -> (
        EvmMemorySliceLength('required),
        EvmMemorySlice,
    )

function memory_expand_to(mem, new_size) = {
    let memory = mem;
    if memory.len < new_size then {
        let expanded = mem_expand(new_size);
        (expanded, expanded)
    } else {
        (sub_slice(memory, 0, new_size), mem)
    }
}

/*! Returns an already-expanded memory range together with the updated
    cursor. The successful expansion value retains the high-water
    containment fact that would be lost by repacking the existential
    cursor. */
val active_memory_slice : forall 'off 'len, memory_valid_range('off, 'len). (EvmMemorySlice, int('off), int('len)) -> (
        EvmMemorySlice,
        EvmMemorySlice,
    )

function active_memory_slice(mem, off, len) =
    if len == 0 then {
        (EMPTY_EVM_MEMORY_SLICE, mem)
    } else {
        let (window, expanded) = memory_expand_to(mem, off + len);
        (sub_slice(window, off, len), expanded)
    }

/*! Returns expanded memory as executable initcode together with the
    updated cursor, retaining the headroom established by the successful
    memory-expansion range. */
val memory_code_slice : forall 'off 'len, memory_valid_range('off, 'len). (EvmMemorySlice, int('off), int('len)) -> (
        CodeSlice,
        EvmMemorySlice,
    )

function memory_code_slice(mem, off, len) =
    if len == 0 then {
        (EMPTY_CODE_SLICE, mem)
    } else {
        let (window, expanded) = memory_expand_to(mem, off + len);
        let initcode = sub_slice(window, off, len);
        (code_db_intern_memory(initcode), expanded)
    }

/* Entering a child returns the saved parent pointer. Restoring that value is
 * the complete Sail-side memory-frame unwind; no second size register exists. */
/*! Saves the parent range and enters an empty child-memory frame. */
function memory_frame_enter() -> EvmMemorySlice = {
    let parent = evm_memory;
    let base : source_pointer = mem_frame_enter();
    evm_memory = evm_memory_slice(base, 0);
    parent
}

/*! Releases child memory and restores the saved parent range. */
function memory_frame_leave(parent : EvmMemorySlice) -> unit = {
    mem_frame_leave();
    evm_memory = parent
}

/*! Captures parent execution state and enters child stack and memory frames.
    The caller has published its carried machine state to the frame
    registers: this checkpoint reads them at the one authoritative
    boundary. The host operand-frame cursor mirrors `call_depth`: push the
    empty child operand stack before installing the child's semantic depth,
    and pop it before restoring the parent's semantic depth. */
function suspend_frame() -> FrameCheckpoint = {
    k_journal_checkpoint();
    let saved_pc = pc;
    let saved_gas = gas_remaining;
    let saved_stack = stack_top;
    let saved_state_gas = state_gas_remaining;
    let saved_state_spill = state_gas_spilled;
    let saved_refund = frame_refund;
    let saved_status = frame_status;
    let saved_message = message;
    let saved_depth = call_depth;
    let saved_code = frame_code;
    let saved_calldata = calldata;
    stack_top = operand_stack_push_empty_frame();
    let saved_memory = memory_frame_enter();
    struct {
        pc = saved_pc,
        gas_remaining = saved_gas,
        stack_top = saved_stack,
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
    operand_stack_pop_frame();
    memory_frame_leave(checkpoint.memory);
    pc = checkpoint.pc;
    gas_remaining = checkpoint.gas_remaining;
    stack_top = checkpoint.stack_top;
    state_gas_remaining = checkpoint.state_gas_remaining;
    state_gas_spilled = checkpoint.state_gas_spilled;
    frame_refund = checkpoint.refund;
    frame_status = checkpoint.status;
    message = checkpoint.message;
    call_depth = checkpoint.call_depth;
    frame_code = checkpoint.code;
    calldata = checkpoint.calldata
}

/*! Writes one memory byte and raises the high-water mark. */
function mem_set_byte(off : memory_pointer, v : byte) -> unit =
    mem_write_byte(off, v)

/*! `MLOAD`: the big-endian word at `off`. */
function mem_load(off : memory_pointer) -> word =
    mem_load_word(off)

/*! `MSTORE`: writes the big-endian word at `off` and raises the
    high-water mark. */
function mem_store(off : memory_pointer, w : word) -> unit =
    mem_store_word(off, w)

/*! `MSTORE8`: writes the low byte of `w`. */
function mem_store_byte(off : memory_pointer, w : word) -> unit = {
    let value = word_low_byte(w);
    mem_set_byte(off, value)
}

/*! `MCOPY` (EIP-5656): overlapping-safe memory-to-memory copy. */
function mem_mcopy(dst : memory_pointer, src : memory_pointer, len : memory_length) -> unit =
    if len != 0 then {
        mem_move(dst, src, len)
    }

/*! `KECCAK256` over the memory range `[off, off+len)`: semantically, the
    caller reads the addressed bytes and hashes that byte list. Returns
    the digest word and the updated memory cursor. */
function mem_keccak(mem : EvmMemorySlice, range : MemoryRange) -> (word, EvmMemorySlice) = {
    let (bytes, expanded) = active_memory_slice(mem, range.off, range.len);
    let digest = keccak256(bytes);
    (hash_to_word(digest), expanded)
}
