#include "host_crypto.h"
#include "lbits_convert.h"
#include "code_db.h"
#include "memory.h"
#include "zkvm_accelerators.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern const uint8_t *evmsail_ssz_ptr(uint64_t off, uint64_t len);

static const uint8_t HOST_empty_source = 0;
static uint8_t *HOST_bytes;
static uint64_t HOST_bytes_len;
static uint64_t HOST_bytes_cap;
static int HOST_bytes_ok = 1;

static uint64_t host_be64(const uint8_t *p) {
  uint64_t w = 0;
  for (int i = 0; i < 8; i++) w = (w << 8) | p[i];
  return w;
}

static void host_words_to_lbits(lbits *rop, const uint64_t words[4]) {
  be_words4_to_lbits(rop, words);
}

static void host_hash_bytes(uint64_t id, uint64_t out[4], const uint8_t *p, uint64_t len) {
  uint8_t digest[32] = {0};
  uint8_t empty = 0;
  const uint8_t *src = len ? p : &empty;
  int ok = (src != NULL && len <= UINT32_MAX);

  if (ok) {
    if (id == 0) {
      zkvm_keccak256_hash h;
      ok = (zkvm_keccak256(src, (size_t)len, &h) == ZKVM_EOK);
      if (ok) memcpy(digest, h.data, 32);
    } else if (id == 2) {
      zkvm_sha256_hash h;
      ok = (zkvm_sha256(src, (size_t)len, &h) == ZKVM_EOK);
      if (ok) memcpy(digest, h.data, 32);
    } else {
      ok = 0;
    }
  }

  if (!ok) memset(digest, 0, sizeof digest);
  for (int i = 0; i < 4; i++) out[i] = host_be64(digest + i * 8);
}

void host_keccak256_bytes(uint64_t out[4], const uint8_t *p, uint64_t len) {
  host_hash_bytes(0, out, p, len);
}

void host_sha256_bytes(uint64_t out[4], const uint8_t *p, uint64_t len) {
  host_hash_bytes(2, out, p, len);
}

void host_keccak256_lbits(lbits *rop, const uint8_t *p, uint64_t len) {
  uint64_t out[4] = {0, 0, 0, 0};
  host_keccak256_bytes(out, p, len);
  host_words_to_lbits(rop, out);
}

void host_sha256_lbits(lbits *rop, const uint8_t *p, uint64_t len) {
  uint64_t out[4] = {0, 0, 0, 0};
  host_sha256_bytes(out, p, len);
  host_words_to_lbits(rop, out);
}

void host_keccak_word(lbits *rop, const lbits w) {
  uint8_t buf[32];
  lbits_to_be_bytes(buf, sizeof buf, w);
  host_keccak256_lbits(rop, buf, sizeof buf);
}

void host_keccak_address(lbits *rop, const lbits a) {
  uint8_t buf[20];
  lbits_to_be_bytes(buf, sizeof buf, a);
  host_keccak256_lbits(rop, buf, sizeof buf);
}

void host_keccak_create2(lbits *rop, const lbits sender, const lbits salt,
                         const lbits init_hash) {
  uint8_t buf[85];
  buf[0] = 0xff;
  lbits_to_be_bytes(buf + 1, 20, sender);
  lbits_to_be_bytes(buf + 21, 32, salt);
  lbits_to_be_bytes(buf + 53, 32, init_hash);
  host_keccak256_lbits(rop, buf, sizeof buf);
}

void host_sha256_pair(lbits *rop, const lbits a, const lbits b) {
  uint8_t buf[64];
  lbits_to_be_bytes(buf, 32, a);
  lbits_to_be_bytes(buf + 32, 32, b);
  host_sha256_lbits(rop, buf, sizeof buf);
}

/* sha256 of the present 32-byte digests concatenated in order (mask bits 0..2
 * select d0,d1,d2). The EIP-7685 execution-requests commitment. */
void host_sha256_digests3(lbits *rop, uint64_t mask, const lbits d0, const lbits d1,
                          const lbits d2) {
  uint8_t buf[96];
  size_t n = 0;
  if (mask & 1) { lbits_to_be_bytes(buf + n, 32, d0); n += 32; }
  if (mask & 2) { lbits_to_be_bytes(buf + n, 32, d1); n += 32; }
  if (mask & 4) { lbits_to_be_bytes(buf + n, 32, d2); n += 32; }
  host_sha256_lbits(rop, buf, n);
}

