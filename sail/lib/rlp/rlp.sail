
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
let RLP_SHORT_LENGTH_LIMIT : byte_length = ByteQuantity(55)
let RLP_WORD_LENGTH_LIMIT : byte_length = WORD_BYTE_LENGTH
let RLP_ENCODED_WORD_LENGTH : byte_length = ByteQuantity(33)
let RLP_ENCODED_ADDRESS_LENGTH : byte_length = ByteQuantity(21)

/*md
## The encoders
*/
/*! Decrements a positive RLP length width. */
function rlp_length_width_decrement(value : range(0, 8)) -> range(0, 8) = {
    assert(0 < value);
    value - 1
}

/*! Returns the minimal big-endian byte sequence of a protocol quantity and
    its length. */
function minimal_protocol_quantity_bytes(n : protocol_quantity) -> (list(byte), range(0, 8)) = {
    var remaining : protocol_quantity = n;
    var out : list(byte) = [||];
    var len : range(0, 8) = 0;
    foreach (byte_index from 0 to 7) {
        if remaining != 0 then {
            out = get_slice_int(8, tmod_int(remaining, 256), 0) :: out;
            remaining = protocol_quantity_quotient(remaining, 256);
            len = byte_index + 1
        }
    };
    (out, len)
}

/*! Returns the minimal big-endian byte sequence of a gas quantity and its
    length. */
function minimal_gas_bytes(Gas(n) : gas) -> (list(byte), range(0, 8)) = {
    var remaining : gas = Gas(n);
    var out : list(byte) = [||];
    var len : range(0, 8) = 0;
    foreach (byte_index from 0 to 7) {
        let Gas(remaining_value) = remaining;
        if remaining_value != 0 then {
            out = get_slice_int(8, tmod_int(remaining_value, 256), 0) :: out;
            remaining = gas_quotient(remaining, 256);
            len = byte_index + 1
        }
    };
    (out, len)
}

/*! The minimal big-endian encoding of a word and its length. */
function minimal_word_bytes(w : word) -> (list(byte), range(0, 32)) = {
    var remaining : word = w;
    var out : list(byte) = [||];
    var len : range(0, 32) = 0;
    foreach (byte_index from 0 to 31) {
        if word_nonzero(remaining) then {
            out = word_low_byte(remaining) :: out;
            remaining = word_shift_right_limb(remaining, 0x0000000000000008);
            len = byte_index + 1
        }
    };
    (out, len)
}

/* Wire-format conversions stay with RLP rather than defining arithmetic over
 * every byte quantity. Callers establish the stated bounds before encoding. */
function rlp_nat_length_byte(value : range(0, 255)) -> byte =
    get_slice_int(8, value, 0)

/*! Narrows a previously bounded byte length for an RLP prefix byte. */
function rlp_byte_length_byte(value : byte_length) -> byte = {
    assert(value <= ByteQuantity(255));
    to_limb(value)[7 .. 0]
}

overload rlp_length_byte = { rlp_nat_length_byte, rlp_byte_length_byte }

function rlp_length_word(value : byte_length) -> word = {
    word_of_quantity(value)
}

/*! The RLP string prefix for a payload of `len` bytes whose first byte
    is `first`; single bytes below `0x80` need no prefix. */
function rlp_string_prefix(len : byte_length, first : byte) -> (list(byte), byte_length) =
    if (len == BYTE_ONE) & (first[7] == bitzero) then {
        ([||], BYTE_ZERO)
    } else {
        if len <= RLP_SHORT_LENGTH_LIMIT then {
            ([|add_bits(0x80, rlp_length_byte(len))|], BYTE_ONE)
        } else {
            let (length_bytes, length_len) = minimal_word_bytes(rlp_length_word(len));
            (add_bits(0xb7, rlp_length_byte(length_len)) :: length_bytes, ByteQuantity(1 + length_len))
        }
    }

/*! The RLP list header for a payload of `len` content bytes. */
function rlp_list_prefix(len : byte_length) -> (list(byte), byte_length) =
    if len <= RLP_SHORT_LENGTH_LIMIT then {
        ([|add_bits(0xc0, rlp_length_byte(len))|], BYTE_ONE)
    } else {
        let (length_bytes, length_len) = minimal_word_bytes(rlp_length_word(len));
        (add_bits(0xf7, rlp_length_byte(length_len)) :: length_bytes, ByteQuantity(1 + length_len))
    }

