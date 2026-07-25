
/*md
# Regions and byte slices

Unmaterialized byte sequences. The specification's bulk data lives in
named **regions** of the host interface — the stateless input, frame
memory, code, log data, output, and scratch stores enumerated by
[ByteSource][type-ByteSource]. A [ByteSlice][type-ByteSlice] denotes a
byte range inside one region without copying it into Sail; a slice's
meaning is the byte sequence it references.

## Constants

The fixed lengths identify addresses, words, limbs, and double words;
`EMPTY_SLICE` is the canonical zero-length range.
*/
/*! The regions of the host interface a [ByteSlice][type-ByteSlice] may
    reference. `StatelessInputSource` is the SSZ stateless-input envelope;
    `EvmMemorySource` is the current frame's memory; `CodeSource` is stored
    contract code; `LogDataSource` retains log payloads for receipts;
    `OutputSource` is frame output frozen at halt; and `ScratchSource` is
    the executor's scratch arena. */
enum ByteSource = { StatelessInputSource, EvmMemorySource, CodeSource, LogDataSource, OutputSource, ScratchSource }

/*! The witness-carrying fields of an unmaterialized byte range. */
struct ByteSliceFields('off : Int, 'len : Int), source_valid_range('off, 'len) = {
    source : ByteSource,
    off    : int('off),
    len    : int('len),
}

/*! An unmaterialized byte range: a region, an offset, and a length. A
    slice denotes the byte sequence it references; reads resolve in O(1)
    without copying. */
type ByteSlice = {'off 'len, source_valid_range('off, 'len). ByteSliceFields('off, 'len)}

/*! A source-backed byte range whose length is known statically. */
type ByteSliceLength('required : Int) = {
    'off 'len,
    source_valid_range('off, 'len) & 'len == 'required.
    ByteSliceFields('off, 'len)
}

/*! A source-backed byte range whose validated length is at least `minimum`. */
type ByteSliceAtLeast('minimum : Int) = {
    'off 'len,
    source_valid_range('off, 'len) & 0 <= 'minimum & 'minimum <= 'len.
    ByteSliceFields('off, 'len)
}

/*! Returns the length carried by an existential byte slice. */
function byte_slice_length(s : ByteSlice) -> source_length =
    s.len

/*! The result of making a requested prefix of a byte region available.
    Success carries a slice of exactly the requested length. This shared type
    deliberately imposes no global region maximum: each region-specific
    producer may refine the admissible length independently. */
union ByteRegionResult('required : Int), 0 <= 'required = {
    ByteRegionReady : ByteSliceLength('required),
    ByteRegionFailed : unit,
}

let ADDRESS_BYTE_LENGTH : int(20) = 20
let WORD_BYTE_LENGTH : int(32) = 32
let EIGHT_BYTE_LENGTH : int(8) = 8
let DOUBLE_WORD_BYTE_LENGTH : int(64) = 64

/*! Constructs a [ByteSlice][type-ByteSlice] from integer offset and
    length. */
val byte_slice : forall 'off 'len, source_valid_range('off, 'len). (ByteSource, int('off), int('len)) -> ByteSliceFields(
        'off,
        'len,
    )
function byte_slice(src, off, len) = {
    let fields : ByteSliceFields('off, 'len) = struct { source = src, off = off, len = len };
    fields
}

/*! The zero-length placeholder slice, for unused value roles (subtree
    references, deletes). */
let EMPTY_SLICE : ByteSliceFields(0, 0) = byte_slice(StatelessInputSource, 0, 0)

/*! A relative sub-range of a slice, within the same source. */
val sub_slice : forall 'base 'source_len 'off 'len,
                  source_valid_range('base, 'source_len) & 0 <= 'off & 0 <= 'len & 'off + 'len <= 'source_len. (
        ByteSliceFields('base, 'source_len),
        int('off),
        int('len),
    ) -> ByteSliceFields('base + 'off, 'len)
function sub_slice(s, off, len) = {
    byte_slice(s.source, s.off + off, len)
}

/*! The suffix of a slice beginning at a contained relative offset. */
val slice_suffix : forall 'base 'source_len 'relative,
                     source_valid_range('base, 'source_len) & 0 <= 'relative & 'relative <= 'source_len. (
        ByteSliceFields('base, 'source_len),
        int('relative),
    ) -> ByteSliceFields('base + 'relative, 'source_len - 'relative)
function slice_suffix(s, off) = {
    byte_slice(s.source, s.off + off, s.len - off)
}

/* Materialized fragments carry their length at construction time. This keeps
 * every generic Bytes operation O(1) without rediscovering a linked-list
 * length. Bulk data remains source-backed through ByteSlice. */
/*! A finite list of bytes paired with its construction-time length. */
struct MaterializedBytes = {
    data : list(byte),
    len : source_length,
}

val materialized_bytes : forall 'len, source_valid_length('len). (list(byte), int('len)) -> MaterializedBytes
function materialized_bytes(data, len) =
    struct { data = data, len = len }

/*! A prefix of a fixed 32-byte vector, in wire order. */
struct FixedBytes32 = {
    data : b256,
    len : range(0, 32),
}

/*! A byte-sequence segment: materialized bytes or a region-backed span.
    A `list(Bytes)` denotes the concatenation of its segments.

    !!! note "Implementation"
        A segment list crosses the host interface whole
        (`ffi/hash_glue.c`); no per-byte crossings occur. */
union Bytes = {
    /* materialized bytes */
    BytesList : MaterializedBytes,
    /* a region-backed reference */
    BytesSlice : ByteSlice,
    /* a prefix of an inline fixed-width vector */
    BytesFixed32 : FixedBytes32,
}

val bytes_list : forall 'len, source_valid_length('len). (list(byte), int('len)) -> Bytes
function bytes_list(data, len) =
    BytesList(materialized_bytes(data, len))

function bytes_fixed32(data : b256, len : range(0, 32)) -> Bytes =
    BytesFixed32(struct { data = data, len = len })
