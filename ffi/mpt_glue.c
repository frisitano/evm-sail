/*
 * Optimized MPT construction.
 *
 * The Sail trie equations remain the normative implementation. Optimized C
 * builds use this shape-equivalent fixed-stack builder for transaction,
 * withdrawal, receipt, storage, and account tries. The roots are constructed
 * sequentially, so all of them reuse one fixed 64-level workspace.
 */
#include EVMSAIL_MODEL_H

#include "mpt_glue.h"

#include "byte_slice_glue.h"
#include "code_db.h"
#include "optimized_result.h"
#include "scratch.h"
#include "state_db.h"
#include "trie_node_db.h"
#include "value_convert.h"
#include "zkvm_accelerators.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum {
  SR_OK = 0,
  SR_WITNESS_DEFICIENT = 1,
  SR_RLP_DECODE = 2,
  SR_INVALID_CONFIG = 3,
  SR_PATH_NIBBLES = 64,
  SR_INLINE_MAX = 31,
  SR_VALUE_MAX = 128,
  SR_NODE_MAX = 600,
};

static const uint8_t sr_empty_trie_root[32] = {
    0x56, 0xe8, 0x1f, 0x17, 0x1b, 0xcc, 0x55, 0xa6,
    0xff, 0x83, 0x45, 0xe6, 0x92, 0xc0, 0xf8, 0x6e,
    0x5b, 0x48, 0xe0, 0x1b, 0x99, 0x6c, 0xad, 0xc0,
    0x01, 0x62, 0x2f, 0xb5, 0xe3, 0x63, 0xb4, 0x21,
};

static const uint8_t sr_keccak_empty[32] = {
    0xc5, 0xd2, 0x46, 0x01, 0x86, 0xf7, 0x23, 0x3c,
    0x92, 0x7e, 0x7d, 0xb2, 0xdc, 0xc7, 0x03, 0xc0,
    0xe5, 0x00, 0xb6, 0x53, 0xca, 0x82, 0x27, 0x3b,
    0x7b, 0xfa, 0xd8, 0x04, 0x5d, 0x85, 0xa4, 0x70,
};

static uint64_t sr_last_status;
static bool sr_account_found;
static struct zAccountInfo sr_account_info;
static sail_word sr_storage_value;

struct sr_ctx {
  uint64_t status;
};

struct sr_span {
  const uint8_t *data;
  size_t len;
};

struct sr_path {
  uint8_t nibble[SR_PATH_NIBBLES];
  uint8_t len;
};

enum sr_ref_kind { SR_REF_EMPTY, SR_REF_INLINE, SR_REF_HASH };

struct sr_ref {
  enum sr_ref_kind kind;
  uint8_t len;
  uint8_t bytes[32];
};

enum sr_item_kind { SR_ITEM_LEAF, SR_ITEM_BRANCH, SR_ITEM_SUBTREE };

struct sr_item {
  struct sr_path path;
  enum sr_item_kind kind;
  struct sr_ref ref;
  uint8_t value[SR_VALUE_MAX];
  uint8_t value_len;
  bool generated_value;
  struct zByteSliceFields value_slice;
  bool external_value;
  const uint8_t *external_value_bytes;
  size_t external_value_len;
};

struct sr_update {
  struct sr_path key;
  bool delete;
  uint8_t value[SR_VALUE_MAX];
  uint8_t value_len;
};

struct sr_branch_frame {
  uint8_t depth;
  uint16_t mask;
  struct sr_ref children[16];
};

struct sr_builder {
  struct sr_branch_frame frames[SR_PATH_NIBBLES];
  uint8_t frame_count;
  struct sr_ref root;
  bool complete;
};

struct sr_sink {
  struct sr_builder builder;
  bool pending;
  struct sr_item item;
};

/*
 * One workspace is shared by all sequential storage-trie builds and the final
 * state-trie build. It is reset at each trie_root call.
 */
static struct sr_sink sr_workspace;

/*
 * Receipt execution order is numeric, while trie insertion order is the
 * lexical order of RLP(index). The optimized high-level root operation owns
 * both that ordering and construction of the shared fixed-stack MPT.
 */
struct sr_receipt_entry {
  uint8_t *value;
  size_t value_len;
  size_t value_cap;
};

static struct sr_receipt_entry *sr_receipts;
static size_t sr_receipt_count;
static size_t sr_receipt_cap;

static bool sr_fail(struct sr_ctx *ctx, uint64_t status) {
  if (ctx->status == SR_OK) ctx->status = status;
  return false;
}

static enum zBlockError sr_status_error(uint64_t status) {
  switch (status) {
    case SR_WITNESS_DEFICIENT:
      return zWitnessDeficient;
    case SR_RLP_DECODE:
      return zRlpDecode;
    default:
      return zInvalidConfig;
  }
}

static void sr_unit_result(struct zOptimizzedUnitResult *result,
                           uint64_t status) {
  if (status == SR_OK)
    evmsail_unit_result_ok(result);
  else
    evmsail_unit_result_error(result, sr_status_error(status));
}

static void sr_hash_result(struct zOptimizzedHashResult *result,
                           uint64_t status, sail_hash value) {
  if (status == SR_OK)
    evmsail_hash_result_ok(result, value);
  else
    evmsail_hash_result_error(result, sr_status_error(status));
}

static bool sr_hash_equal(const sail_hash *a, const uint8_t b[32]) {
  return memcmp(a->bytes, b, 32) == 0;
}

static bool sr_word_zero(sail_word value) {
  return (value.limbs[0] | value.limbs[1] | value.limbs[2] |
          value.limbs[3]) == 0;
}

static bool sr_word_equal(sail_word a, sail_word b) {
  return memcmp(&a, &b, sizeof(a)) == 0;
}

static struct sr_ref sr_empty_ref(void) {
  struct sr_ref ref;
  memset(&ref, 0, sizeof(ref));
  ref.kind = SR_REF_EMPTY;
  return ref;
}

static struct sr_ref sr_hash_ref(const uint8_t bytes[32]) {
  struct sr_ref ref = sr_empty_ref();
  ref.kind = SR_REF_HASH;
  ref.len = 32;
  memcpy(ref.bytes, bytes, 32);
  return ref;
}

static struct sr_ref sr_inline_ref(const uint8_t *bytes, size_t len) {
  struct sr_ref ref = sr_empty_ref();
  ref.kind = SR_REF_INLINE;
  ref.len = (uint8_t)len;
  if (len) memcpy(ref.bytes, bytes, len);
  return ref;
}

static bool sr_keccak(struct sr_ctx *ctx, const uint8_t *data, size_t len,
                      uint8_t out[32]) {
  static const uint8_t empty[1] = {0};
  zkvm_keccak256_hash digest = {{0}};
  if (zkvm_keccak256(len ? data : empty, len, &digest) != ZKVM_EOK)
    return sr_fail(ctx, SR_RLP_DECODE);
  memcpy(out, digest.data, 32);
  return true;
}

static struct sr_path sr_path_empty(void) {
  struct sr_path path;
  memset(&path, 0, sizeof(path));
  return path;
}

static struct sr_path sr_path_hash(const sail_hash *hash) {
  struct sr_path path;
  path.len = 64;
  for (size_t i = 0; i < 32; ++i) {
    path.nibble[2 * i] = hash->bytes[i] >> 4;
    path.nibble[2 * i + 1] = hash->bytes[i] & 0x0f;
  }
  return path;
}

static bool sr_path_generated(struct sr_ctx *ctx,
                              const struct zTriePath *generated,
                              struct sr_path *path) {
  if (generated->zlen > SR_PATH_NIBBLES)
    return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  *path = sr_path_empty();
  path->len = (uint8_t)generated->zlen;
  for (size_t i = 0; i < path->len; ++i) {
    const uint8_t byte = generated->zdata.bytes[i / 2];
    path->nibble[i] = (i & 1) ? byte & 0x0f : byte >> 4;
  }
  return true;
}

static struct zTriePath sr_generated_path(const struct sr_path *path) {
  struct zTriePath generated;
  memset(&generated, 0, sizeof(generated));
  generated.zlen = path->len;
  for (size_t i = 0; i < path->len; ++i) {
    if (i & 1)
      generated.zdata.bytes[i / 2] |= path->nibble[i];
    else
      generated.zdata.bytes[i / 2] = (uint8_t)(path->nibble[i] << 4);
  }
  return generated;
}

static bool sr_path_concat(struct sr_ctx *ctx, const struct sr_path *a,
                           const struct sr_path *b, struct sr_path *out) {
  if ((unsigned)a->len + b->len > SR_PATH_NIBBLES)
    return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  *out = sr_path_empty();
  out->len = a->len + b->len;
  memcpy(out->nibble, a->nibble, a->len);
  memcpy(out->nibble + a->len, b->nibble, b->len);
  return true;
}

static struct sr_path sr_path_drop(const struct sr_path *path, unsigned count) {
  struct sr_path out = sr_path_empty();
  if (count >= path->len) return out;
  out.len = path->len - count;
  memcpy(out.nibble, path->nibble + count, out.len);
  return out;
}

static struct sr_path sr_path_slice(const struct sr_path *path, unsigned start,
                                    unsigned len) {
  struct sr_path out = sr_path_empty();
  if (start >= path->len) return out;
  if (len > (unsigned)path->len - start) len = path->len - start;
  out.len = (uint8_t)len;
  memcpy(out.nibble, path->nibble + start, len);
  return out;
}

static struct sr_path sr_path_single(unsigned nibble) {
  struct sr_path out = sr_path_empty();
  out.len = 1;
  out.nibble[0] = (uint8_t)nibble;
  return out;
}

static bool sr_path_equal(const struct sr_path *a, const struct sr_path *b) {
  return a->len == b->len && memcmp(a->nibble, b->nibble, a->len) == 0;
}

static bool sr_path_less(const struct sr_path *a, const struct sr_path *b) {
  const size_t n = a->len < b->len ? a->len : b->len;
  const int order = memcmp(a->nibble, b->nibble, n);
  return order < 0 || (order == 0 && a->len < b->len);
}

