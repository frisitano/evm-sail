/*
 * Optimized Merkle Patricia Trie operations.
 *
 * The Sail implementation remains the normative specification. This backend
 * implements the same operations without materializing Sail lists:
 *
 *   - payload tries are built from entries supplied in canonical key order;
 *   - state reads traverse immutable RLP nodes in the stateless input; and
 *   - state-root construction merges ordered semantic updates into that
 *     immutable witness, reusing every unaffected child reference.
 *
 * Witness nodes receive stable NodeIds only when traversal reaches them. A
 * decoded parent keeps the original child-reference pointer and a parallel
 * child-ID slot. Zero means an unresolved non-empty edge, UINT32_MAX means an
 * empty edge, and every other value identifies a materialized witness node.
 * The input allocation outlives the guest, so reference bytes are borrowed
 * directly instead of copied into each decoded node.
 */
#include "evmsail/host/nodes.h"
#include "evmsail/host/state/primitives.h"
#include "evmsail/prelude.h"

#include "lib/mpt/trie.h"
#include "evmsail/spec/exceptions.h"
#include "executor/receipts.h"
#include "host/code/store.h"
#include "lib/mpt/paths.h"
#include "lib/mpt/common.h"
#include "primitives/hash.h"
#include "evmsail/host/scratch.h"
#include "host/state/account.h"
#include "host/state/storage.h"
#include "lib/mpt/codec.h"
#include "lib/mpt/nodes.h"
#include "workspace.h"
#include "primitives/value.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* ======================================================================== */
/* TYPES                                                                    */
/* ======================================================================== */

typedef struct {
  NodeReferenceKind kind;
  uint8_t len;
  uint8_t bytes[32];
} NodeReference;

typedef enum { TRIE_ENTRY_LEAF, TRIE_ENTRY_BRANCH, TRIE_ENTRY_SUBTREE } TrieEntryKind;

typedef enum {
  PENDING_NODE_EMPTY,
  PENDING_NODE_LEAF,
  PENDING_NODE_REFERENCE,
} PendingNodeKind;

/*
 * A not-yet-materialized canonical node. Leaf/extension paths stay logical
 * while branch frames close, so a parent can collapse a single-child branch
 * without decoding a node that this execution just built.
 */
typedef struct {
  PendingNodeKind kind;
  NibblePath path;
  NodeReference reference;
  const uint8_t *value;
  size_t value_len;
} PendingNode;

typedef struct {
  NibblePath path;
  TrieEntryKind kind;
  NodeReference reference;
  bool has_pending;
  PendingNode pending;
  uint8_t value[MPT_ENCODED_VALUE_MAX];
  uint8_t value_len;
  bool external_value;
  const uint8_t *external_value_bytes;
  size_t external_value_len;
} TrieEntry;

typedef struct {
  NibblePath key;
  bool changed;
  bool is_delete;
  uint8_t value[MPT_ENCODED_VALUE_MAX];
  uint8_t value_len;
  bool original_present;
  uint8_t original[MPT_ENCODED_VALUE_MAX];
  const uint8_t *original_data;
  uint8_t original_len;
  /* Stable terminal identity in this trie's immutable witness arena. */
  NodeId terminal_node;
  bool authenticated;
} TrieUpdate;

typedef struct {
  uint8_t depth;
  uint16_t mask;
  NodeReference children[16];
} TrieBranchFrame;

typedef struct {
  TrieBranchFrame frames[MPT_SECURE_KEY_NIBBLES];
  uint8_t frame_count;
  PendingNode root;
  bool complete;
} OrderedTrieBuilder;

typedef struct {
  uint8_t *data;
  size_t len;
  size_t capacity;
} ByteWriter;

/* Shared permutation workspace for state-binding order and update-buffer key
 * order. order holds the indices being sorted; destinations holds the inverse
 * permutation consumed by one cycle-walk application. The storage fields pin
 * the candidate range and generation of the storage trie being ordered. */
typedef struct {
  uint32_t *order;
  uint32_t *destinations;
  uint32_t storage_count;
  StorageGeneration storage_generation;
} UpdateOrderWorkspace;

typedef struct {
  TrieUpdate *entries;
  size_t count;
  size_t capacity;
} TrieUpdateBuffer;

typedef struct {
  TrieEntry *entries;
  size_t count;
  size_t capacity;
} TrieEntryBuffer;

/*
 * Backend cursor for Sail's closed TrieUpdateSource union. The discriminant
 * selects only how the next semantic update is obtained; ordering,
 * authentication, and canonical reduction remain shared below.
 */
typedef enum {
  MPT_UPDATE_SOURCE_STORAGE,
  MPT_UPDATE_SOURCE_ACCOUNTS,
} MptUpdateSourceKind;

/* Closed set of index-ordering domains accepted by the shared heap sorter.
 * The tag defunctionalizes the three comparators without function pointers:
 * state bindings use (terminal NodeId, secure key), while collected updates
 * use their complete nibble paths. */
typedef enum {
  MPT_ORDER_STORAGE_BINDINGS,
  MPT_ORDER_ACCOUNT_BINDINGS,
  MPT_ORDER_TRIE_UPDATES,
} MptOrderKind;

typedef struct {
  MptUpdateSourceKind kind;
  uint32_t position;
  uint32_t count;
  StorageGeneration storage_generation;
} MptUpdateSource;

typedef struct {
  NodeReference original;
  PendingNode replacement;
  bool changed;
} SubtreeMergeResult;

static bool mpt_merge_update_source_node(NodeId base_node, const Hash32 *base_root,
                                         MptUpdateSource source, Hash32 *root, bool *changed);

/* Root of the most recent state-root computation. */
static Hash32 mpt_last_root;

/*
 * One workspace is shared by all sequential storage-trie builds and the final
 * state-trie build. It is reset at each trie_root call.
 */
static OrderedTrieBuilder ordered_trie_workspace;

static bool mpt_variable_list_item(const struct BoundedSszListRef *items, ByteSpan bytes,
                                   uint64_t index, ByteSpan *item);

static bool mpt_word_zero(U256 value)
{
  return (value.limbs[0] | value.limbs[1] | value.limbs[2] | value.limbs[3]) == 0;
}

static NodeReference mpt_empty_reference(void)
{
  NodeReference ref;
  memset(&ref, 0, sizeof(ref));
  ref.kind = NODE_REFERENCE_EMPTY;
  return ref;
}

static NodeReference mpt_hash_reference(const uint8_t bytes[32])
{
  NodeReference ref = mpt_empty_reference();
  ref.kind = NODE_REFERENCE_HASHED;
  ref.len = 32;
  memcpy(ref.bytes, bytes, 32);
  return ref;
}

static NodeReference mpt_inline_reference(const uint8_t *bytes, size_t len)
{
  NodeReference ref = mpt_empty_reference();
  ref.kind = NODE_REFERENCE_INLINE;
  ref.len = (uint8_t)len;
  if (len) {
    memcpy(ref.bytes, bytes, len);
  }
  return ref;
}

/* Copy only when an original edge enters the owned post-state builder. */
static NodeReference mpt_copy_witness_reference(const WitnessChild *ref)
{
  switch (mpt_witness_child_kind(ref)) {
  case NODE_REFERENCE_HASHED:
    return mpt_hash_reference(ref->encoded);
  case NODE_REFERENCE_INLINE:
    return mpt_inline_reference(ref->encoded, ref->encoded_len);
  case NODE_REFERENCE_EMPTY:
    return mpt_empty_reference();
  }
}

static bool mpt_write(ByteWriter *writer, const void *data, size_t len)
{
  if (len > writer->capacity - writer->len) {
    fatal_error(RlpDecode);
  }
  memcpy(writer->data + writer->len, data, len);
  writer->len += len;
  return true;
}

static size_t mpt_uint_be(uint64_t value, uint8_t out[8])
{
  size_t len = 0;
  while (value) {
    out[7 - len++] = (uint8_t)value;
    value >>= 8;
  }
  if (len) {
    memmove(out, out + 8 - len, len);
  }
  return len;
}

static bool mpt_rlp_prefix(ByteWriter *writer, bool list, size_t len)
{
  uint8_t prefix[9];
  if (len <= 55) {
    prefix[0] = (uint8_t)(((int)list ? 0xc0 : 0x80) + len);
    return mpt_write(writer, prefix, 1);
  }
  uint8_t length[8];
  const size_t length_len = mpt_uint_be((uint64_t)len, length);
  prefix[0] = (uint8_t)(((int)list ? 0xf7 : 0xb7) + length_len);
  memcpy(prefix + 1, length, length_len);
  return mpt_write(writer, prefix, 1 + length_len);
}

static size_t mpt_rlp_string_size(const uint8_t *data, size_t len)
{
  if (len == 1 && data[0] < 0x80) {
    return 1;
  }
  if (len <= 55) {
    return 1 + len;
  }
  uint8_t ignored[8];
  return 1 + mpt_uint_be((uint64_t)len, ignored) + len;
}

static size_t mpt_rlp_prefix_size(size_t len)
{
  if (len <= 55) {
    return 1;
  }
  uint8_t ignored[8];
  return 1 + mpt_uint_be((uint64_t)len, ignored);
}

static bool mpt_rlp_string(ByteWriter *writer, const uint8_t *data, size_t len)
{
  if (len == 1 && data[0] < 0x80) {
    return mpt_write(writer, data, 1);
  }
  return (mpt_rlp_prefix(writer, false, len) && mpt_write(writer, data, len)) != 0;
}

static size_t mpt_compact_path(const NibblePath *path, bool leaf, uint8_t out[33])
{
  const bool odd = (path->len & 1) != 0;
  size_t pos = 0;
  unsigned index = 0;
  out[pos++] = (uint8_t)((((int)leaf ? 2 : 0) | ((int)odd ? 1 : 0)) << 4);
  if (odd) {
    out[0] |= path->nibbles[0];
    index = 1;
  }
  while (index < path->len) {
    out[pos++] = (uint8_t)((path->nibbles[index] << 4) | path->nibbles[index + 1]);
    index += 2;
  }
  return pos;
}

