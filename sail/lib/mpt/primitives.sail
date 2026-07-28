
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

/*! Increments a path length, rejecting a value already at the key bound. */
function trie_path_len_increment(value : trie_path_len) -> trie_path_len =
    if value < 64 then {
        value + 1
    } else {
        throw InvalidBlock(WitnessDeficient)
    }

/*! Constructs a path from high-aligned data and a nibble length. */
function path_new(data : b256, len : trie_path_len) -> TriePath =
    struct { data = data, len = len }

/*! Narrows a nonterminal path position to a branch depth. */
function to_trie_depth(value : trie_path_len) -> trie_depth =
    if 64 <= value then {
        throw InvalidBlock(WitnessDeficient)
    } else {
        value
    }

/*! The empty path. */
function path_empty() -> TriePath =
    path_new(ZERO_HASH, 0)

/*! The 64-nibble path of a 32-byte hash — the secure-trie key form. */
function path_from_hash(h : hash) -> TriePath =
    path_new(h, 64)

/*! Maps a nibble cursor to the corresponding canonical-order path byte. */
function path_byte_index(i : trie_path_cursor) -> b256_index = {
    let quotient = tdiv_nat(i, 2);
    let natural_index : b256_index = if (0 <= quotient) & (quotient <= 31) then {
            quotient
        } else {
            assert(false);
            0
        };
    natural_index
}

/*! The `i`-th nibble, most significant first; out of range yields `0`. */
function path_nibble(path : TriePath, i : trie_path_cursor) -> nibble =
    if path_len(path) <= i then {
        0x0
    } else {
        let bytes = path.data;
        let byte_index = path_byte_index(i);
        if tmod_int(i, 2) == 0 then {
            bytes[byte_index][7 .. 4]
        } else {
            bytes[byte_index][3 .. 0]
        }
    }

/*! Appends one nibble to a path, rejecting paths already at the key bound. */
function path_append_nibble(path : TriePath, value : nibble) -> TriePath = {
    let length = path_len(path);
    if 64 <= length then {
        throw InvalidBlock(WitnessDeficient)
    };
    let original = path.data;
    var bytes = original;
    let byte_index = path_byte_index(length);
    if tmod_int(length, 2) == 0 then {
        bytes[byte_index] = append(value, 0x0)
    } else {
        bytes[byte_index] = append(bytes[byte_index][7 .. 4], value)
    };
    path_new(B256(bytes), trie_path_len_increment(length))
}

function path_append_byte(path : TriePath, value : byte) -> TriePath =
    path_append_nibble(path_append_nibble(path, value[7 .. 4]), value[3 .. 0])

/*! A one-nibble path. */
function path_single(n : nibble) -> TriePath =
    path_append_nibble(path_empty(), n)

/*! Path concatenation; over 64 nibbles is a witness fault. */
function path_concat(a : TriePath, b : TriePath) -> TriePath = {
    let alen = path_len(a);
    let blen = path_len(b);
    let combined = alen + blen;
    if combined <= 64 then {
        var result = a;
        var index : trie_path_len = 0;
        while index < blen termination_measure(blen - index) do {
            result = path_append_nibble(result, path_nibble(b, index));
            index = trie_path_len_increment(index)
        };
        result
    } else {
        throw InvalidBlock(WitnessDeficient)
    }
}

/*! The first `n` nibbles. */
function path_take(path : TriePath, n : trie_path_len) -> TriePath =
    if n == 0 then {
        path_empty()
    } else {
        if path_len(path) <= n then {
            path
        } else {
            var result = path_empty();
            var index : trie_path_len = 0;
            while index < n termination_measure(n - index) do {
                result = path_append_nibble(result, path_nibble(path, index));
                index = trie_path_len_increment(index)
            };
            result
        }
    }

/*! The path with its first `n` nibbles removed. */
function path_drop(path : TriePath, n : trie_path_len) -> TriePath = {
    let length = path_len(path);
    if length <= n then {
        path_empty()
    } else {
        if n == 0 then {
            path
        } else {
            let remain : trie_path_len = length - n;
            var result = path_empty();
            var offset : trie_path_len = 0;
            while offset < remain termination_measure(remain - offset) do {
                let candidate = n + offset;
                let source_index : trie_path_cursor = if (0 <= candidate) & (candidate <= 64) then {
                        candidate
                    } else {
                        assert(false);
                        0
                    };
                result = path_append_nibble(result, path_nibble(path, source_index));
                offset = trie_path_len_increment(offset)
            };
            result
        }
    }
}

/*! Path equality. */
function path_eq(a : TriePath, b : TriePath) -> bool =
    (a.len == b.len) & (a.data == b.data)

/*! Lexicographic path order (data, then length). */
function path_lt(a : TriePath, b : TriePath) -> bool =
    if a.data == b.data then {
        path_len(a) < path_len(b)
    } else {
        hash_lt(a.data, b.data)
    }

/*! Whether `seg` occurs in `key` at nibble position `pos`. */
function path_matches(key : TriePath, pos : trie_path_cursor, seg : TriePath) -> bool = {
    let stop = pos + path_len(seg);
    if path_len(key) < stop then {
        false
    } else {
        var ok : bool = true;
        var offset : trie_path_len = 0;
        while ok & offset < path_len(seg) termination_measure(path_len(seg) - offset) do {
            let key_index = pos + offset;
            if key_index <= 64 then {
                if path_nibble(key, key_index) != path_nibble(seg, offset) then {
                    ok = false
                }
            } else {
                ok = false
            };
            offset = trie_path_len_increment(offset)
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
    let stop = if alen < blen then { alen } else { blen };
    var length : trie_path_len = 0;
    var matching : bool = true;
    while matching & length < stop termination_measure(stop - length) do {
        if path_nibble(a, length) == path_nibble(b, length) then {
            length = trie_path_len_increment(length)
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
    let flag : nibble = if is_leaf then { 0x2 } else { 0x0 };
    if odd then {
        append(flag | 0x1, path_nibble(path, 0))
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
        throw InvalidBlock(RlpDecode)
    };
    let n = f.content_len;
    if n == 0 then {
        (false, path_empty())
    } else {
        let maximum_length = HEX_PREFIX_MAX_LENGTH;
        if maximum_length < n then {
            throw InvalidBlock(RlpDecode)
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
                    var bytes = word_to_hash(word_shift_right(packed, 4));
                    bytes[0] = append(fb[3 .. 0], bytes[0][3 .. 0]);
                    (is_leaf, path_new(B256(bytes), paired_nibbles + 1))
                } else {
                    throw InvalidBlock(WitnessDeficient)
                }
            } else {
                (is_leaf, path_new(word_to_hash(packed), paired_nibbles))
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
        throw InvalidBlock(RlpDecode)
    };
    let n = f.content_len;
    if n == 0 then {
        (false, path_empty())
    } else {
        let maximum_length = HEX_PREFIX_MAX_LENGTH;
        if maximum_length < n then {
            throw InvalidBlock(RlpDecode)
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
                    var bytes = word_to_hash(word_shift_right(packed, 4));
                    bytes[0] = append(fb[3 .. 0], bytes[0][3 .. 0]);
                    (is_leaf, path_new(B256(bytes), paired_nibbles + 1))
                } else {
                    throw InvalidBlock(WitnessDeficient)
                }
            } else {
                (is_leaf, path_new(word_to_hash(packed), paired_nibbles))
            }
        }
    }
}
