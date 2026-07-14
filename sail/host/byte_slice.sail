/*md
# Byte-slice access

[ByteSlice][type-ByteSlice] is the common read-only view used by calldata
and executable frame code alike. Transaction input is already a
source-backed slice; no parallel input buffer or per-frame source
descriptor exists. Proof targets see the `val`s as bodyless axioms over
abstract byte lists.
*/

val host_slice_byte = impure { c: "slice_byte_at" } : (ByteSlice, bits(64)) -> bits(8)
val host_slice_count_nonzero = impure { c: "slice_count_nonzero" } : ByteSlice -> bits(64)
val host_slice_strided_zero = impure { c: "slice_strided_zero" } : (ByteSlice, bits(64), bits(64), bits(64), bits(64)) -> bool
val host_slice_load_word = impure { c: "slice_load_word" } : (ByteSlice, bits(64)) -> word
val host_slice_load_n_word = impure { c: "slice_load_n_word" } : (ByteSlice, bits(64), bits(64)) -> word
val host_slice_copy_to_memory = impure { c: "slice_copy_to_memory" } : (ByteSlice, bits(64), bits(64), bits(64)) -> unit
val bytes_segments_equal_slice = impure { c: "host_bytes_segments_equal_slice" } : (list(Bytes), ByteSlice) -> bool

let SOURCE_PAST_END : int = 16777216

/*! Clamps a source offset so astronomically large (attacker-controlled)
    offsets stay in the 64-bit ABI; every in-range read past the end is
    zero anyway. */
function clamp_source_offset(off : int) -> int =
    if off > SOURCE_PAST_END then { SOURCE_PAST_END } else { off }

/*! The byte at slice offset `off`; zero past the end. */
function slice_byte(s : ByteSlice, off : int) -> byte =
    host_slice_byte(s, get_slice_int(64, clamp_source_offset(off), 0))

/*! The number of nonzero bytes in the slice (EIP-2028 / EIP-7623
    calldata gas). */
function slice_count_nonzero(s : ByteSlice) -> int =
    unsigned(host_slice_count_nonzero(s))

/*! Whether `count` fixed-width regions in a regular source layout contain
    only zero bytes — a bulk slice predicate, not a format-specific
    rule. */
function slice_strided_zero(s : ByteSlice, start : int, stride : int, width : int, count : int) -> bool =
    host_slice_strided_zero(
        s,
        get_slice_int(64, start, 0),
        get_slice_int(64, stride, 0),
        get_slice_int(64, width, 0),
        get_slice_int(64, count, 0),
    )

/*! Materializes a slice as a byte list (O(len)); the semantic meaning of
    every slice-based axiom. */
function read_byte_slice(s : ByteSlice) -> list(byte) = {
    let len = unsigned(s.len);
    var out : list(byte) = [||];
    var i : int = 0;
    while i < len do {
        out = slice_byte(s, i) :: out;
        i = i + 1
    };
    list_reverse(out)
}

/*! The length of a `Bytes` segment. */
function bytes_len(data : Bytes) -> int =
    match data {
        BytesList(bs) => list_len(bs),
        BytesSlice(s) => unsigned(s.len),
    }

/*! The first byte of a segment; `0x00` when empty. */
function bytes_first(data : Bytes) -> byte =
    match data {
        BytesList(bs) => match bs {
            byte :: _ => byte,
            [||] => 0x00,
        },
        BytesSlice(s) => slice_byte(s, 0),
    }

/*! Materializes a segment as a byte list. */
function bytes_to_list(data : Bytes) -> list(byte) =
    match data {
        BytesList(bs) => bs,
        BytesSlice(s) => read_byte_slice(s),
    }

/*! The byte at offset `off` of a segment. */
function bytes_at(data : Bytes, off : int) -> byte =
    match data {
        BytesList(bs) => bytes_get(bs, off),
        BytesSlice(s) => slice_byte(s, off),
    }

/*! The sub-range `[off, off+len)` of a segment, staying source-backed
    where possible. */
function bytes_range(data : Bytes, off : int, len : int) -> Bytes =
    match data {
        BytesList(bs) => BytesList(bytes_slice(bs, off, len)),
        BytesSlice(s) => BytesSlice(sub_slice(s, off, len)),
    }

/*! The 32-byte word at slice offset `off`, zero-padded past the end. */
function slice_load(s : ByteSlice, off : int) -> word =
    host_slice_load_word(s, get_slice_int(64, clamp_source_offset(off), 0))

/*! An `n`-byte big-endian word at slice offset `off`, right-aligned and
    zero-padded past the end; used by `PUSH0`–`PUSH32`. */
function slice_load_n(s : ByteSlice, off : int, n : int) -> word =
    host_slice_load_n_word(s, get_slice_int(64, clamp_source_offset(off), 0), get_slice_int(64, n, 0))

/*! Copies slice range `[off, off+len)` into frame memory at `dst`,
    zero-filling past the end (EELS `buffer_read`). */
function slice_copy(s : ByteSlice, dst : int, off : int, len : int) -> unit =
    host_slice_copy_to_memory(
        s,
        get_slice_int(64, dst, 0),
        get_slice_int(64, clamp_source_offset(off), 0),
        get_slice_int(64, len, 0),
    )