static size_t node_reference_size(const NodeReference *ref)
{
  if (ref->kind == NODE_REFERENCE_EMPTY) {
    return 1;
  }
  if (ref->kind == NODE_REFERENCE_INLINE) {
    return ref->len;
  }
  return 33;
}

static bool mpt_write_reference(ByteWriter *writer, const NodeReference *ref)
{
  static const uint8_t empty = 0x80;
  if (ref->kind == NODE_REFERENCE_EMPTY) {
    return mpt_write(writer, &empty, 1);
  }
  if (ref->kind == NODE_REFERENCE_INLINE) {
    return mpt_write(writer, ref->bytes, ref->len);
  }
  return mpt_rlp_string(writer, ref->bytes, 32);
}

static bool mpt_make_child_reference(const uint8_t *encoded, size_t len, NodeReference *out)
{
  if (len < 32) {
    *out = mpt_inline_reference(encoded, len);
    return true;
  }
  Hash32 digest;
  if (!mpt_keccak(encoded, len, &digest)) {
    return false;
  }
  *out = mpt_hash_reference(hash_bytes_const(&digest));
  return true;
}

static bool mpt_encode_leaf_reference(const NibblePath *path, const uint8_t *value,
                                      size_t value_len, NodeReference *out)
{
  uint8_t compact[33];
  uint8_t encoded[MPT_ENCODED_NODE_MAX];
  const size_t compact_len = mpt_compact_path(path, true, compact);
  const size_t content =
      mpt_rlp_string_size(compact, compact_len) + mpt_rlp_string_size(value, value_len);
  ByteWriter writer = {encoded, 0, sizeof(encoded)};
  if (!mpt_rlp_prefix(&writer, true, content) || !mpt_rlp_string(&writer, compact, compact_len) ||
      !mpt_rlp_string(&writer, value, value_len)) {
    return false;
  }
  return mpt_make_child_reference(encoded, writer.len, out);
}

/*
 * Variable-length receipt leaves can be much larger than the fixed-size
 * account/storage encodings. Borrow unused space above the executor scratch
 * high-water mark so all sequential MPT roots share the arena that already
 * framed the receipt value. The borrow does not alter Sail's visible cursor.
 */
static bool mpt_encode_large_leaf_reference(const NibblePath *path, const uint8_t *value,
                                            size_t value_len, NodeReference *out)
{
  uint8_t compact[33];
  const size_t compact_len = mpt_compact_path(path, true, compact);
  const size_t path_size = mpt_rlp_string_size(compact, compact_len);
  const size_t value_size = mpt_rlp_string_size(value, value_len);
  if (path_size > SIZE_MAX - value_size) {
    fatal_error(WitnessDeficient);
  }
  const size_t content = path_size + value_size;
  const size_t prefix = mpt_rlp_prefix_size(content);
  if (prefix > SIZE_MAX - content) {
    fatal_error(WitnessDeficient);
  }
  const size_t encoded_len = prefix + content;
  uint8_t *encoded = scratch_borrow(encoded_len);
  if (!encoded) {
    fatal_error(WitnessDeficient);
  }
  ByteWriter writer = {encoded, 0, encoded_len};
  if (!mpt_rlp_prefix(&writer, true, content) || !mpt_rlp_string(&writer, compact, compact_len) ||
      !mpt_rlp_string(&writer, value, value_len)) {
    return false;
  }
  return mpt_make_child_reference(encoded, writer.len, out);
}

static bool mpt_encode_extension_reference(const NibblePath *path, const NodeReference *child,
                                           NodeReference *out)
{
  if (path->len == 0) {
    *out = *child;
    return true;
  }
  uint8_t compact[33];
  uint8_t encoded[MPT_ENCODED_NODE_MAX];
  const size_t compact_len = mpt_compact_path(path, false, compact);
  const size_t content = mpt_rlp_string_size(compact, compact_len) + node_reference_size(child);
  ByteWriter writer = {encoded, 0, sizeof(encoded)};
  if (!mpt_rlp_prefix(&writer, true, content) || !mpt_rlp_string(&writer, compact, compact_len) ||
      !mpt_write_reference(&writer, child)) {
    return false;
  }
  return mpt_make_child_reference(encoded, writer.len, out);
}

static bool mpt_encode_branch_reference(const TrieBranchFrame *frame, NodeReference *out)
{
  uint8_t encoded[MPT_ENCODED_NODE_MAX];
  size_t content = 1;
  for (unsigned i = 0; i < 16; ++i) {
    content += (frame->mask & (uint16_t)(1U << i)) ? node_reference_size(&frame->children[i]) : 1;
  }
  ByteWriter writer = {encoded, 0, sizeof(encoded)};
  if (!mpt_rlp_prefix(&writer, true, content)) {
    return false;
  }
  for (unsigned i = 0; i < 16; ++i) {
    if (frame->mask & (uint16_t)(1U << i)) {
      if (!mpt_write_reference(&writer, &frame->children[i])) {
        return false;
      }
    } else {
      const uint8_t empty = 0x80;
      if (!mpt_write(&writer, &empty, 1)) {
        return false;
      }
    }
  }
  const uint8_t empty = 0x80;
  if (!mpt_write(&writer, &empty, 1)) {
    return false;
  }
  return mpt_make_child_reference(encoded, writer.len, out);
}

static PendingNode pending_node_empty(void)
{
  PendingNode pending;
  memset(&pending, 0, sizeof(pending));
  pending.kind = PENDING_NODE_EMPTY;
  pending.path = nibble_path_empty();
  pending.reference = mpt_empty_reference();
  return pending;
}

static PendingNode pending_node_reference(const NodeReference *ref)
{
  PendingNode pending = pending_node_empty();
  if (ref->kind != NODE_REFERENCE_EMPTY) {
    pending.kind = PENDING_NODE_REFERENCE;
    pending.reference = *ref;
  }
  return pending;
}

static PendingNode pending_node_leaf(const NibblePath *path, const uint8_t *value, size_t value_len)
{
  PendingNode pending = pending_node_empty();
  pending.kind = PENDING_NODE_LEAF;
  pending.path = *path;
  pending.value = value;
  pending.value_len = value_len;
  return pending;
}

static bool pending_node_prepend(const NibblePath *prefix, const PendingNode *child,
                                 PendingNode *out)
{
  if (prefix->len == 0 || child->kind == PENDING_NODE_EMPTY) {
    *out = *child;
    return true;
  }
  PendingNode result = *child;
  if (!nibble_path_concat(prefix, &child->path, &result.path)) {
    return false;
  }
  *out = result;
  return true;
}

static bool pending_node_finalize(const PendingNode *pending, NodeReference *out)
{
  if (pending->kind == PENDING_NODE_EMPTY) {
    *out = mpt_empty_reference();
    return true;
  }
  if (pending->kind == PENDING_NODE_LEAF) {
    if (pending->value_len > MPT_ENCODED_VALUE_MAX) {
      return mpt_encode_large_leaf_reference(&pending->path, pending->value, pending->value_len,
                                             out);
    }
    return mpt_encode_leaf_reference(&pending->path, pending->value, pending->value_len, out);
  }
  return mpt_encode_extension_reference(&pending->path, &pending->reference, out);
}

/*
 * Resolve an authenticated pre-state reference strictly from the stateless
 * input. Planning uses this path for every original trie edge so an identical
 * post-state node generated earlier cannot suppress a sequential witness item.
 */
static bool mpt_resolve_input_reference(const NodeReference *ref, ByteSpan *out)
{
  if (ref->kind == NODE_REFERENCE_EMPTY) {
    out->data = NULL;
    out->len = 0;
    return true;
  }
  if (ref->kind == NODE_REFERENCE_INLINE) {
    out->data = ref->bytes;
    out->len = ref->len;
    return true;
  }
  const Hash32 hash = hash_from_be_bytes(ref->bytes);
  if (!mpt_node_index_span(&hash, out)) {
    fatal_error(WitnessDeficient);
  }
  return true;
}

static bool mpt_resolve_reference(const NodeReference *ref, ByteSpan *out)
{
  return mpt_resolve_input_reference(ref, out);
}

static bool mpt_node_reference(ByteSpan encoded, uint32_t node_id, NodeReference *out)
{
  if (encoded.len < 32) {
    *out = mpt_inline_reference(encoded.data, encoded.len);
    return true;
  }
  Hash32 digest;
  if (!mpt_node_digest(node_id, &digest)) {
    return false;
  }
  *out = mpt_hash_reference(hash_bytes_const(&digest));
  return true;
}

/*
 * Authenticated secure-trie point read over the input-backed node database.
 * This is the iterative counterpart of trie_lookup/trie_walk in the Sail
 * specification. It deliberately returns a borrowed leaf-value span.
 */
static bool mpt_point_lookup_node(NodeId root_node, const Hash32 *key, ByteSpan *value,
                                  Hash32 *leaf_digest, uint8_t *leaf_postfix_len,
                                  NodeId *terminal_node)
{
  value->data = NULL;
  value->len = 0;
  if (leaf_digest) {
    memset(leaf_digest, 0, sizeof(*leaf_digest));
  }
  if (leaf_postfix_len) {
    *leaf_postfix_len = 0;
  }
  if (terminal_node) {
    *terminal_node = MPT_NODE_ID_EMPTY;
  }
  if (root_node == MPT_NODE_ID_EMPTY) {
    return true;
  }

  ByteSpan encoded;
  NodeId node_id = root_node;
  if (!mpt_node_span(node_id, &encoded)) {
    return false;
  }
  unsigned position = 0;
  while (encoded.len != 0) {
    DecodedNode *node = NULL;
    if (!mpt_decode_cached_node(node_id, &node)) {
      return false;
    }
    if (terminal_node) {
      *terminal_node = node_id;
    }
    if (node->kind == DECODED_NODE_LEAF) {
      const bool matched = (position + node->path.len == MPT_SECURE_KEY_NIBBLES &&
                            mpt_key_matches(key, position, &node->path)) != 0;
      if (matched) {
        if (leaf_digest && !mpt_node_digest(node_id, leaf_digest)) {
          return false;
        }
        *value = node->value;
        if (leaf_postfix_len) {
          *leaf_postfix_len = node->path.len;
        }
      }
      return true;
    }
    if (node->kind == DECODED_NODE_EXTENSION) {
      if (!mpt_key_matches(key, position, &node->path)) {
        return true;
      }
      position += node->path.len;
      WitnessChild child = mpt_decoded_child(node, 0);
      if (!mpt_link_witness_child(&child, &encoded, &node_id)) {
        return false;
      }
      continue;
    }
    if (position == MPT_SECURE_KEY_NIBBLES) {
      *value = node->value;
      return true;
    }
    const unsigned nibble = mpt_hash_nibble(key, position++);
    WitnessChild child = mpt_decoded_child(node, nibble);
    if (mpt_witness_child_kind(&child) == NODE_REFERENCE_EMPTY) {
      return true;
    }
    if (!mpt_link_witness_child(&child, &encoded, &node_id)) {
      return false;
    }
  }
  return true;
}

