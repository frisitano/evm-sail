/*md
# Merkle-Patricia trie node codec

Canonical hex-prefix and RLP encoding, decoding, and references for trie nodes
(YP Appendix C/D).
*/

/*! Returns the RLP width of a child reference in its parent node. */
function node_ref_size(r : NodeRef) -> range(0, 33) =
    match r {
        EmptyRef() => 1,
        InputInlineRef(node) => node.len,
        ScratchInlineRef(node) => node.len,
        HashRef(_) => rlp_word_size(),
    }

/*! Appends a child reference in its canonical RLP representation. */
function rlp_write_node_ref(r : NodeRef) -> unit =
    match r {
        EmptyRef() => scratch_push_byte(0x80),
        InputInlineRef(node) => scratch_push_slice(node),
        ScratchInlineRef(node) => scratch_push_b256(node.data, node.len),
        HashRef(h) => {
            let hash_word = hash_to_word(h);
            rlp_write_word(hash_word)
        },
    }

/*! Returns the canonical RLP width of a compact trie path. */
function rlp_hex_prefix_size(path : TriePath, is_leaf : bool) -> range(1, 34) = {
    let encoded_length = hex_prefix_encoded_length(path);
    let first = hex_prefix_first_byte(path, is_leaf);
    if (encoded_length == 1) & (first[7] == bitzero) then {
        1
    } else {
        encoded_length + 1
    }
}

/*! Writes the hex-prefix path directly into scratch in wire order. */
function rlp_write_hex_prefix(path : TriePath, is_leaf : bool) -> unit = {
    let length = path_len(path);
    let encoded_length = hex_prefix_encoded_length(path);
    let first = hex_prefix_first_byte(path, is_leaf);
    rlp_write_string_prefix(encoded_length, first);
    scratch_push_byte(first);
    let odd_length = tmod_nat(length, 2) != 0;
    var index : hex_prefix_cursor =
        if odd_length then 1 else 0;
    while index < length termination_measure(length - index) do {
        let current : trie_path_cursor = tmod_nat(index, 65);
        let next : trie_path_cursor = tmod_nat(current + 1, 65);
        let high = path_nibble(path, current);
        let low = path_nibble(path, next);
        let path_byte = append(high, low);
        scratch_push_byte(path_byte);
        index = next + 1
    }
}

/*! The canonical child reference for an encoded node: inline under 32
    bytes, otherwise its hash (YP Appendix D, Eq. 207). */
function child_ref(encoded : ScratchSlice) -> NodeRef =
    if encoded.len < MPT_HASH_LENGTH then {
        let inline_node = inline_node_from_slice(encoded);
        ScratchInlineRef(inline_node)
    } else {
        let node_hash = keccak256(encoded);
        HashRef(node_hash)
    }

/*! A compact presence bitset for the sixteen children of a branch. */
type branch_mask = bits(16)

/*! Returns the one-hot presence mask for a branch-child nibble. */
function branch_mask_for(index : nibble) -> branch_mask = {
    let shift = unsigned(index);
    sail_shiftleft(0x0001, shift)
}

function branch_mask_has(mask : branch_mask, index : nibble) -> bool = {
    let index_mask = branch_mask_for(index);
    and_vec(mask, index_mask) != 0x0000
}

function branch_mask_set(mask : branch_mask, index : nibble) -> branch_mask = {
    let index_mask = branch_mask_for(index);
    or_vec(mask, index_mask)
}

/* Yellow Paper Appendix D, Eq. 208. Keep leaf values in their native
 * representation; only a sub-32-byte node is materialized into InlineNode. */
/*! The child reference of a leaf, keeping the value in its native
    representation: long nodes hash the RLP framing and value as segments;
    only an inline node materializes a slice. */
function input_leaf_child_ref(key : TriePath, value : StatelessInputSlice) -> NodeRef = {
    let path_size = rlp_hex_prefix_size(key, true);
    let value_size = rlp_scratch_slice_size(value);
    let content_len = rlp_scratch_length_add(path_size, value_size);
    let encoded_size = rlp_scratch_list_size(content_len);
    let encoder = rlp_encoder_begin(encoded_size);
    rlp_write_list_prefix(content_len);
    rlp_write_hex_prefix(key, true);
    rlp_write_slice(value);
    let encoded = rlp_encoder_finish(encoder);
    let result = child_ref(encoded);
    rlp_encoder_rewind(encoder);
    result
}

