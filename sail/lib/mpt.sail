/* Witness-native Ethereum MPT. `trie_root` merges ordered updates into an
 * authenticated base trie and fails closed when a touched hash is absent.
 * With an empty base the same builder computes Yellow Paper TRIE(I). */

/* keccak(node) -> span in the SSZ witness buffer. */
val nodedb_reset = impure { c: "nodedb_reset" } : unit -> unit
val nodedb_insert = impure { c: "nodedb_insert" } : (hash, bits(64), bits(64)) -> unit
val nodedb_off = impure { c: "nodedb_off" } : hash -> bits(64)
val nodedb_len = impure { c: "nodedb_len" } : hash -> bits(64)

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
            if n == 0 then { ZERO_WORD } else { sail_shiftleft(bytes_be256(bs), 256 - 8 * n) },
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
    if a.data == b.data then { path_len(a) < path_len(b) } else { word_ult(a.data, b.data) }

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

union NodeRef = {
    EmptyRef : unit,
    InlineRef : Bytes,
    HashRef : hash,
}

function node_ref_rlp(r : NodeRef) -> list(byte) =
    match r {
        EmptyRef() => [|0x80|],
        InlineRef(node) => bytes_to_list(node),
        HashRef(h) => [|0xa0|] ++ word_to_bytes32(h),
    }

/* Appendix D, Eq. 207: inline nodes under 32 bytes, otherwise hash them. */
function child_ref(enc : list(byte)) -> NodeRef =
    if list_len(enc) < 32 then {
        InlineRef(BytesList(enc))
    } else {
        HashRef(keccak256(enc))
    }

/* Bit i marks the child at nibble i. */
function branch_mask_has_bit(mask : bits(16), i : int) -> bool =
    (sail_shiftright(mask, i) & 0x0001) == 0x0001
function branch_mask_set_bit(mask : bits(16), i : int) -> bits(16) =
    mask | sail_shiftleft(0x0001, i)

/* Yellow Paper Appendix D, Eq. 208 node forms. */
function leaf_node_rlp(key : TriePath, value : list(byte)) -> list(byte) =
    rlp_list(rlp_bytes(hex_prefix_compact(key, true)) ++ rlp_bytes(value))
function extension_node_rlp(key : TriePath, childref : NodeRef) -> list(byte) =
    rlp_list(rlp_bytes(hex_prefix_compact(key, false)) ++ node_ref_rlp(childref))

/* Keep leaf values in their native representation. Long nodes hash the RLP
 * framing and value as segments; only an inline node materializes a slice. */
function leaf_child_ref(key : TriePath, value : Bytes) -> NodeRef = {
    let path = rlp_bytes(hex_prefix_compact(key, true));
    let value_len = bytes_len(value);
    let value_prefix = rlp_string_prefix(value_len, bytes_first(value));
    let payload_len = list_len(path) + list_len(value_prefix) + value_len;
    let head = rlp_list_prefix(payload_len) ++ path ++ value_prefix;
    if list_len(head) + value_len < 32 then {
        InlineRef(BytesList(head ++ bytes_to_list(value)))
    } else {
        HashRef(keccak256([|BytesList(head), value|]))
    }
}

function extension_child_ref(key : TriePath, childref : NodeRef) -> NodeRef =
    child_ref(extension_node_rlp(key, childref))

