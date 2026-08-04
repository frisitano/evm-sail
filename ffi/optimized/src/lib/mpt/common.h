/* Shared private MPT base layer: status context, byte spans, size limits,
 * NodeId sentinels, and the accelerator keccak helper. Every lib/mpt unit
 * (paths, codec, nodes, trie) builds on this header; nothing here owns
 * state. */
#ifndef EVMSAIL_OPTIMIZED_LIB_MPT_COMMON_H
#define EVMSAIL_OPTIMIZED_LIB_MPT_COMMON_H

#include "evmsail/host/nodes.h"
#include "evmsail/prelude.h"
#include "primitives/hash.h"
#include "zkvm_accelerators.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* First-failure classification recorded in mpt_status. */
typedef enum {
  MPT_OK = 0,
  MPT_WITNESS_DEFICIENT = 1,
  MPT_RLP_DECODE = 2,
  MPT_INVALID_CONFIG = 3,
} MptStatus;

enum {
  MPT_SECURE_KEY_NIBBLES = 64,
  MPT_INLINE_REFERENCE_MAX = 31,
  MPT_ENCODED_VALUE_MAX = 128,
  MPT_ENCODED_NODE_MAX = 600,
};

/* NodeId 0 is exclusively an unresolved edge. UINT32_MAX denotes the
 * canonical empty trie, which has no encoded witness node at all. Real ids
 * are digest-bucket positions plus one, then a dynamic tail. */
#define MPT_NODE_ID_UNLINKED EVMSAIL_NODE_ID_UNLINKED
#define MPT_NODE_ID_EMPTY EVMSAIL_NODE_ID_EMPTY

/* Single first-failure status latch shared by every lib/mpt unit (defined
 * in paths.c, the base unit). Fallible functions return false and record
 * their classification here; the guest is single-threaded and MPT
 * operations are strictly sequential, so top-level operations reset the
 * latch on entry and read it on exit. */
extern uint64_t mpt_status;

/* Borrowed view of immutable bytes; never owns its storage. */
typedef struct {
  const uint8_t *data;
  size_t len;
} ByteSpan;

static inline bool mpt_fail(uint64_t status) {
  if (mpt_status == MPT_OK) {
    mpt_status = status;
  }
  return false;
}

static inline bool mpt_keccak(const uint8_t *data, size_t len, Hash32 *out) {
  static const uint8_t empty[1] = {0};
  _Static_assert(sizeof(*out) == sizeof(zkvm_keccak256_hash),
                 "accelerator digest must be 32 bytes");
  if (zkvm_keccak256(len ? data : empty, len,
                     (zkvm_keccak256_hash *)(void *)out) != ZKVM_EOK)
    return mpt_fail(MPT_RLP_DECODE);
  return true;
}

#endif
