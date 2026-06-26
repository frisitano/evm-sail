#ifndef HOST_ACCTMAP_H
#define HOST_ACCTMAP_H
#include "sail.h"
#include <stdint.h>

/* C-backed raw-address -> account overlay map (ffi/host_acctmap.c): the kernel's
 * mutable account working set (k_accounts), keyed by the RAW 20-byte address so
 * reads/writes never hash. Replaces the Sail assoc list whose acc_lookup /
 * acc_has / acc_update were O(n) linear scans; this is an O(1)-amortized
 * open-addressing hash map. The witness-authenticated pre-state is the base it
 * sits over: a miss in stateless mode is materialized once from the acctdb span
 * store (one keccak) and cached here by raw address, so repeat touches are
 * hash-free O(1).
 *
 * An account value is (nonce, balance, storage_root, code_hash): nonce is one
 * word (EIP-2681 bounds it under 2^64); the three 256-bit fields cross as four
 * big-endian words each (w3 most significant), matching ffi/host_map.c. A key is
 * selected once with acctmap_key (caches addr + hash) and the accessors resolve
 * against it. Iteration (acctmap_count/_at/_at_*) feeds the post-state root. */
unit acctmap_reset(const unit u);
unit acctmap_key(uint64_t a2, uint64_t a1, uint64_t a0);   /* select+cache key */
uint64_t acctmap_present(const unit u);                    /* 1 if selected key present */
unit acctmap_store(uint64_t nonce,
                   uint64_t b3, uint64_t b2, uint64_t b1, uint64_t b0,
                   uint64_t sr3, uint64_t sr2, uint64_t sr1, uint64_t sr0,
                   uint64_t ch3, uint64_t ch2, uint64_t ch1, uint64_t ch0);
uint64_t acctmap_nonce(const unit u);                      /* fields at selected key */
uint64_t acctmap_bal(uint64_t w);
uint64_t acctmap_sroot(uint64_t w);
uint64_t acctmap_chash(uint64_t w);
unit acctmap_remove(uint64_t a2, uint64_t a1, uint64_t a0);

uint64_t acctmap_count(const unit u);                      /* iteration (post-state root) */
unit acctmap_at(uint64_t idx);
uint64_t acctmap_at_key(uint64_t w);                       /* w 0..2 = addr a2,a1,a0 */
uint64_t acctmap_at_nonce(const unit u);
uint64_t acctmap_at_bal(uint64_t w);
uint64_t acctmap_at_chash(uint64_t w);

#endif