static bool mpt_point_lookup(const Hash32 *root, const Hash32 *key, ByteSpan *value,
                             Hash32 *leaf_digest, uint8_t *leaf_postfix_len, NodeId *terminal_node)
{
  NodeId root_node = MPT_NODE_ID_UNLINKED;
  if (!mpt_bind_root(root, &root_node)) {
    return false;
  }
  return mpt_point_lookup_node(root_node, key, value, leaf_digest, leaf_postfix_len, terminal_node);
}

static bool mpt_uint_canonical(const RlpItem *item, size_t maximum)
{
  if (item->list || item->content.len > maximum ||
      (item->content.len != 0 && item->content.data[0] == 0)) {
    fatal_error(RlpDecode);
  }
  return true;
}

static bool mpt_decode_u64_item(const RlpItem *item, uint64_t *value)
{
  if (!mpt_uint_canonical(item, 8)) {
    return false;
  }
  *value = 0;
  for (size_t i = 0; i < item->content.len; ++i) {
    *value = (*value << 8) | item->content.data[i];
  }
  return true;
}

static bool mpt_decode_word_item(const RlpItem *item, U256 *value)
{
  if (!mpt_uint_canonical(item, 32)) {
    return false;
  }
  uint8_t bytes[32] = {0};
  memcpy(bytes + 32 - item->content.len, item->content.data, item->content.len);
  *value = be_bytes_to_sail_word(bytes);
  return true;
}

static bool mpt_decode_hash_item(const RlpItem *item, const Hash32 *empty, Hash32 *value)
{
  if (item->list || item->content.len > 32) {
    fatal_error(RlpDecode);
  }
  if (item->content.len == 0) {
    *value = *empty;
    return true;
  }
  uint8_t *bytes = hash_bytes(value);
  memset(bytes, 0, 32);
  memcpy(bytes + 32 - item->content.len, item->content.data, item->content.len);
  return true;
}

static bool mpt_decode_account_value(ByteSpan value, struct AccountInfo *account)
{
  RlpItem outer;
  RlpItem field[4];
  ByteSpan root = value;
  if (!mpt_rlp_pop(&root, &outer) || root.len != 0 || !outer.list) {
    fatal_error(RlpDecode);
  }
  ByteSpan fields = outer.content;
  for (unsigned i = 0; i < 4; ++i) {
    if (!mpt_rlp_pop(&fields, &field[i])) {
      return false;
    }
  }
  if (fields.len != 0) {
    fatal_error(RlpDecode);
  }
  return (mpt_decode_u64_item(&field[0], &account->nonce) &&
          mpt_decode_word_item(&field[1], &account->balance) &&
          mpt_decode_hash_item(&field[2], &EVMSAIL_EMPTY_TRIE_ROOT, &account->storage_root) &&
          mpt_decode_hash_item(&field[3], &EVMSAIL_KECCAK_EMPTY, &account->code_hash)) != 0;
}

static bool mpt_reference_from_encoded_node(ByteSpan node, NodeReference *out)
{
  if (node.len == 0) {
    *out = mpt_empty_reference();
    return true;
  }
  return mpt_make_child_reference(node.data, node.len, out);
}

static bool mpt_node_reference_matches_witness(const NodeReference *left, const WitnessChild *right)
{
  const NodeReferenceKind right_kind = mpt_witness_child_kind(right);
  const size_t right_len = right_kind == NODE_REFERENCE_EMPTY ? 0 : right->encoded_len;
  return (left->kind == right_kind && left->len == right_len &&
          (right_len == 0 || memcmp(left->bytes, right->encoded, right_len) == 0)) != 0;
}

static bool pending_node_from_node(ByteSpan encoded, PendingNode *out)
{
  if (encoded.len == 0) {
    *out = pending_node_empty();
    return true;
  }
  DecodedNode node;
  if (!mpt_decode_trusted_state_node(encoded, &node)) {
    return false;
  }
  if (node.kind == DECODED_NODE_LEAF) {
    *out = pending_node_leaf(&node.path, node.value.data, node.value.len);
    return true;
  }
  if (node.kind == DECODED_NODE_EXTENSION) {
    WitnessChild witness_child = mpt_decoded_child(&node, 0);
    const NodeReference child = mpt_copy_witness_reference(&witness_child);
    *out = pending_node_reference(&child);
    out->path = node.path;
    return true;
  }
  NodeReference ref;
  if (!mpt_reference_from_encoded_node(encoded, &ref)) {
    return false;
  }
  *out = pending_node_reference(&ref);
  return true;
}

static bool trie_entry_pending(const TrieEntry *item, unsigned depth, PendingNode *out)
{
  if (depth > item->path.len) {
    fatal_error(WitnessDeficient);
  }
  const NibblePath suffix = nibble_path_drop(&item->path, depth);
  if (item->kind == TRIE_ENTRY_LEAF) {
    const uint8_t *value = (int)item->external_value ? item->external_value_bytes : item->value;
    const size_t value_len = (int)item->external_value ? item->external_value_len : item->value_len;
    *out = pending_node_leaf(&suffix, value, value_len);
    return true;
  }
  if (item->has_pending) {
    return pending_node_prepend(&suffix, &item->pending, out);
  }
  if (suffix.len == 0) {
    *out = pending_node_reference(&item->reference);
    return true;
  }
  if (item->kind == TRIE_ENTRY_SUBTREE) {
    ByteSpan encoded;
    if (!mpt_resolve_reference(&item->reference, &encoded)) {
      return false;
    }
    PendingNode child;
    if (!pending_node_from_node(encoded, &child)) {
      return false;
    }
    return pending_node_prepend(&suffix, &child, out);
  }
  PendingNode child = pending_node_reference(&item->reference);
  return pending_node_prepend(&suffix, &child, out);
}

static void ordered_trie_builder_reset(OrderedTrieBuilder *builder)
{
  memset(builder, 0, sizeof(*builder));
  builder->root = pending_node_empty();
}

static bool ordered_trie_builder_push(OrderedTrieBuilder *builder, unsigned depth)
{
  if (depth >= 64 || builder->frame_count >= 64) {
    fatal_error(WitnessDeficient);
  }
  TrieBranchFrame *frame = &builder->frames[builder->frame_count++];
  memset(frame, 0, sizeof(*frame));
  frame->depth = (uint8_t)depth;
  for (unsigned i = 0; i < 16; ++i) {
    frame->children[i] = mpt_empty_reference();
  }
  return true;
}

static bool ordered_trie_builder_attach(OrderedTrieBuilder *builder, const NibblePath *path,
                                        const PendingNode *child)
{
  if (builder->frame_count == 0) {
    fatal_error(WitnessDeficient);
  }
  TrieBranchFrame *frame = &builder->frames[builder->frame_count - 1];
  if (path->len <= frame->depth) {
    fatal_error(WitnessDeficient);
  }
  const unsigned index = path->nibbles[frame->depth];
  const uint16_t bit = (uint16_t)(1U << index);
  if (frame->mask & bit) {
    fatal_error(WitnessDeficient);
  }
  NodeReference child_ref;
  if (!pending_node_finalize(child, &child_ref)) {
    return false;
  }
  frame->mask |= bit;
  frame->children[index] = child_ref;
  return true;
}

static bool mpt_wrap_branch(const NibblePath *anchor, unsigned parent_depth, unsigned child_depth,
                            const NodeReference *child, PendingNode *out)
{
  *out = pending_node_reference(child);
  const unsigned child_start = parent_depth + 1;
  if (child_depth <= child_start) {
    return true;
  }
  const NibblePath gap = nibble_path_slice(anchor, child_start, child_depth - child_start);
  return pending_node_prepend(&gap, out, out);
}

static bool ordered_trie_builder_close(OrderedTrieBuilder *builder, const NibblePath *anchor,
                                       bool has_common, unsigned next_common)
{
  unsigned fuel = 64;
  while (builder->frame_count) {
    TrieBranchFrame *frame = &builder->frames[builder->frame_count - 1];
    if (has_common && next_common >= frame->depth) {
      return true;
    }
    if (fuel-- == 0) {
      fatal_error(WitnessDeficient);
    }

    const unsigned frame_depth = frame->depth;
    NodeReference child;
    if (!mpt_encode_branch_reference(frame, &child)) {
      return false;
    }
    --builder->frame_count;

    if (builder->frame_count) {
      TrieBranchFrame *parent = &builder->frames[builder->frame_count - 1];
      if (has_common && parent->depth < next_common) {
        PendingNode wrapped;
        if (!mpt_wrap_branch(anchor, next_common, frame_depth, &child, &wrapped) ||
            !ordered_trie_builder_push(builder, next_common) ||
            !ordered_trie_builder_attach(builder, anchor, &wrapped)) {
          return false;
        }
      } else {
        PendingNode wrapped;
        if (!mpt_wrap_branch(anchor, parent->depth, frame_depth, &child, &wrapped) ||
            !ordered_trie_builder_attach(builder, anchor, &wrapped)) {
          return false;
        }
      }
    } else if (has_common) {
      PendingNode wrapped;
      if (!mpt_wrap_branch(anchor, next_common, frame_depth, &child, &wrapped) ||
          !ordered_trie_builder_push(builder, next_common) ||
          !ordered_trie_builder_attach(builder, anchor, &wrapped)) {
        return false;
      }
    } else {
      if (frame_depth == 0) {
        builder->root = pending_node_reference(&child);
      } else {
        const NibblePath prefix = nibble_path_slice(anchor, 0, frame_depth);
        PendingNode pending = pending_node_reference(&child);
        if (!pending_node_prepend(&prefix, &pending, &builder->root)) {
          return false;
        }
      }
      builder->complete = true;
    }
  }
  return true;
}

