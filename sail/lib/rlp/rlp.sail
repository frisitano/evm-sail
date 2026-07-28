
/*md
# RLP encoding and decoding

Recursive Length Prefix (Yellow Paper Appendix B), the canonical
serialization Ethereum uses for accounts, block headers, transactions,
receipts, and Merkle-Patricia trie nodes. RLP encodes a recursive
structure of byte strings and lists; it imposes no type semantics, only a
minimal length-prefixed framing.

A byte `< 0x80` is its own encoding; strings of length `n ≤ 55` use
`0x80 + n`; longer strings use `0xb7 + |len|` followed by the big-endian
length. Lists mirror this with the `0xc0`/`0xf7` prefix bands over an
already-encoded payload. Integers encode as their minimal big-endian byte
string with no leading zeros. Decoding proceeds one level at a time; a
field reference retains the complete encoded item as a source slice and its
content length without copying; the content is always the corresponding
suffix.

Malformed RLP is a normal invalid-block outcome: the decoders throw
`InvalidBlock(RlpDecode)` at the point of detection, caught once at the
guest top level.
*/
/*md
## Constants

RLP's short-form threshold and the fixed encoded widths used throughout the
execution-layer data model.
*/
let RLP_SHORT_LENGTH_LIMIT : int(55) = 55
let RLP_WORD_LENGTH_LIMIT : int(32) = WORD_BYTE_LENGTH
let RLP_UINT64_LENGTH_LIMIT : int(8) = EIGHT_BYTE_LENGTH
let RLP_ENCODED_WORD_LENGTH : int(33) = 33
let RLP_ENCODED_ADDRESS_LENGTH : int(21) = 21

/*! An RLP byte count that can be materialized in the scratch arena. The
    canonical model is unbounded; optimized builds refine this representation
    and validate recursive collection totals at this boundary. */
type rlp_scratch_length = nat

/*! Values accepted by the generic natural-number RLP helpers. Canonical RLP
    is unbounded; optimized builds refine values entering the native encoder. */
type rlp_natural_valid('value : Int) -> Bool = 0 <= 'value

/*! Natural values that can be incremented while counting encoded bytes. */
type rlp_natural_increment_valid('value : Int) -> Bool = 0 <= 'value

/*! Encoded width returned by the generic natural-number RLP helper. */
type rlp_natural_size = nat

/*! Introduces the scratch-arena representation for a fixed-width RLP
    component. */
val rlp_scratch_small_length : forall 'value, 0 <= 'value & 'value <= 33. int('value) -> rlp_scratch_length
function rlp_scratch_small_length(value) = value

/*! Adds two materializable RLP byte counts. */
val rlp_scratch_length_add : forall 'left 'right, 0 <= 'left & 0 <= 'right. (int('left), int('right)) -> rlp_scratch_length
function rlp_scratch_length_add(left, right) =
    left + right

/*md
## The encoders
*/
/* Wire-format conversions stay with RLP rather than defining arithmetic over
 * every byte quantity. Callers establish the stated bounds before encoding. */
function rlp_nat_length_byte(value : range(0, 255)) -> byte =
    get_slice_int(8, value, 0)

/*! Narrows a previously bounded byte length for an RLP prefix byte. */
val rlp_byte_length_byte : forall 'value, 0 <= 'value. int('value) -> byte
function rlp_byte_length_byte(value) = {
    assert(value <= 255);
    let length = value;
    get_slice_int(8, length, 0)
}

overload rlp_length_byte = { rlp_nat_length_byte, rlp_byte_length_byte }

val rlp_length_word : forall 'value, 0 <= 'value. int('value) -> word
function rlp_length_word(value) = {
    word_of_nat_byte_count(value)
}

/*! Returns the minimal big-endian byte length of a 256-bit word. */
val rlp_minimal_word_len : forall 'word, 0 <= 'word & 'word < 2 ^ 256. int('word) -> range(0, 32)
function rlp_minimal_word_len(w) = {
    word_byte_length(w)
}

/*! Returns the minimal big-endian byte length of a natural. */
val rlp_minimal_uint_len : forall 'n, rlp_natural_valid('n). int('n) -> range(0, 'n)

/*! Advances a minimal-byte count by one. */
val rlp_natural_increment : forall 'value, rlp_natural_increment_valid('value). int('value) -> int('value + 1)

function rlp_natural_increment(value) = value + 1

