/* Optimized C overrides for `sail/lib/rlp/codecs/block_header.sail`. */

val block_header_hash_with_roots = impure { c: "block_header_hash" } : (BlockHeader, hash, hash, hash, hash) -> hash

function block_header_hash(
    header : BlockHeader,
    transactions_root : hash,
    withdrawals_root : hash,
    requests_hash : hash,
    block_access_list_hash : hash,
) -> (
    hash
) =
    block_header_hash_with_roots(header, transactions_root, withdrawals_root, requests_hash, block_access_list_hash)
