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

/* write-set storage overlay (stage 1: tx overlay + block base) */
unit storage_wset_reset(const unit u);
unit storage_wset_tx_clear(const unit u);
unit storage_wset_merge(const unit u);
bool storage_wset_present(const lbits a, const lbits s);
void storage_wset_load(lbits *rop, const lbits a, const lbits s);
bool storage_wset_base_present(const lbits a, const lbits s);
void storage_wset_base_load(lbits *rop, const lbits a, const lbits s);
unit storage_wset_touch(const lbits a, const lbits s);
unit storage_wset_cache_read(const lbits a, const lbits s, const lbits v);
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

/* write-set account overlay (stage 2: per-tx overlay + block base) */
unit acct_wset_reset(const unit u);
unit acct_wset_tx_clear(const unit u);
unit acct_wset_merge(const unit u);
unit acct_wset_wipe_addr(const lbits a);
bool acct_wset_present(const lbits a);
uint64_t acct_wset_nonce(const lbits a);
void acct_wset_bal(lbits *rop, const lbits a);
void acct_wset_sroot(lbits *rop, const lbits a);
void acct_wset_chash(lbits *rop, const lbits a);
unit acct_wset_write(const lbits a, uint64_t nonce,
                     const lbits bal, const lbits sroot, const lbits chash);
unit acct_wset_restore(const lbits a, uint64_t nonce,
                       const lbits bal, const lbits sroot, const lbits chash);
unit acct_wset_seed_read(const lbits a, uint64_t nonce,
                         const lbits bal, const lbits sroot, const lbits chash);
unit acct_wset_mark_base_exists(const lbits a);

/* compute_root enumeration over acct_wset_block: witness = dirty rows, native =
   cache-union-block rows. Fields mirror AcctRow (host/state.sail). */
uint64_t acct_wset_block_dirty_count(const unit u);
void acct_wset_block_dirty_hkey(lbits *rop, uint64_t i);
uint64_t acct_wset_block_dirty_nonce(uint64_t i);
void acct_wset_block_dirty_bal(lbits *rop, uint64_t i);
void acct_wset_block_dirty_sroot(lbits *rop, uint64_t i);
void acct_wset_block_dirty_chash(lbits *rop, uint64_t i);
bool acct_wset_block_dirty_base_exists(uint64_t i);
uint64_t acct_wset_union_count(const unit u);
void acct_wset_union_hkey(lbits *rop, uint64_t i);
uint64_t acct_wset_union_nonce(uint64_t i);
void acct_wset_union_bal(lbits *rop, uint64_t i);
void acct_wset_union_sroot(lbits *rop, uint64_t i);
void acct_wset_union_chash(lbits *rop, uint64_t i);
bool acct_wset_union_base_exists(uint64_t i);

/* EIP-7928 block access list, recomputed from execution. The overlay merges
   harvest the changes/reads (keyed by keccak(address), tagged with bal_set_index);
   bal_recompute_hash returns keccak(rlp(bal)) for the header commitment. */
unit bal_reset(const unit u);
unit bal_set_index(uint64_t n);
void bal_recompute_hash(lbits *rop, const unit u);

#endif
