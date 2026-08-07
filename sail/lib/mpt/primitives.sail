
/*md
# Trie paths and hex-prefix encoding

Nibble paths through the Merkle-Patricia trie and the compact (hex-prefix)
encoding of YP Appendix C.
*/
/*! A four-bit path element (YP Appendix D). */
type nibble = bits(4)
/*! The number of nibbles in a trie path. */
type trie_path_len = range(0, 64)
/*! The depth of a branch node in a fixed 64-nibble secure key. */
type trie_depth = range(0, 63)
/*! A cursor at or immediately after a position in a trie path. */
type trie_path_cursor = range(0, 64)
/*! A cursor through the at-most-65 positions used by hex-prefix decoding. */
type hex_prefix_cursor = range(0, 65)
/*! A byte position in a 32-byte secure key. */
type b256_index = range(0, 31)
/*! A trie path of at most 64 nibbles — secure state and storage keys are
    32-byte hashes, and list tries use short RLP indices. `data` is
    high-aligned; `len` preserves leading zeroes and prefixes. */
struct TriePath = { data : b256, len : trie_path_len }

let HEX_PREFIX_MAX_LENGTH : int(33) = 33

/*! The path length in nibbles. */
function path_len(path : TriePath) -> trie_path_len = path.len

/*! Constructs a path from high-aligned data and a nibble length. */
function path_new(data : b256, len : trie_path_len) -> TriePath =
    struct { data = data, len = len }

/*! The empty path. */
function path_empty() -> TriePath =
    path_new(ZERO_HASH, 0)

/*! Maps a nibble cursor to the corresponding canonical-order path byte. */
function path_byte_index(i : trie_path_cursor) -> b256_index = {
    let quotient = tdiv_nat(i, 2);
    let natural_index : b256_index =
        if (0 <= quotient) & (quotient <= 31) then {
            quotient
        } else {
            assert(false);
            0
        };
    natural_index
}

/*! The `i`-th nibble, most significant first; out of range yields `0`. */
function path_nibble(path : TriePath, i : trie_path_cursor) -> nibble =
    let length = path_len(path) in
    if length <= i then {
        0x0
    } else {
        let bytes = path.data;
        let byte_index = path_byte_index(i);
        let parity = tmod_int(i, 2);
        if parity == 0 then {
            bytes[byte_index][7 .. 4]
        } else {
            bytes[byte_index][3 .. 0]
        }
    }

/*! Appends one nibble to a path, rejecting paths already at the key bound. */
function path_append_nibble(path : TriePath, value : nibble) -> TriePath = {
    let length = path_len(path);
    if length < 64 then {
        let original = path.data;
        var bytes = original;
        let byte_index = path_byte_index(length);
        let parity = tmod_int(length, 2);
        if parity == 0 then {
            bytes[byte_index] = append(value, 0x0)
        } else {
            bytes[byte_index] = append(bytes[byte_index][7 .. 4], value)
        };
        let path_data = B256(bytes);
        path_new(path_data, length + 1)
    } else {
        fatal_error(WitnessDeficient)
    }
}

/*! Appends both nibbles of a byte to a path, high nibble first. */
function path_append_byte(path : TriePath, value : byte) -> TriePath = {
    let high_nibble = path_append_nibble(path, value[7 .. 4]);
    path_append_nibble(high_nibble, value[3 .. 0])
}

/*! A one-nibble path. */
function path_single(n : nibble) -> TriePath = {
    let empty_path = path_empty();
    path_append_nibble(empty_path, n)
}

/*! Path concatenation; over 64 nibbles is a witness fault. */
function path_concat(a : TriePath, b : TriePath) -> TriePath = {
    let alen = path_len(a);
    let blen = path_len(b);
    let combined = alen + blen;
    if combined <= 64 then {
        var result = a;
        var index : trie_path_len = 0;
        while index < blen termination_measure(blen - index) do {
            let nibble = path_nibble(b, index);
            result = path_append_nibble(result, nibble);
            let current_index = index;
            index =
                if current_index < 64 then {
                    current_index + 1
                } else {
                    fatal_error(WitnessDeficient)
                }
        };
        result
    } else {
        fatal_error(WitnessDeficient)
    }
}

/*! The path with its first `n` nibbles removed. */
function path_drop(path : TriePath, n : trie_path_len) -> TriePath = {
    let length = path_len(path);
    if length <= n then {
        path_empty()
    } else if n == 0 then {
        path
    } else {
        let remain : trie_path_len = length - n;
        var result = path_empty();
        var offset : trie_path_len = 0;
        while offset < remain termination_measure(remain - offset) do {
            let candidate = n + offset;
            let source_index : trie_path_cursor =
                if (0 <= candidate) & (candidate <= 64) then {
                    candidate
                } else {
                    assert(false);
                    0
                };
            let nibble = path_nibble(path, source_index);
            result = path_append_nibble(result, nibble);
            let current_offset = offset;
            offset =
                if current_offset < 64 then {
                    current_offset + 1
                } else {
                    fatal_error(WitnessDeficient)
                }
        };
        result
    }
}

/*! Path equality. */
function path_eq(a : TriePath, b : TriePath) -> bool =
    (a.len == b.len) & (a.data == b.data)

