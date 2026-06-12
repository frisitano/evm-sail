/* C-backed layered (address, slot) -> 256-bit value maps (see el_map.c):
 * id 0 = transient storage, id 1 = warm-slot set, id 2 = persistent STORAGE
 * (overlay layers: push at snapshot, pop_commit / pop_discard at commit/revert;
 * base layer = tx-start state = the EIP-2200 "original" values). Declared here
 * so the Sail-generated C call sites are prototyped via `sail -c --c-include`.
 * Only mach_bits (uint64_t) cross the FFI. */
#ifndef EL_MAP_H
#define EL_MAP_H
#include "sail.h"
#include <stdint.h>
unit el_map_reset(uint64_t id);                       /* clear to one empty base layer */
unit el_map_push(uint64_t id);                        /* open an overlay (frame/tx)    */
unit el_map_pop_commit(uint64_t id);                  /* merge top into the layer below */
unit el_map_pop_discard(uint64_t id);                 /* drop top (revert)             */
unit el_map_key(uint64_t id, uint64_t a2, uint64_t a1, uint64_t a0,
                uint64_t s3, uint64_t s2, uint64_t s1, uint64_t s0);  /* select key   */
unit el_map_store(uint64_t v3, uint64_t v2, uint64_t v1, uint64_t v0); /* write (top) */
uint64_t el_map_word(uint64_t i);                     /* value word (3=msw..0), layered */
uint64_t el_map_base_word(uint64_t i);                /* value word, BASE layer only    */
uint64_t el_map_present(const unit u);                /* 1 if key present in any layer  */
uint64_t el_map_has_addr(uint64_t id, uint64_t a2, uint64_t a1, uint64_t a0);
unit el_map_wipe_addr(uint64_t id, uint64_t a2, uint64_t a1, uint64_t a0);
uint64_t el_map_count(uint64_t id);                   /* flatten; returns entry count   */
unit el_map_sel(uint64_t j);                          /* select flattened entry j       */
uint64_t el_map_it_key(uint64_t i);                   /* key word: 0..2 addr, 3..6 slot */
uint64_t el_map_it_val(uint64_t i);                   /* value word (3=msw..0)          */
#endif
