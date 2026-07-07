/* ======================================================================== */
/* MERKLE-PATRICIA TRIE AND WORLD-STATE ROOTS: ONE IMPLEMENTATION           */
/*                                                                          */
/* Single-implementation contract: every trie root in the system is         */
/* computed by ONE witness-native implementation with ONE public entry,     */
/* trie_root(base_root, updates). It operates over comprehensive witnesses: */
/* every hash reference that must be resolved is looked up in the witness   */
/* node database, and any missing node material aborts the computation      */
/* (k_witness_bad, fail closed). There is no separate from-scratch          */
/* ("yellow paper") trie build; the Yellow Paper survives as documentation  */
/* -- Appendix C.1 (hex-prefix) and Appendix D (Eq. 206 TRIE / Eq. 207 n /  */
/* Eq. 208 c) are cited on the corresponding internal functions, and        */
/* restricting trie_root to an empty base computes TRIE(I) directly (see    */
/* the remark at trie_root).                                                */
/*                                                                          */
/* Layers:                                                                  */
/* (1) trie primitives: nibbles, hex-prefix paths, node RLP composition,    */
/*     child references with the < 32-byte inline cap.                      */
/* (2) the update list: ONE ordered external update type (leaf writes and   */
/*     leaf deletes over the composed map).                                 */
/* (3) the item stream: the witness walker merges the pre-state trie with   */
/*     the update list along touched paths only and emits the post-state    */
/*     map in key order (leaves + untouched-subtree refs; deletes emit      */
/*     nothing); the trie builder recomposes the stream by recursive        */
/*     lookahead, canonical by construction. trie_root is the single entry. */
/* (4) world state: account/storage update collection and the state root    */
/*     (keyed by keccak256(address) / keccak256(slot), value                */
/*     RLP([nonce, balance, storageRoot, codeHash])).                       */
/* (5) lazy witness reads: authenticated Merkle walks resolving accounts    */
/*     and storage slots from the anchored parent root.                     */
/*                                                                          */
/* Design decision (final): post-state roots are computed in BATCH -- one   */
/* walk + one build over the whole ordered update list -- NOT by per-op     */
/* insert/delete maintenance. The Yellow Paper defines the trie             */
/* declaratively over the final map (Appendix D; S D.1 leaves maintenance   */
/* strategy to implementations); the batch passes touch and hash each       */
/* node once, and a per-op delete would face the same blinded-sibling       */
/* collapse problem while multiplying hashing. Do not migrate this to       */
/* incremental per-operation insert/delete.                                 */
/* ======================================================================== */

/* ======================================================================== */
/* (1) trie primitives                                                      */
/* ======================================================================== */

/* A nibble is half a byte: the unit of trie path traversal (YP Appendix D:
 * "Any series of bytes may also trivially be viewed as a series of nibbles"). */
type nibble = bits(4)

/* 32-byte key -> 64 nibbles, high nibble first (single pass + reverse). */
function to_nibbles(bs : list(byte)) -> list(nibble) = {
    var out : list(nibble) = [||];
    var rest : list(byte) = bs;
    var more : bool = true;
    while more do {
        match rest {
            b :: r => {
                out = b[7 .. 4] :: out;
                out = b[3 .. 0] :: out;
                rest = r
            },
            [||] => more = false,
        }
    };
    list_reverse(out)
}

/* hex-prefix (compact) encoding of a nibble path + leaf/extension flag
 * (Yellow Paper Appendix C.1, the HP function). */
function hex_prefix_compact(nibs : list(nibble), is_leaf : bool) -> list(byte) = {
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
    var more : bool = true;
    while more do {
        match rest {
            a :: (b :: t) => {
                out = append(a, b) :: out;
                rest = t
            },
            _ => more = false,
        }
    };
    list_reverse(out)
}

