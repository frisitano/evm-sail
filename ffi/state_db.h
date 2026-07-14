/* C-backed account and persistent-storage state database for evm-sail.
 *
 * Rows are stored sorted by keccak256(address) (accounts) and
 * (keccak256(address), keccak256(slot), generation) (storage), but the point-access hooks
 * are keyed by raw address/slot: the secure trie hashes are computed and
 * memoized C-side. Both stores are cache/update backed. */
#ifndef STATE_DB_H
#define STATE_DB_H
#include "sail.h"
#include <stdbool.h>
#include <stdint.h>

/* Persistent storage transaction state and cumulative block state. */
unit storage_db_reset(const unit u);
/* Per-layer row probe (layer 0 = tx writes, 1 = block cache/update map). */
uint64_t storage_row_probe(uint64_t layer, const lbits a, const lbits s,
                           lbits *cur, lbits *orig);
unit storage_tx_update_raw(const lbits a, const lbits s, const lbits v,
                           const lbits orig);
uint64_t storage_tx_checkpoint(const unit u);
unit storage_tx_revert(uint64_t checkpoint);
unit storage_tx_clear(const lbits a);
unit storage_tx_reset(const unit u);
bool storage_has_writes(const lbits a);
unit storage_block_clear(const lbits a);

/* Debug snapshot enumeration over the cumulative storage table, grouped by
   keccak(address). This is not part of the Sail state-root interface. */
uint64_t storage_dump_count(const lbits ak);
void storage_dump_slot(lbits *rop, const lbits ak, uint64_t j);
void storage_dump_value(lbits *rop, const lbits ak, uint64_t j);

/* Account transaction state and cumulative block state. */
unit acct_db_reset(const unit u);
/* per-layer account probe (layer 0 = tx, 1 = block): 0 = absent, 1 = present.
   Transaction-only fields are zero for block rows. */
uint64_t acct_row_probe(uint64_t layer, const lbits a, uint64_t *nonce,
                        lbits *bal, lbits *sroot, lbits *chash,
                        bool *exists, bool *storage_cleared,
                        bool *created, bool *selfdestructed);
unit acct_tx_update_raw(const lbits a, uint64_t nonce,
                        const lbits bal, const lbits sroot, const lbits chash,
                        bool exists, bool storage_cleared, bool created,
                        bool selfdestructed);
unit acct_tx_set_balance(const lbits a, const lbits balance);
unit acct_tx_set_nonce(const lbits a, uint64_t nonce);
unit acct_tx_set_code_hash(const lbits a, const lbits code_hash);
uint64_t acct_tx_checkpoint(const unit u);
unit acct_tx_revert(uint64_t checkpoint);
unit acct_tx_reset(const unit u);

/* Debug snapshot enumeration over cumulative account state. */
uint64_t acct_dump_count(const unit u);
void acct_dump_hkey(lbits *rop, uint64_t i);
uint64_t acct_dump_nonce(uint64_t i);
void acct_dump_balance(lbits *rop, uint64_t i);
void acct_dump_storage_root(lbits *rop, uint64_t i);
void acct_dump_code_hash(lbits *rop, uint64_t i);

/* EIP-7928 record store. Sail owns construction and canonical encoding. */
unit bal_reset(const unit u);
unit bal_set_index(uint64_t n);
unit bal_prepare(const unit u);
uint64_t bal_account_count(const unit u);
void bal_account_address(lbits *rop, uint64_t account);

uint64_t bal_storage_change_count(uint64_t account);
void bal_storage_change_slot(lbits *rop, uint64_t account, uint64_t record);
uint64_t bal_storage_change_index(uint64_t account, uint64_t record);
void bal_storage_change_value(lbits *rop, uint64_t account, uint64_t record);
uint64_t bal_storage_read_count(uint64_t account);
void bal_storage_read_slot(lbits *rop, uint64_t account, uint64_t record);

uint64_t bal_balance_change_count(uint64_t account);
uint64_t bal_balance_change_index(uint64_t account, uint64_t record);
void bal_balance_change_value(lbits *rop, uint64_t account, uint64_t record);
uint64_t bal_nonce_change_count(uint64_t account);
uint64_t bal_nonce_change_index(uint64_t account, uint64_t record);
uint64_t bal_nonce_change_value(uint64_t account, uint64_t record);
uint64_t bal_code_change_count(uint64_t account);
uint64_t bal_code_change_index(uint64_t account, uint64_t record);
void bal_code_change_hash(lbits *rop, uint64_t account, uint64_t record);

/* EIP-7928 record sinks + tx-row enumeration (harvest logic is Sail-side) */
unit bal_note_storage_change(const lbits a, const lbits slot, const lbits val);
unit bal_note_account_touch(const lbits a);
unit bal_note_storage_read(const lbits a, const lbits slot);
unit bal_note_balance_change(const lbits a, const lbits val);
unit bal_note_nonce_change(const lbits a, uint64_t nonce);
unit bal_note_code_change(const lbits a, const lbits chash);
/* State-root enumeration over the cumulative maps. */
uint64_t storage_block_count(const lbits a);
bool storage_block_changed(const lbits a, uint64_t i);
void storage_block_slot(lbits *rop, const lbits a, uint64_t i);
void storage_block_current(lbits *rop, const lbits a, uint64_t i);
uint64_t acct_block_count(const unit u);
bool acct_block_changed(uint64_t i);
void acct_block_address(lbits *rop, uint64_t i);
/* k_tx_merge drain pops (side-effect-free) + block propagation hooks */
uint64_t storage_tx_pop_probe(lbits *addr, lbits *slot, lbits *curr,
                              lbits *orig);
unit storage_block_put_raw(const lbits a, const lbits s_, const lbits curr,
                           const lbits orig);
unit storage_block_cache_raw(const lbits a, const lbits s_, const lbits v);
uint64_t acct_tx_pop_probe(lbits *addr,
                           uint64_t *cn, lbits *cb, lbits *cs, lbits *cc,
                           bool *ce, bool *csc, bool *ccr, bool *csd,
                           uint64_t *on, lbits *ob, lbits *os, lbits *oc,
                           bool *oe, bool *osc, bool *ocr, bool *osd);
unit acct_block_write_raw(const lbits a, uint64_t nonce, const lbits bal,
                          const lbits sroot, const lbits chash,
                          bool exists, bool storage_cleared,
                          uint64_t ononce, const lbits obal,
                          const lbits osroot, const lbits ochash,
                          bool oexists, bool ostorage_cleared);
unit acct_block_cache_raw(const lbits a, uint64_t nonce, const lbits bal,
                          const lbits sroot, const lbits chash,
                          bool exists, bool storage_cleared);
#endif
