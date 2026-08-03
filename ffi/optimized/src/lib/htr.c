/*
 * Optimized hash_tree_root for SszNewPayloadRequest.
 *
 * This translation unit is linked only into optimized C builds. It resolves
 * the already validated StatelessInputRef slices to native pointer/length
 * spans and keeps only an O(depth) Merkle frontier. The readable Sail
 * equations in lib/htr.sail remain the standard implementation and the source
 * used by proof extraction.
 */
#include "evmsail/prelude.h"

#include "evmsail/lib/htr.h"
#include "evmsail/host/region_access.h"
#include "primitives/value.h"
#include "zkvm_accelerators.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum {
  HTR_MAX_DEPTH = 25,
  HTR_CHUNK_SIZE = 32,
  EVMSAIL_HTR_BYTE_LIST_LIMIT = 1073741824,
};

typedef uint8_t htr_root[HTR_CHUNK_SIZE];

struct htr_span {
  const uint8_t *bytes;
  uint64_t len;
};

struct htr_input_view {
  struct htr_span new_payload_request;
  struct htr_span execution_payload;
  struct htr_span versioned_hashes;
  struct htr_span deposits;
  struct htr_span withdrawal_requests;
  struct htr_span consolidation_requests;
  struct htr_span builder_deposit_requests;
  struct htr_span builder_exit_requests;
  struct htr_span extra_data;
  struct htr_span transactions;
  struct htr_span withdrawals;
  struct htr_span block_access_list;
  uint64_t transaction_count;
  uint64_t withdrawal_count;
};

struct htr_accumulator {
  htr_root frontier[HTR_MAX_DEPTH + 1];
  uint64_t count;
  unsigned depth;
};

