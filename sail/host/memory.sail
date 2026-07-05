/* ======================================================================== */
/* PER-FRAME EVM MEMORY                                                     */
/*                                                                          */
/* The volatile byte array each call frame addresses from zero (Yellow      */
/* Paper section 9.4.1): flat, byte-addressed, zero-initialized, and lazily */
/* expanding in 32-byte words. MLOAD/MSTORE/MSTORE8/MSIZE and the *COPY     */
/* opcodes read and write it; expansion past the high-water mark is charged */
/* as memory-expansion gas.                                                 */
/*                                                                          */
/* Frames form a stack: a sub-call (CALL/CREATE) receives fresh memory      */
/* (mem_frame_enter) and the caller's is restored on return                 */
/* (mem_frame_leave); mem_clear resets to a single empty frame per          */
/* transaction. The Sail side tracks only the high-water mark (memory_size) */
/* for expansion gas; the bytes themselves live in C (ffi/memory.c) for     */
/* O(1) access, since Sail has no in-model mutable byte array.              */
/* ======================================================================== */

/* kept only as the type of the (now unused) StackFrame snapshot field */
struct MemByte = { offset : int, value : byte }

/* The high-water mark, in bytes: the largest offset ever touched in this
 * frame. MSIZE rounds it up to a word; memory-expansion gas is charged on
 * each growth of mem_size_words (Yellow Paper section 9.4.1, the C_mem term). */
register memory_size : int = 0

/* call-frame memory lifecycle (the C side owns the bytes) */
function mem_clear() -> unit = host_mem_reset()
function mem_frame_enter() -> unit = host_mem_push()
function mem_frame_leave() -> unit = host_mem_pop()

/* --- raw byte access (O(1), C-backed) --- */

/* frame liveness (defined in evm/machine.sail); forward-declared so the byte
 * primitives below can no-op when the frame has exceptionally halted. */
val is_running : unit -> bool

/* A memory access only follows a successful charge_expansion; if that charge ran
 * the frame out of gas (exc_halt sets the status but does not abort the opcode
 * handler), the access must NOT touch memory -- otherwise an out-of-gas write to
 * a huge offset (e.g. MSTORE at 2^32) would grow the C buffer to gigabytes. */
function mem_get_byte(off : int) -> byte =
    if is_running() then {
        host_mem_read(get_slice_int(64, off, 0))
    } else {
        0x00
    }

function mem_set_byte(off : int, v : byte) -> unit =
    if is_running() then {
        host_mem_write(get_slice_int(64, off, 0), v);
        if off + 1 > memory_size then {
            memory_size = off + 1
        }
    }

/* --- word access (big-endian, 32 bytes): MLOAD / MSTORE / MSTORE8 --- */

function mem_load(off : int) -> word = {
    var acc : word = word_of_int(0);
    foreach (k from 0 to 31) {
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(mem_get_byte(off + k), 256))
    };
    acc
}

function mem_store(off : int, w : word) -> unit = {
    foreach (k from 0 to 31) {
        let shift = 8 * (31 - k);
        let b : byte = sail_shiftright(w, shift)[7 .. 0];
        mem_set_byte(off + k, b)
    }
}

function mem_store_byte(off : int, w : word) -> unit =
    mem_set_byte(off, w[7 .. 0])

/* --- byte-range read (for KECCAK / RETURN / LOG / CALL args), as a list --- */

/* Read mem[off .. off+len) into a list, in address order. */
function mem_read_range(off : int, len : int) -> list(byte) = {
    var out : list(byte) = [||]; /* prepend from the high end so the list ends in address order */
    foreach (k from 0 to (len - 1)) {
        out = mem_get_byte(off + (len - 1 - k)) :: out
    };
    out
}

/* Current memory size in 32-byte words, rounded up (the MSIZE value, and the
 * unit of memory-expansion gas). */
function mem_size_words() -> int = ediv_int(memory_size + 31, 32)

