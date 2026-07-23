/* Segment-list hashing glue: the second hand-written C file (after
 * journal_glue.c) that handles GENERATED Sail types, compiled per build
 * against that build's model header (EVMSAIL_MODEL_H) so it always sees the
 * exact generated layout -- never a hand-mirrored one.
 *
 * host_keccak_segments / host_sha256_segments implement the hash axioms, and
 * host_bytes_segments_equal_slice implements content equality against a
 * source-backed slice. Their list(Bytes) argument denotes the CONCATENATION
 * of the segments. One FFI call performs each complete operation; the walk
 * over the generated cons cells is native pointer chasing.
 * A BytesList segment contributes its materialized Sail bytes; a BytesSlice
 * segment resolves through the central ByteSlice source resolver; and a
 * BytesFixed32 segment contributes the selected prefix of its fixed vector --
 * an unresolvable slice poisons the preimage, yielding the same sentinel
 * digest path the old source hashers used. */
#include EVMSAIL_MODEL_H
#include "byte_slice_glue.h"
#include "host_crypto.h"
#include "kernel_state.h"
#include "value_convert.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* growable preimage accumulator (single-threaded, reused across calls) */
static uint8_t *seg_buf;
static uint64_t seg_len, seg_cap;
static int seg_ok;

static uint64_t materialized_bytes_len(
    const struct zMaterializzedBytes *bytes) {
  return evmsail_byte_quantity_value(bytes->zlen);
}

static uint64_t byte_slice_off(const struct zByteSliceFields *slice) {
  return evmsail_byte_quantity_value(slice->zoff);
}

static uint64_t byte_slice_len(const struct zByteSliceFields *slice) {
  return evmsail_byte_quantity_value(slice->zlen);
}

static uint64_t fixed_bytes32_len(const struct zFixedBytes32 *bytes) {
  /* range(0, 32) is native-width in both generated ABIs.  It is deliberately
     narrower than the host_access fields handled by quantity_abi.h. */
  return bytes->zlen;
}

static int fixed_bytes32_value(const struct zFixedBytes32 *fixed,
                               uint8_t bytes[32], uint64_t *len) {
  *len = fixed_bytes32_len(fixed);
  if (*len > 32) return 0;
  evmsail_hash_to_be_bytes(bytes, fixed->zdata);
  return 1;
}

static void seg_put(const uint8_t *p, uint64_t n) {
  if (!seg_ok || n == 0) return;
  if (seg_len + n > seg_cap) {
    uint64_t cap = seg_cap ? seg_cap : 256;
    while (cap < seg_len + n) cap *= 2;
    uint8_t *nb = realloc(seg_buf, (size_t)cap);
    if (!nb) {
      seg_ok = 0;
      return;
    }
    seg_buf = nb;
    seg_cap = cap;
  }
  memcpy(seg_buf + seg_len, p, (size_t)n);
  seg_len += n;
}

static void seg_accumulate(zz5listz8z5unionz0zzBytesz9 segs) {
  seg_len = 0;
  seg_ok = 1;
  for (const struct node_zz5listz8z5unionz0zzBytesz9 *n = segs; n; n = n->tl) {
    if (n->hd.kind == Kind_zBytesList) {
      const struct zMaterializzedBytes *materialized =
          &n->hd.variants.zBytesList;
      evmsail_byte_list b = materialized->zdata;
      uint64_t remaining = materialized_bytes_len(materialized);
      while (remaining && b) {
        uint8_t v = evmsail_byte_value(b->hd);
        seg_put(&v, 1);
        b = b->tl;
        remaining--;
      }
      if (remaining || b) {
        seg_ok = 0;
        return;
      }
    } else if (n->hd.kind == Kind_zBytesSlice) {
      const struct zByteSliceFields *s = &n->hd.variants.zBytesSlice;
      const uint8_t *p = NULL;
      uint64_t off = byte_slice_off(s);
      uint64_t len = byte_slice_len(s);
      uint64_t rlen = 0;
      if (!evmsail_resolve_byte_source(evmsail_source_kind(s->zsource), off,
                                       len, &p, &rlen) ||
          rlen != len) {
        seg_ok = 0;
        return;
      }
      seg_put(p, len);
    } else { /* Kind_zBytesFixed32 */
      const struct zFixedBytes32 *fixed = &n->hd.variants.zBytesFixed32;
      uint8_t bytes[32];
      uint64_t len = 0;
      if (!fixed_bytes32_value(fixed, bytes, &len)) {
        seg_ok = 0;
        return;
      }
      seg_put(bytes, len);
    }
  }
}