/* SHA256 roots of all-zero subtrees, indexed by subtree depth. */
static const htr_root htr_zero_hashes[HTR_MAX_DEPTH + 1] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xf5, 0xa5, 0xfd, 0x42, 0xd1, 0x6a, 0x20, 0x30, 0x27, 0x98, 0xef,
     0x6e, 0xd3, 0x09, 0x97, 0x9b, 0x43, 0x00, 0x3d, 0x23, 0x20, 0xd9,
     0xf0, 0xe8, 0xea, 0x98, 0x31, 0xa9, 0x27, 0x59, 0xfb, 0x4b},
    {0xdb, 0x56, 0x11, 0x4e, 0x00, 0xfd, 0xd4, 0xc1, 0xf8, 0x5c, 0x89,
     0x2b, 0xf3, 0x5a, 0xc9, 0xa8, 0x92, 0x89, 0xaa, 0xec, 0xb1, 0xeb,
     0xd0, 0xa9, 0x6c, 0xde, 0x60, 0x6a, 0x74, 0x8b, 0x5d, 0x71},
    {0xc7, 0x80, 0x09, 0xfd, 0xf0, 0x7f, 0xc5, 0x6a, 0x11, 0xf1, 0x22,
     0x37, 0x06, 0x58, 0xa3, 0x53, 0xaa, 0xa5, 0x42, 0xed, 0x63, 0xe4,
     0x4c, 0x4b, 0xc1, 0x5f, 0xf4, 0xcd, 0x10, 0x5a, 0xb3, 0x3c},
    {0x53, 0x6d, 0x98, 0x83, 0x7f, 0x2d, 0xd1, 0x65, 0xa5, 0x5d, 0x5e,
     0xea, 0xe9, 0x14, 0x85, 0x95, 0x44, 0x72, 0xd5, 0x6f, 0x24, 0x6d,
     0xf2, 0x56, 0xbf, 0x3c, 0xae, 0x19, 0x35, 0x2a, 0x12, 0x3c},
    {0x9e, 0xfd, 0xe0, 0x52, 0xaa, 0x15, 0x42, 0x9f, 0xae, 0x05, 0xba,
     0xd4, 0xd0, 0xb1, 0xd7, 0xc6, 0x4d, 0xa6, 0x4d, 0x03, 0xd7, 0xa1,
     0x85, 0x4a, 0x58, 0x8c, 0x2c, 0xb8, 0x43, 0x0c, 0x0d, 0x30},
    {0xd8, 0x8d, 0xdf, 0xee, 0xd4, 0x00, 0xa8, 0x75, 0x55, 0x96, 0xb2,
     0x19, 0x42, 0xc1, 0x49, 0x7e, 0x11, 0x4c, 0x30, 0x2e, 0x61, 0x18,
     0x29, 0x0f, 0x91, 0xe6, 0x77, 0x29, 0x76, 0x04, 0x1f, 0xa1},
    {0x87, 0xeb, 0x0d, 0xdb, 0xa5, 0x7e, 0x35, 0xf6, 0xd2, 0x86, 0x67,
     0x38, 0x02, 0xa4, 0xaf, 0x59, 0x75, 0xe2, 0x25, 0x06, 0xc7, 0xcf,
     0x4c, 0x64, 0xbb, 0x6b, 0xe5, 0xee, 0x11, 0x52, 0x7f, 0x2c},
    {0x26, 0x84, 0x64, 0x76, 0xfd, 0x5f, 0xc5, 0x4a, 0x5d, 0x43, 0x38,
     0x51, 0x67, 0xc9, 0x51, 0x44, 0xf2, 0x64, 0x3f, 0x53, 0x3c, 0xc8,
     0x5b, 0xb9, 0xd1, 0x6b, 0x78, 0x2f, 0x8d, 0x7d, 0xb1, 0x93},
    {0x50, 0x6d, 0x86, 0x58, 0x2d, 0x25, 0x24, 0x05, 0xb8, 0x40, 0x01,
     0x87, 0x92, 0xca, 0xd2, 0xbf, 0x12, 0x59, 0xf1, 0xef, 0x5a, 0xa5,
     0xf8, 0x87, 0xe1, 0x3c, 0xb2, 0xf0, 0x09, 0x4f, 0x51, 0xe1},
    {0xff, 0xff, 0x0a, 0xd7, 0xe6, 0x59, 0x77, 0x2f, 0x95, 0x34, 0xc1,
     0x95, 0xc8, 0x15, 0xef, 0xc4, 0x01, 0x4e, 0xf1, 0xe1, 0xda, 0xed,
     0x44, 0x04, 0xc0, 0x63, 0x85, 0xd1, 0x11, 0x92, 0xe9, 0x2b},
    {0x6c, 0xf0, 0x41, 0x27, 0xdb, 0x05, 0x44, 0x1c, 0xd8, 0x33, 0x10,
     0x7a, 0x52, 0xbe, 0x85, 0x28, 0x68, 0x89, 0x0e, 0x43, 0x17, 0xe6,
     0xa0, 0x2a, 0xb4, 0x76, 0x83, 0xaa, 0x75, 0x96, 0x42, 0x20},
    {0xb7, 0xd0, 0x5f, 0x87, 0x5f, 0x14, 0x00, 0x27, 0xef, 0x51, 0x18,
     0xa2, 0x24, 0x7b, 0xbb, 0x84, 0xce, 0x8f, 0x2f, 0x0f, 0x11, 0x23,
     0x62, 0x30, 0x85, 0xda, 0xf7, 0x96, 0x0c, 0x32, 0x9f, 0x5f},
    {0xdf, 0x6a, 0xf5, 0xf5, 0xbb, 0xdb, 0x6b, 0xe9, 0xef, 0x8a, 0xa6,
     0x18, 0xe4, 0xbf, 0x80, 0x73, 0x96, 0x08, 0x67, 0x17, 0x1e, 0x29,
     0x67, 0x6f, 0x8b, 0x28, 0x4d, 0xea, 0x6a, 0x08, 0xa8, 0x5e},
    {0xb5, 0x8d, 0x90, 0x0f, 0x5e, 0x18, 0x2e, 0x3c, 0x50, 0xef, 0x74,
     0x96, 0x9e, 0xa1, 0x6c, 0x77, 0x26, 0xc5, 0x49, 0x75, 0x7c, 0xc2,
     0x35, 0x23, 0xc3, 0x69, 0x58, 0x7d, 0xa7, 0x29, 0x37, 0x84},
    {0xd4, 0x9a, 0x75, 0x02, 0xff, 0xcf, 0xb0, 0x34, 0x0b, 0x1d, 0x78,
     0x85, 0x68, 0x85, 0x00, 0xca, 0x30, 0x81, 0x61, 0xa7, 0xf9, 0x6b,
     0x62, 0xdf, 0x9d, 0x08, 0x3b, 0x71, 0xfc, 0xc8, 0xf2, 0xbb},
    {0x8f, 0xe6, 0xb1, 0x68, 0x92, 0x56, 0xc0, 0xd3, 0x85, 0xf4, 0x2f,
     0x5b, 0xbe, 0x20, 0x27, 0xa2, 0x2c, 0x19, 0x96, 0xe1, 0x10, 0xba,
     0x97, 0xc1, 0x71, 0xd3, 0xe5, 0x94, 0x8d, 0xe9, 0x2b, 0xeb},
    {0x8d, 0x0d, 0x63, 0xc3, 0x9e, 0xba, 0xde, 0x85, 0x09, 0xe0, 0xae,
     0x3c, 0x9c, 0x38, 0x76, 0xfb, 0x5f, 0xa1, 0x12, 0xbe, 0x18, 0xf9,
     0x05, 0xec, 0xac, 0xfe, 0xcb, 0x92, 0x05, 0x76, 0x03, 0xab},
    {0x95, 0xee, 0xc8, 0xb2, 0xe5, 0x41, 0xca, 0xd4, 0xe9, 0x1d, 0xe3,
     0x83, 0x85, 0xf2, 0xe0, 0x46, 0x61, 0x9f, 0x54, 0x49, 0x6c, 0x23,
     0x82, 0xcb, 0x6c, 0xac, 0xd5, 0xb9, 0x8c, 0x26, 0xf5, 0xa4},
    {0xf8, 0x93, 0xe9, 0x08, 0x91, 0x77, 0x75, 0xb6, 0x2b, 0xff, 0x23,
     0x29, 0x4d, 0xbb, 0xe3, 0xa1, 0xcd, 0x8e, 0x6c, 0xc1, 0xc3, 0x5b,
     0x48, 0x01, 0x88, 0x7b, 0x64, 0x6a, 0x6f, 0x81, 0xf1, 0x7f},
    {0xcd, 0xdb, 0xa7, 0xb5, 0x92, 0xe3, 0x13, 0x33, 0x93, 0xc1, 0x61,
     0x94, 0xfa, 0xc7, 0x43, 0x1a, 0xbf, 0x2f, 0x54, 0x85, 0xed, 0x71,
     0x1d, 0xb2, 0x82, 0x18, 0x3c, 0x81, 0x9e, 0x08, 0xeb, 0xaa},
    {0x8a, 0x8d, 0x7f, 0xe3, 0xaf, 0x8c, 0xaa, 0x08, 0x5a, 0x76, 0x39,
     0xa8, 0x32, 0x00, 0x14, 0x57, 0xdf, 0xb9, 0x12, 0x8a, 0x80, 0x61,
     0x14, 0x2a, 0xd0, 0x33, 0x56, 0x29, 0xff, 0x23, 0xff, 0x9c},
    {0xfe, 0xb3, 0xc3, 0x37, 0xd7, 0xa5, 0x1a, 0x6f, 0xbf, 0x00, 0xb9,
     0xe3, 0x4c, 0x52, 0xe1, 0xc9, 0x19, 0x5c, 0x96, 0x9b, 0xd4, 0xe7,
     0xa0, 0xbf, 0xd5, 0x1d, 0x5c, 0x5b, 0xed, 0x9c, 0x11, 0x67},
    {0xe7, 0x1f, 0x0a, 0xa8, 0x3c, 0xc3, 0x2e, 0xdf, 0xbe, 0xfa, 0x9f,
     0x4d, 0x3e, 0x01, 0x74, 0xca, 0x85, 0x18, 0x2e, 0xec, 0x9f, 0x3a,
     0x09, 0xf6, 0xa6, 0xc0, 0xdf, 0x63, 0x77, 0xa5, 0x10, 0xd7},
    {0x31, 0x20, 0x6f, 0xa8, 0x0a, 0x50, 0xbb, 0x6a, 0xbe, 0x29, 0x08,
     0x50, 0x58, 0xf1, 0x62, 0x12, 0x21, 0x2a, 0x60, 0xee, 0xc8, 0xf0,
     0x49, 0xfe, 0xcb, 0x92, 0xd8, 0xc8, 0xe0, 0xa8, 0x4b, 0xc0},
    {0x21, 0x35, 0x2b, 0xfe, 0xcb, 0xed, 0xdd, 0xe9, 0x93, 0x83, 0x9f,
     0x61, 0x4c, 0x3d, 0xac, 0x0a, 0x3e, 0xe3, 0x75, 0x43, 0xf9, 0xb4,
     0x12, 0xb1, 0x61, 0x99, 0xdc, 0x15, 0x8e, 0x23, 0xb5, 0x44},
};