/*! Returns the minimal big-endian byte length of a 256-bit word. */
function rlp_minimal_word_len(w : word) -> range(0, 32) = {
    var remaining : word = w;
    var len : range(0, 32) = 0;
    foreach (byte_index from 0 to 31) {
        if word_nonzero(remaining) then {
            remaining = word_shift_right_limb(remaining, 0x0000000000000008);
            len = byte_index + 1
        }
    };
    len
}

/*! Returns the minimal big-endian byte length of a protocol quantity. */
function rlp_minimal_protocol_quantity_len(n : protocol_quantity) -> range(0, 8) = {
    var remaining : protocol_quantity = n;
    var len : range(0, 8) = 0;
    foreach (byte_index from 0 to 7) {
        if remaining != 0 then {
            remaining = protocol_quantity_quotient(remaining, 256);
            len = byte_index + 1
        }
    };
    len
}

/*! Returns the minimal big-endian byte length of a gas quantity. */
function rlp_minimal_gas_len(Gas(n) : gas) -> range(0, 8) = {
    var remaining : gas = Gas(n);
    var len : range(0, 8) = 0;
    foreach (byte_index from 0 to 7) {
        let Gas(remaining_value) = remaining;
        if remaining_value != 0 then {
            remaining = gas_quotient(remaining, 256);
            len = byte_index + 1
        }
    };
    len
}

/*! Returns the RLP header width for a string or list payload length. */
function rlp_length_prefix_len(len : byte_length) -> byte_length =
    if len <= RLP_SHORT_LENGTH_LIMIT then {
        BYTE_ONE
    } else {
        ByteQuantity(1 + rlp_minimal_word_len(rlp_length_word(len)))
    }

/*! Returns the encoded RLP width of a byte string from its length and first
    byte. */
function rlp_string_size(len : byte_length, first : byte) -> byte_length =
    if (len == BYTE_ONE) & (first[7] == bitzero) then {
        BYTE_ONE
    } else {
        rlp_length_prefix_len(len) + len
    }

/*! Returns the encoded RLP width of a materialized byte sequence. */
function rlp_bytes_size(data : list(byte), len : byte_length) -> byte_length = {
    let first : byte = match data {
        b :: _ => b,
        [||] => 0x00,
    };
    rlp_string_size(len, first)
}

/*! Returns the encoded RLP width of a source-backed byte slice. */
function rlp_slice_size(data : ByteSlice) -> byte_length =
    rlp_string_size(
        data.len,
        if data.len == BYTE_ZERO then {
            0x00
        } else {
            slice_byte(data, BYTE_ZERO)
        },
    )

/*! Returns the encoded RLP width of a minimally represented word integer. */
function rlp_uint_word_size(w : word) -> byte_length = {
    let len = rlp_minimal_word_len(w);
    rlp_string_size(
        ByteQuantity(len),
        if len == 1 then {
            word_low_byte(w)
        } else {
            0x00
        },
    )
}

/*! Returns the encoded RLP width of a protocol quantity. */
function rlp_protocol_quantity_size(n : protocol_quantity) -> byte_length = {
    let len = rlp_minimal_protocol_quantity_len(n);
    rlp_string_size(
        ByteQuantity(len),
        if len == 1 then {
            get_slice_int(8, n, 0)
        } else {
            0x00
        },
    )
}

/*! Returns the encoded RLP width of a gas quantity. */
function rlp_gas_size(value : gas) -> byte_length = {
    let Gas(n) = value;
    let len = rlp_minimal_gas_len(value);
    rlp_string_size(
        ByteQuantity(len),
        if len == 1 then {
            get_slice_int(8, n, 0)
        } else {
            0x00
        },
    )
}

overload rlp_uint_size = {rlp_protocol_quantity_size, rlp_gas_size}

function rlp_word_size() -> byte_length = RLP_ENCODED_WORD_LENGTH
function rlp_addr_size() -> byte_length = RLP_ENCODED_ADDRESS_LENGTH

