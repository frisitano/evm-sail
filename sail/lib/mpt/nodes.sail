/* An inline MPT child is at most 31 bytes. Keep it self-contained so a node
 * reference never borrows scratch memory or retains a heap byte list. `data`
 * is the big-endian payload in the low bits. */

/*md
# Trie nodes

Merkle-Patricia trie node forms, references, and decoding
(YP Appendix D).
*/
/*! A self-contained trie-node encoding shorter than 32 bytes. */
struct InlineNode = {
    data : b256,
    len : range(0, 31),
}

let MPT_HASH_LENGTH : int(32) = WORD_BYTE_LENGTH

/*! Copies a sub-32-byte node encoding into an inline node value. */
function inline_node_from_slice(bytes : ByteSlice) -> InlineNode = {
    let length = bytes.len;
    if MPT_HASH_LENGTH <= length then {
        throw InvalidBlock(WitnessDeficient)
    };
    struct { data = word_to_hash(slice_load(bytes, 0)), len = length }
}

/*! Materializes an inline node in scratch memory as a byte slice. */
function inline_node_slice(node : InlineNode) -> ByteSlice = {
    let start = scratch_begin();
    scratch_push_b256(node.data, node.len);
    scratch_finish(start)
}

/*! Hashes an inline node directly from its fixed byte vector. */
function inline_node_hash(node : InlineNode) -> hash =
    keccak256([|bytes_fixed32(node.data, node.len)|])

/*! A reference to a trie node: empty, inline (encodings under 32 bytes),
    or by KECCAK-256 hash (YP Appendix D, Eq. 207). */
union NodeRef = {
    /* the empty node */
    EmptyRef : unit,
    /* a node embedded in its parent (encoding under 32 bytes) */
    InlineRef : InlineNode,
    /* a node referenced by its KECCAK-256 hash */
    HashRef : hash,
}

/*! The sixteen child references of a branch, indexed by nibble. */
type BranchRefs = vector(16, dec, NodeRef)

/*! The RLP payload of a branch contains sixteen child references of at most
    33 bytes and one empty value byte. */
type branch_content_length = range(0, 529)

/*! Advances the branch payload length while preserving its structural bound. */
function branch_content_length_add(current : branch_content_length, addition : range(0, 33)) -> branch_content_length =
    if addition <= 529 - current then {
        current + addition
    } else {
        throw InvalidBlock(RlpDecode)
    }

/*! Returns the RLP width of a child reference in its parent node. */
function node_ref_size(r : NodeRef) -> range(0, 33) =
    match r {
        EmptyRef() => 1,
        InlineRef(node) => node.len,
        HashRef(_) => rlp_word_size(),
    }

/*! Appends a child reference in its canonical RLP representation. */
function rlp_write_node_ref(r : NodeRef) -> unit =
    match r {
        EmptyRef() => scratch_push_bytes([|0x80|], 1),
        InlineRef(node) => scratch_push_b256(node.data, node.len),
        HashRef(h) => rlp_write_word(hash_to_word(h)),
    }

/*! The canonical child reference for an encoded node: inline under 32
    bytes, otherwise its hash (YP Appendix D, Eq. 207). */
function child_ref(encoded : ByteSlice) -> NodeRef =
    if encoded.len < MPT_HASH_LENGTH then {
        InlineRef(inline_node_from_slice(encoded))
    } else {
        HashRef(keccak256_slice(encoded))
    }

/*! Returns the one-hot presence mask for a branch-child nibble. */
function branch_mask_for(index : nibble) -> bits(16) =
    sail_shiftleft(0x0001, unsigned(index))

function branch_mask_has(mask : bits(16), index : nibble) -> bool =
    and_vec(mask, branch_mask_for(index)) != 0x0000

function branch_mask_set(mask : bits(16), index : nibble) -> bits(16) =
    or_vec(mask, branch_mask_for(index))

/* Yellow Paper Appendix D, Eq. 208. Keep leaf values in their native
 * representation; only a sub-32-byte node is materialized into InlineNode. */
/*! The child reference of a leaf, keeping the value in its native
    representation: long nodes hash the RLP framing and value as segments;
    only an inline node materializes a slice. */
function leaf_child_ref(key : TriePath, value : ByteSlice) -> NodeRef = {
    let (path, encoded_path_len) = hex_prefix_compact(key, true);
    let content_len = rlp_scratch_length_add(
        rlp_scratch_bytes_size(path, encoded_path_len),
        rlp_scratch_slice_size(value),
    );
    let mark = scratch_begin();
    rlp_write_list_prefix(content_len);
    rlp_write_bytes(path, encoded_path_len);
    rlp_write_slice(value);
    let result = child_ref(rlp_finish(mark));
    scratch_rewind(mark);
    result
}

/*! The child reference of an extension node. */
function extension_child_ref(key : TriePath, childref : NodeRef) -> NodeRef = {
    let (path, encoded_path_len) = hex_prefix_compact(key, false);
    let path_length = rlp_bytes_size(path, encoded_path_len);
    let child_length = node_ref_size(childref);
    let content_len = path_length + child_length;
    let mark = scratch_begin();
    rlp_write_list_prefix(content_len);
    rlp_write_bytes(path, encoded_path_len);
    rlp_write_node_ref(childref);
    let result = child_ref(rlp_finish(mark));
    scratch_rewind(mark);
    result
}

