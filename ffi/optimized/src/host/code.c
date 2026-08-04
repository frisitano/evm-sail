/* Fixed-layout code-region and lookup adapters for the optimized ABI. */
#include "evmsail/prelude.h"
#include "evmsail/host/code.h"
#include "host/code/store.h"
#include "evmsail/host/region_access.h"
#include "primitives/value.h"
#include "workspace.h"

#include <stdlib.h>
#include <stdint.h>

static struct CodeRegionSliceFields code_region_value(uint8_t *bytes,
                                                       uint64_t len) {
  struct CodeRegionSliceFields out;
  out.bytes = bytes;
  out.len = len;
  return out;
}

/* Unstable source bytes are copied into the arena and registered; workspace
 * exhaustion fails closed. */
static struct CodeRegionSliceFields code_region_from_bytes(
    const uint8_t *bytes, uint64_t len) {
  if (len == 0) return code_region_value(0, 0);
  if (!bytes) GUEST_ABORT();
  uint8_t *stable = code_region_intern_copy(bytes, len);
  if (!stable) GUEST_ABORT();
  return code_region_value(stable, len);
}

/* Input-backed code is referenced in place: the row records the resolved
 * input pointer and the bytes are never copied. A slice that fails
 * re-validation against the live input is structural corruption, not a
 * witness deficiency. */
struct CodeRegionSliceFields code_region_from_input(
    struct StatelessInputSliceFields input) {
  const uint64_t len = input.len;
  if (len == 0) return code_region_value(0, 0);
  const uint8_t *bytes = input.bytes;
  if (!bytes) GUEST_ABORT();
  return code_region_value((uint8_t *)(uintptr_t)bytes, len);
}

struct CodeRegionSliceFields code_region_from_memory(
    struct EvmMemorySliceFields input) {
  const uint64_t len = input.len;
  return code_region_from_bytes(input.bytes, len);
}

struct CodeRegionSliceFields code_region_from_output(
    struct OutputSliceFields input) {
  const uint64_t len = input.len;
  return code_region_from_bytes(input.bytes, len);
}

struct CodeRegionSliceFields code_region_from_delegation(Address address) {
  uint8_t bytes[23] = {0xef, 0x01, 0x00};
  address_to_be_bytes(bytes + 3, address);
  return code_region_from_bytes(bytes, sizeof(bytes));
}

struct zoptionzIRCodeFieldszK code_db_lookup(Hash32 hash) {
  struct zoptionzIRCodeFieldszK out = {0};
  const uint8_t *bytes = NULL;
  uint8_t *jumpdests = NULL;
  uint64_t len = 0;
  if (!code_db_lookup_view(hash, &bytes, &len, &jumpdests)) {
    out.kind = Kind_zNonezIRCodeFieldszK;
    out.variants.zNonezIRCodeFieldszK = UNIT;
    return out;
  }
  out.kind = Kind_zSomezIRCodeFieldszK;
  struct CodeFields *code = &out.variants.zSomezIRCodeFieldszK;
  code->bytes = (uint8_t *)(uintptr_t)bytes;
  code->len = len;
  code->jumpdests = jumpdests;
  return out;
}

struct AddressResult
code_db_read_delegation(Hash32 hash) {
  struct AddressResult result = {0};
  result.success =
      code_db_read_delegation_address(&result.address, hash);
  return result;
}

uint8_t *code_db_analyze_indexed(struct CodeRegionSliceFields code,
                                 bool amsterdam_or_later) {
  uint8_t *jumpdest_ref =
      code_db_analyze_region(code, amsterdam_or_later);
  if (code.len != 0 && !jumpdest_ref) GUEST_ABORT();
  return jumpdest_ref;
}
