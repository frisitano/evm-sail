/* Direct crypto helpers for C-side implementation hooks.
 *
 * Sail treats Keccak/SHA as abstract byte-list operations. C helpers here are
 * for optimized pointer/length refinements such as stateless-input code
 * insertion into the code DB. Byte-source resolution belongs to
 * byte_slice_glue.h and is deliberately not part of this interface.
 */
#ifndef HOST_CRYPTO_H
#define HOST_CRYPTO_H

#include "sail_abi.h"
#include <stdint.h>

/* Generated aggregate boundary implemented by hash_glue.c. These incomplete
 * types let the generated model see the correct calling convention before it
 * emits the corresponding Sail type definitions. */
struct zByteSlice;
struct node_zz5listz8z5unionz0zzBytesz9;
EVMSAIL_HASH_RETURN host_keccak_segments(
    EVMSAIL_HASH_RESULT(result)
    struct node_zz5listz8z5unionz0zzBytesz9 *segments);
EVMSAIL_HASH_RETURN host_sha256_segments(
    EVMSAIL_HASH_RESULT(result)
    struct node_zz5listz8z5unionz0zzBytesz9 *segments);
bool host_bytes_segments_equal_slice(
    struct node_zz5listz8z5unionz0zzBytesz9 *segments,
    struct zByteSlice expected);

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
#endif
