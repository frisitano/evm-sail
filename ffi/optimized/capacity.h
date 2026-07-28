#ifndef EVMSAIL_CAPACITY_H
#define EVMSAIL_CAPACITY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum evmsail_capacity_region {
  EVMSAIL_CAP_MEMORY_BYTES = 0,
  EVMSAIL_CAP_SCRATCH_BYTES,
  EVMSAIL_CAP_CODE_BYTES,
  EVMSAIL_CAP_JUMPDEST_WORDS,
  EVMSAIL_CAP_LOG_DATA_BYTES,
  EVMSAIL_CAP_OUTPUT_BYTES,
  EVMSAIL_CAP_REGION_COUNT
};

#define EVMSAIL_CAPACITY_MAGIC "EVMCAP01"
#define EVMSAIL_CAPACITY_TRAILER_SIZE \
  (24u + 8u * (unsigned)EVMSAIL_CAP_REGION_COUNT)

/*
 * Configure one execution input. Fixed optimized builds consume and strip the
 * capacity trailer. Measurement and standard builds leave the input untouched.
 */
bool evmsail_capacity_prepare_input(const uint8_t *input, size_t input_size,
                                    size_t *canonical_size);

/* Measurement builds record exact requested high-water marks. */
void evmsail_capacity_observe(enum evmsail_capacity_region region,
                              uint64_t need);
void evmsail_capacity_measure_reset(void);
uint64_t evmsail_capacity_measure_get(uint64_t region);

/* Fixed builds expose the per-execution limit to backing arenas. */
uint64_t evmsail_capacity_limit(enum evmsail_capacity_region region);

#endif
