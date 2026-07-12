/* ======================================================================== */
/* CALLDATA + TRANSACTION INPUT                                             */
/*                                                                          */
/* Two byte sources the evm layer reads but never mutates, both held as     */
/* REFERENCES. The TX INPUT (ffi/memory.c): the transaction's data span     */
/* (Transaction.input_src, usually the witness), BOUND once per tx; block   */
/* system calls STAGE their 32-byte word instead (the one value with no     */
/* backing span). CALLDATA (I_d) is a Sail-side ByteSlice -- the register   */
/* lives in evm/machine.sail; the readers here take the slice as an         */
/* argument and resolve it generically.                                     */
/*                                                                          */
/* Naming: `bind` = set a reference, `stage` = copy a value.                */
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

/* ---- generic slice reads (serve the calldata register) ----------------- */

val slice_load_word = impure { c: "slice_load_word" } : (bits(64), bits(64), bits(64), bits(64)) -> word
val slice_copy_to_memory = impure { c: "slice_copy_to_memory" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
val mem_establish_absolute = impure { c: "mem_establish_absolute" } : (bits(64), bits(64)) -> bits(64)

/* mint a slice of THIS frame's memory [off, off+len): the range is
 * established (it is already gas-charged) and referenced by ABSOLUTE arena
 * offset -- the frame is about to suspend, so the bytes are frozen for the
 * reader. EMPTY_SLICE on overflow/OOM (fail-closed). */
function mem_slice(off : int, len : int) -> ByteSlice =
    if len == 0 then {
        EMPTY_SLICE
    } else {
        let abs = mem_establish_absolute(get_slice_int(64, off, 0), get_slice_int(64, len, 0));
        if abs == 0xFFFFFFFFFFFFFFFF then {
            EMPTY_SLICE
        } else {
            struct { source = MemoryArenaSource, off = abs, len = get_slice_int(64, len, 0) }
        }
    }

/* CALLDATALOAD: the 32-byte word of slice `s` at offset `off`, zero-padded
 * past the end. The offset passes through clamp_source_offset
 * (host/memory.sail) so 64-bit truncation cannot wrap into real data. */
function calldata_load(s : ByteSlice, off : int) -> word =
    slice_load_word(byte_source_kind(s.source), s.off, s.len, get_slice_int(64, clamp_source_offset(off), 0))

/* CALLDATACOPY: slice `s` [off, off+len) into memory at dst, zero-filling
 * past the end (EELS buffer_read); same clamp as calldata_load. */
function calldata_copy(s : ByteSlice, dst : int, off : int, len : int) -> unit =
    slice_copy_to_memory(
        byte_source_kind(s.source),
        s.off,
        s.len,
        get_slice_int(64, dst, 0),
        get_slice_int(64, clamp_source_offset(off), 0),
        get_slice_int(64, len, 0),
    )