static bool ordered_trie_builder_insert(OrderedTrieBuilder *builder, const TrieEntry *item,
                                        const NibblePath *next_key)
{
  if (builder->complete) {
    fatal_error(WitnessDeficient);
  }
  bool has_common = next_key != NULL;
  unsigned common = 0;
  if (has_common) {
    common = nibble_path_common(&item->path, next_key);
    if (!nibble_path_less(&item->path, next_key) || item->path.len <= common ||
        next_key->len <= common) {
      fatal_error(WitnessDeficient);
    }
  }
  const bool open_child =
      (has_common && (builder->frame_count == 0 ||
                      builder->frames[builder->frame_count - 1].depth < common)) != 0;
  if (open_child) {
    PendingNode child;
    if (!trie_entry_pending(item, common + 1, &child) ||
        !ordered_trie_builder_push(builder, common) ||
        !ordered_trie_builder_attach(builder, &item->path, &child)) {
      return false;
    }
    return true;
  }
  if (builder->frame_count == 0) {
    if (!trie_entry_pending(item, 0, &builder->root)) {
      return false;
    }
    builder->complete = true;
    return true;
  }
  const unsigned depth = builder->frames[builder->frame_count - 1].depth + 1;
  PendingNode child;
  if (!trie_entry_pending(item, depth, &child) ||
      !ordered_trie_builder_attach(builder, &item->path, &child)) {
    return false;
  }
  return ordered_trie_builder_close(builder, &item->path, has_common, common);
}

static bool ordered_trie_builder_finish_pending(OrderedTrieBuilder *builder, PendingNode *root)
{
  if (builder->frame_count) {
    fatal_error(WitnessDeficient);
  }
  if (!builder->complete) {
    *root = pending_node_empty();
    return true;
  }
  *root = builder->root;
  return true;
}

static bool ordered_trie_builder_finish_ref(OrderedTrieBuilder *builder, NodeReference *root)
{
  PendingNode pending;
  return (ordered_trie_builder_finish_pending(builder, &pending) &&
          pending_node_finalize(&pending, root)) != 0;
}

static bool ordered_trie_builder_finish(OrderedTrieBuilder *builder, Hash32 *root)
{
  NodeReference ref;
  if (!ordered_trie_builder_finish_ref(builder, &ref)) {
    return false;
  }
  if (ref.kind == NODE_REFERENCE_HASHED) {
    *root = hash_from_be_bytes(ref.bytes);
    return true;
  }
  if (ref.kind == NODE_REFERENCE_EMPTY) {
    *root = EVMSAIL_EMPTY_TRIE_ROOT;
    return true;
  }
  return mpt_keccak(ref.bytes, ref.len, root);
}

static bool mpt_make_borrowed_leaf_entry(TrieEntry *item, const NibblePath *path,
                                         const uint8_t *value, size_t value_len)
{
  if (value_len > MPT_ENCODED_VALUE_MAX) {
    fatal_error(WitnessDeficient);
  }
  memset(item, 0, sizeof(*item));
  item->path = *path;
  item->kind = TRIE_ENTRY_LEAF;
  item->external_value = true;
  item->external_value_bytes = value;
  item->external_value_len = value_len;
  return true;
}

static TrieEntry trie_entry_from_reference(const NibblePath *path, TrieEntryKind kind,
                                           const NodeReference *ref)
{
  TrieEntry item;
  memset(&item, 0, sizeof(item));
  item.path = *path;
  item.kind = kind;
  item.reference = *ref;
  return item;
}

static bool mpt_rlp_uint64(ByteWriter *writer, uint64_t value)
{
  uint8_t bytes[8];
  const size_t len = mpt_uint_be(value, bytes);
  return mpt_rlp_string(writer, bytes, len);
}

static bool mpt_rlp_word(ByteWriter *writer, U256 value)
{
  uint8_t bytes[32];
  sail_word_to_be_bytes(bytes, value);
  size_t first = 0;
  while (first < 32 && bytes[first] == 0) {
    ++first;
  }
  return mpt_rlp_string(writer, bytes + first, 32 - first);
}

static size_t mpt_rlp_uint64_size(uint64_t value)
{
  uint8_t bytes[8];
  const size_t len = mpt_uint_be(value, bytes);
  return mpt_rlp_string_size(bytes, len);
}

static size_t mpt_rlp_word_size(U256 value)
{
  uint8_t bytes[32];
  sail_word_to_be_bytes(bytes, value);
  size_t first = 0;
  while (first < 32 && bytes[first] == 0) {
    ++first;
  }
  return mpt_rlp_string_size(bytes + first, 32 - first);
}

static bool mpt_encode_storage(U256 value, uint8_t out[MPT_ENCODED_VALUE_MAX], uint8_t *out_len)
{
  ByteWriter writer = {out, 0, MPT_ENCODED_VALUE_MAX};
  if (!mpt_rlp_word(&writer, value)) {
    return false;
  }
  *out_len = (uint8_t)writer.len;
  return true;
}

static bool mpt_encode_account(uint64_t nonce, U256 balance, const Hash32 *storage_root,
                               const Hash32 *code_hash, uint8_t out[MPT_ENCODED_VALUE_MAX],
                               uint8_t *out_len)
{
  const size_t content = mpt_rlp_uint64_size(nonce) + mpt_rlp_word_size(balance) + 33 + 33;
  ByteWriter writer = {out, 0, MPT_ENCODED_VALUE_MAX};
  if (!mpt_rlp_prefix(&writer, true, content) || !mpt_rlp_uint64(&writer, nonce) ||
      !mpt_rlp_word(&writer, balance) ||
      !mpt_rlp_string(&writer, hash_bytes_const(storage_root), 32) ||
      !mpt_rlp_string(&writer, hash_bytes_const(code_hash), 32)) {
    return false;
  }
  *out_len = (uint8_t)writer.len;
  return true;
}

static bool mpt_next_storage_update(MptUpdateSource *source, TrieUpdate *update, bool *found)
{
  *found = false;
  while (source->position < source->count) {
    StorageTrieView view;
    if (!storage_trie_binding_get(mpt_storage_update_id_at(source->position++),
                                  source->storage_generation, &view)) {
      return false;
    }
    memset(update, 0, sizeof(*update));
    update->key = nibble_path_from_secure_key(view.secure_key);
    update->changed = memcmp(view.current, view.original, sizeof(*view.current)) != 0;
    if (!update->changed) {
      continue;
    }
    update->is_delete = mpt_word_zero(*view.current);
    if (!update->is_delete &&
        !mpt_encode_storage(*view.current, update->value, &update->value_len)) {
      return false;
    }
    update->original_present = view.prestate_exists;
    update->terminal_node = view.terminal_node;
    if (view.prestate_exists == mpt_word_zero(*view.original)) {
      fatal_error(WitnessDeficient);
    }
    if (update->original_present &&
        !mpt_encode_storage(*view.original, update->original, &update->original_len)) {
      return false;
    }
    *found = true;
    return true;
  }
  return true;
}

static bool mpt_original_account_value(uint32_t terminal_node, ByteSpan *value)
{
  ByteSpan encoded;
  DecodedNode *node = NULL;
  if (!mpt_node_span(terminal_node, &encoded) || encoded.len == 0 ||
      !mpt_decode_cached_node(terminal_node, &node) || node->kind != DECODED_NODE_LEAF) {
    fatal_error(WitnessDeficient);
  }
  *value = node->value;
  return true;
}

static bool mpt_make_account_update(const AccountTrieView *meta, const Hash32 *storage_root,
                                    TrieUpdate *update)
{
  const Hash32 original_storage_root = *meta->original_storage_root;
  memset(update, 0, sizeof(*update));
  update->key = nibble_path_from_secure_key(meta->secure_key);
  update->is_delete = ((!meta->current_live) != 0);
  update->changed =
      ((meta->current_live != meta->original_exists ||
        (meta->current_live &&
         (meta->fields_changed || !hash_equal(storage_root, &original_storage_root)))) != 0);
  update->original_present = meta->original_exists;
  update->terminal_node = meta->terminal_node;
  if (meta->original_exists) {
    ByteSpan original;
    if (!mpt_original_account_value(meta->terminal_node, &original) || original.len == 0 ||
        original.len > UINT8_MAX) {
      fatal_error(WitnessDeficient);
    }
    update->original_data = original.data;
    update->original_len = (uint8_t)original.len;
  }
  if (update->changed && !update->is_delete) {
    if (!mpt_encode_account(meta->nonce, *meta->balance, storage_root, meta->code_hash,
                            update->value, &update->value_len)) {
      return false;
    }
    if (update->original_present && update->value_len == update->original_len &&
        memcmp(update->value, update->original_data, update->value_len) == 0) {
      update->changed = false;
      update->value_len = 0;
    }
  }
  return true;
}

/*
 * Pulls one net-changing update from either source variant. Account updates
 * recursively finalize that account's storage source before encoding the
 * account leaf, matching account_trie_update in sail/lib/state_trie.sail.
 */
