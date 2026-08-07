#ifndef EVMSAIL_OPTIMIZED_LIB_MPT_TRIE_H
#define EVMSAIL_OPTIMIZED_LIB_MPT_TRIE_H

#include "evmsail/host/nodes.h"
#include "evmsail/host/state/primitives.h"
#include "evmsail/prelude.h"

/*
 * Optimized-C refinement of the explicit Sail MPT equations. This header and
 * its implementation are injected only into optimized C builds; standard C
 * and proof extraction continue to use the Sail bodies.
 */
struct BoundedSszListRef;
struct AccountInfo;
struct TriePath;

/*
 * Whole-operation ordered payload tries. C owns SSZ item navigation,
 * RLP(index) ordering, leaf framing, and use of the shared MPT workspace.
 */
bytes32 mpt_transaction_trie_root(struct BoundedSszListRef transactions);
bytes32 mpt_withdrawals_trie_root(struct BoundedSszListRef withdrawals);

/*
 * Internal authenticated point readers used by the complete optimized state
 * operations. The standard model retains the explicit trie and RLP equations.
 */
void stateless_account_read(bytes32 root, bytes32 address_hash, struct AccountInfo *info,
                            bool *found, NodeId *terminal_node, NodeId *storage_root_node);
void stateless_storage_read(NodeId root_node, bytes32 slot_hash, u256 *value, bool *found,
                            NodeId *terminal_node);

/* Reusable state-binding ordering. Account bindings are permuted once inside
 * AccountTable; storage IDs remain stable and are exposed through the shared
 * MPT order workspace for the duration of one storage-trie traversal. */
uint32_t mpt_account_updates_prepare(void);
uint32_t mpt_storage_updates_prepare(AccountId account_id, StorageGeneration *generation);
StorageId mpt_storage_update_id_at(uint32_t index);

/* One-call optimized indexing of the source-backed SSZ witness-node list. */
void mpt_index_witness_nodes(struct BoundedSszListRef nodes);
void mpt_index_witness_codes(struct BoundedSszListRef codes, bool amsterdam_or_later);

/* Reset the reusable ordered-trie builder before a new independent root. */
void mpt_reset(void);

/* Receipt ownership lives in executor/receipts.c. The MPT consumes that
 * module's immutable record stream and owns only ordering/construction. */
bytes32 mpt_receipt_table_root(uint64_t count);

bytes32 mpt_compute_state_root(bytes32 parent_state_root);
#ifdef EVMSAIL_NATIVE_TEST
/* Diagnostic latch of the most recent state root (src/test/debug.c). */
bytes32 mpt_last_state_root(void);
#endif

#endif
