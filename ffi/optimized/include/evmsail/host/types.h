#ifndef EVMSAIL_HOST_TYPES_H
#define EVMSAIL_HOST_TYPES_H

#include <stdint.h>

/* Nominal host-region slices share a fixed two-word layout while retaining
 * distinct Sail types. Sail's optimized-model backend includes these concrete
 * declarations instead of duplicating them in generated model headers. */
struct StatelessInputSliceFields {
  uint64_t len;
  uint64_t off;
};

struct ScratchSliceFields {
  uint64_t len;
  uint64_t off;
};

struct EvmMemorySliceFields {
  uint64_t len;
  uint64_t off;
};

struct CodeRegionSliceFields {
  uint64_t len;
  uint64_t off;
};

struct LogDataSliceFields {
  uint64_t len;
  uint64_t off;
};

struct OutputSliceFields {
  uint64_t len;
  uint64_t off;
};

#endif /* EVMSAIL_HOST_TYPES_H */