static bool sr_path_prefix(const struct sr_path *prefix,
                           const struct sr_path *path) {
  return prefix->len <= path->len &&
         memcmp(prefix->nibble, path->nibble, prefix->len) == 0;
}

static unsigned sr_path_common(const struct sr_path *a,
                               const struct sr_path *b) {
  const unsigned stop = a->len < b->len ? a->len : b->len;
  unsigned common = 0;
  while (common < stop && a->nibble[common] == b->nibble[common]) ++common;
  return common;
}

struct sr_writer {
  uint8_t *data;
  size_t len;
  size_t cap;
};

static bool sr_write(struct sr_ctx *ctx, struct sr_writer *writer,
                     const void *data, size_t len) {
  if (len > writer->cap - writer->len)
    return sr_fail(ctx, SR_RLP_DECODE);
  memcpy(writer->data + writer->len, data, len);
  writer->len += len;
  return true;
}

static size_t sr_uint_be(uint64_t value, uint8_t out[8]) {
  size_t len = 0;
  while (value) {
    out[7 - len++] = (uint8_t)value;
    value >>= 8;
  }
  if (len) memmove(out, out + 8 - len, len);
  return len;
}

static bool sr_rlp_prefix(struct sr_ctx *ctx, struct sr_writer *writer,
                          bool list, size_t len) {
  uint8_t prefix[9];
  if (len <= 55) {
    prefix[0] = (uint8_t)((list ? 0xc0 : 0x80) + len);
    return sr_write(ctx, writer, prefix, 1);
  }
  uint8_t length[8];
  const size_t length_len = sr_uint_be((uint64_t)len, length);
  prefix[0] = (uint8_t)((list ? 0xf7 : 0xb7) + length_len);
  memcpy(prefix + 1, length, length_len);
  return sr_write(ctx, writer, prefix, 1 + length_len);
}

static size_t sr_rlp_string_size(const uint8_t *data, size_t len) {
  if (len == 1 && data[0] < 0x80) return 1;
  if (len <= 55) return 1 + len;
  uint8_t ignored[8];
  return 1 + sr_uint_be((uint64_t)len, ignored) + len;
}

static size_t sr_rlp_prefix_size(size_t len) {
  if (len <= 55) return 1;
  uint8_t ignored[8];
  return 1 + sr_uint_be((uint64_t)len, ignored);
}

static bool sr_rlp_string(struct sr_ctx *ctx, struct sr_writer *writer,
                          const uint8_t *data, size_t len) {
  if (len == 1 && data[0] < 0x80) return sr_write(ctx, writer, data, 1);
  return sr_rlp_prefix(ctx, writer, false, len) &&
         sr_write(ctx, writer, data, len);
}

static size_t sr_compact_path(const struct sr_path *path, bool leaf,
                              uint8_t out[33]) {
  const bool odd = (path->len & 1) != 0;
  size_t pos = 0;
  unsigned index = 0;
  out[pos++] = (uint8_t)(((leaf ? 2 : 0) | (odd ? 1 : 0)) << 4);
  if (odd) {
    out[0] |= path->nibble[0];
    index = 1;
  }
  while (index < path->len) {
    out[pos++] = (uint8_t)((path->nibble[index] << 4) |
                           path->nibble[index + 1]);
    index += 2;
  }
  return pos;
}

static size_t sr_ref_size(const struct sr_ref *ref) {
  if (ref->kind == SR_REF_EMPTY) return 1;
  if (ref->kind == SR_REF_INLINE) return ref->len;
  return 33;
}

static bool sr_write_ref(struct sr_ctx *ctx, struct sr_writer *writer,
                         const struct sr_ref *ref) {
  static const uint8_t empty = 0x80;
  if (ref->kind == SR_REF_EMPTY)
    return sr_write(ctx, writer, &empty, 1);
  if (ref->kind == SR_REF_INLINE)
    return sr_write(ctx, writer, ref->bytes, ref->len);
  return sr_rlp_string(ctx, writer, ref->bytes, 32);
}

static bool sr_child_ref(struct sr_ctx *ctx, const uint8_t *encoded, size_t len,
                         struct sr_ref *out) {
  if (len < 32) {
    *out = sr_inline_ref(encoded, len);
    return true;
  }
  uint8_t digest[32];
  if (!sr_keccak(ctx, encoded, len, digest)) return false;
  *out = sr_hash_ref(digest);
  return true;
}

static bool sr_leaf_ref(struct sr_ctx *ctx, const struct sr_path *path,
                        const uint8_t *value, size_t value_len,
                        struct sr_ref *out) {
  uint8_t compact[33], encoded[SR_NODE_MAX];
  const size_t compact_len = sr_compact_path(path, true, compact);
  const size_t content = sr_rlp_string_size(compact, compact_len) +
                         sr_rlp_string_size(value, value_len);
  struct sr_writer writer = {encoded, 0, sizeof(encoded)};
  if (!sr_rlp_prefix(ctx, &writer, true, content) ||
      !sr_rlp_string(ctx, &writer, compact, compact_len) ||
      !sr_rlp_string(ctx, &writer, value, value_len))
    return false;
  return sr_child_ref(ctx, encoded, writer.len, out);
}

/*
 * Variable-length receipt leaves can be much larger than the fixed-size
 * account/storage encodings. Borrow unused space above the executor scratch
 * high-water mark so all sequential MPT roots share the arena that already
 * framed the receipt value. The borrow does not alter Sail's visible cursor.
 */
static bool sr_large_leaf_ref(struct sr_ctx *ctx, const struct sr_path *path,
                              const uint8_t *value, size_t value_len,
                              struct sr_ref *out) {
  uint8_t compact[33];
  const size_t compact_len = sr_compact_path(path, true, compact);
  const size_t path_size = sr_rlp_string_size(compact, compact_len);
  const size_t value_size = sr_rlp_string_size(value, value_len);
  if (path_size > SIZE_MAX - value_size)
    return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  const size_t content = path_size + value_size;
  const size_t prefix = sr_rlp_prefix_size(content);
  if (prefix > SIZE_MAX - content)
    return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  const size_t encoded_len = prefix + content;
  uint8_t *encoded = scratch_borrow(encoded_len);
  if (!encoded) return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  struct sr_writer writer = {encoded, 0, encoded_len};
  if (!sr_rlp_prefix(ctx, &writer, true, content) ||
      !sr_rlp_string(ctx, &writer, compact, compact_len) ||
      !sr_rlp_string(ctx, &writer, value, value_len))
    return false;
  return sr_child_ref(ctx, encoded, writer.len, out);
}

static bool sr_extension_ref(struct sr_ctx *ctx, const struct sr_path *path,
                             const struct sr_ref *child, struct sr_ref *out) {
  if (path->len == 0) {
    *out = *child;
    return true;
  }
  uint8_t compact[33], encoded[SR_NODE_MAX];
  const size_t compact_len = sr_compact_path(path, false, compact);
  const size_t content =
      sr_rlp_string_size(compact, compact_len) + sr_ref_size(child);
  struct sr_writer writer = {encoded, 0, sizeof(encoded)};
  if (!sr_rlp_prefix(ctx, &writer, true, content) ||
      !sr_rlp_string(ctx, &writer, compact, compact_len) ||
      !sr_write_ref(ctx, &writer, child))
    return false;
  return sr_child_ref(ctx, encoded, writer.len, out);
}

static bool sr_branch_ref(struct sr_ctx *ctx,
                          const struct sr_branch_frame *frame,
                          struct sr_ref *out) {
  uint8_t encoded[SR_NODE_MAX];
  size_t content = 1;
  for (unsigned i = 0; i < 16; ++i)
    content += (frame->mask & (uint16_t)(1u << i))
                   ? sr_ref_size(&frame->children[i])
                   : 1;
  struct sr_writer writer = {encoded, 0, sizeof(encoded)};
  if (!sr_rlp_prefix(ctx, &writer, true, content)) return false;
  for (unsigned i = 0; i < 16; ++i) {
    if (frame->mask & (uint16_t)(1u << i)) {
      if (!sr_write_ref(ctx, &writer, &frame->children[i])) return false;
    } else {
      const uint8_t empty = 0x80;
      if (!sr_write(ctx, &writer, &empty, 1)) return false;
    }
  }
  const uint8_t empty = 0x80;
  if (!sr_write(ctx, &writer, &empty, 1)) return false;
  return sr_child_ref(ctx, encoded, writer.len, out);
}

struct sr_rlp_item {
  struct sr_span raw;
  struct sr_span content;
  bool list;
};

static bool sr_decode_length(struct sr_ctx *ctx, const uint8_t *data,
                             size_t len, size_t width, size_t *value) {
  if (width == 0 || width > sizeof(size_t) || width > len || data[0] == 0)
    return sr_fail(ctx, SR_RLP_DECODE);
  size_t result = 0;
  for (size_t i = 0; i < width; ++i) {
    if (result > (SIZE_MAX >> 8)) return sr_fail(ctx, SR_RLP_DECODE);
    result = (result << 8) | data[i];
  }
  *value = result;
  return true;
}

static bool sr_rlp_pop(struct sr_ctx *ctx, struct sr_span *cursor,
                       struct sr_rlp_item *item) {
  if (cursor->len == 0) return sr_fail(ctx, SR_RLP_DECODE);
  const uint8_t first = cursor->data[0];
  size_t header = 0, content = 0;
  bool list = false;
  if (first <= 0x7f) {
    header = 0;
    content = 1;
  } else if (first <= 0xb7) {
    header = 1;
    content = first - 0x80;
    if (content == 1 && cursor->len > 1 && cursor->data[1] < 0x80)
      return sr_fail(ctx, SR_RLP_DECODE);
  } else if (first <= 0xbf) {
    const size_t width = first - 0xb7;
    if (cursor->len < 1 + width ||
        !sr_decode_length(ctx, cursor->data + 1, cursor->len - 1, width,
                          &content))
      return false;
    if (content <= 55) return sr_fail(ctx, SR_RLP_DECODE);
    header = 1 + width;
  } else if (first <= 0xf7) {
    list = true;
    header = 1;
    content = first - 0xc0;
  } else {
    list = true;
    const size_t width = first - 0xf7;
    if (cursor->len < 1 + width ||
        !sr_decode_length(ctx, cursor->data + 1, cursor->len - 1, width,
                          &content))
      return false;
    if (content <= 55) return sr_fail(ctx, SR_RLP_DECODE);
    header = 1 + width;
  }
  if (header > cursor->len || content > cursor->len - header)
    return sr_fail(ctx, SR_RLP_DECODE);
  item->raw.data = cursor->data;
  item->raw.len = header + content;
  item->content.data = cursor->data + header;
  item->content.len = content;
  item->list = list;
  cursor->data += item->raw.len;
  cursor->len -= item->raw.len;
  return true;
}

