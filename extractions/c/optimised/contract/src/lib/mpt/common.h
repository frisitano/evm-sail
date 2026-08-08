/* Shared private MPT base layer: byte spans, size limits,
 * NodeId sentinels, and the accelerator keccak helper. Every lib/mpt unit
 * (paths, codec, nodes, trie) builds on this header; nothing here owns
 * state. */
#ifndef EVMSAIL_OPTIMIZED_LIB_MPT_COMMON_H
#define EVMSAIL_OPTIMIZED_LIB_MPT_COMMON_H

#include "evmsail/host/nodes.h"
#include "evmsail/exceptions.h"
#include "evmsail/prelude.h"
#include "primitives/hash.h"
#include "zkvm_accelerators.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
  MPT_SECURE_KEY_NIBBLES = 64,
  MPT_INLINE_REFERENCE_MAX = 31,
  MPT_ENCODED_VALUE_MAX = 128,
  MPT_ENCODED_NODE_MAX = 600,
};

/* NodeId 0 is exclusively an unresolved edge. UINT32_MAX denotes the
 * canonical empty trie, which has no encoded witness node at all. Real ids
 * are dense row numbers in the sealed witness-node region. */
#define MPT_NODE_ID_UNLINKED EVMSAIL_NODE_ID_UNLINKED
#define MPT_NODE_ID_EMPTY EVMSAIL_NODE_ID_EMPTY

/* Borrowed view of immutable bytes; never owns its storage. */
typedef struct {
  const uint8_t *data;
  size_t len;
} ByteSpan;

static inline void mpt_keccak(const uint8_t *data, size_t len, bytes32 *out)
{
  static const uint8_t empty[1] = {0};
  zkvm_keccak256_hash digest;
  _Static_assert(sizeof(*out) == sizeof(zkvm_keccak256_hash),
                 "accelerator digest must be 32 bytes");
  if (zkvm_keccak256(len ? data : empty, len, &digest) != ZKVM_EOK) {
    fatal_error(RlpDecode);
  }
  memcpy(out, &digest, sizeof *out);
}

#endif
