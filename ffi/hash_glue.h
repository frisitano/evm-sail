/* Generated aggregate boundaries implemented by hash_glue.c. */
#ifndef EVMSAIL_HASH_GLUE_H
#define EVMSAIL_HASH_GLUE_H

#include "sail_abi.h"
#include <stdbool.h>
#include <stdint.h>

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

#endif