static bool sr_decode_compact(struct sr_ctx *ctx,
                              const struct sr_rlp_item *field, bool *leaf,
                              struct sr_path *path) {
  if (field->list) return sr_fail(ctx, SR_RLP_DECODE);
  *path = sr_path_empty();
  if (field->content.len == 0) {
    *leaf = false;
    return true;
  }
  if (field->content.len > 33) return sr_fail(ctx, SR_RLP_DECODE);
  const uint8_t first = field->content.data[0];
  const uint8_t flag = first >> 4;
  *leaf = (flag & 2) != 0;
  const bool odd = (flag & 1) != 0;
  if (odd) path->nibble[path->len++] = first & 0x0f;
  for (size_t i = 1; i < field->content.len; ++i) {
    if (path->len > 62) return sr_fail(ctx, SR_WITNESS_DEFICIENT);
    path->nibble[path->len++] = field->content.data[i] >> 4;
    path->nibble[path->len++] = field->content.data[i] & 0x0f;
  }
  return true;
}

static bool sr_field_ref(struct sr_ctx *ctx,
                         const struct sr_rlp_item *field,
                         struct sr_ref *out) {
  if (field->list) {
    if (field->raw.len >= 32)
      return sr_fail(ctx, SR_WITNESS_DEFICIENT);
    *out = sr_inline_ref(field->raw.data, field->raw.len);
  } else if (field->content.len == 32) {
    *out = sr_hash_ref(field->content.data);
  } else {
    *out = sr_empty_ref();
  }
  return true;
}

enum sr_node_kind { SR_NODE_LEAF, SR_NODE_EXTENSION, SR_NODE_BRANCH };

struct sr_node {
  enum sr_node_kind kind;
  struct sr_path path;
  struct sr_span value;
  struct sr_ref child;
  struct sr_ref children[16];
};

static bool sr_decode_node(struct sr_ctx *ctx, struct sr_span encoded,
                           struct sr_node *node) {
  struct sr_rlp_item outer;
  struct sr_span root = encoded;
  if (!sr_rlp_pop(ctx, &root, &outer) || root.len != 0 || !outer.list)
    return sr_fail(ctx, SR_RLP_DECODE);
  struct sr_span fields = outer.content;
  struct sr_rlp_item item[17];
  unsigned count = 0;
  while (fields.len && count < 17) {
    if (!sr_rlp_pop(ctx, &fields, &item[count++])) return false;
  }
  if (fields.len != 0) return sr_fail(ctx, SR_RLP_DECODE);
  memset(node, 0, sizeof(*node));
  if (count == 2) {
    bool leaf;
    if (!sr_decode_compact(ctx, &item[0], &leaf, &node->path)) return false;
    if (leaf) {
      node->kind = SR_NODE_LEAF;
      node->value = item[1].content;
      return true;
    }
    if (node->path.len == 0) return sr_fail(ctx, SR_RLP_DECODE);
    node->kind = SR_NODE_EXTENSION;
    return sr_field_ref(ctx, &item[1], &node->child);
  }
  if (count != 17) return sr_fail(ctx, SR_RLP_DECODE);
  node->kind = SR_NODE_BRANCH;
  for (unsigned i = 0; i < 16; ++i)
    if (!sr_field_ref(ctx, &item[i], &node->children[i])) return false;
  node->value = item[16].content;
  return true;
}

static bool sr_resolve_ref(struct sr_ctx *ctx, const struct sr_ref *ref,
                           struct sr_span *out) {
  if (ref->kind == SR_REF_EMPTY) {
    out->data = NULL;
    out->len = 0;
    return true;
  }
  if (ref->kind == SR_REF_INLINE) {
    out->data = ref->bytes;
    out->len = ref->len;
    return true;
  }
  sail_hash hash;
  uint64_t off = 0, len = 0;
  memcpy(hash.bytes, ref->bytes, 32);
  if (!nodedb_lookup_span(hash, &off, &len) || len == 0)
    return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  const uint8_t *bytes = evmsail_stateless_input_ptr(off, len);
  if (!bytes) return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  out->data = bytes;
  out->len = (size_t)len;
  return true;
}

static bool sr_lookup_root(struct sr_ctx *ctx, const sail_hash *root,
                           struct sr_span *out) {
  struct sr_ref ref = sr_hash_ref(root->bytes);
  return sr_resolve_ref(ctx, &ref, out);
}

static uint8_t sr_hash_nibble(const sail_hash *key, unsigned position) {
  const uint8_t byte = key->bytes[position / 2];
  return (position & 1) ? byte & 0x0f : byte >> 4;
}

static bool sr_key_matches(const sail_hash *key, unsigned position,
                           const struct sr_path *path) {
  if (position + path->len > SR_PATH_NIBBLES) return false;
  for (unsigned i = 0; i < path->len; ++i)
    if (path->nibble[i] != sr_hash_nibble(key, position + i)) return false;
  return true;
}

/*
 * Authenticated secure-trie point read over the input-backed node database.
 * This is the iterative counterpart of trie_lookup/trie_walk in the Sail
 * specification. It deliberately returns a borrowed leaf-value span.
 */
static bool sr_point_lookup(struct sr_ctx *ctx, const sail_hash *root,
                            const sail_hash *key, struct sr_span *value) {
  value->data = NULL;
  value->len = 0;
  if (sr_hash_equal(root, sr_empty_trie_root)) return true;

  struct sr_span encoded;
  if (!sr_lookup_root(ctx, root, &encoded)) return false;
  unsigned position = 0;
  while (encoded.len != 0) {
    struct sr_node node;
    if (!sr_decode_node(ctx, encoded, &node)) return false;
    if (node.kind == SR_NODE_LEAF) {
      if (position + node.path.len == SR_PATH_NIBBLES &&
          sr_key_matches(key, position, &node.path))
        *value = node.value;
      return true;
    }
    if (node.kind == SR_NODE_EXTENSION) {
      if (!sr_key_matches(key, position, &node.path)) return true;
      position += node.path.len;
      if (!sr_resolve_ref(ctx, &node.child, &encoded)) return false;
      continue;
    }
    if (position == SR_PATH_NIBBLES) {
      *value = node.value;
      return true;
    }
    const unsigned nibble = sr_hash_nibble(key, position++);
    if (node.children[nibble].kind == SR_REF_EMPTY) return true;
    if (!sr_resolve_ref(ctx, &node.children[nibble], &encoded)) return false;
  }
  return true;
}

static bool sr_uint_canonical(struct sr_ctx *ctx,
                              const struct sr_rlp_item *item,
                              size_t maximum) {
  if (item->list || item->content.len > maximum ||
      (item->content.len != 0 && item->content.data[0] == 0))
    return sr_fail(ctx, SR_RLP_DECODE);
  return true;
}

static bool sr_decode_u64_item(struct sr_ctx *ctx,
                               const struct sr_rlp_item *item,
                               uint64_t *value) {
  if (!sr_uint_canonical(ctx, item, 8)) return false;
  *value = 0;
  for (size_t i = 0; i < item->content.len; ++i)
    *value = (*value << 8) | item->content.data[i];
  return true;
}

static bool sr_decode_word_item(struct sr_ctx *ctx,
                                const struct sr_rlp_item *item,
                                sail_word *value) {
  if (!sr_uint_canonical(ctx, item, 32)) return false;
  uint8_t bytes[32] = {0};
  memcpy(bytes + 32 - item->content.len, item->content.data,
         item->content.len);
  *value = be_bytes_to_sail_word(bytes);
  return true;
}

static bool sr_decode_hash_item(struct sr_ctx *ctx,
                                const struct sr_rlp_item *item,
                                const uint8_t empty[32], sail_hash *value) {
  if (item->list || item->content.len > 32)
    return sr_fail(ctx, SR_RLP_DECODE);
  if (item->content.len == 0) {
    memcpy(value->bytes, empty, 32);
    return true;
  }
  memset(value->bytes, 0, 32);
  memcpy(value->bytes + 32 - item->content.len, item->content.data,
         item->content.len);
  return true;
}

static bool sr_decode_account_value(struct sr_ctx *ctx, struct sr_span value,
                                    struct zAccountInfo *account) {
  struct sr_rlp_item outer, field[4];
  struct sr_span root = value;
  if (!sr_rlp_pop(ctx, &root, &outer) || root.len != 0 || !outer.list)
    return sr_fail(ctx, SR_RLP_DECODE);
  struct sr_span fields = outer.content;
  for (unsigned i = 0; i < 4; ++i)
    if (!sr_rlp_pop(ctx, &fields, &field[i])) return false;
  if (fields.len != 0) return sr_fail(ctx, SR_RLP_DECODE);
  return sr_decode_u64_item(ctx, &field[0], &account->znonce) &&
         sr_decode_word_item(ctx, &field[1], &account->zbalance) &&
         sr_decode_hash_item(ctx, &field[2], sr_empty_trie_root,
                             &account->zstorage_root) &&
         sr_decode_hash_item(ctx, &field[3], sr_keccak_empty,
                             &account->zcode_hash);
}

