#include "capacity.h"

#include "code_db.h"
#include "kernel_state.h"
#include "memory.h"
#include "output.h"
#include "scratch.h"

#include <stdlib.h>
#include <string.h>

static uint64_t measured[EVMSAIL_CAP_REGION_COUNT];
static uint64_t limits[EVMSAIL_CAP_REGION_COUNT];

static uint32_t load_u32_le(const uint8_t *p) {
  return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
         (uint32_t)p[3] << 24;
}

static uint64_t load_u64_le(const uint8_t *p) {
  uint64_t value = 0;
  for (unsigned i = 0; i < 8; ++i) value |= (uint64_t)p[i] << (8 * i);
  return value;
}

void evmsail_capacity_measure_reset(void) {
  memset(measured, 0, sizeof(measured));
}

void evmsail_capacity_observe(enum evmsail_capacity_region region,
                              uint64_t need) {
  (void)region;
  (void)need;
}

uint64_t evmsail_capacity_measure_get(uint64_t region) {
  return region < EVMSAIL_CAP_REGION_COUNT ? measured[region] : 0;
}

uint64_t evmsail_capacity_limit(enum evmsail_capacity_region region) {
  return (unsigned)region < EVMSAIL_CAP_REGION_COUNT ? limits[region] : 0;
}

bool evmsail_capacity_prepare_input(const uint8_t *input, size_t input_size,
                                    size_t *canonical_size) {
  if (!canonical_size) return false;
  (void)input;
  evmsail_capacity_measure_reset();
  *canonical_size = input_size;
  return true;
}