/*! Computes a natural's minimal big-endian byte length recursively. */
function rlp_minimal_uint_len(n) =
    if n == 0 then {
        0
    } else {
        rlp_natural_increment(rlp_minimal_uint_len(n / 256))
    }
termination_measure rlp_minimal_uint_len(n) = n

/*! Returns the RLP header width for a string or list payload length. */
val rlp_length_prefix_len : forall 'len, 0 <= 'len. int('len) -> range(1, 33)
function rlp_length_prefix_len(len) =
    if len <= RLP_SHORT_LENGTH_LIMIT then {
        1
    } else {
        1 + rlp_minimal_word_len(rlp_length_word(len))
    }

/*! Returns the encoded RLP width of a byte string from its length and first
    byte. */
val rlp_string_size : forall 'len, 0 <= 'len. (int('len), byte) -> range(1, 'len + 33)
function rlp_string_size(len, first) = {
    if (len == 1) & (first[7] == bitzero) then {
        1
    } else {
        rlp_length_prefix_len(len) + len
    }
}

/*! Returns the encoded RLP width of a source-backed byte slice. */
val rlp_input_slice_size : forall 'off 'len, source_valid_range('off, 'len). StatelessInputSliceFields('off, 'len) -> range(
        1,
        'len + 33,
    )
function rlp_input_slice_size(data) = {
    let len = data.len;
    let first = if len == 0 then {
            0x00
        } else {
            slice_byte(data, 0)
        };
    rlp_string_size(len, first)
}

function rlp_scratch_region_size(data : ScratchSlice) -> rlp_scratch_length = {
    let first = if data.len == 0 then {
            0x00
        } else {
            slice_byte(data, 0)
        };
    rlp_string_size(data.len, first)
}

function rlp_log_data_size(data : LogDataSlice) -> rlp_scratch_length = {
    let first = if data.len == 0 then {
            0x00
        } else {
            slice_byte(data, 0)
        };
    rlp_string_size(data.len, first)
}

overload rlp_slice_size = {rlp_input_slice_size, rlp_scratch_region_size, rlp_log_data_size}

/*! Returns the encoded RLP width of a minimally represented word integer. */
val rlp_uint_word_size : forall 'word, 0 <= 'word & 'word < 2 ^ 256. int('word) -> range(1, 33)
function rlp_uint_word_size(w) = {
    let len = rlp_minimal_word_len(w);
    let first = if len == 1 then {
            word_low_byte(w)
        } else {
            0x00
        };
    if (len == 1) & (first[7] == bitzero) then {
        1
    } else {
        1 + len
    }
}

/*! Returns the encoded RLP width of a natural integer. */
val rlp_uint_nat_size : forall 'n, rlp_natural_valid('n). int('n) -> rlp_natural_size
function rlp_uint_nat_size(n) = {
    if n < sizeof(word_modulus) then {
        rlp_uint_word_size(U256(n))
    } else {
        /* n >= 2^256 implies a minimal encoding of at least 33 bytes, so the
         * single-byte RLP case is unreachable on this branch. */
        let len = rlp_minimal_uint_len(n);
        if len <= RLP_SHORT_LENGTH_LIMIT then {
            1 + len
        } else {
            1 + rlp_minimal_uint_len(len) + len
        }
    }
}

/*! Returns the encoded RLP width of a uint64-range protocol quantity. */
val rlp_uint_u64_size : forall 'n, 0 <= 'n & 'n <= 2 ^ 64 - 1. int('n) -> range(1, 9)
function rlp_uint_u64_size(n) =
    if n < 2 ^ 7 then {
        1
    } else {
        if n < 2 ^ 8 then {
            2
        } else {
            if n < 2 ^ 16 then {
                3
            } else {
                if n < 2 ^ 24 then {
                    4
                } else {
                    if n < 2 ^ 32 then {
                        5
                    } else {
                        if n < 2 ^ 40 then {
                            6
                        } else {
                            if n < 2 ^ 48 then { 7 } else { if n < 2 ^ 56 then { 8 } else { 9 } }
                        }
                    }
                }
            }
        }
    }

/* Semantic uint64 subtypes select the tight protocol-quantity arm; only
 * genuinely unbounded canonical naturals retain the recursive specification. */
overload rlp_uint_size = {rlp_uint_word_size, rlp_uint_u64_size, rlp_uint_nat_size}

function rlp_word_size() -> int(33) = RLP_ENCODED_WORD_LENGTH
function rlp_addr_size() -> int(21) = RLP_ENCODED_ADDRESS_LENGTH

