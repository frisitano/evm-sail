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
guest top level (mirroring the EELS `raise` on a decoding error).
*/

function rlp_byte(n : int) -> byte = get_slice_int(8, n, 0)

/*! The minimal big-endian encoding of a non-negative integer and its
    length, built in one pass: consing successive low bytes yields
    big-endian order and the length without a second traversal. */
function minimal_int_bytes(n : int) -> (list(byte), int) = {
    var remaining : int = n;
    var out : list(byte) = [||];
    var len : int = 0;
    while remaining > 0 do {
        out = rlp_byte(emod_int(remaining, 256)) :: out;
        remaining = ediv_int(remaining, 256);
        len = len + 1
    };
    (out, len)
}

/*! The minimal big-endian encoding of a word and its length. */
function minimal_word_bytes(w : word) -> (list(byte), int) = {
    var remaining : word = w;
    var out : list(byte) = [||];
    var len : int = 0;
    while word_nonzero(remaining) do {
        out = remaining[7 .. 0] :: out;
        remaining = sail_shiftright(remaining, 8);
        len = len + 1
    };
    (out, len)
}

/*! The RLP string prefix for a payload of `len` bytes whose first byte
    is `first`; single bytes below `0x80` need no prefix. */
function rlp_string_prefix(len : int, first : byte) -> list(byte) =
    if (len == 1) & (unsigned(first) < 128) then {
        [||]
    } else {
        if len <= 55 then {
            [|rlp_byte(128 + len)|]
        } else {
            let (length_bytes, length_len) = minimal_int_bytes(len);
            rlp_byte(183 + length_len) :: length_bytes
        }
    }

function rlp_bytes_known_len(data : list(byte), len : int) -> list(byte) = {
    let first : byte = match data {
        b :: _ => b,
        [||] => 0x00,
    };
    list_append(rlp_string_prefix(len, first), data)
}

/*! RLP-encodes a byte string. */
function rlp_bytes(data : list(byte)) -> list(byte) = {
    rlp_bytes_known_len(data, list_len(data))
}

/*! The RLP list header for a payload of `len` content bytes. */
function rlp_list_prefix(len : int) -> list(byte) =
    if len <= 55 then {
        [|rlp_byte(192 + len)|]
    } else {
        let (length_bytes, length_len) = minimal_int_bytes(len);
        rlp_byte(247 + length_len) :: length_bytes
    }

/*! Wraps an already-encoded item payload as an RLP list. */
function rlp_list(payload : list(byte)) -> list(byte) = {
    list_append(rlp_list_prefix(list_len(payload)), payload)
}

/*! A builder holding one already-encoded segment. */
function rlp_builder_encoded(encoded : list(byte)) -> RlpBuilder = {
    let n = list_len(encoded);
    if n == 0 then {
        EMPTY_RLP_BUILDER
    } else {
        struct { segments_rev = [|BytesList(encoded)|], len = n }
    }
}

/*! Logical concatenation of builders. Only the newly appended value's
    segment spine is traversed; the already-built prefix remains shared. */
function rlp_builder_append(left : RlpBuilder, right : RlpBuilder) -> RlpBuilder =
    struct { segments_rev = list_append(right.segments_rev, left.segments_rev), len = left.len + right.len }

function rlp_builder_append_encoded(builder : RlpBuilder, encoded : list(byte)) -> RlpBuilder =
    rlp_builder_append(builder, rlp_builder_encoded(encoded))

/*! Encodes a source-backed byte string as an RLP prefix segment followed
    by the original slice; the payload bytes are never copied into a Sail
    list. */
function rlp_builder_slice(s : ByteSlice) -> RlpBuilder = {
    let n = unsigned(s.len);
    let prefix = rlp_string_prefix(
        n,
        if n == 0 then {
            0x00
        } else {
            slice_byte(s, 0)
        },
    );
    let prefixed = rlp_builder_encoded(prefix);
    if n == 0 then {
        prefixed
    } else {
        rlp_builder_append(prefixed, struct { segments_rev = [|BytesSlice(s)|], len = n })
    }
}

