/* ======================================================================== */
/* PRIMITIVES: BYTE SOURCES                                                 */
/* ======================================================================== */

/* A ByteSlice denotes a byte sequence inside one of the host's C-backed
 * stores without materializing it. The complete value crosses the FFI; the
 * generated-type glue resolves its ByteSource without exposing a parallel
 * numeric source tag to Sail. */
enum ByteSource = { StatelessInputSource, EvmMemorySource, CodeSource, LogDataSource, OutputSource, ScratchSource }

struct ByteSlice = {
    source : ByteSource,
    off    : byte_quantity,
    len    : byte_quantity,
}

let ADDRESS_BYTE_LENGTH : byte_length = ByteQuantity(20)
let WORD_BYTE_LENGTH : byte_length = ByteQuantity(32)
let EIGHT_BYTE_LENGTH : byte_length = ByteQuantity(8)
let DOUBLE_WORD_BYTE_LENGTH : byte_length = ByteQuantity(64)

function byte_slice(src : ByteSource, off : source_pointer, len : byte_length) -> ByteSlice =
    struct { source = src, off = off, len = len }

/* zero-length placeholder for unused value roles (subtree refs, deletes) */
let EMPTY_SLICE : ByteSlice = struct { source = StatelessInputSource, off = BYTE_ZERO, len = BYTE_ZERO }

function stateless_input_byte_slice(off : source_pointer, len : byte_length) -> ByteSlice =
    byte_slice(StatelessInputSource, off, len)

/* a relative sub-range of a slice (same source) */
function sub_slice(s : ByteSlice, off : source_pointer, len : byte_length) -> ByteSlice = {
    assert(off <= s.len);
    assert(len <= s.len - off);
    assert(s.off <= MAX_BYTE_QUANTITY);
    assert(off <= MAX_BYTE_QUANTITY - s.off);
    let absolute_off = s.off + off;
    byte_slice(s.source, absolute_off, len)
}

/* Absolute span in the append-only content-addressed code arena. */
function code_byte_slice(off : source_pointer, len : byte_length) -> ByteSlice =
    byte_slice(CodeSource, off, len)

/* Materialized fragments carry their length at construction time. This keeps
 * every generic Bytes operation O(1) without rediscovering a linked-list
 * length. Bulk data remains source-backed through ByteSlice. */
struct MaterializedBytes = {
    data : list(byte),
    len : byte_length,
}

function materialized_bytes(data : list(byte), len : byte_length) -> MaterializedBytes =
    struct { data = data, len = len }

/* A byte-sequence segment: materialized Sail bytes or a C-backed span. A
 * list(Bytes) denotes the concatenation of its segments and crosses the FFI
 * whole (ffi/hash_glue.c). */
union Bytes = {
    BytesList : MaterializedBytes,
    BytesSlice : ByteSlice,
}

function bytes_list(data : list(byte), len : byte_length) -> Bytes =
    BytesList(materialized_bytes(data, len))
