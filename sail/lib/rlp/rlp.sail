
/* Minimal big-endian encodings are built in one pass. Consing successive low
 * bytes puts them in big-endian order and gives us the encoded length without
 * a second list traversal. */
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
field reference retains its source, full span, and content span without
copying.

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
/*! Decrements a positive RLP length width. */
function rlp_length_width_decrement(value : range(0, 8)) -> range(0, 8) = {
    assert(0 < value);
    value - 1
}

/*! The minimal big-endian encoding of a word and its length. */
val minimal_word_bytes : forall 'word, 0 <= 'word & 'word < 2 ^ 256. int('word) -> (list(byte), range(0, 32))

/*! Converts a zero-based word-byte index to its one-based byte count. */
val word_byte_count : forall 'index, 0 <= 'index & 'index <= 31. int('index) -> int('index + 1)

function word_byte_count(index) = index + 1

/*! Materializes a word's minimal big-endian byte sequence and its length. */
function minimal_word_bytes(w) = {
    var remaining : word = w;
    var out : list(byte) = [||];
    var len : range(0, 32) = 0;
    foreach (byte_index from 0 to 31) {
        if word_nonzero(remaining) then {
            out = word_low_byte(remaining) :: out;
            remaining = word_shift_right(remaining, 8);
            len = word_byte_count(byte_index)
        }
    };
    (out, len)
}

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

/*! The RLP string prefix for a payload of `len` bytes whose first byte
    is `first`; single bytes below `0x80` need no prefix. */
val rlp_string_prefix : forall 'len, 0 <= 'len. (int('len), byte) -> (list(byte), range(0, 33))
function rlp_string_prefix(len, first) =
    if (len == 1) & (first[7] == bitzero) then {
        ([||], 0)
    } else {
        if len <= RLP_SHORT_LENGTH_LIMIT then {
            ([|add_bits(0x80, rlp_length_byte(len))|], 1)
        } else {
            let (length_bytes, length_len) = minimal_word_bytes(rlp_length_word(len));
            (add_bits(0xb7, rlp_length_byte(length_len)) :: length_bytes, 1 + length_len)
        }
    }

/*! The RLP list header for a payload of `len` content bytes. */
val rlp_list_prefix : forall 'len, 0 <= 'len. int('len) -> (list(byte), range(1, 33))
function rlp_list_prefix(len) =
    if len <= RLP_SHORT_LENGTH_LIMIT then {
        ([|add_bits(0xc0, rlp_length_byte(len))|], 1)
    } else {
        let (length_bytes, length_len) = minimal_word_bytes(rlp_length_word(len));
        (add_bits(0xf7, rlp_length_byte(length_len)) :: length_bytes, 1 + length_len)
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

/*! Returns the encoded RLP width of a materialized byte sequence. */
val rlp_bytes_size : forall 'len, 0 <= 'len. (list(byte), int('len)) -> range(1, 'len + 33)
function rlp_bytes_size(data, len) = {
    let first : byte = match data {
        b :: _ => b,
        [||] => 0x00,
    };
    rlp_string_size(len, first)
}

/*! Returns the encoded RLP width of a source-backed byte slice. */
val rlp_slice_size : forall 'off 'len, source_valid_range('off, 'len). ByteSliceFields('off, 'len) -> range(
        1,
        'len + 33,
    )
function rlp_slice_size(data) = {
    let len = data.len;
    let first = if len == 0 then {
            0x00
        } else {
            slice_byte(data, 0)
        };
    rlp_string_size(len, first)
}

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
function rlp_scratch_slice_size(data : ByteSlice) -> rlp_scratch_length =
    rlp_slice_size(data)

/*! Returns the materializable RLP width of a byte list and declared source
    length. */
val rlp_scratch_bytes_size : forall 'len, source_valid_length('len). (list(byte), int('len)) -> rlp_scratch_length
function rlp_scratch_bytes_size(data, len) =
    rlp_bytes_size(data, len)

val rlp_write_raw_bytes : forall 'len, source_valid_length('len). (list(byte), int('len)) -> unit
function rlp_write_raw_bytes(data, len) =
    scratch_push_bytes(data, len)

function rlp_write_raw_slice(data : ByteSlice) -> unit =
    scratch_push_slice(data)

/*! Appends the canonical RLP prefix of a byte-string payload. */
val rlp_write_string_prefix : forall 'len, 0 <= 'len. (int('len), byte) -> unit
function rlp_write_string_prefix(len, first) = {
    let (encoded_prefix, prefix_len) = rlp_string_prefix(len, first);
    scratch_push_bytes(encoded_prefix, prefix_len)
}

/*! Appends the canonical RLP prefix of a list payload. */
val rlp_write_list_prefix : forall 'len, 0 <= 'len. int('len) -> unit
function rlp_write_list_prefix(content_len) = {
    let (encoded_prefix, prefix_len) = rlp_list_prefix(content_len);
    scratch_push_bytes(encoded_prefix, prefix_len)
}

/*! Appends a materialized byte sequence as an RLP string. */
val rlp_write_bytes : forall 'len, source_valid_length('len). (list(byte), int('len)) -> unit
function rlp_write_bytes(data, len) = {
    let first : byte = match data {
        b :: _ => b,
        [||] => 0x00,
    };
    rlp_write_string_prefix(len, first);
    scratch_push_bytes(data, len)
}

/*! Appends a source-backed byte slice as an RLP string. */
function rlp_write_slice(data : ByteSlice) -> unit = {
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

/*! Appends a word as a minimal RLP integer. */
val rlp_write_uint_word : forall 'word, 0 <= 'word & 'word < 2 ^ 256. int('word) -> unit
function rlp_write_uint_word(w) = {
    let (bytes, len) = minimal_word_bytes(w);
    rlp_write_bytes(bytes, len)
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

function rlp_write_word(w : word) -> unit =
    rlp_write_bytes(word_to_bytes32(w), WORD_BYTE_LENGTH)

function rlp_write_addr(a : address) -> unit =
    rlp_write_bytes(address_to_bytes(a), ADDRESS_BYTE_LENGTH)

/*md
## The decoders
*/
/*! Finishes a forward scratch encoding. */
function rlp_finish(start : source_pointer) -> ByteSlice =
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
function rlp_uint64_width forall 'n, 0 <= 'n <= 8. (content : ByteSlice, width : int('n)) -> range(0, 2 ^ (8 * 'n) - 1) =
    if width == 0 then {
        0
    } else {
        rlp_uint64_append(width, rlp_uint64_width(content, width - 1), slice_byte(content, width - 1))
    }
termination_measure rlp_uint64_width(content, width) = width

/*! Decodes a big-endian RLP length-of-length field of at most eight bytes. */
val rlp_ref_be_length : forall 'start 'stop 'count,
                          source_valid_length('start) & source_valid_length('stop) & 0 <= 'count & 'count <= 8. (
        ByteSlice,
        int('start),
        int('stop),
        int('count),
    ) -> range(0, 2 ^ 64 - 1)
function rlp_ref_be_length(source, start, stop, count) = {
    let source_length = source.len;
    let (start_value as 'start_value) = start;
    let (stop_value as 'stop_value) = stop;
    if stop_value < start_value then {
        throw InvalidBlock(RlpDecode)
    } else {
        if source_length < stop_value then {
            throw InvalidBlock(RlpDecode)
        } else {
            if stop_value - start_value < count then {
                throw InvalidBlock(RlpDecode)
            } else {
                let content = sub_slice(source, start_value, count);
                rlp_uint64_width(content, count)
            }
        }
    }
}

/* An RLP length-of-length is encoded in one prefix byte, so it is at most
 * eight bytes for every source length representable by a ByteSlice. */
/*! Tests a bounded expected byte sequence against a source position. */
function rlp_bytes_equal_at(expected : list(byte), source : ByteSlice, start : source_pointer) -> bool = {
    var rest : list(byte) = expected;
    var current : source_pointer = start;
    var equal : bool = true;
    foreach (byte_index from 0 to 7) {
        match rest {
            b :: tail => {
                let position = current;
                let source_length = source.len;
                if position < source_length then {
                    equal = equal & (slice_byte(source, current) == b);
                    current = position + 1
                } else {
                    equal = false
                };
                rest = tail
            },
            [||] => (),
        }
    };
    match rest {
        [||] => equal,
        _ => false,
    }
}

/*! Decodes the RLP header at `pos`: `(is_list, content_offset,
    content_length)`. */
val rlp_ref_hdr : forall 'pos 'stop, source_valid_length('pos) & source_valid_length('stop). (
        ByteSlice,
        int('pos),
        int('stop),
    ) -> (bool, source_pointer, range(0, 2 ^ 64 - 1))
function rlp_ref_hdr(b, pos, stop) = {
    if not_bool(pos < stop) then {
        throw InvalidBlock(RlpDecode)
    };
    let h = unsigned(slice_byte(b, pos));
    if h < 128 then {
        (false, pos, 1)
    } else {
        let position = pos;
        let stop_position = stop;
        if stop_position <= position then {
            throw InvalidBlock(RlpDecode)
        };
        let length_start : source_pointer = position + 1;
        if h < 184 then {
            (false, length_start, h - 128)
        } else {
            if h < 192 then {
                let length_width : range(1, 8) = h - 183;
                if stop_position - (position + 1) < length_width then {
                    throw InvalidBlock(RlpDecode)
                };
                let content : source_pointer = position + 1 + length_width;
                (false, content, rlp_ref_be_length(b, length_start, content, length_width))
            } else {
                if h < 248 then {
                    (true, length_start, h - 192)
                } else {
                    let length_width : range(1, 8) = h - 247;
                    if stop_position - (position + 1) < length_width then {
                        throw InvalidBlock(RlpDecode)
                    };
                    let content : source_pointer = position + 1 + length_width;
                    (true, content, rlp_ref_be_length(b, length_start, content, length_width))
                }
            }
        }
    }
}

/*! Constructs an [RlpFieldRef][type-RlpFieldRef] from validated offsets. */
val rlp_field_ref : forall 'source_off 'source_len 'full_off 'full_len 'content_off 'content_len,
                         source_valid_range('source_off, 'source_len)
                      &  0
                      <= 'full_off
                      &  0
                      <= 'full_len
                      &  'full_off
                      +  'full_len
                      <= 'source_len
                      &  0
                      <= 'content_off
                      &  0
                      <= 'content_len
                      &  'content_off
                      +  'content_len
                      <= 'source_len. (
        ByteSliceFields('source_off, 'source_len),
        bool,
        int('full_off),
        int('full_len),
        int('content_off),
        int('content_len),
    ) -> RlpFieldRef
function rlp_field_ref(source, is_list : bool, full_off, full_len, content_off, content_len) = {
    let fields : RlpFieldRefFields('source_off, 'source_len, 'full_off, 'full_len, 'content_off, 'content_len) = struct {
            source = source,
            is_list = is_list,
            full_off = full_off,
            full_len = full_len,
            content_off = content_off,
            content_len = content_len,
        };
    fields
}

/*! Constructs a cursor whose bounds are contained by its source. */
val rlp_cursor : forall 'source_off 'source_len 'current 'stop,
                      source_valid_range('source_off, 'source_len)
                   &  0
                   <= 'current
                   &  'current
                   <= 'stop
                   &  'stop
                   <= 'source_len. (ByteSliceFields('source_off, 'source_len), int('current), int('stop), bool) -> RlpCursor
function rlp_cursor(source, current, stop, valid) = {
    let fields : RlpCursorFields('source_off, 'source_len, 'current, 'stop) = struct {
            source = source,
            current = current,
            stop = stop,
            valid = valid,
        };
    fields
}

/*! The invalid cursor; popping it throws `InvalidBlock(RlpDecode)`. */
function rlp_invalid_cursor(source : ByteSlice) -> RlpCursor =
    rlp_cursor(source, 0, 0, false)

/*! A cursor over the children of a byte sequence that must be exactly
    one RLP list (e.g. a trie node). */
function rlp_node_cursor(node : ByteSlice) -> RlpCursor = {
    let node_length = node.len;
    if node_length == 0 then {
        rlp_invalid_cursor(node)
    } else {
        let node_stop = node_length;
        let (is_list, content, content_len) = rlp_ref_hdr(node, 0, node_stop);
        let content_offset = content;
        let payload_length = content_len;
        if content_offset <= node_length then {
            if payload_length <= node_length - content_offset then {
                let content_stop = content_offset + payload_length;
                if is_list then {
                    if content_stop == node_stop then {
                        rlp_cursor(node, content, content_stop, true)
                    } else {
                        rlp_invalid_cursor(node)
                    }
                } else {
                    rlp_invalid_cursor(node)
                }
            } else {
                rlp_invalid_cursor(node)
            }
        } else {
            rlp_invalid_cursor(node)
        }
    }
}

/*! A cursor over a decoded field's children; throws if the field is not
    a list. */
val rlp_ref_cursor : forall 'source_off 'source_len 'full_off 'full_len 'content_off 'content_len,
                          source_valid_range('source_off, 'source_len)
                       &  0
                       <= 'full_off
                       &  0
                       <= 'full_len
                       &  'full_off
                       +  'full_len
                       <= 'source_len
                       &  0
                       <= 'content_off
                       &  0
                       <= 'content_len
                       &  'content_off
                       +  'content_len
                       <= 'source_len. RlpFieldRefFields(
        'source_off,
        'source_len,
        'full_off,
        'full_len,
        'content_off,
        'content_len,
    ) -> {'cursor_source_off 'cursor_source_len 'cursor_current 'cursor_stop,
             source_valid_range('cursor_source_off, 'cursor_source_len)
          &  0
          <= 'cursor_current
          &  'cursor_current
          <= 'cursor_stop
          &  'cursor_stop
          <= 'cursor_source_len
          &  'cursor_stop
          -  'cursor_current
          <= 'full_len.
          RlpCursorFields('cursor_source_off, 'cursor_source_len, 'cursor_current, 'cursor_stop)}

function rlp_ref_cursor(f) =
    if f.is_list then {
        let content_offset = f.content_off;
        let content_length = f.content_len;
        let source_length = f.source.len;
        if content_offset <= source_length then {
            if content_length <= source_length - content_offset then {
                let stop = content_offset + content_length;
                let cursor = rlp_cursor(f.source, content_offset, stop, true);
                let cursor_span = cursor.stop - cursor.current;
                let full_length = f.full_len;
                if cursor_span <= full_length then {
                    cursor
                } else {
                    throw InvalidBlock(RlpDecode)
                }
            } else {
                throw InvalidBlock(RlpDecode)
            }
        } else {
            throw InvalidBlock(RlpDecode)
        }
    } else {
        throw InvalidBlock(RlpDecode)
    }

/*! Whether the cursor has consumed all children. */
function rlp_cursor_empty(cursor : RlpCursor) -> bool =
    cursor.valid & (cursor.current == cursor.stop)

/*! Decodes the next child and advances; throws on truncated input. */
val rlp_cursor_pop : forall 'source_off 'source_len 'current 'stop,
                          source_valid_range('source_off, 'source_len)
                       &  0
                       <= 'current
                       &  'current
                       <= 'stop
                       &  'stop
                       <= 'source_len. RlpCursorFields('source_off, 'source_len, 'current, 'stop) -> {'content 'content_len 'next 'full_len,
                                                                                                         0
                                                                                                      <= 'content
                                                                                                      &  0
                                                                                                      <= 'content_len
                                                                                                      &  'next
                                                                                                      == 'content
                                                                                                      +  'content_len
                                                                                                      &  'current
                                                                                                      <  'next
                                                                                                      &  'next
                                                                                                      <= 'stop
                                                                                                      &  'full_len
                                                                                                      == 'next
                                                                                                      -  'current.
                                                                                                      (
                                                                                                          RlpFieldRefFields(
                                                                                                              'source_off,
                                                                                                              'source_len,
                                                                                                              'current,
                                                                                                              'full_len,
                                                                                                              'content,
                                                                                                              'content_len,
                                                                                                          ),
                                                                                                          RlpCursorFields(
                                                                                                              'source_off,
                                                                                                              'source_len,
                                                                                                              'next,
                                                                                                              'stop,
                                                                                                          ),
                                                                                                      )}
function rlp_cursor_pop(cursor) = {
    let current = cursor.current;
    let stop = cursor.stop;
    if not_bool(cursor.valid) | not_bool(current < stop) then {
        throw InvalidBlock(RlpDecode)
    };
    let (is_list, content_value, content_len_value) = rlp_ref_hdr(cursor.source, current, stop);
    let (content as 'content) = content_value;
    let (content_len as 'content_len) = content_len_value;
    let content_offset = content;
    let payload_length = content_len;
    let stop_offset = stop;
    if stop_offset < content_offset then {
        throw InvalidBlock(RlpDecode)
    };
    if stop_offset - content_offset < payload_length then {
        throw InvalidBlock(RlpDecode)
    };
    let (next as 'next) = content_offset + payload_length;
    let current_offset = current;
    let next_offset = next;
    if current_offset < next_offset then {
        let (full_len as 'full_len) = next_offset - current_offset;
        let field : RlpFieldRefFields('source_off, 'source_len, 'current, 'full_len, 'content, 'content_len) = struct {
                source = cursor.source,
                is_list = is_list,
                full_off = current,
                full_len = full_len,
                content_off = content,
                content_len = content_len,
            };
        let advanced : RlpCursorFields('source_off, 'source_len, 'next, 'stop) = struct {
                source = cursor.source,
                current = next,
                stop = stop,
                valid = true,
            };
        (field, advanced)
    } else {
        throw InvalidBlock(RlpDecode)
    }
}

/*! Asserts the cursor is exhausted — trailing bytes are a decode
    error. */
function rlp_cursor_expect_end(cursor : RlpCursor) -> unit =
    if rlp_cursor_empty(cursor) then {
        ()
    } else {
        throw InvalidBlock(RlpDecode)
    }

/*! Decodes a byte sequence that must be exactly one RLP item. */
function rlp_single_ref(item : ByteSlice) -> RlpFieldRef = {
    let item_length = item.len;
    if item_length == 0 then {
        throw InvalidBlock(RlpDecode)
    } else {
        let item_stop = item_length;
        let (isl, cs, cl) = rlp_ref_hdr(item, 0, item_stop);
        let content_offset = cs;
        let content_length = cl;
        if content_offset > item_length then {
            throw InvalidBlock(RlpDecode)
        };
        if content_length != item_length - content_offset then {
            throw InvalidBlock(RlpDecode)
        };
        rlp_field_ref(item, isl, 0, item.len, cs, cl)
    }
}

/*! The content span of a field. */
val rlp_ref_content : forall 'source_off 'source_len 'full_off 'full_len 'content_off 'content_len,
                           source_valid_range('source_off, 'source_len)
                        &  0
                        <= 'full_off
                        &  0
                        <= 'full_len
                        &  'full_off
                        +  'full_len
                        <= 'source_len
                        &  0
                        <= 'content_off
                        &  0
                        <= 'content_len
                        &  'content_off
                        +  'content_len
                        <= 'source_len. RlpFieldRefFields(
        'source_off,
        'source_len,
        'full_off,
        'full_len,
        'content_off,
        'content_len,
    ) -> ByteSliceFields('source_off + 'content_off, 'content_len)
function rlp_ref_content(f) =
    sub_slice(f.source, f.content_off, f.content_len)

/*! The full encoded span of a field. */
function rlp_ref_full(f : RlpFieldRef) -> ByteSlice =
    sub_slice(f.source, f.full_off, f.full_len)

/*! Whether a field uses the unique canonical RLP framing for its payload. */
function rlp_ref_framing_canonical(f : RlpFieldRef) -> bool = {
    let n = f.content_len;
    let payload_length = n;
    let full_length = f.full_len;
    let fo = f.full_off;
    let full_offset = fo;
    let co = f.content_off;
    let source = f.source;
    let source_length = source.len;
    if full_length == 0 then {
        false
    } else {
        if f.is_list then {
            if n <= RLP_SHORT_LENGTH_LIMIT then {
                (full_length == payload_length + 1) & (slice_byte(f.source, fo) == add_bits(0xc0, rlp_length_byte(n)))
            } else {
                let (len_bytes, length_width) = minimal_word_bytes(rlp_length_word(n));
                  (full_length == 1 + length_width + payload_length)
                & (slice_byte(f.source, fo) == add_bits(0xf7, rlp_length_byte(length_width)))
                & (if full_offset < source_length then {
                      rlp_bytes_equal_at(len_bytes, f.source, full_offset + 1)
                  } else {
                      false
                  })
            }
        } else {
            if payload_length == 0 then {
                (full_length == 1) & (slice_byte(f.source, fo) == 0x80)
            } else {
                let first = slice_byte(f.source, co);
                if (payload_length == 1) & (first[7] == bitzero) then {
                    (full_length == 1) & (slice_byte(f.source, fo) == first)
                } else {
                    if n <= RLP_SHORT_LENGTH_LIMIT then {
                        (full_length == payload_length + 1) &
                            (slice_byte(f.source, fo) == add_bits(0x80, rlp_length_byte(n)))
                    } else {
                        let (len_bytes, length_width) = minimal_word_bytes(rlp_length_word(n));
                          (full_length == 1 + length_width + payload_length)
                        & (slice_byte(f.source, fo) == add_bits(0xb7, rlp_length_byte(length_width)))
                        & (if full_offset < source_length then {
                              rlp_bytes_equal_at(len_bytes, f.source, full_offset + 1)
                          } else {
                              false
                          })
                    }
                }
            }
        }
    }
}

function rlp_ref_bytes_canonical(f : RlpFieldRef) -> bool =
    not_bool(f.is_list) & rlp_ref_framing_canonical(f)

/*! Whether a field is the canonical RLP encoding of a non-negative
    integer: minimal big-endian content with no leading zeros and the
    exact matching prefix. */
function rlp_ref_uint_canonical(f : RlpFieldRef) -> bool =
    if not_bool(rlp_ref_bytes_canonical(f)) then {
        false
    } else {
        (f.content_len == 0) | (slice_byte(f.source, f.content_off) != 0x00)
    }

/*! Returns a canonical byte-string field of exactly `len` bytes. */
function rlp_ref_fixed_bytes(f : RlpFieldRef, len : nat) -> ByteSlice =
    if rlp_ref_bytes_canonical(f) & (f.content_len == len) then {
        rlp_ref_content(f)
    } else {
        throw InvalidBlock(RlpDecode)
    }

/*! Decodes a string field of at most 32 bytes into a word. */
function rlp_ref_word(f : RlpFieldRef) -> word = {
    let n = f.content_len;
    if f.is_list | (RLP_WORD_LENGTH_LIMIT < n) then {
        throw InvalidBlock(RlpDecode)
    } else {
        slice_load_n(f.source, f.content_off, n)
    }
}

/*! Decodes a canonical unsigned integer field into a word; throws
    otherwise. */
function rlp_ref_uint_word(f : RlpFieldRef) -> word =
    if rlp_ref_uint_canonical(f) then {
        rlp_ref_word(f)
    } else {
        throw InvalidBlock(RlpDecode)
    }

/*! Decodes a canonical unsigned integer into the uint64 wire domain used by
    EIP-2681 account nonces and EIP-4844 excess blob gas. */
function rlp_ref_uint64(f : RlpFieldRef) -> ssz_uint = {
    if not_bool(rlp_ref_uint_canonical(f)) | (RLP_UINT64_LENGTH_LIMIT < f.content_len) then {
        throw InvalidBlock(RlpDecode)
    };
    let content_length = f.content_len;
    let content = rlp_ref_content(f);

    /* The semantic check above establishes content_length <= 8.  Modulo nine
     * only exposes that already-proven bound to Sail's dependent type system. */
    let width : range(0, 8) = tmod_nat(content_length, 9);
    rlp_uint64_width(content, width)
}

/*! Decodes an arbitrary-width big-endian natural. */
function rlp_uint_content(content : ByteSlice) -> nat = {
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
function rlp_ref_uint(f : RlpFieldRef) -> nat =
    if rlp_ref_uint_canonical(f) then {
        rlp_uint_content(rlp_ref_content(f))
    } else {
        throw InvalidBlock(RlpDecode)
    }

/*! Typed RLP boundaries document and enforce the field's protocol domain. */
function rlp_ref_account_nonce(f : RlpFieldRef) -> account_nonce = rlp_ref_uint64(f)
function rlp_ref_block_number(f : RlpFieldRef) -> block_number = rlp_ref_uint(f)
/*! Decodes block blob gas after enforcing the supported schedule bound. */
function rlp_ref_blob_gas_used(f : RlpFieldRef) -> blob_gas_used = {
    let value = rlp_ref_uint64(f);
    if value <= sizeof(blob_count_bound * gas_per_blob_value) then {
        value
    } else {
        throw InvalidBlock(RlpDecode)
    }
}
function rlp_ref_excess_blob_gas(f : RlpFieldRef) -> excess_blob_gas = rlp_ref_uint64(f)
function rlp_ref_chain_identifier(f : RlpFieldRef) -> chain_identifier = rlp_ref_uint(f)
