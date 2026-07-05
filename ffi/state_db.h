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
unit storage_map_key(uint64_t s3, uint64_t s2, uint64_t s1, uint64_t s0,
                     uint64_t ah3, uint64_t ah2, uint64_t ah1, uint64_t ah0,
                     uint64_t sh3, uint64_t sh2, uint64_t sh1, uint64_t sh0);
unit storage_map_seed(uint64_t v3, uint64_t v2, uint64_t v1, uint64_t v0);
unit storage_map_store(uint64_t v3, uint64_t v2, uint64_t v1, uint64_t v0);
uint64_t storage_map_word(uint64_t i);
uint64_t storage_map_base_word(uint64_t i);
bool storage_map_present(const unit u);
bool storage_map_base_present(const unit u);
unit storage_map_wipe_acct_hash(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0);
uint64_t storage_map_count(const unit u);
uint64_t storage_map_update_count(const unit u);
unit storage_map_at(uint64_t j);
unit storage_map_update_at(uint64_t j);
uint64_t storage_map_it_acct_hash(uint64_t i);
uint64_t storage_map_it_slot(uint64_t i);
uint64_t storage_map_it_val(uint64_t i);

#endif
