/* Fixed-layout code-region and lookup adapters for the optimized ABI. */
#include "evmsail/prelude.h"
#include "evmsail/spec/exceptions.h"
#include "host/code/store.h"
#include "evmsail/host/region_access.h"
#include "primitives/hash.h"
#include "primitives/value.h"
#include "workspace.h"

#include <stdlib.h>
#include <stdint.h>

static struct CodeRegionSliceFields code_region_value(uint8_t *bytes, uint32_t len)
{
  struct CodeRegionSliceFields out;
  out.bytes = bytes;
  out.len = len;
  return out;
}

/* Unstable source bytes are copied into the arena and registered; workspace
 * exhaustion fails closed. */
static struct CodeRegionSliceFields code_region_from_bytes(const uint8_t *bytes, uint32_t len)
{
  if (len == 0) {
    return code_region_value(0, 0);
  }
  if (!bytes) {
    GUEST_ABORT();
  }
  uint8_t *stable = code_region_intern_copy(bytes, len);
  if (!stable) {
    GUEST_ABORT();
  }
  return code_region_value(stable, len);
}

/* Input-backed code is referenced in place: the row records the resolved
 * input pointer and the bytes are never copied. A slice that fails
 * re-validation against the live input is structural corruption, not a
 * witness deficiency. */
struct CodeRegionSliceFields code_region_from_input(struct StatelessInputSliceFields input)
{
  const uint32_t len = input.len;
  if (len == 0) {
    return code_region_value(0, 0);
  }
  const uint8_t *bytes = input.bytes;
  if (!bytes) {
    GUEST_ABORT();
  }
  return code_region_value(sail_read_only_bytes(bytes), len);
}

#define DEFINE_CODE_REGION_FROM(name, type)                                                        \
  struct CodeRegionSliceFields name(struct type input)                                             \
  {                                                                                                \
    return code_region_from_bytes(input.bytes, input.len);                                         \
  }

DEFINE_CODE_REGION_FROM(code_region_from_memory, EvmMemorySliceFields)
DEFINE_CODE_REGION_FROM(code_region_from_output, OutputSliceFields)

#undef DEFINE_CODE_REGION_FROM

struct CodeRegionSliceFields code_region_from_delegation(Address address)
{
  uint8_t bytes[23] = {0xef, 0x01, 0x00};
  address_to_be_bytes(bytes + 3, address);
  return code_region_from_bytes(bytes, sizeof(bytes));
}

struct CodeFields code_db_resolve_indexed(Hash32 hash)
{
  struct CodeFields code = {0};
  if (hash_equal(&hash, &EVMSAIL_KECCAK_EMPTY)) {
    return code;
  }

  const uint8_t *bytes = NULL;
  uint8_t *jumpdests = NULL;
  uint32_t len = 0;
  if (!code_db_lookup_view(hash, &bytes, &len, &jumpdests)) {
    fatal_error(WitnessDeficient);
  }
  code.bytes = sail_read_only_bytes(bytes);
  code.len = len;
  code.jumpdests = jumpdests;
  return code;
}