/* hex-prefix decode (Appendix C.1 inverted): (is_leaf, path nibbles) */
function hex_prefix_decode(p : list(byte)) -> (bool, list(nibble)) = {
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
/* leaf:      child_ref( rlp[ hex_prefix_compact(key,leaf) , value ] )         */
/* extension: child_ref( rlp[ hex_prefix_compact(key,ext)  , child-ref ] )     */
/* branch:    child_ref( rlp[ 16 child-refs-or-0x80 , 0x80 ] )                 */
/* word:      0xa0 ++ hash   (a blinded subtree / >=32B child, already a ref)  */
function leaf_node_rlp(key : list(nibble), value : list(byte)) -> list(byte) =
    rlp_list(rlp_bytes(hex_prefix_compact(key, true)) ++ rlp_bytes(value))
function extension_node_rlp(key : list(nibble), childref : list(byte)) -> list(byte) =
    rlp_list(rlp_bytes(hex_prefix_compact(key, false)) ++ childref)
function leaf_child_ref(key : list(nibble), value : list(byte)) -> list(byte) =
    child_ref(leaf_node_rlp(key, value))
function extension_child_ref(key : list(nibble), childref : list(byte)) -> list(byte) =
    child_ref(extension_node_rlp(key, childref))
function hash_child_ref(hash : list(byte)) -> list(byte) = [|0xa0|] ++ hash

function branch_payload_rlp(mask : bits(16), children : list(list(byte))) -> list(byte) = {
    var payload : list(byte) = [||];
    var ch : list(list(byte)) = children; /* ascending nibble order */
    foreach (i from 0 to 15) {
        if branch_mask_has_bit(mask, i) then {
            match ch {
                c :: t => {
                    payload = payload ++ c;
                    ch = t
                },
                [||] => (),
            }
        } else {
            payload = payload ++ [|0x80|]
        }
    };
    payload ++ [|0x80|] /* branch value slot: empty */
}
function branch_node_rlp(mask : bits(16), children : list(list(byte))) -> list(byte) =
    rlp_list(branch_payload_rlp(mask, children))

/* branch child ref (Eq. 208 branch form): a refinement pair. The spec backend
 * composes and hashes the 17-item node as a byte list (spec/mpt.sail); the
 * build backend assembles it in C from fixed-width child refs so the node byte
 * list never crosses the FFI hash channel (build/mpt.sail). */
val branch_child_ref : (bits(16), list(list(byte))) -> list(byte)

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
/* (2) the update list                                                      */
/*                                                                          */
/* THE single external update type: an ordered (lexicographic nibble-key)   */
/* list of leaf writes and leaf deletes over the composed map. Deletes      */
/* erase base-trie leaves; a delete of a key with no base leaf is a no-op.  */
/* Subtree references never appear here -- they exist only in the internal  */
/* item stream (TrieItem, below) that the witness walker emits.             */
/* ======================================================================== */
struct TriePatch = {
    key : list(nibble),
    is_delete : bool,
    bytes : list(byte),
}

function patch_leaf(key : list(nibble), value : list(byte)) -> TriePatch =
    struct { key = key, is_delete = false, bytes = value }

function patch_delete(key : list(nibble)) -> TriePatch =
    struct { key = key, is_delete = true, bytes = [||] }

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
        _ => false,
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
/* (3) the item stream: witness walker -> trie builder                      */
/*                                                                          */
/* Root computation is two single-policy passes over ONE sorted stream      */
/* (the shape of alloy-trie's HashBuilder / TrieWalker split, in recursive  */
/* form):                                                                   */
/*                                                                          */
/*   witness_items  walks the witnessed pre-state trie along touched paths  */
/*                  only, merging in the update list, and emits the         */
/*                  post-state map in key order: live leaves carry full     */
/*                  keys, untouched subtrees pass through as one ref item   */
/*                  with their full path prefix. Deletes emit nothing --    */
/*                  no delete or collapse logic exists anywhere else.       */
/*   build_group    composes the stream back into a trie by recursive       */
/*                  lookahead: a node closes exactly where the sorted keys  */
/*                  diverge, so canonical shape (extension merging, branch  */
/*                  only when >= 2 children survive) holds by construction  */
/*                  -- there is no separate collapse pass.                  */
/*                                                                          */
/* Stateless verification mirrors execution-specs witness_state.py: the     */
/* node database is keyed by keccak256(node) (C-backed,                     */
/* ffi/trie_node_db.c) and a node absent from the witness on a needed path  */
/* is a missing proof that fails closed (k_witness_bad).                    */
/* ======================================================================== */

/* node bytes for keccak `h` from the C node-db, or [] if absent (read straight
 * from the witness buffer by the recorded span -- no per-node Sail copy held). */
function node_db_lookup(h : bits(256)) -> list(byte) = {
    let len = unsigned(nodedb_len(h));
    if len == 0 then {
        [||]
    } else {
        ssz_bytes(unsigned(nodedb_off(h)), len)
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

/* re-key a decoded child node under `prefix` nibbles: a leaf/extension child
 * absorbs the prefix into its own path (Yellow Paper canonical form); any
 * other node is referenced through an extension. */
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
                let (is_leaf, seg) = hex_prefix_decode(list_nth(items, 0, RLP_NONE).content);
                let key = prefix ++ seg;
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

/* merge_ext_node over a child REF: an inline ref carries its node bytes and
 * re-keys canonically; a 32-byte hash ref is wrapped in an extension, which
 * is canonical ONLY when the referenced node is a branch. */
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
                        let (is_leaf, seg) = hex_prefix_decode(list_nth(items, 0, RLP_NONE).content);
                        let key = prefix ++ seg;
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

/* ---- the item stream ----------------------------------------------------
 *
 * A TrieItem is one entry of the sorted post-state stream:
 *   - a live leaf (is_ref = false): path = full key, bytes = RLP value;
 *   - an untouched subtree (is_ref = true): path = the subtree's full path
 *     prefix, bytes = its child ref (inline node or 0xa0 ++ hash).
 * `is_branch` records subtree provenance: a ref taken from an extension's
 * child position is guaranteed a branch node (MPT invariant), so the
 * builder may wrap an extension around its hash without node material; a
 * ref taken from a branch child slot has unknown node type, and absorbing
 * extra path into it (delete collapse) must resolve the node from the
 * witness db -- EIP-8025 guarantees it is present exactly when needed. */
struct TrieItem = {
    path : list(nibble),
    is_ref : bool,
    is_branch : bool,
    bytes : list(byte),
}

function item_leaf(path : list(nibble), value : list(byte)) -> TrieItem =
    struct { path = path, is_ref = false, is_branch = false, bytes = value }

function item_subtree(path : list(nibble), childref : list(byte), is_branch : bool) -> TrieItem =
    struct { path = path, is_ref = true, is_branch = is_branch, bytes = childref }

/* sorted live-leaf items of an update list (deletes emit nothing) */
function patches_live_items(ps : list(TriePatch)) -> list(TrieItem) =
    match ps {
        [||] => [||],
        p :: r => if p.is_delete then {
            patches_live_items(r)
        } else {
            item_leaf(p.key, p.bytes) :: patches_live_items(r)
        },
    }

/* the post-state items at a base leaf, in one pass: the live update leaves
 * with the base leaf inserted at its sorted position; an update on the
 * leaf's own key replaces (write) or erases (delete) it, and diverging
 * updates are inserts */
function leaf_overlay_items(ps : list(TriePatch), key : list(nibble), value : list(byte)) -> list(TrieItem) =
    match ps {
        [||] => [|item_leaf(key, value)|],
        p :: r => if nib_eq(p.key, key) then {
            patches_live_items(ps)
        } else {
            if nib_lt(key, p.key) then {
                item_leaf(key, value) :: patches_live_items(ps)
            } else {
                if p.is_delete then {
                    leaf_overlay_items(r, key, value)
                } else {
                    item_leaf(p.key, p.bytes) :: leaf_overlay_items(r, key, value)
                }
            }
        },
    }

/* merge two sorted item streams by path */
function items_merge(a : list(TrieItem), b : list(TrieItem)) -> list(TrieItem) =
    match (a, b) {
        ([||], _) => b,
        (_, [||]) => a,
        (x :: ta, y :: tb) => if nib_lt(y.path, x.path) then {
            y :: items_merge(a, tb)
        } else {
            x :: items_merge(ta, b)
        },
    }
termination_measure items_merge(a, b) = list_len(a) + list_len(b)

function items_last_path(items : list(TrieItem)) -> list(nibble) =
    match items {
        [||] => [||],
        it :: r => match r {
            [||] => it.path,
            _ => items_last_path(r),
        },
    }

function items_at_nibble(items : list(TrieItem), depth : int, nib : nibble) -> list(TrieItem) =
    match items {
        [||] => [||],
        it :: r => if list_len(it.path) > depth & list_nth(it.path, depth, 0x0) == nib then {
            it :: items_at_nibble(r, depth, nib)
        } else {
            items_at_nibble(r, depth, nib)
        },
    }

/* the child ref of a single-item subtree at `depth`: the item's remaining
 * path is absorbed into it. This is the ONE place a delete collapse can
 * demand node material: an unknown-type hash ref absorbing a nonempty
 * suffix resolves its node from the witness db (fail-closed). */
function item_ref(it : TrieItem, depth : int) -> list(byte) = {
    let suffix = list_drop_prefix(it.path, depth);
    if not_bool(it.is_ref) then {
        leaf_child_ref(suffix, it.bytes)
    } else {
        if list_len(suffix) == 0 then {
            it.bytes
        } else {
            if it.is_branch then {
                merge_ext_ref(suffix, it.bytes)
            } else {
                match it.bytes {
                    0xa0 :: h => if list_len(h) == 32 then {
                        let node = node_db_lookup(bytes_be256(h));
                        if list_len(node) == 0 then {
                            k_witness_bad = true;
                            extension_child_ref(suffix, it.bytes)
                        } else {
                            node_to_ref(merge_ext_node(suffix, node))
                        }
                    } else {
                        merge_ext_ref(suffix, it.bytes)
                    },
                    _ => merge_ext_ref(suffix, it.bytes),
                }
            }
        }
    }
}

/* ---- the trie builder (Yellow Paper Appendix D) --------------------------
 *
 * build_group composes the subtree of a sorted, prefix-free item group
 * whose paths all extend the group's first `depth` nibbles. On a leaf-only
 * stream this IS the protocol trie definition, transcribed from the Yellow
 * Paper (Shanghai version) and specialized to fixed-length secure-trie
 * keys (so a branch node's value slot -- the 17th item -- is always
 * empty):
 *
 *   c(I,i): leaf | extension | 17-item branch      (Eq. 208)  build_group
 *   n(I,i): () if I = {}; c(I,i) if ||RLP|| < 32;
 *           KEC(RLP(c(I,i))) otherwise             (Eq. 207)  child_ref cap
 *   TRIE(I) = KEC(RLP(c(I,0)))                     (Eq. 206)  items_root
 *
 * Node closure is by lookahead on the sorted keys: an extension covers
 * exactly the prefix shared by the whole group (first/last suffices when
 * sorted), and a branch materializes only where >= 2 key groups actually
 * diverge -- the canonical form is reached by construction, with no
 * separate collapse or canonicalization step. `fuel` only bounds recursion
 * for the termination checker: keys are 64 nibbles, callers pass 256. */
function build_group(items : list(TrieItem), depth : int, fuel : int) -> list(byte) = {
    if fuel <= 0 then {
        k_witness_bad = true;
        trie_empty_ref()
    } else {
        match items {
            [||] => trie_empty_ref(),
            it :: [||] => item_ref(it, depth),
            it :: _ => {
                let last = items_last_path(items);
                let common = common_prefix_len(list_drop_prefix(it.path, depth), list_drop_prefix(last, depth));
                if common > 0 then {
                    /* >= 2 items share `shared`, so the child one level down
                       is necessarily a branch: a plain extension wrap is
                       canonical */
                    let shared = list_take_prefix(list_drop_prefix(it.path, depth), common);
                    extension_child_ref(shared, build_group(items, depth + common, fuel - common))
                } else {
                    /* >= 2 items diverge right here: a branch with >= 2
                       children by construction (groups of a nonempty sorted
                       stream are nonempty and never build to empty) */
                    var mask : bits(16) = 0x0000;
                    var children : list(list(byte)) = [||];
                    foreach (i from 0 to 15) {
                        let group = items_at_nibble(items, depth, get_slice_int(4, i, 0));
                        if list_len(group) > 0 then {
                            mask = branch_mask_set_bit(mask, i);
                            children = children ++ [|build_group(group, depth + 1, fuel - 1)|]
                        }
                    };
                    branch_child_ref(mask, children)
                }
            },
        }
    }
}
termination_measure build_group(items, depth, fuel) = fuel

/* TRIE of a sorted item stream (Eq. 206 on a leaf-only stream) */
function items_root(items : list(TrieItem)) -> bits(256) =
    match items {
        [||] => EMPTY_TRIE_ROOT,
        _ => trie_ref_to_root(build_group(items, 0, 256)),
    }

/* ---- the witness walker --------------------------------------------------
 *
 * witness_items(node, prefix, updates, fuel): the sorted post-state item
 * stream of the subtree rooted at `node` (whose position is `prefix`) after
 * applying `updates` (all of whose keys extend `prefix`; nonempty).
 *
 * The walker descends ONLY along touched paths; untouched children pass
 * through as single ref items with zero node-db work. Deletes are consumed
 * here and only here: a delete suppresses its base leaf, and a delete with
 * no base leaf (the walk proves absence) emits nothing. Whatever survives
 * is rebuilt by build_group, so a branch losing children to deletes
 * collapses in the builder with no walker involvement. */
val witness_items : (list(byte), list(nibble), list(TriePatch), int) -> list(TrieItem)
val witness_child_items : (RlpField, list(nibble), list(TriePatch), int) -> list(TrieItem)

function witness_child_items(
    f : RlpField,
    prefix : list(nibble),
    updates : list(TriePatch),
    fuel : int,
) -> (
    list(TrieItem)
) = {
    if f.is_list then {
        witness_items(f.full, prefix, updates, fuel)
    } else {
        let c = f.content;
        if list_len(c) == 0 then {
            /* proven-absent subtree: the surviving updates are pure inserts */
            patches_live_items(updates)
        } else {
            if list_len(c) == 32 then {
                let node = node_db_lookup(bytes_be256(c));
                if list_len(node) == 0 then {
                    /* missing proof on a touched path: fail closed; keep the
                       blinded ref so the (already invalid) stream stays a trie */
                    k_witness_bad = true;
                    [|item_subtree(prefix, field_to_ref(f), false)|]
                } else {
                    witness_items(node, prefix, updates, fuel)
                }
            } else {
                [||]
            }
        }
    }
}

function witness_items(
    node : list(byte),
    prefix : list(nibble),
    updates : list(TriePatch),
    fuel : int,
) -> (
    list(TrieItem)
) = {
    if fuel <= 0 then {
        k_witness_bad = true;
        [||]
    } else {
        if list_len(node) == 0 then {
            patches_live_items(updates)
        } else {
            let items = rlp_node_items(node);
            let n = list_len(items);
            if n == 2 then {
                let (is_leaf, seg) = hex_prefix_decode(list_nth(items, 0, RLP_NONE).content);
                let key = prefix ++ seg;
                if is_leaf then {
                    leaf_overlay_items(updates, key, list_nth(items, 1, RLP_NONE).content)
                } else {
                    let under = patches_under_prefix(updates, key);
                    let outside = patches_outside_prefix(updates, key);
                    let sub : list(TrieItem) = if patches_empty(under) then {
                            /* untouched extension child: one whole-subtree
                               ref; an extension's child is always a branch */
                            [|item_subtree(key, field_to_ref(list_nth(items, 1, RLP_NONE)), true)|]
                        } else {
                            witness_child_items(list_nth(items, 1, RLP_NONE), key, under, fuel - 1)
                        };

                    /* updates diverging inside the extension segment become
                       siblings; the builder reshapes the segment around them */
                    items_merge(patches_live_items(outside), sub)
                }
            } else {
                if n == 17 then {
                    let branch_value = list_nth(items, 16, RLP_NONE).content;
                    if list_len(branch_value) > 0 then {
                        k_witness_bad = true
                    };
                    var out : list(TrieItem) = [||];
                    foreach (i from 0 to 15) {
                        let nib : nibble = get_slice_int(4, i, 0);
                        let field = list_nth(items, i, RLP_NONE);
                        let group = patches_at_nibble(updates, list_len(prefix), nib);

                        /* a child slot holds an embedded node, a 32-byte hash,
                           or nothing (field_to_ref of anything else is empty) */
                        let present = field.is_list | list_len(field.content) == 32;
                        let contrib : list(TrieItem) = if patches_empty(group) then {
                                if present then {
                                    /* untouched branch child: node type unknown
                                       for hash refs (is_branch = false) */
                                    [|item_subtree(prefix ++ [|nib|], field_to_ref(field), false)|]
                                } else {
                                    [||]
                                }
                            } else {
                                if present then {
                                    witness_child_items(field, prefix ++ [|nib|], group, fuel - 1)
                                } else {
                                    /* no base child: the surviving updates are
                                       pure inserts */
                                    patches_live_items(group)
                                }
                            };
                        out = out ++ contrib
                    };
                    out
                } else {
                    [||]
                }
            }
        }
    }
}
termination_measure witness_items(node, prefix, updates, fuel) = fuel
termination_measure witness_child_items(f, prefix, updates, fuel) = fuel

/* ---- THE trie entry ------------------------------------------------------
 *
 * trie_root(base_root, updates): the root of the trie anchored at base_root
 * after applying the ordered update list. This is the ONLY public root
 * computation: witness-native and fail-closed -- the walker resolves every
 * touched hash reference in the witness node-db and any missing node sets
 * k_witness_bad; the builder recomposes the emitted stream canonically.
 *
 * Theorem-shaped remark: restricted to an empty base
 * (base_root = EMPTY_TRIE_ROOT), the walker is the identity on the live
 * update leaves and trie_root computes TRIE(I) of Appendix D directly -- an
 * empty base contains no hash refs, so the node-db is never consulted and
 * no failure path can fire. The native (full-state) backend exercises
 * exactly this restriction: same implementation, different input. */
function trie_root(base_root : bits(256), updates : list(TriePatch)) -> bits(256) = {
    if patches_empty(updates) then {
        base_root
    } else {
        if base_root == EMPTY_TRIE_ROOT then {
            items_root(patches_live_items(updates))
        } else {
            let node = node_db_lookup(base_root);
            if list_len(node) == 0 then {
                k_witness_bad = true;
                base_root
            } else {
                items_root(witness_items(node, [||], updates, 256))
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
    let n = storage_acct_update_rows(ak);
    foreach (j from 0 to (n - 1)) {
        let (es, ev) = storage_acct_update_row(ak, j);
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
    };
    out
}

/* full live cache/update union (native fixture base) */
function storage_updates_union(ak : bits(256)) -> list(TriePatch) = {
    var out : list(TriePatch) = [||];
    let n = storage_acct_rows(ak);
    foreach (j from 0 to (n - 1)) {
        let (es, ev) = storage_acct_row(ak, j);
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

/* secure-trie updates for the account map. Base-layer contract (defined per
 * entry): stateless block validation consumes only execution updates
 * (witness-materialized read-only accounts are base-state cache, not
 * post-state updates); the native EEST runner has no parent root anchor, so
 * it uses the complete cache/update union to build a full empty-base root. */
val state_updates_map : unit -> list(TriePatch)

/* trie update for one enumerated acctmap row (AcctRow, host/state.sail).
 * skip_empty_unbased drops empty accounts with no authenticated base leaf
 * (witness base: there is nothing to delete); otherwise empty accounts
 * become deletes. */
function state_updates_account_row(row : AcctRow, prev : list(TriePatch), skip_empty_unbased : bool) -> list(TriePatch) = {
    let acc : Account = struct {
            nonce = unsigned(row.row_nonce),
            balance = row.row_bal,
            code_hash = row.row_chash,
            storage_root = row.row_sroot,
        };
    let ak = to_nibbles(word_to_bytes32(row.row_hkey));
    if skip_empty_unbased & account_empty(acc) & not_bool(row.row_base_exists) then {
        prev
    } else {
        let upd : TriePatch = if account_empty(acc) then {
                patch_delete(ak)
            } else {
                patch_leaf(
                    ak,
                    account_rlp(
                        acc.nonce,
                        acc.balance,
                        storage_root_from(row.row_hkey, acc.storage_root),
                        acc.code_hash,
                    ),
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
        out = state_updates_account_row(acctmap_update_row(j), out, true)
    };
    out
}

/* complete cache/update union (native fixture base) */
function state_updates_map_union() -> list(TriePatch) = {
    var out : list(TriePatch) = [||];
    let n = unsigned(acctmap_count());
    foreach (j from 0 to (n - 1)) {
        out = state_updates_account_row(acctmap_row(j), out, false)
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
            let (is_leaf, seg) = hex_prefix_decode(list_nth(items, 0, RLP_NONE).content);
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
