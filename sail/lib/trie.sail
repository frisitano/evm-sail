/* ======================================================================== */
/* MERKLE-PATRICIA TRIE: THE YELLOW PAPER DEFINITION                        */
/*                                                                          */
/* The canonical trie of Yellow Paper Appendices C and D, self-contained:   */
/* hex-prefix path encoding (HP, Appendix C), leaf / extension / branch     */
/* node composition with the < 32-byte node cap (n, Eq. 207), the ordered   */
/* update-list algebra, and the whole-map trie function                     */
/* TRIE(I) = KEC(RLP(c(I,0))) (Eq. 206/208) as updates_root / updates_ref   */
/* / updates_node over the live update set.                                 */
/*                                                                          */
/* This file is the semantic anchor: it never reads the witness node        */
/* database and performs no overlay. The witness machinery in lib/mpt.sail  */
/* (partial-trie re-rooting, the pre-state overlay, Merkle-proof reads)     */
/* refines this definition and must agree with it on the composed map.      */
/* ======================================================================== */

/* ---- small generic list helpers (take/drop n from the front) ----------- */
/* ---- shared trie primitives (formerly lib/mpt.sail) -------------------- */

function byte_list_nonempty(xs : list(byte)) -> bool =
    match xs {
        [||] => false,
        _ => true,
    }

/* A nibble is half a byte: the unit of trie path traversal (YP Appendix D:
 * "Any series of bytes may also trivially be viewed as a series of nibbles"). */
type nibble = bits(4)

function int_list_nonempty(xs : list(nibble)) -> bool =
    match xs {
        [||] => false,
        _ => true,
    }

/* 32-byte key -> 64 nibbles, high nibble first (single pass + reverse). */
function to_nibbles(bs : list(byte)) -> list(nibble) = {
    var out : list(nibble) = [||];
    var rest : list(byte) = bs;
    while byte_list_nonempty(rest) do {
        match rest {
            b :: r => {
                out = b[7 .. 4] :: out;
                out = b[3 .. 0] :: out;
                rest = r
            },
            [||] => (),
        }
    };
    list_reverse(out)
}

/* hex-prefix (compact) encoding of a nibble path + leaf/extension flag (YP C.1). */
function hp_compact(nibs : list(nibble), is_leaf : bool) -> list(byte) = {
    let len = list_len(nibs);
    let odd = emod_int(len, 2) == 1;
    let flag : nibble = if is_leaf then { 0x2 } else { 0x0 };
    var out : list(byte) = [||];
    var rest : list(nibble) = nibs;
    if odd then {
        match rest {
            n0 :: t => {
                out = append(flag | 0x1, n0) :: out;
                rest = t
            },
            [||] => (),
        }
    } else {
        out = append(flag, 0x0) :: out
    };
    while int_list_nonempty(rest) do {
        match rest {
            a :: (b :: t) => {
                out = append(a, b) :: out;
                rest = t
            },
            _ => rest = [||],
        }
    };
    list_reverse(out)
}

/* reference to a child node (Yellow Paper Appendix D, the node cap function n):
 * the node's RLP inline when under 32 bytes, otherwise 0xa0 ++ keccak256(node). */
val child_ref : list(byte) -> list(byte)

/* A trie leaf VALUE is the RLP encoding of the slot value (a string), e.g.
 * RLP(132) = 0x81 0x84, RLP(32-byte) = 0xa0 ++ 32 bytes. rlp_field_word reads
 * content as a raw big-endian word, which is only correct AFTER the RLP
 * string header is stripped: decode the short-string header (0x80..0xb7 -> drop
 * the length byte; a single byte < 0x80 is the value itself). Without this, a
 * small value keeps its 0x81 prefix (e.g. -> 0x8184) and is double-encoded on
 * re-RLP; large 32-byte values happened to work only because the 0xa0 prefix
 * overflows out of the 256-bit word. */
function rlp_str_word(bs : list(byte)) -> word =
    match bs {
        [||] => sail_zeros(256),
        b0 :: rest => if unsigned(b0) < 128 then {
            rlp_field_word(struct { is_list = false, content = bs, full = bs })
        } else {
            rlp_field_word(struct { is_list = false, content = rest, full = rest })
        },
    }