static bool sr_node_to_ref(struct sr_ctx *ctx, struct sr_span node,
                           struct sr_ref *out) {
  if (node.len == 0) {
    *out = sr_empty_ref();
    return true;
  }
  return sr_child_ref(ctx, node.data, node.len, out);
}

static bool sr_merge_ext_ref(struct sr_ctx *ctx,
                             const struct sr_path *prefix,
                             const struct sr_ref *child,
                             struct sr_ref *out) {
  if (prefix->len == 0 || child->kind == SR_REF_EMPTY) {
    *out = *child;
    return true;
  }
  if (child->kind == SR_REF_HASH)
    return sr_extension_ref(ctx, prefix, child, out);
  struct sr_span encoded = {child->bytes, child->len};
  struct sr_node node;
  if (!sr_decode_node(ctx, encoded, &node)) return false;
  if (node.kind == SR_NODE_LEAF) {
    struct sr_path merged;
    if (!sr_path_concat(ctx, prefix, &node.path, &merged)) return false;
    return sr_leaf_ref(ctx, &merged, node.value.data, node.value.len, out);
  }
  if (node.kind == SR_NODE_EXTENSION) {
    struct sr_path merged;
    if (!sr_path_concat(ctx, prefix, &node.path, &merged)) return false;
    return sr_extension_ref(ctx, &merged, &node.child, out);
  }
  return sr_extension_ref(ctx, prefix, child, out);
}

static bool sr_merge_ext_node(struct sr_ctx *ctx,
                              const struct sr_path *prefix,
                              struct sr_span child_node,
                              struct sr_ref *out) {
  if (prefix->len == 0) return sr_node_to_ref(ctx, child_node, out);
  if (child_node.len == 0) {
    *out = sr_empty_ref();
    return true;
  }
  struct sr_node node;
  if (!sr_decode_node(ctx, child_node, &node)) return false;
  if (node.kind == SR_NODE_LEAF) {
    struct sr_path merged;
    if (!sr_path_concat(ctx, prefix, &node.path, &merged)) return false;
    return sr_leaf_ref(ctx, &merged, node.value.data, node.value.len, out);
  }
  if (node.kind == SR_NODE_EXTENSION) {
    struct sr_path merged;
    if (!sr_path_concat(ctx, prefix, &node.path, &merged)) return false;
    return sr_extension_ref(ctx, &merged, &node.child, out);
  }
  struct sr_ref child;
  if (!sr_node_to_ref(ctx, child_node, &child)) return false;
  return sr_extension_ref(ctx, prefix, &child, out);
}

static bool sr_generated_leaf_ref(struct sr_ctx *ctx,
                                  struct zTriePath path,
                                  struct zByteSliceFields value,
                                  struct sr_ref *out) {
  struct zNodeRef generated;
  if (path.zlen > SR_PATH_NIBBLES)
    return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  memset(&generated, 0, sizeof(generated));
  zleaf_child_ref(&generated, path, value);
  switch (generated.kind) {
    case Kind_zHashRef:
      *out = sr_hash_ref(generated.variants.zHashRef.bytes);
      return true;
    case Kind_zInlineRef:
      if (generated.variants.zInlineRef.zlen > SR_INLINE_MAX)
        return sr_fail(ctx, SR_RLP_DECODE);
      *out = sr_inline_ref(generated.variants.zInlineRef.zdata.bytes,
                           generated.variants.zInlineRef.zlen);
      return true;
    case Kind_zEmptyRef:
      return sr_fail(ctx, SR_RLP_DECODE);
  }
  return sr_fail(ctx, SR_RLP_DECODE);
}

static bool sr_item_ref(struct sr_ctx *ctx, const struct sr_item *item,
                        unsigned depth, struct sr_ref *out) {
  if (depth > item->path.len)
    return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  const struct sr_path suffix = sr_path_drop(&item->path, depth);
  if (item->kind == SR_ITEM_LEAF && item->generated_value)
    return sr_generated_leaf_ref(ctx, sr_generated_path(&suffix),
                                 item->value_slice, out);
  if (item->kind == SR_ITEM_LEAF && item->external_value)
    return sr_large_leaf_ref(ctx, &suffix, item->external_value_bytes,
                             item->external_value_len, out);
  if (item->kind == SR_ITEM_LEAF)
    return sr_leaf_ref(ctx, &suffix, item->value, item->value_len, out);
  if (suffix.len == 0) {
    *out = item->ref;
    return true;
  }
  if (item->kind == SR_ITEM_SUBTREE && item->ref.kind == SR_REF_HASH) {
    struct sr_span node;
    if (!sr_resolve_ref(ctx, &item->ref, &node)) return false;
    return sr_merge_ext_node(ctx, &suffix, node, out);
  }
  return sr_merge_ext_ref(ctx, &suffix, &item->ref, out);
}

static void sr_builder_reset(struct sr_builder *builder) {
  memset(builder, 0, sizeof(*builder));
  builder->root = sr_empty_ref();
}

static bool sr_builder_push(struct sr_ctx *ctx, struct sr_builder *builder,
                            unsigned depth) {
  if (depth >= 64 || builder->frame_count >= 64)
    return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  struct sr_branch_frame *frame = &builder->frames[builder->frame_count++];
  memset(frame, 0, sizeof(*frame));
  frame->depth = (uint8_t)depth;
  for (unsigned i = 0; i < 16; ++i) frame->children[i] = sr_empty_ref();
  return true;
}

static bool sr_builder_attach(struct sr_ctx *ctx, struct sr_builder *builder,
                              const struct sr_path *path,
                              const struct sr_ref *child) {
  if (builder->frame_count == 0)
    return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  struct sr_branch_frame *frame =
      &builder->frames[builder->frame_count - 1];
  if (path->len <= frame->depth)
    return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  const unsigned index = path->nibble[frame->depth];
  const uint16_t bit = (uint16_t)(1u << index);
  if (frame->mask & bit) return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  frame->mask |= bit;
  frame->children[index] = *child;
  return true;
}

static bool sr_wrap_branch(struct sr_ctx *ctx, const struct sr_path *anchor,
                           unsigned parent_depth, unsigned child_depth,
                           const struct sr_ref *child, struct sr_ref *out) {
  const unsigned child_start = parent_depth + 1;
  if (child_depth <= child_start) {
    *out = *child;
    return true;
  }
  const struct sr_path gap =
      sr_path_slice(anchor, child_start, child_depth - child_start);
  return sr_extension_ref(ctx, &gap, child, out);
}

static bool sr_builder_close(struct sr_ctx *ctx, struct sr_builder *builder,
                             const struct sr_path *anchor, bool has_common,
                             unsigned next_common) {
  unsigned fuel = 64;
  while (builder->frame_count) {
    struct sr_branch_frame *frame =
        &builder->frames[builder->frame_count - 1];
    if (has_common && next_common >= frame->depth) return true;
    if (fuel-- == 0) return sr_fail(ctx, SR_WITNESS_DEFICIENT);

    const unsigned frame_depth = frame->depth;
    struct sr_ref child;
    if (!sr_branch_ref(ctx, frame, &child)) return false;
    --builder->frame_count;

    if (builder->frame_count) {
      struct sr_branch_frame *parent =
          &builder->frames[builder->frame_count - 1];
      if (has_common && parent->depth < next_common) {
        struct sr_ref wrapped;
        if (!sr_wrap_branch(ctx, anchor, next_common, frame_depth, &child,
                            &wrapped) ||
            !sr_builder_push(ctx, builder, next_common) ||
            !sr_builder_attach(ctx, builder, anchor, &wrapped))
          return false;
      } else {
        struct sr_ref wrapped;
        if (!sr_wrap_branch(ctx, anchor, parent->depth, frame_depth, &child,
                            &wrapped) ||
            !sr_builder_attach(ctx, builder, anchor, &wrapped))
          return false;
      }
    } else if (has_common) {
      struct sr_ref wrapped;
      if (!sr_wrap_branch(ctx, anchor, next_common, frame_depth, &child,
                          &wrapped) ||
          !sr_builder_push(ctx, builder, next_common) ||
          !sr_builder_attach(ctx, builder, anchor, &wrapped))
        return false;
    } else {
      if (frame_depth == 0) {
        builder->root = child;
      } else {
        const struct sr_path prefix = sr_path_slice(anchor, 0, frame_depth);
        if (!sr_extension_ref(ctx, &prefix, &child, &builder->root))
          return false;
      }
      builder->complete = true;
    }
  }
  return true;
}

static bool sr_builder_insert(struct sr_ctx *ctx, struct sr_builder *builder,
                              const struct sr_item *item,
                              const struct sr_path *next_key) {
  if (builder->complete) return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  bool has_common = next_key != NULL;
  unsigned common = 0;
  if (has_common) {
    common = sr_path_common(&item->path, next_key);
    if (!sr_path_less(&item->path, next_key) || item->path.len <= common ||
        next_key->len <= common)
      return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  }
  const bool open_child =
      has_common &&
      (builder->frame_count == 0 ||
       builder->frames[builder->frame_count - 1].depth < common);
  if (open_child) {
    struct sr_ref child;
    if (!sr_item_ref(ctx, item, common + 1, &child) ||
        !sr_builder_push(ctx, builder, common) ||
        !sr_builder_attach(ctx, builder, &item->path, &child))
      return false;
    return true;
  }
  if (builder->frame_count == 0) {
    if (!sr_item_ref(ctx, item, 0, &builder->root)) return false;
    builder->complete = true;
    return true;
  }
  const unsigned depth =
      builder->frames[builder->frame_count - 1].depth + 1;
  struct sr_ref child;
  if (!sr_item_ref(ctx, item, depth, &child) ||
      !sr_builder_attach(ctx, builder, &item->path, &child))
    return false;
  return sr_builder_close(ctx, builder, &item->path, has_common, common);
}

static bool sr_sink_emit(struct sr_ctx *ctx, struct sr_sink *sink,
                         const struct sr_item *item) {
  if (sink->pending &&
      !sr_builder_insert(ctx, &sink->builder, &sink->item, &item->path))
    return false;
  sink->item = *item;
  sink->pending = true;
  return true;
}

