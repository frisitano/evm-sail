#ifndef EVMSAIL_SCRATCH_H
#define EVMSAIL_SCRATCH_H

#include "sail.h"

#include <stdbool.h>
#include <stdint.h>

uint8_t *scratch_prepare(uint64_t off, uint64_t len);
bool scratch_commit(uint64_t off, uint64_t len);
bool scratch_append_source(uint64_t dst, uint64_t source, uint64_t off,
                           uint64_t len);
unit scratch_truncate(uint64_t len);
const uint8_t *scratch_region(uint64_t off, uint64_t len);

#endif