function list_take_prefix forall ('a : Type). (xs : list('a), n : int) -> list('a) = {
    var out : list('a) = [||];
    var rest : list('a) = xs;
    var i : int = 0;
    while i < n do {
        match rest {
            h :: t => {
                out = h :: out;
                rest = t
            },
            [||] => (),
        };
        i = i + 1
    };
    list_reverse(out)
}
function list_drop_prefix forall ('a : Type). (xs : list('a), n : int) -> list('a) = {
    var rest : list('a) = xs;
    var i : int = 0;
    while i < n do {
        match rest {
            _ :: t => rest = t,
            [||] => (),
        };
        i = i + 1
    };
    rest
}

/* ---- bit / mask helpers (16-bit branch masks held as int) --------------- */
function branch_mask_pow2(n : int) -> int = {
    var r : int = 1;
    var i : int = 0;
    while i < n do {
        r = r * 2;
        i = i + 1
    };
    r
}
function branch_mask_has_bit(mask : int, i : int) -> bool = emod_int(ediv_int(mask, branch_mask_pow2(i)), 2) == 1
function branch_mask_set_bit(mask : int, i : int) -> int = if branch_mask_has_bit(mask, i) then {
    mask
} else {
    mask + branch_mask_pow2(i)
}
function branch_mask_popcount(mask : int) -> int = {
    var c : int = 0;
    foreach (i from 0 to 15) {
        if branch_mask_has_bit(mask, i) then {
            c = c + 1
        }
    };
    c
}

/* longest common prefix length of two nibble lists */
function common_prefix_len(a : list(nibble), b : list(nibble)) -> int = {
    var i : int = 0;
    var ra : list(nibble) = a;
    var rb : list(nibble) = b;
    var stop : bool = false;
    while not_bool(stop) do {
        match (ra, rb) {
            (x :: ta, y :: tb) => if x == y then {
                i = i + 1;
                ra = ta;
                rb = tb
            } else {
                stop = true
            },
            (_, _) => stop = true,
        }
    };
    i
}

/* 32 big-endian bytes -> bits(256) (local; avoids clash with mpt_witness) */
function bytes_to_word_be256(c : list(byte)) -> bits(256) = {
    var acc : bits(256) = sail_zeros(256);
    let n = list_len(c);
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(list_nth(c, k, 0x00), 256))
    };
    acc
}

/* ---- node child-refs (the values the stack holds) ----------------------- */
/* leaf:      child_ref( rlp[ hp_compact(key,leaf) , value ] )                 */
/* extension: child_ref( rlp[ hp_compact(key,ext)  , child-ref ] )            */
/* branch:    child_ref( rlp[ 16 child-refs-or-0x80 , 0x80 ] )                 */
/* word:      0xa0 ++ hash   (a blinded subtree / >=32B child, already a ref)  */
function leaf_node_rlp(key : list(nibble), value : list(byte)) -> list(byte) =
    rlp_list(list_append(rlp_bytes(hp_compact(key, true)), rlp_bytes(value)))
function extension_node_rlp(key : list(nibble), childref : list(byte)) -> list(byte) =
    rlp_list(list_append(rlp_bytes(hp_compact(key, false)), childref))
function leaf_child_ref(key : list(nibble), value : list(byte)) -> list(byte) =
    child_ref(leaf_node_rlp(key, value))
function extension_child_ref(key : list(nibble), childref : list(byte)) -> list(byte) =
    child_ref(extension_node_rlp(key, childref))
function hash_child_ref(hash : list(byte)) -> list(byte) = list_append([|0xa0|], hash)

function branch_payload_rlp(mask : int, children : list(list(byte))) -> list(byte) = {
    var payload : list(byte) = [||];
    var ch : list(list(byte)) = children; /* ascending nibble order */
    foreach (i from 0 to 15) {
        if branch_mask_has_bit(mask, i) then {
            match ch {
                c :: t => {
                    payload = list_append(payload, c);
                    ch = t
                },
                [||] => (),
            }
        } else {
            payload = list_append(payload, [|0x80|])
        }
    };
    list_append(payload, [|0x80|]) /* branch value slot: empty */
}
function branch_node_rlp(mask : int, children : list(list(byte))) -> list(byte) =
    rlp_list(branch_payload_rlp(mask, children))
function branch_child_ref(mask : int, children : list(list(byte))) -> list(byte) =
    child_ref(branch_node_rlp(mask, children))

function trie_empty_ref() -> list(byte) = [|0x80|]

