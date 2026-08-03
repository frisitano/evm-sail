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
Hash32
mpt_transaction_trie_root(struct BoundedSszListRef transactions);
Hash32
mpt_withdrawals_trie_root(struct BoundedSszListRef withdrawals);

/*
 * Internal authenticated point readers used by the complete optimized state
 * operations. The standard model retains the explicit trie and RLP equations.
 */
uint64_t stateless_account_read(
    Hash32 root,
    Hash32 address_hash, struct AccountInfo *info,
    bool *found, NodeId *terminal_node, NodeId *storage_root_node);
uint64_t stateless_storage_read(
    NodeId root_node, Hash32 slot_hash,
    U256 *value, bool *found, NodeId *terminal_node);

/* Recover the canonical digest represented by a storage-root NodeId. */
Hash32 mpt_storage_root_hash(NodeId root_node);

/* Reusable state-binding ordering. Account bindings are permuted once inside
 * AccountTable; storage IDs remain stable and are exposed through the shared
 * MPT order workspace for the duration of one storage-trie traversal. */
uint32_t mpt_account_updates_prepare(void);
uint32_t mpt_storage_updates_prepare(Address address);
StorageId mpt_storage_update_id_at(uint32_t index);

/* One-call optimized indexing of the source-backed SSZ witness-node list. */
unit mpt_index_witness_nodes(struct BoundedSszListRef nodes);
unit mpt_index_witness_codes(struct BoundedSszListRef codes,
                                     bool amsterdam_or_later);

/* Reset the reusable ordered-trie builder before a new independent root. */
unit mpt_reset(unit ignored);

/* Receipt ownership lives in executor/receipts.c. The MPT consumes that
 * module's immutable record stream and owns only ordering/construction. */
Hash32
mpt_receipt_table_root(uint64_t count);

Hash32
mpt_compute_state_root(
    Hash32 parent_state_root);
Hash32 mpt_last_state_root(void);

#if defined(EVMSAIL_NATIVE_DEBUG_AGGREGATES)
/*
 * Native-only differential checks for the bottom-up state-trie reducer.
 * Each numbered case compares an incremental reduction with a canonical
 * rebuild of the same small synthetic trie.
 */
uint64_t mpt_selftest(uint64_t scenario);
#endif

#endif
