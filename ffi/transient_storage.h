/* C-backed transient storage table (see transient_storage.c).
 *
 * Persistent account storage is implemented separately by storage_map_* in
 * state_db.c/.h, keyed by secure trie keys. Warm access sets live in Sail.
 *
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. Addresses/slots/values cross as whole lbits. */
#ifndef TRANSIENT_STORAGE_H
#define TRANSIENT_STORAGE_H
#include "sail.h"
#include <stdint.h>

unit transient_storage_reset(uint64_t id);
unit transient_storage_write(uint64_t id, const lbits addr, const lbits slot, const lbits v);
void transient_storage_read(lbits *rop, uint64_t id, const lbits addr, const lbits slot);

#endif