function trie_ref_is_empty(r : list(byte)) -> bool =
    list_len(r) == 1 & list_nth(r, 0, 0x00) == 0x80

val trie_ref_to_root : list(byte) -> bits(256)
val mpt_node_hash : list(byte) -> hash

function node_to_ref(node : list(byte)) -> list(byte) =
    if list_len(node) == 0 then {
        trie_empty_ref()
    } else {
        child_ref(node)
    }

/* ---- ordered post-state updates ---------------------------------------- */
/* lexicographic nibble-key order for recursive trie composition */
struct TrieUpdate = { key : list(nibble), is_delete : bool, value : list(byte) }

function nib_lt(a : list(nibble), b : list(nibble)) -> bool = {
    var ra : list(nibble) = a;
    var rb : list(nibble) = b;
    var res : int = 0; /* 0 undecided, 1 a<b, 2 a>=b */
    while res == 0 do {
        match (ra, rb) {
            ([||], [||]) => res = 2,
            ([||], _) => res = 1,
            (_, [||]) => res = 2,
            (x :: ta, y :: tb) => if unsigned(x) < unsigned(y) then {
                res = 1
            } else {
                if unsigned(x) > unsigned(y) then {
                    res = 2
                } else {
                    ra = ta;
                    rb = tb
                }
            },
        }
    };
    res == 1
}
function nib_eq(a : list(nibble), b : list(nibble)) -> bool =
    not_bool(nib_lt(a, b)) & not_bool(nib_lt(b, a))
function nib_prefix_of(prefix : list(nibble), key : list(nibble)) -> bool =
    match prefix {
        [||] => true,
        x :: xs => match key {
            [||] => false,
            y :: ys => (x == y) & nib_prefix_of(xs, ys),
        },
    }

function update_insert_sorted(u : TrieUpdate, us : list(TrieUpdate)) -> list(TrieUpdate) =
    match us {
        [||] => [|u|],
        v :: r => if nib_eq(u.key, v.key) then {
            u :: r
        } else {
            if nib_lt(u.key, v.key) then {
                u :: us
            } else {
                v :: update_insert_sorted(u, r)
            }
        },
    }

function updates_insert_all(src : list(TrieUpdate), dst : list(TrieUpdate)) -> list(TrieUpdate) =
    match src {
        [||]   => dst,
        u :: r => updates_insert_all(r, update_insert_sorted(u, dst)),
    }

function updates_strictly_sorted(us : list(TrieUpdate)) -> bool =
    match us {
        [||] => true,
        u :: r => match r {
            [||]   => true,
            v :: _ => nib_lt(u.key, v.key) & updates_strictly_sorted(r),
        },
    }

function updates_empty(us : list(TrieUpdate)) -> bool =
    match us {
        [||] => true,
        _    => false,
    }

function updates_has_live(us : list(TrieUpdate)) -> bool =
    match us {
        [||] => false,
        u :: r => if u.is_delete then {
            updates_has_live(r)
        } else {
            true
        },
    }

function updates_has_delete(us : list(TrieUpdate)) -> bool =
    match us {
        [||] => false,
        u :: r => u.is_delete | updates_has_delete(r),
    }

function updates_at_nibble(us : list(TrieUpdate), depth : int, nib : nibble) -> list(TrieUpdate) =
    match us {
        [||] => [||],
        u :: r => if list_len(u.key) > depth & list_nth(u.key, depth, 0x0) == nib then {
            u :: updates_at_nibble(r, depth, nib)
        } else {
            updates_at_nibble(r, depth, nib)
        },
    }

function updates_all_under_prefix(us : list(TrieUpdate), prefix : list(nibble)) -> bool =
    match us {
        [||]   => true,
        u :: r => nib_prefix_of(prefix, u.key) & updates_all_under_prefix(r, prefix),
    }

function updates_under_prefix(us : list(TrieUpdate), prefix : list(nibble)) -> list(TrieUpdate) =
    match us {
        [||] => [||],
        u :: r => if nib_prefix_of(prefix, u.key) then {
            u :: updates_under_prefix(r, prefix)
        } else {
            updates_under_prefix(r, prefix)
        },
    }

function updates_outside_prefix(us : list(TrieUpdate), prefix : list(nibble)) -> list(TrieUpdate) =
    match us {
        [||] => [||],
        u :: r => if nib_prefix_of(prefix, u.key) then {
            updates_outside_prefix(r, prefix)
        } else {
            u :: updates_outside_prefix(r, prefix)
        },
    }

