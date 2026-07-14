/*md
# Machine state

The user-space half of the machine: per-frame registers, operand stack,
gas counter, and the saved-frame stack for nested calls. It holds the gas
counter and all per-frame compute state (`pc`, stack, memory, code,
message, returndata). World state lives in the host kernel and is reached
only via `k_*` calls; world rollback on revert is a kernel snapshot, not
saved here.

## Active-frame registers

One [Message][type-Message] per active frame (YP "message call"
`I = (I_a, I_o, I_s, …)`); sub-calls save and restore these registers in
the interpreter.
*/

/*! The program counter (YP μ_pc). */
register pc : int = 0
/*! The frame's remaining gas (YP μ_g) — the one gas counter. */
register gas_remaining : gas = 0
/*! The frame's running/halted/exceptional status. */
register frame_status : FrameStatus = Running()
/*! The frame's call message. */
register message : Message = DEFAULT_MESSAGE
/*! The call depth (YP I_e), bounded by `DEPTH_LIMIT`. */
register call_depth : int = 0

/*! The frame's executable code as one indivisible value: its byte slice
    plus the resolved `JUMPDEST` table produced when that byte sequence
    was indexed. Frame creators supply this value; the interpreter only
    reads it. Keeping the pair in one register makes mismatched
    bytes/analysis unrepresentable during frame save and restore. */
register frame_code : Code = EMPTY_CODE

/*! The frame code length in bytes (`CODESIZE`). */
function frame_code_len() -> int = unsigned(frame_code.bytes.len)

/*! Whether `dest` is a valid `JUMPDEST` in the frame's code
    (YP §9.4.3). */
function frame_jumpdest_valid(dest : int) -> bool =
    jumpdest_ref_contains(frame_code.jumpdests, frame_code.bytes.len, get_slice_int(64, dest, 0))

/*md
## The operand stack
*/

/*! The 1024-element operand-stack limit (YP §9.1). */
let STACK_LIMIT : int = 1024

/*! Enters the exceptional halting state (YP §9.4.2), consuming all
    remaining gas — unlike `REVERT`, which refunds it. Every exceptional
    halt routes through here so the gas is always zeroed. */
function exc_halt(k : ExceptionKind) -> unit = {
    gas_remaining = 0;
    frame_status = Exceptional(k)
}

/*! The stack height as a Sail integer. */
function stack_height() -> int = unsigned(stack_depth())

/*! The `n`-th-from-top operand (`n = 0` is the top); zero out of
    range. */
function peek(n : int) -> word = stack_peek_word(get_slice_int(64, n, 0))

/*! Pushes a word, entering `StackOverflow` at the 1024 limit. */
function push(w : word) -> unit =
    if stack_height() >= STACK_LIMIT then {
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
function stack_set(n : int, w : word) -> unit = stack_set_word(get_slice_int(64, n, 0), w)

/*! Pops the top word as an unsigned integer. */
function pop_int() -> int = unsigned(pop())

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
function returndata_size() -> int = unsigned(returndata.len)

/*! One returndata byte. */
function returndata_byte(index : int) -> byte =
    slice_byte(returndata, index)

/*! `RETURNDATACOPY` into frame memory (bounds pre-checked by the
    opcode). */
function returndata_copy(dst : int, off : int, len : int) -> unit =
    slice_copy(returndata, dst, off, len)

/*! Copies `min(want, size)` returndata bytes — the `CALL`-family output
    write-back. */
function returndata_copy_prefix(dst : int, want : int) -> unit = {
    let have = returndata_size();
    slice_copy(returndata, dst, 0, if want < have then { want } else { have })
}

/*md
## Frame memory (the EVM's view)

The EVM's view over the C-backed frame memory. Every access is guarded by
[is_running][] — a failed expansion charge sets the halt status without
aborting the opcode handler, and a post-out-of-gas write to a huge offset
(e.g. `MSTORE` at 2^32) must not grow the C buffer — and writes raise the
high-water mark that `MSIZE` and memory-expansion gas read.
*/

/*! The high-water mark in bytes: the largest offset ever touched in this
    frame. `MSIZE` rounds it up to a word; memory-expansion gas is charged
    on each growth of [mem_size_words][] (YP §9.4.1, the C_mem term). */
register memory_size : int = 0

/*! One memory byte; zero when the frame has halted. */
function mem_get_byte(off : int) -> byte =
    if is_running() then {
        mem_read_byte(get_slice_int(64, off, 0))
    } else {
        0x00
    }

/*! Writes one memory byte and raises the high-water mark; a no-op when
    halted. */
function mem_set_byte(off : int, v : byte) -> unit =
    if is_running() then {
        mem_write_byte(get_slice_int(64, off, 0), v);
        if off + 1 > memory_size then {
            memory_size = off + 1
        }
    }

/*! `MLOAD`: the big-endian word at `off`; zero when halted. */
function mem_load(off : int) -> word =
    if is_running() then {
        mem_load_word(get_slice_int(64, off, 0))
    } else {
        ZERO_WORD
    }

/*! `MSTORE`: writes the big-endian word at `off` and raises the
    high-water mark. */
function mem_store(off : int, w : word) -> unit =
    if is_running() then {
        mem_store_word(get_slice_int(64, off, 0), w);
        if off + 32 > memory_size then {
            memory_size = off + 32
        }
    }

/*! `MSTORE8`: writes the low byte of `w`. */
function mem_store_byte(off : int, w : word) -> unit =
    mem_set_byte(off, w[7 .. 0])

/*! The memory size in 32-byte words, rounded up — the `MSIZE` value and
    the unit of memory-expansion gas. */
function mem_size_words() -> int = ediv_int(memory_size + 31, 32)

/*! `MCOPY` (EIP-5656): overlapping-safe memory-to-memory copy. */
function mem_mcopy(dst : int, src : int, len : int) -> unit =
    mem_move(get_slice_int(64, dst, 0), get_slice_int(64, src, 0), get_slice_int(64, len, 0))

/*! `CODECOPY`: copies the frame's own code into memory, zero-padded past
    the end. */
function mem_codecopy(dst : int, off : int, len : int) -> unit =
    slice_copy(frame_code.bytes, dst, off, len)

/*! `KECCAK256` over the memory range `[off, off+len)`: semantically, the
    caller reads the addressed bytes and hashes that byte list. */
function mem_keccak(off : int, len : int) -> word =
    keccak256(memory_byte_slice(off, len))
