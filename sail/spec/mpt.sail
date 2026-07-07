/* Spec backend: MPT node encode + hash as canonical byte lists. The 17-item
 * branch node is composed and hashed via child_ref exactly as the Yellow Paper
 * defines it (lib/mpt.sail helpers); the build backend refines this to a
 * C-side node assembly (build/mpt.sail). */

function branch_child_ref(mask : bits(16), children : list(list(byte))) -> list(byte) =
    child_ref(branch_node_rlp(mask, children))

function extension_child_ref(key : list(nibble), childref : list(byte)) -> list(byte) =
    child_ref(extension_node_rlp(key, childref))

/* leaf child ref: compose rlp([hp(key,leaf), value]) and hash it. The value is
 * materialized from its byte source via the read_byte_slice axiom -- for update
 * leaves that source is the value arena, so read_byte_slice(arena_intern(l)) = l
 * (the arena read/write law) recovers the canonical account/storage RLP. */
function leaf_child_ref(key : list(nibble), value : ByteSlice) -> list(byte) =
    child_ref(leaf_node_rlp(key, read_byte_slice(value)))

/* update-leaf values: the canonical RLP interned into the value arena. account:
 * rlp([nonce, balance, storageRoot, codeHash]); storage: rlp_int(value). */
function account_leaf_value(nonce : int, balance : word, sroot : hash, chash : hash) -> ByteSlice =
    arena_intern(account_rlp(nonce, balance, sroot, chash))

function storage_leaf_value(value : word) -> ByteSlice =
    arena_intern(rlp_int(unsigned(value)))

/* the arena is a build-side C buffer; in spec it is the abstract write side of
 * the byte-source interface (arena_intern, an axiom) and needs no reset. */
function trie_arena_reset() -> unit = ()
