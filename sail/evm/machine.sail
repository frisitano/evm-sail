
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

val record_refund : forall 'delta, -gas_refund_bound <= 'delta & 'delta <= gas_refund_bound. (gas_refund, int('delta)) -> gas_refund
function record_refund(refund, delta) =
    validated_refund_add(refund, delta)

/*! The frame code length in bytes (`CODESIZE`). */
function frame_code_len(frame_code : Code) -> code_length = {
    let code = frame_code;
    let length = code.len;
    length
}

/*! Whether `dest` is a valid `JUMPDEST` in the frame's code
    (YP §9.4.3). */
val frame_jumpdest_valid : forall 'dest, source_valid_length('dest). (Code, int('dest)) -> bool
function frame_jumpdest_valid(frame_code, dest) = {
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
 * budget. `gas_credit` retains that relationship without a runtime check. */
/*! Adds gas whose shared-budget conservation invariant is already known. */
val conserved_gas_add : (gas, gas) -> gas
function conserved_gas_add(available, credit) =
    if credit <= (2 ^ 64 - 1) - available then {
        available + credit
    } else {
        fatal_error(ExecutionInvalid)
    }

/*! Returns any Amsterdam state-gas spill to the carried execution gas and
    restores the frame's state-gas reservoir. */
val refill_frame_state_gas : (gas, state_gas, state_gas_spill, state_gas) -> (gas, state_gas, state_gas_spill)
function refill_frame_state_gas(g, state_gas_remaining, state_gas_spilled, state_gas_reservoir) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if profile.fork >= Amsterdam then {
        let refilled = conserved_gas_add(g, state_gas_spilled);
        (refilled, state_gas_reservoir, STATE_GAS_SPILL_ZERO)
    } else {
        (g, state_gas_remaining, state_gas_spilled)
    }
}

/*! Computes the signed state gas consumed by the current frame. */
function frame_state_gas_used(state_gas_reservoir : state_gas, state_gas_remaining : state_gas, state_gas_spilled : state_gas_spill) -> frame_state_gas_delta = {
    let entry = state_gas_reservoir;
    let remaining = state_gas_remaining;
    let spilled = state_gas_spilled;
    entry - remaining + spilled
}

/*! Constructs an exceptional frame status while normalizing state gas.
    The exception producer is responsible for returning zero execution gas.
    Amsterdam restores the frame's state-gas reservoir and clears its spill. */
val exceptional_state :
    (state_gas, state_gas_spill, state_gas, ExceptionKind) -> {'next_spill,
           0 <= 'next_spill
        &  'next_spill <= 2 ^ 24
        &  'next_spill <= transaction_execution_gas_limit_value.
        (state_gas, int('next_spill), FrameStatus)}
function exceptional_state(state_gas_remaining, state_gas_spilled, state_gas_reservoir, k) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if profile.fork >= Amsterdam then
        (state_gas_reservoir, STATE_GAS_SPILL_ZERO, Exceptional(k))
    else
        (state_gas_remaining, state_gas_spilled, Exceptional(k))
}

/*! The stack height below a carried cursor. */
function stack_height(top : StackPointer) -> operand_stack_height = stack_top_height(top)

/*! Checks the Yellow Paper stack precondition for one instruction before it
    charges gas or performs side effects. `inputs` is the instruction's
    required stack height (delta) and `outputs` is the height it contributes
    after consuming those inputs (alpha). This is the single stack-bounds
    guard: handler bodies consume and produce operands unchecked behind it. */
enum StackValidation = {
    StackValid,
    StackUnderflowFailure,
    StackOverflowFailure,
}

val validate_stack : (StackPointer, operand_stack_height, operand_stack_height) -> StackValidation
function validate_stack(top, inputs, outputs) = {
    let height = stack_height(top);
    if height < inputs then StackUnderflowFailure
    else if STACK_LIMIT < height - inputs + outputs then StackOverflowFailure
    else StackValid
}

/*! Reads the `n=0` slot under a validated cursor. */
function read_stack_word(sp : StackPointer) -> word = stack_slot_read(sp, 0)

/*! Writes the `n=0` slot under a validated cursor. */
function write_stack_word(sp : StackPointer, value : word) -> unit =
    stack_slot_write(sp, 0, value)

/*! Overwrites the `n`-th-from-top operand (`SWAP`); the cursor is
    unchanged. */
function stack_set(top : StackPointer, n : stack_index, w : word) -> unit =
    stack_slot_write(top, n, w)

/*! Whether the frame is still running. */
function is_running(frame_status : FrameStatus) -> bool = match frame_status {
    Running() => true,
    _ => false,
}

/*! Installs the frame's calldata reference. */
function calldata_install(data : CalldataSlice) -> CalldataSlice = data

/*! Clears the returndata buffer (a new sub-call begins). */
function returndata_clear() -> OutputSlice = EMPTY_OUTPUT_SLICE

/*! `RETURNDATASIZE`. */
function returndata_size(returndata : OutputSlice) -> source_pointer = {
    let data = returndata;
    data.len
}

/*! `RETURNDATACOPY` into frame memory (bounds pre-checked by the
    opcode). */
val returndata_copy : forall 'dst 'off 'len,
                        host_valid_access('dst) & source_valid_length('off) & host_valid_access('len). (
        OutputSlice,
        int('dst),
        int('off),
        int('len),
    ) -> unit
function returndata_copy(returndata, dst, off, len) =
    slice_copy(returndata, dst, off, len)

/*! Copies `min(want, size)` returndata bytes — the `CALL`-family output
    write-back. */
function returndata_copy_prefix(returndata : OutputSlice, dst : memory_pointer, want : memory_length) -> unit = {
    let wanted = want;
    let available = returndata_size(returndata);
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
/*! Computes the returndata remaining after a validated source offset. */
val returndata_remaining : forall 'available 'offset,
                             source_valid_length('available) & 0 <= 'offset & 'offset <= 'available. (
        int('available),
        int('offset),
    ) -> range(0, 'available)
function returndata_remaining(available, offset) = available - offset

/* ----------------------- frame memory (EVM view) ------------------------ */
/* Each frame owns one absolute slice of the shared host arena. Relative EVM
 * offsets are represented by subslicing this value, so calldata captured
 * from a suspended parent and an immediate memory read have the same source
 * representation. The slice length is the exact highest accessed byte; gas
 * and `MSIZE` round that watermark to EVM words when required. */
/*! Returns a carried cursor's exact byte high-water mark. */
function memory_high_water(mem : EvmMemorySlice) -> memory_length = {
    let memory = mem;
    let length = memory.len;
    length
}

/*! Clears all frame-memory storage and resets the active range. */
function memory_reset() -> EvmMemorySlice = {
    mem_clear();
    EMPTY_EVM_MEMORY_SLICE
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
    let base : source_pointer = mem_frame_enter();
    evm_memory_slice(base, 0)
}

/*! Releases child memory and restores the saved parent range. */
function memory_frame_leave(parent : EvmMemorySlice) -> EvmMemorySlice = {
    mem_frame_leave();
    parent
}

/*! Captures parent execution state and enters child stack and memory frames.
    The caller has published its carried machine state to the frame
    registers: this checkpoint reads them at the one authoritative
    boundary. The host operand-frame cursor mirrors `call_depth`: push the
    empty child operand stack before installing the child's semantic depth,
    and pop it before restoring the parent's semantic depth. */
function suspend_frame(
    pc : code_pointer,
    gas_remaining : gas,
    stack_top : StackPointer,
    evm_memory : EvmMemorySlice,
    state_gas_remaining : state_gas,
    state_gas_spilled : state_gas_spill,
    frame_refund : gas_refund,
    frame_status : FrameStatus,
    message : Message,
    frame_code : Code,
    calldata : CalldataSlice,
) -> (FrameCheckpoint, StackPointer, EvmMemorySlice) = {
    k_journal_checkpoint();
    let child_stack = operand_stack_push_empty_frame();
    let child_memory = memory_frame_enter();
    let checkpoint : FrameCheckpoint = struct {
        pc = pc,
        gas_remaining = gas_remaining,
        stack_top = stack_top,
        state_gas_remaining = state_gas_remaining,
        state_gas_spilled = state_gas_spilled,
        refund = frame_refund,
        status = frame_status,
        message = message,
        code = frame_code,
        calldata = calldata,
        memory = evm_memory,
    };
    (checkpoint, child_stack, child_memory)
}

/*! Restores a suspended parent after nested execution completes. */
function restore_frame(checkpoint : FrameCheckpoint) -> (code_pointer, gas, StackPointer, EvmMemorySlice, state_gas, state_gas_spill, gas_refund, FrameStatus, Message, Code, CalldataSlice) = {
    operand_stack_pop_frame();
    let memory = memory_frame_leave(checkpoint.memory);
    (
        checkpoint.pc,
        checkpoint.gas_remaining,
        checkpoint.stack_top,
        memory,
        checkpoint.state_gas_remaining,
        checkpoint.state_gas_spilled,
        checkpoint.refund,
        checkpoint.status,
        checkpoint.message,
        checkpoint.code,
        checkpoint.calldata,
    )
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
