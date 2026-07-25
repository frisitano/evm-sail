
/*md
# RLP field references

Decoded RLP fields that keep offsets into their original byte source
(YP Appendix B). Pure data — no registers, no externs.
*/
/*! The complete containment invariant for an RLP field reference. `source`
    is normalized to the complete encoded item. RLP content is its suffix, so
    the content offset is derived as `source.len - content_len`. */
type rlp_field_ref_valid(
    'source_off : Int,
    'source_len : Int,
    'content_len : Int,
) -> Bool =
       source_valid_range('source_off, 'source_len)
    &  0 <= 'content_len
    &  'content_len <= 'source_len

/*! The result of popping one complete field from a cursor. In addition to
    the field's own validity, the complete encoding is non-empty and contained
    by the cursor being consumed. */
type rlp_cursor_pop_valid(
    'source_off : Int,
    'source_len : Int,
    'full_len : Int,
    'content_len : Int,
) -> Bool =
       rlp_field_ref_valid('source_off, 'full_len, 'content_len)
    &  0 < 'full_len
    &  'full_len <= 'source_len

/*! The witness-carrying fields of a decoded RLP reference. Both the complete
    encoding and its content are statically contained by the source slice. */
struct RlpFieldRef(
    'source_off : Int,
    'source_len : Int,
    'content_len : Int,
),
    rlp_field_ref_valid(
        'source_off,
        'source_len,
        'content_len,
    ) = {
    source      : ByteSliceFields('source_off, 'source_len),
    is_list     : bool,
    content_len : int('content_len),
}

/*! A one-pass RLP cursor is the unconsumed suffix of its source. Popping an
    item advances both its offset and its decreasing remaining length without
    carrying a second position field. */
type RlpCursor('source_off : Int, 'source_len : Int),
    source_valid_range('source_off, 'source_len) =
    ByteSliceFields('source_off, 'source_len)
