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
struct zByteSliceFields;
struct node_zz5listz8z5unionz0zzBytesz9;
EVMSAIL_HASH_RETURN host_keccak_segments(
    EVMSAIL_HASH_RESULT(result)
    struct node_zz5listz8z5unionz0zzBytesz9 *segments);
EVMSAIL_HASH_RETURN host_sha256_segments(
    EVMSAIL_HASH_RESULT(result)
    struct node_zz5listz8z5unionz0zzBytesz9 *segments);
bool host_bytes_segments_equal_slice(
    struct node_zz5listz8z5unionz0zzBytesz9 *segments,
    struct zByteSliceFields expected);

void host_keccak256_bytes(uint64_t out[4], const uint8_t *p, uint64_t len);
void host_sha256_bytes(uint64_t out[4], const uint8_t *p, uint64_t len);
#endif
