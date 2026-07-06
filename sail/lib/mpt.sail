/* ======================================================================== */
/* MERKLE-PATRICIA TRIE AND WORLD-STATE ROOTS: ONE IMPLEMENTATION           */
/*                                                                          */
/* Single-implementation contract: every trie root in the system is         */
/* computed by ONE witness-native implementation with ONE public entry,     */
/* trie_root(base_root, updates) -- the recursive overlay of an ordered     */
/* patch list on the trie anchored at base_root. It operates over           */
/* comprehensive witnesses: every hash reference that must be resolved is   */
/* looked up in the witness node database, and any missing node material    */
/* aborts the computation (k_witness_bad, fail closed). There is no         */
/* separate from-scratch ("yellow paper") trie build; the Yellow Paper      */
/* survives as documentation -- Appendix C.1 (hex-prefix) and Appendix D    */
/* (Eq. 206 TRIE / Eq. 207 n / Eq. 208 c) are cited on the corresponding    */
/* internal functions, and restricting trie_root to an empty base computes  */
/* TRIE(I) directly (see the remark at trie_root).                          */
/*                                                                          */
/* Layers:                                                                  */
/* (1) trie primitives: nibbles, hex-prefix paths, node RLP composition,    */
/*     child references with the < 32-byte inline cap.                      */
/* (2) the patch algebra: ONE ordered patch type (leaf write / delete /     */
/*     subtree reference, optionally carrying overlaid node bytes) and the  */
/*     recursive composition of pure-patch subtrees.                        */
/* (3) the witness overlay: recursive traversal of the witnessed node       */
/*     database applying patches along touched paths, preserving untouched  */
/*     (possibly blinded) children by reference, and canonicalizing trie    */
/*     shape after deletes. trie_root is the single entry.                  */
/* (4) world state: account/storage update collection and the state root    */
/*     (keyed by keccak256(address) / keccak256(slot), value                */
/*     RLP([nonce, balance, storageRoot, codeHash])).                       */
/* (5) lazy witness reads: authenticated Merkle walks resolving accounts    */
/*     and storage slots from the anchored parent root.                     */
/*                                                                          */
/* Design decision (final): post-state roots are computed by BATCH overlay  */
/* -- one recursive pass applying the whole ordered update list -- NOT by   */
/* per-op insert/delete maintenance. The Yellow Paper defines the trie      */
/* declaratively over the final map (Appendix D; S D.1 leaves maintenance   */
/* strategy to implementations); the batch pass touches and hashes each     */
/* node once, and a per-op delete would face the same blinded-sibling       */
/* collapse problem while multiplying hashing. Do not migrate this to       */
/* incremental per-operation insert/delete.                                 */
/* ======================================================================== */

/* ======================================================================== */
/* (1) trie primitives                                                      */
/* ======================================================================== */

function list_nonempty forall ('a : Type). (xs : list('a)) -> bool =
    match xs {
        [||] => false,
        _ => true,
    }

/* A nibble is half a byte: the unit of trie path traversal (YP Appendix D:
 * "Any series of bytes may also trivially be viewed as a series of nibbles"). */
type nibble = bits(4)