function branch_payload_rlp(mask : bits(16), children : list(NodeRef)) -> list(byte) = {
    var payload : list(byte) = [||];
    var ch : list(NodeRef) = children; /* ascending nibble order */
    foreach (i from 0 to 15) {
        if branch_mask_has_bit(mask, i) then {
            match ch {
                c :: t => {
                    payload = payload ++ node_ref_rlp(c);
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
function branch_child_ref(mask : bits(16), children : list(NodeRef)) -> NodeRef =
    child_ref(rlp_list(branch_payload_rlp(mask, children)))

function trie_ref_to_root(r : NodeRef) -> bits(256) =
    match r {
        EmptyRef() => EMPTY_TRIE_ROOT,
        InlineRef(node) => keccak256([|node|]),
        HashRef(h) => h,
    }

function node_to_ref(node : Bytes) -> NodeRef =
    if bytes_len(node) == 0 then {
        EmptyRef()
    } else {
        if bytes_len(node) < 32 then {
            InlineRef(node)
        } else {
            HashRef(keccak256([|node|]))
        }
    }

/* Ordered leaf updates. */
union TrieChange = {
    TriePut : Bytes,
    TrieDelete : unit,
}

struct TriePatch = { key : TriePath, change : TrieChange }

function patch_leaf(key : TriePath, value : Bytes) -> TriePatch =
    struct { key = key, change = TriePut(value) }

function patch_delete(key : TriePath) -> TriePatch =
    struct { key = key, change = TrieDelete() }

function patch_insert_sorted(p : TriePatch, ps : list(TriePatch)) -> list(TriePatch) =
    match ps {
        [||] => [|p|],
        q :: r => if path_eq(p.key, q.key) then {
            p :: r
        } else {
            if path_lt(p.key, q.key) then {
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
        p :: r => if path_len(p.key) > depth & path_nibble(p.key, depth) == nib then {
            p :: patches_at_nibble(r, depth, nib)
        } else {
            patches_at_nibble(r, depth, nib)
        },
    }

function patches_partition_prefix(
    ps : list(TriePatch),
    prefix : TriePath,
) -> (list(TriePatch), list(TriePatch)) =
    match ps {
        [||] => ([||], [||]),
        p :: r => if path_prefix_of(prefix, p.key) then {
            let (inside, outside) = patches_partition_prefix(r, prefix);
            (p :: inside, outside)
        } else {
            let (inside, outside) = patches_partition_prefix(r, prefix);
            (inside, p :: outside)
        },
    }

/* The walker merges patches into an ordered item stream; the builder turns
 * that stream back into canonical trie nodes. Untouched subtrees stay refs. */

/* Node bytes for keccak `h`, retained as a witness slice. */
function node_db_lookup(h : bits(256)) -> Bytes = {
    let len = unsigned(nodedb_len(h));
    if len == 0 then {
        BytesList([||])
    } else {
        BytesSlice(witness_byte_slice(unsigned(nodedb_off(h)), len))
    }
}

struct LeafNodeData = { path : TriePath, value : RlpFieldRef }
struct ExtensionNodeData = { path : TriePath, child : RlpFieldRef }
struct BranchNodeData = { fields : list(RlpFieldRef) }

union TrieNode = {
    LeafNode : LeafNodeData,
    ExtensionNode : ExtensionNodeData,
    BranchNode : BranchNodeData,
    InvalidNode : unit,
}

function decode_trie_node(node : Bytes) -> TrieNode = {
    let fields = rlp_node_refs(node);
    match list_len(fields) {
        2 => {
            let (is_leaf, path) = hex_prefix_decode_ref(list_nth(fields, 0, RLP_REF_NONE));
            if is_leaf then {
                LeafNode(struct { path = path, value = list_nth(fields, 1, RLP_REF_NONE) })
            } else {
                ExtensionNode(struct { path = path, child = list_nth(fields, 1, RLP_REF_NONE) })
            }
        },
        17 => BranchNode(struct { fields = fields }),
        _ => InvalidNode(),
    }
}

function field_to_ref(f : RlpFieldRef) -> NodeRef =
    if f.is_list then {
        InlineRef(rlp_ref_full(f))
    } else {
        if unsigned(f.content_len) == 32 then {
            HashRef(rlp_ref_word(f))
        } else {
            EmptyRef()
        }
    }

/* Resolving a missing hash is a deficient witness, never an empty subtree. */
function resolve_ref(r : NodeRef) -> Bytes =
    match r {
        EmptyRef() => BytesList([||]),
        InlineRef(node) => node,
        HashRef(h) => {
            let node = node_db_lookup(h);
            if bytes_len(node) == 0 then {
                throw InvalidBlock(WitnessDeficient)
            } else {
                node
            }
        },
    }

/* Re-key a decoded child under `prefix` without copying a leaf value. */
function merge_ext_node(prefix : TriePath, childnode : Bytes) -> NodeRef = {
    if path_len(prefix) == 0 then {
        node_to_ref(childnode)
    } else {
        if bytes_len(childnode) == 0 then {
            EmptyRef()
        } else {
            match decode_trie_node(childnode) {
                LeafNode(leaf) => leaf_child_ref(
                    path_concat(prefix, leaf.path),
                    rlp_ref_content(leaf.value),
                ),
                ExtensionNode(ext) => extension_child_ref(
                    path_concat(prefix, ext.path),
                    field_to_ref(ext.child),
                ),
                _ => extension_child_ref(prefix, node_to_ref(childnode)),
            }
        }
    }
}

/* merge_ext_node over a child REF: an inline ref carries its node bytes and
 * re-keys canonically; a 32-byte hash ref is wrapped in an extension, which
 * is canonical ONLY when the referenced node is a branch. */
function merge_ext_ref(prefix : TriePath, childref : NodeRef) -> NodeRef = {
    if path_len(prefix) == 0 then {
        childref
    } else {
        match childref {
            EmptyRef() => EmptyRef(),
            HashRef(_) => extension_child_ref(prefix, childref),
            InlineRef(node) => {
                    match decode_trie_node(node) {
                        LeafNode(leaf) => leaf_child_ref(
                            path_concat(prefix, leaf.path),
                            rlp_ref_content(leaf.value),
                        ),
                        ExtensionNode(ext) => extension_child_ref(
                            path_concat(prefix, ext.path),
                            field_to_ref(ext.child),
                        ),
                        _ => extension_child_ref(prefix, childref),
                    }
                },
        }
    }
}

/* Extension children are known branches; branch children have unknown kind
 * until resolved. That distinction permits untouched hashes to stay opaque. */
union TrieItemValue = {
    LeafItem : Bytes,
    BranchItem : NodeRef,
    SubtreeItem : NodeRef,
}

struct TrieItem = { path : TriePath, value : TrieItemValue }

function item_leaf(path : TriePath, value : Bytes) -> TrieItem =
    struct { path = path, value = LeafItem(value) }

function item_branch(path : TriePath, childref : NodeRef) -> TrieItem =
    struct { path = path, value = BranchItem(childref) }

function item_subtree(path : TriePath, childref : NodeRef) -> TrieItem =
    struct { path = path, value = SubtreeItem(childref) }

/* sorted live-leaf items of an update list (deletes emit nothing) */
function patches_live_items(ps : list(TriePatch)) -> list(TrieItem) =
    match ps {
        [||] => [||],
        p :: r => match p.change {
            TrieDelete() => patches_live_items(r),
            TriePut(value) => item_leaf(p.key, value) :: patches_live_items(r),
        },
    }

/* the post-state items at a base leaf, in one pass: the live update leaves
 * with the base leaf inserted at its sorted position; an update on the
 * leaf's own key replaces (write) or erases (delete) it, and diverging
 * updates are inserts */
function leaf_overlay_items(ps : list(TriePatch), key : TriePath, value : Bytes) -> list(TrieItem) =
    match ps {
        [||] => [|item_leaf(key, value)|],
        p :: r => if path_eq(p.key, key) then {
            patches_live_items(ps)
        } else {
            if path_lt(key, p.key) then {
                item_leaf(key, value) :: patches_live_items(ps)
            } else {
                match p.change {
                    TrieDelete() => leaf_overlay_items(r, key, value),
                    TriePut(updated) => item_leaf(p.key, updated) :: leaf_overlay_items(r, key, value),
                }
            }
        },
    }

/* merge two sorted item streams by path */
function items_merge(a : list(TrieItem), b : list(TrieItem)) -> list(TrieItem) =
    match (a, b) {
        ([||], _) => b,
        (_, [||]) => a,
        (x :: ta, y :: tb) => if path_lt(y.path, x.path) then {
            y :: items_merge(a, tb)
        } else {
            x :: items_merge(ta, b)
        },
    }
termination_measure items_merge(a, b) = list_len(a) + list_len(b)

function items_last_path(items : list(TrieItem)) -> TriePath =
    match items {
        [||] => path_empty(),
        it :: r => match r {
            [||] => it.path,
            _ => items_last_path(r),
        },
    }

function items_at_nibble(items : list(TrieItem), depth : int, nib : nibble) -> list(TrieItem) =
    match items {
        [||] => [||],
        it :: r => if path_len(it.path) > depth & path_nibble(it.path, depth) == nib then {
            it :: items_at_nibble(r, depth, nib)
        } else {
            items_at_nibble(r, depth, nib)
        },
    }

/* the child ref of a single-item subtree at `depth`: the item's remaining
 * path is absorbed into it. This is the ONE place a delete collapse can
 * demand node material: an unknown-type hash ref absorbing a nonempty
 * suffix resolves its node from the witness db (fail-closed). */
function item_ref(it : TrieItem, depth : int) -> NodeRef = {
    let suffix = path_drop(it.path, depth);
    match it.value {
        LeafItem(value) => leaf_child_ref(suffix, value),
        BranchItem(subref) => if path_len(suffix) == 0 then {
            subref
        } else {
            merge_ext_ref(suffix, subref)
        },
        SubtreeItem(subref) => if path_len(suffix) == 0 then {
            subref
        } else {
            match subref {
                HashRef(h) => {
                    let node = node_db_lookup(h);
                    if bytes_len(node) == 0 then {
                        throw InvalidBlock(WitnessDeficient)
                    } else {
                        merge_ext_node(suffix, node)
                    }
                },
                _ => merge_ext_ref(suffix, subref),
            }
        },
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
function build_group(items : list(TrieItem), depth : int, fuel : int) -> NodeRef = {
    if fuel <= 0 then {
        throw InvalidBlock(WitnessDeficient)
    } else {
        match items {
            [||] => EmptyRef(),
            it :: [||] => item_ref(it, depth),
            it :: _ => {
                let last = items_last_path(items);
                let common = common_prefix_from(it.path, last, depth);
                if common > 0 then {
                    /* >= 2 items share `shared`, so the child one level down
                       is necessarily a branch: a plain extension wrap is
                       canonical */
                    let shared = path_take(path_drop(it.path, depth), common);
                    extension_child_ref(shared, build_group(items, depth + common, fuel - common))
                } else {
                    /* >= 2 items diverge right here: a branch with >= 2
                       children by construction (groups of a nonempty sorted
                       stream are nonempty and never build to empty) */
                    var mask : bits(16) = 0x0000;
                    var children : list(NodeRef) = [||];
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
 * collapses in the builder with no walker involvement. RLP fields retain their
 * source and spans, so embedded nodes and leaf values remain witness slices. */
val witness_items : (Bytes, TriePath, list(TriePatch), int) -> list(TrieItem)
val witness_child_items : (RlpFieldRef, TriePath, list(TriePatch), int) -> list(TrieItem)

function witness_child_items(
    f : RlpFieldRef,
    prefix : TriePath,
    updates : list(TriePatch),
    fuel : int,
) -> (
    list(TrieItem)
) = {
    match field_to_ref(f) {
        InlineRef(node) => witness_items(node, prefix, updates, fuel),
        EmptyRef() => {
            /* proven-absent subtree: the surviving updates are pure inserts */
            patches_live_items(updates)
        },
        HashRef(h) => {
            let node = node_db_lookup(h);
            if bytes_len(node) == 0 then {
                throw InvalidBlock(WitnessDeficient)
            } else {
                witness_items(node, prefix, updates, fuel)
            }
        },
    }
}

function witness_items(
    node : Bytes,
    prefix : TriePath,
    updates : list(TriePatch),
    fuel : int,
) -> (
    list(TrieItem)
) = {
    if fuel <= 0 then {
        throw InvalidBlock(WitnessDeficient)
    } else {
        if bytes_len(node) == 0 then {
            patches_live_items(updates)
        } else {
            match decode_trie_node(node) {
                LeafNode(leaf) => {
                    let key = path_concat(prefix, leaf.path);
                    leaf_overlay_items(updates, key, rlp_ref_content(leaf.value))
                },
                ExtensionNode(ext) => {
                    let key = path_concat(prefix, ext.path);
                    let (under, outside) = patches_partition_prefix(updates, key);
                    let sub : list(TrieItem) = if patches_empty(under) then {
                            /* untouched extension child: one whole-subtree
                               ref; an extension's child is always a branch */
                            [|item_branch(key, field_to_ref(ext.child))|]
                        } else {
                            witness_child_items(
                                ext.child,
                                key,
                                under,
                                fuel - 1,
                            )
                        };

                    /* updates diverging inside the extension segment become
                       siblings; the builder reshapes the segment around them */
                    items_merge(patches_live_items(outside), sub)
                },
                BranchNode(branch) => {
                    let branch_value = list_nth(branch.fields, 16, RLP_REF_NONE);

                    /* a branch value at slot 16 is impossible for 64-nibble hashed
                       keys (no key is a prefix of another): a malformed witness. */
                    if unsigned(branch_value.content_len) > 0 then {
                        throw InvalidBlock(WitnessDeficient)
                    };
                    var out : list(TrieItem) = [||];
                    foreach (i from 0 to 15) {
                        let nib : nibble = get_slice_int(4, i, 0);
                        let field = list_nth(branch.fields, i, RLP_REF_NONE);
                        let group = patches_at_nibble(updates, path_len(prefix), nib);

                        /* a child slot holds an embedded node, a 32-byte hash,
                           or nothing (field_to_ref of anything else is empty) */
                        let childref = field_to_ref(field);
                        let present : bool = match childref {
                                EmptyRef() => false,
                                _ => true,
                            };
                        let contrib : list(TrieItem) = if patches_empty(group) then {
                                if present then {
                                    /* untouched branch child: node type unknown
                                       for hash refs (is_branch = false) */
                                    [|item_subtree(path_concat(prefix, path_single(nib)), childref)|]
                                } else {
                                    [||]
                                }
                            } else {
                                if present then {
                                    witness_child_items(
                                        field,
                                        path_concat(prefix, path_single(nib)),
                                        group,
                                        fuel - 1,
                                    )
                                } else {
                                    /* no base child: the surviving updates are
                                       pure inserts */
                                    patches_live_items(group)
                                }
                            };
                        out = out ++ contrib
                    };
                    out
                },
                InvalidNode() => [||],
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
 * touched hash reference in the witness node-db and any missing node throws
 * InvalidBlock(WitnessDeficient); otherwise the builder recomposes the emitted
 * stream canonically.
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
            if bytes_len(node) == 0 then {
                throw InvalidBlock(WitnessDeficient)
            } else {
                items_root(witness_items(node, path_empty(), updates, 256))
            }
        }
    }
}

/* Walk the secure trie without copying its leaf value. */
function trie_walk(node : Bytes, key : TriePath, pos : int, fuel : int) -> Bytes = {
    if fuel <= 0 | bytes_len(node) == 0 then {
        BytesList([||])
    } else {
        match decode_trie_node(node) {
            LeafNode(leaf) => if not_bool(path_matches(key, pos, leaf.path)) then {
                    BytesList([||])
                } else {
                    if pos + path_len(leaf.path) == path_len(key) then {
                        rlp_ref_content(leaf.value)
                    } else {
                        BytesList([||])
                    }
                },
            ExtensionNode(ext) => if not_bool(path_matches(key, pos, ext.path)) then {
                    BytesList([||])
                } else {
                    trie_walk(
                        resolve_ref(field_to_ref(ext.child)),
                        key,
                        pos + path_len(ext.path),
                        fuel - 1,
                    )
                },
            BranchNode(branch) => {
                if pos == path_len(key) then {
                    rlp_ref_content(list_nth(branch.fields, 16, RLP_REF_NONE))
                } else {
                    let child = resolve_ref(
                        field_to_ref(
                            list_nth(branch.fields, unsigned(path_nibble(key, pos)), RLP_REF_NONE),
                        ),
                    );
                    trie_walk(child, key, pos + 1, fuel - 1)
                }
            },
            InvalidNode() => BytesList([||]),
        }
    }
}
termination_measure trie_walk(node, key, pos, fuel) = fuel

function trie_lookup(root : hash, key : TriePath) -> Bytes = {
    if root == EMPTY_TRIE_ROOT then {
        BytesList([||])
    } else {
        let node = node_db_lookup(root);
        if bytes_len(node) == 0 then {
            throw InvalidBlock(WitnessDeficient)
        } else {
            trie_walk(node, key, 0, 256)
        }
    }
}

/* Populate the C node-db from witness.state: keccak(node) -> source span. */
function build_node_db(nodes : list(ByteSlice)) -> unit = {
    nodedb_reset();
    var rest : list(ByteSlice) = nodes;
    var more : bool = true;
    while more do {
        match rest {
            [||] => more = false,
            s :: r => {
                nodedb_insert(keccak256(s), s.off, s.len);
                rest = r
            },
        }
    }
}
