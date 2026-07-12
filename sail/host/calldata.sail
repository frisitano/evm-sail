/* ======================================================================== */
/* CALLDATA + TRANSACTION INPUT                                             */
/*                                                                          */
/* Two byte sources the evm layer reads but never mutates, both held as     */
/* REFERENCES (ffi/memory.c). CALLDATA (I_d): the current frame's input, a  */
/* per-frame descriptor bound at call entry to a suspended caller-memory    */
/* range, the empty string, or the whole tx input; served to CALLDATALOAD   */
/* / SIZE / COPY. The TX INPUT: the transaction's data span                 */
/* (Transaction.input_src, usually the witness), bound once per tx; block   */
/* system calls stage their 32-byte word instead. Proof targets see         */
/* bodyless axioms over abstract byte-list models.                          */
/* ======================================================================== */

/* ---- the transaction input (bound once per tx) ------------------------- */

val txdata_bind_source = impure { c: "txdata_bind_source" } : (bits(64), bits(64), bits(64)) -> bits(64)
val txdata_stage_word = impure { c: "txdata_stage_word" } : word -> bits(64)
val txdata_byte_at = impure { c: "txdata_byte_at" } : bits(64) -> bits(8)
val txdata_length = impure { c: "txdata_length" } : unit -> bits(64)

/* EIP-2028 / EIP-7623 calldata gas: nonzero input bytes, counted C-side */
val txdata_count_nonzero = impure { c: "txdata_count_nonzero" } : unit -> bits(64)

/* bind the tx input by reference; returns the bound length (fail-closed 0) */
function txdata_set_source(s : ByteSlice) -> int =
    unsigned(txdata_bind_source(byte_source_kind(s.source), s.off, s.len))

/* stage a block system call's 32-byte input word */
function txdata_set_word(w : word) -> unit = {
    let _ = txdata_stage_word(w)
}

function txdata_byte(i : int) -> byte = txdata_byte_at(get_slice_int(64, i, 0))

function txdata_size() -> int = unsigned(txdata_length())

/* ---- the current frame's calldata (bound per call) --------------------- */

val calldata_bind_memory = impure { c: "calldata_bind_memory" } : (bits(64), bits(64)) -> unit
val calldata_bind_empty = impure { c: "calldata_bind_empty" } : unit -> unit
val calldata_bind_tx_input = impure { c: "calldata_bind_tx_input" } : unit -> unit

/* CALLDATALOAD: the 32-byte word at calldata offset i (zero-padded past end) */
val calldata_load_word = impure { c: "calldata_load_word" } : bits(64) -> word
val calldata_copy_to_memory = impure { c: "calldata_copy_to_memory" } : (bits(64), bits(64), bits(64)) -> unit

/* bind the NEXT frame's calldata to a range of the (about-to-suspend)
 * caller's memory: staged pending, adopted at mem_frame_enter */
function calldata_bind_next(off : int, len : int) -> unit =
    calldata_bind_memory(get_slice_int(64, off, 0), get_slice_int(64, len, 0))

/* A past-end CALLDATALOAD reads all zeros; a past-end copy source zero-fills
 * the whole destination (EELS buffer_read). Source offsets pass through
 * clamp_source_offset (host/memory.sail) so the 64-bit truncation cannot
 * wrap a huge offset back into real data. */
function calldata_word(off : int) -> word =
    calldata_load_word(get_slice_int(64, clamp_source_offset(off), 0))

function calldata_copy_to_mem(dst : int, off : int, len : int) -> unit =
    calldata_copy_to_memory(
        get_slice_int(64, dst, 0),
        get_slice_int(64, clamp_source_offset(off), 0),
        get_slice_int(64, len, 0),
    )
