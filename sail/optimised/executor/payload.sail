/* Optimized C overrides for `sail/executor/payload.sail`. */

val optimized_execution_requests_hash = impure { c: "execution_requests_hash" } : StatelessInputRef -> hash

function execution_requests_hash(input_ref : StatelessInputRef) -> hash =
    optimized_execution_requests_hash(input_ref)

/*
 * All ordered protocol tries use the same optimized fixed-stack builder as
 * state/storage roots. Transaction and withdrawal roots cross as complete
 * operations so optimized C can consume their already validated SSZ list
 * references directly. The standard model retains the explicit RLP(index)
 * cursor, withdrawal encoding, and recursive subtree equations.
 */
val optimized_transaction_trie_root = impure { c: "transaction_trie_root" } : TransactionListRef -> hash
val optimized_withdrawals_trie_root = impure { c: "withdrawals_trie_root" } : WithdrawalListRef -> hash

function transaction_trie_root(txs : TransactionListRef) -> hash =
    optimized_transaction_trie_root(txs)

function withdrawals_trie_root(wds : WithdrawalListRef) -> hash =
    optimized_withdrawals_trie_root(wds)