function rlp_list_size(content_len : byte_length) -> byte_length =
    rlp_length_prefix_len(content_len) + content_len

function rlp_write_raw_bytes(data : list(byte), len : byte_length) -> unit =
    scratch_push_bytes(data, len)

function rlp_write_raw_slice(data : ByteSlice) -> unit =
    scratch_push_slice(data)

/*! Appends the canonical RLP prefix of a byte-string payload. */
function rlp_write_string_prefix(len : byte_length, first : byte) -> unit = {
    let (encoded_prefix, prefix_len) = rlp_string_prefix(len, first);
    scratch_push_bytes(encoded_prefix, prefix_len)
}

/*! Appends the canonical RLP prefix of a list payload. */
function rlp_write_list_prefix(content_len : byte_length) -> unit = {
    let (encoded_prefix, prefix_len) = rlp_list_prefix(content_len);
    scratch_push_bytes(encoded_prefix, prefix_len)
}

/*! Appends a materialized byte sequence as an RLP string. */
function rlp_write_bytes(data : list(byte), len : byte_length) -> unit = {
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
        if data.len == BYTE_ZERO then {
            0x00
        } else {
            slice_byte(data, BYTE_ZERO)
        },
    );
    scratch_push_slice(data)
}

/*! Appends a word as a minimal RLP integer. */
function rlp_write_uint_word(w : word) -> unit = {
    let (bytes, len) = minimal_word_bytes(w);
    rlp_write_bytes(bytes, ByteQuantity(len))
}

/*! Appends a protocol quantity as a minimal RLP integer. */
function rlp_write_protocol_quantity(n : protocol_quantity) -> unit = {
    let (bytes, len) = minimal_protocol_quantity_bytes(n);
    rlp_write_bytes(bytes, ByteQuantity(len))
}

/*! Appends a gas quantity as a minimal RLP integer. */
function rlp_write_gas(value : gas) -> unit = {
    let (bytes, len) = minimal_gas_bytes(value);
    rlp_write_bytes(bytes, ByteQuantity(len))
}

overload rlp_write_uint = {rlp_write_protocol_quantity, rlp_write_gas}

function rlp_write_word(w : word) -> unit =
    rlp_write_bytes(word_to_bytes32(w), WORD_BYTE_LENGTH)

function rlp_write_addr(a : address) -> unit =
    rlp_write_bytes(address_to_bytes(a), ADDRESS_BYTE_LENGTH)

/*md
## The decoders
*/
/*! Finishes a forward scratch encoding and verifies its predicted width. */
function rlp_finish(start : source_pointer, expected_len : byte_length) -> ByteSlice = {
    let encoded = scratch_finish(start);
    assert(encoded.len == expected_len, "RLP forward writer length");
    encoded
}

/* Malformed RLP is a normal invalid-block outcome: the decoders throw
 * InvalidBlock(RlpDecode) at detection and the guest catches it once at the
 * top level. */

/*! Adds an RLP source offset and length, rejecting values outside the byte
    domain. */
function rlp_checked_add_quantity(a : source_pointer, b : byte_length) -> source_pointer = {
    if a <= MAX_BYTE_QUANTITY then {
        if b <= MAX_BYTE_QUANTITY - a then {
            a + b
        } else {
            throw InvalidBlock(RlpDecode)
        }
    } else {
        throw InvalidBlock(RlpDecode)
    }
}

/*! Adds a bounded natural displacement to an RLP source offset. */
function rlp_checked_add_nat(a : source_pointer, b : nat) -> source_pointer =
    rlp_checked_add_quantity(a, ByteQuantity(b))

overload rlp_checked_add = { rlp_checked_add_quantity, rlp_checked_add_nat }

/* Decode an RLP length-of-length without entering the arbitrary-precision
 * integer domain. `remaining` is derived from one prefix byte and is at most
 * eight; the fixed loop therefore covers every ByteSlice-representable span. */