static int htr_subspan(const struct htr_span *span, uint64_t off, uint64_t len,
                       struct htr_span *result) {
  if (off > span->len || len > span->len - off)
    return 0;
  result->bytes = span->bytes + off;
  result->len = len;
  return 1;
}

static int htr_resolve(struct StatelessInputSliceFields slice,
                       struct htr_span *result) {
  const uint64_t off = slice.off;
  const uint64_t len = slice.len;
  result->bytes = stateless_input_ptr(off, len);
  if (!result->bytes) return 0;
  result->len = len;
  return 1;
}

static void htr_sha256(const uint8_t *bytes, uint64_t len, htr_root out) {
  static const uint8_t empty[1] = {0};
  _Static_assert(sizeof(htr_root) == sizeof(zkvm_sha256_hash),
                 "accelerator digest must be 32 bytes");
  const uint8_t *input = len ? bytes : empty;
  if (!input || len > UINT32_MAX ||
      zkvm_sha256(input, (size_t)len,
                  (zkvm_sha256_hash *)(void *)out) != ZKVM_EOK) {
    memset(out, 0, sizeof(htr_root));
    return;
  }
}

static void htr_hash_pair(const htr_root left, const htr_root right,
                          htr_root out) {
  uint8_t pair[64];
  memcpy(pair, left, 32);
  memcpy(pair + 32, right, 32);
  htr_sha256(pair, sizeof(pair), out);
}

