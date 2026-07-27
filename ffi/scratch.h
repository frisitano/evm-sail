#ifndef EVMSAIL_SCRATCH_H
#define EVMSAIL_SCRATCH_H

#include "quantity_abi.h"
#include "sail.h"

#include <stdbool.h>
#include <stdint.h>

uint8_t *scratch_prepare(uint64_t off, uint64_t len);
bool scratch_commit(uint64_t off, uint64_t len);
/*
 * Borrows uncommitted space above the current Sail-owned high-water mark.
 * The bytes are valid only until the next scratch operation and never change
 * the visible arena length.
 */
uint8_t *scratch_borrow(uint64_t len);
bool scratch_append_source(uint64_t dst, uint64_t source, uint64_t off,
                           uint64_t len);
unit scratch_truncate(EVMSAIL_BYTE_QUANTITY_PARAM(len));
const uint8_t *scratch_region(uint64_t off, uint64_t len);

#endif
