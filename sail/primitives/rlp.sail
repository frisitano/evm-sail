
/*md
# RLP field references

Decoded RLP fields that keep offsets into their original byte source
(YP Appendix B). Pure data — no registers, no externs.
*/
/*! A decoded RLP field: its source, list/string tag, and the offsets and
    lengths of both the full encoding and its content. */
struct RlpFieldRef = {
    source      : ByteSlice,
    is_list     : bool,
    full_off    : source_pointer,
    full_len    : byte_length,
    content_off : source_pointer,
    content_len : byte_length,
}

/*! A one-pass view over the immediate children of an RLP list. */
struct RlpCursor = {
    source : ByteSlice,
    current : source_pointer,
    stop : source_pointer,
    valid : bool,
}