static int htr_accumulator_init(struct htr_accumulator *acc, unsigned depth) {
  if (depth > HTR_MAX_DEPTH)
    return 0;
  memset(acc, 0, sizeof(*acc));
  acc->depth = depth;
  return 1;
}

static int htr_accumulator_push(struct htr_accumulator *acc,
                                const htr_root leaf) {
  const uint64_t capacity = UINT64_C(1) << acc->depth;
  if (acc->count >= capacity)
    return 0;

  htr_root carry;
  memcpy(carry, leaf, 32);
  uint64_t occupied = acc->count;
  unsigned level = 0;
  while ((occupied & 1) != 0) {
    htr_root parent;
    htr_hash_pair(acc->frontier[level], carry, parent);
    memcpy(carry, parent, 32);
    occupied >>= 1;
    level++;
  }
  memcpy(acc->frontier[level], carry, 32);
  acc->count++;
  return 1;
}

static int htr_accumulator_root(const struct htr_accumulator *acc,
                                htr_root out) {
  uint64_t remaining = acc->count;
  htr_root right = {0};
  bool has_right = false;

  for (unsigned level = 0; level < acc->depth; level++) {
    const bool occupied = (remaining & 1) != 0;
    htr_root next = {0};
    if (occupied) {
      htr_hash_pair(acc->frontier[level],
                    has_right ? right : htr_zero_hashes[level], next);
    } else if (has_right) {
      htr_hash_pair(right, htr_zero_hashes[level], next);
    }
    if (occupied || has_right)
      memcpy(right, next, 32);
    has_right = has_right || occupied;
    remaining >>= 1;
  }

  if (remaining == 1) {
    memcpy(out, acc->frontier[acc->depth], 32);
    return 1;
  }
  if (remaining != 0)
    return 0;
  memcpy(out, has_right ? right : htr_zero_hashes[acc->depth], 32);
  return 1;
}