static bool sr_sink_finish(struct sr_ctx *ctx, struct sr_sink *sink,
                           sail_hash *root) {
  if (sink->pending) {
    if (!sr_builder_insert(ctx, &sink->builder, &sink->item, NULL))
      return false;
    sink->pending = false;
  }
  if (sink->builder.frame_count)
    return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  if (!sink->builder.complete) {
    memcpy(root->bytes, sr_empty_trie_root, 32);
    return true;
  }
  if (sink->builder.root.kind == SR_REF_HASH) {
    memcpy(root->bytes, sink->builder.root.bytes, 32);
    return true;
  }
  if (sink->builder.root.kind == SR_REF_EMPTY) {
    memcpy(root->bytes, sr_empty_trie_root, 32);
    return true;
  }
  return sr_keccak(ctx, sink->builder.root.bytes, sink->builder.root.len,
                   root->bytes);
}

static bool sr_make_leaf_item(struct sr_ctx *ctx, struct sr_item *item,
                              const struct sr_path *path, const uint8_t *value,
                              size_t value_len) {
  if (value_len > SR_VALUE_MAX)
    return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  memset(item, 0, sizeof(*item));
  item->path = *path;
  item->kind = SR_ITEM_LEAF;
  item->value_len = (uint8_t)value_len;
  if (value_len) memcpy(item->value, value, value_len);
  return true;
}

static struct sr_item sr_ref_item(const struct sr_path *path,
                                  enum sr_item_kind kind,
                                  const struct sr_ref *ref) {
  struct sr_item item;
  memset(&item, 0, sizeof(item));
  item.path = *path;
  item.kind = kind;
  item.ref = *ref;
  return item;
}

static bool sr_rlp_uint64(struct sr_ctx *ctx, struct sr_writer *writer,
                          uint64_t value) {
  uint8_t bytes[8];
  const size_t len = sr_uint_be(value, bytes);
  return sr_rlp_string(ctx, writer, bytes, len);
}

static bool sr_rlp_word(struct sr_ctx *ctx, struct sr_writer *writer,
                        sail_word value) {
  uint8_t bytes[32];
  sail_word_to_be_bytes(bytes, value);
  size_t first = 0;
  while (first < 32 && bytes[first] == 0) ++first;
  return sr_rlp_string(ctx, writer, bytes + first, 32 - first);
}

static size_t sr_rlp_uint64_size(uint64_t value) {
  uint8_t bytes[8];
  const size_t len = sr_uint_be(value, bytes);
  return sr_rlp_string_size(bytes, len);
}

static size_t sr_rlp_word_size(sail_word value) {
  uint8_t bytes[32];
  sail_word_to_be_bytes(bytes, value);
  size_t first = 0;
  while (first < 32 && bytes[first] == 0) ++first;
  return sr_rlp_string_size(bytes + first, 32 - first);
}

static bool sr_encode_storage(struct sr_ctx *ctx, sail_word value,
                              uint8_t out[SR_VALUE_MAX], uint8_t *out_len) {
  struct sr_writer writer = {out, 0, SR_VALUE_MAX};
  if (!sr_rlp_word(ctx, &writer, value)) return false;
  *out_len = (uint8_t)writer.len;
  return true;
}

static bool sr_encode_account(struct sr_ctx *ctx, uint64_t nonce,
                              sail_word balance, const sail_hash *storage_root,
                              const sail_hash *code_hash,
                              uint8_t out[SR_VALUE_MAX], uint8_t *out_len) {
  const size_t content = sr_rlp_uint64_size(nonce) +
                         sr_rlp_word_size(balance) + 33 + 33;
  struct sr_writer writer = {out, 0, SR_VALUE_MAX};
  if (!sr_rlp_prefix(ctx, &writer, true, content) ||
      !sr_rlp_uint64(ctx, &writer, nonce) ||
      !sr_rlp_word(ctx, &writer, balance) ||
      !sr_rlp_string(ctx, &writer, storage_root->bytes, 32) ||
      !sr_rlp_string(ctx, &writer, code_hash->bytes, 32))
    return false;
  *out_len = (uint8_t)writer.len;
  return true;
}

enum sr_source_kind { SR_SOURCE_STORAGE, SR_SOURCE_ACCOUNTS };

struct sr_source {
  enum sr_source_kind kind;
  sail_address address;
  uint32_t position;
  uint32_t count;
};

struct sr_cursor {
  struct sr_source source;
  bool pending;
  struct sr_update update;
};

static bool sr_storage_next(struct sr_ctx *ctx, struct sr_source *source,
                            struct sr_update *update, bool *found) {
  sail_word slot, current, original;
  sail_hash address_hash, slot_hash;
  *found = false;
  while (source->position < source->count &&
         storage_block_update_probe_at(source->position++, &slot, &current,
                                       &original, &address_hash, &slot_hash)) {
    (void)slot;
    (void)address_hash;
    if (sr_word_equal(current, original)) continue;
    memset(update, 0, sizeof(*update));
    update->key = sr_path_hash(&slot_hash);
    update->delete = sr_word_zero(current);
    if (!update->delete &&
        !sr_encode_storage(ctx, current, update->value, &update->value_len))
      return false;
    *found = true;
    return true;
  }
  return true;
}

static bool sr_account_empty(uint64_t nonce, sail_word balance,
                             const sail_hash *code_hash) {
  return nonce == 0 && sr_word_zero(balance) &&
         sr_hash_equal(code_hash, sr_keccak_empty);
}

static bool sr_account_fields_changed(
    uint64_t current_nonce, sail_word current_balance,
    const sail_hash *current_storage, const sail_hash *current_code,
    bool current_exists, bool current_storage_cleared, uint64_t original_nonce,
    sail_word original_balance, const sail_hash *original_storage,
    const sail_hash *original_code, bool original_exists,
    bool original_storage_cleared) {
  return current_nonce != original_nonce ||
         !sr_word_equal(current_balance, original_balance) ||
         memcmp(current_storage->bytes, original_storage->bytes, 32) != 0 ||
         memcmp(current_code->bytes, original_code->bytes, 32) != 0 ||
         current_exists != original_exists ||
         current_storage_cleared != original_storage_cleared;
}

static bool sr_account_next(struct sr_ctx *ctx, struct sr_source *source,
                            struct sr_update *update,
                            bool *found) {
  *found = false;
  while (source->position < source->count) {
    sail_address address;
    uint64_t cn = 0, on = 0;
    sail_word cb = {{0}}, ob = {{0}};
    sail_hash cs = {{0}}, cc = {{0}}, os = {{0}}, oc = {{0}};
    sail_hash address_hash = {{0}}, storage_root = {{0}};
    bool ce = false, csc = false, ccr = false, csd = false;
    bool oe = false, osc = false, ocr = false, osd = false;
    if (!acct_block_update_probe_at(
            source->position++, &address, &cn, &cb, &cs, &cc, &ce, &csc,
            &ccr, &csd, &on, &ob, &os, &oc, &oe, &osc, &ocr, &osd,
            &address_hash, &storage_root))
      return sr_fail(ctx, SR_INVALID_CONFIG);
    (void)ccr;
    (void)csd;
    (void)ocr;
    (void)osd;

    const bool storage_changed =
        memcmp(storage_root.bytes, cs.bytes, sizeof(storage_root.bytes)) != 0;
    if (!storage_changed &&
        !sr_account_fields_changed(cn, cb, &cs, &cc, ce, csc, on, ob, &os,
                                   &oc, oe, osc))
      continue;

    memset(update, 0, sizeof(*update));
    update->key = sr_path_hash(&address_hash);
    update->delete = !ce || sr_account_empty(cn, cb, &cc);
    if (!update->delete &&
        !sr_encode_account(ctx, cn, cb, &storage_root, &cc, update->value,
                           &update->value_len))
      return false;
    *found = true;
    return true;
  }
  return true;
}

static bool sr_source_next(struct sr_ctx *ctx, struct sr_source *source,
                           struct sr_update *update, bool *found) {
  if (source->kind == SR_SOURCE_STORAGE)
    return sr_storage_next(ctx, source, update, found);
  return sr_account_next(ctx, source, update, found);
}

static bool sr_cursor_begin(struct sr_ctx *ctx, struct sr_source source,
                            struct sr_cursor *cursor) {
  memset(cursor, 0, sizeof(*cursor));
  cursor->source = source;
  return sr_source_next(ctx, &cursor->source, &cursor->update,
                        &cursor->pending);
}

static bool sr_cursor_advance(struct sr_ctx *ctx, struct sr_cursor *cursor) {
  return sr_source_next(ctx, &cursor->source, &cursor->update,
                        &cursor->pending);
}

static bool sr_next_update_under(const struct sr_cursor *cursor,
                                 const struct sr_path *prefix) {
  return cursor->pending && sr_path_prefix(prefix, &cursor->update.key);
}

static bool sr_emit_update(struct sr_ctx *ctx, struct sr_sink *sink,
                           const struct sr_update *update) {
  if (update->delete) return true;
  struct sr_item item;
  if (!sr_make_leaf_item(ctx, &item, &update->key, update->value,
                         update->value_len))
    return false;
  return sr_sink_emit(ctx, sink, &item);
}

static bool sr_emit_live_under(struct sr_ctx *ctx, struct sr_sink *sink,
                               struct sr_cursor *cursor,
                               const struct sr_path *prefix) {
  while (sr_next_update_under(cursor, prefix)) {
    if (!sr_emit_update(ctx, sink, &cursor->update) ||
        !sr_cursor_advance(ctx, cursor))
      return false;
  }
  return true;
}

static bool sr_emit_before_child(struct sr_ctx *ctx, struct sr_sink *sink,
                                 struct sr_cursor *cursor,
                                 const struct sr_path *prefix,
                                 const struct sr_path *child) {
  while (cursor->pending && sr_path_prefix(prefix, &cursor->update.key) &&
         !sr_path_prefix(child, &cursor->update.key) &&
         sr_path_less(&cursor->update.key, child)) {
    if (!sr_emit_update(ctx, sink, &cursor->update) ||
        !sr_cursor_advance(ctx, cursor))
      return false;
  }
  return true;
}