/* fast path: a single-slice preimage hashes IN PLACE from its source store
 * (no accumulator copy) -- the common case for witness nodes, code, and
 * memory KECCAK256 */
static int seg_single_slice(zz5listz8z5unionz0zzBytesz9 segs, const uint8_t **p,
                            uint64_t *len) {
  if (!segs || segs->tl || segs->hd.kind != Kind_zBytesSlice) return 0;
  const struct zByteSliceFields *s = &segs->hd.variants.zBytesSlice;
  uint64_t off = byte_slice_off(s);
  uint64_t slice_len = byte_slice_len(s);
  uint64_t rlen = 0;
  if (!evmsail_resolve_byte_source(evmsail_source_kind(s->zsource), off,
                                   slice_len, p, &rlen) ||
      rlen != slice_len)
    return 0;
  *len = slice_len;
  return 1;
}

bool host_bytes_segments_equal_slice(zz5listz8z5unionz0zzBytesz9 segs,
                                     struct zByteSliceFields expected) {
  const uint8_t *want = NULL;
  uint64_t expected_off = byte_slice_off(&expected);
  uint64_t expected_len = byte_slice_len(&expected);
  uint64_t want_len = 0;
  if (!evmsail_resolve_byte_source(evmsail_source_kind(expected.zsource),
                                   expected_off, expected_len, &want,
                                   &want_len) ||
      want_len != expected_len)
    return false;

  uint64_t offset = 0;
  for (const struct node_zz5listz8z5unionz0zzBytesz9 *n = segs; n;
       n = n->tl) {
    if (n->hd.kind == Kind_zBytesList) {
      const struct zMaterializzedBytes *materialized =
          &n->hd.variants.zBytesList;
      evmsail_byte_list b = materialized->zdata;
      uint64_t remaining = materialized_bytes_len(materialized);
      while (remaining && b) {
        if (offset >= expected_len) return false;
        uint8_t v = evmsail_byte_value(b->hd);
        if (v != want[offset]) return false;
        offset++;
        b = b->tl;
        remaining--;
      }
      if (remaining || b) return false;
    } else if (n->hd.kind == Kind_zBytesSlice) {
      const struct zByteSliceFields *s = &n->hd.variants.zBytesSlice;
      uint64_t slice_off = byte_slice_off(s);
      uint64_t slice_len = byte_slice_len(s);
      if (offset > expected_len || slice_len > expected_len - offset)
        return false;
      const uint8_t *actual = NULL;
      uint64_t actual_len = 0;
      if (!evmsail_resolve_byte_source(evmsail_source_kind(s->zsource),
                                       slice_off, slice_len, &actual,
                                       &actual_len) ||
          actual_len != slice_len ||
          memcmp(actual, want + offset, (size_t)slice_len) != 0)
        return false;
      offset += slice_len;
    } else { /* Kind_zBytesFixed32 */
      const struct zFixedBytes32 *fixed = &n->hd.variants.zBytesFixed32;
      uint8_t bytes[32];
      uint64_t len = 0;
      if (!fixed_bytes32_value(fixed, bytes, &len) ||
          offset > expected_len || len > expected_len - offset ||
          memcmp(bytes, want + offset, (size_t)len) != 0)
        return false;
      offset += len;
    }
  }
  return offset == expected_len;
}

EVMSAIL_HASH_RETURN host_keccak_segments(
    EVMSAIL_HASH_RESULT(result) zz5listz8z5unionz0zzBytesz9 segs) {
  const uint8_t *p = NULL;
  uint64_t len = 0;
  uint64_t digest[4];
  uint8_t bytes[32];
  if (!seg_single_slice(segs, &p, &len)) {
    seg_accumulate(segs);
    p = seg_ok ? seg_buf : NULL;
    len = seg_ok ? seg_len : UINT64_MAX;
  }
  host_keccak256_bytes(digest, p, len);
  be_words4_to_be_bytes(bytes, digest);
  EVMSAIL_RETURN_HASH_BE_BYTES(result, bytes);
}