/* Account update expansion recursively reduces its bounded storage source. */
// NOLINTNEXTLINE(misc-no-recursion)
static bool mpt_update_source_next(MptUpdateSource *source, TrieUpdate *update, bool *found)
{
  switch (source->kind) {
  case MPT_UPDATE_SOURCE_STORAGE:
    return mpt_next_storage_update(source, update, found);
  case MPT_UPDATE_SOURCE_ACCOUNTS:
    break;
  }

  *found = false;
  while (source->position < source->count) {
    const uint32_t account_index = source->position++;
    AccountTrieView meta;
    if (!account_trie_binding_get(account_index, &meta)) {
      return false;
    }

    Hash32 post = *meta.storage_base_root;
    StorageGeneration storage_generation;
    const uint32_t storage_count =
        mpt_storage_updates_prepare(meta.account_id, &storage_generation);
    if (meta.current_live && storage_count != 0) {
      const MptUpdateSource storage_source = {
          .kind = MPT_UPDATE_SOURCE_STORAGE,
          .position = 0,
          .count = storage_count,
          .storage_generation = storage_generation,
      };
      bool storage_changed = false;
      if (!mpt_merge_update_source_node(meta.storage_base_node, meta.storage_base_root,
                                        storage_source, &post, &storage_changed)) {
        return false;
      }
      (void)storage_changed;
    }

    if (!mpt_make_account_update(&meta, &post, update)) {
      return false;
    }
    if (!update->changed) {
      continue;
    }
    *found = true;
    return true;
  }
  return true;
}

static UpdateOrderWorkspace order_workspace;

/* Ordered semantic updates for the trie currently being reduced. */
static TrieUpdateBuffer mpt_updates;
static uint32_t order_workspace_capacity;

static bool mpt_order_less(MptOrderKind kind, const TrieUpdateBuffer *updates, uint32_t left,
                           uint32_t right)
{
  NodeId left_node;
  NodeId right_node;
  Hash32 left_key;
  Hash32 right_key;
  switch (kind) {
  case MPT_ORDER_ACCOUNT_BINDINGS:
    account_trie_binding_order_key(left, &left_node, &left_key);
    account_trie_binding_order_key(right, &right_node, &right_key);
    break;
  case MPT_ORDER_STORAGE_BINDINGS:
    if (!storage_trie_binding_order_key(left, order_workspace.storage_generation, &left_node,
                                        &left_key) ||
        !storage_trie_binding_order_key(right, order_workspace.storage_generation, &right_node,
                                        &right_key)) {
      GUEST_ABORT();
    }
    break;
  case MPT_ORDER_TRIE_UPDATES:
    if (updates == NULL || left >= updates->count || right >= updates->count) {
      GUEST_ABORT();
    }
    return nibble_path_less(&updates->entries[left].key, &updates->entries[right].key);
  }
  if (left_node != right_node) {
    return left_node < right_node;
  }
  return hash_compare(&left_key, &right_key) < 0;
}

static void mpt_order_sift_down(MptOrderKind kind, const TrieUpdateBuffer *updates, uint32_t start,
                                uint32_t end)
{
  if (end < 2) {
    return;
  }
  uint32_t root = start;
  while (root <= (end - 2U) / 2U) {
    uint32_t child = (2U * root) + 1U;
    if (child + 1U < end && mpt_order_less(kind, updates, order_workspace.order[child],
                                           order_workspace.order[child + 1U])) {
      ++child;
    }
    if (!mpt_order_less(kind, updates, order_workspace.order[root], order_workspace.order[child])) {
      return;
    }
    const uint32_t temporary = order_workspace.order[root];
    order_workspace.order[root] = order_workspace.order[child];
    order_workspace.order[child] = temporary;
    root = child;
  }
}

static void mpt_order_sort(MptOrderKind kind, const TrieUpdateBuffer *updates, uint32_t count)
{
  for (uint32_t start = count / 2U; start != 0; --start) {
    mpt_order_sift_down(kind, updates, start - 1U, count);
  }
  for (uint32_t end = count; end > 1U; --end) {
    const uint32_t temporary = order_workspace.order[0];
    order_workspace.order[0] = order_workspace.order[end - 1U];
    order_workspace.order[end - 1U] = temporary;
    mpt_order_sift_down(kind, updates, 0, end - 1U);
  }
}

static bool mpt_order_is_strict(MptOrderKind kind, const TrieUpdateBuffer *updates, uint32_t count)
{
  for (uint32_t position = 1; position < count; ++position) {
    if (!mpt_order_less(kind, updates, order_workspace.order[position - 1U],
                        order_workspace.order[position])) {
      return false;
    }
  }
  return true;
}

uint32_t mpt_account_updates_prepare(void)
{
  const uint32_t count = account_trie_binding_count();
  if (count > order_workspace_capacity) {
    GUEST_ABORT();
  }
  for (uint32_t index = 0; index < count; ++index) {
    order_workspace.order[index] = index;
  }
  mpt_order_sort(MPT_ORDER_ACCOUNT_BINDINGS, NULL, count);
  if (!mpt_order_is_strict(MPT_ORDER_ACCOUNT_BINDINGS, NULL, count)) {
    GUEST_ABORT();
  }
  for (uint32_t position = 0; position < count; ++position) {
    order_workspace.destinations[order_workspace.order[position]] = position;
  }
  account_trie_bindings_permute(order_workspace.destinations, count);
  return count;
}

uint32_t mpt_storage_updates_prepare(AccountId account_id, StorageGeneration *generation)
{
  StorageId begin;
  const uint32_t candidates =
      storage_trie_candidates(account_id, &begin, &order_workspace.storage_generation);
  *generation = order_workspace.storage_generation;
  if (candidates > order_workspace_capacity) {
    fatal_error(WitnessDeficient);
  }
  uint32_t count = 0;
  for (StorageId id = begin; id < begin + candidates; ++id) {
    NodeId terminal_node;
    Hash32 secure_key;
    if (storage_trie_binding_order_key(id, order_workspace.storage_generation, &terminal_node,
                                       &secure_key)) {
      order_workspace.order[count++] = id;
    }
  }
  mpt_order_sort(MPT_ORDER_STORAGE_BINDINGS, NULL, count);
  if (!mpt_order_is_strict(MPT_ORDER_STORAGE_BINDINGS, NULL, count)) {
    GUEST_ABORT();
  }
  order_workspace.storage_count = count;
  return count;
}

StorageId mpt_storage_update_id_at(uint32_t index)
{
  if (index >= order_workspace.storage_count) {
    GUEST_ABORT();
  }
  return order_workspace.order[index];
}

static bool trie_update_buffer_append(TrieUpdateBuffer *set, const TrieUpdate *update)
{
  if (set->count == set->capacity) {
    fatal_error(WitnessDeficient);
  }
  set->entries[set->count++] = *update;
  return true;
}

/*
 * State tables expose terminal-NodeId order so one account's storage trie can
 * be finalized immediately before its account value is encoded. The current
 * generic reducer still partitions update ranges by secure-key prefix. Sort
 * only compact indices here, then permute its private update buffer once.
 */