static bool sr_emit_leaf_overlay(struct sr_ctx *ctx, struct sr_sink *sink,
                                 struct sr_cursor *cursor,
                                 const struct sr_path *prefix,
                                 const struct sr_path *key,
                                 struct sr_span value) {
  for (;;) {
    if (!cursor->pending ||
        !sr_path_prefix(prefix, &cursor->update.key)) {
      struct sr_item item;
      return sr_make_leaf_item(ctx, &item, key, value.data, value.len) &&
             sr_sink_emit(ctx, sink, &item);
    }
    if (sr_path_equal(&cursor->update.key, key)) {
      if (!sr_emit_update(ctx, sink, &cursor->update) ||
          !sr_cursor_advance(ctx, cursor))
        return false;
      return sr_emit_live_under(ctx, sink, cursor, prefix);
    }
    if (sr_path_less(key, &cursor->update.key)) {
      struct sr_item item;
      if (!sr_make_leaf_item(ctx, &item, key, value.data, value.len) ||
          !sr_sink_emit(ctx, sink, &item))
        return false;
      return sr_emit_live_under(ctx, sink, cursor, prefix);
    }
    if (!sr_emit_update(ctx, sink, &cursor->update) ||
        !sr_cursor_advance(ctx, cursor))
      return false;
  }
}

static bool sr_witness_emit(struct sr_ctx *ctx, struct sr_span encoded,
                            const struct sr_path *prefix,
                            struct sr_cursor *cursor, struct sr_sink *sink,
                            unsigned depth) {
  if (encoded.len == 0)
    return sr_emit_live_under(ctx, sink, cursor, prefix);
  struct sr_node node;
  if (!sr_decode_node(ctx, encoded, &node)) return false;

  if (node.kind == SR_NODE_LEAF) {
    struct sr_path key;
    if (!sr_path_concat(ctx, prefix, &node.path, &key)) return false;
    return sr_emit_leaf_overlay(ctx, sink, cursor, prefix, &key, node.value);
  }

  if (node.kind == SR_NODE_EXTENSION) {
    const unsigned next_depth = depth + node.path.len;
    if (node.path.len == 0 || next_depth > 64)
      return sr_fail(ctx, SR_WITNESS_DEFICIENT);
    struct sr_path child_prefix;
    if (!sr_path_concat(ctx, prefix, &node.path, &child_prefix) ||
        !sr_emit_before_child(ctx, sink, cursor, prefix, &child_prefix))
      return false;
    if (sr_next_update_under(cursor, &child_prefix)) {
      struct sr_span child;
      if (!sr_resolve_ref(ctx, &node.child, &child)) return false;
      if (child.len == 0) {
        if (!sr_emit_live_under(ctx, sink, cursor, &child_prefix)) return false;
      } else if (!sr_witness_emit(ctx, child, &child_prefix, cursor, sink,
                                  next_depth)) {
        return false;
      }
    } else {
      const struct sr_item item =
          sr_ref_item(&child_prefix, SR_ITEM_BRANCH, &node.child);
      if (!sr_sink_emit(ctx, sink, &item)) return false;
    }
    return sr_emit_live_under(ctx, sink, cursor, prefix);
  }

  if (node.value.len != 0 || depth >= 64)
    return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  const unsigned next_depth = depth + 1;
  for (unsigned nibble = 0; nibble < 16; ++nibble) {
    const struct sr_path one = sr_path_single(nibble);
    struct sr_path child_prefix;
    if (!sr_path_concat(ctx, prefix, &one, &child_prefix)) return false;
    const struct sr_ref *child_ref = &node.children[nibble];
    const bool present = child_ref->kind != SR_REF_EMPTY;
    if (sr_next_update_under(cursor, &child_prefix)) {
      if (present) {
        struct sr_span child;
        if (!sr_resolve_ref(ctx, child_ref, &child) ||
            !sr_witness_emit(ctx, child, &child_prefix, cursor, sink,
                             next_depth))
          return false;
      } else if (!sr_emit_live_under(ctx, sink, cursor, &child_prefix)) {
        return false;
      }
    } else if (present) {
      const struct sr_item item =
          sr_ref_item(&child_prefix, SR_ITEM_SUBTREE, child_ref);
      if (!sr_sink_emit(ctx, sink, &item)) return false;
    }
  }
  return true;
}

static bool sr_trie_root(struct sr_ctx *ctx, const sail_hash *base_root,
                         struct sr_source source, sail_hash *root,
                         bool *changed) {
  struct sr_cursor cursor;
  if (!sr_cursor_begin(ctx, source, &cursor)) return false;
  *changed = cursor.pending;
  if (!cursor.pending) {
    *root = *base_root;
    return true;
  }

  memset(&sr_workspace, 0, sizeof(sr_workspace));
  sr_builder_reset(&sr_workspace.builder);
  if (sr_hash_equal(base_root, sr_empty_trie_root)) {
    const struct sr_path empty = sr_path_empty();
    if (!sr_emit_live_under(ctx, &sr_workspace, &cursor, &empty)) return false;
  } else {
    struct sr_span node;
    const struct sr_path empty = sr_path_empty();
    if (!sr_lookup_root(ctx, base_root, &node) ||
        !sr_witness_emit(ctx, node, &empty, &cursor, &sr_workspace, 0))
      return false;
  }
  if (cursor.pending) return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  return sr_sink_finish(ctx, &sr_workspace, root);
}

static bool sr_prepare_storage_roots(struct sr_ctx *ctx,
                                     uint32_t account_count) {
  for (uint32_t account_index = 0; account_index < account_count;
       account_index++) {
    sail_address address;
    uint64_t cn = 0, on = 0;
    sail_word cb = {{0}}, ob = {{0}};
    sail_hash cs = {{0}}, cc = {{0}}, os = {{0}}, oc = {{0}};
    sail_hash address_hash = {{0}}, ignored_post_storage_root = {{0}};
    bool ce = false, csc = false, ccr = false, csd = false;
    bool oe = false, osc = false, ocr = false, osd = false;
    if (!acct_block_update_probe_at(
            account_index, &address, &cn, &cb, &cs, &cc, &ce, &csc, &ccr,
            &csd, &on, &ob, &os, &oc, &oe, &osc, &ocr, &osd,
            &address_hash, &ignored_post_storage_root))
      return sr_fail(ctx, SR_INVALID_CONFIG);
    (void)on;
    (void)ob;
    (void)os;
    (void)oc;
    (void)oe;
    (void)osc;
    (void)ccr;
    (void)csd;
    (void)ocr;
    (void)osd;
    (void)address_hash;

    sail_hash base = cs;
    if (csc) memcpy(base.bytes, sr_empty_trie_root, 32);
    sail_hash post = base;
    const uint32_t storage_count = storage_block_updates_prepare(address);
    struct sr_source source = {
        SR_SOURCE_STORAGE, address, 0, storage_count};
    struct sr_cursor probe;
    if (!sr_cursor_begin(ctx, source, &probe)) return false;
    if (ce && !sr_account_empty(cn, cb, &cc) && probe.pending) {
      bool changed = false;
      if (!sr_trie_root(ctx, &base, source, &post, &changed)) return false;
      (void)changed;
    }
    acct_block_update_post_storage_store_at(account_index, post);
  }
  return true;
}

unit evmsail_mpt_reset(unit ignored) {
  (void)ignored;
  memset(&sr_workspace, 0, sizeof(sr_workspace));
  sr_builder_reset(&sr_workspace.builder);
  sr_last_status = SR_OK;
  return UNIT;
}

static bool sr_receipt_table_reserve(struct sr_ctx *ctx, size_t need) {
  if (need <= sr_receipt_cap) return true;
  size_t cap = sr_receipt_cap ? sr_receipt_cap : 16;
  while (cap < need) {
    if (cap > SIZE_MAX / 2)
      return sr_fail(ctx, SR_WITNESS_DEFICIENT);
    cap *= 2;
  }
  if (cap > SIZE_MAX / sizeof(*sr_receipts))
    return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  struct sr_receipt_entry *next =
      realloc(sr_receipts, cap * sizeof(*sr_receipts));
  if (!next) return sr_fail(ctx, SR_WITNESS_DEFICIENT);
  memset(next + sr_receipt_cap, 0,
         (cap - sr_receipt_cap) * sizeof(*next));
  sr_receipts = next;
  sr_receipt_cap = cap;
  return true;
}

unit evmsail_receipt_table_reset(unit ignored) {
  (void)ignored;
  evmsail_mpt_reset(UNIT);
  sr_receipt_count = 0;
  return UNIT;
}

void evmsail_receipt_table_push(struct zOptimizzedUnitResult *result,
                                uint64_t index,
                                struct zByteSliceFields value) {
  struct sr_ctx ctx = {sr_last_status};
  const uint64_t off = evmsail_byte_quantity_value(value.zoff);
  const uint64_t len = evmsail_byte_quantity_value(value.zlen);
  const uint8_t *bytes = NULL;
  uint64_t resolved_len = 0;
  if (ctx.status != SR_OK || index != sr_receipt_count ||
      len > SIZE_MAX ||
      !evmsail_resolve_byte_source(evmsail_source_kind(value.zsource), off,
                                   len, &bytes, &resolved_len) ||
      resolved_len != len ||
      !sr_receipt_table_reserve(&ctx, sr_receipt_count + 1)) {
    sr_fail(&ctx, SR_WITNESS_DEFICIENT);
    sr_last_status = ctx.status;
    sr_unit_result(result, ctx.status);
    return;
  }
  struct sr_receipt_entry *entry = &sr_receipts[sr_receipt_count];
  if ((size_t)len > entry->value_cap) {
    uint8_t *next = realloc(entry->value, (size_t)len);
    if (!next) {
      sr_fail(&ctx, SR_WITNESS_DEFICIENT);
      sr_last_status = ctx.status;
      sr_unit_result(result, ctx.status);
      return;
    }
    entry->value = next;
    entry->value_cap = (size_t)len;
  }
  if (len) memcpy(entry->value, bytes, (size_t)len);
  entry->value_len = (size_t)len;
  ++sr_receipt_count;
  sr_last_status = ctx.status;
  sr_unit_result(result, ctx.status);
}

