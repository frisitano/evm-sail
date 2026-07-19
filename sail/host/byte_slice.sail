
/* The standard private input, exposed once as a source-backed slice. The C
 * adapter only binds read_input's pointer/length pair to this generated Sail
 * aggregate; every subsequent access uses the generic slice operations. */
/*md
# Byte-slice access

[ByteSlice][type-ByteSlice] is the common read-only view used by calldata
and executable frame code alike. Transaction input is already a
source-backed slice; no parallel input buffer or per-frame source
descriptor exists. Proof targets see the `val`s as bodyless axioms over
abstract byte lists.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/
/*! The complete private input supplied to the stateless validator. */
val stateless_input = impure { c: "stateless_input" } : unit -> ByteSlice

/*! The byte at a slice offset, zero past the end; the axiom behind
    [slice_byte][]. */
val host_slice_byte = impure { c: "slice_byte_at" } : (ByteSlice, byte_quantity) -> bits(8)
/*! The number of nonzero bytes in a slice; the axiom behind
    [slice_count_nonzero][]. */
val host_slice_count_nonzero = impure { c: "slice_count_nonzero" } : ByteSlice -> byte_quantity
/*! Whether `count` regions of `width` bytes, `stride` apart from
    `start`, contain only zero bytes; the axiom behind
    [slice_strided_zero][]. */
val host_slice_strided_zero = impure { c: "slice_strided_zero" } : (
        ByteSlice,
        byte_quantity,
        byte_quantity,
        byte_quantity,
        byte_quantity,
    ) -> bool
/*! The 32-byte big-endian word at a slice offset, zero-padded past the
    end; the axiom behind [slice_load][]. */
val host_slice_load_word = impure { c: "slice_load_word" } : (ByteSlice, byte_quantity) -> word
/*! An `n`-byte big-endian word at a slice offset, right-aligned and
    zero-padded; the axiom behind [slice_load_n][]. */
val host_slice_load_n_word = impure { c: "slice_load_n_word" } : (ByteSlice, byte_quantity, byte_quantity) -> word
/*! Copies a slice range into frame memory, zero-filling past the end;
    the axiom behind [slice_copy][]. */
val host_slice_copy_to_memory = impure { c: "slice_copy_to_memory" } : (
        ByteSlice,
        byte_quantity,
        byte_quantity,
        byte_quantity,
    ) -> unit
/*! Whether the concatenation of the `Bytes` segments equals the slice
    byte for byte, without materializing either side. */
val bytes_segments_equal_slice = impure { c: "host_bytes_segments_equal_slice" } : (list(Bytes), ByteSlice) -> bool

/*! The byte at slice offset `off`; zero past the end. */
function slice_byte(s : ByteSlice, off : source_pointer) -> byte =
    if off < s.len then {
        host_slice_byte(s, off)
    } else {
        0x00
    }

/*! The number of nonzero bytes in the slice (EIP-2028 / EIP-7623
    calldata gas). */
function slice_count_nonzero(s : ByteSlice) -> byte_length =
    host_slice_count_nonzero(s)

/*! Whether `count` fixed-width regions in a regular source layout contain
    only zero bytes — a bulk slice predicate, not a format-specific
    rule. */
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

/*! The 32-byte word at slice offset `off`, zero-padded past the end. */
function slice_load(s : ByteSlice, off : source_pointer) -> word =
    if off < s.len then {
        host_slice_load_word(s, off)
    } else {
        ZERO_WORD
    }

/*! The word at a transaction-controlled 256-bit source offset, returning zero
    when the offset cannot designate a byte in the slice. */
function slice_load_word_offset(s : ByteSlice, off : word) -> word = {
    match word_to_limb(off) {
        Some(offset_bits) => {
            let offset = unsigned(offset_bits);
            let ByteQuantity(slice_len) = s.len;
            if offset < slice_len then {
                slice_load(s, ByteQuantity(offset))
            } else {
                ZERO_WORD
            }
        },
        None() => ZERO_WORD,
    }
}

/*! An `n`-byte big-endian word at slice offset `off`, right-aligned and
    zero-padded past the end; used by `PUSH0`–`PUSH32`. */
function slice_load_n(s : ByteSlice, off : source_pointer, n : byte_length) -> word =
    if off < s.len then {
        host_slice_load_n_word(s, off, n)
    } else {
        ZERO_WORD
    }

/*! Copies slice range `[off, off+len)` into frame memory at `dst`,
    zero-filling past the end. */
function slice_copy(s : ByteSlice, dst : memory_pointer, off : source_pointer, len : memory_length) -> unit =
    if len != BYTE_ZERO then {
        if off < s.len then {
            host_slice_copy_to_memory(s, dst, off, len)
        } else {
            host_slice_copy_to_memory(EMPTY_SLICE, dst, BYTE_ZERO, len)
        }
    }

/* buffer_read semantics for a transaction-controlled source offset. */
/*! Copies from a transaction-controlled 256-bit source offset into EVM
    memory, applying the source operation's empty-read and zero-padding rules. */
function slice_copy_word_offset(s : ByteSlice, dst : memory_pointer, off : word, len : memory_length) -> unit = {
    match word_to_limb(off) {
        Some(offset_bits) => {
            let offset = unsigned(offset_bits);
            let ByteQuantity(slice_len) = s.len;
            if offset < slice_len then {
                slice_copy(s, dst, ByteQuantity(offset), len)
            } else {
                slice_copy(EMPTY_SLICE, dst, BYTE_ZERO, len)
            }
        },
        None() => slice_copy(EMPTY_SLICE, dst, BYTE_ZERO, len),
    }
}