/* MCOPY (EIP-5656): overlapping-safe memory-to-memory copy (one C memmove). */
function mem_mcopy(dst : int, src : int, len : int) -> unit =
    hm_move(get_slice_int(64, dst, 0), get_slice_int(64, src, 0), get_slice_int(64, len, 0))

/* CODECOPY: the current frame's code (already in the C buffer) into memory.
 * Clamp a beyond-2^64 source offset to a beyond-any-code sentinel (see
 * calldata_copy_to_mem): a past-end source must zero-fill the destination. */
function mem_codecopy(dst : int, off : int, len : int) -> unit = {
    let o = if off > 16777216 then { 16777216 } else { off };
    code_db_copy_frame_code_to_memory(get_slice_int(64, dst, 0), get_slice_int(64, o, 0), get_slice_int(64, len, 0))
}

/* KECCAK256 over the memory range [off, off+len). Semantically, the caller
 * reads the addressed bytes and hashes that byte list. */
function mem_keccak(off : int, len : int) -> word =
    bytes_keccak(memory_byte_slice(off, len))

function returndata_size() -> int = unsigned(returndata_current_len())
function returndata_capture(off : int, len : int) -> unit =
    returndata_capture_memory(get_slice_int(64, off, 0), get_slice_int(64, len, 0))
function returndata_copy_to_mem(dst : int, off : int, len : int) -> unit =
    returndata_copy_current_to_memory(get_slice_int(64, dst, 0), get_slice_int(64, off, 0), get_slice_int(64, len, 0))
function returndata_out_region(dst : int, want : int) -> unit =
    returndata_copy_current_prefix_to_memory(get_slice_int(64, dst, 0), get_slice_int(64, want, 0))

function returndata_pending_size() -> int = unsigned(returndata_pending_len())
function returndata_pending_byte(i : int) -> byte =
    returndata_pending_byte_at(get_slice_int(64, i, 0))

/* Materialize the pending child output (the init-code return value a CREATE
 * deploys as the new account's code) as a Sail list. */
function returndata_pending_bytes() -> list(byte) = {
    let n = returndata_pending_size();
    var out : list(byte) = [||];
    foreach (i from 0 to (n - 1)) {
        out = returndata_pending_byte(n - 1 - i) :: out
    };
    out
}

/* The transaction input, streamed into the C buffer ONCE per tx. Intrinsic-gas
 * inspection (EIP-2028 / EIP-7623 zero vs nonzero byte counting) and length
 * checks read it here, byte-addressed, rather than re-walking a Sail list. */
function txdata_byte(i : int) -> byte = txd_at(get_slice_int(64, i, 0))
function txdata_size() -> int = unsigned(txd_length())
function txdata_read_range(off : int, len : int) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to (len - 1)) {
        out = txdata_byte(off + (len - 1 - k)) :: out
    };
    out
}

function calldata_word(off : int) -> word = code_db_calldata_load_word(get_slice_int(64, off, 0))

/* clamp a beyond-2^64 source offset to a beyond-any-calldata sentinel so the
 * 64-bit truncation cannot wrap back into real calldata: a past-end source
 * offset must zero-fill the whole destination (EELS buffer_read). */
function calldata_copy_to_mem(dst : int, off : int, len : int) -> unit = {
    let o = if off > 16777216 then { 16777216 } else { off };
    cd_to_mem(get_slice_int(64, dst, 0), get_slice_int(64, o, 0), get_slice_int(64, len, 0))
}
function calldata_set_next(off : int, len : int) -> unit =
    cd_set(get_slice_int(64, off, 0), get_slice_int(64, len, 0))

/* Load the current transaction's input bytes into the owned C buffer and return
 * the staged length. */
function txdata_set_input(data : list(byte)) -> int = {
    txin_begin();
    var rest : list(byte) = data;
    var n : int = 0;
    var more : bool = true;
    while more do {
        match rest {
            b :: t => {
                txin_byte(b);
                n = n + 1;
                rest = t
            },
            [||] => more = false,
        }
    };
    n
}
