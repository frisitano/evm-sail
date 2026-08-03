/* Fixed-layout code-region and lookup adapters for the optimized ABI. */
#include "evmsail/prelude.h"
#include "evmsail/host/code.h"
#include "host/code/store.h"
#include "evmsail/host/region_access.h"
#include "workspace.h"

#include <stdlib.h>
#include <stdint.h>

static struct CodeRegionSliceFields code_region_value(uint64_t off,
                                                       uint64_t len) {
  struct CodeRegionSliceFields out;
  out.off = off;
  out.len = len;
  return out;
}

/* Unstable source bytes are copied into the arena and registered; workspace
 * exhaustion fails closed. */
static struct CodeRegionSliceFields code_region_from_bytes(
    const uint8_t *bytes, uint64_t len) {
  if (len == 0) return code_region_value(0, 0);
  if (!bytes) GUEST_ABORT();
  const uint64_t region = code_region_intern_copy(bytes, len);
  if (region == 0) GUEST_ABORT();
  return code_region_value(region, len);
}

/* Input-backed code is referenced in place: the row records the resolved
 * input pointer and the bytes are never copied. A slice that fails
 * re-validation against the live input is structural corruption, not a
 * witness deficiency. */
struct CodeRegionSliceFields code_region_from_input(
    struct StatelessInputSliceFields input) {
  const uint64_t off = input.off;
  const uint64_t len = input.len;
  if (len == 0) return code_region_value(0, 0);
  const uint8_t *bytes = stateless_input_ptr(off, len);
  if (!bytes) GUEST_ABORT();
  const uint64_t region = code_region_register(bytes, len);
  if (region == 0) GUEST_ABORT();
  return code_region_value(region, len);
}

struct CodeRegionSliceFields code_region_from_memory(
    struct EvmMemorySliceFields input) {
  const uint64_t off = input.off;
  const uint64_t len = input.len;
  return code_region_from_bytes(memory_ptr(off, len), len);
}

struct CodeRegionSliceFields code_region_from_output(
    struct OutputSliceFields input) {
  const uint64_t off = input.off;
  const uint64_t len = input.len;
  return code_region_from_bytes(output_ptr(off, len), len);
}

struct zoptionzIRCodezK code_db_lookup(Hash32 hash) {
  struct zoptionzIRCodezK out = {0};
  uint64_t off = 0, len = 0, jumpdest_ref = 0;
  if (!code_db_lookup_indexed(hash, &off, &len, &jumpdest_ref)) {
    out.kind = Kind_zNonezIRCodezK;
    out.variants.zNonezIRCodezK = UNIT;
    return out;
  }
  out.kind = Kind_zSomezIRCodezK;
  struct Code *code = &out.variants.zSomezIRCodezK;
  code->bytes.off = off;
  code->bytes.len = len;
  code->jumpdests = jumpdest_ref;
  return out;
}

struct AddressResult
code_db_read_delegation(Hash32 hash) {
  struct AddressResult result = {0};
  result.success =
      code_db_read_delegation_address(&result.address, hash);
  return result;
}

uint64_t code_db_analyze_indexed(struct CodeRegionSliceFields code,
                                 bool amsterdam_or_later) {
  const uint64_t len = code.len;
  const uint8_t *bytes = code_ptr(code.off, len);
  const uint64_t jumpdest_ref =
      code_db_analyze_bytes(bytes, len, amsterdam_or_later);
  if (len != 0 && jumpdest_ref == 0) GUEST_ABORT();
  return jumpdest_ref;
}