/* RLP-encode a non-negative u64 as its minimal big-endian scalar string into
 * out (<= 9 bytes); returns the encoded length. 0 -> 0x80, v < 0x80 -> [v],
 * else 0x80+len ++ big-endian bytes with no leading zeros. */
static size_t host_rlp_uint(uint8_t *out, uint64_t v) {
  if (v < 0x80) {
    out[0] = v ? (uint8_t)v : 0x80;
    return 1;
  }
  uint8_t be[8];
  int n = 0;
  for (uint64_t x = v; x != 0; x >>= 8) be[n++] = (uint8_t)(x & 0xff);
  out[0] = (uint8_t)(0x80 + n);
  for (int i = 0; i < n; i++) out[1 + i] = be[n - 1 - i];
  return (size_t)(1 + n);
}

void host_create_address(lbits *rop, const lbits sender, uint64_t nonce) {
  uint8_t payload[30]; /* rlp_addr = 21, rlp_int(u64) <= 9 */
  size_t n = 0;
  payload[n++] = 0x94; /* 0x80 + 20-byte address */
  lbits_to_be_bytes(payload + n, 20, sender);
  n += 20;
  n += host_rlp_uint(payload + n, nonce);
  uint8_t buf[31]; /* one-byte list header (payload < 55) + payload */
  buf[0] = (uint8_t)(0xc0 + n);
  memcpy(buf + 1, payload, n);
  host_keccak256_lbits(rop, buf, n + 1);
}

/* ---- RLP append helpers over a caller buffer (build-side header framing) --- */
static void rlp_put_raw(uint8_t *buf, size_t *n, const uint8_t *p, size_t len) {
  memcpy(buf + *n, p, len);
  *n += len;
}

/* 0xa0 ++ 32-byte word (rlp_word). */
static void rlp_put_word32(uint8_t *buf, size_t *n, const lbits w) {
  buf[(*n)++] = 0xa0;
  lbits_to_be_bytes(buf + *n, 32, w);
  *n += 32;
}

/* 0x94 ++ 20-byte address (rlp_addr). */
static void rlp_put_addr(uint8_t *buf, size_t *n, const lbits a) {
  buf[(*n)++] = 0x94;
  lbits_to_be_bytes(buf + *n, 20, a);
  *n += 20;
}

/* minimal big-endian 256-bit scalar (rlp_int_word). */
static void rlp_put_word_min(uint8_t *buf, size_t *n, const lbits w) {
  uint8_t be[32];
  lbits_to_be_bytes(be, 32, w);
  int i = 0;
  while (i < 32 && be[i] == 0) i++;
  size_t len = 32 - (size_t)i;
  if (len == 0) {
    buf[(*n)++] = 0x80;
  } else if (len == 1 && be[i] < 0x80) {
    buf[(*n)++] = be[i];
  } else {
    buf[(*n)++] = (uint8_t)(0x80 + len);
    rlp_put_raw(buf, n, be + i, len);
  }
}

/* RLP-encode a byte string (rlp_bytes). */
static void rlp_put_str(uint8_t *buf, size_t *n, const uint8_t *p, size_t len) {
  if (len == 1 && p[0] < 0x80) {
    buf[(*n)++] = p[0];
    return;
  }
  if (len <= 55) {
    buf[(*n)++] = (uint8_t)(0x80 + len);
  } else {
    uint8_t lb[8];
    int m = 0;
    for (size_t x = len; x != 0; x >>= 8) lb[m++] = (uint8_t)(x & 0xff);
    buf[(*n)++] = (uint8_t)(0xb7 + m);
    for (int i = 0; i < m; i++) buf[(*n)++] = lb[m - 1 - i];
  }
  rlp_put_raw(buf, n, p, len);
}

/* Ommer hash of an empty ommer list: keccak(rlp([])). */
static const uint8_t HOST_EMPTY_OMMER[32] = {
    0x1d, 0xcc, 0x4d, 0xe8, 0xde, 0xc7, 0x5d, 0x7a, 0xab, 0x85, 0xb5,
    0x67, 0xb6, 0xcc, 0xd4, 0x1a, 0xd3, 0x12, 0x45, 0x1b, 0x94, 0x8a,
    0x74, 0x13, 0xf0, 0xa1, 0x42, 0xfd, 0x40, 0xd4, 0x93, 0x47};

