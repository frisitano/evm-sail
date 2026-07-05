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

static void host_put_be64(uint8_t *p, uint64_t w) {
  for (int i = 7; i >= 0; i--) {
    p[7 - i] = (uint8_t)(w >> (8 * i));
  }
}

static void host_put_be32(uint8_t *p, uint64_t w) {
  for (int i = 3; i >= 0; i--) {
    p[3 - i] = (uint8_t)(w >> (8 * i));
  }
}

static void host_put_word(uint8_t *p, uint64_t w3, uint64_t w2, uint64_t w1, uint64_t w0) {
  host_put_be64(p + 0, w3);
  host_put_be64(p + 8, w2);
  host_put_be64(p + 16, w1);
  host_put_be64(p + 24, w0);
}

static void host_put_address(uint8_t *p, uint64_t a2, uint64_t a1, uint64_t a0) {
  host_put_be64(p + 0, a2);
  host_put_be64(p + 8, a1);
  host_put_be32(p + 16, a0);
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

void host_keccak_word(lbits *rop, uint64_t w3, uint64_t w2, uint64_t w1, uint64_t w0) {
  uint8_t buf[32];
  host_put_word(buf, w3, w2, w1, w0);
  host_keccak256_lbits(rop, buf, sizeof buf);
}

void host_keccak_address(lbits *rop, uint64_t a2, uint64_t a1, uint64_t a0) {
  uint8_t buf[20];
  host_put_address(buf, a2, a1, a0);
  host_keccak256_lbits(rop, buf, sizeof buf);
}

void host_keccak_create2(lbits *rop, uint64_t a2, uint64_t a1, uint64_t a0,
                         uint64_t salt3, uint64_t salt2, uint64_t salt1, uint64_t salt0,
                         uint64_t init3, uint64_t init2, uint64_t init1, uint64_t init0) {
  uint8_t buf[85];
  buf[0] = 0xff;
  host_put_address(buf + 1, a2, a1, a0);
  host_put_word(buf + 21, salt3, salt2, salt1, salt0);
  host_put_word(buf + 53, init3, init2, init1, init0);
  host_keccak256_lbits(rop, buf, sizeof buf);
}

void host_sha256_pair(lbits *rop, uint64_t a3, uint64_t a2, uint64_t a1, uint64_t a0,
                      uint64_t b3, uint64_t b2, uint64_t b1, uint64_t b0) {
  uint8_t buf[64];
  host_put_word(buf, a3, a2, a1, a0);
  host_put_word(buf + 32, b3, b2, b1, b0);
  host_sha256_lbits(rop, buf, sizeof buf);
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
