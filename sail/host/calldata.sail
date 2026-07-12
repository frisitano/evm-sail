/* ======================================================================== */
/* CALLDATA + TRANSACTION INPUT                                             */
/*                                                                          */
/* Two byte sources the evm layer reads but never mutates, both held as     */
/* REFERENCES (ffi/memory.c) -- nothing on this path is copied.             */
/*                                                                          */
/* TX INPUT: the transaction's data span (Transaction.input_src, usually    */
/* the witness), BOUND once per tx; block system calls STAGE their 32-byte  */
/* word instead (the one value that has no backing span). Intrinsic gas,    */
/* create-tx initcode, and tx-level precompiles read it byte-addressed.     */
/*                                                                          */
/* CALLDATA (I_d): the current frame's input, a per-frame slice bound at    */
/* call entry to a suspended caller-memory range, the empty string, or the  */
/* whole tx input; served to CALLDATALOAD / CALLDATASIZE / CALLDATACOPY.    */
/*                                                                          */
/* Naming: `bind` = set a reference, `stage` = copy a value; the extern     */
/* vals carry the C-level names, the functions the model-facing ones.       */
/* Proof targets see the vals as bodyless axioms over abstract byte lists.  */
/* ======================================================================== */

/* ---- the transaction input (bound once per tx) ------------------------- */

val txdata_bind_source = impure { c: "txdata_bind_source" } : (bits(64), bits(64), bits(64)) -> bits(64)
val txdata_stage_word = impure { c: "txdata_stage_word" } : word -> bits(64)
val txdata_byte_at = impure { c: "txdata_byte_at" } : bits(64) -> bits(8)
val txdata_length = impure { c: "txdata_length" } : unit -> bits(64)

/* EIP-2028 / EIP-7623 calldata gas: nonzero input bytes, counted C-side */
val txdata_count_nonzero = impure { c: "txdata_count_nonzero" } : unit -> bits(64)

/* bind the tx input to a byte source; returns the bound length
 * (fail-closed 0 on a bad source) */
function txdata_bind(s : ByteSlice) -> int =
    unsigned(txdata_bind_source(byte_source_kind(s.source), s.off, s.len))

/* stage a block system call's 32-byte input word */
function txdata_stage(w : word) -> unit = {
    let _ = txdata_stage_word(w)
}

/* byte i of the bound input (0x00 past the end) */
function txdata_byte(i : int) -> byte = txdata_byte_at(get_slice_int(64, i, 0))

/* the bound input's length in bytes */
function txdata_len() -> int = unsigned(txdata_length())

/* ---- the current frame's calldata (bound per call) --------------------- */

val calldata_bind_memory = impure { c: "calldata_bind_memory" } : (bits(64), bits(64)) -> unit
val calldata_bind_empty = impure { c: "calldata_bind_empty" } : unit -> unit
val calldata_bind_tx_input = impure { c: "calldata_bind_tx_input" } : unit -> unit
val calldata_load_word = impure { c: "calldata_load_word" } : bits(64) -> word
val calldata_copy_to_memory = impure { c: "calldata_copy_to_memory" } : (bits(64), bits(64), bits(64)) -> unit

/* bind the NEXT frame's calldata to a range of the (about-to-suspend)
 * caller's memory: staged pending, adopted at mem_frame_enter */
function calldata_bind_next(off : int, len : int) -> unit =
    calldata_bind_memory(get_slice_int(64, off, 0), get_slice_int(64, len, 0))

/* CALLDATALOAD: the 32-byte word at offset `off`. A past-end read is all
 * zeros; the offset passes through clamp_source_offset (host/memory.sail)
 * so 64-bit truncation cannot wrap a huge offset back into real data. */
function calldata_load(off : int) -> word =
    calldata_load_word(get_slice_int(64, clamp_source_offset(off), 0))

/* CALLDATACOPY: calldata[off, off+len) into memory at dst, zero-filling
 * past the end (EELS buffer_read); same clamp as calldata_load. */
function calldata_copy(dst : int, off : int, len : int) -> unit =
    calldata_copy_to_memory(
        get_slice_int(64, dst, 0),
        get_slice_int(64, clamp_source_offset(off), 0),
        get_slice_int(64, len, 0),
    )
