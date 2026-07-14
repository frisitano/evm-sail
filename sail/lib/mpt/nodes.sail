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

/* Node bytes for keccak `h`, retained as a witness slice. */
function node_db_lookup(h : bits(256)) -> Bytes = {
    let len = unsigned(nodedb_len(h));
    if len == 0 then {
        BytesList([||])
    } else {
        BytesSlice(stateless_input_byte_slice(unsigned(nodedb_off(h)), len))
    }
}

struct LeafNodeData = { path : TriePath, value : RlpFieldRef }
struct ExtensionNodeData = { path : TriePath, child : RlpFieldRef }
type BranchChildren = vector(16, dec, RlpFieldRef)
struct BranchNodeData = { children : BranchChildren, value : RlpFieldRef }

union TrieNode = {
    LeafNode : LeafNodeData,
    ExtensionNode : ExtensionNodeData,
    BranchNode : BranchNodeData,
    InvalidNode : unit,
}

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
                LeafNode(leaf) => leaf_child_ref(path_concat(prefix, leaf.path), rlp_ref_content(leaf.value)),
                ExtensionNode(ext) => extension_child_ref(path_concat(prefix, ext.path), field_to_ref(ext.child)),
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
                    LeafNode(leaf) => leaf_child_ref(path_concat(prefix, leaf.path), rlp_ref_content(leaf.value)),
                    ExtensionNode(ext) => extension_child_ref(path_concat(prefix, ext.path), field_to_ref(ext.child)),
                    _ => extension_child_ref(prefix, childref),
                }
            },
        }
    }
}
