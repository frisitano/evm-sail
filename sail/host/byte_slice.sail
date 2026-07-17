/* ======================================================================== */
/* BYTE-SLICE ACCESS                                                        */
/*                                                                          */
/* ByteSlice is the common read-only view used by calldata AND executable   */
/* frame code. Transaction input is already a source-backed ByteSlice; no   */
/* parallel input buffer or per-frame source descriptor exists.             */
/* Proof targets see the vals as bodyless axioms over abstract byte lists.  */
/* ======================================================================== */

/* ---- generic slice reads (serve calldata and executable code) ---------- */

/* The standard private input, exposed once as a source-backed slice. The C
 * adapter only binds read_input's pointer/length pair to this generated Sail
 * aggregate; every subsequent access uses the generic slice operations. */
val stateless_input = impure { c: "stateless_input" } : unit -> ByteSlice

val host_slice_byte = impure { c: "slice_byte_at" } : (ByteSlice, byte_quantity) -> bits(8)
val host_slice_count_nonzero = impure { c: "slice_count_nonzero" } : ByteSlice -> byte_quantity
val host_slice_strided_zero = impure { c: "slice_strided_zero" } : (
        ByteSlice,
        byte_quantity,
        byte_quantity,
        byte_quantity,
        byte_quantity,
    ) -> bool
val host_slice_load_word = impure { c: "slice_load_word" } : (ByteSlice, byte_quantity) -> word
val host_slice_load_n_word = impure { c: "slice_load_n_word" } : (ByteSlice, byte_quantity, byte_quantity) -> word
val host_slice_copy_to_memory = impure { c: "slice_copy_to_memory" } : (
        ByteSlice,
        byte_quantity,
        byte_quantity,
        byte_quantity,
    ) -> unit
val bytes_segments_equal_slice = impure { c: "host_bytes_segments_equal_slice" } : (list(Bytes), ByteSlice) -> bool

/* The byte at slice offset `off`, zero past the end. */
function slice_byte(s : ByteSlice, off : source_pointer) -> byte =
    if off < s.len then {
        host_slice_byte(s, off)
    } else {
        0x00
    }

/* Number of nonzero bytes in the slice (EIP-2028 / EIP-7623 calldata gas). */
function slice_count_nonzero(s : ByteSlice) -> byte_length =
    host_slice_count_nonzero(s)

/* Whether `count` fixed-width regions in a regular source layout contain only
 * zero bytes. This is a bulk ByteSlice predicate, not a format-specific rule. */
function slice_strided_zero(
    s : ByteSlice,
    start : source_pointer,
    stride : byte_length,
    width : byte_length,
    count : byte_length,
) -> (
    bool
) =
    host_slice_strided_zero(s, start, stride, width, count)

/* The 32-byte word at slice offset `off`, zero-padded past the end. */
function slice_load(s : ByteSlice, off : source_pointer) -> word =
    if off < s.len then {
        host_slice_load_word(s, off)
    } else {
        ZERO_WORD
    }

/* EVM source offsets are exact 256-bit quantities. An offset beyond the
 * source reads zero; only an in-range offset reaches the source operation. */
function slice_load_word_offset(s : ByteSlice, off : word) -> word = {
    let offset = unsigned(off);
    let ByteQuantity(slice_len) = s.len;
    if offset < slice_len then {
        slice_load(s, ByteQuantity(offset))
    } else {
        ZERO_WORD
    }
}

/* An n-byte big-endian word at slice offset `off`, right-aligned and
 * zero-padded past the end. Used by PUSH0..PUSH32. */
function slice_load_n(s : ByteSlice, off : source_pointer, n : byte_length) -> word =
    if off < s.len then {
        host_slice_load_n_word(s, off, n)
    } else {
        ZERO_WORD
    }

/* slice `s` [off, off+len) into current memory at dst, zero-filling past
 * the end (EELS buffer_read). */
function slice_copy(s : ByteSlice, dst : memory_pointer, off : source_pointer, len : memory_length) -> unit =
    if len != BYTE_ZERO then {
        if off < s.len then {
            host_slice_copy_to_memory(s, dst, off, len)
        } else {
            host_slice_copy_to_memory(EMPTY_SLICE, dst, BYTE_ZERO, len)
        }
    }

/* buffer_read semantics for a transaction-controlled source offset. */
function slice_copy_word_offset(s : ByteSlice, dst : memory_pointer, off : word, len : memory_length) -> unit = {
    let offset = unsigned(off);
    let ByteQuantity(slice_len) = s.len;
    if offset < slice_len then {
        slice_copy(s, dst, ByteQuantity(offset), len)
    } else {
        slice_copy(EMPTY_SLICE, dst, BYTE_ZERO, len)
    }
}
