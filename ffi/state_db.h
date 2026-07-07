/* C-backed account and persistent-storage state database for evm-sail.
 *
 * Rows are stored sorted by keccak256(address) (accounts) and
 * (keccak256(address), keccak256(slot)) (storage), but the point-access hooks
 * are keyed by raw address/slot: the secure trie hashes are computed and
 * memoized C-side. Both stores are cache/update backed. */
#ifndef STATE_DB_H
#define STATE_DB_H
#include "sail.h"
#include <stdbool.h>
#include <stdint.h>

/* Account map: materialized base/cache rows plus execution update rows. */
unit acctmap_reset(const unit u);
bool acctmap_present(const lbits a);
unit acctmap_store(const lbits a, uint64_t nonce,
                   const lbits bal, const lbits sroot, const lbits chash);
unit acctmap_mark_base_exists(const lbits a);
unit acctmap_seed(const lbits a, uint64_t nonce,
                  const lbits bal, const lbits sroot, const lbits chash);
uint64_t acctmap_nonce(const lbits a);
void acctmap_bal(lbits *rop, const lbits a);
void acctmap_sroot(lbits *rop, const lbits a);
void acctmap_chash(lbits *rop, const lbits a);
unit acctmap_remove(const lbits a);

uint64_t acctmap_count(const unit u);
uint64_t acctmap_update_count(const unit u);
void acctmap_row_hkey(lbits *rop, uint64_t idx);
uint64_t acctmap_row_nonce(uint64_t idx);
void acctmap_row_bal(lbits *rop, uint64_t idx);
void acctmap_row_sroot(lbits *rop, uint64_t idx);
void acctmap_row_chash(lbits *rop, uint64_t idx);
bool acctmap_row_base_exists(uint64_t idx);
void acctmap_update_row_hkey(lbits *rop, uint64_t idx);
uint64_t acctmap_update_row_nonce(uint64_t idx);
void acctmap_update_row_bal(lbits *rop, uint64_t idx);
void acctmap_update_row_sroot(lbits *rop, uint64_t idx);
void acctmap_update_row_chash(lbits *rop, uint64_t idx);
bool acctmap_update_row_base_exists(uint64_t idx);

/* Persistent storage map: materialized base/cache rows plus frame overlays. */
unit storage_map_reset(const unit u);
unit storage_map_push(const unit u);
unit storage_map_pop_commit(const unit u);
unit storage_map_pop_discard(const unit u);
unit storage_map_seed(const lbits a, const lbits s, const lbits v);
unit storage_map_store(const lbits a, const lbits s, const lbits v);
void storage_map_load(lbits *rop, const lbits a, const lbits s);
void storage_map_base_load(lbits *rop, const lbits a, const lbits s);
bool storage_map_present(const lbits a, const lbits s);
bool storage_map_base_present(const lbits a, const lbits s);
bool storage_cache_present(const lbits a, const lbits s);
void storage_cache_load(lbits *rop, const lbits a, const lbits s);
bool storage_map_cache_nonzero(const lbits ah, const lbits sh);
unit storage_map_wipe_addr(const lbits a);
uint64_t storage_map_acct_count(const lbits ak);
void storage_map_acct_slot(lbits *rop, const lbits ak, uint64_t j);
void storage_map_acct_val(lbits *rop, const lbits ak, uint64_t j);
uint64_t storage_map_acct_update_count(const lbits ak);
void storage_map_acct_update_slot(lbits *rop, const lbits ak, uint64_t j);
void storage_map_acct_update_val(lbits *rop, const lbits ak, uint64_t j);

/* write-set storage overlay (stage 1: tx overlay + block base) */
unit storage_wset_reset(const unit u);
unit storage_wset_tx_clear(const unit u);
unit storage_wset_merge(const unit u);
bool storage_wset_present(const lbits a, const lbits s);
void storage_wset_load(lbits *rop, const lbits a, const lbits s);
bool storage_wset_base_present(const lbits a, const lbits s);
void storage_wset_base_load(lbits *rop, const lbits a, const lbits s);
unit storage_wset_touch(const lbits a, const lbits s);
void storage_wset_prior(lbits *rop, const lbits a, const lbits s, const lbits orig);
unit storage_wset_write(const lbits a, const lbits s, const lbits v, const lbits orig);
unit storage_wset_restore(const lbits a, const lbits s, const lbits prior);
bool storage_wset_warm(const lbits a, const lbits s);
bool storage_wset_is_warm(const lbits a, const lbits s);
unit storage_wset_unwarm(const lbits a, const lbits s);
unit storage_wset_wipe_addr(const lbits a);

/* compute_root enumeration over storage_wset_block, per account (keccak(addr)):
   witness = dirty block rows; native = cache-union-block rows */
uint64_t storage_wset_block_dirty_count(const lbits ak);
void storage_wset_block_dirty_slot(lbits *rop, const lbits ak, uint64_t j);
void storage_wset_block_dirty_val(lbits *rop, const lbits ak, uint64_t j);
uint64_t storage_wset_union_count(const lbits ak);
void storage_wset_union_slot(lbits *rop, const lbits ak, uint64_t j);
void storage_wset_union_val(lbits *rop, const lbits ak, uint64_t j);

#endif