/*! Wraps a builder's content as an RLP list. */
function rlp_builder_list(content : RlpBuilder) -> RlpBuilder =
    rlp_builder_append(rlp_builder_encoded(rlp_list_prefix(content.len)), content)

/*! The builder's segments in logical order. */
function rlp_builder_segments(builder : RlpBuilder) -> list(Bytes) =
    list_reverse(builder.segments_rev)

/*! Compares the logical builder bytes directly with a source-backed
    slice. */
function rlp_builder_equals_slice(builder : RlpBuilder, expected : ByteSlice) -> bool =
    if builder.len != unsigned(expected.len) then {
        false
    } else {
        bytes_segments_equal_slice(rlp_builder_segments(builder), expected)
    }

/*! RLP-encodes a non-negative integer as a minimal big-endian string. */
function rlp_int(n : int) -> list(byte) = {
    let (bytes, len) = minimal_int_bytes(n);
    rlp_bytes_known_len(bytes, len)
}

/*! RLP-encodes a word as a fixed 32-byte string. */
function rlp_word(w : word) -> list(byte) = rlp_bytes(word_to_bytes32(w))
/*! RLP-encodes an address as a fixed 20-byte string. */
function rlp_addr(a : address) -> list(byte) = rlp_bytes(address_to_bytes(a))

/*! RLP-encodes a word as a minimal (integer-canonical) string. */
function rlp_int_word(w : word) -> list(byte) = {
    let (bytes, len) = minimal_word_bytes(w);
    rlp_bytes_known_len(bytes, len)
}

function zero_bytes(n : int) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (i from 0 to (n - 1)) {
        out = 0x00 :: out
    };
    out
}

/*! `keccak256(domain ++ rlp_list_prefix(len(payload)) ++ payload)`, as
    segments (the `val` is declared with the crypto primitives). */
function keccak_rlp_payload(domain : list(byte), payload : list(byte)) -> bits(256) =
    keccak256([|BytesList(domain), BytesList(rlp_list_prefix(list_len(payload))), BytesList(payload)|])

/*md
## Decoding
*/

/*! A big-endian integer read of `n` bytes at `pos` within an RLP source. */
function rlp_ref_be_int(b : Bytes, pos : int, n : int) -> int = {
    match b {
        BytesSlice(s) => {
            var acc : int = 0;
            var current : int = pos;
            var remaining : int = n;
            while remaining > 0 do {
                acc = acc * 256 + unsigned(slice_byte(s, current));
                current = current + 1;
                remaining = remaining - 1
            };
            acc
        },
        BytesList(bs) => {
            var rest : list(byte) = bs;
            var skip : int = pos;
            while skip > 0 do {
                match rest {
                    _ :: tail => rest = tail,
                    [||] => throw InvalidBlock(RlpDecode),
                };
                skip = skip - 1
            };
            var acc : int = 0;
            var remaining : int = n;
            while remaining > 0 do {
                match rest {
                    byte :: tail => {
                        acc = acc * 256 + unsigned(byte);
                        rest = tail
                    },
                    [||] => throw InvalidBlock(RlpDecode),
                };
                remaining = remaining - 1
            };
            acc
        },
    }
}

/*! Decodes the RLP header at `pos`: `(is_list, content_offset,
    content_length)`. */
function rlp_ref_hdr(b : Bytes, pos : int) -> (bool, int, int) = {
    let h = unsigned(bytes_at(b, pos));
    if h < 128 then {
        (false, pos, 1)
    } else {
        if h < 184 then {
            (false, pos + 1, h - 128)
        } else {
            if h < 192 then {
                let lol = h - 183;
                (false, pos + 1 + lol, rlp_ref_be_int(b, pos + 1, lol))
            } else {
                if h < 248 then {
                    (true, pos + 1, h - 192)
                } else {
                    let lol = h - 247;
                    (true, pos + 1 + lol, rlp_ref_be_int(b, pos + 1, lol))
                }
            }
        }
    }
}

