/* ======================================================================== */
/* PRIMITIVES: BYTE SOURCES                                                 */
/* ======================================================================== */

/* A ByteSlice denotes a byte sequence inside one of the host's C-backed
 * stores without materializing it. The complete value crosses the FFI; the
 * generated-type glue resolves its ByteSource without exposing a parallel
 * numeric source tag to Sail. */
enum ByteSource = {
    StatelessInputSource,
    MemorySource,
    CodeSource,
    LogDataSource,
    MemoryArenaSource,
    OutputSource,
    ScratchSource,
}

struct ByteSlice = {
    source : ByteSource,
    off    : bits(64),
    len    : bits(64),
}

function byte_slice(src : ByteSource, off : int, len : int) -> ByteSlice =
    struct { source = src, off = get_slice_int(64, off, 0), len = get_slice_int(64, len, 0) }

/* zero-length placeholder for unused value roles (subtree refs, deletes) */
let EMPTY_SLICE : ByteSlice = struct {
        source = StatelessInputSource,
        off = 0x0000000000000000,
        len = 0x0000000000000000,
    }

function stateless_input_byte_slice(off : int, len : int) -> ByteSlice =
    byte_slice(StatelessInputSource, off, len)

/* a relative sub-range of a slice (same source) */
function sub_slice(s : ByteSlice, off : int, len : int) -> ByteSlice =
    byte_slice(s.source, unsigned(s.off) + off, len)

function memory_byte_slice(off : int, len : int) -> ByteSlice =
    byte_slice(MemorySource, off, len)

/* Absolute span in the append-only content-addressed code arena. */
function code_byte_slice(off : int, len : int) -> ByteSlice =
    byte_slice(CodeSource, off, len)

/* A byte-sequence segment: materialized Sail bytes or a C-backed span. A
 * list(Bytes) denotes the concatenation of its segments and crosses the FFI
 * whole (ffi/hash_glue.c). */
union Bytes = {
    BytesList : list(byte),
    BytesSlice : ByteSlice,
}
