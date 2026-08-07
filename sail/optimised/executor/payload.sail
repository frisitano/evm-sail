/* Optimized C overrides for `sail/executor/payload.sail`. */

val execution_requests_hash_from_input = impure { c: "execution_requests_hash" } : StatelessInputRef -> hash

function execution_requests_hash(input_ref : StatelessInputRef) -> hash =
    execution_requests_hash_from_input(input_ref)

/*
 * All ordered protocol tries use the same optimized fixed-stack builder as
 * state/storage roots. Transaction and withdrawal roots cross as complete
 * operations so optimized C can consume their already validated SSZ list
 * references directly. The standard model retains the explicit RLP(index)
 * cursor, withdrawal encoding, and recursive subtree equations.
 */
val transaction_trie_root_from_items = impure { c: "transaction_trie_root" } : TransactionListRef -> hash
val withdrawals_trie_root_from_items = impure { c: "withdrawals_trie_root" } : WithdrawalListRef -> hash

function transaction_trie_root(txs : TransactionListRef) -> hash =
    transaction_trie_root_from_items(txs)

function withdrawals_trie_root(wds : WithdrawalListRef) -> hash =
    withdrawals_trie_root_from_items(wds)
