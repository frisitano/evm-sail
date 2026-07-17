#include "host_crypto.h"
#include "lbits_convert.h"
#include "zkvm_accelerators.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
