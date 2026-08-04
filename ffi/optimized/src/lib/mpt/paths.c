/* Nibble-path primitive implementations. See paths.h. */
#include "lib/mpt/paths.h"

/* The shared lib/mpt first-failure latch declared in common.h. */
uint64_t mpt_status = MPT_OK;

#include <string.h>

/* ======================================================================== */

NibblePath nibble_path_empty(void) {
  NibblePath path;
  memset(&path, 0, sizeof(path));
  return path;
}

NibblePath nibble_path_from_secure_key(const Hash32 *hash) {
  NibblePath path;
  const uint8_t *bytes = hash_bytes_const(hash);
  path.len = 64;
  for (size_t i = 0; i < 32; ++i) {
    path.nibbles[2 * i] = bytes[i] >> 4;
    path.nibbles[2 * i + 1] = bytes[i] & 0x0f;
  }
  return path;
}

bool nibble_path_concat(const NibblePath *a,
                        const NibblePath *b, NibblePath *out) {
  if ((unsigned)a->len + b->len > MPT_SECURE_KEY_NIBBLES)
    return mpt_fail(MPT_WITNESS_DEFICIENT);
  *out = nibble_path_empty();
  out->len = a->len + b->len;
  memcpy(out->nibbles, a->nibbles, a->len);
  memcpy(out->nibbles + a->len, b->nibbles, b->len);
  return true;
}

NibblePath nibble_path_drop(const NibblePath *path,
                                    unsigned count) {
  NibblePath out = nibble_path_empty();
  if (count >= path->len)
    return out;
  out.len = path->len - count;
  memcpy(out.nibbles, path->nibbles + count, out.len);
  return out;
}

NibblePath nibble_path_slice(const NibblePath *path,
                                     unsigned start, unsigned len) {
  NibblePath out = nibble_path_empty();
  if (start >= path->len)
    return out;
  if (len > (unsigned)path->len - start)
    len = path->len - start;
  out.len = (uint8_t)len;
  memcpy(out.nibbles, path->nibbles + start, len);
  return out;
}

NibblePath nibble_path_single(unsigned nibble) {
  NibblePath out = nibble_path_empty();
  out.len = 1;
  out.nibbles[0] = (uint8_t)nibble;
  return out;
}

bool nibble_path_equal(const NibblePath *a,
                       const NibblePath *b) {
  return a->len == b->len && memcmp(a->nibbles, b->nibbles, a->len) == 0;
}

bool nibble_path_less(const NibblePath *a,
                      const NibblePath *b) {
  const size_t n = a->len < b->len ? a->len : b->len;
  const int order = memcmp(a->nibbles, b->nibbles, n);
  return order < 0 || (order == 0 && a->len < b->len);
}

bool nibble_path_prefix(const NibblePath *prefix,
                        const NibblePath *path) {
  return prefix->len <= path->len &&
         memcmp(prefix->nibbles, path->nibbles, prefix->len) == 0;
}

unsigned nibble_path_common(const NibblePath *a,
                            const NibblePath *b) {
  const unsigned stop = a->len < b->len ? a->len : b->len;
  unsigned common = 0;
  while (common < stop && a->nibbles[common] == b->nibbles[common])
    ++common;
  return common;
}

uint8_t mpt_hash_nibble(const Hash32 *key, unsigned position) {
  const uint8_t byte = hash_bytes_const(key)[position / 2];
  return (position & 1) ? byte & 0x0f : byte >> 4;
}

bool mpt_key_matches(const Hash32 *key, unsigned position,
                     const NibblePath *path) {
  if (position + path->len > MPT_SECURE_KEY_NIBBLES)
    return false;
  for (unsigned i = 0; i < path->len; ++i)
    if (path->nibbles[i] != mpt_hash_nibble(key, position + i))
      return false;
  return true;
}