void host_block_header_hash(
    lbits *rop, const lbits parent_hash, const lbits fee_recipient,
    const lbits state_root, const lbits transactions_root,
    const lbits receipts_root, uint64_t bloom_off, uint64_t number,
    uint64_t gas_limit, uint64_t gas_used, uint64_t timestamp,
    uint64_t extra_off, uint64_t extra_len, const lbits prev_randao,
    const lbits base_fee, const lbits withdrawals_root, uint64_t blob_gas_used,
    uint64_t excess_blob_gas, const lbits parent_beacon_block_root,
    const lbits requests_hash, const lbits block_access_list_hash,
    uint64_t slot_number) {
  const uint8_t *bloom = evmsail_ssz_ptr(bloom_off, 256);
  const uint8_t *extra = NULL;
  if (extra_len != 0) extra = evmsail_ssz_ptr(extra_off, extra_len);
  if (bloom == NULL || extra_len > 1024 || (extra_len != 0 && extra == NULL)) {
    host_keccak256_lbits(rop, NULL, UINT64_MAX); /* invalid witness -> zero digest */
    return;
  }

  uint8_t payload[2048];
  size_t n = 0;
  rlp_put_word32(payload, &n, parent_hash);
  payload[n++] = 0xa0; /* ommers hash */
  rlp_put_raw(payload, &n, HOST_EMPTY_OMMER, 32);
  rlp_put_addr(payload, &n, fee_recipient);
  rlp_put_word32(payload, &n, state_root);
  rlp_put_word32(payload, &n, transactions_root);
  rlp_put_word32(payload, &n, receipts_root);
  rlp_put_str(payload, &n, bloom, 256);
  payload[n++] = 0x80; /* difficulty = 0 */
  n += host_rlp_uint(payload + n, number);
  n += host_rlp_uint(payload + n, gas_limit);
  n += host_rlp_uint(payload + n, gas_used);
  n += host_rlp_uint(payload + n, timestamp);
  {
    const uint8_t empty = 0;
    rlp_put_str(payload, &n, extra ? extra : &empty, (size_t)extra_len);
  }
  rlp_put_word32(payload, &n, prev_randao);
  {
    uint8_t z[8] = {0};
    rlp_put_str(payload, &n, z, 8); /* nonce = 0x0000000000000000 */
  }
  rlp_put_word_min(payload, &n, base_fee);
  rlp_put_word32(payload, &n, withdrawals_root);
  n += host_rlp_uint(payload + n, blob_gas_used);
  n += host_rlp_uint(payload + n, excess_blob_gas);
  rlp_put_word32(payload, &n, parent_beacon_block_root);
  rlp_put_word32(payload, &n, requests_hash);
  rlp_put_word32(payload, &n, block_access_list_hash);
  n += host_rlp_uint(payload + n, slot_number);

  /* frame the list: payload is always > 55 bytes here */
  uint8_t out[2064];
  size_t hn = 0;
  {
    uint8_t lb[8];
    int m = 0;
    for (size_t x = n; x != 0; x >>= 8) lb[m++] = (uint8_t)(x & 0xff);
    out[hn++] = (uint8_t)(0xf7 + m);
    for (int i = 0; i < m; i++) out[hn++] = lb[m - 1 - i];
  }
  memcpy(out + hn, payload, n);
  host_keccak256_lbits(rop, out, hn + n);
}

/* Tx signing preimage hash. The pre-signature RLP content is a witness span
 * (src_kind/off/len); frame it with the optional type-byte domain, the RLP
 * list prefix, and the optional EIP-155 (chain_id, 0, 0) suffix, then keccak. */
