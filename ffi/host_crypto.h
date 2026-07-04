/* Direct host crypto helpers for byte ranges and EVM memory ranges.
 *
 * Sail chooses the bytes. This module provides pointer/length host boundaries
 * for keccak256 and sha256, typed witness-slice hashing, and a small scratch
 * byte input for Sail list-backed specification helpers.
 */
#ifndef HOST_CRYPTO_H
#define HOST_CRYPTO_H

#include "sail.h"
#include <stdint.h>

void host_keccak256_bytes(uint64_t out[4], const uint8_t *p, uint64_t len);
void host_sha256_bytes(uint64_t out[4], const uint8_t *p, uint64_t len);

unit host_bytes_reset(const unit u);
unit host_bytes_append_byte(uint64_t b);
unit host_bytes_append8(uint64_t w);
const uint8_t *host_bytes_data(uint64_t *len_out);

unit host_keccak_input(const unit u);
unit host_sha256_input(const unit u);
unit host_keccak_witness(uint64_t off, uint64_t len);
unit host_sha256_request_digest(uint64_t request_type, uint64_t off, uint64_t len);
unit host_keccak_word(uint64_t w0, uint64_t w1, uint64_t w2, uint64_t w3);
unit host_keccak_address(uint64_t a0, uint64_t a1, uint64_t a2);
unit host_sha256_pair_words(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                            uint64_t b0, uint64_t b1, uint64_t b2, uint64_t b3);
unit host_keccak_memory(uint64_t off, uint64_t len);
uint64_t host_hash_word(uint64_t i);

#endif
