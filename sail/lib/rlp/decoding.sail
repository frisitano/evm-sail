/*md
# RLP decoding

Canonical Recursive Length Prefix framing, cursor traversal, and typed scalar
decoding. Input- and scratch-backed cursors remain nominally distinct so a
decoder never pays for a runtime provenance tag.
*/

/* Malformed RLP terminates validation with fatal_error(RlpDecode) at the
 * point where the invalid encoding is detected. */

/*! Parses exactly `width` big-endian bytes.  The result bound follows from the
    width, so callers can expose a protocol-width bound through the argument
    rather than reconstructing it with a result-side check. */
val rlp_uint64_append : forall 'width 'prefix,
                          1 <= 'width & 'width <= 8 & 0 <= 'prefix & 'prefix <= 2 ^ (8 * ('width - 1)) - 1. (
        int('width),
        int('prefix),
        byte,
    ) -> range(0, 2 ^ (8 * 'width) - 1)

function rlp_uint64_append(_width, prefix, next) =
    prefix * 256 + unsigned(next)

/*! Decodes exactly `width` big-endian bytes into a bounded unsigned value. */
function rlp_uint64_width forall 'n, 0 <= 'n <= 8. (
    content : StatelessInputSlice,
    width : int('n),
) -> (
    range(0, 2 ^ (8 * 'n) - 1)
) =
    if width == 0 then {
        0
    } else {
        let prefix = rlp_uint64_width(content, width - 1);
        let next = slice_byte(content, width - 1);
        rlp_uint64_append(width, prefix, next)
    }
termination_measure rlp_uint64_width(content, width) = width

/*! Decodes the first RLP header in an exact remaining source slice:
    `(is_list, content_offset, content_length)`. */
function rlp_ref_hdr(b : StatelessInputSlice) -> (bool, source_pointer, range(0, 2 ^ 64 - 1)) = {
    let source_len = b.len;
    if source_len == 0 then {
        fatal_error(RlpDecode)
    };
    let first_byte = slice_byte(b, 0);
    let h = unsigned(first_byte);
    if h < 128 then {
        (false, 0, 1)
    } else if h < 184 then {
        (false, 1, h - 128)
    } else if h < 192 then {
        let length_width : range(1, 8) = h - 183;
        if length_width <= source_len - 1 then {
            let length_bytes = sub_slice(b, 1, length_width);
            (false, 1 + length_width, rlp_uint64_width(length_bytes, length_width))
        } else {
            fatal_error(RlpDecode)
        }
    } else if h < 248 then {
        (true, 1, h - 192)
    } else {
        let length_width : range(1, 8) = h - 247;
        if length_width <= source_len - 1 then {
            let length_bytes = sub_slice(b, 1, length_width);
            (true, 1 + length_width, rlp_uint64_width(length_bytes, length_width))
        } else {
            fatal_error(RlpDecode)
        }
    }
}

/*! A cursor over a decoded field's children; throws if the field is not
    a list. */
val rlp_decode_list : forall 'source_off 'source_len 'content_len,
                        rlp_field_ref_valid('source_off, 'source_len, 'content_len). RlpFieldRef(
        'source_off,
        'source_len,
        'content_len,
    ) -> RlpCursor('source_off + ('source_len - 'content_len), 'content_len)

function rlp_decode_list(f) =
    if f.is_list then {
        sub_slice(f.source, f.source.len - f.content_len, f.content_len)
    } else {
        fatal_error(RlpDecode)
    }

/*! Decodes the item at the head of a cursor without advancing it. The
    resulting field's complete source length witnesses a valid subsequent
    `rlp_cursor_advance`. */
val rlp_decode_item : forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). RlpCursor(
        'source_off,
        'source_len,
    ) -> {'content_len 'full_len,
          rlp_decoded_item_valid('source_off, 'source_len, 'full_len, 'content_len).
          RlpFieldRef('source_off, 'full_len, 'content_len)}

function rlp_decode_item(cursor) = {
    if cursor.len == 0 then {
        fatal_error(RlpDecode)
    };
    let (is_list, content_off, content_len_value) = rlp_ref_hdr(cursor);
    let (content_len as 'content_len) = content_len_value;
    if cursor.len < content_off then {
        fatal_error(RlpDecode)
    };
    if cursor.len - content_off < content_len then {
        fatal_error(RlpDecode)
    };
    let (full_len as 'full_len) = content_off + content_len;
    if (0 < full_len) & (full_len <= cursor.len) then {
        let field_source = sub_slice(cursor, 0, full_len);
        let field : RlpFieldRef('source_off, 'full_len, 'content_len) = struct {
                source = field_source,
                is_list = is_list,
                content_len = content_len,
            };
        field
    } else {
        fatal_error(RlpDecode)
    }
}

/*! Consumes a decoded item's proven complete length from a cursor. */
val rlp_cursor_advance : forall 'source_off 'source_len 'consumed,
                           source_valid_range('source_off, 'source_len) &
                               rlp_cursor_advance_valid('source_len, 'consumed). (
        RlpCursor('source_off, 'source_len),
        int('consumed),
    ) -> RlpCursor('source_off + 'consumed, 'source_len - 'consumed)
function rlp_cursor_advance(cursor, consumed) =
    slice_suffix(cursor, consumed)

/*! Asserts the cursor is exhausted — trailing bytes are a decode
    error. */
val rlp_cursor_expect_end : forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). RlpCursor(
        'source_off,
        'source_len,
    ) -> unit
function rlp_cursor_expect_end(cursor) = {
    if cursor.len == 0 then {
        return ()
    };
    fatal_error(RlpDecode)
}

/*! Decodes a byte sequence that must be exactly one RLP item. */
val rlp_single_ref : forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). StatelessInputSliceFields(
        'source_off,
        'source_len,
    ) -> {'content_len,
          rlp_field_ref_valid('source_off, 'source_len, 'content_len).
          RlpFieldRef('source_off, 'source_len, 'content_len)}
function rlp_single_ref(item) = {
    let item_length = item.len;
    if item_length == 0 then {
        fatal_error(RlpDecode)
    } else {
        let (is_list, content_off, content_len_value) = rlp_ref_hdr(item);
        let (content_len as 'content_len) = content_len_value;
        if (content_off <= item_length) & (content_len == item_length - content_off) then {
            let field : RlpFieldRef('source_off, 'source_len, 'content_len) = struct {
                    source = item,
                    is_list = is_list,
                    content_len = content_len,
                };
            field
        } else {
            fatal_error(RlpDecode)
        }
    }
}

/*! A cursor over the children of a byte sequence that must be exactly
    one RLP list (e.g. a trie node). */
function rlp_node_cursor(node : StatelessInputSlice) -> (
    {'source_off 'source_len,
     source_valid_range('source_off, 'source_len).
     RlpCursor('source_off, 'source_len)}
) = {
    if node.len == 0 then {
        fatal_error(RlpDecode)
    } else {
        let item = rlp_single_ref(node);
        rlp_decode_list(item)
    }
}

/*! The content span of a field. */
function rlp_item_content forall 'source_off 'source_len 'content_len,
                            rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    StatelessInputSlice
) =
    sub_slice(f.source, f.source.len - f.content_len, f.content_len)

/*md
### Scratch-node decoding

Canonical MPT construction can embed a freshly encoded child in its parent.
Those sub-32-byte encodings live in scratch, so they use a separate cursor
family rather than a generic source-tagged RLP cursor.
*/
/*! [rlp_uint64_width][] over a scratch-backed content slice. */
function scratch_rlp_uint64_width forall 'n, 0 <= 'n <= 8. (
    content : ScratchSlice,
    width : int('n),
) -> (
    range(0, 2 ^ (8 * 'n) - 1)
) =
    if width == 0 then {
        0
    } else {
        let prefix = scratch_rlp_uint64_width(content, width - 1);
        let next = slice_byte(content, width - 1);
        rlp_uint64_append(width, prefix, next)
    }
termination_measure scratch_rlp_uint64_width(content, width) = width

/*! [rlp_ref_hdr][] over a scratch-backed source slice. */
function scratch_rlp_ref_hdr(b : ScratchSlice) -> (bool, source_pointer, range(0, 2 ^ 64 - 1)) = {
    let source_len = b.len;
    if source_len == 0 then {
        fatal_error(RlpDecode)
    };
    let first_byte = slice_byte(b, 0);
    let h = unsigned(first_byte);
    if h < 128 then {
        (false, 0, 1)
    } else if h < 184 then {
        (false, 1, h - 128)
    } else if h < 192 then {
        let length_width : range(1, 8) = h - 183;
        if length_width <= source_len - 1 then {
            let length_bytes = sub_slice(b, 1, length_width);
            (false, 1 + length_width, scratch_rlp_uint64_width(length_bytes, length_width))
        } else {
            fatal_error(RlpDecode)
        }
    } else if h < 248 then {
        (true, 1, h - 192)
    } else {
        let length_width : range(1, 8) = h - 247;
        if length_width <= source_len - 1 then {
            let length_bytes = sub_slice(b, 1, length_width);
            (true, 1 + length_width, scratch_rlp_uint64_width(length_bytes, length_width))
        } else {
            fatal_error(RlpDecode)
        }
    }
}

/*! [rlp_decode_list][] over the scratch cursor family. */
val scratch_rlp_decode_list : forall 'source_off 'source_len 'content_len,
                                rlp_field_ref_valid('source_off, 'source_len, 'content_len). ScratchRlpFieldRef(
        'source_off,
        'source_len,
        'content_len,
    ) -> ScratchRlpCursor('source_off + ('source_len - 'content_len), 'content_len)
function scratch_rlp_decode_list(f) =
    if f.is_list then {
        sub_slice(f.source, f.source.len - f.content_len, f.content_len)
    } else {
        fatal_error(RlpDecode)
    }

/*! [rlp_decode_item][] over the scratch cursor family. */
val scratch_rlp_decode_item : forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). ScratchRlpCursor(
        'source_off,
        'source_len,
    ) -> {'content_len 'full_len,
          rlp_decoded_item_valid('source_off, 'source_len, 'full_len, 'content_len).
          ScratchRlpFieldRef('source_off, 'full_len, 'content_len)}
function scratch_rlp_decode_item(cursor) = {
    if cursor.len == 0 then {
        fatal_error(RlpDecode)
    };
    let (is_list, content_off, content_len_value) = scratch_rlp_ref_hdr(cursor);
    let (content_len as 'content_len) = content_len_value;
    if cursor.len < content_off then {
        fatal_error(RlpDecode)
    };
    if cursor.len - content_off < content_len then {
        fatal_error(RlpDecode)
    };
    let (full_len as 'full_len) = content_off + content_len;
    if (0 < full_len) & (full_len <= cursor.len) then {
        let field_source = sub_slice(cursor, 0, full_len);
        let field : ScratchRlpFieldRef('source_off, 'full_len, 'content_len) = struct {
                source = field_source,
                is_list = is_list,
                content_len = content_len,
            };
        field
    } else {
        fatal_error(RlpDecode)
    }
}

val scratch_rlp_cursor_advance : forall 'source_off 'source_len 'consumed,
                                   source_valid_range('source_off, 'source_len) &
                                       rlp_cursor_advance_valid('source_len, 'consumed). (
        ScratchRlpCursor('source_off, 'source_len),
        int('consumed),
    ) -> ScratchRlpCursor('source_off + 'consumed, 'source_len - 'consumed)
function scratch_rlp_cursor_advance(cursor, consumed) =
    slice_suffix(cursor, consumed)

/*! [rlp_cursor_expect_end][] over the scratch cursor family. */
val scratch_rlp_cursor_expect_end : forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). ScratchRlpCursor(
        'source_off,
        'source_len,
    ) -> unit
function scratch_rlp_cursor_expect_end(cursor) = {
    if cursor.len == 0 then {
        return ()
    };
    fatal_error(RlpDecode)
}

/*! [rlp_single_ref][] over a scratch-backed byte sequence. */
val scratch_rlp_single_ref : forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). ScratchSliceFields(
        'source_off,
        'source_len,
    ) -> {'content_len,
          rlp_field_ref_valid('source_off, 'source_len, 'content_len).
          ScratchRlpFieldRef('source_off, 'source_len, 'content_len)}
function scratch_rlp_single_ref(item) = {
    let item_length = item.len;
    if item_length == 0 then {
        fatal_error(RlpDecode)
    } else {
        let (is_list, content_off, content_len_value) = scratch_rlp_ref_hdr(item);
        let (content_len as 'content_len) = content_len_value;
        if (content_off <= item_length) & (content_len == item_length - content_off) then {
            let field : ScratchRlpFieldRef('source_off, 'source_len, 'content_len) = struct {
                    source = item,
                    is_list = is_list,
                    content_len = content_len,
                };
            field
        } else {
            fatal_error(RlpDecode)
        }
    }
}

/*! [rlp_node_cursor][] over a freshly encoded scratch node. */
function scratch_rlp_node_cursor(node : ScratchSlice) -> (
    {'source_off 'source_len,
     source_valid_range('source_off, 'source_len).
     ScratchRlpCursor('source_off, 'source_len)}
) =
    let item = scratch_rlp_single_ref(node) in
    scratch_rlp_decode_list(item)

/*! [rlp_item_content][] for a scratch-backed field. */
function scratch_rlp_item_content forall 'source_off 'source_len 'content_len,
                                    rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    ScratchRlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    ScratchSlice
) =
    sub_slice(f.source, f.source.len - f.content_len, f.content_len)

/*! [rlp_decode_word][] for a scratch-backed field. */
function scratch_rlp_decode_word forall 'source_off 'source_len 'content_len,
                                   rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    ScratchRlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    word
) = {
    let n = f.content_len;
    if f.is_list | (RLP_WORD_LENGTH_LIMIT < n) then {
        fatal_error(RlpDecode)
    } else {
        slice_load_n(f.source, f.source.len - n, n)
    }
}

/*! Whether a field uses the unique canonical RLP framing for its payload. */
function rlp_ref_framing_canonical forall 'source_off 'source_len 'content_len,
                                     rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    bool
) = {
    let n = f.content_len;
    let payload_length = n;
    let full_length = f.source.len;
    let full_offset = 0;
    let content_offset = full_length - payload_length;
    let source = f.source;
    let source_length = source.len;
    if full_length == 0 then {
        false
    } else if f.is_list then {
        if n <= RLP_SHORT_LENGTH_LIMIT then {
            let length_byte = rlp_length_byte(n);
            (full_length == payload_length + 1) & (slice_byte(f.source, 0) == add_bits(0xc0, length_byte))
        } else {
            let length_word = rlp_length_word(n);
            let length_width = rlp_minimal_word_len(length_word);
            let length_byte = rlp_length_byte(length_width);
              (full_length == 1 + length_width + payload_length)
            & (slice_byte(f.source, 0) == add_bits(0xf7, length_byte))
            & (if full_offset < source_length
              then slice_load_n(f.source, full_offset + 1, length_width) == length_word
              else false)
        }
    } else if payload_length == 0 then {
        (full_length == 1) & (slice_byte(f.source, 0) == 0x80)
    } else {
        let first = slice_byte(f.source, content_offset);
        if (payload_length == 1) & (first[7] == bitzero) then {
            (full_length == 1) & (slice_byte(f.source, 0) == first)
        } else if n <= RLP_SHORT_LENGTH_LIMIT then {
            let length_byte = rlp_length_byte(n);
            (full_length == payload_length + 1) & (slice_byte(f.source, 0) == add_bits(0x80, length_byte))
        } else {
            let length_word = rlp_length_word(n);
            let length_width = rlp_minimal_word_len(length_word);
            let length_byte = rlp_length_byte(length_width);
              (full_length == 1 + length_width + payload_length)
            & (slice_byte(f.source, 0) == add_bits(0xb7, length_byte))
            & (if full_offset < source_length
              then slice_load_n(f.source, full_offset + 1, length_width) == length_word
              else false)
        }
    }
}

/*! Whether a field is a byte string with its unique canonical RLP framing. */
function rlp_ref_bytes_canonical forall 'source_off 'source_len 'content_len,
                                   rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    bool
) =
    not_bool(f.is_list) & rlp_ref_framing_canonical(f)

/*! Whether a field is the canonical RLP encoding of a non-negative
    integer: minimal big-endian content with no leading zeros and the
    exact matching prefix. */
function rlp_item_uint_canonical forall 'source_off 'source_len 'content_len,
                                   rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    bool
) =
    let bytes_canonical = rlp_ref_bytes_canonical(f) in
    let invalid = not_bool(bytes_canonical) in
    if invalid then {
        false
    } else {
        (f.content_len == 0) | (slice_byte(f.source, f.source.len - f.content_len) != 0x00)
    }

/*! Whether a string field fits one EVM word. This is the non-throwing form
    used by structural validation passes that do not need the decoded value. */
function rlp_word_valid forall 'source_off 'source_len 'content_len,
                          rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    bool
) =
    not_bool(f.is_list) & (f.content_len <= RLP_WORD_LENGTH_LIMIT)

/*! Whether a field is a canonical unsigned EVM-word integer. */
function rlp_u256_valid forall 'source_off 'source_len 'content_len,
                          rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    bool
) = {
    let canonical = rlp_item_uint_canonical(f);
    canonical & (f.content_len <= RLP_WORD_LENGTH_LIMIT)
}

/*! Whether a field is a canonical unsigned 64-bit integer. */
function rlp_uint64_valid forall 'source_off 'source_len 'content_len,
                            rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    bool
) = {
    let canonical = rlp_item_uint_canonical(f);
    canonical & (f.content_len <= RLP_UINT64_LENGTH_LIMIT)
}

/*! Decodes a string field of at most 32 bytes into a word. */
function rlp_decode_word forall 'source_off 'source_len 'content_len,
                           rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    word
) = {
    let n = f.content_len;
    if f.is_list | (RLP_WORD_LENGTH_LIMIT < n) then {
        fatal_error(RlpDecode)
    } else {
        slice_load_n(f.source, f.source.len - n, n)
    }
}

/*! Decodes a canonical unsigned integer field into a word; throws
    otherwise. */
function rlp_decode_u256 forall 'source_off 'source_len 'content_len,
                           rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    word
) =
    let canonical = rlp_item_uint_canonical(f) in
    if canonical then {
        rlp_decode_word(f)
    } else {
        fatal_error(RlpDecode)
    }

/*! Decodes a canonical unsigned integer into the uint64 wire domain used by
    EIP-2681 account nonces and EIP-4844 excess blob gas. */
function rlp_decode_uint64 forall 'source_off 'source_len 'content_len,
                             rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    ssz_uint
) = {
    let canonical = rlp_item_uint_canonical(f);
    let invalid_encoding = not_bool(canonical);
    if invalid_encoding | (RLP_UINT64_LENGTH_LIMIT < f.content_len) then {
        fatal_error(RlpDecode)
    };
    let content_length = f.content_len;
    let content = rlp_item_content(f);

    /* The semantic check above establishes content_length <= 8.  Modulo nine
     * only exposes that already-proven bound to Sail's dependent type system. */
    let width : range(0, 8) = tmod_nat(content_length, 9);
    rlp_uint64_width(content, width)
}

/*! Interprets one canonical RLP integer as `0`/`1`. Other canonical integers
    return `RlpInvalidValue` rather than throwing so callers such as EIP-7702
    can reject only the containing authorization tuple. */
val rlp_decode_bool : forall 'source_off 'source_len 'content_len,
                        rlp_field_ref_valid('source_off, 'source_len, 'content_len). RlpFieldRef(
        'source_off,
        'source_len,
        'content_len,
    ) -> RlpResult(bool)

function rlp_decode_bool(field) = {
    let value = rlp_decode_uint64(field);
    match value {
        0 => RlpOk(false),
        1 => RlpOk(true),
        _ => RlpInvalidValue(),
    }
}
