/* C-backed account and persistent-storage state database for evm-sail.
 *
 * One account table is keyed by raw address and one storage table by raw
 * (address, slot). Each row carries cumulative block state, an optional
 * transaction projection, and BAL metadata. Storage generations are lifecycle
 * metadata rather than row identity. Cached secure trie hashes are derived
 * once when witness material is first loaded and used only for MPT traversal. */
#ifndef STATE_DB_H
#define STATE_DB_H
#include "quantity_abi.h"
#include "sail_abi.h"
#include <stdbool.h>
#include <stdint.h>

/* Generated account/storage adapters implemented by journal_glue.c. The Sail
 * operations already live in host/state.sail; these declarations only fix the
 * generated C calling convention for aggregate arguments and results. */
struct zStorageTxLookup;
struct zoptionzIRStorageValuezK;
struct zoptionzIRStorageEntryzK;
struct zoptionzIRStorageTrieEntryzK;
struct zStorageKey;
struct zStorageEntry;
struct zBalIterEntry;
void storage_tx_get(struct zStorageTxLookup *result, struct zStorageKey key);
void storage_block_get(struct zoptionzIRStorageValuezK *result,
                       struct zStorageKey key);
void storage_block_iter_next(struct zoptionzIRStorageTrieEntryzK *result,
                             sail_address address);
unit storage_tx_update(struct zStorageEntry entry);
unit storage_block_put(struct zStorageEntry entry);
unit storage_block_cache(struct zStorageKey key, sail_hash slot_hash,
                         EVMSAIL_WORD_PARAM(value));

struct zoptionzIRAccountzK;
struct zoptionzIRAcctEntryzK;
struct zoptionzIRAcctTrieEntryzK;
struct zAccount;
struct zAcctEntry;
void acct_tx_get(struct zoptionzIRAccountzK *result, sail_address address);
void acct_block_get(struct zoptionzIRAccountzK *result, sail_address address);
void acct_block_iter_next(struct zoptionzIRAcctTrieEntryzK *result, unit u);
void acct_debug_iter_next(struct zoptionzIRAcctTrieEntryzK *result, unit u);
unit acct_tx_update(sail_address address, struct zAccount account);
unit acct_block_write(struct zAcctEntry entry);
unit acct_block_cache(sail_address address, sail_hash address_hash,
                      struct zAccount account);

void storage_tx_pop(struct zoptionzIRStorageEntryzK *result, unit u);
void acct_tx_pop(struct zoptionzIRAcctEntryzK *result, unit u);

/* Shared persistent-storage rows with transaction and cumulative projections. */
unit storage_db_reset(const unit u);
/* Per-layer row probe (layer 0 = tx writes, 1 = block cache/update map). */
uint64_t storage_row_probe(uint64_t layer, sail_address a, sail_word s,
                           sail_word *cur, sail_word *orig);
unit storage_tx_update_raw(sail_address a, sail_word s, sail_word v,
                           sail_word orig);
void storage_secure_key(sail_word slot, sail_hash *slot_hash);
uint64_t storage_tx_checkpoint(const unit u);
unit storage_tx_revert(uint64_t checkpoint);
unit storage_tx_clear(sail_address a);
unit storage_tx_reset(const unit u);
bool storage_has_writes(sail_address a);
unit storage_block_clear(sail_address a);

/* Debug snapshot enumeration over the cumulative storage table, grouped by
   keccak(address). This is not part of the Sail state-root interface. */
uint64_t storage_dump_count(sail_word ak);
sail_word storage_dump_slot(sail_word ak, uint64_t j);
sail_word storage_dump_value(sail_word ak, uint64_t j);

/* Shared account rows with transaction and cumulative projections. */
unit acct_db_reset(const unit u);
/* per-layer account probe (layer 0 = tx, 1 = block): 0 = absent, 1 = present.
   Transaction-only fields are zero for block rows. */
uint64_t acct_row_probe(uint64_t layer, sail_address a, uint64_t *nonce,
                        sail_word *bal, sail_hash *sroot, sail_hash *chash,
                        bool *exists, bool *storage_cleared,
                        bool *created, bool *selfdestructed);
unit acct_tx_update_raw(sail_address a, uint64_t nonce,
                        sail_word bal, sail_hash sroot, sail_hash chash,
                        bool exists, bool storage_cleared, bool created,
                        bool selfdestructed);
void acct_secure_key(sail_address address, sail_hash *address_hash);
unit acct_tx_set_balance(sail_address a, EVMSAIL_WORD_PARAM(balance));
unit acct_tx_set_nonce(sail_address a, uint64_t nonce);
unit acct_tx_set_code_hash(sail_address a, sail_hash code_hash);
uint64_t acct_tx_checkpoint(const unit u);
unit acct_tx_revert(uint64_t checkpoint);
unit acct_tx_reset(const unit u);
/* Optimized whole-operation transaction merge. The standard build retains
 * the explicit Sail account-then-storage loops. */
unit evmsail_tx_merge(bool cancun_or_later, bool amsterdam_or_later,
                      uint64_t block_access_index);

/* Debug snapshot enumeration over cumulative account state. */
uint64_t acct_dump_count(const unit u);
sail_word acct_dump_hkey(uint64_t i);
sail_address acct_dump_address(uint64_t i);
uint64_t acct_dump_nonce(uint64_t i);
sail_word acct_dump_balance(uint64_t i);
sail_word acct_dump_storage_root(uint64_t i);
sail_word acct_dump_code_hash(uint64_t i);

/* EIP-7928 metadata shares the cumulative raw-key account/storage rows.
 * Sail owns canonical validation and consumes a non-destructive canonical
 * stream through generated-layout glue. */