/*! Constructs an [RlpFieldRef][type-RlpFieldRef] from integer offsets. */
function rlp_field_ref(
    source : Bytes,
    is_list : bool,
    full_off : int,
    full_len : int,
    content_off : int,
    content_len : int,
) -> (
    RlpFieldRef
) =
    struct {
        source = source,
        is_list = is_list,
        full_off = get_slice_int(64, full_off, 0),
        full_len = get_slice_int(64, full_len, 0),
        content_off = get_slice_int(64, content_off, 0),
        content_len = get_slice_int(64, content_len, 0),
    }

/*! The invalid cursor; popping it throws `InvalidBlock(RlpDecode)`. */
function rlp_invalid_cursor(source : Bytes) -> RlpCursor =
    struct { source = source, current = 0, stop = 0, valid = false }

/*! A cursor over the children of a byte sequence that must be exactly
    one RLP list (e.g. a trie node). */
function rlp_node_cursor(node : Bytes) -> RlpCursor = {
    let n = bytes_len(node);
    if n == 0 then {
        rlp_invalid_cursor(node)
    } else {
        let (is_list, content, content_len) = rlp_ref_hdr(node, 0);
        if is_list & (content + content_len == n) then {
            struct { source = node, current = content, stop = content + content_len, valid = true }
        } else {
            rlp_invalid_cursor(node)
        }
    }
}

/*! A cursor over a decoded field's children; throws if the field is not
    a list. */
function rlp_ref_cursor(f : RlpFieldRef) -> RlpCursor =
    if f.is_list then {
        struct {
            source = f.source,
            current = unsigned(f.content_off),
            stop = unsigned(f.content_off) + unsigned(f.content_len),
            valid = true,
        }
    } else {
        throw InvalidBlock(RlpDecode)
    }

/*! Whether the cursor has consumed all children. */
function rlp_cursor_empty(cursor : RlpCursor) -> bool =
    cursor.valid & (cursor.current == cursor.stop)

/*! Decodes the next child and advances; throws on truncated input. */
function rlp_cursor_pop(cursor : RlpCursor) -> (RlpFieldRef, RlpCursor) = {
    if not_bool(cursor.valid) | (cursor.current >= cursor.stop) then {
        throw InvalidBlock(RlpDecode)
    };
    let (is_list, content, content_len) = rlp_ref_hdr(cursor.source, cursor.current);
    let next = content + content_len;
    if next > cursor.stop then {
        throw InvalidBlock(RlpDecode)
    };
    (
        rlp_field_ref(cursor.source, is_list, cursor.current, next - cursor.current, content, content_len),
        struct { source = cursor.source, current = next, stop = cursor.stop, valid = true },
    )
}

/*! Asserts the cursor is exhausted — trailing bytes are a decode
    error. */
function rlp_cursor_expect_end(cursor : RlpCursor) -> unit =
    if rlp_cursor_empty(cursor) then {
        ()
    } else {
        throw InvalidBlock(RlpDecode)
    }

/*! Decodes all children in `[start, stop)` into field references. */
function rlp_split_refs(b : Bytes, start : int, stop : int) -> list(RlpFieldRef) = {
    var out : list(RlpFieldRef) = [||];
    var pos : int = start;
    while pos < stop do {
        let (isl, cs, cl) = rlp_ref_hdr(b, pos);
        let nxt = cs + cl;
        if nxt > stop then {
            throw InvalidBlock(RlpDecode)
        };
        out = rlp_field_ref(b, isl, pos, nxt - pos, cs, cl) :: out;
        pos = nxt
    };
    list_reverse(out)
}

/*! The children of a byte sequence that is exactly one RLP list, or the
    empty list if it is not. */
