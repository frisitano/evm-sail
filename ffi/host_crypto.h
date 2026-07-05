/* Direct crypto helpers for C-side implementation hooks.
 *
 * Sail treats Keccak/SHA as abstract byte-list operations. C helpers here are
 * for optimized pointer/length refinements such as witness-code insertion into
 * the code DB.
 */
#ifndef HOST_CRYPTO_H
#define HOST_CRYPTO_H

#include "sail.h"
#include <stdint.h>

enum evmsail_byte_source_kind {
  EVMSAIL_SOURCE_WITNESS = 1,
  EVMSAIL_SOURCE_MEMORY = 2,
  EVMSAIL_SOURCE_TX_INPUT = 3,
  EVMSAIL_SOURCE_ACTIVE_CODE = 4,
};

void host_keccak256_bytes(uint64_t out[4], const uint8_t *p, uint64_t len);
void host_sha256_bytes(uint64_t out[4], const uint8_t *p, uint64_t len);
void host_keccak256_lbits(lbits *rop, const uint8_t *p, uint64_t len);
void host_sha256_lbits(lbits *rop, const uint8_t *p, uint64_t len);
void host_keccak_word(lbits *rop, uint64_t w3, uint64_t w2, uint64_t w1, uint64_t w0);
void host_keccak_address(lbits *rop, uint64_t a2, uint64_t a1, uint64_t a0);
void host_keccak_create2(lbits *rop, uint64_t a2, uint64_t a1, uint64_t a0,
                         uint64_t salt3, uint64_t salt2, uint64_t salt1, uint64_t salt0,
                         uint64_t init3, uint64_t init2, uint64_t init1, uint64_t init0);
void host_sha256_pair(lbits *rop, uint64_t a3, uint64_t a2, uint64_t a1, uint64_t a0,
                      uint64_t b3, uint64_t b2, uint64_t b1, uint64_t b0);
void host_sha256_execution_requests(lbits *rop,
                                    uint64_t has0, uint64_t d03, uint64_t d02,
                                    uint64_t d01, uint64_t d00,
                                    uint64_t has1, uint64_t d13, uint64_t d12,
                                    uint64_t d11, uint64_t d10,
                                    uint64_t has2, uint64_t d23, uint64_t d22,
                                    uint64_t d21, uint64_t d20);
int evmsail_resolve_byte_source(uint64_t kind, uint64_t off, uint64_t len,
                                const uint8_t **p, uint64_t *resolved_len);
void host_source_keccak(lbits *rop, uint64_t source_kind, uint64_t off, uint64_t len);
void host_source_sha256_prefixed(lbits *rop, uint64_t prefix, uint64_t source_kind,
                                 uint64_t off, uint64_t len);

unit host_bytes_reset(const unit u);
unit host_bytes_push(uint64_t b);
unit host_bytes_push8(uint64_t w);
void host_bytes_keccak_finish(lbits *rop, const unit u);
void host_bytes_sha256_finish(lbits *rop, const unit u);

#endif