static struct sr_path sr_index_path(uint64_t index) {
  uint8_t encoded[9];
  size_t encoded_len = 0;
  if (index == 0) {
    encoded[encoded_len++] = 0x80;
  } else if (index <= 0x7f) {
    encoded[encoded_len++] = (uint8_t)index;
  } else {
    uint8_t bytes[8];
    const size_t width = sr_uint_be(index, bytes);
    encoded[encoded_len++] = (uint8_t)(0x80 + width);
    memcpy(encoded + encoded_len, bytes, width);
    encoded_len += width;
  }
  struct sr_path path = sr_path_empty();
  path.len = (uint8_t)(encoded_len * 2);
  for (size_t i = 0; i < encoded_len; ++i) {
    path.nibble[2 * i] = encoded[i] >> 4;
    path.nibble[2 * i + 1] = encoded[i] & 0x0f;
  }
  return path;
}

static uint64_t sr_index_at_position(uint64_t count, uint64_t position) {
  const uint64_t rest = count - 1;
  const uint64_t single_count = rest < 127 ? rest : 127;
  if (position < single_count) return position + 1;
  if (position == single_count) return 0;
  return position;
}

static bool sr_resolve_list(struct sr_ctx *ctx,
                            const struct zBoundedSszzListRef *items,
                            struct sr_span *bytes) {
  const uint64_t off = evmsail_byte_quantity_value(items->zbytes.zoff);
  const uint64_t len = evmsail_byte_quantity_value(items->zbytes.zlen);
  const uint8_t *resolved = NULL;
  uint64_t resolved_len = 0;
  if (len > SIZE_MAX ||
      !evmsail_resolve_byte_source(
          evmsail_source_kind(items->zbytes.zsource), off, len, &resolved,
          &resolved_len) ||
      resolved_len != len)
    return sr_fail(ctx, SR_INVALID_CONFIG);
  bytes->data = resolved;
  bytes->len = (size_t)len;
  return true;
}