static int htr_merkleize(const htr_root *leaves, uint64_t count, unsigned depth,
                         htr_root out) {
  struct htr_accumulator acc;
  if (!htr_accumulator_init(&acc, depth))
    return 0;
  for (uint64_t i = 0; i < count; i++) {
    if (!htr_accumulator_push(&acc, leaves[i]))
      return 0;
  }
  return htr_accumulator_root(&acc, out);
}

static void htr_length_chunk(uint64_t len, htr_root out) {
  memset(out, 0, 32);
  for (unsigned i = 0; i < 8; i++)
    out[i] = (uint8_t)(len >> (8 * i));
}

static void htr_mix_in_length(const htr_root root, uint64_t len, htr_root out) {
  htr_root length;
  htr_length_chunk(len, length);
  htr_hash_pair(root, length, out);
}

static unsigned htr_clog2(uint64_t n) {
  unsigned depth = 0;
  uint64_t remaining = n == 0 ? 0 : n - 1;
  while (remaining != 0) {
    remaining >>= 1;
    depth++;
  }
  return depth;
}

static int htr_bytes_root(const uint8_t *bytes, uint64_t len, unsigned depth,
                          htr_root out) {
  struct htr_accumulator acc;
  if (!htr_accumulator_init(&acc, depth))
    return 0;
  const uint64_t chunks = (len + 31) / 32;
  for (uint64_t i = 0; i < chunks; i++) {
    htr_root chunk = {0};
    const uint64_t off = i * 32;
    const uint64_t available = len - off;
    const uint64_t take = available < 32 ? available : 32;
    memcpy(chunk, bytes + off, (size_t)take);
    if (!htr_accumulator_push(&acc, chunk))
      return 0;
  }
  return htr_accumulator_root(&acc, out);
}

static int htr_bytevector(const uint8_t *bytes, uint64_t len, htr_root out) {
  return htr_bytes_root(bytes, len, htr_clog2((len + 31) / 32), out);
}

static int htr_bytelist(const uint8_t *bytes, uint64_t len,
                        uint64_t limit_bytes, htr_root out) {
  if (len > limit_bytes || limit_bytes > EVMSAIL_HTR_BYTE_LIST_LIMIT)
    return 0;
  htr_root root;
  const uint64_t capacity_chunks = (limit_bytes + 31) / 32;
  if (!htr_bytes_root(bytes, len, htr_clog2(capacity_chunks), root))
    return 0;
  htr_mix_in_length(root, len, out);
  return 1;
}

static int htr_bytes32(const struct htr_span *span, uint64_t off,
                       htr_root out) {
  struct htr_span field;
  if (!htr_subspan(span, off, 32, &field))
    return 0;
  memcpy(out, field.bytes, 32);
  return 1;
}

static int htr_uint64(const struct htr_span *span, uint64_t off, htr_root out) {
  struct htr_span field;
  if (!htr_subspan(span, off, 8, &field))
    return 0;
  memset(out, 0, 32);
  memcpy(out, field.bytes, 8);
  return 1;
}

static int htr_address(const struct htr_span *span, uint64_t off,
                       htr_root out) {
  struct htr_span field;
  if (!htr_subspan(span, off, 20, &field))
    return 0;
  memset(out, 0, 32);
  memcpy(out, field.bytes, 20);
  return 1;
}

