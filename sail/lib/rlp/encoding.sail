
/*md
# RLP encoding

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
    host scratch-region limit is enforced while recursive collection totals
    are accumulated. */
type rlp_scratch_length = range(0, scratch_region_bound)

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
    if right <= sizeof(scratch_region_bound) - left then {
        left + right
    } else {
        throw InvalidBlock(RlpDecode)
    }

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

/*! Returns the materializable RLP width of a byte slice after its caller has
    selected the backing region and loaded the first byte. */
function rlp_materialized_slice_size(
    length : rlp_scratch_length,
    first : byte,
) -> rlp_scratch_length =
    if (length == 1) & (first[7] == bitzero) then {
        1
    } else {
        rlp_scratch_length_add(
            length,
            rlp_scratch_small_length(rlp_length_prefix_len(length)),
        )
    }

function rlp_scratch_region_size(data : ScratchSlice) -> rlp_scratch_length =
    rlp_materialized_slice_size(
        data.len,
        if data.len == 0 then {
            0x00
        } else {
            slice_byte(data, 0)
        },
    )

function rlp_log_data_size(data : LogDataSlice) -> rlp_scratch_length =
    rlp_materialized_slice_size(
        data.len,
        if data.len == 0 then {
            0x00
        } else {
            slice_byte(data, 0)
        },
    )

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
    rlp_materialized_slice_size(
        data.len,
        if data.len == 0 then {
            0x00
        } else {
            slice_byte(data, 0)
        },
    )

function rlp_scratch_scratch_slice_size(data : ScratchSlice) -> rlp_scratch_length =
    rlp_scratch_region_size(data)

function rlp_log_scratch_slice_size(data : LogDataSlice) -> rlp_scratch_length =
    rlp_log_data_size(data)

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
## Exact scratch encoding

An encoder reserves its complete output before the first write. The expected
length is checked once at the boundary, keeping allocation and scratch lifetime
out of domain callers while leaving primitive writes direct and allocation-free.
*/
/*! One exact-size RLP construction in the shared scratch arena. */
struct RlpEncoder = {
    start : source_pointer,
    expected_len : source_length,
}

/*! Begins an exact-size RLP construction and reserves its complete capacity. */
val rlp_encoder_begin : forall 'len, source_valid_length('len). int('len) -> RlpEncoder
function rlp_encoder_begin(expected_len) =
    struct { start = scratch_reserve(expected_len), expected_len = expected_len }

/*! Finishes an exact-size construction and retains its encoded bytes. */
function rlp_encoder_finish(encoder : RlpEncoder) -> ScratchSlice = {
    let encoded = scratch_finish(encoder.start);
    assert(encoded.len == encoder.expected_len, "RLP encoder length");
    encoded
}

/*! Releases every byte appended by an encoder after its result is consumed. */
function rlp_encoder_rewind(encoder : RlpEncoder) -> unit =
    scratch_rewind(encoder.start)