function scratch_leaf_child_ref(key : TriePath, value : ScratchSlice) -> NodeRef = {
    let path_size = rlp_hex_prefix_size(key, true);
    let value_size = rlp_scratch_slice_size(value);
    let content_len = rlp_scratch_length_add(path_size, value_size);
    let encoded_size = rlp_scratch_list_size(content_len);
    let encoder = rlp_encoder_begin(encoded_size);
    rlp_write_list_prefix(content_len);
    rlp_write_hex_prefix(key, true);
    rlp_write_slice(value);
    let encoded = rlp_encoder_finish(encoder);
    let result = child_ref(encoded);
    rlp_encoder_rewind(encoder);
    result
}

function leaf_child_ref(key : TriePath, value : TrieLeafValue) -> NodeRef =
    match value {
        InputTrieLeaf(bytes) => input_leaf_child_ref(key, bytes),
        ScratchTrieLeaf(bytes) => scratch_leaf_child_ref(key, bytes),
    }

/*! The child reference of an extension node. */
function extension_child_ref(key : TriePath, childref : NodeRef) -> NodeRef = {
    let path_length = rlp_hex_prefix_size(key, false);
    let child_length = node_ref_size(childref);
    let content_len = path_length + child_length;
    let encoded_size = rlp_list_size(content_len);
    let encoder = rlp_encoder_begin(encoded_size);
    rlp_write_list_prefix(content_len);
    rlp_write_hex_prefix(key, false);
    rlp_write_node_ref(childref);
    let encoded = rlp_encoder_finish(encoder);
    let result = child_ref(encoded);
    rlp_encoder_rewind(encoder);
    result
}

/*! The child reference of a branch node. */
function branch_child_ref(mask : branch_mask, children : BranchRefs) -> NodeRef = {
    var content_length : branch_content_length = 1;
    var child_bit : branch_mask = 0x0001;
    foreach (i from 0 to 15) {
        let child_present = and_vec(mask, child_bit) != 0x0000;
        if child_present then {
            let child_length = node_ref_size(children[i]);
            content_length = branch_content_length_add(content_length, child_length)
        } else {
            content_length = branch_content_length_add(content_length, 1)
        };
        child_bit = sail_shiftleft(child_bit, 1)
    };

    let scratch_content_length = rlp_scratch_length_add(content_length, 0);
    let encoded_size = rlp_scratch_list_size(scratch_content_length);
    let encoder = rlp_encoder_begin(encoded_size);
    rlp_write_list_prefix(content_length);
    child_bit = 0x0001;
    foreach (i from 0 to 15) {
        let child_present = and_vec(mask, child_bit) != 0x0000;
        if child_present then {
            rlp_write_node_ref(children[i])
        } else {
            scratch_push_byte(0x80)
        };
        child_bit = sail_shiftleft(child_bit, 1)
    };
    scratch_push_byte(0x80);
    let encoded = rlp_encoder_finish(encoder);
    let result = child_ref(encoded);
    rlp_encoder_rewind(encoder);
    result
}

/*! The root hash a node reference commits to; the empty reference is the
    empty-trie root. */
function trie_ref_to_root(r : NodeRef) -> hash =
    match r {
        EmptyRef() => EMPTY_TRIE_ROOT,
        InputInlineRef(node) => keccak256(node),
        ScratchInlineRef(node) => inline_node_hash(node),
        HashRef(h) => h,
    }

/*! The reference form of authenticated input node bytes. */
function input_node_to_ref(node : StatelessInputSlice) -> NodeRef =
    if node.len == 0 then {
        EmptyRef()
    } else if node.len < MPT_HASH_LENGTH then {
        InputInlineRef(node)
    } else {
        let node_hash = keccak256(node);
        HashRef(node_hash)
    }

/*! The reference form of freshly encoded scratch node bytes. */
function scratch_node_to_ref(node : ScratchSlice) -> NodeRef =
    if node.len == 0 then {
        EmptyRef()
    } else if node.len < MPT_HASH_LENGTH then {
        let inline_node = inline_node_from_slice(node);
        ScratchInlineRef(inline_node)
    } else {
        let node_hash = keccak256(node);
        HashRef(node_hash)
    }

overload node_to_ref = {input_node_to_ref, scratch_node_to_ref}

/*! The witness node bytes whose KECCAK-256 digest is `h`, retained as a
    slice into the stateless input; empty if unwitnessed. */
function node_db_lookup(h : hash) -> StatelessInputSlice = {
    nodedb_lookup(h)
}