static bool trie_update_buffer_order_by_key(TrieUpdateBuffer *set)
{
  if (set->count < 2) {
    return true;
  }
  if (set->count > order_workspace_capacity) {
    fatal_error(WitnessDeficient);
  }

  for (size_t i = 0; i < set->count; ++i) {
    order_workspace.order[i] = (uint32_t)i;
  }

  mpt_order_sort(MPT_ORDER_TRIE_UPDATES, set, (uint32_t)set->count);
  if (!mpt_order_is_strict(MPT_ORDER_TRIE_UPDATES, set, (uint32_t)set->count)) {
    fatal_error(InvalidConfig);
  }

  for (size_t position = 0; position < set->count; ++position) {
    order_workspace.destinations[order_workspace.order[position]] = (uint32_t)position;
  }

  for (size_t position = 0; position < set->count; ++position) {
    while (order_workspace.destinations[position] != position) {
      const size_t destination = order_workspace.destinations[position];
      const TrieUpdate temporary = set->entries[position];
      set->entries[position] = set->entries[destination];
      set->entries[destination] = temporary;

      const uint32_t destination_swap = order_workspace.destinations[position];
      order_workspace.destinations[position] = order_workspace.destinations[destination];
      order_workspace.destinations[destination] = destination_swap;
    }
  }
  return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool mpt_collect_ordered_updates(MptUpdateSource source, TrieUpdateBuffer *updates)
{
  updates->count = 0;
  for (;;) {
    TrieUpdate update;
    bool found = false;
    if (!mpt_update_source_next(&source, &update, &found)) {
      return false;
    }
    if (!found) {
      break;
    }
    if (!trie_update_buffer_append(updates, &update)) {
      return false;
    }
  }
  return trie_update_buffer_order_by_key(updates);
}

/* Reusable frontier of canonical entries for the bottom-up subtree merge. */
static TrieEntryBuffer mpt_merge_entries;

static size_t trie_update_lower_bound(const TrieUpdateBuffer *updates, const NibblePath *prefix)
{
  size_t lo = 0;
  size_t hi = updates->count;
  while (lo < hi) {
    const size_t mid = lo + ((hi - lo) / 2);
    if (nibble_path_less(&updates->entries[mid].key, prefix)) {
      lo = mid + 1;
    } else {
      hi = mid;
    }
  }
  return lo;
}

static void trie_update_range(const TrieUpdateBuffer *updates, const NibblePath *prefix,
                              size_t *begin, size_t *end)
{
  *begin = trie_update_lower_bound(updates, prefix);
  *end = *begin;
  while (*end < updates->count && nibble_path_prefix(prefix, &updates->entries[*end].key)) {
    ++*end;
  }
}

static bool mpt_authenticate_absent(TrieUpdate *update)
{
  if (update->authenticated || update->original_present) {
    fatal_error(WitnessDeficient);
  }
  update->authenticated = true;
  return true;
}

static bool mpt_authenticate_present(TrieUpdate *update, ByteSpan value)
{
  const uint8_t *original =
      update->original_data != NULL ? update->original_data : update->original;
  if (update->authenticated || !update->original_present || update->original_len != value.len ||
      memcmp(original, value.data, value.len) != 0) {
    fatal_error(WitnessDeficient);
  }
  update->authenticated = true;
  return true;
}

static bool mpt_authenticate_terminal_leaf(const TrieUpdate *update, NodeId node_id)
{
  if (update->terminal_node == MPT_NODE_ID_UNLINKED || update->terminal_node != node_id) {
    fatal_error(WitnessDeficient);
  }
  return true;
}

static bool mpt_merge_entry_push(const TrieEntry *item)
{
  if (mpt_merge_entries.count == mpt_merge_entries.capacity) {
    fatal_error(WitnessDeficient);
  }
  mpt_merge_entries.entries[mpt_merge_entries.count++] = *item;
  return true;
}

static bool mpt_merge_add_update(const NibblePath *prefix, const TrieUpdate *update)
{
  if (!update->changed || update->is_delete) {
    return true;
  }
  TrieEntry item;
  const NibblePath relative = nibble_path_drop(&update->key, prefix->len);
  return (mpt_make_borrowed_leaf_entry(&item, &relative, update->value, update->value_len) &&
          mpt_merge_entry_push(&item)) != 0;
}

static bool mpt_merge_add_original_child(const NibblePath *prefix, const NibblePath *absolute,
                                         TrieEntryKind kind, const WitnessChild *ref)
{
  if (mpt_witness_child_kind(ref) == NODE_REFERENCE_EMPTY) {
    return true;
  }
  const NibblePath relative = nibble_path_drop(absolute, prefix->len);
  const NodeReference owned = mpt_copy_witness_reference(ref);
  const TrieEntry item = trie_entry_from_reference(&relative, kind, &owned);
  return mpt_merge_entry_push(&item);
}

static bool mpt_merge_add_rebuilt_child(const NibblePath *prefix, const NibblePath *absolute,
                                        const PendingNode *pending)
{
  if (pending->kind == PENDING_NODE_EMPTY) {
    return true;
  }
  const NibblePath relative = nibble_path_drop(absolute, prefix->len);
  TrieEntry item;
  memset(&item, 0, sizeof(item));
  item.path = relative;
  item.kind = TRIE_ENTRY_SUBTREE;
  item.has_pending = true;
  item.pending = *pending;
  return mpt_merge_entry_push(&item);
}

static bool mpt_build_pending_subtree(size_t begin, PendingNode *root)
{
  for (size_t index = begin + 1; index < mpt_merge_entries.count; ++index) {
    if (!nibble_path_less(&mpt_merge_entries.entries[index - 1].path,
                          &mpt_merge_entries.entries[index].path)) {
      mpt_merge_entries.count = begin;
      fatal_error(WitnessDeficient);
    }
  }

  memset(&ordered_trie_workspace, 0, sizeof(ordered_trie_workspace));
  ordered_trie_builder_reset(&ordered_trie_workspace);
  for (size_t index = begin; index < mpt_merge_entries.count; ++index) {
    const NibblePath *next =
        index + 1 < mpt_merge_entries.count ? &mpt_merge_entries.entries[index + 1].path : NULL;
    if (!ordered_trie_builder_insert(&ordered_trie_workspace, &mpt_merge_entries.entries[index],
                                     next)) {
      mpt_merge_entries.count = begin;
      return false;
    }
  }
  const bool ok = ordered_trie_builder_finish_pending(&ordered_trie_workspace, root);
  mpt_merge_entries.count = begin;
  return ok;
}

static bool mpt_build_subtree_reference(size_t begin, NodeReference *root)
{
  PendingNode pending;
  return (mpt_build_pending_subtree(begin, &pending) && pending_node_finalize(&pending, root)) != 0;
}

static bool mpt_merge_node(ByteSpan encoded, uint32_t node_id, const NibblePath *prefix,
                           TrieUpdateBuffer *updates, SubtreeMergeResult *result);

static bool mpt_merge_decoded_node(DecodedNode *node, ByteSpan encoded, uint32_t node_id,
                                   const NibblePath *prefix, TrieUpdateBuffer *updates,
                                   SubtreeMergeResult *result);

/*
 * Follow one changed edge in the immutable original trie. The first traversal
 * resolves and assigns its stable NodeId; subsequent traversals use the ID
 * stored directly in the parent edge.
 */
static bool mpt_follow_original_child(WitnessChild *expected, ByteSpan *encoded, NodeId *node_id)
{
  if (mpt_witness_child_kind(expected) == NODE_REFERENCE_EMPTY) {
    fatal_error(WitnessDeficient);
  }
  return (mpt_link_witness_child(expected, encoded, node_id) && encoded->len != 0) != 0;
}

/* Trie merging descends a 64-nibble secure-key path, so recursion is bounded. */
// NOLINTNEXTLINE(misc-no-recursion)
static bool mpt_merge_original_child(WitnessChild *original, const NibblePath *prefix,
                                     TrieUpdateBuffer *updates, PendingNode *updated, bool *changed)
{
  ByteSpan encoded;
  uint32_t node_id = MPT_NODE_ID_UNLINKED;
  SubtreeMergeResult reduced;
  if (!mpt_follow_original_child(original, &encoded, &node_id) ||
      !mpt_merge_node(encoded, node_id, prefix, updates, &reduced) ||
      !mpt_node_reference_matches_witness(&reduced.original, original)) {
    fatal_error(WitnessDeficient);
  }
  *changed = reduced.changed;
  *updated = (int)reduced.changed ? reduced.replacement : pending_node_reference(&reduced.original);
  return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool mpt_merge_node(ByteSpan encoded, uint32_t node_id, const NibblePath *prefix,
                           TrieUpdateBuffer *updates, SubtreeMergeResult *result)
{
  DecodedNode *node = NULL;
  if (!mpt_decode_cached_node(node_id, &node)) {
    return false;
  }
  return mpt_merge_decoded_node(node, encoded, node_id, prefix, updates, result);
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool mpt_merge_decoded_node(DecodedNode *node, ByteSpan encoded, uint32_t node_id,
                                   const NibblePath *prefix, TrieUpdateBuffer *updates,
                                   SubtreeMergeResult *result)
{
  memset(result, 0, sizeof(*result));
  if (!mpt_node_reference(encoded, node_id, &result->original)) {
    return false;
  }
  const size_t item_begin = mpt_merge_entries.count;
  bool node_changed = false;

  size_t begin = 0;
  size_t end = 0;
  trie_update_range(updates, prefix, &begin, &end);
  if (node->kind == DECODED_NODE_LEAF) {
    NibblePath key;
    if (!nibble_path_concat(prefix, &node->path, &key) || key.len != MPT_SECURE_KEY_NIBBLES) {
      fatal_error(WitnessDeficient);
    }
    bool retained_original = true;
    for (size_t index = begin; index < end; ++index) {
      TrieUpdate *update = &updates->entries[index];
      if (retained_original && nibble_path_less(&key, &update->key)) {
        TrieEntry item;
        const NibblePath relative = nibble_path_drop(&key, prefix->len);
        if (!mpt_make_borrowed_leaf_entry(&item, &relative, node->value.data, node->value.len) ||
            !mpt_merge_entry_push(&item)) {
          return false;
        }
        retained_original = false;
      }
      if (nibble_path_equal(&update->key, &key)) {
        if (!mpt_authenticate_terminal_leaf(update, node_id) ||
            !mpt_authenticate_present(update, node->value)) {
          return false;
        }
        if (update->changed) {
          retained_original = false;
          node_changed = true;
        }
      } else if (!mpt_authenticate_absent(update)) {
        return false;
      }
      if (!nibble_path_equal(&update->key, &key) && update->changed && !update->is_delete) {
        node_changed = true;
      }
      if (!mpt_merge_add_update(prefix, update)) {
        return false;
      }
    }
    if (retained_original) {
      TrieEntry item;
      const NibblePath relative = nibble_path_drop(&key, prefix->len);
      if (!mpt_make_borrowed_leaf_entry(&item, &relative, node->value.data, node->value.len) ||
          !mpt_merge_entry_push(&item)) {
        return false;
      }
    }
  } else if (node->kind == DECODED_NODE_EXTENSION) {
    NibblePath child_prefix;
    if (!nibble_path_concat(prefix, &node->path, &child_prefix) || node->path.len == 0) {
      fatal_error(WitnessDeficient);
    }
    size_t child_begin = 0;
    size_t child_end = 0;
    trie_update_range(updates, &child_prefix, &child_begin, &child_end);
    const bool child_has_updates = child_begin != child_end;
    WitnessChild original_child = mpt_decoded_child(node, 0);
    const NodeReference owned_child = mpt_copy_witness_reference(&original_child);
    PendingNode child = pending_node_reference(&owned_child);
    if (child_has_updates) {
      bool changed = false;
      if (!mpt_merge_original_child(&original_child, &child_prefix, updates, &child, &changed)) {
        return false;
      }
      node_changed |= changed;
    }
    for (size_t index = begin; index < child_begin; ++index) {
      TrieUpdate *update = &updates->entries[index];
      if (!mpt_authenticate_absent(update) || !mpt_merge_add_update(prefix, update)) {
        return false;
      }
      if (update->changed && !update->is_delete) {
        node_changed = true;
      }
    }
    if (child_has_updates) {
      if (!mpt_merge_add_rebuilt_child(prefix, &child_prefix, &child)) {
        return false;
      }
    } else if (!mpt_merge_add_original_child(prefix, &child_prefix, TRIE_ENTRY_BRANCH,
                                             &original_child)) {
      return false;
    }
    for (size_t index = child_end; index < end; ++index) {
      TrieUpdate *update = &updates->entries[index];
      if (!mpt_authenticate_absent(update) || !mpt_merge_add_update(prefix, update)) {
        return false;
      }
      if (update->changed && !update->is_delete) {
        node_changed = true;
      }
    }
  } else {
    if (node->value.len != 0 || prefix->len >= MPT_SECURE_KEY_NIBBLES) {
      fatal_error(WitnessDeficient);
    }
    for (unsigned nibble = 0; nibble < 16; ++nibble) {
      const NibblePath one = nibble_path_single(nibble);
      NibblePath child_prefix;
      if (!nibble_path_concat(prefix, &one, &child_prefix)) {
        return false;
      }
      size_t child_begin = 0;
      size_t child_end = 0;
      trie_update_range(updates, &child_prefix, &child_begin, &child_end);
      WitnessChild original_child = mpt_decoded_child(node, nibble);
      const NodeReference owned_child = mpt_copy_witness_reference(&original_child);
      PendingNode child = pending_node_reference(&owned_child);
      if (child_begin != child_end) {
        if (mpt_witness_child_kind(&original_child) == NODE_REFERENCE_EMPTY) {
          for (size_t index = child_begin; index < child_end; ++index) {
            TrieUpdate *update = &updates->entries[index];
            if (!mpt_authenticate_absent(update) || !mpt_merge_add_update(prefix, update)) {
              return false;
            }
            if (update->changed && !update->is_delete) {
              node_changed = true;
            }
          }
          continue;
        }
        bool changed = false;
        if (!mpt_merge_original_child(&original_child, &child_prefix, updates, &child, &changed)) {
          return false;
        }
        node_changed |= changed;
      }
      if (child_begin != child_end) {
        if (!mpt_merge_add_rebuilt_child(prefix, &child_prefix, &child)) {
          return false;
        }
      } else if (!mpt_merge_add_original_child(prefix, &child_prefix, TRIE_ENTRY_SUBTREE,
                                               &original_child)) {
        return false;
      }
    }
  }
  if (!node_changed) {
    mpt_merge_entries.count = item_begin;
    return true;
  }
  PendingNode rebuilt_pending;
  if (!mpt_build_pending_subtree(item_begin, &rebuilt_pending)) {
    return false;
  }
  result->changed = true;
  result->replacement = rebuilt_pending;
  return true;
}

static bool mpt_reference_root(const NodeReference *ref, Hash32 *root)
{
  if (ref->kind == NODE_REFERENCE_EMPTY) {
    *root = EVMSAIL_EMPTY_TRIE_ROOT;
    return true;
  }
  if (ref->kind == NODE_REFERENCE_HASHED) {
    *root = hash_from_be_bytes(ref->bytes);
    return true;
  }
  return mpt_keccak(ref->bytes, ref->len, root);
}

static bool mpt_merge_collected_updates_from_node(NodeId base_node, const Hash32 *base_root,
                                                  TrieUpdateBuffer *updates, Hash32 *root,
                                                  bool *changed)
{
  *changed = false;
  if (updates->count == 0) {
    *root = *base_root;
    return true;
  }

  if (base_node == MPT_NODE_ID_EMPTY) {
    if (!hash_equal(base_root, &EVMSAIL_EMPTY_TRIE_ROOT)) {
      fatal_error(WitnessDeficient);
    }
    const NibblePath empty = nibble_path_empty();
    mpt_merge_entries.count = 0;
    for (size_t index = 0; index < updates->count; ++index) {
      TrieUpdate *update = &updates->entries[index];
      if (!mpt_authenticate_absent(update) || !mpt_merge_add_update(&empty, update)) {
        return false;
      }
    }
    NodeReference ref;
    const bool ok = (mpt_build_subtree_reference(0, &ref) && mpt_reference_root(&ref, root)) != 0;
    *changed = ((ok && !hash_equal(base_root, root)) != 0);
    return ok;
  }

  /*
   * Every nonempty changed trie starts with its original root encoding. Child
   * spans follow the recursive update walk but continue to point into the
   * canonical, unchanged stateless input. The root itself is authenticated by
   * its 32-byte trie root, even when its encoding is short enough to be inline
   * as a child.
   */
  ByteSpan encoded;
  if (!mpt_node_span(base_node, &encoded) || encoded.len == 0) {
    fatal_error(WitnessDeficient);
  }
  Hash32 original_root;
  if (!mpt_node_digest(base_node, &original_root) || !hash_equal(base_root, &original_root)) {
    fatal_error(WitnessDeficient);
  }

  const NibblePath empty = nibble_path_empty();
  SubtreeMergeResult reduced;
  if (!mpt_merge_node(encoded, base_node, &empty, updates, &reduced)) {
    return false;
  }
  for (size_t index = 0; index < updates->count; ++index) {
    if (!updates->entries[index].authenticated) {
      fatal_error(WitnessDeficient);
    }
  }
  NodeReference changed_ref;
  if (reduced.changed && !pending_node_finalize(&reduced.replacement, &changed_ref)) {
    return false;
  }
  const NodeReference *effective = (int)reduced.changed ? &changed_ref : &reduced.original;
  *changed = reduced.changed;
  return mpt_reference_root(effective, root);
}

static bool mpt_merge_collected_updates(const Hash32 *base_root, TrieUpdateBuffer *updates,
                                        Hash32 *root, bool *changed)
{
  if (updates->count == 0) {
    *changed = false;
    *root = *base_root;
    return true;
  }

  if (hash_equal(base_root, &EVMSAIL_EMPTY_TRIE_ROOT)) {
    return mpt_merge_collected_updates_from_node(MPT_NODE_ID_EMPTY, base_root, updates, root,
                                                 changed);
  }

  NodeId base_node = MPT_NODE_ID_UNLINKED;
  if (!mpt_bind_root(base_root, &base_node)) {
    return false;
  }
  return mpt_merge_collected_updates_from_node(base_node, base_root, updates, root, changed);
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool mpt_merge_update_source_node(NodeId base_node, const Hash32 *base_root,
                                         MptUpdateSource source, Hash32 *root, bool *changed)
{
  return (mpt_collect_ordered_updates(source, &mpt_updates) &&
          mpt_merge_collected_updates_from_node(base_node, base_root, &mpt_updates, root,
                                                changed)) != 0;
}

static TrieUpdateBuffer mpt_account_update_buffer;

void mpt_workspace_bind(uint32_t account_count, uint32_t storage_count)
{
  const uint32_t order_count = account_count > storage_count ? account_count : storage_count;
  mpt_updates.capacity = storage_count;
  mpt_account_update_buffer.capacity = account_count;
  mpt_merge_entries.capacity = (size_t)storage_count + 1U;
  order_workspace_capacity = order_count;
  WORKSPACE_BIND(mpt_updates.entries, storage_count);
  WORKSPACE_BIND(mpt_account_update_buffer.entries, account_count);
  WORKSPACE_BIND(order_workspace.order, order_count);
  WORKSPACE_BIND(order_workspace.destinations, order_count);
  WORKSPACE_BIND(mpt_merge_entries.entries, (size_t)storage_count + 1U);
}

void mpt_reset(void)
{
  memset(&ordered_trie_workspace, 0, sizeof(ordered_trie_workspace));
  ordered_trie_builder_reset(&ordered_trie_workspace);
}

static NibblePath mpt_index_path(uint64_t index)
{
  uint8_t encoded[9];
  size_t encoded_len = 0;
  if (index == 0) {
    encoded[encoded_len++] = 0x80;
  } else if (index <= 0x7f) {
    encoded[encoded_len++] = (uint8_t)index;
  } else {
    uint8_t bytes[8];
    const size_t width = mpt_uint_be(index, bytes);
    encoded[encoded_len++] = (uint8_t)(0x80 + width);
    memcpy(encoded + encoded_len, bytes, width);
    encoded_len += width;
  }
  NibblePath path = nibble_path_empty();
  path.len = (uint8_t)(encoded_len * 2);
  for (size_t i = 0; i < encoded_len; ++i) {
    path.nibbles[2 * i] = encoded[i] >> 4;
    path.nibbles[(2 * i) + 1] = encoded[i] & 0x0f;
  }
  return path;
}

static uint64_t mpt_index_at_position(uint64_t count, uint64_t position)
{
  const uint64_t rest = count - 1;
  const uint64_t single_count = rest < 127 ? rest : 127;
  if (position < single_count) {
    return position + 1;
  }
  if (position == single_count) {
    return 0;
  }
  return position;
}

static bool mpt_resolve_list(const struct BoundedSszListRef *items, ByteSpan *bytes)
{
  const uint64_t len = items->bytes.len;
  const uint8_t *resolved = items->bytes.bytes;
  if (len > SIZE_MAX || !resolved) {
    fatal_error(InvalidConfig);
  }
  bytes->data = resolved;
  bytes->len = (size_t)len;
  return true;
}

static bool mpt_read_u32_le(ByteSpan bytes, size_t off, uint32_t *value)
{
  if (off > bytes.len || sizeof(uint32_t) > bytes.len - off) {
    fatal_error(InvalidConfig);
  }
  const uint8_t *p = bytes.data + off;
  *value = (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
  return true;
}

static bool mpt_read_u64_le(ByteSpan bytes, size_t off, uint64_t *value)
{
  if (off > bytes.len || sizeof(uint64_t) > bytes.len - off) {
    fatal_error(InvalidConfig);
  }
  const uint8_t *p = bytes.data + off;
  *value = (uint64_t)p[0] | ((uint64_t)p[1] << 8) | ((uint64_t)p[2] << 16) |
           ((uint64_t)p[3] << 24) | ((uint64_t)p[4] << 32) | ((uint64_t)p[5] << 40) |
           ((uint64_t)p[6] << 48) | ((uint64_t)p[7] << 56);
  return true;
}

static bool mpt_variable_list_item(const struct BoundedSszListRef *items, ByteSpan bytes,
                                   uint64_t index, ByteSpan *item)
{
  uint32_t start = 0;
  uint32_t stop = 0;
  if (index >= items->count || index > SIZE_MAX / 4 ||
      !mpt_read_u32_le(bytes, (size_t)index * 4, &start)) {
    fatal_error(InvalidConfig);
  }
  if (index + 1 < items->count) {
    if (!mpt_read_u32_le(bytes, (size_t)(index + 1) * 4, &stop)) {
      return false;
    }
  } else {
    if (bytes.len > UINT32_MAX) {
      fatal_error(InvalidConfig);
    }
    stop = (uint32_t)bytes.len;
  }
  if (start > stop || stop > bytes.len ||
      (items->max_item_length != 0 && (uint64_t)(stop - start) > items->max_item_length)) {
    fatal_error(InvalidConfig);
  }
  item->data = bytes.data + start;
  item->len = stop - start;
  return true;
}

void mpt_index_witness_nodes(struct BoundedSszListRef nodes)
{
  enum {
    MPT_MAX_WITNESS_NODES = 1U << 22,
    MPT_MAX_WITNESS_NODE_LENGTH = 1U << 10,
  };
  ByteSpan bytes = {0};
  if (nodes.count > MPT_MAX_WITNESS_NODES || !mpt_resolve_list(&nodes, &bytes)) {
    fatal_error(InvalidConfig);
  }

  mpt_node_arena_reset();
  mpt_node_arena_begin();
  for (uint64_t index = 0; index < nodes.count; ++index) {
    ByteSpan node = {0};
    Hash32 digest = {{0}};
    if (!mpt_variable_list_item(&nodes, bytes, index, &node) ||
        node.len > MPT_MAX_WITNESS_NODE_LENGTH || !mpt_keccak(node.data, node.len, &digest)) {
      fatal_error(InvalidConfig);
    }
    if (!mpt_node_table_insert(&digest, node)) {
      fatal_error(InvalidConfig);
    }
  }
  if (!mpt_node_index_seal()) {
    fatal_error(InvalidConfig);
  }
}

void mpt_index_witness_codes(struct BoundedSszListRef codes, bool amsterdam_or_later)
{
  enum {
    MPT_MAX_WITNESS_CODES = 1U << 18,
    MPT_MAX_WITNESS_CODE_LENGTH = 1U << 16,
  };
  ByteSpan bytes = {0};
  if (codes.count > MPT_MAX_WITNESS_CODES || !mpt_resolve_list(&codes, &bytes)) {
    fatal_error(InvalidConfig);
  }

  for (uint64_t index = 0; index < codes.count; ++index) {
    ByteSpan code = {0};
    if (!mpt_variable_list_item(&codes, bytes, index, &code) ||
        code.len > MPT_MAX_WITNESS_CODE_LENGTH ||
        !code_db_insert_analyzed_bytes(code.data, code.len, amsterdam_or_later)) {
      fatal_error(InvalidConfig);
    }
  }
}

static bool mpt_ordered_insert(TrieEntry *item, uint64_t count, uint64_t position)
{
  const uint64_t index = mpt_index_at_position(count, position);
  item->path = mpt_index_path(index);
  NibblePath next;
  const NibblePath *next_key = NULL;
  if (position + 1 < count) {
    next = mpt_index_path(mpt_index_at_position(count, position + 1));
    next_key = &next;
  }
  return ordered_trie_builder_insert(&ordered_trie_workspace, item, next_key);
}

static void mpt_ordered_root_finish(Hash32 *root)
{
  if (!ordered_trie_builder_finish(&ordered_trie_workspace, root)) {
    memset(root, 0, sizeof(*root));
  }
}

Hash32 mpt_transaction_trie_root(struct BoundedSszListRef transactions)
{
  mpt_reset();
  ByteSpan bytes = {0};
  Hash32 root = {{0}};
  if (transactions.count > (UINT64_C(1) << 20) || !mpt_resolve_list(&transactions, &bytes)) {
    fatal_error(InvalidConfig);
  }
  for (uint64_t position = 0; position < transactions.count; ++position) {
    const uint64_t index = mpt_index_at_position(transactions.count, position);
    ByteSpan transaction;
    if (!mpt_variable_list_item(&transactions, bytes, index, &transaction)) {
      break;
    }
    TrieEntry item;
    memset(&item, 0, sizeof(item));
    item.kind = TRIE_ENTRY_LEAF;
    item.external_value = true;
    item.external_value_bytes = transaction.data;
    item.external_value_len = transaction.len;
    if (!mpt_ordered_insert(&item, transactions.count, position)) {
      break;
    }
  }
  mpt_ordered_root_finish(&root);
  return root;
}

Hash32 mpt_withdrawals_trie_root(struct BoundedSszListRef withdrawals)
{
  enum {
    MPT_WITHDRAWAL_SIZE = 44,
    MPT_WITHDRAWAL_ADDRESS_OFF = 16,
    MPT_WITHDRAWAL_ADDRESS_LEN = 20,
    MPT_WITHDRAWAL_AMOUNT_OFF = 36,
  };
  mpt_reset();
  ByteSpan bytes = {0};
  Hash32 root = {{0}};
  if (withdrawals.count > 16 || !mpt_resolve_list(&withdrawals, &bytes) ||
      bytes.len != (size_t)withdrawals.count * MPT_WITHDRAWAL_SIZE) {
    fatal_error(InvalidConfig);
  }
  for (uint64_t position = 0; position < withdrawals.count; ++position) {
    const uint64_t index = mpt_index_at_position(withdrawals.count, position);
    const size_t off = (size_t)index * MPT_WITHDRAWAL_SIZE;
    ByteSpan withdrawal = {
        bytes.data + off,
        MPT_WITHDRAWAL_SIZE,
    };
    uint64_t withdrawal_index = 0;
    uint64_t validator_index = 0;
    uint64_t amount = 0;
    if (!mpt_read_u64_le(withdrawal, 0, &withdrawal_index) ||
        !mpt_read_u64_le(withdrawal, 8, &validator_index) ||
        !mpt_read_u64_le(withdrawal, MPT_WITHDRAWAL_AMOUNT_OFF, &amount)) {
      break;
    }
    const uint8_t *address = withdrawal.data + MPT_WITHDRAWAL_ADDRESS_OFF;
    const size_t content =
        mpt_rlp_uint64_size(withdrawal_index) + mpt_rlp_uint64_size(validator_index) +
        mpt_rlp_string_size(address, MPT_WITHDRAWAL_ADDRESS_LEN) + mpt_rlp_uint64_size(amount);
    TrieEntry item;
    memset(&item, 0, sizeof(item));
    ByteWriter writer = {item.value, 0, sizeof(item.value)};
    if (content > 48 || !mpt_rlp_prefix(&writer, true, content) ||
        !mpt_rlp_uint64(&writer, withdrawal_index) || !mpt_rlp_uint64(&writer, validator_index) ||
        !mpt_rlp_string(&writer, address, MPT_WITHDRAWAL_ADDRESS_LEN) ||
        !mpt_rlp_uint64(&writer, amount)) {
      break;
    }
    item.kind = TRIE_ENTRY_LEAF;
    item.value_len = (uint8_t)writer.len;
    if (!mpt_ordered_insert(&item, withdrawals.count, position)) {
      break;
    }
  }
  mpt_ordered_root_finish(&root);
  return root;
}

Hash32 mpt_receipt_table_root(uint64_t count)
{
  Hash32 root = {{0}};
  if (count != receipt_record_count()) {
    fatal_error(WitnessDeficient);
  }
  for (uint64_t position = 0; position < count; ++position) {
    const uint64_t index = mpt_index_at_position(count, position);
    const NibblePath key = mpt_index_path(index);
    NibblePath next;
    const NibblePath *next_key = NULL;
    if (position + 1 < count) {
      const uint64_t next_index = mpt_index_at_position(count, position + 1);
      next = mpt_index_path(next_index);
      next_key = &next;
    }
    const uint8_t *value_bytes = NULL;
    uint64_t value_length = 0;
    TrieEntry item;
    memset(&item, 0, sizeof(item));
    item.path = key;
    item.kind = TRIE_ENTRY_LEAF;
    item.external_value = true;
    if (!receipt_record_span(index, &value_bytes, &value_length) || value_length > SIZE_MAX) {
      fatal_error(WitnessDeficient);
    }
    item.external_value_bytes = value_bytes;
    item.external_value_len = (size_t)value_length;
    if (!ordered_trie_builder_insert(&ordered_trie_workspace, &item, next_key)) {
      break;
    }
  }
  if (!ordered_trie_builder_finish(&ordered_trie_workspace, &root)) {
    memset(&root, 0, sizeof(root));
  }
  return root;
}

void stateless_account_read(Hash32 root, Hash32 address_hash, struct AccountInfo *info, bool *found,
                            NodeId *terminal_node, NodeId *storage_root_node)
{
  *found = false;
  *storage_root_node = MPT_NODE_ID_EMPTY;
  memset(info, 0, sizeof(*info));
  ByteSpan value;
  if (mpt_point_lookup(&root, &address_hash, &value, NULL, NULL, terminal_node) && value.len != 0) {
    if (mpt_decode_account_value(value, info) &&
        mpt_bind_storage_root_identity(&info->storage_root, storage_root_node)) {
      *found = true;
    }
  }
}

void stateless_storage_read(NodeId root_node, Hash32 slot_hash, U256 *value_out, bool *found,
                            NodeId *terminal_node)
{
  *found = false;
  memset(value_out, 0, sizeof(*value_out));
  ByteSpan value;
  if (mpt_point_lookup_node(root_node, &slot_hash, &value, NULL, NULL, terminal_node) &&
      value.len != 0) {
    RlpItem item;
    ByteSpan cursor = value;
    if (mpt_rlp_pop(&cursor, &item) && cursor.len == 0) {
      if (mpt_decode_word_item(&item, value_out)) {
        *found = true;
      }
    } else {
      {
        fatal_error(RlpDecode);
      }
    }
  }
}

Hash32 mpt_compute_state_root(Hash32 parent_state_root)
{
  Hash32 root = {{0}};
  const uint32_t account_count = mpt_account_updates_prepare();
  const MptUpdateSource account_source = {
      .kind = MPT_UPDATE_SOURCE_ACCOUNTS,
      .position = 0,
      .count = account_count,
  };
  if (mpt_collect_ordered_updates(account_source, &mpt_account_update_buffer)) {
    bool changed = false;
    if (!mpt_merge_collected_updates(&parent_state_root, &mpt_account_update_buffer, &root,
                                     &changed)) {
      memset(&root, 0, sizeof(root));
    }
  }
  mpt_last_root = root;
  return root;
}

#ifdef EVMSAIL_NATIVE_TEST
Hash32 mpt_last_state_root(void)
{
  return mpt_last_root;
}
#endif
