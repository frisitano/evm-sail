/* ======================================================================== */
/* PRIMITIVES: RLP FIELD                                                    */
/*                                                                          */
/* A decoded field that keeps offsets into its original byte source.        */
/* Pure data -- no registers, no externs.                                   */
/* ======================================================================== */

struct RlpFieldRef = {
    source      : ByteSlice,
    is_list     : bool,
    full_off    : source_pointer,
    full_len    : byte_length,
    content_off : source_pointer,
    content_len : byte_length,
}

/* A one-pass view over the immediate children of an RLP list. */
struct RlpCursor = {
    source : ByteSlice,
    current : source_pointer,
    stop : source_pointer,
    valid : bool,
}