/*! Decodes a big-endian RLP length-of-length field of at most eight bytes. */
function rlp_ref_be_length(source : ByteSlice, start : source_pointer, count : range(0, 8)) -> byte_length = {
    var value : limb = LIMB_ZERO;
    var current : source_pointer = start;
    var remaining : range(0, 8) = count;
    foreach (_byte_index from 0 to 7) {
        if remaining != 0 then {
            value = or_vec(sail_shiftleft(value, 8), sail_zero_extend(slice_byte(source, current), 64));
            current = rlp_checked_add(current, BYTE_ONE);
            remaining = rlp_length_width_decrement(remaining)
        }
    };
    ByteQuantity(unsigned(value))
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
                equal = equal & (slice_byte(source, current) == b);
                current = rlp_checked_add(current, BYTE_ONE);
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
function rlp_ref_hdr(b : ByteSlice, pos : source_pointer, stop : source_pointer) -> (bool, source_pointer, byte_length) = {
    if not_bool(pos < stop) then {
        throw InvalidBlock(RlpDecode)
    };
    let h = unsigned(slice_byte(b, pos));
    if h < 128 then {
        (false, pos, BYTE_ONE)
    } else {
        if h < 184 then {
            (false, rlp_checked_add(pos, BYTE_ONE), ByteQuantity(h - 128))
        } else {
            if h < 192 then {
                let length_width : range(1, 8) = h - 183;
                let length_start = rlp_checked_add(pos, BYTE_ONE);
                let content = rlp_checked_add(length_start, ByteQuantity(length_width));
                if stop < content then {
                    throw InvalidBlock(RlpDecode)
                };
                (false, content, rlp_ref_be_length(b, length_start, length_width))
            } else {
                if h < 248 then {
                    (true, rlp_checked_add(pos, BYTE_ONE), ByteQuantity(h - 192))
                } else {
                    let length_width : range(1, 8) = h - 247;
                    let length_start = rlp_checked_add(pos, BYTE_ONE);
                    let content = rlp_checked_add(length_start, ByteQuantity(length_width));
                    if stop < content then {
                        throw InvalidBlock(RlpDecode)
                    };
                    (true, content, rlp_ref_be_length(b, length_start, length_width))
                }
            }
        }
    }
}

/*! Constructs an [RlpFieldRef][type-RlpFieldRef] from integer offsets. */
function rlp_field_ref(
    source : ByteSlice,
    is_list : bool,
    full_off : source_pointer,
    full_len : byte_length,
    content_off : source_pointer,
    content_len : byte_length,
) -> (
    RlpFieldRef
) =
    struct {
        source = source,
        is_list = is_list,
        full_off = full_off,
        full_len = full_len,
        content_off = content_off,
        content_len = content_len,
    }

/*! The invalid cursor; popping it throws `InvalidBlock(RlpDecode)`. */
function rlp_invalid_cursor(source : ByteSlice) -> RlpCursor =
    struct { source = source, current = BYTE_ZERO, stop = BYTE_ZERO, valid = false }

/*! A cursor over the children of a byte sequence that must be exactly
    one RLP list (e.g. a trie node). */
function rlp_node_cursor(node : ByteSlice) -> RlpCursor = {
    let n = node.len;
    if n == BYTE_ZERO then {
        rlp_invalid_cursor(node)
    } else {
        let (is_list, content, content_len) = rlp_ref_hdr(node, BYTE_ZERO, n);
        let content_stop = rlp_checked_add(content, content_len);
        if is_list & (content_stop == n) then {
            struct { source = node, current = content, stop = content_stop, valid = true }
        } else {
            rlp_invalid_cursor(node)
        }
    }
}

/*! A cursor over a decoded field's children; throws if the field is not
    a list. */
function rlp_ref_cursor(f : RlpFieldRef) -> RlpCursor =
    if f.is_list then {
        let stop = rlp_checked_add(f.content_off, f.content_len);
        struct { source = f.source, current = f.content_off, stop = stop, valid = true }
    } else {
        throw InvalidBlock(RlpDecode)
    }

/*! Whether the cursor has consumed all children. */
function rlp_cursor_empty(cursor : RlpCursor) -> bool =
    cursor.valid & (cursor.current == cursor.stop)

/*! Decodes the next child and advances; throws on truncated input. */
function rlp_cursor_pop(cursor : RlpCursor) -> (RlpFieldRef, RlpCursor) = {
    let current : source_pointer = cursor.current;
    let stop : source_pointer = cursor.stop;
    if not_bool(cursor.valid) | not_bool(current < stop) then {
        throw InvalidBlock(RlpDecode)
    };
    let (is_list, content, content_len) = rlp_ref_hdr(cursor.source, current, stop);
    let next = rlp_checked_add(content, content_len);
    if stop < next then {
        throw InvalidBlock(RlpDecode)
    };
    if current <= next then {
        let full_len : byte_length = next - current;
        (
            rlp_field_ref(cursor.source, is_list, current, full_len, content, content_len),
            struct { source = cursor.source, current = next, stop = stop, valid = true },
        )
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
    let item_len = item.len;
    if item_len == BYTE_ZERO then {
        throw InvalidBlock(RlpDecode)
    } else {
        let (isl, cs, cl) = rlp_ref_hdr(item, BYTE_ZERO, item_len);
        let len = rlp_checked_add(cs, cl);
        if len != item_len then {
            throw InvalidBlock(RlpDecode)
        };
        rlp_field_ref(item, isl, BYTE_ZERO, len, cs, cl)
    }
}

/*! The content span of a field. */
function rlp_ref_content(f : RlpFieldRef) -> ByteSlice =
    sub_slice(f.source, f.content_off, f.content_len)

/*! The full encoded span of a field. */
function rlp_ref_full(f : RlpFieldRef) -> ByteSlice =
    sub_slice(f.source, f.full_off, f.full_len)

/*! Whether a field uses the unique canonical RLP framing for its payload. */
function rlp_ref_framing_canonical(f : RlpFieldRef) -> bool = {
    let n = f.content_len;
    let fl = f.full_len;
    let fo = f.full_off;
    let co = f.content_off;
    if fl == BYTE_ZERO then {
        false
    } else {
        if f.is_list then {
            if n <= RLP_SHORT_LENGTH_LIMIT then {
                let encoded_len = n + BYTE_ONE;
                (fl == encoded_len) & (slice_byte(f.source, fo) == add_bits(0xc0, rlp_length_byte(n)))
            } else {
                let (len_bytes, length_width) = minimal_word_bytes(rlp_length_word(n));
                let encoded_len = rlp_checked_add(ByteQuantity(1 + length_width), n);
                  (fl == encoded_len)
                & (slice_byte(f.source, fo) == add_bits(0xf7, rlp_length_byte(length_width)))
                & rlp_bytes_equal_at(len_bytes, f.source, rlp_checked_add(fo, BYTE_ONE))
            }
        } else {
            if n == BYTE_ZERO then {
                (fl == BYTE_ONE) & (slice_byte(f.source, fo) == 0x80)
            } else {
                let first = slice_byte(f.source, co);
                if (n == BYTE_ONE) & (first[7] == bitzero) then {
                    (fl == BYTE_ONE) & (slice_byte(f.source, fo) == first)
                } else {
                    if n <= RLP_SHORT_LENGTH_LIMIT then {
                        let encoded_len = n + BYTE_ONE;
                        (fl == encoded_len) & (slice_byte(f.source, fo) == add_bits(0x80, rlp_length_byte(n)))
                    } else {
                        let (len_bytes, length_width) = minimal_word_bytes(rlp_length_word(n));
                        let encoded_len = rlp_checked_add(ByteQuantity(1 + length_width), n);
                          (fl == encoded_len)
                        & (slice_byte(f.source, fo) == add_bits(0xb7, rlp_length_byte(length_width)))
                        & rlp_bytes_equal_at(len_bytes, f.source, rlp_checked_add(fo, BYTE_ONE))
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
        (f.content_len == BYTE_ZERO) | (slice_byte(f.source, f.content_off) != 0x00)
    }

/*! Returns a canonical byte-string field of exactly `len` bytes. */
function rlp_ref_fixed_bytes(f : RlpFieldRef, len : byte_length) -> ByteSlice =
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

/*! Decodes a canonical unsigned integer that fits the protocol-quantity
    domain. */
function rlp_ref_uint(f : RlpFieldRef) -> protocol_quantity = {
    match word_to_limb(rlp_ref_uint_word(f)) {
        Some(value) => unsigned(value),
        None() => throw InvalidBlock(RlpDecode),
    }
}