function updates_has_key(us : list(TrieUpdate), key : list(nibble)) -> bool =
    match us {
        [||] => false,
        u :: r => if nib_eq(u.key, key) then {
            true
        } else {
            updates_has_key(r, key)
        },
    }

/* ---- the Yellow Paper trie functions (Appendix D) -----------------------
 *
 * updates_root / updates_ref / updates_node ARE the protocol definition of
 * the trie, transcribed from the Yellow Paper (Shanghai version) and
 * specialized to the secure trie's fixed-length 64-nibble keys (so a branch
 * node's value slot -- the 17th item -- is always empty):
 *
 *   TRIE(I) = KEC(RLP(c(I,0)))                     (Eq. 206)  updates_root
 *   n(I,i): () if I = {}; c(I,i) if ||RLP|| < 32;
 *           KEC(RLP(c(I,i))) otherwise             (Eq. 207)  updates_ref
 *   c(I,i): leaf | extension | 17-item branch      (Eq. 208)  updates_node
 *
 * I is the LIVE key/value set: delete rows are identities of the whole-map
 * function (they exist for the pre-state overlay, which erases leaves), so c
 * consumes only the surviving pairs. The overlay machinery below refines
 * this definition over (pre-state map) + updates; this build is the anchor
 * it must agree with. */

function updates_filter_live(us : list(TrieUpdate)) -> list(TrieUpdate) =
    match us {
        [||] => [||],
        u :: r => if u.is_delete then {
            updates_filter_live(r)
        } else {
            u :: updates_filter_live(r)
        },
    }

/* the maximal key prefix shared by every entry past `depth` (Eq. 208's
 * j = max{x : all keys agree on nibbles i..x}, returned as the prefix) */
function updates_shared_prefix(us : list(TrieUpdate), depth : int) -> list(nibble) =
    match us {
        [||] => [||],
        u :: [||] => list_drop_prefix(u.key, depth),
        u :: r => {
            let mine = list_drop_prefix(u.key, depth);
            let rest = updates_shared_prefix(r, depth);
            list_take_prefix(mine, common_prefix_len(mine, rest))
        },
    }

/* c(I, i) -- Eq. 208: the structural composition of the live set at nibble
 * depth `depth`; [||] denotes the empty node. `fuel` only bounds recursion
 * for the termination checker: keys are 64 nibbles, callers pass >= 256. */
function updates_node(us0 : list(TrieUpdate), depth : int, fuel : int) -> list(byte) = {
    if fuel <= 0 then {
        k_witness_bad = true;
        [||]
    } else {
        let us = updates_filter_live(us0);
        match us {
            [||] => [||],
            u :: [||] => leaf_node_rlp(list_drop_prefix(u.key, depth), u.value),
            _ => {
                let shared = updates_shared_prefix(us, depth);
                if int_list_nonempty(shared) then {
                    /* extension: >= 2 distinct keys share `shared`, so the
                       child one level down is necessarily a branch */
                    let sn = list_len(shared);
                    extension_node_rlp(shared, node_to_ref(updates_node(us, depth + sn, fuel - sn)))
                } else {
                    /* branch: u(j) = n({I : key[depth] = j}, depth+1); the
                       shared-prefix maximality above guarantees >= 2 children */
                    var mask : int = 0;
                    var children : list(list(byte)) = [||];
                    foreach (i from 0 to 15) {
                        let group = updates_at_nibble(us, depth, get_slice_int(4, i, 0));
                        if list_len(group) > 0 then {
                            mask = branch_mask_set_bit(mask, i);
                            children = list_append(children, [|node_to_ref(updates_node(group, depth + 1, fuel - 1))|])
                        }
                    };
                    branch_node_rlp(mask, children)
                }
            },
        }
    }
}
termination_measure updates_node(us0, depth, fuel) = fuel

/* n(I, i) -- Eq. 207: the node cap (inline when RLP < 32 bytes, else the
 * keccak reference) */
function updates_ref(us : list(TrieUpdate), depth : int, fuel : int) -> list(byte) =
    node_to_ref(updates_node(us, depth, fuel))

/* TRIE(I) -- Eq. 206: the root of the trie representing the live update set */
function updates_root(us : list(TrieUpdate)) -> bits(256) =
    trie_ref_to_root(updates_ref(us, 0, 256))
