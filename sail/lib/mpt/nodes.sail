/*md
# Trie nodes

Merkle-Patricia trie node forms, references, and decoding
(YP Appendix D).
*/

/*! A reference to a trie node: empty, inline (encodings under 32 bytes),
    or by KECCAK-256 hash (YP Appendix D, Eq. 207). */
union NodeRef = {
    EmptyRef : unit,
    InlineRef : Bytes,
    HashRef : hash,
}

/*! The RLP of a node reference as it appears inside a parent node. */
function node_ref_rlp(r : NodeRef) -> list(byte) =
    match r {
        EmptyRef() => [|0x80|],
        InlineRef(node) => bytes_to_list(node),
        HashRef(h) => [|0xa0|] ++ word_to_bytes32(h),
    }

/*! The canonical child reference for an encoded node: inline under 32
    bytes, otherwise its hash (YP Appendix D, Eq. 207). */
function child_ref(enc : list(byte)) -> NodeRef =
    if list_len(enc) < 32 then {
        InlineRef(BytesList(enc))
    } else {
        HashRef(keccak256(enc))
    }

/*! Whether the branch mask has a child at nibble `i`. */
function branch_mask_has_bit(mask : bits(16), i : int) -> bool =
    (sail_shiftright(mask, i) & 0x0001) == 0x0001
/*! Sets the branch-mask bit for nibble `i`. */
function branch_mask_set_bit(mask : bits(16), i : int) -> bits(16) =
    mask | sail_shiftleft(0x0001, i)

/*! The RLP of a leaf node: `[HP(key, true), value]` (YP Appendix D,
    Eq. 208). */
function leaf_node_rlp(key : TriePath, value : list(byte)) -> list(byte) =
    rlp_list(rlp_bytes(hex_prefix_compact(key, true)) ++ rlp_bytes(value))
/*! The RLP of an extension node: `[HP(key, false), child]`. */
function extension_node_rlp(key : TriePath, childref : NodeRef) -> list(byte) =
    rlp_list(rlp_bytes(hex_prefix_compact(key, false)) ++ node_ref_rlp(childref))

/*! The child reference of a leaf, keeping the value in its native
    representation: long nodes hash the RLP framing and value as segments;
    only an inline node materializes a slice. */
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

/*! The child reference of an extension node. */
function extension_child_ref(key : TriePath, childref : NodeRef) -> NodeRef =
    child_ref(extension_node_rlp(key, childref))

/*! The RLP payload of a branch: sixteen child slots in ascending nibble
    order (absent children encode as `0x80`) plus the empty value slot. */
function branch_payload_rlp(mask : bits(16), children : list(NodeRef)) -> list(byte) = {
    var payload_rev : list(byte) = [||];
    var ch : list(NodeRef) = children; /* ascending nibble order */
    foreach (i from 0 to 15) {
        if branch_mask_has_bit(mask, i) then {
            match ch {
                c :: t => {
                    payload_rev = rev_onto(node_ref_rlp(c), payload_rev);
                    ch = t
                },
                [||] => (),
            }
        } else {
            payload_rev = 0x80 :: payload_rev
        }
    };
    list_reverse(0x80 :: payload_rev) /* branch value slot: empty */
}
/*! The child reference of a branch node. */
function branch_child_ref(mask : bits(16), children : list(NodeRef)) -> NodeRef =
    child_ref(rlp_list(branch_payload_rlp(mask, children)))

/*! The root hash a node reference commits to; the empty reference is the
    empty-trie root. */
function trie_ref_to_root(r : NodeRef) -> bits(256) =
    match r {
        EmptyRef() => EMPTY_TRIE_ROOT,
        InlineRef(node) => keccak256([|node|]),
        HashRef(h) => h,
    }

/*! The reference form of raw node bytes: empty, inline under 32 bytes,
    else hashed. */
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

/*! The witness node bytes whose KECCAK-256 digest is `h`, retained as a
    slice into the stateless input; empty if unwitnessed. */
function node_db_lookup(h : bits(256)) -> Bytes = {
    let len = unsigned(nodedb_len(h));
    if len == 0 then {
        BytesList([||])
    } else {
        BytesSlice(stateless_input_byte_slice(unsigned(nodedb_off(h)), len))
    }
}

