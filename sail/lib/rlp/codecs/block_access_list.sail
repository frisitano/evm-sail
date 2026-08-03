/*md
# Block access list RLP decoding

Canonical list, byte-string, and integer field decoding shared by EIP-7928
validation.
*/

/*! Requires a canonical RLP list and returns its child cursor. */
val bal_ref_cursor : forall 'source_off 'source_len 'content_len,
                       rlp_field_ref_valid('source_off, 'source_len, 'content_len). RlpFieldRef(
        'source_off,
        'source_len,
        'content_len,
    ) -> RlpCursor('source_off + ('source_len - 'content_len), 'content_len)
function bal_ref_cursor(f) =
    if f.is_list & rlp_ref_framing_canonical(f) then {
        rlp_decode_list(f)
    } else {
        throw InvalidBlock(InvalidBlockAccessList)
    }

/*! Requires a canonical RLP byte string and returns its content slice. */
function bal_ref_bytes forall 'source_off 'source_len 'content_len,
                         rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    StatelessInputSlice
) =
    if rlp_ref_bytes_canonical(f) then {
        rlp_item_content(f)
    } else {
        throw InvalidBlock(InvalidBlockAccessList)
    }

/*! Requires a canonical RLP integer in the EVM-word domain. */
function bal_ref_word forall 'source_off 'source_len 'content_len,
                        rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    word
) =
    if rlp_item_uint_canonical(f) & (f.content_len <= 32) then {
        rlp_decode_word(f)
    } else {
        throw InvalidBlock(InvalidBlockAccessList)
    }

/*! Requires a canonical RLP integer in the host-index/nonce domain. */
function bal_ref_uint64 forall 'source_off 'source_len 'content_len,
                          rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    ssz_uint
) =
    if rlp_item_uint_canonical(f) & (f.content_len <= 8) then {
        rlp_decode_uint64(f)
    } else {
        throw InvalidBlock(InvalidBlockAccessList)
    }

/*! Requires that a decoded list has no unconsumed children. */
function bal_expect_end forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (cursor :
    RlpCursor('source_off, 'source_len)) -> (
    unit
) =
    if cursor.len == 0 then {
        ()
    } else {
        throw InvalidBlock(InvalidBlockAccessList)
    }