static int htr_fixed_bytevector(const struct htr_span *span, uint64_t off,
                                uint64_t len, htr_root out) {
  struct htr_span field;
  return htr_subspan(span, off, len, &field) &&
         htr_bytevector(field.bytes, field.len, out);
}

static uint32_t htr_load_u32(const uint8_t *bytes) {
  return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) |
         ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

static int htr_resolve_input(struct htr_input_view *view,
                             struct StatelessInputRef input_ref) {
  view->transaction_count = input_ref.transactions.count;
  view->withdrawal_count = input_ref.withdrawals.count;
  return htr_resolve(input_ref.new_payload_request,
                     &view->new_payload_request) &&
         htr_resolve(input_ref.execution_payload, &view->execution_payload) &&
         htr_resolve(input_ref.versioned_hashes, &view->versioned_hashes) &&
         htr_resolve(input_ref.deposits, &view->deposits) &&
         htr_resolve(input_ref.withdrawal_requests,
                     &view->withdrawal_requests) &&
         htr_resolve(input_ref.consolidation_requests,
                     &view->consolidation_requests) &&
         htr_resolve(input_ref.builder_deposit_requests,
                     &view->builder_deposit_requests) &&
         htr_resolve(input_ref.builder_exit_requests,
                     &view->builder_exit_requests) &&
         htr_resolve(input_ref.extra_data, &view->extra_data) &&
         htr_resolve(input_ref.transactions.bytes, &view->transactions) &&
         htr_resolve(input_ref.withdrawals.bytes, &view->withdrawals) &&
         htr_resolve(input_ref.block_access_list, &view->block_access_list);
}

static int htr_withdrawal(const uint8_t *bytes, htr_root out) {
  const struct htr_span item = {bytes, 44};
  htr_root leaves[4];
  return htr_uint64(&item, 0, leaves[0]) && htr_uint64(&item, 8, leaves[1]) &&
         htr_address(&item, 16, leaves[2]) &&
         htr_uint64(&item, 36, leaves[3]) && htr_merkleize(leaves, 4, 2, out);
}

static int htr_deposit(const uint8_t *bytes, htr_root out) {
  const struct htr_span item = {bytes, 192};
  htr_root leaves[5];
  return htr_fixed_bytevector(&item, 0, 48, leaves[0]) &&
         htr_bytes32(&item, 48, leaves[1]) &&
         htr_uint64(&item, 80, leaves[2]) &&
         htr_fixed_bytevector(&item, 88, 96, leaves[3]) &&
         htr_uint64(&item, 184, leaves[4]) && htr_merkleize(leaves, 5, 3, out);
}

static int htr_withdrawal_request(const uint8_t *bytes, htr_root out) {
  const struct htr_span item = {bytes, 76};
  htr_root leaves[3];
  return htr_address(&item, 0, leaves[0]) &&
         htr_fixed_bytevector(&item, 20, 48, leaves[1]) &&
         htr_uint64(&item, 68, leaves[2]) && htr_merkleize(leaves, 3, 2, out);
}

static int htr_consolidation_request(const uint8_t *bytes, htr_root out) {
  const struct htr_span item = {bytes, 116};
  htr_root leaves[3];
  return htr_address(&item, 0, leaves[0]) &&
         htr_fixed_bytevector(&item, 20, 48, leaves[1]) &&
         htr_fixed_bytevector(&item, 68, 48, leaves[2]) &&
         htr_merkleize(leaves, 3, 2, out);
}

static int htr_builder_deposit_request(const uint8_t *bytes, htr_root out) {
  const struct htr_span item = {bytes, 184};
  htr_root leaves[4];
  return htr_fixed_bytevector(&item, 0, 48, leaves[0]) &&
         htr_bytes32(&item, 48, leaves[1]) &&
         htr_uint64(&item, 80, leaves[2]) &&
         htr_fixed_bytevector(&item, 88, 96, leaves[3]) &&
         htr_merkleize(leaves, 4, 2, out);
}

