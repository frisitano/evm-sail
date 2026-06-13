/* ======================================================================== */
/* Merkle-Patricia Trie root (Ethereum) in Sail                              */
/*                                                                           */
/* Computes a trie root from a set of (key-nibbles, value) pairs via the      */
/* standard recursive construction (leaf / extension / branch nodes with      */
/* hex-prefix path encoding, RLP, and keccak). Used for the state root,       */
/* storage roots, and (with index keys) the tx/receipt roots -- i.e. the      */
/* pieces needed to authenticate a block's pre/post state and roots. Keys are */
/* expected pre-hashed (secure trie): the caller passes keccak(address) etc.  */
/* Self-contained -> compiles to the RISC-V target.                          */
/* ======================================================================== */

struct TriePair = { tp_key : list(int), tp_val : list(byte) }

/* key = nibbles */

/* 32-byte key -> 64 nibbles (high nibble first). O(n): single pass + reverse
 * (was O(n^2): list_append + per-index list_nth on a fat list(int)). */
function to_nibbles(bs : list(byte)) -> list(int) = {
    var out : list(int) = [||];
    var rest : list(byte) = bs;
    while match rest {
        [||] => false,
        _ => true,
    } do {
        match rest {
            b :: r => {
                let bb = unsigned(b);
                out = ediv_int(bb, 16) :: out;
                out = emod_int(bb, 16) :: out;
                rest = r
            },
            [||]   => (),
        }
    };
    list_reverse(out)
}

function nib_at(key : list(int), i : int) -> int = list_nth(key, i, 0)

/* hex-prefix (compact) encoding of a nibble path + leaf/extension flag.
 * O(n): single pass over nibs + reverse (was O(n^2)). */
function hp_compact(nibs : list(int), is_leaf : bool) -> list(byte) = {
    let len = list_len(nibs);
    let odd = emod_int(len, 2) == 1;
    let flag : int = if is_leaf then { 2 } else { 0 };
    var out : list(byte) = [||];
    var rest : list(int) = nibs;
    if odd then {
        match rest {
            n0 :: t => {
                out = get_slice_int(8, (flag + 1) * 16 + n0, 0) :: out;
                rest = t
            },
            [||] => (),
        }
    } else {
        out = get_slice_int(8, flag * 16, 0) :: out
    };
    while match rest {
        [||] => false,
        _ => true,
    } do {
        match rest {
            a :: (b :: t) => {
                out = get_slice_int(8, a * 16 + b, 0) :: out;
                rest = t
            },
            _             => rest = [||],
        }
    };
    list_reverse(out)
}

/* nibbles [lo, hi) of a key. O(hi): drop lo, take hi-lo (was O((hi-lo)^2)). */
function nib_slice(key : list(int), lo : int, hi : int) -> list(int) = {
    var rest : list(int) = key;
    var i : int = 0;
    while i < lo do {
        match rest {
            _ :: t => rest = t,
            [||] => (),
        };
        i = i + 1
    };
    var out : list(int) = [||];
    var j : int = 0;
    while j < (hi - lo) do {
        match rest {
            h :: t => {
                out = h :: out;
                rest = t
            },
            [||] => (),
        };
        j = j + 1
    };
    list_reverse(out)
}

/* remaining-nibble sum, the termination measure for build. */
function remaining(pairs : list(TriePair), depth : int) -> int =
    match pairs {
        [||] => 0,
        p :: r => (list_len(p.tp_key) - depth) + remaining(r, depth),
    }

/* longest common nibble prefix of all keys, starting at `depth`. */
function common_prefix(pairs : list(TriePair), depth : int) -> int = {
    match pairs {
        [||] => 0,
        p0 :: _ => {
            let key0 = p0.tp_key;
            var cp : int = 0;
            var stop : bool = false;
            while not_bool(stop) do {
                let pos = depth + cp;

                /* every key must be long enough and agree at `pos` */
                var ok : bool = true;
                var ps : list(TriePair) = pairs;
                while match ps {
                    [||] => false,
                    _ => true,
                } do {
                    match ps {
                        [||] => (),
                        q :: rest => {
                            if list_len(q.tp_key) <= pos | nib_at(q.tp_key, pos) != nib_at(key0, pos) then {
                                ok = false
                            };
                            ps = rest
                        },
                    }
                };
                if ok then {
                    cp = cp + 1
                } else {
                    stop = true
                }
            };
            cp
        },
    }
}

/* pairs whose key has nibble `i` at position `depth`. */
function group_by_nibble(pairs : list(TriePair), depth : int, i : int) -> list(TriePair) =
    match pairs {
        [||] => [||],
        p :: r => if list_len(p.tp_key) > depth & nib_at(p.tp_key, depth) == i then {
            p :: group_by_nibble(r, depth, i)
        } else {
            group_by_nibble(r, depth, i)
        },
    }

/* value of the pair (if any) whose key ends exactly at `depth`. */
function value_at_depth(pairs : list(TriePair), depth : int) -> list(byte) =
    match pairs {
        [||] => [||],
        p :: r => if list_len(p.tp_key) == depth then {
            p.tp_val
        } else {
            value_at_depth(r, depth)
        },
    }

/* forward decl for the build<->child_ref recursion */
val build_node : (list(TriePair), int) -> list(byte)

/* reference to a child node: inline if its RLP < 32 bytes, else its keccak. */
function child_ref(enc : list(byte)) -> list(byte) =
    if list_len(enc) < 32 then {
        enc
    } else {
        rlp_bytes(word_to_bytes32(keccak256(enc)))
    }

/* build the RLP encoding of the node covering `pairs` at `depth`. */
function build_node(pairs : list(TriePair), depth : int) -> list(byte) =
    match pairs {
        [||] => rlp_bytes([||]),
        p :: [||] => {
            let path = nib_slice(p.tp_key, depth, list_len(p.tp_key));
            rlp_list(list_append(rlp_bytes(hp_compact(path, true)), rlp_bytes(p.tp_val)))
        },
        _ => {
            let cp = common_prefix(pairs, depth);
            if cp > 0 then {
                let path = nib_slice(
                    match pairs {
                        q :: _ => q.tp_key,
                        [||] => [||],
                    },
                    depth,
                    depth + cp,
                );
                let child = build_node(pairs, depth + cp);
                rlp_list(list_append(rlp_bytes(hp_compact(path, false)), child_ref(child)))
            } else {
                var payload : list(byte) = [||];
                foreach (i from 0 to 15) {
                    let g = group_by_nibble(pairs, depth, i);
                    match g {
                        [||] => payload = list_append(payload, rlp_bytes([||])),
                        _    => payload = list_append(payload, child_ref(build_node(g, depth + 1))),
                    }
                };
                let v = value_at_depth(pairs, depth);
                payload = list_append(payload, rlp_bytes(v));
                rlp_list(payload)
            }
        },
    }
termination_measure build_node(pairs, depth) = remaining(pairs, depth)

/* trie root: keccak of the root node's RLP (empty trie -> keccak(0x80)). */
function trie_root(pairs : list(TriePair)) -> bits(256) = keccak256(build_node(pairs, 0))
