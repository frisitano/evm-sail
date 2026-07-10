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
/* per-layer row probe (layer 0 = tx overlay, 1 = block base) for the Sail
   StorageRow glue (journal_glue.c): 0 = absent, 1 = read, 2 = written */
uint64_t storage_row_probe(uint64_t layer, const lbits a, const lbits s,
                           lbits *cur, lbits *orig);
unit storage_tx_cache(const lbits a, const lbits s, const lbits v);
unit storage_tx_update(const lbits a, const lbits s, const lbits v);
unit storage_wset_wipe_addr(const lbits a);

/* compute_root enumeration over storage_wset_block, per account (keccak(addr)):
   witness = dirty block rows; native = cache-union-block rows */
uint64_t storage_wset_union_count(const lbits ak);
void storage_wset_union_slot(lbits *rop, const lbits ak, uint64_t j);
void storage_wset_union_val(lbits *rop, const lbits ak, uint64_t j);

/* write-set account overlay (stage 2: per-tx overlay + block base) */
unit acct_wset_reset(const unit u);
unit acct_wset_wipe_addr(const lbits a);
/* per-layer account probe (layer 0 = tx, 1 = block) for the Sail
   option(Account) glue: 0 = absent, 1 = present (fields in the out params) */
uint64_t acct_row_probe(uint64_t layer, const lbits a, uint64_t *nonce,
                        lbits *bal, lbits *sroot, lbits *chash);
unit acct_tx_update(const lbits a, uint64_t nonce,
                    const lbits bal, const lbits sroot, const lbits chash);
unit acct_tx_cache(const lbits a, uint64_t nonce, const lbits bal,
                   const lbits sroot, const lbits chash, bool base_exists);

/* compute_root enumeration over acct_wset_block: witness = dirty rows, native =
   cache-union-block rows. Fields mirror AcctRow (host/state.sail). */
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

/* EIP-7928 record sinks + tx-row enumeration (harvest logic is Sail-side) */
unit bal_note_storage_change(const lbits ah, const lbits slot, const lbits val);
unit bal_note_storage_read(const lbits ah, const lbits slot);
unit bal_note_balance_change(const lbits ah, const lbits val);
unit bal_note_nonce_change(const lbits ah, uint64_t nonce);
unit bal_note_code_change(const lbits ah, const lbits chash);
/* state-root enumeration: unfiltered block rows (Sail filters curr != orig) */
uint64_t storage_block_row_count(const lbits ak);
unit storage_block_probe_row(const lbits ak, uint64_t j, lbits *slot, lbits *curr, lbits *orig);
uint64_t acct_block_row_count(const unit u);
uint64_t acct_probe_row(uint64_t layer, uint64_t i, lbits *hkey,
                        uint64_t *cn, lbits *cb, lbits *cs, lbits *cc,
                        uint64_t *on, lbits *ob, lbits *os, lbits *oc);
/* k_tx_merge drain pops (side-effect-free) + block propagation hooks */
uint64_t storage_tx_pop_probe(lbits *ahash, lbits *slot, lbits *curr, lbits *orig);
unit storage_block_put(const lbits ah, const lbits s_, const lbits curr,
                       const lbits orig);
unit storage_block_cache(const lbits ah, const lbits s_, const lbits v);
uint64_t acct_tx_pop_probe(lbits *addr, lbits *hkey,
                           uint64_t *cn, lbits *cb, lbits *cs, lbits *cc,
                           uint64_t *on, lbits *ob, lbits *os, lbits *oc);
unit acct_block_write(const lbits a, uint64_t nonce, const lbits bal,
                      const lbits sroot, const lbits chash,
                      uint64_t ononce, const lbits obal,
                      const lbits osroot, const lbits ochash, bool base_exists);
unit acct_block_cache(const lbits a, uint64_t nonce, const lbits bal,
                      const lbits sroot, const lbits chash, bool base_exists);
#endif
