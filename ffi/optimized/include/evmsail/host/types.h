#ifndef EVMSAIL_HOST_TYPES_H
#define EVMSAIL_HOST_TYPES_H

#include <stdint.h>

/* Nominal host-region slices retain semantic Sail coordinates, but optimized
 * C lowers every live byte coordinate to a stable pointer. */
struct StatelessInputSliceFields {
  uint32_t len;
  uint8_t *bytes;
};

struct ScratchSliceFields {
  uint32_t len;
  uint8_t *bytes;
};

struct EvmMemorySliceFields {
  uint32_t len;
  uint8_t *bytes;
};

struct CodeRegionSliceFields {
  uint32_t len;
  uint8_t *bytes;
};

struct LogDataSliceFields {
  uint32_t len;
  uint8_t *bytes;
};

struct OutputSliceFields {
  uint32_t len;
  uint8_t *bytes;
};

#endif /* EVMSAIL_HOST_TYPES_H */