void host_tx_signing_hash(lbits *rop, uint64_t ttype, uint64_t apply_155,
                          const lbits chain_id, uint64_t src_kind, uint64_t off,
                          uint64_t len) {
  const uint8_t *content = NULL;
  uint64_t rlen = 0;
  if (!evmsail_resolve_byte_source(src_kind, off, len, &content, &rlen) ||
      rlen != len) {
    host_keccak256_lbits(rop, NULL, UINT64_MAX);
    return;
  }

  uint8_t suffix[36]; /* rlp_int(chain_id) <= 33, then 0x80 0x80 */
  size_t sl = 0;
  if (apply_155) {
    rlp_put_word_min(suffix, &sl, chain_id);
    suffix[sl++] = 0x80;
    suffix[sl++] = 0x80;
  }
  size_t content_len = (size_t)len + sl;

  uint8_t hdr[10]; /* domain byte + list prefix (<= 9) */
  size_t hl = 0;
  if (ttype != 0) hdr[hl++] = (uint8_t)ttype;
  if (content_len <= 55) {
    hdr[hl++] = (uint8_t)(0xc0 + content_len);
  } else {
    uint8_t lb[8];
    int m = 0;
    for (size_t x = content_len; x != 0; x >>= 8) lb[m++] = (uint8_t)(x & 0xff);
    hdr[hl++] = (uint8_t)(0xf7 + m);
    for (int i = 0; i < m; i++) hdr[hl++] = lb[m - 1 - i];
  }

  size_t total = hl + (size_t)len + sl;
  uint8_t *buf = malloc(total);
  if (buf == NULL) {
    host_keccak256_lbits(rop, NULL, UINT64_MAX);
    return;
  }
  memcpy(buf, hdr, hl);
  memcpy(buf + hl, content, (size_t)len);
  memcpy(buf + hl + (size_t)len, suffix, sl);
  host_keccak256_lbits(rop, buf, total);
  free(buf);
}

/* EIP-7702 authorization signing hash: keccak(0x05 ++ rlp([chain_id, address,
 * nonce])). The three signed fields are re-encoded from decoded values. */
void host_auth_signing_hash(lbits *rop, const lbits chain_id, const lbits address,
                            uint64_t nonce) {
  uint8_t payload[64]; /* rlp_int(word) <= 33 + rlp_addr 21 + rlp_int(u64) <= 9 */
  size_t n = 0;
  rlp_put_word_min(payload, &n, chain_id);
  rlp_put_addr(payload, &n, address);
  n += host_rlp_uint(payload + n, nonce);
  uint8_t buf[80];
  size_t bn = 0;
  buf[bn++] = 0x05; /* EIP-7702 magic domain */
  if (n <= 55) {
    buf[bn++] = (uint8_t)(0xc0 + n);
  } else {
    buf[bn++] = 0xf8; /* n < 256 */
    buf[bn++] = (uint8_t)n;
  }
  memcpy(buf + bn, payload, n);
  bn += n;
  host_keccak256_lbits(rop, buf, bn);
}

static int host_bytes_reserve(uint64_t need) {
  if (need <= HOST_bytes_cap) return 1;
  uint64_t cap = HOST_bytes_cap ? HOST_bytes_cap : 256;
  while (cap < need) {
    if (cap > UINT64_MAX / 2) return 0;
    cap *= 2;
  }
  uint8_t *p = realloc(HOST_bytes, (size_t)cap);
  if (!p) return 0;
  HOST_bytes = p;
  HOST_bytes_cap = cap;
  return 1;
}

unit host_bytes_reset(const unit u) {
  (void)u;
  HOST_bytes_len = 0;
  HOST_bytes_ok = 1;
  return UNIT;
}

unit host_bytes_push(uint64_t b) {
  if (HOST_bytes_ok && host_bytes_reserve(HOST_bytes_len + 1)) {
    HOST_bytes[HOST_bytes_len++] = (uint8_t)b;
  } else {
    HOST_bytes_ok = 0;
  }
  return UNIT;
}

unit host_bytes_push8(uint64_t w) {
  if (HOST_bytes_ok && host_bytes_reserve(HOST_bytes_len + 8)) {
    for (int i = 0; i < 8; i++) {
      HOST_bytes[HOST_bytes_len + (uint64_t)i] = (uint8_t)(w >> (8 * (7 - i)));
    }
    HOST_bytes_len += 8;
  } else {
    HOST_bytes_ok = 0;
  }
  return UNIT;
}

void host_bytes_keccak_finish(lbits *rop, const unit u) {
  (void)u;
  if (HOST_bytes_ok) host_keccak256_lbits(rop, HOST_bytes, HOST_bytes_len);
  else host_keccak256_lbits(rop, NULL, UINT64_MAX);
}

void host_bytes_sha256_finish(lbits *rop, const unit u) {
  (void)u;
  if (HOST_bytes_ok) host_sha256_lbits(rop, HOST_bytes, HOST_bytes_len);
  else host_sha256_lbits(rop, NULL, UINT64_MAX);
}