static int htr_builder_exit_request(const uint8_t *bytes, htr_root out) {
  const struct htr_span item = {bytes, 68};
  htr_root leaves[2];
  return htr_address(&item, 0, leaves[0]) &&
         htr_fixed_bytevector(&item, 20, 48, leaves[1]) &&
         htr_merkleize(leaves, 2, 1, out);
}

enum htr_item_kind {
  HTR_ITEM_DEPOSIT,
  HTR_ITEM_WITHDRAWAL_REQUEST,
  HTR_ITEM_CONSOLIDATION_REQUEST,
  HTR_ITEM_BUILDER_DEPOSIT_REQUEST,
  HTR_ITEM_BUILDER_EXIT_REQUEST,
};

static int htr_item_root(enum htr_item_kind kind, const uint8_t *bytes,
                         htr_root out) {
  switch (kind) {
    case HTR_ITEM_DEPOSIT:
      return htr_deposit(bytes, out);
    case HTR_ITEM_WITHDRAWAL_REQUEST:
      return htr_withdrawal_request(bytes, out);
    case HTR_ITEM_CONSOLIDATION_REQUEST:
      return htr_consolidation_request(bytes, out);
    case HTR_ITEM_BUILDER_DEPOSIT_REQUEST:
      return htr_builder_deposit_request(bytes, out);
    case HTR_ITEM_BUILDER_EXIT_REQUEST:
      return htr_builder_exit_request(bytes, out);
  }
  return 0;
}

static int htr_fixed_list(const struct htr_span *span, uint64_t item_size,
                          unsigned depth, enum htr_item_kind item_kind,
                          htr_root out) {
  if (item_size == 0 || span->len % item_size != 0) {
    return 0;
  }
  const uint64_t count = span->len / item_size;
  struct htr_accumulator acc;
  if (!htr_accumulator_init(&acc, depth))
    return 0;
  for (uint64_t i = 0; i < count; i++) {
    htr_root item;
    if (!htr_item_root(item_kind, span->bytes + i * item_size, item) ||
        !htr_accumulator_push(&acc, item)) {
      return 0;
    }
  }
  htr_root root;
  if (!htr_accumulator_root(&acc, root))
    return 0;
  htr_mix_in_length(root, count, out);
  return 1;
}

static int htr_transactions(const struct htr_span *span, uint64_t count,
                            htr_root out) {
  if (count > (UINT64_C(1) << 20) || count > span->len / 4) {
    return 0;
  }

  struct htr_accumulator acc;
  if (!htr_accumulator_init(&acc, 20))
    return 0;
  for (uint64_t i = 0; i < count; i++) {
    const uint64_t table_off = i * 4;
    const uint64_t start = htr_load_u32(span->bytes + table_off);
    const uint64_t stop =
        i + 1 < count ? htr_load_u32(span->bytes + table_off + 4) : span->len;
    if (start > stop || stop > span->len ||
        stop - start > EVMSAIL_HTR_BYTE_LIST_LIMIT) {
      return 0;
    }
    htr_root item;
    if (!htr_bytelist(span->bytes + start, stop - start,
                      EVMSAIL_HTR_BYTE_LIST_LIMIT,
                      item) ||
        !htr_accumulator_push(&acc, item)) {
      return 0;
    }
  }
  htr_root root;
  if (!htr_accumulator_root(&acc, root))
    return 0;
  htr_mix_in_length(root, count, out);
  return 1;
}

static int htr_withdrawals(const struct htr_span *span, uint64_t count,
                           htr_root out) {
  if (count > 16 || count > UINT64_MAX / 44 || span->len != count * 44)
    return 0;

  struct htr_accumulator acc;
  if (!htr_accumulator_init(&acc, 4))
    return 0;
  for (uint64_t i = 0; i < count; i++) {
    htr_root item;
    if (!htr_withdrawal(span->bytes + i * 44, item) ||
        !htr_accumulator_push(&acc, item)) {
      return 0;
    }
  }
  htr_root root;
  if (!htr_accumulator_root(&acc, root))
    return 0;
  htr_mix_in_length(root, count, out);
  return 1;
}

