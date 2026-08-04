#ifndef EVMSAIL_HOST_TYPES_H
#define EVMSAIL_HOST_TYPES_H

#include <stdint.h>

/* Nominal host-region slices retain semantic Sail coordinates, but optimized
 * C lowers every live byte coordinate to a stable pointer. */
struct StatelessInputSliceFields {
  uint64_t len;
  uint8_t *bytes;
};

struct ScratchSliceFields {
  uint64_t len;
  uint8_t *bytes;
};

struct EvmMemorySliceFields {
  uint64_t len;
  uint8_t *bytes;
};

struct CodeRegionSliceFields {
  uint64_t len;
  uint8_t *bytes;
};

struct LogDataSliceFields {
  uint64_t len;
  uint8_t *bytes;
};

struct OutputSliceFields {
  uint64_t len;
  uint8_t *bytes;
};

/* Constructor adapters used only by generated optimized C when a semantic
 * offset enters one of the pointer-backed record fields above. */
uint8_t *stateless_input_at(uint64_t off);
uint8_t *scratch_at(uint64_t off);
uint8_t *memory_at(uint64_t off);
uint8_t *code_at(uint64_t off);
uint8_t *log_data_at(uint64_t off);
uint8_t *output_at(uint64_t off);

#endif /* EVMSAIL_HOST_TYPES_H */
