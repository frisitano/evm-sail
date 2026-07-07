/* Spec backend: MPT node encode + hash as canonical byte lists. The 17-item
 * branch node is composed and hashed via child_ref exactly as the Yellow Paper
 * defines it (lib/mpt.sail helpers); the build backend refines this to a
 * C-side node assembly (build/mpt.sail). */

function branch_child_ref(mask : bits(16), children : list(list(byte))) -> list(byte) =
    child_ref(branch_node_rlp(mask, children))

function extension_child_ref(key : list(nibble), childref : list(byte)) -> list(byte) =
    child_ref(extension_node_rlp(key, childref))
