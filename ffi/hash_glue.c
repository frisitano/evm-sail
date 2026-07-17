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
 * segment resolves through the central ByteSlice source resolver --
 * an unresolvable slice poisons the preimage, yielding the same sentinel
 * digest path the old source hashers used. */
#include EVMSAIL_MODEL_H
#include "byte_slice_glue.h"
#include "host_crypto.h"
#include "kernel_state.h"
#include "lbits_convert.h"
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

static uint64_t byte_slice_off(const struct zByteSlice *slice) {
  return evmsail_byte_quantity_value(slice->zoff);
}

static uint64_t byte_slice_len(const struct zByteSlice *slice) {
  return evmsail_byte_quantity_value(slice->zlen);
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
      const struct node_zz5listz8z5bvz9 *b = materialized->zdata;
      uint64_t remaining = materialized_bytes_len(materialized);
      while (remaining && b) {
        uint8_t v;
        lbits_to_be_bytes(&v, 1, b->hd);
        seg_put(&v, 1);
        b = b->tl;
        remaining--;
      }
      if (remaining || b) {
        seg_ok = 0;
        return;
      }
    } else { /* Kind_zBytesSlice */
      const struct zByteSlice *s = &n->hd.variants.zBytesSlice;
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
    }
  }
}

/* fast path: a single-slice preimage hashes IN PLACE from its source store
 * (no accumulator copy) -- the common case for witness nodes, code, and
 * memory KECCAK256 */
static int seg_single_slice(zz5listz8z5unionz0zzBytesz9 segs, const uint8_t **p,
                            uint64_t *len) {
  if (!segs || segs->tl || segs->hd.kind != Kind_zBytesSlice) return 0;
  const struct zByteSlice *s = &segs->hd.variants.zBytesSlice;
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
                                     struct zByteSlice expected) {
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
      const struct node_zz5listz8z5bvz9 *b = materialized->zdata;
      uint64_t remaining = materialized_bytes_len(materialized);
      while (remaining && b) {
        if (offset >= expected_len) return false;
        uint8_t v;
        lbits_to_be_bytes(&v, 1, b->hd);
        if (v != want[offset]) return false;
        offset++;
        b = b->tl;
        remaining--;
      }
      if (remaining || b) return false;
    } else { /* Kind_zBytesSlice */
      const struct zByteSlice *s = &n->hd.variants.zBytesSlice;
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
    }
  }
  return offset == expected_len;
}

void host_keccak_segments(lbits *rop, zz5listz8z5unionz0zzBytesz9 segs) {
  const uint8_t *p = NULL;
  uint64_t len = 0;
  if (seg_single_slice(segs, &p, &len)) {
    host_keccak256_lbits(rop, p, len);
    return;
  }
  seg_accumulate(segs);
  if (seg_ok) host_keccak256_lbits(rop, seg_buf, seg_len);
  else host_keccak256_lbits(rop, NULL, UINT64_MAX);
}

void host_sha256_segments(lbits *rop, zz5listz8z5unionz0zzBytesz9 segs) {
  const uint8_t *p = NULL;
  uint64_t len = 0;
  if (seg_single_slice(segs, &p, &len)) {
    host_sha256_lbits(rop, p, len);
    return;
  }
  seg_accumulate(segs);
  if (seg_ok) host_sha256_lbits(rop, seg_buf, seg_len);
  else host_sha256_lbits(rop, NULL, UINT64_MAX);
}

/* ---- log records (host/state.sail log_append / read_logs) --------------- */
/* ONE call appends a whole LOG record: the data crosses as a Bytes segment
 * (the LOG0..4 memory span resolves + memcpies straight from the memory
 * store; EIP-7708 system logs pass materialized words) and is snapshotted
 * into kernel_state.c's arenas. */
unit log_append_record(const lbits a, zz5listz8z5bvz9 topics, struct zBytes data) {
  log_begin(a);
  for (const struct node_zz5listz8z5bvz9 *t = topics; t; t = t->tl)
    log_add_topic(t->hd);
  if (data.kind == Kind_zBytesList) {
    const struct zMaterializzedBytes *materialized = &data.variants.zBytesList;
    const struct node_zz5listz8z5bvz9 *b = materialized->zdata;
    uint64_t remaining = materialized_bytes_len(materialized);
    while (remaining && b) {
      uint8_t v;
      lbits_to_be_bytes(&v, 1, b->hd);
      log_add_data_bulk(&v, 1);
      b = b->tl;
      remaining--;
    }
  } else {
    const struct zByteSlice *s = &data.variants.zBytesSlice;
    const uint8_t *p = NULL;
    uint64_t off = byte_slice_off(s);
    uint64_t len = byte_slice_len(s);
    uint64_t rlen = 0;
    if (evmsail_resolve_byte_source(evmsail_source_kind(s->zsource), off, len,
                                    &p, &rlen) &&
        rlen == len)
      log_add_data_bulk(p, len);
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

    /* address: the low 160 bits of the stored word */
    lbits a256;
    uint8_t ab[32];
    log_addr(&a256, i);
    lbits_to_be_bytes(ab, 32, a256);
    be_bytes_to_lbits(&node->hd.zaddress, 160, ab + 12, 20);

    zz5listz8z5bvz9 ts = NULL;
    for (uint64_t j = log_topic_count(i); j-- > 0;) {
      struct node_zz5listz8z5bvz9 *tn = sail_new(struct node_zz5listz8z5bvz9);
      tn->rc = 1;
      tn->tl = ts;
      log_topic(&tn->hd, i, j);
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