/* 32-byte key -> 64 nibbles, high nibble first (single pass + reverse). */
function to_nibbles(bs : list(byte)) -> list(nibble) = {
    var out : list(nibble) = [||];
    var rest : list(byte) = bs;
    while list_nonempty(rest) do {
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

/* hex-prefix (compact) encoding of a nibble path + leaf/extension flag
 * (Yellow Paper Appendix C.1, the HP function). */
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
    while list_nonempty(rest) do {
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

/* hex-prefix decode (Appendix C.1 inverted): (is_leaf, path nibbles) */
function hp_decode(p : list(byte)) -> (bool, list(nibble)) = {
    let n = list_len(p);
    if n == 0 then {
        (false, [||])
    } else {
        let fb = bytes_get(p, 0);
        let flag : nibble = fb[7 .. 4];
        let is_leaf : bool = flag[1] == bitone;
        let odd : bool = flag[0] == bitone;
        var rev : list(nibble) = [||];
        if odd then {
            rev = fb[3 .. 0] :: rev
        };
        foreach (i from 1 to (n - 1)) {
            let b = bytes_get(p, i);
            rev = b[7 .. 4] :: rev;
            rev = b[3 .. 0] :: rev
        };
        (is_leaf, list_reverse(rev))
    }
}

/* do seg nibbles match key[pos ..] ? */
function nibs_match(key : list(nibble), pos : int, seg : list(nibble)) -> bool = {
    let m = list_len(seg);
    if pos + m > list_len(key) then {
        false
    } else {
        var ok : bool = true;
        foreach (k from 0 to (m - 1)) {
            if list_nth(key, pos + k, 0x0) != list_nth(seg, k, 0x0) then {
                ok = false
            }
        };
        ok
    }
}

/* reference to a child node (Yellow Paper Appendix D, the node cap function
 * n, Eq. 207): the node's RLP inline when under 32 bytes, otherwise
 * 0xa0 ++ keccak256(node). Defined in lib/consensus_hash.sail. */
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

/* ---- branch masks: bit i set = child at nibble i present ---------------- */
function branch_mask_has_bit(mask : bits(16), i : int) -> bool =
    (sail_shiftright(mask, i) & 0x0001) == 0x0001
function branch_mask_set_bit(mask : bits(16), i : int) -> bits(16) =
    mask | sail_shiftleft(0x0001, i)
function branch_mask_popcount(mask : bits(16)) -> int = {
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

/* big-endian bytes -> 256-bit word */
function bytes_be256(c : list(byte)) -> bits(256) = {
    var acc : bits(256) = sail_zeros(256);
    let n = list_len(c);
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(list_nth(c, k, 0x00), 256))
    };
    acc
}

/* ---- node composition (Yellow Paper Appendix D, Eq. 208's node forms) --- */
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

function branch_payload_rlp(mask : bits(16), children : list(list(byte))) -> list(byte) = {
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
function branch_node_rlp(mask : bits(16), children : list(list(byte))) -> list(byte) =
    rlp_list(branch_payload_rlp(mask, children))
function branch_child_ref(mask : bits(16), children : list(list(byte))) -> list(byte) =
    child_ref(branch_node_rlp(mask, children))

function trie_empty_ref() -> list(byte) = [|0x80|]

function trie_ref_is_empty(r : list(byte)) -> bool =
    list_len(r) == 1 & list_nth(r, 0, 0x00) == 0x80

val trie_ref_to_root : list(byte) -> bits(256)

function node_to_ref(node : list(byte)) -> list(byte) =
    if list_len(node) == 0 then {
        trie_empty_ref()
    } else {
        child_ref(node)
    }

/* ---- lexicographic nibble-key order (recursive trie composition) -------- */
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
    match (a, b) {
        ([||], [||]) => true,
        (x :: ta, y :: tb) => (x == y) & nib_eq(ta, tb),
        (_, _) => false,
    }
function nib_prefix_of(prefix : list(nibble), key : list(nibble)) -> bool =
    match prefix {
        [||] => true,
        x :: xs => match key {
            [||] => false,
            y :: ys => (x == y) & nib_prefix_of(xs, ys),
        },
    }

/* ======================================================================== */
/* (2) the patch algebra                                                    */
/*                                                                          */
/* THE single update/patch type. A patch is one of:                         */
/*   - a leaf write  (is_ref = false, is_delete = false, bytes = RLP value) */
/*   - a leaf delete (is_ref = false, is_delete = true; the key's leaf is   */
/*     erased from the composed map)                                        */
/*   - a subtree reference (is_ref = true, bytes = child ref), produced     */
/*     only INSIDE the overlay for preserved/overlaid subtrees.             */
/* For subtree refs, `node` optionally carries the referenced node's full   */
/* RLP: an overlaid subtree exists only in memory (its post-state hash is   */
/* NOT a witness node-db key), so a later collapse into the ref must use    */
/* these carried bytes -- a node-db lookup of the fresh hash can never      */
/* succeed. `node` is empty for untouched witness subtrees, where the       */
/* node-db lookup is the (EIP-8025 guaranteed) collapse-material path.      */
/* ======================================================================== */
struct TriePatch = {
    key : list(nibble),
    is_delete : bool,
    is_ref : bool,
    bytes : list(byte),
    node : list(byte),
}

function patch_leaf(key : list(nibble), value : list(byte)) -> TriePatch =
    struct { key = key, is_delete = false, is_ref = false, bytes = value, node = [||] }

function patch_delete(key : list(nibble)) -> TriePatch =
    struct { key = key, is_delete = true, is_ref = false, bytes = [||], node = [||] }

function patch_ref(key : list(nibble), child : list(byte)) -> TriePatch =
    struct { key = key, is_delete = false, is_ref = true, bytes = child, node = [||] }

function patch_live(p : TriePatch) -> bool = p.is_ref | not_bool(p.is_delete)

function patch_insert_sorted(p : TriePatch, ps : list(TriePatch)) -> list(TriePatch) =
    match ps {
        [||] => [|p|],
        q :: r => if nib_eq(p.key, q.key) then {
            p :: r
        } else {
            if nib_lt(p.key, q.key) then {
                p :: ps
            } else {
                q :: patch_insert_sorted(p, r)
            }
        },
    }

function patches_empty(ps : list(TriePatch)) -> bool =
    match ps {
        [||] => true,
        _    => false,
    }

function patches_has_live(ps : list(TriePatch)) -> bool =
    match ps {
        [||] => false,
        p :: r => if patch_live(p) then {
            true
        } else {
            patches_has_live(r)
        },
    }

function patches_has_delete(ps : list(TriePatch)) -> bool =
    match ps {
        [||] => false,
        p :: r => p.is_delete | patches_has_delete(r),
    }

function patches_has_key(ps : list(TriePatch), key : list(nibble)) -> bool =
    match ps {
        [||] => false,
        p :: r => if nib_eq(p.key, key) then {
            true
        } else {
            patches_has_key(r, key)
        },
    }

function patches_filter_live(ps : list(TriePatch)) -> list(TriePatch) =
    match ps {
        [||] => [||],
        p :: r => if patch_live(p) then {
            p :: patches_filter_live(r)
        } else {
            patches_filter_live(r)
        },
    }

function patches_at_nibble(ps : list(TriePatch), depth : int, nib : nibble) -> list(TriePatch) =
    match ps {
        [||] => [||],
        p :: r => if list_len(p.key) > depth & list_nth(p.key, depth, 0x0) == nib then {
            p :: patches_at_nibble(r, depth, nib)
        } else {
            patches_at_nibble(r, depth, nib)
        },
    }

function patches_first_key(ps : list(TriePatch)) -> list(nibble) =
    match ps {
        [||]   => [||],
        p :: _ => p.key,
    }

function patches_last_key(ps : list(TriePatch)) -> list(nibble) =
    match ps {
        [||] => [||],
        p :: r => match r {
            [||] => p.key,
            _    => patches_last_key(r),
        },
    }

function patches_all_under_prefix(ps : list(TriePatch), prefix : list(nibble)) -> bool =
    match ps {
        [||]   => true,
        p :: r => nib_prefix_of(prefix, p.key) & patches_all_under_prefix(r, prefix),
    }

function patches_under_prefix(ps : list(TriePatch), prefix : list(nibble)) -> list(TriePatch) =
    match ps {
        [||] => [||],
        p :: r => if nib_prefix_of(prefix, p.key) then {
            p :: patches_under_prefix(r, prefix)
        } else {
            patches_under_prefix(r, prefix)
        },
    }

function patches_outside_prefix(ps : list(TriePatch), prefix : list(nibble)) -> list(TriePatch) =
    match ps {
        [||] => [||],
        p :: r => if nib_prefix_of(prefix, p.key) then {
            patches_outside_prefix(r, prefix)
        } else {
            p :: patches_outside_prefix(r, prefix)
        },
    }

/* ======================================================================== */
/* (3) the witness overlay                                                  */
/*                                                                          */
/* Stateless verification of the world-state trie from a Merkle witness     */
/* (the node set covering the proof paths), mirroring execution-specs       */
/* witness_state.py. A node database keyed by keccak256(node) (C-backed,    */
/* ffi/trie_node_db.c) backs every read. A node absent from the witness on  */
/* a needed path is a missing proof and fails closed (k_witness_bad).       */
/* ======================================================================== */

/* node bytes for keccak `h` from the C node-db, or [] if absent (read straight
 * from the witness buffer by the recorded span -- no per-node Sail copy held). */
function node_db_lookup(h : bits(256)) -> list(byte) = {
    if not_bool(nodedb_sel(h)) then {
        [||]
    } else {
        ssz_bytes(unsigned(nodedb_sel_off()), unsigned(nodedb_sel_len()))
    }
}

/* resolve a child reference: embedded node (full bytes), 32-byte hash, or empty.
 * A 32-byte hash that is absent from the witness is a MISSING node (the prover
 * omitted a node on a walked path) -- fail closed, so a resolved-empty result
 * always means a legitimate exclusion, never a withheld subtree. */
function resolve_child(f : RlpField) -> list(byte) =
    if f.is_list then {
        f.full
    } else {
        if list_len(f.content) == 32 then {
            let node = node_db_lookup(bytes_be256(f.content));

            /* a 32-byte ref absent from the witness means the proof is deficient:
               flag the block invalid (valid=false) and end the walk here, rather
               than aborting -- a missing proof is a NORMAL failure outcome
               (EIP-8025 optional proofs), not a guest crash. */
            if list_len(node) == 0 then {
                k_witness_bad = true
            };
            node
        } else {
            [||]
        }
    }

function field_to_ref(f : RlpField) -> list(byte) =
    if f.is_list then {
        f.full
    } else {
        if list_len(f.content) == 32 then {
            hash_child_ref(f.content)
        } else {
            trie_empty_ref()
        }
    }

function merge_ext_node(prefix : list(nibble), childnode : list(byte)) -> list(byte) = {
    if list_len(prefix) == 0 then {
        childnode
    } else {
        if list_len(childnode) == 0 then {
            [||]
        } else {
            let items = rlp_node_items(childnode);
            let n = list_len(items);
            if n == 2 then {
                let (is_leaf, seg) = hp_decode(list_nth(items, 0, RLP_NONE).content);
                let key = list_append(prefix, seg);
                if is_leaf then {
                    leaf_node_rlp(key, list_nth(items, 1, RLP_NONE).content)
                } else {
                    extension_node_rlp(key, field_to_ref(list_nth(items, 1, RLP_NONE)))
                }
            } else {
                extension_node_rlp(prefix, node_to_ref(childnode))
            }
        }
    }
}

function merge_ext_ref(prefix : list(nibble), childref : list(byte)) -> list(byte) = {
    if list_len(prefix) == 0 then {
        childref
    } else {
        if trie_ref_is_empty(childref) then {
            trie_empty_ref()
        } else {
            match childref {
                0xa0 :: _ => extension_child_ref(prefix, childref),
                _ => {
                    let items = rlp_node_items(childref);
                    let n = list_len(items);
                    if n == 2 then {
                        let (is_leaf, seg) = hp_decode(list_nth(items, 0, RLP_NONE).content);
                        let key = list_append(prefix, seg);
                        if is_leaf then {
                            leaf_child_ref(key, list_nth(items, 1, RLP_NONE).content)
                        } else {
                            extension_child_ref(key, field_to_ref(list_nth(items, 1, RLP_NONE)))
                        }
                    } else {
                        extension_child_ref(prefix, childref)
                    }
                },
            }
        }
    }
}

function mask_single_index(mask : bits(16)) -> int = {
    var ix : int = 0;
    foreach (i from 0 to 15) {
        if branch_mask_has_bit(mask, i) then {
            ix = i
        }
    };
    ix
}

function first_child_ref(children : list(list(byte))) -> list(byte) =
    match children {
        c :: _ => c,
        [||]   => trie_empty_ref(),
    }

/* the Yellow Paper's branch canonicalization invariant (Appendix D): "a
 * branch is then only used when necessary; no branch nodes may exist that
 * contain only a single non-zero entry" -- a lone surviving child collapses
 * into its parent as an extension. Only overlay deletes can create this
 * shape; a pure-patch subtree is canonical by construction. */
function canonical_branch_ref(mask : bits(16), children : list(list(byte))) -> list(byte) =
    if mask == 0x0000 then {
        trie_empty_ref()
    } else {
        if branch_mask_popcount(mask) == 1 then {
            merge_ext_ref([|get_slice_int(4, mask_single_index(mask), 0)|], first_child_ref(children))
        } else {
            branch_child_ref(mask, children)
        }
    }

function patch_insert_ref_if_live(key : list(nibble), child : list(byte), ps : list(TriePatch)) -> list(TriePatch) =
    if trie_ref_is_empty(child) then {
        ps
    } else {
        patch_insert_sorted(patch_ref(key, child), ps)
    }

/* subtree-ref patch for a freshly OVERLAID (in-memory) child node: the ref is
 * derived from the node, and the node bytes ride along for collapse (see
 * TriePatch). An empty node (subtree fully deleted) inserts nothing. */
function patch_insert_subtree_if_live(
    key : list(nibble),
    childnode : list(byte),
    ps : list(TriePatch),
) -> (
    list(TriePatch)
) = {
    let childref = node_to_ref(childnode);
    if trie_ref_is_empty(childref) then {
        ps
    } else {
        patch_insert_sorted(
            struct { key = key, is_delete = false, is_ref = true, bytes = childref, node = childnode },
            ps,
        )
    }
}

function patch_entry_ref(p : TriePatch, depth : int) -> list(byte) = {
    let suffix = list_drop_prefix(p.key, depth);
    if p.is_ref then {
        if list_len(p.node) > 0 then {
            /* overlaid subtree: merge on the carried node bytes so a non-branch
             * top node (post-delete collapse) merges keys canonically */
            node_to_ref(merge_ext_node(suffix, p.node))
        } else {
            merge_ext_ref(suffix, p.bytes)
        }
    } else {
        leaf_child_ref(suffix, p.bytes)
    }
}

function ref_node_for_collapse(r : list(byte)) -> list(byte) =
    if trie_ref_is_empty(r) then {
        [||]
    } else {
        match r {
            0xa0 :: h => if list_len(h) == 32 then {
                let node = node_db_lookup(bytes_be256(h));
                if list_len(node) == 0 then {
                    k_witness_bad = true
                };
                node
            } else {
                r
            },
            _ => r,
        }
    }

function patch_entry_node(p : TriePatch, depth : int) -> list(byte) = {
    let suffix = list_drop_prefix(p.key, depth);
    if p.is_ref then {
        if list_len(p.node) > 0 then {
            /* overlaid subtree: its hash is not a node-db key, so the collapse
             * (and canonical key-merge) must use the carried node bytes */
            merge_ext_node(suffix, p.node)
        } else {
            if list_len(suffix) == 0 then {
                ref_node_for_collapse(p.bytes)
            } else {
                extension_node_rlp(suffix, p.bytes)
            }
        }
    } else {
        leaf_node_rlp(suffix, p.bytes)
    }
}

/* ---- pure-patch subtree composition (Yellow Paper Appendix D) -----------
 *
 * On a patch list of pure leaf writes/deletes, patches_node / node_to_ref /
 * patches_ref ARE the protocol trie functions, transcribed from the Yellow
 * Paper (Shanghai version) and specialized to the secure trie's fixed-length
 * 64-nibble keys (so a branch node's value slot -- the 17th item -- is
 * always empty):
 *
 *   c(I,i): leaf | extension | 17-item branch      (Eq. 208)  patches_node
 *   n(I,i): () if I = {}; c(I,i) if ||RLP|| < 32;
 *           KEC(RLP(c(I,i))) otherwise             (Eq. 207)  patches_ref
 *   TRIE(I) = KEC(RLP(c(I,0)))                     (Eq. 206)  trie_root on
 *                                                             an empty base
 *
 * I is the LIVE key/value set: delete rows are identities of the whole-map
 * function (they exist to erase base-trie leaves), so c consumes only the
 * surviving pairs. This construction is NOT an independent entry point: it
 * is the overlay's internal case for pure-patch subtrees -- the composition
 * below an empty or fully-consumed base node. */

function patches_ref(ps0 : list(TriePatch), depth : int, fuel : int) -> list(byte) = {
    if fuel <= 0 then {
        k_witness_bad = true;
        trie_empty_ref()
    } else {
        let ps = patches_filter_live(ps0);
        match ps {
            [||] => trie_empty_ref(),
            p :: [||] => patch_entry_ref(p, depth),
            _ => {
                let first = patches_first_key(ps);
                let last = patches_last_key(ps);
                let common = common_prefix_len(list_drop_prefix(first, depth), list_drop_prefix(last, depth));
                if common > 0 then {
                    let prefix = list_take_prefix(list_drop_prefix(first, depth), common);
                    merge_ext_ref(prefix, patches_ref(ps, depth + common, fuel - common))
                } else {
                    var mask : bits(16) = 0x0000;
                    var children : list(list(byte)) = [||];
                    foreach (i from 0 to 15) {
                        let group = patches_at_nibble(ps, depth, get_slice_int(4, i, 0));
                        if list_len(group) > 0 then {
                            mask = branch_mask_set_bit(mask, i);
                            children = list_append(children, [|patches_ref(group, depth + 1, fuel - 1)|])
                        }
                    };
                    canonical_branch_ref(mask, children)
                }
            },
        }
    }
}
termination_measure patches_ref(ps0, depth, fuel) = fuel

function patches_node(ps0 : list(TriePatch), depth : int, fuel : int) -> list(byte) = {
    if fuel <= 0 then {
        k_witness_bad = true;
        [||]
    } else {
        let ps = patches_filter_live(ps0);
        match ps {
            [||] => [||],
            p :: [||] => patch_entry_node(p, depth),
            _ => {
                let first = patches_first_key(ps);
                let last = patches_last_key(ps);
                let common = common_prefix_len(list_drop_prefix(first, depth), list_drop_prefix(last, depth));
                if common > 0 then {
                    let prefix = list_take_prefix(list_drop_prefix(first, depth), common);
                    merge_ext_node(prefix, patches_node(ps, depth + common, fuel - common))
                } else {
                    var mask : bits(16) = 0x0000;
                    var children : list(list(byte)) = [||];
                    var lone_nib : nibble = 0x0;
                    var lone_child : list(byte) = [||];
                    foreach (i from 0 to 15) {
                        let group = patches_at_nibble(ps, depth, get_slice_int(4, i, 0));
                        if list_len(group) > 0 then {
                            let cnode = patches_node(group, depth + 1, fuel - 1);
                            if list_len(cnode) > 0 then {
                                mask = branch_mask_set_bit(mask, i);
                                children = list_append(children, [|node_to_ref(cnode)|]);
                                lone_nib = get_slice_int(4, i, 0);
                                lone_child = cnode
                            }
                        }
                    };
                    if mask == 0x0000 then {
                        [||]
                    } else {
                        if branch_mask_popcount(mask) == 1 then {
                            merge_ext_node([|lone_nib|], lone_child)
                        } else {
                            branch_node_rlp(mask, children)
                        }
                    }
                }
            },
        }
    }
}
termination_measure patches_node(ps0, depth, fuel) = fuel

function leaf_overlay_patches(key : list(nibble), value : list(byte), ps : list(TriePatch)) -> list(TriePatch) =
    if patches_has_key(ps, key) then {
        ps
    } else {
        patch_insert_sorted(patch_leaf(key, value), ps)
    }

val trie_overlay_node_ref : (list(byte), list(nibble), list(TriePatch), int) -> list(byte)
val trie_overlay_child_ref : (RlpField, list(nibble), list(TriePatch), int) -> list(byte)
val trie_overlay_node_for_collapse : (list(byte), list(nibble), list(TriePatch), int) -> list(byte)
val trie_overlay_child_node_for_collapse : (RlpField, list(nibble), list(TriePatch), int) -> list(byte)
val trie_overlay_node_ref_preserve : (list(byte), list(nibble), list(TriePatch), int) -> list(byte)
val trie_overlay_child_ref_preserve : (RlpField, list(nibble), list(TriePatch), int) -> list(byte)

function trie_overlay_child_ref_preserve(
    f : RlpField,
    prefix : list(nibble),
    updates : list(TriePatch),
    fuel : int,
) -> (
    list(byte)
) = {
    if patches_empty(updates) then {
        field_to_ref(f)
    } else {
        if f.is_list then {
            trie_overlay_node_ref_preserve(f.full, prefix, updates, fuel)
        } else {
            let c = f.content;
            if list_len(c) == 0 then {
                patches_ref(updates, list_len(prefix), fuel)
            } else {
                if list_len(c) == 32 then {
                    let node = node_db_lookup(bytes_be256(c));
                    if list_len(node) == 0 then {
                        k_witness_bad = true;
                        field_to_ref(f)
                    } else {
                        trie_overlay_node_ref_preserve(node, prefix, updates, fuel)
                    }
                } else {
                    trie_empty_ref()
                }
            }
        }
    }
}

function trie_overlay_node_ref_preserve(
    node : list(byte),
    prefix : list(nibble),
    updates : list(TriePatch),
    fuel : int,
) -> (
    list(byte)
) = {
    if fuel <= 0 then {
        k_witness_bad = true;
        trie_empty_ref()
    } else {
        if list_len(node) == 0 then {
            patches_ref(updates, list_len(prefix), fuel)
        } else {
            let items = rlp_node_items(node);
            let n = list_len(items);
            if n == 2 then {
                let (is_leaf, seg) = hp_decode(list_nth(items, 0, RLP_NONE).content);
                let key = list_append(prefix, seg);
                if is_leaf then {
                    patches_ref(
                        leaf_overlay_patches(key, list_nth(items, 1, RLP_NONE).content, updates),
                        list_len(prefix),
                        fuel,
                    )
                } else {
                    if patches_all_under_prefix(updates, key) then {
                        merge_ext_ref(
                            seg,
                            trie_overlay_child_ref_preserve(list_nth(items, 1, RLP_NONE), key, updates, fuel - 1),
                        )
                    } else {
                        let under = patches_under_prefix(updates, key);
                        let outside = patches_outside_prefix(updates, key);
                        let old = trie_overlay_child_ref_preserve(list_nth(items, 1, RLP_NONE), key, under, fuel - 1);
                        patches_ref(patch_insert_ref_if_live(key, old, outside), list_len(prefix), fuel)
                    }
                }
            } else {
                if n == 17 then {
                    let branch_value = list_nth(items, 16, RLP_NONE).content;
                    if list_len(branch_value) > 0 then {
                        k_witness_bad = true
                    };
                    var mask : bits(16) = 0x0000;
                    var children : list(list(byte)) = [||];
                    foreach (i from 0 to 15) {
                        let field = list_nth(items, i, RLP_NONE);
                        let child_prefix = list_append(prefix, [|get_slice_int(4, i, 0)|]);
                        let group = patches_at_nibble(updates, list_len(prefix), get_slice_int(4, i, 0));
                        let cref = trie_overlay_child_ref_preserve(field, child_prefix, group, fuel - 1);
                        if not_bool(trie_ref_is_empty(cref)) then {
                            mask = branch_mask_set_bit(mask, i);
                            children = list_append(children, [|cref|])
                        }
                    };
                    canonical_branch_ref(mask, children)
                } else {
                    trie_empty_ref()
                }
            }
        }
    }
}

function trie_overlay_child_ref(
    f : RlpField,
    prefix : list(nibble),
    updates : list(TriePatch),
    fuel : int,
) -> (
    list(byte)
) = {
    if patches_empty(updates) then {
        field_to_ref(f)
    } else {
        if f.is_list then {
            trie_overlay_node_ref(f.full, prefix, updates, fuel)
        } else {
            let c = f.content;
            if list_len(c) == 0 then {
                patches_ref(updates, list_len(prefix), fuel)
            } else {
                if list_len(c) == 32 then {
                    let node = node_db_lookup(bytes_be256(c));
                    if list_len(node) == 0 then {
                        k_witness_bad = true;
                        field_to_ref(f)
                    } else {
                        trie_overlay_node_ref(node, prefix, updates, fuel)
                    }
                } else {
                    trie_empty_ref()
                }
            }
        }
    }
}

function trie_overlay_child_node_for_collapse(
    f : RlpField,
    prefix : list(nibble),
    updates : list(TriePatch),
    fuel : int,
) -> (
    list(byte)
) = {
    if patches_empty(updates) then {
        if f.is_list then {
            trie_overlay_node_for_collapse(f.full, prefix, updates, fuel)
        } else {
            let c = f.content;
            if list_len(c) == 0 then {
                [||]
            } else {
                if list_len(c) == 32 then {
                    let node = node_db_lookup(bytes_be256(c));
                    if list_len(node) == 0 then {
                        k_witness_bad = true;
                        [||]
                    } else {
                        trie_overlay_node_for_collapse(node, prefix, updates, fuel)
                    }
                } else {
                    [||]
                }
            }
        }
    } else {
        if f.is_list then {
            trie_overlay_node_for_collapse(f.full, prefix, updates, fuel)
        } else {
            let c = f.content;
            if list_len(c) == 0 then {
                patches_node(updates, list_len(prefix), fuel)
            } else {
                if list_len(c) == 32 then {
                    let node = node_db_lookup(bytes_be256(c));
                    if list_len(node) == 0 then {
                        k_witness_bad = true;
                        [||]
                    } else {
                        trie_overlay_node_for_collapse(node, prefix, updates, fuel)
                    }
                } else {
                    [||]
                }
            }
        }
    }
}

function trie_overlay_node_for_collapse(
    node : list(byte),
    prefix : list(nibble),
    updates : list(TriePatch),
    fuel : int,
) -> (
    list(byte)
) = {
    if fuel <= 0 then {
        k_witness_bad = true;
        [||]
    } else {
        if list_len(node) == 0 then {
            patches_node(updates, list_len(prefix), fuel)
        } else {
            let items = rlp_node_items(node);
            let n = list_len(items);
            if n == 2 then {
                let (is_leaf, seg) = hp_decode(list_nth(items, 0, RLP_NONE).content);
                let key = list_append(prefix, seg);
                if is_leaf then {
                    patches_node(
                        leaf_overlay_patches(key, list_nth(items, 1, RLP_NONE).content, updates),
                        list_len(prefix),
                        fuel,
                    )
                } else {
                    if patches_all_under_prefix(updates, key) then {
                        merge_ext_node(
                            seg,
                            trie_overlay_child_node_for_collapse(list_nth(items, 1, RLP_NONE), key, updates, fuel - 1),
                        )
                    } else {
                        let under = patches_under_prefix(updates, key);
                        let outside = patches_outside_prefix(updates, key);
                        if not_bool(patches_empty(under)) & not_bool(patches_has_live(outside)) then {
                            merge_ext_node(
                                seg,
                                trie_overlay_child_node_for_collapse(list_nth(items, 1, RLP_NONE), key, under, fuel - 1),
                            )
                        } else {
                            if patches_empty(under) then {
                                /* untouched witness subtree: a plain hash ref;
                                 * collapse material resolves via the node-db */
                                let old = trie_overlay_child_ref(list_nth(items, 1, RLP_NONE), key, under, fuel - 1);
                                patches_node(patch_insert_ref_if_live(key, old, outside), list_len(prefix), fuel)
                            } else {
                                /* overlaid subtree: its post-state node exists only
                                 * in memory, so carry the node bytes in the patch --
                                 * sibling deletes may collapse into it, and its fresh
                                 * hash can never be found in the witness node-db */
                                let old_node = trie_overlay_child_node_for_collapse(
                                    list_nth(items, 1, RLP_NONE),
                                    key,
                                    under,
                                    fuel - 1,
                                );
                                patches_node(
                                    patch_insert_subtree_if_live(key, old_node, outside),
                                    list_len(prefix),
                                    fuel,
                                )
                            }
                        }
                    }
                }
            } else {
                if n == 17 then {
                    let branch_value = list_nth(items, 16, RLP_NONE).content;
                    if list_len(branch_value) > 0 then {
                        k_witness_bad = true
                    };
                    var mask : bits(16) = 0x0000;
                    var children : list(list(byte)) = [||];
                    var lone_nib : nibble = 0x0;
                    var lone_field : RlpField = RLP_NONE;
                    var lone_prefix : list(nibble) = [||];
                    var lone_updates : list(TriePatch) = [||];
                    foreach (i from 0 to 15) {
                        let field = list_nth(items, i, RLP_NONE);
                        let child_prefix = list_append(prefix, [|get_slice_int(4, i, 0)|]);
                        let group = patches_at_nibble(updates, list_len(prefix), get_slice_int(4, i, 0));
                        let cref = trie_overlay_child_ref(field, child_prefix, group, fuel - 1);
                        if not_bool(trie_ref_is_empty(cref)) then {
                            mask = branch_mask_set_bit(mask, i);
                            children = list_append(children, [|cref|]);
                            lone_nib = get_slice_int(4, i, 0);
                            lone_field = field;
                            lone_prefix = child_prefix;
                            lone_updates = group
                        }
                    };
                    if mask == 0x0000 then {
                        [||]
                    } else {
                        if branch_mask_popcount(mask) == 1 then {
                            let cnode = trie_overlay_child_node_for_collapse(
                                lone_field,
                                lone_prefix,
                                lone_updates,
                                fuel - 1,
                            );
                            merge_ext_node([|lone_nib|], cnode)
                        } else {
                            branch_node_rlp(mask, children)
                        }
                    }
                } else {
                    [||]
                }
            }
        }
    }
}

function trie_overlay_node_ref(
    node : list(byte),
    prefix : list(nibble),
    updates : list(TriePatch),
    fuel : int,
) -> (
    list(byte)
) = {
    node_to_ref(trie_overlay_node_for_collapse(node, prefix, updates, fuel))
}
termination_measure trie_overlay_node_ref(node, prefix, updates, fuel) = fuel
termination_measure trie_overlay_child_ref(f, prefix, updates, fuel) = fuel
termination_measure trie_overlay_node_for_collapse(node, prefix, updates, fuel) = fuel
termination_measure trie_overlay_child_node_for_collapse(f, prefix, updates, fuel) = fuel
termination_measure trie_overlay_node_ref_preserve(node, prefix, updates, fuel) = fuel
termination_measure trie_overlay_child_ref_preserve(f, prefix, updates, fuel) = fuel

/* ---- THE trie entry ------------------------------------------------------
 *
 * trie_root(base_root, updates): the root of the trie anchored at base_root
 * after applying the ordered patch list. This is the ONLY public root
 * computation: witness-native and fail-closed -- every hash reference that
 * must be resolved is looked up in the witness node-db, and any missing
 * node sets k_witness_bad. Deletes take the collapsing walk (which may
 * materialize sibling nodes to restore canonical shape); a delete-free
 * patch list takes the preserve walk (references only, no materialization).
 *
 * Theorem-shaped remark: restricted to an empty base
 * (base_root = EMPTY_TRIE_ROOT), trie_root computes TRIE(I) of Appendix D
 * directly -- an empty base contains no hash refs, so the node-db is never
 * consulted and no failure path can fire. The native (full-state) backend
 * exercises exactly this restriction: same implementation, different input. */
function trie_root(base_root : bits(256), updates : list(TriePatch)) -> bits(256) = {
    if patches_empty(updates) then {
        base_root
    } else {
        if base_root == EMPTY_TRIE_ROOT then {
            trie_ref_to_root(patches_ref(updates, 0, 256))
        } else {
            let node = node_db_lookup(base_root);
            if list_len(node) == 0 then {
                k_witness_bad = true;
                base_root
            } else {
                if patches_has_delete(updates) then {
                    trie_ref_to_root(trie_overlay_node_ref(node, [||], updates, 256))
                } else {
                    trie_ref_to_root(trie_overlay_node_ref_preserve(node, [||], updates, 256))
                }
            }
        }
    }
}

/* ======================================================================== */
/* (4) Ethereum world-state trie                                            */
/*                                                                          */
/* The state root (Yellow Paper section 4.1): the secure-trie root over all */
/* accounts. The trie is 'secure' -- keys are hashed: key =                 */
/* keccak256(address), value = RLP([nonce, balance, storageRoot,            */
/* codeHash]). storageRoot is the root of the account's storage trie, the   */
/* secure trie over keccak256(slot) -> RLP(value) with zero-valued slots    */
/* absent. Accounts that are empty per EIP-161 (zero nonce, zero balance,   */
/* no code) are excluded.                                                   */
/*                                                                          */
/* compute_state_root over the post-execution world equals the block's      */
/* stateRoot exactly when execution is correct -- it authenticates the      */
/* result. The legitimate per-backend difference is update COLLECTION and   */
/* the base anchor (host/base_witness.sail: the write set over the          */
/* authenticated parent root; host/base_native.sail: the full cache/update  */
/* union over the empty trie) -- both feed the single trie_root.            */
/* ======================================================================== */

/* Reused account-RLP payload accumulator. account_rlp is invoked once per account
 * when authenticating the post-state root (compute_state_root); a block-level
 * root or the proving path runs it over the whole account set, so it builds the
 * payload into this single register (O(1) prepend, reversed once) instead of
 * allocating fresh intermediate lists per account. Not reentrant: cleared at the
 * start of every account_rlp call, and account_rlp calls nothing that touches it. */
register rlp_scratch : list(byte) = [||]
function rlp_push(bs : list(byte)) -> unit = {
    var rest : list(byte) = bs;
    var more : bool = true;
    while more do {
        match rest {
            b :: t => {
                rlp_scratch = b :: rlp_scratch;
                rest = t
            },
            [||] => more = false,
        }
    }
}

/* rlp([nonce, balance, storageRoot, codeHash]) */
function account_rlp(nonce : int, balance : word, sroot : bits(256), chash : bits(256)) -> list(byte) = {
    rlp_scratch = [||];
    rlp_push(rlp_int(nonce));
    rlp_push(rlp_int(unsigned(balance)));
    rlp_push(rlp_word(sroot));
    rlp_push(rlp_word(chash));
    rlp_list(list_reverse(rlp_scratch)) /* prepend builds newest-first; reverse -> field order */
}

/* secure-trie updates for one account's post-state storage, selected by
 * keccak(address). Base-layer contract (defined per entry): the witness
 * backend consumes only the write set (untouched slots remain inside the
 * recursive overlay rooted at the account's pre-state storage root); the
 * native backend enumerates the full live cache/update union because it has
 * no authenticated witness root. */
val storage_updates : bits(256) -> list(TriePatch)

/* write-set-only updates (witness base): nonzero writes set RLP(value), zero
 * writes delete the slot; a zero write to a slot with no authenticated
 * pre-state leaf is dropped (nothing to delete). */
function storage_updates_writes_only(ak : bits(256)) -> list(TriePatch) = {
    var out : list(TriePatch) = [||];
    let n = storage_update_count();
    foreach (j from 0 to (n - 1)) {
        let (eak, es, ev) = storage_hashed_update_entry(j);
        if eak == ak then {
            let sk = keccak256_word(es);
            if word_is_zero(ev) & not_bool(storage_map_cache_nonzero(ak, sk)) then {
                ()
            } else {
                let key = to_nibbles(word_to_bytes32(sk));
                out = patch_insert_sorted(
                    if word_is_zero(ev) then {
                        patch_delete(key)
                    } else {
                        patch_leaf(key, rlp_int(unsigned(ev)))
                    },
                    out,
                )
            }
        }
    };
    out
}

/* full live cache/update union (native fixture base) */
function storage_updates_union(ak : bits(256)) -> list(TriePatch) = {
    var out : list(TriePatch) = [||];
    let n = storage_count();
    foreach (j from 0 to (n - 1)) {
        let (eak, es, ev) = storage_hashed_entry(j);
        if eak == ak then {
            let sk = keccak256_word(es);
            let key = to_nibbles(word_to_bytes32(sk));
            out = patch_insert_sorted(
                if word_is_zero(ev) then {
                    patch_delete(key)
                } else {
                    patch_leaf(key, rlp_int(unsigned(ev)))
                },
                out,
            )
        }
    };
    out
}

/* post-state storage root for keccak(address): the single trie entry over the
 * account's pre-state storage root. An empty update list keeps the committed
 * root (including an EIP-8025 omitted storage proof, which is only deficient
 * if the block actually writes that account's storage -- then trie_root's
 * missing-root path fails closed). */
function storage_root_from(ak : bits(256), sroot : bits(256)) -> bits(256) =
    trie_root(sroot, storage_updates(ak))

function account_empty(acc : Account) -> bool =
    (acc.code_hash == KECCAK_EMPTY) & acc.nonce == 0 & word_is_zero(acc.balance)

/* 256-bit account field (balance / code_hash) of the current acctmap_at row */
function selected_account_balance() -> word = acctmap_at_bal()
function selected_account_code_hash() -> word = acctmap_at_chash()
function selected_account_storage_root() -> word = acctmap_at_sroot()

/* secure trie key of the current acctmap_at row (keccak(address)) */
function selected_account_key_hash() -> bits(256) = acctmap_at_hkey()

/* secure-trie updates for the account map. Base-layer contract (defined per
 * entry): stateless block validation consumes only execution updates
 * (witness-materialized read-only accounts are base-state cache, not
 * post-state updates); the native EEST runner has no parent root anchor, so
 * it uses the complete cache/update union to build a full empty-base root. */
val state_updates_map : unit -> list(TriePatch)

/* trie update for the currently-selected acctmap row. skip_empty_unbased
 * drops empty accounts with no authenticated base leaf (witness base: there
 * is nothing to delete); otherwise empty accounts become deletes. */
function state_updates_selected_row(prev : list(TriePatch), skip_empty_unbased : bool) -> list(TriePatch) = {
    let ak_hash = selected_account_key_hash();
    let acc : Account = struct {
            nonce = unsigned(acctmap_at_nonce()),
            balance = selected_account_balance(),
            code_hash = selected_account_code_hash(),
            storage_root = selected_account_storage_root(),
        };
    let ak = to_nibbles(word_to_bytes32(ak_hash));
    let base_exists = acctmap_at_base_exists();
    if skip_empty_unbased & account_empty(acc) & not_bool(base_exists) then {
        prev
    } else {
        let upd : TriePatch = if account_empty(acc) then {
                patch_delete(ak)
            } else {
                patch_leaf(
                    ak,
                    account_rlp(acc.nonce, acc.balance, storage_root_from(ak_hash, acc.storage_root), acc.code_hash),
                )
            };
        patch_insert_sorted(upd, prev)
    }
}

/* execution updates only (witness base) */
function state_updates_map_updates_only() -> list(TriePatch) = {
    var out : list(TriePatch) = [||];
    let n = unsigned(acctmap_update_count());
    foreach (j from 0 to (n - 1)) {
        acctmap_update_at(get_slice_int(64, j, 0));
        out = state_updates_selected_row(out, true)
    };
    out
}

/* complete cache/update union (native fixture base) */
function state_updates_map_union() -> list(TriePatch) = {
    var out : list(TriePatch) = [||];
    let n = unsigned(acctmap_count());
    foreach (j from 0 to (n - 1)) {
        acctmap_at(get_slice_int(64, j, 0));
        out = state_updates_selected_row(out, false)
    };
    out
}

/* the base anchor of the state trie (defined per entry): the authenticated
 * parent state root (witness base) or EMPTY_TRIE_ROOT (native base). */
val state_root_anchor : unit -> bits(256)

/* the post-state world root: the single trie entry over the per-backend
 * anchor and update collection. */
function compute_state_root() -> bits(256) =
    trie_root(state_root_anchor(), state_updates_map())

/* ======================================================================== */
/* (5) stateless witness-backed reads                                       */
/*                                                                          */
/* Reads are LAZY: point reads walk the secure trie from the authenticated  */
/* parent root (k_parent_state_root) by keccak256(key) nibbles (branch /    */
/* extension / leaf, hex-prefix paths) to the account leaf and decode       */
/* RLP([nonce, balance, storageRoot, codeHash]); storage reads walk the     */
/* account's storage root the same way. Because the node database is keyed  */
/* by keccak256(node), every walk is itself a hash-chain proof from its     */
/* root -- no up-front re-root pass is needed to authenticate the witness.  */
/*                                                                          */
/* A node absent from the witness on a walked path is a missing proof and   */
/* fails closed. Depends on the RLP library and the SSZ                     */
/* witness-list readers (host/io.sail).                                     */
/* ======================================================================== */

/* EMPTY_TRIE_ROOT is defined in host/state.sail (next to KECCAK_EMPTY) so the
 * Account struct's storage_root default can reference it. */
let EMPTY_CODE_HASH : bits(256) = 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470

/* decode an account leaf value: (nonce, balance, storage_root, code_hash) */
function decode_account(leaf : list(byte)) -> (int, word, bits(256), bits(256)) = {
    let items = rlp_node_items(leaf);
    let nonce = rlp_field_int(rlp_item(items, 0));
    let balance = rlp_field_uint_word(rlp_item(items, 1));
    let sr_f = rlp_item(items, 2);
    let ch_f = rlp_item(items, 3);
    let sroot = if list_len(sr_f.content) == 0 then {
            EMPTY_TRIE_ROOT
        } else {
            bytes_be256(sr_f.content)
        };
    let chash = if list_len(ch_f.content) == 0 then {
            EMPTY_CODE_HASH
        } else {
            bytes_be256(ch_f.content)
        };
    (nonce, balance, sroot, chash)
}

/* 64 nibbles (MSB-first) of a 256-bit key hash */
function hash_nibbles(h : bits(256)) -> list(nibble) = {
    var rev : list(nibble) = [||];
    foreach (i from 0 to 63) {
        rev = sail_shiftright(h, 4 * (63 - i))[3 .. 0] :: rev
    };
    list_reverse(rev)
}

/* walk the secure trie from `node` following key nibbles; return the leaf value */
function trie_walk(node : list(byte), key : list(nibble), pos : int, fuel : int) -> list(byte) = {
    if fuel <= 0 | list_len(node) == 0 then {
        [||]
    } else {
        let items = rlp_node_items(node);
        let n = list_len(items);
        if n == 2 then {
            let (is_leaf, seg) = hp_decode(list_nth(items, 0, RLP_NONE).content);
            if not_bool(nibs_match(key, pos, seg)) then {
                [||]
            } else {
                if is_leaf then {
                    if pos + list_len(seg) == list_len(key) then {
                        list_nth(items, 1, RLP_NONE).content
                    } else {
                        [||]
                    }
                } else {
                    let child = resolve_child(list_nth(items, 1, RLP_NONE));
                    trie_walk(child, key, pos + list_len(seg), fuel - 1)
                }
            }
        } else {
            if n == 17 then {
                if pos == list_len(key) then {
                    list_nth(items, 16, RLP_NONE).content
                } else {
                    let child = resolve_child(list_nth(items, unsigned(list_nth(key, pos, 0x0)), RLP_NONE));
                    trie_walk(child, key, pos + 1, fuel - 1)
                }
            } else {
                [||]
            }
        }
    }
}
termination_measure trie_walk(node, key, pos, fuel) = fuel

/* ---- account / storage resolution against the witness ------------------ */

/* populate the C node-db from witness.state: keccak(node) -> span. The node is
 * materialized once here to hash it; the recorded span (not the bytes) is what
 * the walk reads back, so no per-node Sail copy is retained. */
function build_node_db() -> unit = {
    nodedb_reset();
    let n = wit_n_state();
    foreach (i from 0 to (n - 1)) {
        let (off, len) = ssz_witness_node_span(i);
        let h = witness_keccak(ssz_witness_node_slice(i));
        nodedb_insert(h, get_slice_int(64, off, 0), get_slice_int(64, len, 0))
    }
}

/* the parent header's state_root (field 3) = pre-state root. The witness headers
 * form a chain header[0] (oldest) .. header[n-1] (newest), where header[i]'s
 * parent is header[i-1] (validated by headers_ok). The block being validated
 * extends the NEWEST, so its parent is header[n-1] -- NOT header[0]. With a
 * single header they coincide; when BLOCKHASH/EIP-2935 pulls in older headers
 * (EIP-8025 witness header collection) the parent is the last one. */
function parent_state_root() -> bits(256) = {
    let items = rlp_node_items(ssz_witness_header(wit_n_headers() - 1));
    bytes_be256(list_nth(items, 3, RLP_NONE).content)
}

/* EIP-8025 (optional proofs): the witness omits an account's storage proof when
 * the block never touches its storage. Such an account has a non-empty committed
 * storage_root whose root node is ABSENT from the witness node-db -- its storage
 * is unchanged, so its post-state storage_root stays the committed value, and
 * any storage read against it is a missing proof. */
function storage_proof_omitted(sroot : bits(256)) -> bool =
    (list_len(node_db_lookup(sroot)) == 0) & (sroot != EMPTY_TRIE_ROOT)

/* Stateless storage resolver (declared in host/state.sail): the (a, s) pre-state
 * value, resolved (and its presence/absence PROVEN) by a lazy Merkle walk of the
 * account's storage trie from its authenticated pre-state root. The node-db is
 * keyed by keccak256(node), so the walk is itself a hash-chain proof from that
 * root:
 *   - the walk reaches the leaf            => present, use that value;
 *   - the walk reaches a diverging/empty
 *     terminus through present nodes        => PROVEN ABSENT => 0;
 *   - the walk hits a blinded (witness-
 *     absent) node on the path              => missing proof => fail closed
 *                                              (k_witness_bad).
 * The callers (k_sload / k_orig, host/base_witness.sail) seed the resolved value
 * into the layered working set, so repeat reads never re-walk. */
function stateless_storage(a : address, s : word) -> word = {
    let sroot = load_account(a).storage_root;
    if storage_proof_omitted(sroot) then {
        /* the block reads storage whose proof the witness omitted */
        k_witness_bad = true;
        sail_zeros(256)
    } else {
        let leaf = trie_walk(node_db_lookup(sroot), hash_nibbles(keccak256_word(s)), 0, 256);
        if list_len(leaf) > 0 then {
            rlp_str_word(leaf)
        } else {
            sail_zeros(256)
        }
    }
}

/* Stateless account materialization (declared in host/state.sail): bind `a` on
 * first touch. Presence/absence is PROVEN by a lazy Merkle walk of the state
 * trie from the authenticated parent root (k_parent_state_root):
 *   - the walk reaches the leaf             => present, decode + seed;
 *   - reaches a diverging/empty terminus
 *     through present nodes                  => PROVEN ABSENT => empty account;
 *   - hits a blinded (witness-absent) node   => missing proof => fail closed
 *                                              (k_witness_bad).
 * So a not-materialized account is never silently treated as empty -- its
 * absence is a witness-proven exclusion. The result is cached raw in the acctmap
 * (k_seed_account_hashed), so subsequent load_account(a) is an O(1) raw read. */
function stateless_account_load(a : address) -> unit = {
    let ak = keccak256_address(a);
    let rootnode = node_db_lookup(k_parent_state_root);
    if (list_len(rootnode) == 0) & (k_parent_state_root != EMPTY_TRIE_ROOT) then {
        /* the anchor root itself is missing from the witness */
        k_witness_bad = true
    };
    let value = trie_walk(rootnode, hash_nibbles(ak), 0, 256);
    if list_len(value) > 0 then {
        let (nonce, bal, sroot, chash) = decode_account(value);
        k_seed_account_hashed(a, nonce, bal, sroot, chash);
        acctmap_mark_base_exists(a)
    } else {
        k_seed_account_hashed(a, 0, sail_zeros(256), EMPTY_TRIE_ROOT, KECCAK_EMPTY)
    }
}

/* EIP-7928 block-access-list item count over the RLP-encoded BlockAccessList (a
 * ByteList in the payload): rlp([ [address, storage_changes, storage_reads,
 * balance_changes, nonce_changes, code_changes], ... ]). Item count =
 * #accounts + sum over accounts of (#storage_changes + #storage_reads), where
 * storage_changes (RLP item 1) is a list of [slot, changes] pairs and
 * storage_reads (item 2) is a list of slots. EELS
 * validate_block_access_list_gas_limit rejects the block if this exceeds
 * block_gas_limit / BLOCK_ACCESS_LIST_ITEM (2000). */
function bal_account_items(acc : list(byte)) -> int = {
    let items = rlp_node_items(acc);
    let sc = list_nth(items, 1, RLP_NONE);
    let sr = list_nth(items, 2, RLP_NONE);
    1 + list_len(rlp_node_items(sc.full)) + list_len(rlp_node_items(sr.full))
}

function bal_item_count() -> int = {
    let b = pl_bal_start();
    let e = payload_end();
    if b >= e then {
        0
    } else {
        var total : int = 0;
        var rest : list(RlpField) = rlp_node_items(ssz_bytes(b, e - b));
        var more : bool = true;
        while more do {
            match rest {
                a :: r => {
                    total = total + bal_account_items(a.full);
                    rest = r
                },
                [||] => more = false,
            }
        };
        total
    }
}