function rlp_node_refs(node : Bytes) -> list(RlpFieldRef) = {
    if bytes_len(node) == 0 then {
        [||]
    } else {
        let (isl, cs, cl) = rlp_ref_hdr(node, 0);
        if isl & (cs + cl == bytes_len(node)) then {
            rlp_split_refs(node, cs, cs + cl)
        } else {
            [||]
        }
    }
}

/*! Decodes a byte sequence that must be exactly one RLP item. */
function rlp_single_ref(item : Bytes) -> RlpFieldRef = {
    if bytes_len(item) == 0 then {
        throw InvalidBlock(RlpDecode)
    } else {
        let (isl, cs, cl) = rlp_ref_hdr(item, 0);
        let len = cs + cl;
        if len != bytes_len(item) then {
            throw InvalidBlock(RlpDecode)
        };
        rlp_field_ref(item, isl, 0, len, cs, cl)
    }
}

/*! The content span of a field. */
function rlp_ref_content(f : RlpFieldRef) -> Bytes =
    bytes_range(f.source, unsigned(f.content_off), unsigned(f.content_len))

/*! The full encoded span of a field. */
function rlp_ref_full(f : RlpFieldRef) -> Bytes =
    bytes_range(f.source, unsigned(f.full_off), unsigned(f.full_len))

/*! The children of a list field; throws if the field is a string. */
function rlp_ref_list_items(f : RlpFieldRef) -> list(RlpFieldRef) =
    if f.is_list then {
        rlp_split_refs(f.source, unsigned(f.content_off), unsigned(f.content_off) + unsigned(f.content_len))
    } else {
        throw InvalidBlock(RlpDecode)
    }

/*! Whether a field is the canonical RLP encoding of a non-negative
    integer: minimal big-endian content with no leading zeros and the
    exact matching prefix. */
function rlp_ref_uint_canonical(f : RlpFieldRef) -> bool = {
    let n = unsigned(f.content_len);
    let fl = unsigned(f.full_len);
    let fo = unsigned(f.full_off);
    let co = unsigned(f.content_off);
    if f.is_list | (fl == 0) then {
        false
    } else {
        if n == 0 then {
            (fl == 1) & (unsigned(bytes_at(f.source, fo)) == 128)
        } else {
            let first = unsigned(bytes_at(f.source, co));
            if first == 0 then {
                false
            } else {
                if (n == 1) & (first < 128) then {
                    (fl == 1) & (unsigned(bytes_at(f.source, fo)) == first)
                } else {
                    if n <= 55 then {
                        (fl == n + 1) & (unsigned(bytes_at(f.source, fo)) == 128 + n)
                    } else {
                        let (len_bytes, lol) = minimal_int_bytes(n);
                        var ok : bool = (fl == 1 + lol + n) & (unsigned(bytes_at(f.source, fo)) == 183 + lol);
                        var expected : list(byte) = len_bytes;
                        var current : int = fo + 1;
                        var more : bool = true;
                        while more do {
                            match expected {
                                byte :: rest => {
                                    ok = ok & (bytes_at(f.source, current) == byte);
                                    expected = rest;
                                    current = current + 1
                                },
                                [||] => more = false,
                            }
                        };
                        ok
                    }
                }
            }
        }
    }
}

/*! Decodes a canonical unsigned integer field; throws otherwise. */
function rlp_ref_int(f : RlpFieldRef) -> int =
    if rlp_ref_uint_canonical(f) then {
        rlp_ref_be_int(f.source, unsigned(f.content_off), unsigned(f.content_len))
    } else {
        throw InvalidBlock(RlpDecode)
    }

/*! Decodes a string field of at most 32 bytes into a word. */
function rlp_ref_word(f : RlpFieldRef) -> word = {
    let n = unsigned(f.content_len);
    if f.is_list | (n > 32) then {
        throw InvalidBlock(RlpDecode)
    } else {
        match f.source {
            BytesSlice(s) => slice_load_n(s, unsigned(f.content_off), n),
            BytesList(_) => word_of_int(rlp_ref_be_int(f.source, unsigned(f.content_off), n)),
        }
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