/* ---- trie node-assembly channel (build-side node encode + child_ref) ------
 * Accumulate an MPT node's already-encoded child-ref fields as fixed-width
 * refs, frame them as an RLP list, and return the node's child_ref: the node
 * bytes inline when < 32 bytes, else 0xa0 ++ keccak(node). A ref crosses the
 * FFI as (kind, data, len): kind 0 = empty (RLP 0x80), kind 1 = an inline node
 * (data holds its low `len` big-endian bytes), kind 2 = a keccak hash (data
 * holds the 32-byte hash, framed as 0xa0 ++ hash). Mirrors lib/mpt.sail's
 * branch_child_ref / branch_payload_rlp so the build backend never streams the
 * node byte list through the hash channel. */
#define NASM_CAP 4096
static uint8_t NASM_payload[NASM_CAP];
static size_t NASM_n;
static int NASM_ok = 1;
static uint8_t NASM_node[NASM_CAP + 16];
static size_t NASM_node_len;
static uint64_t NASM_hash[4];

static void nasm_append(const uint8_t *p, size_t len) {
  if (!NASM_ok || NASM_n + len > NASM_CAP) {
    NASM_ok = 0;
    return;
  }
  memcpy(NASM_payload + NASM_n, p, len);
  NASM_n += len;
}

unit node_asm_reset(const unit u) {
  (void)u;
  NASM_n = 0;
  NASM_ok = 1;
  return UNIT;
}

unit node_asm_push_ref(uint64_t kind, const lbits data, uint64_t len) {
  uint8_t buf[33];
  if (kind == 0) {
    buf[0] = 0x80;
    nasm_append(buf, 1);
  } else if (kind == 1) {
    if (len > 32) {
      NASM_ok = 0;
      return UNIT;
    }
    lbits_to_be_bytes(buf, 32, data);
    nasm_append(buf + (32 - (size_t)len), (size_t)len);
  } else if (kind == 2) {
    buf[0] = 0xa0;
    lbits_to_be_bytes(buf + 1, 32, data);
    nasm_append(buf, 33);
  } else {
    NASM_ok = 0;
  }
  return UNIT;
}

/* extract nibble j (0 = most significant) from a big-endian nibble array a32
 * holding `cnt` nibbles right-aligned. */
static uint8_t nasm_nib(const uint8_t *a32, uint64_t cnt, uint64_t j) {
  uint64_t p = 4 * (cnt - 1 - j);
  size_t b = 31 - (size_t)(p / 8);
  return (p % 8 == 0) ? (a32[b] & 0xf) : (a32[b] >> 4);
}

/* append rlp_bytes(hex_prefix_compact(nibbles, is_leaf)) as a node item. The
 * `cnt` nibbles are packed big-endian in the 256-bit `nibbles` word (top nibble
 * first, right-aligned); mirrors lib/mpt.sail hex_prefix_compact. */
unit node_asm_push_path(const lbits nibbles, uint64_t cnt, uint64_t is_leaf) {
  uint8_t a32[32];
  lbits_to_be_bytes(a32, 32, nibbles);
  uint8_t fn = is_leaf ? 2 : 0;
  uint8_t path[33];
  size_t pl = 0;
  uint64_t start;
  if (cnt & 1) {
    path[pl++] = (uint8_t)(((fn | 1) << 4) | nasm_nib(a32, cnt, 0));
    start = 1;
  } else {
    path[pl++] = (uint8_t)(fn << 4);
    start = 0;
  }
  for (uint64_t j = start; j < cnt; j += 2) {
    path[pl++] = (uint8_t)((nasm_nib(a32, cnt, j) << 4) | nasm_nib(a32, cnt, j + 1));
  }
  uint8_t enc[40];
  size_t en = 0;
  rlp_put_str(enc, &en, path, pl);
  nasm_append(enc, en);
  return UNIT;
}

/* frame the accumulated payload as an RLP list; stage node length + inline
 * bytes / keccak so Sail can form the child ref. */
