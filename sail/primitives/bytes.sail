/*md
# Byte sources

Unmaterialized byte sequences. A [ByteSlice][type-ByteSlice] denotes a byte
range inside one of the host's C-backed stores without copying it into
Sail; the complete value crosses the FFI, and the generated-type glue
resolves its [ByteSource][type-ByteSource] without exposing a parallel
numeric source tag to Sail.
*/

/*! The backing stores a [ByteSlice][type-ByteSlice] may reference. */
enum ByteSource = {
    StatelessInputSource,
    MemorySource,
    CodeSource,
    LogDataSource,
    MemoryArenaSource,
    OutputSource,
    ScratchSource,
}

/*! An unmaterialized byte range: a backing store, an offset, and a
    length. A slice denotes the byte sequence it references; reads resolve
    in O(1) without copying. */
struct ByteSlice = {
    source : ByteSource,
    off    : bits(64),
    len    : bits(64),
}

/*! Constructs a [ByteSlice][type-ByteSlice] from integer offset and
    length. */
function byte_slice(src : ByteSource, off : int, len : int) -> ByteSlice =
    struct { source = src, off = get_slice_int(64, off, 0), len = get_slice_int(64, len, 0) }

/*! The zero-length placeholder slice, for unused value roles (subtree
    references, deletes). */
let EMPTY_SLICE : ByteSlice = struct {
        source = StatelessInputSource,
        off = 0x0000000000000000,
        len = 0x0000000000000000,
    }

/*! A slice into the stateless input (witness) store. */
function stateless_input_byte_slice(off : int, len : int) -> ByteSlice =
    byte_slice(StatelessInputSource, off, len)

/*! A relative sub-range of a slice, within the same source. */
function sub_slice(s : ByteSlice, off : int, len : int) -> ByteSlice =
    byte_slice(s.source, unsigned(s.off) + off, len)

/*! A slice into frame memory. */
function memory_byte_slice(off : int, len : int) -> ByteSlice =
    byte_slice(MemorySource, off, len)

/*! An absolute span in the append-only content-addressed code arena. */
function code_byte_slice(off : int, len : int) -> ByteSlice =
    byte_slice(CodeSource, off, len)

/*! A byte-sequence segment: materialized bytes or a store-backed span. A
    `list(Bytes)` denotes the concatenation of its segments.

    !!! note "Implementation"
        A segment list crosses the FFI whole (`ffi/hash_glue.c`); no
        per-byte crossings occur. */
union Bytes = {
    BytesList : list(byte),
    BytesSlice : ByteSlice,
}
