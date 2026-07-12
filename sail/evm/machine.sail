/* ======================================================================== */
/* EVM USER-SPACE machine: per-frame registers, operand stack, gas counter, */
/* and the saved-frame stack for nested calls                               */
/*                                                                          */
/* This is the user-space half. It holds the gas counter and all per-frame  */
/* compute state (pc, stack, memory, code, message, returndata). World      */
/* state lives in the host kernel and is reached only via k_*               */
/* calls. World rollback on revert is a kernel snapshot, NOT saved here.    */
/* ======================================================================== */

/* The per-frame call context (YP "message call" I = (I_a, I_o, I_s, ...)).
 * One Message per active frame; sub-calls save/restore it (interpreter.sail). */
/* ----------------------- active-frame registers ------------------------- */

register pc : int = 0
register gas_remaining : gas = 0 /* THE gas counter (user-space) */
register frame_status : FrameStatus = Running()
register message : Message = DEFAULT_MESSAGE
register call_depth : int = 0

/* ------------------------- executable code ------------------------------ */
/* The frame's executable code is one indivisible value: its ByteSlice plus
 * the resolved JumpdestRef produced when that byte sequence was indexed and
 * inserted. Frame creators supply this value; the interpreter only reads it.
 * Keeping the pair in one register makes mismatched bytes/analysis
 * unrepresentable during frame save and restore. */
register frame_code : IndexedCode = EMPTY_INDEXED_CODE

function frame_code_len() -> int = unsigned(frame_code.bytes.len)

function frame_jumpdest_valid(dest : int) -> bool =
    jumpdest_ref_contains(frame_code.jumpdests, frame_code.bytes.len, get_slice_int(64, dest, 0))

/* ------------------------------ the stack ------------------------------- */

let STACK_LIMIT : int = 1024 /* YP: 1024-element operand-stack limit */

/* An exceptional halt (OOG, invalid opcode/jump, stack error, write protection,
 * initcode-too-large) consumes ALL remaining gas -- unlike REVERT, which returns
 * it. Route every exceptional halt through here so the gas is always zeroed. */
function exc_halt(k : ExceptionKind) -> unit = {
    gas_remaining = 0;
    frame_status = Exceptional(k)
}

/* The host ABI returns a fixed-width scalar; EVM bounds use Sail integers. */
function stack_height() -> int = unsigned(stack_depth())

/* nth-from-top operand (n = 0 is the top); zero if out of range */
function peek(n : int) -> word = stack_peek_word(get_slice_int(64, n, 0))

function push(w : word) -> unit =
    if stack_height() >= STACK_LIMIT then {
        exc_halt(StackOverflow)
    } else {
        stack_push_word(w)
    }

function pop() -> word =
    if stack_height() == 0 then {
        exc_halt(StackUnderflow);
        WORD_ZERO
    } else {
        stack_pop_word()
    }

/* overwrite the nth-from-top operand (SWAP) */
function stack_set(n : int, w : word) -> unit = stack_set_word(get_slice_int(64, n, 0), w)

function pop_int() -> int = unsigned(pop())

function is_running() -> bool = match frame_status {
    Running() => true,
    _ => false,
}

/* The current frame's calldata (I_d), as a REFERENCE: the tx input slice at
 * the top level, a minted slice of the suspended caller's memory for
 * sub-calls, EMPTY_SLICE for initcode frames. Saved/restored across
 * sub-calls by the interpreter like the other frame registers. */
register calldata : ByteSlice = EMPTY_SLICE

/* ----------------------- frame memory (EVM view) ------------------------ */
/* The EVM's view over the C-backed frame memory (host/memory.sail): every
 * access is guarded by is_running -- a failed charge_expansion sets the halt
 * status without aborting the opcode handler, and a post-out-of-gas write to
 * a huge offset (e.g. MSTORE at 2^32) must not grow the C buffer -- and
 * writes raise the high-water mark that MSIZE and memory-expansion gas read. */

/* The high-water mark, in bytes: the largest offset ever touched in this
 * frame. MSIZE rounds it up to a word; memory-expansion gas is charged on
 * each growth of mem_size_words (Yellow Paper section 9.4.1, the C_mem term). */
register memory_size : int = 0

function mem_get_byte(off : int) -> byte =
    if is_running() then {
        mem_read_byte(get_slice_int(64, off, 0))
    } else {
        0x00
    }

function mem_set_byte(off : int, v : byte) -> unit =
    if is_running() then {
        mem_write_byte(get_slice_int(64, off, 0), v);
        if off + 1 > memory_size then {
            memory_size = off + 1
        }
    }

/* --- word access (big-endian, 32 bytes): MLOAD / MSTORE / MSTORE8 --- */

function mem_load(off : int) -> word =
    if is_running() then {
        mem_load_word(get_slice_int(64, off, 0))
    } else {
        ZERO_WORD
    }

function mem_store(off : int, w : word) -> unit =
    if is_running() then {
        mem_store_word(get_slice_int(64, off, 0), w);
        if off + 32 > memory_size then {
            memory_size = off + 32
        }
    }

function mem_store_byte(off : int, w : word) -> unit =
    mem_set_byte(off, w[7 .. 0])

/* Current memory size in 32-byte words, rounded up (the MSIZE value, and the
 * unit of memory-expansion gas). */
function mem_size_words() -> int = ediv_int(memory_size + 31, 32)

/* MCOPY (EIP-5656): overlapping-safe memory-to-memory copy (one C memmove). */
function mem_mcopy(dst : int, src : int, len : int) -> unit =
    mem_move(get_slice_int(64, dst, 0), get_slice_int(64, src, 0), get_slice_int(64, len, 0))

/* CODECOPY: the current frame's code slice into memory. */
function mem_codecopy(dst : int, off : int, len : int) -> unit =
    slice_copy(frame_code.bytes, dst, off, len)

/* KECCAK256 over the memory range [off, off+len). Semantically, the caller
 * reads the addressed bytes and hashes that byte list. */
function mem_keccak(off : int, len : int) -> word =
    keccak256(memory_byte_slice(off, len))
