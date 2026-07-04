#include "host_crypto.h"
#include "memory.h"
#include "zkvm_accelerators.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern const uint8_t *evmsail_ssz_ptr(uint64_t off, uint64_t len);

static uint64_t HOST_digest[4];
static uint8_t *HOST_bytes;
static uint64_t HOST_bytes_len;
static uint64_t HOST_bytes_cap;

#ifdef ACCEL_MMIO
static const uintptr_t CRYPTO_MMIO_BASE = 0x40000000UL;
enum {
  CRYPTO_R_OP = 0,
  CRYPTO_R_IN = 1,
  CRYPTO_R_INLEN = 2,
  CRYPTO_R_OUT = 3,
  CRYPTO_R_GO = 4,
  CRYPTO_R_OUTLEN = 5,
  CRYPTO_R_OK = 6,
};

static void crypto_device_call(uint64_t op, const uint8_t *in, uint32_t inlen,
                               uint8_t *out, uint32_t *outlen, int *ok) {
  volatile uint64_t *d = (volatile uint64_t *)CRYPTO_MMIO_BASE;
  d[CRYPTO_R_OP] = op;
  d[CRYPTO_R_IN] = (uint64_t)(uintptr_t)in;
  d[CRYPTO_R_INLEN] = inlen;
  d[CRYPTO_R_OUT] = (uint64_t)(uintptr_t)out;
  d[CRYPTO_R_GO] = 1;
  *ok = (int)d[CRYPTO_R_OK];
  *outlen = (uint32_t)d[CRYPTO_R_OUTLEN];
}
#endif

static uint64_t host_be64(const uint8_t *p) {
  uint64_t w = 0;
  for (int i = 0; i < 8; i++) w = (w << 8) | p[i];
  return w;
}

static int host_bytes_fit(uint64_t need) {
  if (need <= HOST_bytes_cap) return 1;
  uint64_t n = HOST_bytes_cap ? HOST_bytes_cap : 128;
  while (n < need) {
    if (n > UINT64_MAX / 2) return 0;
    n *= 2;
  }
  if (n > (uint64_t)SIZE_MAX) return 0;
  uint8_t *p = (uint8_t *)realloc(HOST_bytes, (size_t)n);
  if (p == NULL) return 0;
  HOST_bytes = p;
  HOST_bytes_cap = n;
  return 1;
}

static void host_bytes_append_raw(const uint8_t *p, uint64_t len) {
  if (len == 0) return;
  if (p == NULL) return;
  if (len > UINT64_MAX - HOST_bytes_len) return;
  if (!host_bytes_fit(HOST_bytes_len + len)) return;
  memcpy(HOST_bytes + HOST_bytes_len, p, (size_t)len);
  HOST_bytes_len += len;
}

static void host_bytes_append_byte_raw(uint8_t b) {
  if (!host_bytes_fit(HOST_bytes_len + 1)) return;
  HOST_bytes[HOST_bytes_len++] = b;
}

static void host_bytes_append8_raw(uint64_t w) {
  if (!host_bytes_fit(HOST_bytes_len + 8)) return;
  for (int i = 7; i >= 0; i--) HOST_bytes[HOST_bytes_len++] = (uint8_t)(w >> (8 * i));
}

static void host_hash_bytes(uint64_t id, uint64_t out[4], const uint8_t *p, uint64_t len) {
  uint8_t digest[32] = {0};
  uint8_t empty = 0;
  const uint8_t *src = len ? p : &empty;
  int ok = (src != NULL && len <= UINT32_MAX);

  if (ok) {
#ifdef ACCEL_MMIO
    uint32_t outlen = 0;
    int dev_ok = 0;
    crypto_device_call(id, src, (uint32_t)len, digest, &outlen, &dev_ok);
    ok = dev_ok && outlen >= 32;
#else
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
#endif
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

unit host_bytes_reset(const unit u) {
  (void)u;
  HOST_bytes_len = 0;
  return UNIT;
}

unit host_bytes_append_byte(uint64_t b) {
  host_bytes_append_byte_raw((uint8_t)b);
  return UNIT;
}

unit host_bytes_append8(uint64_t w) {
  host_bytes_append8_raw(w);
  return UNIT;
}

const uint8_t *host_bytes_data(uint64_t *len_out) {
  if (len_out) *len_out = HOST_bytes_len;
  return HOST_bytes_len ? HOST_bytes : NULL;
}

unit host_keccak_input(const unit u) {
  (void)u;
  host_keccak256_bytes(HOST_digest, HOST_bytes, HOST_bytes_len);
  return UNIT;
}

unit host_sha256_input(const unit u) {
  (void)u;
  host_sha256_bytes(HOST_digest, HOST_bytes, HOST_bytes_len);
  return UNIT;
}

unit host_keccak_witness(uint64_t off, uint64_t len) {
  const uint8_t *p = evmsail_ssz_ptr(off, len);
  host_keccak256_bytes(HOST_digest, p, len);
  return UNIT;
}

unit host_sha256_request_digest(uint64_t request_type, uint64_t off, uint64_t len) {
  HOST_bytes_len = 0;
  host_bytes_append_byte_raw((uint8_t)request_type);
  host_bytes_append_raw(evmsail_ssz_ptr(off, len), len);
  host_sha256_bytes(HOST_digest, HOST_bytes, HOST_bytes_len);
  HOST_bytes_len = 0;
  return UNIT;
}

unit host_keccak_word(uint64_t w0, uint64_t w1, uint64_t w2, uint64_t w3) {
  HOST_bytes_len = 0;
  host_bytes_append8_raw(w0);
  host_bytes_append8_raw(w1);
  host_bytes_append8_raw(w2);
  host_bytes_append8_raw(w3);
  host_keccak256_bytes(HOST_digest, HOST_bytes, HOST_bytes_len);
  HOST_bytes_len = 0;
  return UNIT;
}

unit host_keccak_address(uint64_t a0, uint64_t a1, uint64_t a2) {
  HOST_bytes_len = 0;
  host_bytes_append8_raw(a0);
  host_bytes_append8_raw(a1);
  if (host_bytes_fit(HOST_bytes_len + 4)) {
    HOST_bytes[HOST_bytes_len++] = (uint8_t)(a2 >> 24);
    HOST_bytes[HOST_bytes_len++] = (uint8_t)(a2 >> 16);
    HOST_bytes[HOST_bytes_len++] = (uint8_t)(a2 >> 8);
    HOST_bytes[HOST_bytes_len++] = (uint8_t)a2;
  }
  host_keccak256_bytes(HOST_digest, HOST_bytes, HOST_bytes_len);
  HOST_bytes_len = 0;
  return UNIT;
}

unit host_sha256_pair_words(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                            uint64_t b0, uint64_t b1, uint64_t b2, uint64_t b3) {
  HOST_bytes_len = 0;
  host_bytes_append8_raw(a0);
  host_bytes_append8_raw(a1);
  host_bytes_append8_raw(a2);
  host_bytes_append8_raw(a3);
  host_bytes_append8_raw(b0);
  host_bytes_append8_raw(b1);
  host_bytes_append8_raw(b2);
  host_bytes_append8_raw(b3);
  host_sha256_bytes(HOST_digest, HOST_bytes, HOST_bytes_len);
  HOST_bytes_len = 0;
  return UNIT;
}

unit host_keccak_memory(uint64_t off, uint64_t len) {
  const uint8_t *p = len ? hm_rd(off, len) : NULL;
  host_keccak256_bytes(HOST_digest, p, len);
  return UNIT;
}

uint64_t host_hash_word(uint64_t i) {
  return i < 4 ? HOST_digest[i] : 0;
}
