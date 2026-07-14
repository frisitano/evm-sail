/* Direct crypto helpers for C-side implementation hooks.
 *
 * Sail treats Keccak/SHA as abstract byte-list operations. C helpers here are
 * for optimized pointer/length refinements such as stateless-input code
 * insertion into the code DB.
 */
#ifndef HOST_CRYPTO_H
#define HOST_CRYPTO_H

#include "sail.h"
#include <stdint.h>

enum evmsail_byte_source_kind {
  EVMSAIL_SOURCE_STATELESS_INPUT = 1,
  EVMSAIL_SOURCE_MEMORY = 2,
  EVMSAIL_SOURCE_CODE = 4,
  EVMSAIL_SOURCE_LOG_DATA = 6,
  EVMSAIL_SOURCE_MEMORY_ARENA = 7,
  EVMSAIL_SOURCE_OUTPUT = 8,
  EVMSAIL_SOURCE_SCRATCH = 9,
};

void host_keccak256_bytes(uint64_t out[4], const uint8_t *p, uint64_t len);
void host_sha256_bytes(uint64_t out[4], const uint8_t *p, uint64_t len);
void host_keccak256_lbits(lbits *rop, const uint8_t *p, uint64_t len);
void host_sha256_lbits(lbits *rop, const uint8_t *p, uint64_t len);
void host_keccak_word(lbits *rop, const lbits w);
void host_keccak_address(lbits *rop, const lbits a);
void host_keccak_create2(lbits *rop, const lbits sender, const lbits salt,
                         const lbits init_hash);
void host_create_address(lbits *rop, const lbits sender, uint64_t nonce);
void host_auth_signing_hash(lbits *rop, const lbits chain_id, const lbits address,
                            uint64_t nonce);
void host_sha256_pair(lbits *rop, const lbits a, const lbits b);
int evmsail_resolve_byte_source(uint64_t kind, uint64_t off, uint64_t len,
                                const uint8_t **p, uint64_t *resolved_len);

#endif
