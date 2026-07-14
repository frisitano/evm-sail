/* Paths and hex-prefix encoding. */

/* Trie paths are at most 64 nibbles in every trie built here: secure state and
 * storage keys are 32-byte hashes, while list tries use short RLP indices.
 * `data` is high-aligned and `len` preserves leading zeroes and prefixes. */
type nibble = bits(4)
struct TriePath = { data : bits(256), len : bits(7) }

function path_len(path : TriePath) -> int = unsigned(path.len)

function path_new(data : bits(256), len : int) -> TriePath =
    struct { data = data, len = get_slice_int(7, len, 0) }

function path_empty() -> TriePath =
    path_new(ZERO_WORD, 0)

function path_from_bytes(bs : list(byte)) -> TriePath = {
    let n = list_len(bs);
    if n > 32 then {
        throw InvalidBlock(WitnessDeficient)
    } else {
        path_new(
            if n == 0 then {
                ZERO_WORD
            } else {
                sail_shiftleft(bytes_be256(bs), 256 - 8 * n)
            },
            2 * n,
        )
    }
}

function path_from_hash(h : bits(256)) -> TriePath =
    path_new(h, 64)

function path_nibble(path : TriePath, i : int) -> nibble =
    if (i < 0) | (i >= path_len(path)) then {
        0x0
    } else {
        sail_shiftright(path.data, 4 * (63 - i))[3 .. 0]
    }

function path_single(n : nibble) -> TriePath =
    path_new(sail_shiftleft(sail_zero_extend(n, 256), 252), 1)

function path_concat(a : TriePath, b : TriePath) -> TriePath = {
    let alen = path_len(a);
    let blen = path_len(b);
    if alen + blen > 64 then {
        throw InvalidBlock(WitnessDeficient)
    } else {
        path_new(or_vec(a.data, sail_shiftright(b.data, 4 * alen)), alen + blen)
    }
}

function path_take(path : TriePath, n : int) -> TriePath =
    if n <= 0 then {
        path_empty()
    } else {
        if n >= path_len(path) then {
            path
        } else {
            let shift = 256 - 4 * n;
            path_new(sail_shiftleft(sail_shiftright(path.data, shift), shift), n)
        }
    }

function path_drop(path : TriePath, n : int) -> TriePath = {
    let remain = path_len(path) - n;
    if remain <= 0 then {
        path_empty()
    } else {
        if n <= 0 then {
            path
        } else {
            path_new(sail_shiftleft(path.data, 4 * n), remain)
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
    if path_len(prefix) > path_len(path) then {
        false
    } else {
        path_eq(prefix, path_take(path, path_len(prefix)))
    }
}

function common_prefix_from(a : TriePath, b : TriePath, start : int) -> int = {
    let alen = path_len(a);
    let blen = path_len(b);
    let stop = if alen < blen then { alen } else { blen };
    var i : int = start;
    while (i < stop) & (path_nibble(a, i) == path_nibble(b, i)) do {
        i = i + 1
    };
    i - start
}

/* hex-prefix (compact) encoding of a nibble path + leaf/extension flag
 * (Yellow Paper Appendix C.1, the HP function). */
function hex_prefix_compact(path : TriePath, is_leaf : bool) -> list(byte) = {
    let odd = emod_int(path_len(path), 2) == 1;
    let flag : nibble = if is_leaf then { 0x2 } else { 0x0 };
    var out : list(byte) = [||];
    var i : int = 0;
    if odd then {
        out = append(flag | 0x1, path_nibble(path, 0)) :: out;
        i = 1
    } else {
        out = append(flag, 0x0) :: out
    };
    while i < path_len(path) do {
        out = append(path_nibble(path, i), path_nibble(path, i + 1)) :: out;
        i = i + 2
    };
    list_reverse(out)
}

/* Decode a compact path directly from its RLP source span. */
function hex_prefix_decode_ref(f : RlpFieldRef) -> (bool, TriePath) = {
    if f.is_list then {
        throw InvalidBlock(RlpDecode)
    };
    let n = unsigned(f.content_len);
    let off = unsigned(f.content_off);
    if n == 0 then {
        (false, path_empty())
    } else {
        let fb = bytes_at(f.source, off);
        let flag : nibble = fb[7 .. 4];
        let is_leaf : bool = flag[1] == bitone;
        let odd : bool = flag[0] == bitone;
        var path = path_empty();
        if odd then {
            path = path_concat(path, path_single(fb[3 .. 0]))
        };
        foreach (i from 1 to (n - 1)) {
            let b = bytes_at(f.source, off + i);
            path = path_concat(path, path_single(b[7 .. 4]));
            path = path_concat(path, path_single(b[3 .. 0]))
        };
        (is_leaf, path)
    }
}

function path_matches(key : TriePath, pos : int, seg : TriePath) -> bool = {
    if pos + path_len(seg) > path_len(key) then {
        false
    } else {
        var ok : bool = true;
        foreach (k from 0 to (path_len(seg) - 1)) {
            if path_nibble(key, pos + k) != path_nibble(seg, k) then {
                ok = false
            }
        };
        ok
    }
}