unit bal_reset(const unit u);
unit bal_prepare_iter(const unit u);
void bal_iter_next(struct zBalIterEntry *result, unit u);
enum bal_iter_tag {
  BAL_ITER_EMPTY = 0,
  BAL_ITER_ACCOUNT = 1,
  BAL_ITER_STORAGE_CHANGE = 2,
  BAL_ITER_STORAGE_READ = 3,
  BAL_ITER_BALANCE_CHANGE = 4,
  BAL_ITER_NONCE_CHANGE = 5,
  BAL_ITER_CODE_CHANGE = 6,
  BAL_ITER_ACCOUNT_END = 7
};
enum bal_iter_tag bal_iter_next_probe(sail_address *address, sail_word *slot,
                                      uint64_t *index, sail_word *value,
                                      uint64_t *nonce, sail_hash *code_hash);

/* EIP-7928 record sinks and transaction-row enumeration. */
unit bal_note_storage_change(uint64_t index, sail_address a,
                             EVMSAIL_WORD_PARAM(slot),
                             EVMSAIL_WORD_PARAM(val));
unit bal_note_account_touch(sail_address a);
unit bal_note_storage_read(sail_address a, EVMSAIL_WORD_PARAM(slot));
unit bal_note_balance_change(uint64_t index, sail_address a,
                             EVMSAIL_WORD_PARAM(val));
unit bal_note_nonce_change(uint64_t index, sail_address a, uint64_t nonce);
unit bal_note_code_change(uint64_t index, sail_address a, sail_hash chash);
/* Non-destructive ascending state-root iterators over the cumulative maps.
 * The indexed accessors expose the same prepared views directly to the
 * optimized C root builder, without routing its private cursors through the
 * global iterator state used by generated standard C. */
uint32_t storage_block_updates_prepare(sail_address a);
uint64_t storage_block_update_probe_at(uint32_t index, sail_word *slot,
                                       sail_word *curr, sail_word *orig,
                                       sail_hash *address_hash,
                                       sail_hash *slot_hash);
unit storage_block_iter_begin(sail_address a);
uint64_t storage_block_iter_next_probe(sail_address a, sail_word *slot,
                                       sail_word *curr, sail_word *orig,
                                       sail_hash *address_hash,
                                       sail_hash *slot_hash);
uint32_t acct_block_updates_prepare(void);
uint64_t acct_block_update_probe_at(uint32_t index, sail_address *addr,
                                    uint64_t *cn, sail_word *cb,
                                    sail_hash *cs, sail_hash *cc, bool *ce,
                                    bool *csc, bool *ccr, bool *csd,
                                    uint64_t *on, sail_word *ob,
                                    sail_hash *os, sail_hash *oc, bool *oe,
                                    bool *osc, bool *ocr, bool *osd,
                                    sail_hash *address_hash,
                                    sail_hash *post_storage_root);
unit acct_block_update_post_storage_store_at(uint32_t index, sail_hash root);
unit acct_block_iter_begin(const unit u);
uint64_t acct_block_iter_next_probe(sail_address *addr, uint64_t *cn,
                                    sail_word *cb, sail_hash *cs,
                                    sail_hash *cc, bool *ce, bool *csc,
                                    bool *ccr, bool *csd, uint64_t *on,
                                    sail_word *ob, sail_hash *os,
                                    sail_hash *oc, bool *oe, bool *osc,
                                    bool *ocr, bool *osd,
                                    sail_hash *address_hash);
unit acct_debug_iter_begin(const unit u);
uint64_t acct_debug_iter_next_probe(sail_address *addr, uint64_t *cn,
                                    sail_word *cb, sail_hash *cs,
                                    sail_hash *cc, bool *ce, bool *csc,
                                    bool *ccr, bool *csd, uint64_t *on,
                                    sail_word *ob, sail_hash *os,
                                    sail_hash *oc, bool *oe, bool *osc,
                                    bool *ocr, bool *osd,
                                    sail_hash *address_hash);
unit acct_post_storage_root_store(sail_address a, sail_hash root);
EVMSAIL_HASH_RETURN acct_post_storage_root_read(
    EVMSAIL_HASH_RESULT(result) sail_address a);
/* k_tx_merge drain pops (side-effect-free) + block propagation hooks */
uint64_t storage_tx_pop_probe(sail_address *addr, sail_word *slot,
                              sail_word *curr, sail_word *orig);
unit storage_block_put_raw(sail_address a, sail_word s_, sail_word curr,
                           sail_word orig);
unit storage_block_cache_raw(sail_address a, sail_word s_, sail_hash slot_hash,
                             sail_word v);
uint64_t acct_tx_pop_probe(sail_address *addr,
                           uint64_t *cn, sail_word *cb,
                           sail_hash *cs, sail_hash *cc,
                           bool *ce, bool *csc, bool *ccr, bool *csd,
                           uint64_t *on, sail_word *ob,
                           sail_hash *os, sail_hash *oc,
                           bool *oe, bool *osc, bool *ocr, bool *osd);
unit acct_block_write_raw(sail_address a, uint64_t nonce, sail_word bal,
                          sail_hash sroot, sail_hash chash,
                          bool exists, bool storage_cleared,
                          uint64_t ononce, sail_word obal,
                          sail_hash osroot, sail_hash ochash,
                          bool oexists, bool ostorage_cleared);
unit acct_block_cache_raw(sail_address a, sail_hash address_hash,
                          uint64_t nonce, sail_word bal,
                          sail_hash sroot, sail_hash chash,
                          bool exists, bool storage_cleared);
#endif