static int htr_versioned_hashes(const struct htr_span *span, htr_root out) {
  if (span->len % 32 != 0)
    return 0;
  const uint64_t count = span->len / 32;
  struct htr_accumulator acc;
  if (!htr_accumulator_init(&acc, 12))
    return 0;
  for (uint64_t i = 0; i < count; i++) {
    htr_root item;
    memcpy(item, span->bytes + i * 32, 32);
    if (!htr_accumulator_push(&acc, item))
      return 0;
  }
  htr_root root;
  if (!htr_accumulator_root(&acc, root))
    return 0;
  htr_mix_in_length(root, count, out);
  return 1;
}

static int htr_execution_payload(const struct htr_input_view *input,
                                 htr_root out) {
  htr_root leaves[19];
  const struct htr_span *payload = &input->execution_payload;
  return htr_bytes32(payload, 0, leaves[0]) &&
         htr_address(payload, 32, leaves[1]) &&
         htr_bytes32(payload, 52, leaves[2]) &&
         htr_bytes32(payload, 84, leaves[3]) &&
         htr_fixed_bytevector(payload, 116, 256, leaves[4]) &&
         htr_bytes32(payload, 372, leaves[5]) &&
         htr_uint64(payload, 404, leaves[6]) &&
         htr_uint64(payload, 412, leaves[7]) &&
         htr_uint64(payload, 420, leaves[8]) &&
         htr_uint64(payload, 428, leaves[9]) &&
         htr_bytelist(input->extra_data.bytes, input->extra_data.len, 32,
                      leaves[10]) &&
         htr_bytes32(payload, 440, leaves[11]) &&
         htr_bytes32(payload, 472, leaves[12]) &&
         htr_transactions(&input->transactions, input->transaction_count,
                          leaves[13]) &&
         htr_withdrawals(&input->withdrawals, input->withdrawal_count,
                         leaves[14]) &&
         htr_uint64(payload, 512, leaves[15]) &&
         htr_uint64(payload, 520, leaves[16]) &&
         htr_bytelist(input->block_access_list.bytes,
                      input->block_access_list.len,
                      EVMSAIL_HTR_BYTE_LIST_LIMIT,
                      leaves[17]) &&
         htr_uint64(payload, 532, leaves[18]) &&
         htr_merkleize(leaves, 19, 5, out);
}

static int htr_execution_requests(const struct htr_input_view *input,
                                  htr_root out) {
  htr_root leaves[5];
  return htr_fixed_list(&input->deposits, 192, 13, HTR_ITEM_DEPOSIT,
                        leaves[0]) &&
         htr_fixed_list(&input->withdrawal_requests, 76, 4,
                        HTR_ITEM_WITHDRAWAL_REQUEST, leaves[1]) &&
         htr_fixed_list(&input->consolidation_requests, 116, 1,
                        HTR_ITEM_CONSOLIDATION_REQUEST, leaves[2]) &&
         htr_fixed_list(&input->builder_deposit_requests, 184, 6,
                        HTR_ITEM_BUILDER_DEPOSIT_REQUEST, leaves[3]) &&
         htr_fixed_list(&input->builder_exit_requests, 68, 4,
                        HTR_ITEM_BUILDER_EXIT_REQUEST, leaves[4]) &&
         htr_merkleize(leaves, 5, 3, out);
}

Hash32 htr_new_payload_request(
    struct StatelessInputRef input_ref) {
  struct htr_input_view input = {0};
  htr_root leaves[4] = {{0}};
  htr_root root = {0};
  if (htr_resolve_input(&input, input_ref) &&
      htr_execution_payload(&input, leaves[0]) &&
      htr_versioned_hashes(&input.versioned_hashes, leaves[1]) &&
      htr_bytes32(&input.new_payload_request, 8, leaves[2]) &&
      htr_execution_requests(&input, leaves[3]) &&
      htr_merkleize(leaves, 4, 2, root)) {
    return hash_from_be_bytes(root);
  }
  return hash_from_be_bytes(root);
}