/*! A decoded leaf: its remaining path and value field. */
struct LeafNodeData = { path : TriePath, value : RlpFieldRef }
/*! A decoded extension: its shared path and child field. */
struct ExtensionNodeData = { path : TriePath, child : RlpFieldRef }
/*! The sixteen child fields of a decoded branch node. */
type BranchChildren = vector(16, dec, RlpFieldRef)
/*! A decoded branch: sixteen child fields and the value field. */
struct BranchNodeData = { children : BranchChildren, value : RlpFieldRef }

/*! A decoded trie node, or `InvalidNode` when the bytes are not a
    well-formed node. */
union TrieNode = {
    LeafNode : LeafNodeData,
    ExtensionNode : ExtensionNodeData,
    BranchNode : BranchNodeData,
    InvalidNode : unit,
}

/*! Decodes node bytes into leaf/extension/branch form by field count
    (2 = leaf or extension by the HP flag; 17 = branch). */
function decode_trie_node(node : Bytes) -> TrieNode = {
    let fields = rlp_node_cursor(node);
    if not_bool(fields.valid) | rlp_cursor_empty(fields) then {
        InvalidNode()
    } else {
        let (first, fields) = rlp_cursor_pop(fields);
        if rlp_cursor_empty(fields) then {
            InvalidNode()
        } else {
            let (second, fields) = rlp_cursor_pop(fields);
            if rlp_cursor_empty(fields) then {
                let path_field = first;
                let value = second;
                let (is_leaf, path) = hex_prefix_decode_ref(path_field);
                if is_leaf then {
                    LeafNode(struct { path = path, value = value })
                } else {
                    ExtensionNode(struct { path = path, child = value })
                }
            } else {
                var children : BranchChildren = vector_init(16, first);
                children[0] = first;
                children[1] = second;
                var cursor = fields;
                var index : int = 2;
                while (index < 16) & not_bool(rlp_cursor_empty(cursor)) do {
                    let (child, next) = rlp_cursor_pop(cursor);
                    let child_index = unsigned(get_slice_int(4, index, 0));
                    children[child_index] = child;
                    cursor = next;
                    index = index + 1
                };
                if (index != 16) | rlp_cursor_empty(cursor) then {
                    InvalidNode()
                } else {
                    let (value, cursor) = rlp_cursor_pop(cursor);
                    if rlp_cursor_empty(cursor) then {
                        BranchNode(struct { children = children, value = value })
                    } else {
                        InvalidNode()
                    }
                }
            }
        }
    }
}

/*! The reference denoted by a child field: an inline list, a 32-byte
    hash, or empty. */
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

/*! Resolves a reference to node bytes. Resolving a missing hash is a
    deficient witness (`InvalidBlock(WitnessDeficient)`), never an empty
    subtree. */
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

/*! Re-keys a decoded child node under `prefix` without copying a leaf
    value. */
function merge_ext_node(prefix : TriePath, childnode : Bytes) -> NodeRef = {
    if path_len(prefix) == 0 then {
        node_to_ref(childnode)
    } else {
        if bytes_len(childnode) == 0 then {
            EmptyRef()
        } else {
            match decode_trie_node(childnode) {
                LeafNode(leaf) => leaf_child_ref(path_concat(prefix, leaf.path), rlp_ref_content(leaf.value)),
                ExtensionNode(ext) => extension_child_ref(path_concat(prefix, ext.path), field_to_ref(ext.child)),
                _ => extension_child_ref(prefix, node_to_ref(childnode)),
            }
        }
    }
}

/*! [merge_ext_node][] over a child reference: an inline reference
    carries its node bytes and re-keys canonically; a 32-byte hash
    reference is wrapped in an extension, which is canonical only when the
    referenced node is a branch. */
function merge_ext_ref(prefix : TriePath, childref : NodeRef) -> NodeRef = {
    if path_len(prefix) == 0 then {
        childref
    } else {
        match childref {
            EmptyRef() => EmptyRef(),
            HashRef(_) => extension_child_ref(prefix, childref),
            InlineRef(node) => {
                match decode_trie_node(node) {
                    LeafNode(leaf) => leaf_child_ref(path_concat(prefix, leaf.path), rlp_ref_content(leaf.value)),
                    ExtensionNode(ext) => extension_child_ref(path_concat(prefix, ext.path), field_to_ref(ext.child)),
                    _ => extension_child_ref(prefix, childref),
                }
            },
        }
    }
}