/*! The child reference of a branch node. */
function branch_child_ref(mask : bits(16), children : BranchRefs) -> NodeRef = {
    var content_length : branch_content_length = 1;
    var child_bit : bits(16) = 0x0001;
    foreach (i from 0 to 15) {
        if and_vec(mask, child_bit) != 0x0000 then {
            let child_length = node_ref_size(children[i]);
            content_length = branch_content_length_add(content_length, child_length)
        } else {
            content_length = branch_content_length_add(content_length, 1)
        };
        child_bit = sail_shiftleft(child_bit, 1)
    };

    let mark = scratch_begin();
    rlp_write_list_prefix(content_length);
    child_bit = 0x0001;
    foreach (i from 0 to 15) {
        if and_vec(mask, child_bit) != 0x0000 then {
            rlp_write_node_ref(children[i])
        } else {
            scratch_push_bytes([|0x80|], 1)
        };
        child_bit = sail_shiftleft(child_bit, 1)
    };
    scratch_push_bytes([|0x80|], 1);
    let result = child_ref(rlp_finish(mark));
    scratch_rewind(mark);
    result
}

/*! The root hash a node reference commits to; the empty reference is the
    empty-trie root. */
function trie_ref_to_root(r : NodeRef) -> hash =
    match r {
        EmptyRef() => EMPTY_TRIE_ROOT,
        InlineRef(node) => inline_node_hash(node),
        HashRef(h) => h,
    }

/*! The reference form of raw node bytes: empty, inline under 32 bytes,
    else hashed. */
function node_to_ref(node : ByteSlice) -> NodeRef =
    if node.len == 0 then {
        EmptyRef()
    } else {
        if node.len < MPT_HASH_LENGTH then {
            InlineRef(inline_node_from_slice(node))
        } else {
            HashRef(keccak256_slice(node))
        }
    }

/*! The witness node bytes whose KECCAK-256 digest is `h`, retained as a
    slice into the stateless input; empty if unwitnessed. */
function node_db_lookup(h : hash) -> ByteSlice = {
    nodedb_lookup(h)
}

/*! A decoded leaf: its remaining path and value field. */
struct LeafNodeData = { path : TriePath, value : RlpFieldRef }
/*! A decoded extension: its shared path and child field. */
struct ExtensionNodeData = { path : TriePath, child : RlpFieldRef }
/*! The sixteen child fields of a decoded branch node. */
type BranchChildren = vector(16, dec, RlpFieldRef)

/*! Selects a decoded branch child field by nibble value. */
function branch_children_get(children : BranchChildren, index : nibble) -> RlpFieldRef = match index {
    0x0 => children[0],
    0x1 => children[1],
    0x2 => children[2],
    0x3 => children[3],
    0x4 => children[4],
    0x5 => children[5],
    0x6 => children[6],
    0x7 => children[7],
    0x8 => children[8],
    0x9 => children[9],
    0xa => children[10],
    0xb => children[11],
    0xc => children[12],
    0xd => children[13],
    0xe => children[14],
    0xf => children[15],
}
/*! A decoded branch: sixteen child fields and the value field. */
struct BranchNodeData = { children : BranchChildren, value : RlpFieldRef }

/*! A decoded trie node, or `InvalidNode` when the bytes are not a
    well-formed node. */
union TrieNode = {
    /* remaining path and value */
    LeafNode : LeafNodeData,
    /* shared path prefix and single child */
    ExtensionNode : ExtensionNodeData,
    /* sixteen children and an optional value */
    BranchNode : BranchNodeData,
    /* undecodable bytes: a deficient witness */
    InvalidNode : unit,
}

/*! Decodes node bytes into leaf/extension/branch form by field count
    (2 = leaf or extension by the HP flag; 17 = branch). */
function decode_trie_node(node : ByteSlice) -> TrieNode = {
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
                    if path_len(path) == 0 then {
                        InvalidNode()
                    } else {
                        ExtensionNode(struct { path = path, child = value })
                    }
                }
            } else {
                var children : BranchChildren = vector_init(16, first);
                children[0] = first;
                children[1] = second;
                var cursor : RlpCursor = fields;
                var complete : bool = true;
                foreach (index from 2 to 15) {
                    if rlp_cursor_empty(cursor) then {
                        complete = false
                    } else {
                        let (child, next) = rlp_cursor_pop(cursor);
                        children[index] = child;
                        cursor = next
                    }
                };
                if not_bool(complete) | rlp_cursor_empty(cursor) then {
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
        InlineRef(inline_node_from_slice(rlp_ref_full(f)))
    } else {
        if f.content_len == MPT_HASH_LENGTH then {
            HashRef(word_to_hash(rlp_ref_word(f)))
        } else {
            EmptyRef()
        }
    }

/*! Resolves a reference to node bytes. Resolving a missing hash is a
    deficient witness (`InvalidBlock(WitnessDeficient)`), never an empty
    subtree. */
function resolve_ref(r : NodeRef) -> ByteSlice =
    match r {
        EmptyRef() => EMPTY_SLICE,
        InlineRef(node) => inline_node_slice(node),
        HashRef(h) => {
            let node = node_db_lookup(h);
            if node.len == 0 then {
                throw InvalidBlock(WitnessDeficient)
            } else {
                node
            }
        },
    }

/*! Re-keys a decoded child node under `prefix` without copying a leaf
    value. */
function merge_ext_node(prefix : TriePath, childnode : ByteSlice) -> NodeRef = {
    if path_len(prefix) == 0 then {
        node_to_ref(childnode)
    } else {
        if childnode.len == 0 then {
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
                match decode_trie_node(inline_node_slice(node)) {
                    LeafNode(leaf) => leaf_child_ref(path_concat(prefix, leaf.path), rlp_ref_content(leaf.value)),
                    ExtensionNode(ext) => extension_child_ref(path_concat(prefix, ext.path), field_to_ref(ext.child)),
                    _ => extension_child_ref(prefix, childref),
                }
            },
        }
    }
}