/*! Whether `seg` occurs in `key` at nibble position `pos`. */
function path_matches(key : TriePath, pos : trie_path_cursor, seg : TriePath) -> bool = {
    let segment_len = path_len(seg);
    let key_len = path_len(key);
    let stop = pos + segment_len;
    if key_len < stop then {
        false
    } else {
        var ok : bool = true;
        var offset : trie_path_len = 0;
        while ok & offset < segment_len termination_measure(segment_len - offset) do {
            let key_index = pos + offset;
            if key_index <= 64 then {
                let key_nibble = path_nibble(key, key_index);
                let segment_nibble = path_nibble(seg, offset);
                if key_nibble != segment_nibble then {
                    ok = false
                }
            } else {
                ok = false
            };
            let current_offset = offset;
            offset =
                if current_offset < 64 then {
                    current_offset + 1
                } else {
                    fatal_error(WitnessDeficient)
                }
        };
        ok
    }
}

/*! Whether `prefix` is a prefix of `path`. */
function path_prefix_of(prefix : TriePath, path : TriePath) -> bool =
    path_matches(path, 0, prefix)

/*! The common-prefix length of two canonical nibble paths. */
function common_prefix_length(a : TriePath, b : TriePath) -> trie_path_len = {
    let alen = path_len(a);
    let blen = path_len(b);
    let stop =
        if alen < blen then alen else blen;
    var length : trie_path_len = 0;
    var matching : bool = true;
    while matching & length < stop termination_measure(stop - length) do {
        let a_nibble = path_nibble(a, length);
        let b_nibble = path_nibble(b, length);
        if a_nibble == b_nibble then {
            let current_length = length;
            length =
                if current_length < 64 then {
                    current_length + 1
                } else {
                    fatal_error(WitnessDeficient)
                }
        } else {
            matching = false
        }
    };
    length
}

/*! The encoded byte length of the hex-prefix form of a trie path. */
function hex_prefix_encoded_length(path : TriePath) -> range(1, 33) = {
    let length : trie_path_len = path_len(path);
    let packed_pair_count : range(0, 32) = tdiv_nat(length, 2);
    1 + packed_pair_count
}

/*! The flag byte beginning the hex-prefix form of a trie path. */
function hex_prefix_first_byte(path : TriePath, is_leaf : bool) -> byte = {
    let length : trie_path_len = path_len(path);
    let odd = tmod_nat(length, 2) != 0;
    let flag : nibble =
        if is_leaf then 0x2 else 0x0;
    if odd then {
        let first_nibble = path_nibble(path, 0);
        append(flag | 0x1, first_nibble)
    } else {
        append(flag, 0x0)
    }
}

/*! Decodes a compact path directly from its RLP source span, returning
    the leaf flag and the path. */
function hex_prefix_decode_ref forall 'source_off 'source_len 'content_len,
                                 rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    (bool, TriePath)
) = {
    if f.is_list then {
        fatal_error(RlpDecode)
    };
    let n = f.content_len;
    if n == 0 then {
        (false, path_empty())
    } else {
        let maximum_length = HEX_PREFIX_MAX_LENGTH;
        if maximum_length < n then {
            fatal_error(RlpDecode)
        } else {
            let content = sub_slice(f.source, f.source.len - n, n);
            let fb = slice_byte(content, 0);
            let flag : nibble = fb[7 .. 4];
            let is_leaf : bool = flag[1] == bitone;
            let odd : bool = flag[0] == bitone;
            let tail_length : range(0, 32) = n - 1;
            let tail = slice_suffix(content, 1);
            let packed = slice_load(tail, 0);
            let paired_nibbles : range(0, 64) = tail_length * 2;
            if odd then {
                if paired_nibbles < 64 then {
                    let shifted = word_shift_right(packed, 4);
                    var bytes = word_to_hash(shifted);
                    bytes[0] = append(fb[3 .. 0], bytes[0][3 .. 0]);
                    let path_data = B256(bytes);
                    let path = path_new(path_data, paired_nibbles + 1);
                    (is_leaf, path)
                } else {
                    fatal_error(WitnessDeficient)
                }
            } else {
                let path_data = word_to_hash(packed);
                let path = path_new(path_data, paired_nibbles);
                (is_leaf, path)
            }
        }
    }
}

/*! Scratch-backed counterpart used only when canonicalization reopens an
    embedded node that it just encoded. */
function scratch_hex_prefix_decode_ref forall 'source_off 'source_len 'content_len,
                                         rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    ScratchRlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    (bool, TriePath)
) = {
    if f.is_list then {
        fatal_error(RlpDecode)
    };
    let n = f.content_len;
    if n == 0 then {
        (false, path_empty())
    } else {
        let maximum_length = HEX_PREFIX_MAX_LENGTH;
        if maximum_length < n then {
            fatal_error(RlpDecode)
        } else {
            let content = sub_slice(f.source, f.source.len - n, n);
            let fb = slice_byte(content, 0);
            let flag : nibble = fb[7 .. 4];
            let is_leaf : bool = flag[1] == bitone;
            let odd : bool = flag[0] == bitone;
            let tail_length : range(0, 32) = n - 1;
            let tail = slice_suffix(content, 1);
            let packed = slice_load(tail, 0);
            let paired_nibbles : range(0, 64) = tail_length * 2;
            if odd then {
                if paired_nibbles < 64 then {
                    let shifted = word_shift_right(packed, 4);
                    var bytes = word_to_hash(shifted);
                    bytes[0] = append(fb[3 .. 0], bytes[0][3 .. 0]);
                    let path_data = B256(bytes);
                    let path = path_new(path_data, paired_nibbles + 1);
                    (is_leaf, path)
                } else {
                    fatal_error(WitnessDeficient)
                }
            } else {
                let path_data = word_to_hash(packed);
                let path = path_new(path_data, paired_nibbles);
                (is_leaf, path)
            }
        }
    }
}