/*! Selects a decoded branch child reference by nibble value. */
function branch_refs_get(children : BranchRefs, index : nibble) -> NodeRef = match index {
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

/*! A decoded authenticated node. Every borrowed field remains a stateless
    input slice, including fields reached through an input-inline child. */
union InputTrieNode = {
    InputLeafNode : (TriePath, StatelessInputSlice),
    InputExtensionNode : (TriePath, NodeRef),
    InputBranchNode : (BranchRefs, StatelessInputSlice),
}

/*! A decoded node freshly encoded in scratch during canonical rebuilding. */
union ScratchTrieNode = {
    ScratchLeafNode : (TriePath, ScratchSlice),
    ScratchExtensionNode : (TriePath, NodeRef),
    ScratchBranchNode : (BranchRefs, ScratchSlice),
}

/*! The reference denoted by a child field: an inline list, a 32-byte
    hash, or empty. */
function input_field_to_ref forall 'source_off 'source_len 'content_len,
                              rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    NodeRef
) =
    if f.is_list then {
        if f.source.len < MPT_HASH_LENGTH then {
            InputInlineRef(f.source)
        } else {
            fatal_error(RlpDecode)
        }
    } else if f.content_len == MPT_HASH_LENGTH then {
        let word = rlp_decode_word(f);
        let hash = word_to_hash(word);
        HashRef(hash)
    } else {
        EmptyRef()
    }

function scratch_field_to_ref forall 'source_off 'source_len 'content_len,
                                rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    ScratchRlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    NodeRef
) =
    if f.is_list then {
        let inline_node = inline_node_from_slice(f.source);
        ScratchInlineRef(inline_node)
    } else if f.content_len == MPT_HASH_LENGTH then {
        let word = scratch_rlp_decode_word(f);
        let hash = word_to_hash(word);
        HashRef(hash)
    } else {
        EmptyRef()
    }

/*! Decodes branch children 2 through 15, followed by the branch value. */
function decode_input_branch_node forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    cursor : RlpCursor('source_off, 'source_len),
    index : range(2, 16),
    children : BranchRefs,
) -> (
    InputTrieNode
) =
    if index < 16 then {
        let child = rlp_decode_item(cursor);
        let next = rlp_cursor_advance(cursor, child.source.len);
        var updated = children;
        updated[index] = input_field_to_ref(child);
        decode_input_branch_node(next, index + 1, updated)
    } else {
        let value = rlp_decode_item(cursor);
        let next = rlp_cursor_advance(cursor, value.source.len);
        rlp_cursor_expect_end(next);
        let content = rlp_item_content(value);
        InputBranchNode(children, content)
    }
termination_measure decode_input_branch_node(cursor, index, children) = 16 - index

function decode_scratch_branch_node forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    cursor : ScratchRlpCursor('source_off, 'source_len),
    index : range(2, 16),
    children : BranchRefs,
) -> (
    ScratchTrieNode
) =
    if index < 16 then {
        let child = scratch_rlp_decode_item(cursor);
        let next = scratch_rlp_cursor_advance(cursor, child.source.len);
        var updated = children;
        updated[index] = scratch_field_to_ref(child);
        decode_scratch_branch_node(next, index + 1, updated)
    } else {
        let value = scratch_rlp_decode_item(cursor);
        let next = scratch_rlp_cursor_advance(cursor, value.source.len);
        scratch_rlp_cursor_expect_end(next);
        let content = scratch_rlp_item_content(value);
        ScratchBranchNode(children, content)
    }
termination_measure decode_scratch_branch_node(cursor, index, children) = 16 - index

/*! Decodes node bytes into leaf/extension/branch form by field count
    (2 = leaf or extension by the HP flag; 17 = branch). */
function decode_input_trie_node(node : StatelessInputSlice) -> InputTrieNode = {
    if node.len == 0 then {
        fatal_error(RlpDecode)
    };
    let fields = rlp_node_cursor(node);
    let first = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, first.source.len);
    let second = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, second.source.len);
    if fields.len == 0 then {
        let (is_leaf, path) = hex_prefix_decode_ref(first);
        if is_leaf then {
            let value = rlp_item_content(second);
            InputLeafNode(path, value)
        } else {
            let path_length = path_len(path);
            if path_length == 0 then {
                fatal_error(RlpDecode)
            } else {
                let child = input_field_to_ref(second);
                InputExtensionNode(path, child)
            }
        }
    } else {
        let empty_child = EmptyRef();
        var children : BranchRefs = vector_init(16, empty_child);
        children[0] = input_field_to_ref(first);
        children[1] = input_field_to_ref(second);
        decode_input_branch_node(fields, 2, children)
    }
}

