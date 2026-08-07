/* Nibble-path primitives: the trie key-fragment type and its operations,
 * plus direct secure-key nibble access. Pure functions, no module state.
 * Mirrors the Sail lib/mpt paths/hex-prefix layer; the compact (hex-prefix)
 * codec itself lives with the node RLP in codec.h because it reads RLP
 * items. */
#ifndef EVMSAIL_OPTIMIZED_LIB_MPT_PATHS_H
#define EVMSAIL_OPTIMIZED_LIB_MPT_PATHS_H

#include "lib/mpt/common.h"

/* A trie key fragment: up to one full secure key, one nibble per slot. */
typedef struct {
  uint8_t nibbles[MPT_SECURE_KEY_NIBBLES];
  uint8_t len;
} NibblePath;

NibblePath nibble_path_empty(void);
NibblePath nibble_path_from_secure_key(const bytes32 *hash);
/* Concatenation whose only failure (secure-key overflow) is the Sail
 * path_concat throw, realized directly via fatal_error. */
void nibble_path_concat(const NibblePath *a, const NibblePath *b, NibblePath *out);
NibblePath nibble_path_drop(const NibblePath *path, unsigned count);
NibblePath nibble_path_slice(const NibblePath *path, unsigned start, unsigned len);
NibblePath nibble_path_single(unsigned nibble);
bool nibble_path_equal(const NibblePath *a, const NibblePath *b);
bool nibble_path_less(const NibblePath *a, const NibblePath *b);
bool nibble_path_prefix(const NibblePath *prefix, const NibblePath *path);
unsigned nibble_path_common(const NibblePath *a, const NibblePath *b);

/* Secure-key nibble access without materializing the full path. */
uint8_t mpt_hash_nibble(const bytes32 *key, unsigned position);
bool mpt_key_matches(const bytes32 *key, unsigned position, const NibblePath *path);

#endif
