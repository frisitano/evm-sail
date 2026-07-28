#ifndef EVMSAIL_MPT_GLUE_H
#define EVMSAIL_MPT_GLUE_H

#include "sail_abi.h"

/*
 * Optimized-C refinement of the explicit Sail MPT equations. This header and
 * its implementation are injected only into optimized C builds; standard C
 * and proof extraction continue to use the Sail bodies.
 */
struct zScratchSliceFields;
struct zBoundedSszzListRef;
struct zAccountInfo;
struct zTriePath;

/*
 * Whole-operation ordered payload tries. C owns SSZ item navigation,
 * RLP(index) ordering, leaf framing, and use of the shared MPT workspace.
 */
sail_hash evmsail_transaction_trie_root(
    struct zBoundedSszzListRef transactions);
sail_hash evmsail_withdrawals_trie_root(
    struct zBoundedSszzListRef withdrawals);

/*
 * Internal authenticated point readers used by the complete optimized state
 * operations. The standard model retains the explicit trie and RLP equations.
 */
uint64_t evmsail_stateless_account_read(sail_hash root,
                                        sail_hash address_hash,
                                        struct zAccountInfo *info,
                                        bool *found);
uint64_t evmsail_stateless_storage_read(sail_hash root,
                                        sail_hash slot_hash,
                                        sail_word *value);

/* One-call optimized indexing of the source-backed SSZ witness-node list. */
unit evmsail_index_witness_nodes(struct zBoundedSszzListRef nodes);
unit evmsail_index_witness_codes(struct zBoundedSszzListRef codes,
                                 bool amsterdam_or_later);

/*
 * Optimized receipt source. Receipts are appended in execution order under
 * their numeric index. The high-level root operation owns RLP(index) key
 * generation, canonical ordering, and MPT construction.
 */
unit evmsail_receipt_table_reset(unit);
unit evmsail_receipt_table_push(uint64_t index,
                                struct zScratchSliceFields value);
sail_hash evmsail_receipt_table_root(uint64_t count);

sail_hash evmsail_compute_state_root(sail_hash parent_state_root);

#endif
