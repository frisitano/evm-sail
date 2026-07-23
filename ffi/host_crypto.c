#include "host_crypto.h"
#include "value_convert.h"
#include "zkvm_accelerators.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static uint64_t host_be64(const uint8_t *p) {
  uint64_t w = 0;
  for (int i = 0; i < 8; i++) w = (w << 8) | p[i];
  return w;
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