EVMSAIL_HASH_RETURN host_sha256_segments(
    EVMSAIL_HASH_RESULT(result) zz5listz8z5unionz0zzBytesz9 segs) {
  const uint8_t *p = NULL;
  uint64_t len = 0;
  uint64_t digest[4];
  uint8_t bytes[32];
  if (!seg_single_slice(segs, &p, &len)) {
    seg_accumulate(segs);
    p = seg_ok ? seg_buf : NULL;
    len = seg_ok ? seg_len : UINT64_MAX;
  }
  host_sha256_bytes(digest, p, len);
  be_words4_to_be_bytes(bytes, digest);
  EVMSAIL_RETURN_HASH_BE_BYTES(result, bytes);
}

/* ---- log records (host/state.sail log_append / read_logs) --------------- */
/* ONE call appends a whole LOG record: the data crosses as a Bytes segment
 * (the LOG0..4 memory span resolves + memcpies straight from the memory
 * store; EIP-7708 system logs pass materialized words) and is snapshotted
 * into kernel_state.c's arenas. */
unit log_append_record(sail_address a, evmsail_word_list topics,
                       struct zBytes data) {
  log_begin(a);
  for (const struct node_zz5listz8z5structz0zz__sail_c_repr_u256z9 *t = topics;
       t; t = t->tl)
    log_add_topic(t->hd);
  if (data.kind == Kind_zBytesList) {
    const struct zMaterializzedBytes *materialized = &data.variants.zBytesList;
    evmsail_byte_list b = materialized->zdata;
    uint64_t remaining = materialized_bytes_len(materialized);
    while (remaining && b) {
      uint8_t v = evmsail_byte_value(b->hd);
      log_add_data_bulk(&v, 1);
      b = b->tl;
      remaining--;
    }
  } else if (data.kind == Kind_zBytesSlice) {
    const struct zByteSliceFields *s = &data.variants.zBytesSlice;
    const uint8_t *p = NULL;
    uint64_t off = byte_slice_off(s);
    uint64_t len = byte_slice_len(s);
    uint64_t rlen = 0;
    if (evmsail_resolve_byte_source(evmsail_source_kind(s->zsource), off, len,
                                    &p, &rlen) &&
        rlen == len)
      log_add_data_bulk(p, len);
  } else {
    const struct zFixedBytes32 *fixed = &data.variants.zBytesFixed32;
    uint8_t bytes[32];
    uint64_t len = 0;
    if (fixed_bytes32_value(fixed, bytes, &len))
      log_add_data_bulk(bytes, len);
  }
  return UNIT;
}

/* ONE call reads the whole ordered LOG series back as list(LogEntry) --
 * fresh single-owner nodes (rc = 1), oldest first, built newest-to-oldest. */
void logs_read_all(zz5listz8z5structz0zzLogEntryz9 *rop, unit u) {
  (void)u;
  zz5listz8z5structz0zzLogEntryz9 out = NULL;
  for (uint64_t i = log_count(UNIT); i-- > 0;) {
    struct node_zz5listz8z5structz0zzLogEntryz9 *node =
        sail_new(struct node_zz5listz8z5structz0zzLogEntryz9);
    node->rc = 1;
    node->tl = out;

    /* Address and word-list elements have the same specialized ABI in both
     * build modes; only otherwise-unbounded protocol quantities differ. */
    node->hd.zaddress = log_addr(i);
    zz5listz8z5structz0zz__sail_c_repr_u256z9 ts = NULL;
    for (uint64_t j = log_topic_count(i); j-- > 0;) {
      struct node_zz5listz8z5structz0zz__sail_c_repr_u256z9 *tn =
          sail_new(struct node_zz5listz8z5structz0zz__sail_c_repr_u256z9);
      tn->rc = 1;
      tn->tl = ts;
      tn->hd = log_topic(i, j);
      ts = tn;
    }
    node->hd.ztopics = ts;

    /* data stays a REFERENCE into the block-lifetime log arena */
    node->hd.zdata.zsource = zLogDataSource;
    evmsail_byte_quantity_set(&node->hd.zdata.zoff, log_data_off(i));
    evmsail_byte_quantity_set(&node->hd.zdata.zlen, log_data_len(i));

    out = node;
  }
  *rop = out;
}