function decode_scratch_trie_node(node : ScratchSlice) -> ScratchTrieNode = {
    let fields = scratch_rlp_node_cursor(node);
    let first = scratch_rlp_decode_item(fields);
    let fields = scratch_rlp_cursor_advance(fields, first.source.len);
    let second = scratch_rlp_decode_item(fields);
    let fields = scratch_rlp_cursor_advance(fields, second.source.len);
    if fields.len == 0 then {
        let (is_leaf, path) = scratch_hex_prefix_decode_ref(first);
        if is_leaf then {
            let value = scratch_rlp_item_content(second);
            ScratchLeafNode(path, value)
        } else {
            let path_length = path_len(path);
            if path_length == 0 then {
                fatal_error(RlpDecode)
            } else {
                let child = scratch_field_to_ref(second);
                ScratchExtensionNode(path, child)
            }
        }
    } else {
        let empty_child = EmptyRef();
        var children : BranchRefs = vector_init(16, empty_child);
        children[0] = scratch_field_to_ref(first);
        children[1] = scratch_field_to_ref(second);
        decode_scratch_branch_node(fields, 2, children)
    }
}

/*! Resolves a reference to node bytes. Resolving a missing hash is a
    deficient witness (`fatal_error(WitnessDeficient)`), never an empty
    subtree. */
function resolve_witness_ref(r : NodeRef) -> StatelessInputSlice =
    match r {
        EmptyRef() => EMPTY_STATELESS_INPUT_SLICE,
        InputInlineRef(node) => node,
        ScratchInlineRef(_) => fatal_error(WitnessDeficient),
        HashRef(h) => {
            let node = node_db_lookup(h);
            if node.len == 0 then {
                fatal_error(WitnessDeficient)
            } else {
                node
            }
        },
    }

/*! Re-keys a decoded child node under `prefix` without copying a leaf
    value. */
function merge_ext_node(prefix : TriePath, childnode : StatelessInputSlice) -> NodeRef = {
    let prefix_length = path_len(prefix);
    if prefix_length == 0 then {
        node_to_ref(childnode)
    } else if childnode.len == 0 then {
        EmptyRef()
    } else {
        let decoded = decode_input_trie_node(childnode);
        match decoded {
            InputLeafNode(path, value) => {
                let merged_path = path_concat(prefix, path);
                input_leaf_child_ref(merged_path, value)
            },
            InputExtensionNode(path, child) => {
                let merged_path = path_concat(prefix, path);
                extension_child_ref(merged_path, child)
            },
            _ => {
                let childref = node_to_ref(childnode);
                extension_child_ref(prefix, childref)
            },
        }
    }
}

/*! [merge_ext_node][] over a child reference: an inline reference
    carries its node bytes and re-keys canonically; a 32-byte hash
    reference is wrapped in an extension, which is canonical only when the
    referenced node is a branch. */
function merge_ext_ref(prefix : TriePath, childref : NodeRef) -> NodeRef = {
    let prefix_length = path_len(prefix);
    if prefix_length == 0 then {
        childref
    } else {
        match childref {
            EmptyRef() => EmptyRef(),
            HashRef(_) => extension_child_ref(prefix, childref),
            InputInlineRef(node) => {
                let decoded = decode_input_trie_node(node);
                match decoded {
                    InputLeafNode(path, value) => {
                        let merged_path = path_concat(prefix, path);
                        input_leaf_child_ref(merged_path, value)
                    },
                    InputExtensionNode(path, child) => {
                        let merged_path = path_concat(prefix, path);
                        extension_child_ref(merged_path, child)
                    },
                    _ => extension_child_ref(prefix, childref),
                }
            },
            ScratchInlineRef(node) => {
                let node_slice = inline_node_slice(node);
                let decoded = decode_scratch_trie_node(node_slice);
                match decoded {
                    ScratchLeafNode(path, value) => {
                        let merged_path = path_concat(prefix, path);
                        scratch_leaf_child_ref(merged_path, value)
                    },
                    ScratchExtensionNode(path, child) => {
                        let merged_path = path_concat(prefix, path);
                        extension_child_ref(merged_path, child)
                    },
                    _ => extension_child_ref(prefix, childref),
                }
            },
        }
    }
}
