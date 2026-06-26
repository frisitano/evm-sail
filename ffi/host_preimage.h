#ifndef HOST_PREIMAGE_H
#define HOST_PREIMAGE_H
#include "sail.h"
#include <stdint.h>

/* C-backed pre-image cache (ffi/host_preimage.c): raw key -> keccak(key), the
 * secure-trie key derivation memoized so each touched address/slot is hashed
 * exactly once and reused for reads, non-membership walks, and the post-state
 * root. keccak is computed Sail-side (the accelerator); this only stores and
 * returns the result. Addresses and slots are kept in SEPARATE namespaces (an
 * address and a slot with the same numeric value have different keccaks), so
 * there are two maps: addr (3 key words) and slot (4 key words). The 256-bit
 * keccak crosses as four big-endian words (w3 most significant). Same open-
 * addressing convention as ffi/host_map.c / host_acctmap.c. */

/* address namespace: keccak(address) */
unit pa_reset(const unit u);
unit pa_key(uint64_t a2, uint64_t a1, uint64_t a0);     /* select+cache key */
uint64_t pa_present(const unit u);                       /* 1 if cached */
unit pa_store(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0);
uint64_t pa_word(uint64_t w);                            /* hash word at selected key */

/* slot namespace: keccak(slot) */
unit ps_reset(const unit u);
unit ps_key(uint64_t s3, uint64_t s2, uint64_t s1, uint64_t s0);
uint64_t ps_present(const unit u);
unit ps_store(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0);
uint64_t ps_word(uint64_t w);

#endif
