
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

/*! The result of decoding one complete field at the head of a cursor. In
    addition to the field's own validity, the complete encoding is non-empty
    and contained by the cursor. The field's `source.len` is therefore a
    witness for the amount a caller may subsequently advance. */
type rlp_decoded_item_valid(
    'source_off : Int,
    'source_len : Int,
    'full_len : Int,
    'content_len : Int,
) -> Bool =
       rlp_field_ref_valid('source_off, 'full_len, 'content_len)
    &  0 < 'full_len
    &  'full_len <= 'source_len

/*! A positive amount proven to be contained by the cursor it consumes. */
type rlp_cursor_advance_valid(
    'source_len : Int,
    'consumed : Int,
) -> Bool =
       0 < 'consumed
    &  'consumed <= 'source_len

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
    source      : StatelessInputSliceFields('source_off, 'source_len),
    is_list     : bool,
    content_len : int('content_len),
}

/*! A one-pass RLP cursor is the unconsumed suffix of its source. Decoding
    yields a field whose length witnesses a valid advance; the caller owns the
    corresponding cursor transition. */
type RlpCursor('source_off : Int, 'source_len : Int),
    source_valid_range('source_off, 'source_len) =
    StatelessInputSliceFields('source_off, 'source_len)

/*! The same RLP framing invariants over a node encoding held in the scratch
    arena. Keeping this nominally separate prevents decoded input fields from
    acquiring a runtime byte-source tag. */
struct ScratchRlpFieldRef(
    'source_off : Int,
    'source_len : Int,
    'content_len : Int,
),
    rlp_field_ref_valid(
        'source_off,
        'source_len,
        'content_len,
    ) = {
    source      : ScratchSliceFields('source_off, 'source_len),
    is_list     : bool,
    content_len : int('content_len),
}

type ScratchRlpCursor('source_off : Int, 'source_len : Int),
    source_valid_range('source_off, 'source_len) =
    ScratchSliceFields('source_off, 'source_len)

/*! The result of applying an RLP field's protocol-level value constraint.
    Structurally invalid or non-canonical RLP remains an `InvalidBlock`
    exception; this result distinguishes a well-formed value outside the
    requested field domain. */
union RlpResult('value : Type) = {
    RlpOk : 'value,
    RlpInvalidValue : unit
}
