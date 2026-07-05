/* C-backed account and persistent-storage state database for evm-sail.
 *
 * Accounts are keyed by keccak256(address). Persistent storage rows are keyed
 * by (keccak256(address), keccak256(slot)) and retain raw slots only for dumps.
 * Both stores are sorted, cache/update backed, and expose only uint64_t values
 * across the Sail FFI boundary. */
#ifndef STATE_DB_H
#define STATE_DB_H
#include "sail.h"
#include <stdbool.h>
#include <stdint.h>

/* Account map: materialized base/cache rows plus execution update rows. */
unit acctmap_reset(const unit u);
unit acctmap_key(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0);
bool acctmap_present(const unit u);
unit acctmap_store(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0,
                   uint64_t nonce,
                   uint64_t b3, uint64_t b2, uint64_t b1, uint64_t b0,
                   uint64_t sr3, uint64_t sr2, uint64_t sr1, uint64_t sr0,
                   uint64_t ch3, uint64_t ch2, uint64_t ch1, uint64_t ch0);
unit acctmap_mark_base_exists(const unit u);
unit acctmap_seed(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0,
                  uint64_t nonce,
                  uint64_t b3, uint64_t b2, uint64_t b1, uint64_t b0,
                  uint64_t sr3, uint64_t sr2, uint64_t sr1, uint64_t sr0,
                  uint64_t ch3, uint64_t ch2, uint64_t ch1, uint64_t ch0);
uint64_t acctmap_nonce(const unit u);
uint64_t acctmap_bal(uint64_t w);
uint64_t acctmap_sroot(uint64_t w);
uint64_t acctmap_chash(uint64_t w);
unit acctmap_remove(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0);

uint64_t acctmap_count(const unit u);
uint64_t acctmap_update_count(const unit u);
unit acctmap_at(uint64_t idx);
unit acctmap_update_at(uint64_t idx);
uint64_t acctmap_at_hkey(uint64_t w);
uint64_t acctmap_at_nonce(const unit u);
uint64_t acctmap_at_bal(uint64_t w);
uint64_t acctmap_at_sroot(uint64_t w);
uint64_t acctmap_at_chash(uint64_t w);
bool acctmap_at_base_exists(const unit u);

/* Persistent storage map: materialized base/cache rows plus frame overlays. */
unit storage_map_reset(const unit u);
unit storage_map_push(const unit u);
unit storage_map_pop_commit(const unit u);
unit storage_map_pop_discard(const unit u);
unit storage_map_key(const lbits slot, const lbits acct_hash, const lbits slot_hash);
unit storage_map_seed(const lbits v);
unit storage_map_store(const lbits v);
void storage_map_value(lbits *rop, const unit u);
void storage_map_base_value(lbits *rop, const unit u);
bool storage_map_present(const unit u);
bool storage_map_base_present(const unit u);
unit storage_map_wipe_acct_hash(const lbits ah);
uint64_t storage_map_count(const unit u);
uint64_t storage_map_update_count(const unit u);
unit storage_map_at(uint64_t j);
unit storage_map_update_at(uint64_t j);
void storage_map_it_acct_hash(lbits *rop, const unit u);
void storage_map_it_slot(lbits *rop, const unit u);
void storage_map_it_val(lbits *rop, const unit u);

#endif
