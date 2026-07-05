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
unit acctmap_at(uint64_t idx);
unit acctmap_update_at(uint64_t idx);
void acctmap_at_hkey(lbits *rop, const unit u);
uint64_t acctmap_at_nonce(const unit u);
void acctmap_at_bal(lbits *rop, const unit u);
void acctmap_at_sroot(lbits *rop, const unit u);
void acctmap_at_chash(lbits *rop, const unit u);
bool acctmap_at_base_exists(const unit u);

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
bool storage_map_cache_nonzero(const lbits ah, const lbits sh);
unit storage_map_wipe_addr(const lbits a);
uint64_t storage_map_count(const unit u);
uint64_t storage_map_update_count(const unit u);
unit storage_map_at(uint64_t j);
unit storage_map_update_at(uint64_t j);
void storage_map_it_acct_hash(lbits *rop, const unit u);
void storage_map_it_slot(lbits *rop, const unit u);
void storage_map_it_val(lbits *rop, const unit u);

#endif
