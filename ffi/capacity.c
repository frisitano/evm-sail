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
#ifdef EVMSAIL_CAPACITY_MEASURE
  if ((unsigned)region < EVMSAIL_CAP_REGION_COUNT &&
      measured[region] < need)
    measured[region] = need;
#else
  (void)region;
  (void)need;
#endif
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
#ifdef EVMSAIL_CAPACITY_FIXED
  if (!input || input_size < EVMSAIL_CAPACITY_TRAILER_SIZE) return false;
  const size_t trailer_off = input_size - EVMSAIL_CAPACITY_TRAILER_SIZE;
  const uint8_t *trailer = input + trailer_off;
  if (memcmp(trailer, EVMSAIL_CAPACITY_MAGIC, 8) != 0 ||
      load_u32_le(trailer + 8) != 1 ||
      load_u32_le(trailer + 12) != EVMSAIL_CAP_REGION_COUNT ||
      load_u64_le(trailer + 16) != trailer_off)
    return false;

  for (unsigned i = 0; i < EVMSAIL_CAP_REGION_COUNT; ++i)
    limits[i] = load_u64_le(trailer + 24 + 8 * i);

  if (!evm_memory_configure_capacity(limits[EVMSAIL_CAP_MEMORY_BYTES]) ||
      !scratch_configure_capacity(limits[EVMSAIL_CAP_SCRATCH_BYTES]) ||
      !code_db_configure_capacities(limits[EVMSAIL_CAP_CODE_BYTES],
                                    limits[EVMSAIL_CAP_JUMPDEST_WORDS]) ||
      !log_data_configure_capacity(limits[EVMSAIL_CAP_LOG_DATA_BYTES]) ||
      !output_buffer_configure_capacity(limits[EVMSAIL_CAP_OUTPUT_BYTES]))
    return false;
  *canonical_size = trailer_off;
#else
  (void)input;
  evmsail_capacity_measure_reset();
  *canonical_size = input_size;
#endif
  return true;
}
