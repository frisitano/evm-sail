/* An inline MPT child is at most 31 bytes. Keep it self-contained so a node
 * reference never borrows scratch memory or retains a heap byte list. `data`
 * is the big-endian payload in the low bits. */

/*md
# Trie node types

Merkle-Patricia trie node forms, references, and decoding
(YP Appendix D).
*/
/*! A self-contained trie-node encoding shorter than 32 bytes. */
struct InlineNode = {
    data : b256,
    len : range(0, 31),
}

let MPT_HASH_LENGTH : int(32) = WORD_BYTE_LENGTH

/*! A leaf value retained by trie assembly. Authenticated witness and
    transaction leaves borrow immutable input bytes; newly encoded state,
    receipt, and withdrawal leaves borrow the scratch arena. */
union TrieLeafValue = {
    InputTrieLeaf : StatelessInputSlice,
    ScratchTrieLeaf : ScratchSlice,
}

/*! Copies a sub-32-byte scratch node encoding into an inline node value. */
function inline_node_from_scratch_slice(bytes : ScratchSlice) -> InlineNode = {
    let length = bytes.len;
    if MPT_HASH_LENGTH <= length then {
        throw InvalidBlock(WitnessDeficient)
    };
    struct { data = word_to_hash(slice_load(bytes, 0)), len = length }
}

function inline_node_from_input_slice(bytes : StatelessInputSlice) -> InlineNode = {
    let length = bytes.len;
    if MPT_HASH_LENGTH <= length then {
        throw InvalidBlock(WitnessDeficient)
    };
    struct { data = word_to_hash(slice_load(bytes, 0)), len = length }
}

overload inline_node_from_slice = {inline_node_from_scratch_slice, inline_node_from_input_slice}

/*! Materializes an inline node in scratch memory as a byte slice. */
function inline_node_slice(node : InlineNode) -> ScratchSlice = {
    let start = scratch_reserve(node.len);
    scratch_push_b256(node.data, node.len);
    scratch_finish(start)
}

/*! Hashes an inline node from its existing scratch representation. */
function inline_node_hash(node : InlineNode) -> hash = {
    let mark = scratch_begin();
    let encoded = inline_node_slice(node);
    let digest = keccak256(encoded);
    scratch_rewind(mark);
    digest
}

/*! A reference to a trie node: empty, inline (encodings under 32 bytes),
    or by KECCAK-256 hash (YP Appendix D, Eq. 207). */
union NodeRef = {
    /* the empty node */
    EmptyRef : unit,
    /* an authenticated node embedded in witness input */
    InputInlineRef : StatelessInputSliceAtMost(31),
    /* a freshly encoded node embedded in a generated parent */
    ScratchInlineRef : InlineNode,
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
