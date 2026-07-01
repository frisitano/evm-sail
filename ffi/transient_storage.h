/* C-backed transient storage table (see transient_storage.c).
 *
 * Persistent account storage is implemented separately by storage_map_* in
 * state_db.c/.h, keyed by secure trie keys. Warm access sets live in Sail.
 *
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. Only mach_bits (uint64_t) cross the FFI. */
#ifndef TRANSIENT_STORAGE_H
#define TRANSIENT_STORAGE_H
#include "sail.h"
#include <stdint.h>

unit transient_storage_reset(uint64_t id);
unit transient_storage_key(uint64_t id, uint64_t a2, uint64_t a1, uint64_t a0,
                           uint64_t s3, uint64_t s2, uint64_t s1, uint64_t s0);
unit transient_storage_store(uint64_t v3, uint64_t v2, uint64_t v1, uint64_t v0);
uint64_t transient_storage_word(uint64_t i);

#endif