static uint64_t nasm_frame(void) {
  if (!NASM_ok) {
    NASM_node_len = 0;
    return 0;
  }
  size_t hn = 0;
  if (NASM_n <= 55) {
    NASM_node[hn++] = (uint8_t)(0xc0 + NASM_n);
  } else {
    uint8_t lb[8];
    int m = 0;
    for (size_t x = NASM_n; x != 0; x >>= 8) lb[m++] = (uint8_t)(x & 0xff);
    NASM_node[hn++] = (uint8_t)(0xf7 + m);
    for (int i = 0; i < m; i++) NASM_node[hn++] = lb[m - 1 - i];
  }
  memcpy(NASM_node + hn, NASM_payload, NASM_n);
  NASM_node_len = hn + NASM_n;
  if (NASM_node_len >= 32) host_keccak256_bytes(NASM_hash, NASM_node, NASM_node_len);
  return (uint64_t)NASM_node_len;
}

/* branch node: append the empty 17th value slot, then frame. */
uint64_t node_asm_finish_branch(const unit u) {
  (void)u;
  uint8_t vb = 0x80;
  nasm_append(&vb, 1);
  return nasm_frame();
}

/* leaf / extension node: frame the two pushed items as-is. */
uint64_t node_asm_finish(const unit u) {
  (void)u;
  return nasm_frame();
}

/* the finished node's bytes (< 32) right-aligned into a 256-bit word, so Sail
 * reconstructs the inline child ref; valid only when the node is inline. */
void node_asm_result_data(lbits *rop, const unit u) {
  (void)u;
  uint8_t b32[32] = {0};
  size_t n = NASM_node_len < 32 ? NASM_node_len : 32;
  memcpy(b32 + (32 - n), NASM_node, n);
  uint64_t out[4];
  for (int i = 0; i < 4; i++) out[i] = host_be64(b32 + i * 8);
  host_words_to_lbits(rop, out);
}

/* the finished node's keccak (valid when the node is >= 32 bytes). */
void node_asm_result_hash(lbits *rop, const unit u) {
  (void)u;
  host_words_to_lbits(rop, NASM_hash);
}

int evmsail_resolve_byte_source(uint64_t kind, uint64_t off, uint64_t len,
                                const uint8_t **p, uint64_t *resolved_len) {
  const uint8_t *src = NULL;
  if (kind != EVMSAIL_SOURCE_WITNESS &&
      kind != EVMSAIL_SOURCE_MEMORY &&
      kind != EVMSAIL_SOURCE_TX_INPUT &&
      kind != EVMSAIL_SOURCE_ACTIVE_CODE) {
    return 0;
  }
  if (len == 0) {
    src = &HOST_empty_source;
  } else if (kind == EVMSAIL_SOURCE_WITNESS) {
    src = evmsail_ssz_ptr(off, len);
  } else if (kind == EVMSAIL_SOURCE_MEMORY) {
    if (off > UINT64_MAX - (len - 1)) return 0;
    src = hm_rd(off, len);
  } else if (kind == EVMSAIL_SOURCE_TX_INPUT) {
    src = txd_rd(off, len);
  } else if (kind == EVMSAIL_SOURCE_ACTIVE_CODE) {
    return code_db_frame_resolve_code(off, len, p, resolved_len);
  } else {
    return 0;
  }
  if (src == NULL) return 0;
  if (p) *p = src;
  if (resolved_len) *resolved_len = len;
  return 1;
}

void host_source_keccak(lbits *rop, uint64_t source_kind, uint64_t off, uint64_t len) {
  const uint8_t *src = NULL;
  uint64_t resolved_len = 0;
  if (evmsail_resolve_byte_source(source_kind, off, len, &src, &resolved_len) &&
      resolved_len == len) {
    host_keccak256_lbits(rop, src, len);
  } else {
    host_keccak256_lbits(rop, NULL, UINT64_MAX);
  }
}

void host_source_sha256_prefixed(lbits *rop, uint64_t prefix, uint64_t source_kind,
                                 uint64_t off, uint64_t len) {
  const uint8_t *src = NULL;
  uint64_t resolved_len = 0;
  uint64_t out[4] = {0, 0, 0, 0};
  if (len <= UINT32_MAX - 1 &&
      evmsail_resolve_byte_source(source_kind, off, len, &src, &resolved_len) &&
      resolved_len == len) {
    uint8_t *prefixed = malloc((size_t)len + 1);
    if (prefixed != NULL) {
      prefixed[0] = (uint8_t)prefix;
      if (len != 0) memcpy(prefixed + 1, src, (size_t)len);
      host_sha256_bytes(out, prefixed, len + 1);
      free(prefixed);
    }
  }
  host_words_to_lbits(rop, out);
}
