#ifndef EVMSAIL_MPT_GLUE_H
#define EVMSAIL_MPT_GLUE_H

#include "sail_abi.h"

/*
 * Optimized-C refinement of the explicit Sail MPT equations. This header and
 * its implementation are injected only into optimized C builds; standard C
 * and proof extraction continue to use the Sail bodies.
 */
struct zByteSliceFields;
struct zBoundedSszzListRef;
struct zAccountInfo;
struct zTriePath;
struct zOptimizzedHashResult;
struct zOptimizzedUnitResult;

unit evmsail_mpt_reset(unit);
void evmsail_mpt_insert_leaf(struct zOptimizzedUnitResult *result,
                             struct zTriePath path,
                             struct zByteSliceFields value,
                             struct zTriePath next_path);
void evmsail_mpt_insert_last(struct zOptimizzedUnitResult *result,
                             struct zTriePath path,
                             struct zByteSliceFields value);
void evmsail_mpt_root(struct zOptimizzedHashResult *result, unit);

/*
 * Whole-operation ordered payload tries. C owns SSZ item navigation,
 * RLP(index) ordering, leaf framing, and use of the shared MPT workspace.
 */
void evmsail_transaction_trie_root(
    struct zOptimizzedHashResult *result,
    struct zBoundedSszzListRef transactions);
void evmsail_withdrawals_trie_root(
    struct zOptimizzedHashResult *result,
    struct zBoundedSszzListRef withdrawals);

/*
 * Whole-operation authenticated point reads. The standard model keeps the
 * explicit TriePath/trie_lookup/RLP equations; optimized C walks input-backed
 * witness nodes directly and returns one decoded protocol value.
 */
void evmsail_stateless_account_lookup(struct zOptimizzedUnitResult *result,
                                      sail_hash root,
                                      sail_hash address_hash);
uint64_t evmsail_stateless_account_read(sail_hash root,
                                        sail_hash address_hash,
                                        struct zAccountInfo *info,
                                        bool *found);
bool evmsail_stateless_account_found(unit);
uint64_t evmsail_stateless_account_nonce(unit);
EVMSAIL_WORD_RETURN evmsail_stateless_account_balance(
    EVMSAIL_WORD_RESULT(result) unit);
EVMSAIL_HASH_RETURN evmsail_stateless_account_storage_root(
    EVMSAIL_HASH_RESULT(result) unit);
EVMSAIL_HASH_RETURN evmsail_stateless_account_code_hash(
    EVMSAIL_HASH_RESULT(result) unit);

void evmsail_stateless_storage_lookup(struct zOptimizzedUnitResult *result,
                                      sail_hash root, sail_hash slot_hash);
uint64_t evmsail_stateless_storage_read(sail_hash root,
                                        sail_hash slot_hash,
                                        sail_word *value);
EVMSAIL_WORD_RETURN evmsail_stateless_storage_value(
    EVMSAIL_WORD_RESULT(result) unit);

/* One-call optimized indexing of the source-backed SSZ witness-node list. */
void evmsail_index_witness_nodes(struct zOptimizzedUnitResult *result,
                                 struct zBoundedSszzListRef nodes);
void evmsail_index_witness_codes(struct zOptimizzedUnitResult *result,
                                 struct zBoundedSszzListRef codes,
                                 bool amsterdam_or_later);

/*
 * Optimized receipt source. Receipts are appended in execution order under
 * their numeric index. The high-level root operation owns RLP(index) key
 * generation, canonical ordering, and MPT construction.
 */
unit evmsail_receipt_table_reset(unit);
void evmsail_receipt_table_push(struct zOptimizzedUnitResult *result,
                                uint64_t index,
                                struct zByteSliceFields value);
void evmsail_receipt_table_root(struct zOptimizzedHashResult *result,
                                uint64_t count);

void evmsail_compute_state_root(struct zOptimizzedHashResult *result,
                                sail_hash parent_state_root);

#endif