static bool sr_read_u32_le(struct sr_ctx *ctx, struct sr_span bytes,
                           size_t off, uint32_t *value) {
  if (off > bytes.len || sizeof(uint32_t) > bytes.len - off)
    return sr_fail(ctx, SR_INVALID_CONFIG);
  const uint8_t *p = bytes.data + off;
  *value = (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
  return true;
}

static bool sr_read_u64_le(struct sr_ctx *ctx, struct sr_span bytes,
                           size_t off, uint64_t *value) {
  if (off > bytes.len || sizeof(uint64_t) > bytes.len - off)
    return sr_fail(ctx, SR_INVALID_CONFIG);
  const uint8_t *p = bytes.data + off;
  *value = (uint64_t)p[0] | ((uint64_t)p[1] << 8) |
           ((uint64_t)p[2] << 16) | ((uint64_t)p[3] << 24) |
           ((uint64_t)p[4] << 32) | ((uint64_t)p[5] << 40) |
           ((uint64_t)p[6] << 48) | ((uint64_t)p[7] << 56);
  return true;
}

static bool sr_variable_list_item(struct sr_ctx *ctx,
                                  const struct zBoundedSszzListRef *items,
                                  struct sr_span bytes, uint64_t index,
                                  struct sr_span *item) {
  uint32_t start = 0, stop = 0;
  if (index >= items->zcount || index > SIZE_MAX / 4 ||
      !sr_read_u32_le(ctx, bytes, (size_t)index * 4, &start))
    return sr_fail(ctx, SR_INVALID_CONFIG);
  if (index + 1 < items->zcount) {
    if (!sr_read_u32_le(ctx, bytes, (size_t)(index + 1) * 4, &stop))
      return false;
  } else {
    if (bytes.len > UINT32_MAX) return sr_fail(ctx, SR_INVALID_CONFIG);
    stop = (uint32_t)bytes.len;
  }
  if (start > stop || stop > bytes.len ||
      (items->zmax_item_length != 0 &&
       (uint64_t)(stop - start) > items->zmax_item_length))
    return sr_fail(ctx, SR_INVALID_CONFIG);
  item->data = bytes.data + start;
  item->len = stop - start;
  return true;
}

void evmsail_index_witness_nodes(struct zOptimizzedUnitResult *result,
                                 struct zBoundedSszzListRef nodes) {
  enum {
    SR_MAX_WITNESS_NODES = 1u << 22,
    SR_MAX_WITNESS_NODE_LENGTH = 1u << 10,
  };
  struct sr_ctx ctx = {SR_OK};
  struct sr_span bytes = {0};
  const uint64_t list_off =
      evmsail_byte_quantity_value(nodes.zbytes.zoff);
  if (nodes.zcount > SR_MAX_WITNESS_NODES ||
      !sr_resolve_list(&ctx, &nodes, &bytes)) {
    evmsail_unit_result_error(result, zInvalidConfig);
    return;
  }

  for (uint64_t index = 0; index < nodes.zcount; ++index) {
    struct sr_span node = {0};
    sail_hash digest = {{0}};
    if (!sr_variable_list_item(&ctx, &nodes, bytes, index, &node) ||
        node.len > SR_MAX_WITNESS_NODE_LENGTH ||
        !sr_keccak(&ctx, node.data, node.len, digest.bytes)) {
      evmsail_unit_result_error(result, zInvalidConfig);
      return;
    }
    const uint64_t relative_off = (uint64_t)(node.data - bytes.data);
    if (relative_off > UINT64_MAX - list_off) {
      evmsail_unit_result_error(result, zInvalidConfig);
      return;
    }
    nodedb_insert_digest(&digest, list_off + relative_off, node.len);
  }
  evmsail_unit_result_ok(result);
}

void evmsail_index_witness_codes(struct zOptimizzedUnitResult *result,
                                 struct zBoundedSszzListRef codes,
                                 bool amsterdam_or_later) {
  enum {
    SR_MAX_WITNESS_CODES = 1u << 18,
    SR_MAX_WITNESS_CODE_LENGTH = 1u << 16,
  };
  struct sr_ctx ctx = {SR_OK};
  struct sr_span bytes = {0};
  if (codes.zcount > SR_MAX_WITNESS_CODES ||
      !sr_resolve_list(&ctx, &codes, &bytes)) {
    evmsail_unit_result_error(result, zInvalidConfig);
    return;
  }

  for (uint64_t index = 0; index < codes.zcount; ++index) {
    struct sr_span code = {0};
    if (!sr_variable_list_item(&ctx, &codes, bytes, index, &code) ||
        code.len > SR_MAX_WITNESS_CODE_LENGTH ||
        !code_db_insert_analyzed_bytes(code.data, code.len,
                                       amsterdam_or_later)) {
      evmsail_unit_result_error(result, zInvalidConfig);
      return;
    }
  }
  evmsail_unit_result_ok(result);
}

static bool sr_ordered_insert(struct sr_ctx *ctx, struct sr_item *item,
                              uint64_t count, uint64_t position) {
  const uint64_t index = sr_index_at_position(count, position);
  item->path = sr_index_path(index);
  struct sr_path next;
  const struct sr_path *next_key = NULL;
  if (position + 1 < count) {
    next = sr_index_path(sr_index_at_position(count, position + 1));
    next_key = &next;
  }
  return sr_builder_insert(ctx, &sr_workspace.builder, item, next_key);
}

static void sr_ordered_root_finish(struct sr_ctx *ctx, sail_hash *root) {
  if (ctx->status == SR_OK &&
      !sr_sink_finish(ctx, &sr_workspace, root))
    memset(root, 0, sizeof(*root));
  sr_last_status = ctx->status;
}

void evmsail_transaction_trie_root(struct zOptimizzedHashResult *result,
                                   struct zBoundedSszzListRef transactions) {
  evmsail_mpt_reset(UNIT);
  struct sr_ctx ctx = {SR_OK};
  struct sr_span bytes = {0};
  sail_hash root = {{0}};
  if (transactions.zcount > (UINT64_C(1) << 20) ||
      !sr_resolve_list(&ctx, &transactions, &bytes)) {
    sr_ordered_root_finish(&ctx, &root);
    sr_hash_result(result, ctx.status, root);
    return;
  }
  for (uint64_t position = 0;
       ctx.status == SR_OK && position < transactions.zcount; ++position) {
    const uint64_t index =
        sr_index_at_position(transactions.zcount, position);
    struct sr_span transaction;
    if (!sr_variable_list_item(&ctx, &transactions, bytes, index,
                               &transaction))
      break;
    struct sr_item item;
    memset(&item, 0, sizeof(item));
    item.kind = SR_ITEM_LEAF;
    item.external_value = true;
    item.external_value_bytes = transaction.data;
    item.external_value_len = transaction.len;
    if (!sr_ordered_insert(&ctx, &item, transactions.zcount, position))
      break;
  }
  sr_ordered_root_finish(&ctx, &root);
  sr_hash_result(result, ctx.status, root);
}

void evmsail_withdrawals_trie_root(struct zOptimizzedHashResult *result,
                                   struct zBoundedSszzListRef withdrawals) {
  enum {
    SR_WITHDRAWAL_SIZE = 44,
    SR_WITHDRAWAL_ADDRESS_OFF = 16,
    SR_WITHDRAWAL_ADDRESS_LEN = 20,
    SR_WITHDRAWAL_AMOUNT_OFF = 36,
  };
  evmsail_mpt_reset(UNIT);
  struct sr_ctx ctx = {SR_OK};
  struct sr_span bytes = {0};
  sail_hash root = {{0}};
  if (withdrawals.zcount > 16 ||
      withdrawals.zcount > SIZE_MAX / SR_WITHDRAWAL_SIZE ||
      !sr_resolve_list(&ctx, &withdrawals, &bytes) ||
      bytes.len != (size_t)withdrawals.zcount * SR_WITHDRAWAL_SIZE) {
    sr_fail(&ctx, SR_INVALID_CONFIG);
    sr_ordered_root_finish(&ctx, &root);
    sr_hash_result(result, ctx.status, root);
    return;
  }
  for (uint64_t position = 0;
       ctx.status == SR_OK && position < withdrawals.zcount; ++position) {
    const uint64_t index =
        sr_index_at_position(withdrawals.zcount, position);
    const size_t off = (size_t)index * SR_WITHDRAWAL_SIZE;
    struct sr_span withdrawal = {
        bytes.data + off,
        SR_WITHDRAWAL_SIZE,
    };
    uint64_t withdrawal_index = 0, validator_index = 0, amount = 0;
    if (!sr_read_u64_le(&ctx, withdrawal, 0, &withdrawal_index) ||
        !sr_read_u64_le(&ctx, withdrawal, 8, &validator_index) ||
        !sr_read_u64_le(&ctx, withdrawal, SR_WITHDRAWAL_AMOUNT_OFF, &amount))
      break;
    const uint8_t *address =
        withdrawal.data + SR_WITHDRAWAL_ADDRESS_OFF;
    const size_t content = sr_rlp_uint64_size(withdrawal_index) +
                           sr_rlp_uint64_size(validator_index) +
                           sr_rlp_string_size(address,
                                              SR_WITHDRAWAL_ADDRESS_LEN) +
                           sr_rlp_uint64_size(amount);
    struct sr_item item;
    memset(&item, 0, sizeof(item));
    struct sr_writer writer = {item.value, 0, sizeof(item.value)};
    if (content > 48 ||
        !sr_rlp_prefix(&ctx, &writer, true, content) ||
        !sr_rlp_uint64(&ctx, &writer, withdrawal_index) ||
        !sr_rlp_uint64(&ctx, &writer, validator_index) ||
        !sr_rlp_string(&ctx, &writer, address,
                       SR_WITHDRAWAL_ADDRESS_LEN) ||
        !sr_rlp_uint64(&ctx, &writer, amount))
      break;
    item.kind = SR_ITEM_LEAF;
    item.value_len = (uint8_t)writer.len;
    if (!sr_ordered_insert(&ctx, &item, withdrawals.zcount, position))
      break;
  }
  sr_ordered_root_finish(&ctx, &root);
  sr_hash_result(result, ctx.status, root);
}

void evmsail_receipt_table_root(struct zOptimizzedHashResult *result,
                                uint64_t count) {
  struct sr_ctx ctx = {sr_last_status};
  sail_hash root = {{0}};
  if (ctx.status == SR_OK && count != sr_receipt_count)
    sr_fail(&ctx, SR_WITNESS_DEFICIENT);
  for (uint64_t position = 0; ctx.status == SR_OK && position < count;
       ++position) {
    const uint64_t index = sr_index_at_position(count, position);
    const struct sr_path key = sr_index_path(index);
    struct sr_path next;
    const struct sr_path *next_key = NULL;
    if (position + 1 < count) {
      const uint64_t next_index =
          sr_index_at_position(count, position + 1);
      next = sr_index_path(next_index);
      next_key = &next;
    }
    const struct sr_receipt_entry *entry = &sr_receipts[index];
    struct sr_item item;
    memset(&item, 0, sizeof(item));
    item.path = key;
    item.kind = SR_ITEM_LEAF;
    item.external_value = true;
    item.external_value_bytes = entry->value;
    item.external_value_len = entry->value_len;
    if (!sr_builder_insert(&ctx, &sr_workspace.builder, &item, next_key))
      break;
  }
  if (ctx.status == SR_OK &&
      !sr_sink_finish(&ctx, &sr_workspace, &root))
    memset(&root, 0, sizeof(root));
  sr_last_status = ctx.status;
  sr_hash_result(result, ctx.status, root);
}

static bool sr_insert_generated_leaf(struct zTriePath path,
                                     struct zByteSliceFields value,
                                     const struct zTriePath *next_path) {
  struct sr_ctx ctx = {sr_last_status};
  struct sr_item item;
  struct sr_path next;
  memset(&item, 0, sizeof(item));
  item.kind = SR_ITEM_LEAF;
  item.generated_value = true;
  item.value_slice = value;
  if (ctx.status != SR_OK ||
      !sr_path_generated(&ctx, &path, &item.path) ||
      (next_path != NULL && !sr_path_generated(&ctx, next_path, &next)) ||
      !sr_builder_insert(&ctx, &sr_workspace.builder, &item,
                         next_path != NULL ? &next : NULL)) {
    sr_last_status = ctx.status;
    return false;
  }
  sr_last_status = ctx.status;
  return true;
}

void evmsail_mpt_insert_leaf(struct zOptimizzedUnitResult *result,
                             struct zTriePath path,
                             struct zByteSliceFields value,
                             struct zTriePath next_path) {
  sr_insert_generated_leaf(path, value, &next_path);
  sr_unit_result(result, sr_last_status);
}

void evmsail_mpt_insert_last(struct zOptimizzedUnitResult *result,
                             struct zTriePath path,
                             struct zByteSliceFields value) {
  struct sr_ctx ctx = {sr_last_status};
  struct sr_builder *builder = &sr_workspace.builder;
  if (ctx.status == SR_OK && !builder->complete &&
      builder->frame_count == 0) {
    if (!sr_generated_leaf_ref(&ctx, path, value, &builder->root)) {
      sr_last_status = ctx.status;
      sr_unit_result(result, ctx.status);
      return;
    }
    builder->complete = true;
    sr_last_status = ctx.status;
    sr_unit_result(result, ctx.status);
    return;
  }
  sr_insert_generated_leaf(path, value, NULL);
  sr_unit_result(result, sr_last_status);
}

void evmsail_mpt_root(struct zOptimizzedHashResult *result, unit ignored) {
  (void)ignored;
  struct sr_ctx ctx = {sr_last_status};
  sail_hash root = {{0}};
  if (ctx.status == SR_OK &&
      !sr_sink_finish(&ctx, &sr_workspace, &root))
    memset(&root, 0, sizeof(root));
  sr_last_status = ctx.status;
  sr_hash_result(result, ctx.status, root);
}

uint64_t evmsail_stateless_account_read(sail_hash root,
                                        sail_hash address_hash,
                                        struct zAccountInfo *info,
                                        bool *found) {
  *found = false;
  memset(info, 0, sizeof(*info));
  struct sr_ctx ctx = {SR_OK};
  struct sr_span value;
  if (sr_point_lookup(&ctx, &root, &address_hash, &value) &&
      value.len != 0) {
    *found = true;
    sr_decode_account_value(&ctx, value, info);
  }
  return ctx.status;
}

void evmsail_stateless_account_lookup(struct zOptimizzedUnitResult *result,
                                      sail_hash root,
                                      sail_hash address_hash) {
  const uint64_t status = evmsail_stateless_account_read(
      root, address_hash, &sr_account_info, &sr_account_found);
  sr_unit_result(result, status);
}

bool evmsail_stateless_account_found(unit ignored) {
  (void)ignored;
  return sr_account_found;
}

uint64_t evmsail_stateless_account_nonce(unit ignored) {
  (void)ignored;
  return sr_account_info.znonce;
}

EVMSAIL_WORD_RETURN evmsail_stateless_account_balance(
    EVMSAIL_WORD_RESULT(result) unit ignored) {
  (void)ignored;
  EVMSAIL_RETURN_WORD(result, sr_account_info.zbalance);
}

EVMSAIL_HASH_RETURN evmsail_stateless_account_storage_root(
    EVMSAIL_HASH_RESULT(result) unit ignored) {
  (void)ignored;
  EVMSAIL_RETURN_HASH(result, sr_account_info.zstorage_root);
}

EVMSAIL_HASH_RETURN evmsail_stateless_account_code_hash(
    EVMSAIL_HASH_RESULT(result) unit ignored) {
  (void)ignored;
  EVMSAIL_RETURN_HASH(result, sr_account_info.zcode_hash);
}

uint64_t evmsail_stateless_storage_read(sail_hash root,
                                        sail_hash slot_hash,
                                        sail_word *value_out) {
  memset(value_out, 0, sizeof(*value_out));
  struct sr_ctx ctx = {SR_OK};
  struct sr_span value;
  if (sr_point_lookup(&ctx, &root, &slot_hash, &value) && value.len != 0) {
    struct sr_rlp_item item;
    struct sr_span cursor = value;
    if (sr_rlp_pop(&ctx, &cursor, &item) && cursor.len == 0)
      sr_decode_word_item(&ctx, &item, value_out);
    else if (ctx.status == SR_OK)
      sr_fail(&ctx, SR_RLP_DECODE);
  }
  return ctx.status;
}

void evmsail_stateless_storage_lookup(struct zOptimizzedUnitResult *result,
                                      sail_hash root, sail_hash slot_hash) {
  const uint64_t status =
      evmsail_stateless_storage_read(root, slot_hash, &sr_storage_value);
  sr_unit_result(result, status);
}

EVMSAIL_WORD_RETURN evmsail_stateless_storage_value(
    EVMSAIL_WORD_RESULT(result) unit ignored) {
  (void)ignored;
  EVMSAIL_RETURN_WORD(result, sr_storage_value);
}

void evmsail_compute_state_root(struct zOptimizzedHashResult *result,
                                sail_hash parent_state_root) {
  struct sr_ctx ctx = {SR_OK};
  sail_hash root = {{0}};
  const uint32_t account_count = acct_block_updates_prepare();
  if (sr_prepare_storage_roots(&ctx, account_count)) {
    const struct sr_source accounts = {
        SR_SOURCE_ACCOUNTS, {{0}}, 0, account_count};
    bool changed = false;
    if (!sr_trie_root(&ctx, &parent_state_root, accounts, &root, &changed))
      memset(&root, 0, sizeof(root));
  }
  sr_last_status = ctx.status;
  sr_hash_result(result, ctx.status, root);
}
