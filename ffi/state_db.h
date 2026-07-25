/* C-backed account and persistent-storage state database for evm-sail.
 *
 * Rows are stored sorted by keccak256(address) (accounts) and
 * (keccak256(address), keccak256(slot), generation) (storage), but the point-access hooks
 * are keyed by raw address/slot: the secure trie hashes are computed and
 * memoized C-side. Both stores are cache/update backed. */
#ifndef STATE_DB_H
#define STATE_DB_H
#include "quantity_abi.h"
#include "sail_abi.h"
#include <stdbool.h>
#include <stdint.h>

/* Generated account/storage adapters implemented by journal_glue.c. The Sail
 * operations already live in host/state.sail; these declarations only fix the
 * generated C calling convention for aggregate arguments and results. */
struct zoptionzIRStorageValuezK;
struct zoptionzIRStorageEntryzK;
struct zStorageKey;
struct zStorageEntry;
void storage_tx_get(struct zoptionzIRStorageValuezK *result,
                    struct zStorageKey key);
void storage_block_get(struct zoptionzIRStorageValuezK *result,
                       struct zStorageKey key);
void storage_block_iter_next(struct zoptionzIRStorageEntryzK *result,
                             sail_address address);
unit storage_tx_update(struct zStorageEntry entry);
unit storage_block_put(struct zStorageEntry entry);
unit storage_block_cache(struct zStorageKey key, EVMSAIL_WORD_PARAM(value));

struct zoptionzIRAccountzK;
struct zoptionzIRAcctEntryzK;
struct zAccount;
struct zAcctEntry;
void acct_tx_get(struct zoptionzIRAccountzK *result, sail_address address);
void acct_block_get(struct zoptionzIRAccountzK *result, sail_address address);
void acct_block_iter_next(struct zoptionzIRAcctEntryzK *result, unit u);
void acct_debug_iter_next(struct zoptionzIRAcctEntryzK *result, unit u);
unit acct_tx_update(sail_address address, struct zAccount account);
unit acct_block_write(struct zAcctEntry entry);
unit acct_block_cache(sail_address address, struct zAccount account);

void storage_tx_pop(struct zoptionzIRStorageEntryzK *result, unit u);
void acct_tx_pop_ascending(struct zoptionzIRAcctEntryzK *result, unit u);

/* Persistent storage transaction state and cumulative block state. */
unit storage_db_reset(const unit u);
/* Per-layer row probe (layer 0 = tx writes, 1 = block cache/update map). */
uint64_t storage_row_probe(uint64_t layer, sail_address a, sail_word s,
                           sail_word *cur, sail_word *orig);
unit storage_tx_update_raw(sail_address a, sail_word s, sail_word v,
                           sail_word orig);
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

/* Account transaction state and cumulative block state. */
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
unit acct_tx_set_balance(sail_address a, EVMSAIL_WORD_PARAM(balance));
unit acct_tx_set_nonce(sail_address a, uint64_t nonce);
unit acct_tx_set_code_hash(sail_address a, sail_hash code_hash);
uint64_t acct_tx_checkpoint(const unit u);
unit acct_tx_revert(uint64_t checkpoint);
unit acct_tx_reset(const unit u);

/* Debug snapshot enumeration over cumulative account state. */
uint64_t acct_dump_count(const unit u);
sail_word acct_dump_hkey(uint64_t i);
sail_address acct_dump_address(uint64_t i);
uint64_t acct_dump_nonce(uint64_t i);
sail_word acct_dump_balance(uint64_t i);
sail_word acct_dump_storage_root(uint64_t i);
sail_word acct_dump_code_hash(uint64_t i);

/* EIP-7928 record store. Sail owns canonical validation and consumes these
 * non-destructive sorted cursors through generated-layout glue. */
unit bal_reset(const unit u);
unit bal_prepare(const unit u);
uint64_t bal_account_next_probe(sail_address *address);
uint64_t bal_storage_slot_next_probe(sail_word *slot, uint64_t *has_change,
                                     uint64_t *index, sail_word *value);
uint64_t bal_storage_change_next_probe(uint64_t *index, sail_word *value);
uint64_t bal_balance_change_next_probe(uint64_t *index, sail_word *value);
uint64_t bal_nonce_change_next_probe(uint64_t *index, uint64_t *value);
uint64_t bal_code_change_next_probe(uint64_t *index, sail_hash *code_hash);

/* EIP-7928 record sinks + tx-row enumeration (harvest logic is Sail-side) */
unit bal_note_storage_change(uint64_t index, sail_address a,
                             EVMSAIL_WORD_PARAM(slot),
                             EVMSAIL_WORD_PARAM(val));
unit bal_note_account_touch(sail_address a);
unit bal_note_storage_read(sail_address a, EVMSAIL_WORD_PARAM(slot));
unit bal_note_balance_change(uint64_t index, sail_address a,
                             EVMSAIL_WORD_PARAM(val));
unit bal_note_nonce_change(uint64_t index, sail_address a, uint64_t nonce);
unit bal_note_code_change(uint64_t index, sail_address a, sail_hash chash);
/* Non-destructive ascending state-root iterators over the cumulative maps. */
unit storage_block_iter_begin(sail_address a);
uint64_t storage_block_iter_next_probe(sail_address a, sail_word *slot,
                                       sail_word *curr, sail_word *orig);
unit acct_block_iter_begin(const unit u);
uint64_t acct_block_iter_next_probe(sail_address *addr, uint64_t *cn,
                                    sail_word *cb, sail_hash *cs,
                                    sail_hash *cc, bool *ce, bool *csc,
                                    bool *ccr, bool *csd, uint64_t *on,
                                    sail_word *ob, sail_hash *os,
                                    sail_hash *oc, bool *oe, bool *osc,
                                    bool *ocr, bool *osd);
unit acct_debug_iter_begin(const unit u);
uint64_t acct_debug_iter_next_probe(sail_address *addr, uint64_t *cn,
                                    sail_word *cb, sail_hash *cs,
                                    sail_hash *cc, bool *ce, bool *csc,
                                    bool *ccr, bool *csd, uint64_t *on,
                                    sail_word *ob, sail_hash *os,
                                    sail_hash *oc, bool *oe, bool *osc,
                                    bool *ocr, bool *osd);
unit acct_post_storage_root_store(sail_address a, sail_hash root);
EVMSAIL_HASH_RETURN acct_post_storage_root_read(
    EVMSAIL_HASH_RESULT(result) sail_address a);
/* k_tx_merge drain pops (side-effect-free) + block propagation hooks */
uint64_t storage_tx_pop_probe(sail_address *addr, sail_word *slot,
                              sail_word *curr, sail_word *orig);
unit storage_block_put_raw(sail_address a, sail_word s_, sail_word curr,
                           sail_word orig);
unit storage_block_cache_raw(sail_address a, sail_word s_, sail_word v);
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
unit acct_block_cache_raw(sail_address a, uint64_t nonce, sail_word bal,
                          sail_hash sroot, sail_hash chash,
                          bool exists, bool storage_cleared);
#endif
