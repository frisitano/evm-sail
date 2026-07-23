
/*md
# RLP field references

Decoded RLP fields that keep offsets into their original byte source
(YP Appendix B). Pure data — no registers, no externs.
*/
/*! The witness-carrying fields of a decoded RLP reference. Both the complete
    encoding and its content are statically contained by the source slice. */
struct RlpFieldRefFields(
    'source_off : Int,
    'source_len : Int,
    'full_off : Int,
    'full_len : Int,
    'content_off : Int,
    'content_len : Int,
),
    source_valid_range('source_off, 'source_len) &
    0 <= 'full_off & 0 <= 'full_len & 'full_off + 'full_len <= 'source_len &
    0 <= 'content_off & 0 <= 'content_len & 'content_off + 'content_len <= 'source_len = {
    source      : ByteSliceFields('source_off, 'source_len),
    is_list     : bool,
    full_off    : int('full_off),
    full_len    : int('full_len),
    content_off : int('content_off),
    content_len : int('content_len),
}

/*! A decoded RLP field: its source, list/string tag, and the offsets and
    lengths of both the full encoding and its content. */
type RlpFieldRef = {
    'source_off 'source_len 'full_off 'full_len 'content_off 'content_len,
    source_valid_range('source_off, 'source_len) &
    0 <= 'full_off & 0 <= 'full_len & 'full_off + 'full_len <= 'source_len &
    0 <= 'content_off & 0 <= 'content_len & 'content_off + 'content_len <= 'source_len.
    RlpFieldRefFields(
        'source_off,
        'source_len,
        'full_off,
        'full_len,
        'content_off,
        'content_len,
    )
}

/*! The witness-carrying fields of a one-pass RLP cursor. */
struct RlpCursorFields('source_off : Int, 'source_len : Int, 'current : Int, 'stop : Int),
    source_valid_range('source_off, 'source_len) &
    0 <= 'current & 'current <= 'stop & 'stop <= 'source_len = {
    source : ByteSliceFields('source_off, 'source_len),
    current : int('current),
    stop : int('stop),
    valid : bool,
}

/*! A one-pass view over the immediate children of an RLP list. */
type RlpCursor = {
    'source_off 'source_len 'current 'stop,
    source_valid_range('source_off, 'source_len) &
    0 <= 'current & 'current <= 'stop & 'stop <= 'source_len.
    RlpCursorFields('source_off, 'source_len, 'current, 'stop)
}