/*! Returns the encoded size of an RLP list with `content_len` payload bytes. */
val rlp_list_size : forall 'len, 0 <= 'len. int('len) -> range(1, 'len + 33)
function rlp_list_size(content_len) = {
    rlp_length_prefix_len(content_len) + content_len
}

/*! Adds the canonical list prefix to a materializable RLP content length. */
function rlp_scratch_list_size(content_len : rlp_scratch_length) -> rlp_scratch_length =
    rlp_scratch_length_add(content_len, rlp_scratch_small_length(rlp_length_prefix_len(content_len)))

/*! Returns the materializable RLP width of a source-backed byte slice. */
function rlp_input_scratch_slice_size(data : StatelessInputSlice) -> rlp_scratch_length =
    rlp_slice_size(data)

function rlp_scratch_scratch_slice_size(data : ScratchSlice) -> rlp_scratch_length =
    rlp_slice_size(data)

function rlp_log_scratch_slice_size(data : LogDataSlice) -> rlp_scratch_length =
    rlp_slice_size(data)

overload rlp_scratch_slice_size = {
    rlp_input_scratch_slice_size,
    rlp_scratch_scratch_slice_size,
    rlp_log_scratch_slice_size
}

/*! Appends the canonical RLP prefix of a byte-string payload. */
val rlp_write_string_prefix : forall 'len, 0 <= 'len. (int('len), byte) -> unit
function rlp_write_string_prefix(len, first) = {
    if (len == 1) & (first[7] == bitzero) then {
        ()
    } else {
        if len <= RLP_SHORT_LENGTH_LIMIT then {
            scratch_push_byte(add_bits(0x80, rlp_length_byte(len)))
        } else {
            let length_word = rlp_length_word(len);
            let length_len = rlp_minimal_word_len(length_word);
            scratch_push_byte(add_bits(0xb7, rlp_length_byte(length_len)));
            scratch_push_word_be(length_word, length_len)
        }
    }
}

/*! Appends the canonical RLP prefix of a list payload. */
val rlp_write_list_prefix : forall 'len, 0 <= 'len. int('len) -> unit
function rlp_write_list_prefix(content_len) = {
    if content_len <= RLP_SHORT_LENGTH_LIMIT then {
        scratch_push_byte(add_bits(0xc0, rlp_length_byte(content_len)))
    } else {
        let length_word = rlp_length_word(content_len);
        let length_len = rlp_minimal_word_len(length_word);
        scratch_push_byte(add_bits(0xf7, rlp_length_byte(length_len)));
        scratch_push_word_be(length_word, length_len)
    }
}

/*! Appends a source-backed byte slice as an RLP string. */
function rlp_write_input_slice(data : StatelessInputSlice) -> unit = {
    rlp_write_string_prefix(
        data.len,
        if data.len == 0 then {
            0x00
        } else {
            slice_byte(data, 0)
        },
    );
    scratch_push_slice(data)
}

function rlp_write_scratch_slice(data : ScratchSlice) -> unit = {
    rlp_write_string_prefix(
        data.len,
        if data.len == 0 then {
            0x00
        } else {
            slice_byte(data, 0)
        },
    );
    scratch_push_slice(data)
}

function rlp_write_log_data_slice(data : LogDataSlice) -> unit = {
    rlp_write_string_prefix(
        data.len,
        if data.len == 0 then {
            0x00
        } else {
            slice_byte(data, 0)
        },
    );
    scratch_push_slice(data)
}

overload rlp_write_slice = {rlp_write_input_slice, rlp_write_scratch_slice, rlp_write_log_data_slice}

/*! Appends a word as a minimal RLP integer. */
val rlp_write_uint_word : forall 'word, 0 <= 'word & 'word < 2 ^ 256. int('word) -> unit
function rlp_write_uint_word(w) = {
    let len = rlp_minimal_word_len(w);
    let first = if len == 1 then {
            word_low_byte(w)
        } else {
            0x00
        };
    rlp_write_string_prefix(len, first);
    scratch_push_word_be(U256(w), len)
}

/*! Appends a natural as a minimal RLP integer. */
val rlp_write_uint_nat : forall 'n, rlp_natural_valid('n). int('n) -> unit
function rlp_write_uint_nat(n) =
    if n < sizeof(word_modulus) then {
        rlp_write_uint_word(U256(n))
    } else {
        throw InvalidBlock(InvalidConfig)
    }

/*! Appends a uint64-range protocol quantity as a minimal RLP integer. */
val rlp_write_uint_u64 : forall 'n, 0 <= 'n & 'n <= 2 ^ 64 - 1. int('n) -> unit
function rlp_write_uint_u64(n) = rlp_write_uint_word(U256(n))

/* Optimized semantic u64 subtypes select the word-bounded arm directly.
 * Canonical unbounded naturals retain the generic specification. */
overload rlp_write_uint = {rlp_write_uint_word, rlp_write_uint_u64, rlp_write_uint_nat}

/*! Appends one full-width EVM word as an RLP byte string. */
function rlp_write_word(w : word) -> unit = {
    rlp_write_string_prefix(WORD_BYTE_LENGTH, 0x00);
    scratch_push_word_be(w, WORD_BYTE_LENGTH)
}

/*! Appends one Ethereum address as an RLP byte string. */
function rlp_write_addr(a : address) -> unit = {
    rlp_write_string_prefix(ADDRESS_BYTE_LENGTH, a[0]);
    scratch_push_address(a)
}

/*md
## The decoders
*/
/*! Finishes a forward scratch encoding. */
function rlp_finish(start : source_pointer) -> ScratchSlice =
    scratch_finish(start)

/* Malformed RLP is a normal invalid-block outcome: the decoders throw
 * InvalidBlock(RlpDecode) at detection and the guest catches it once at the
 * top level. */

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
        rlp_uint64_append(width, rlp_uint64_width(content, width - 1), slice_byte(content, width - 1))
    }
