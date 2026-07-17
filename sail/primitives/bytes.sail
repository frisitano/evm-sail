
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

/*! An unmaterialized byte range: a region, an offset, and a length. A
    slice denotes the byte sequence it references; reads resolve in O(1)
    without copying. */
struct ByteSlice = {
    source : ByteSource,
    off    : byte_quantity,
    len    : byte_quantity,
}

let ADDRESS_BYTE_LENGTH : byte_length = ByteQuantity(20)
let WORD_BYTE_LENGTH : byte_length = ByteQuantity(32)
let EIGHT_BYTE_LENGTH : byte_length = ByteQuantity(8)
let DOUBLE_WORD_BYTE_LENGTH : byte_length = ByteQuantity(64)

/*! Constructs a [ByteSlice][type-ByteSlice] from integer offset and
    length. */
function byte_slice(src : ByteSource, off : source_pointer, len : byte_length) -> ByteSlice =
    struct { source = src, off = off, len = len }

/*! The zero-length placeholder slice, for unused value roles (subtree
    references, deletes). */
let EMPTY_SLICE : ByteSlice = struct { source = StatelessInputSource, off = BYTE_ZERO, len = BYTE_ZERO }

/*! A slice into the stateless input (witness) store. */
function stateless_input_byte_slice(off : source_pointer, len : byte_length) -> ByteSlice =
    byte_slice(StatelessInputSource, off, len)

/*! A relative sub-range of a slice, within the same source. */
function sub_slice(s : ByteSlice, off : source_pointer, len : byte_length) -> ByteSlice = {
    assert(off <= s.len);
    assert(len <= s.len - off);
    assert(s.off <= MAX_BYTE_QUANTITY);
    assert(off <= MAX_BYTE_QUANTITY - s.off);
    let absolute_off = s.off + off;
    byte_slice(s.source, absolute_off, len)
}

/*! An absolute span in the append-only content-addressed code arena. */
function code_byte_slice(off : source_pointer, len : byte_length) -> ByteSlice =
    byte_slice(CodeSource, off, len)

/* Materialized fragments carry their length at construction time. This keeps
 * every generic Bytes operation O(1) without rediscovering a linked-list
 * length. Bulk data remains source-backed through ByteSlice. */
/*! A finite list of bytes paired with its construction-time length. */
struct MaterializedBytes = {
    data : list(byte),
    len : byte_length,
}

function materialized_bytes(data : list(byte), len : byte_length) -> MaterializedBytes =
    struct { data = data, len = len }

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
}

function bytes_list(data : list(byte), len : byte_length) -> Bytes =
    BytesList(materialized_bytes(data, len))
