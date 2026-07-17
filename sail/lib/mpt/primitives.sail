/* Paths and hex-prefix encoding. */

/* Trie paths are at most 64 nibbles in every trie built here: secure state and
 * storage keys are 32-byte hashes, while list tries use short RLP indices.
 * `data` is high-aligned and `len` preserves leading zeroes and prefixes. */
type nibble = bits(4)
type trie_path_len = range(0, 64)
type trie_depth = range(0, 63)
type trie_path_cursor = range(0, 65)
type trie_fuel = range(0, 256)
struct TriePath = { data : bits(256), len : trie_path_len }

let HEX_PREFIX_MAX_LENGTH : byte_length = ByteQuantity(33)

function path_len(path : TriePath) -> trie_path_len = path.len

function trie_path_len_increment(value : trie_path_len) -> trie_path_len =
    if value < 64 then {
        value + 1
    } else {
        throw InvalidBlock(WitnessDeficient)
    }

function trie_fuel_decrement(value : trie_fuel) -> trie_fuel =
    if 0 < value then {
        value - 1
    } else {
        throw InvalidBlock(WitnessDeficient)
    }

function path_new(data : bits(256), len : trie_path_len) -> TriePath =
    struct { data = data, len = len }

function to_trie_depth(value : trie_path_len) -> trie_depth =
    if 64 <= value then {
        throw InvalidBlock(WitnessDeficient)
    } else {
        value
    }

function path_empty() -> TriePath =
    path_new(ZERO_WORD, 0)

function path_from_hash(h : bits(256)) -> TriePath =
    path_new(h, 64)

function path_nibble(path : TriePath, i : trie_path_cursor) -> nibble =
    if path_len(path) <= i then {
        0x0
    } else {
        let distance = 63 - i;
        sail_shiftright(path.data, distance * 4)[3 .. 0]
    }

function path_single(n : nibble) -> TriePath =
    path_new(sail_shiftleft(sail_zero_extend(n, 256), 252), 1)

function path_concat(a : TriePath, b : TriePath) -> TriePath = {
    let alen = path_len(a);
    let blen = path_len(b);
    let combined = alen + blen;
    if combined <= 64 then {
        path_new(or_vec(a.data, sail_shiftright(b.data, alen * 4)), combined)
    } else {
        throw InvalidBlock(WitnessDeficient)
    }
}

function path_take(path : TriePath, n : trie_path_len) -> TriePath =
    if n == 0 then {
        path_empty()
    } else {
        if path_len(path) <= n then {
            path
        } else {
            let shift = 256 - n * 4;
            path_new(sail_shiftleft(sail_shiftright(path.data, shift), shift), n)
        }
    }

function path_drop(path : TriePath, n : trie_path_len) -> TriePath = {
    let length = path_len(path);
    if length <= n then {
        path_empty()
    } else {
        if n == 0 then {
            path
        } else {
            let remain : trie_path_len = length - n;
            path_new(sail_shiftleft(path.data, n * 4), remain)
        }
    }
}

function path_eq(a : TriePath, b : TriePath) -> bool =
    (a.len == b.len) & (a.data == b.data)

function path_lt(a : TriePath, b : TriePath) -> bool =
    if a.data == b.data then {
        path_len(a) < path_len(b)
    } else {
        word_ult(a.data, b.data)
    }

function path_prefix_of(prefix : TriePath, path : TriePath) -> bool = {
    if path_len(path) < path_len(prefix) then {
        false
    } else {
        path_eq(prefix, path_take(path, path_len(prefix)))
    }
}

function common_prefix_from(a : TriePath, b : TriePath, start : trie_path_len) -> trie_path_len = {
    let alen = path_len(a);
    let blen = path_len(b);
    let stop = if alen < blen then { alen } else { blen };
    var index : trie_path_len = start;
    var count : trie_path_len = 0;
    var matching : bool = true;
    foreach (_step from 0 to 63) {
        if matching & index < stop then {
            if path_nibble(a, index) == path_nibble(b, index) then {
                count = trie_path_len_increment(count);
                index = trie_path_len_increment(index)
            } else {
                matching = false
            }
        }
    };
    count
}

/* Encode the remaining nibble pairs directly in wire order. The nibble index
 * increases by two and the secure-key path is bounded by 64 nibbles. */
function hex_prefix_pairs(path : TriePath, index : trie_path_cursor) -> list(byte) =
    if path_len(path) <= index then {
        [||]
    } else {
        let next = index + 1;
        append(path_nibble(path, index), path_nibble(path, next)) :: hex_prefix_pairs(path, next + 1)
    }
termination_measure hex_prefix_pairs(path, index) = 65 - index

/* hex-prefix (compact) encoding of a nibble path + leaf/extension flag
 * (Yellow Paper Appendix C.1, the HP function). Its length is known from the
 * path length, so callers never walk the resulting byte list to measure it. */
function hex_prefix_compact(path : TriePath, is_leaf : bool) -> (list(byte), byte_length) = {
    let odd = tmod_int(path_len(path), 2) == 1;
    let flag : nibble = if is_leaf then { 0x2 } else { 0x0 };
    let first = if odd then {
            append(flag | 0x1, path_nibble(path, 0))
        } else {
            append(flag, 0x0)
        };
    let first_path_index : trie_path_cursor = if odd then { 1 } else { 0 };
    let encoded_len : byte_length = ByteQuantity(1 + exact_quotient(path_len(path), 2));
    (first :: hex_prefix_pairs(path, first_path_index), encoded_len)
}

/* Decode a compact path directly from its RLP source span. */
function hex_prefix_decode_ref(f : RlpFieldRef) -> (bool, TriePath) = {
    if f.is_list then {
        throw InvalidBlock(RlpDecode)
    };
    let n = f.content_len;
    let off = f.content_off;
    if n == BYTE_ZERO then {
        (false, path_empty())
    } else {
        if HEX_PREFIX_MAX_LENGTH < n then {
            throw InvalidBlock(RlpDecode)
        };
        let fb = slice_byte(f.source, off);
        let flag : nibble = fb[7 .. 4];
        let is_leaf : bool = flag[1] == bitone;
        let odd : bool = flag[0] == bitone;
        var path = path_empty();
        if odd then {
            path = path_concat(path, path_single(fb[3 .. 0]))
        };
        var cursor : byte_length = BYTE_ONE;
        foreach (_step from 0 to 31) {
            if cursor < n then {
                let b = slice_byte(f.source, off + cursor);
                path = path_concat(path, path_single(b[7 .. 4]));
                path = path_concat(path, path_single(b[3 .. 0]));
                cursor = cursor + BYTE_ONE
            }
        };
        (is_leaf, path)
    }
}

function path_matches(key : TriePath, pos : trie_path_len, seg : TriePath) -> bool = {
    let stop = pos + path_len(seg);
    if path_len(key) < stop then {
        false
    } else {
        var ok : bool = true;
        var offset : trie_path_len = 0;
        foreach (_step from 0 to 63) {
            if offset < path_len(seg) then {
                let key_index = pos + offset;
                if key_index <= 64 then {
                    if path_nibble(key, key_index) != path_nibble(seg, offset) then {
                        ok = false
                    }
                } else {
                    ok = false
                };
                offset = trie_path_len_increment(offset)
            }
        };
        ok
    }
}