termination_measure rlp_uint64_width(content, width) = width

/*! Decodes the first RLP header in an exact remaining source slice:
    `(is_list, content_offset, content_length)`. */
function rlp_ref_hdr(b : StatelessInputSlice) -> (bool, source_pointer, range(0, 2 ^ 64 - 1)) = {
    let source_len = b.len;
    if source_len == 0 then {
        throw InvalidBlock(RlpDecode)
    };
    let h = unsigned(slice_byte(b, 0));
    if h < 128 then {
        (false, 0, 1)
    } else {
        if h < 184 then {
            (false, 1, h - 128)
        } else {
            if h < 192 then {
                let length_width : range(1, 8) = h - 183;
                if source_len - 1 < length_width then {
                    throw InvalidBlock(RlpDecode)
                };
                let length_bytes = sub_slice(b, 1, length_width);
                (false, 1 + length_width, rlp_uint64_width(length_bytes, length_width))
            } else {
                if h < 248 then {
                    (true, 1, h - 192)
                } else {
                    let length_width : range(1, 8) = h - 247;
                    if source_len - 1 < length_width then {
                        throw InvalidBlock(RlpDecode)
                    };
                    let length_bytes = sub_slice(b, 1, length_width);
                    (true, 1 + length_width, rlp_uint64_width(length_bytes, length_width))
                }
            }
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
        throw InvalidBlock(RlpDecode)
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
        throw InvalidBlock(RlpDecode)
    };
    let (is_list, content_off, content_len_value) = rlp_ref_hdr(cursor);
    let (content_len as 'content_len) = content_len_value;
    if cursor.len < content_off then {
        throw InvalidBlock(RlpDecode)
    };
    if cursor.len - content_off < content_len then {
        throw InvalidBlock(RlpDecode)
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
        throw InvalidBlock(RlpDecode)
    }
}

/*! Consumes a decoded item's proven complete length from a cursor. */
val rlp_cursor_advance : forall 'source_off 'source_len 'consumed,
                          source_valid_range('source_off, 'source_len)
                        & rlp_cursor_advance_valid('source_len, 'consumed). (
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
function rlp_cursor_expect_end(cursor) =
    if cursor.len == 0 then {
        ()
    } else {
        throw InvalidBlock(RlpDecode)
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
        throw InvalidBlock(RlpDecode)
    } else {
        let (is_list, content_off, content_len_value) = rlp_ref_hdr(item);
        let (content_len as 'content_len) = content_len_value;
        if content_off > item_length then {
            throw InvalidBlock(RlpDecode)
        };
        if content_len != item_length - content_off then {
            throw InvalidBlock(RlpDecode)
        };
        let field : RlpFieldRef('source_off, 'source_len, 'content_len) = struct {
                source = item,
                is_list = is_list,
                content_len = content_len,
            };
        field
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
        throw InvalidBlock(RlpDecode)
    } else {
        rlp_decode_list(rlp_single_ref(node))
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
function scratch_rlp_uint64_width forall 'n, 0 <= 'n <= 8. (
    content : ScratchSlice,
    width : int('n),
) -> (
    range(0, 2 ^ (8 * 'n) - 1)
) =
    if width == 0 then {
        0
    } else {
        rlp_uint64_append(width, scratch_rlp_uint64_width(content, width - 1), slice_byte(content, width - 1))
    }
termination_measure scratch_rlp_uint64_width(content, width) = width

function scratch_rlp_ref_hdr(b : ScratchSlice) -> (bool, source_pointer, range(0, 2 ^ 64 - 1)) = {
    let source_len = b.len;
    if source_len == 0 then {
        throw InvalidBlock(RlpDecode)
    };
    let h = unsigned(slice_byte(b, 0));
    if h < 128 then {
        (false, 0, 1)
    } else {
        if h < 184 then {
            (false, 1, h - 128)
        } else {
            if h < 192 then {
                let length_width : range(1, 8) = h - 183;
                if source_len - 1 < length_width then {
                    throw InvalidBlock(RlpDecode)
                };
                let length_bytes = sub_slice(b, 1, length_width);
                (false, 1 + length_width, scratch_rlp_uint64_width(length_bytes, length_width))
            } else {
                if h < 248 then {
                    (true, 1, h - 192)
                } else {
                    let length_width : range(1, 8) = h - 247;
                    if source_len - 1 < length_width then {
                        throw InvalidBlock(RlpDecode)
                    };
                    let length_bytes = sub_slice(b, 1, length_width);
                    (true, 1 + length_width, scratch_rlp_uint64_width(length_bytes, length_width))
                }
            }
        }
    }
}

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
        throw InvalidBlock(RlpDecode)
    }

val scratch_rlp_decode_item : forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). ScratchRlpCursor(
        'source_off,
        'source_len,
    ) -> {'content_len 'full_len,
          rlp_decoded_item_valid('source_off, 'source_len, 'full_len, 'content_len).
          ScratchRlpFieldRef('source_off, 'full_len, 'content_len)}
function scratch_rlp_decode_item(cursor) = {
    if cursor.len == 0 then {
        throw InvalidBlock(RlpDecode)
    };
    let (is_list, content_off, content_len_value) = scratch_rlp_ref_hdr(cursor);
    let (content_len as 'content_len) = content_len_value;
    if cursor.len < content_off then {
        throw InvalidBlock(RlpDecode)
    };
    if cursor.len - content_off < content_len then {
        throw InvalidBlock(RlpDecode)
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
        throw InvalidBlock(RlpDecode)
    }
}

val scratch_rlp_cursor_advance : forall 'source_off 'source_len 'consumed,
                                  source_valid_range('source_off, 'source_len)
                                & rlp_cursor_advance_valid('source_len, 'consumed). (
        ScratchRlpCursor('source_off, 'source_len),
        int('consumed),
    ) -> ScratchRlpCursor('source_off + 'consumed, 'source_len - 'consumed)
function scratch_rlp_cursor_advance(cursor, consumed) =
    slice_suffix(cursor, consumed)

val scratch_rlp_cursor_expect_end : forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). ScratchRlpCursor(
        'source_off,
        'source_len,
    ) -> unit
function scratch_rlp_cursor_expect_end(cursor) =
    if cursor.len == 0 then {
        ()
    } else {
        throw InvalidBlock(RlpDecode)
    }

val scratch_rlp_single_ref : forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). ScratchSliceFields(
        'source_off,
        'source_len,
    ) -> {'content_len,
          rlp_field_ref_valid('source_off, 'source_len, 'content_len).
          ScratchRlpFieldRef('source_off, 'source_len, 'content_len)}
function scratch_rlp_single_ref(item) = {
    let item_length = item.len;
    if item_length == 0 then {
        throw InvalidBlock(RlpDecode)
    } else {
        let (is_list, content_off, content_len_value) = scratch_rlp_ref_hdr(item);
        let (content_len as 'content_len) = content_len_value;
        if content_off > item_length then {
            throw InvalidBlock(RlpDecode)
        };
        if content_len != item_length - content_off then {
            throw InvalidBlock(RlpDecode)
        };
        let field : ScratchRlpFieldRef('source_off, 'source_len, 'content_len) = struct {
                source = item,
                is_list = is_list,
                content_len = content_len,
            };
        field
    }
}

function scratch_rlp_node_cursor(node : ScratchSlice) -> (
    {'source_off 'source_len,
     source_valid_range('source_off, 'source_len).
     ScratchRlpCursor('source_off, 'source_len)}
) =
    scratch_rlp_decode_list(scratch_rlp_single_ref(node))

function scratch_rlp_item_content forall 'source_off 'source_len 'content_len,
                                   rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    ScratchRlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    ScratchSlice
) =
    sub_slice(f.source, f.source.len - f.content_len, f.content_len)

function scratch_rlp_decode_word forall 'source_off 'source_len 'content_len,
                                rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    ScratchRlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    word
) = {
    let n = f.content_len;
    if f.is_list | (RLP_WORD_LENGTH_LIMIT < n) then {
        throw InvalidBlock(RlpDecode)
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
    } else {
        if f.is_list then {
            if n <= RLP_SHORT_LENGTH_LIMIT then {
                (full_length == payload_length + 1) & (slice_byte(f.source, 0) == add_bits(0xc0, rlp_length_byte(n)))
            } else {
                let length_word = rlp_length_word(n);
                let length_width = rlp_minimal_word_len(length_word);
                  (full_length == 1 + length_width + payload_length)
                & (slice_byte(f.source, 0) == add_bits(0xf7, rlp_length_byte(length_width)))
                & (if full_offset < source_length then {
                      slice_load_n(f.source, full_offset + 1, length_width) == length_word
                  } else {
                      false
                  })
            }
        } else {
            if payload_length == 0 then {
                (full_length == 1) & (slice_byte(f.source, 0) == 0x80)
            } else {
                let first = slice_byte(f.source, content_offset);
                if (payload_length == 1) & (first[7] == bitzero) then {
                    (full_length == 1) & (slice_byte(f.source, 0) == first)
                } else {
                    if n <= RLP_SHORT_LENGTH_LIMIT then {
                        (full_length == payload_length + 1) &
                            (slice_byte(f.source, 0) == add_bits(0x80, rlp_length_byte(n)))
                    } else {
                        let length_word = rlp_length_word(n);
                        let length_width = rlp_minimal_word_len(length_word);
                          (full_length == 1 + length_width + payload_length)
                        & (slice_byte(f.source, 0) == add_bits(0xb7, rlp_length_byte(length_width)))
                        & (if full_offset < source_length then {
                              slice_load_n(f.source, full_offset + 1, length_width) == length_word
                          } else {
                              false
                          })
                    }
                }
            }
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
    if not_bool(rlp_ref_bytes_canonical(f)) then {
        false
    } else {
        (f.content_len == 0) | (slice_byte(f.source, f.source.len - f.content_len) != 0x00)
    }

/*! Decodes a string field of at most 32 bytes into a word. */
function rlp_decode_word forall 'source_off 'source_len 'content_len,
                        rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    word
) = {
    let n = f.content_len;
    if f.is_list | (RLP_WORD_LENGTH_LIMIT < n) then {
        throw InvalidBlock(RlpDecode)
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
    if rlp_item_uint_canonical(f) then {
        rlp_decode_word(f)
    } else {
        throw InvalidBlock(RlpDecode)
    }

/*! Decodes a canonical unsigned integer into the uint64 wire domain used by
    EIP-2681 account nonces and EIP-4844 excess blob gas. */
function rlp_decode_uint64 forall 'source_off 'source_len 'content_len,
                          rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    ssz_uint
) = {
    if not_bool(rlp_item_uint_canonical(f)) | (RLP_UINT64_LENGTH_LIMIT < f.content_len) then {
        throw InvalidBlock(RlpDecode)
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

/*! Decodes an arbitrary-width big-endian natural. */
function rlp_uint_content(content : StatelessInputSlice) -> nat = {
    let content_length = content.len;
    if content_length == 0 then {
        0
    } else {
        let prefix_length = content_length - 1;
        let prefix = sub_slice(content, 0, prefix_length);
        rlp_uint_content(prefix) * 256 + unsigned(slice_byte(content, prefix_length))
    }
}
termination_measure rlp_uint_content(content) = content.len

/*! Decodes an arbitrary-width canonical RLP unsigned integer. */
function rlp_decode_nat forall 'source_off 'source_len 'content_len,
                        rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    nat
) =
    if rlp_item_uint_canonical(f) then {
        rlp_uint_content(rlp_item_content(f))
    } else {
        throw InvalidBlock(RlpDecode)
    }
