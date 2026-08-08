#ifndef EVMSAIL_OPTIMIZED_HOST_SCRATCH_H
#define EVMSAIL_OPTIMIZED_HOST_SCRATCH_H

#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stdint.h>

uint8_t *scratch_prepare(uint64_t off, uint64_t len);
/* Advance the bump cursor for a span proved by the paired scratch_prepare. */
void scratch_commit(uint64_t off, uint64_t len);
/*
 * Borrows uncommitted space above the current Sail-owned high-water mark.
 * The bytes are valid only until the next scratch operation and never change
 * the visible arena length.
 */
uint8_t *scratch_borrow(uint64_t len);
const uint8_t *scratch_base(void);

#endif
