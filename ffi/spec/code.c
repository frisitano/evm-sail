/* Generated CodeFields adapters for the specification ABI. */
#include EVMSAIL_MODEL_H
#include "code_db.h"
#include "region_access.h"
#include "value_convert.h"

#include <stdint.h>

static struct zCodeRegionSliceFields code_region_value(uint64_t off,
                                                       uint64_t len) {
  struct zCodeRegionSliceFields out;
  out.zbytes = off;
  out.zlen = len;
  return out;
}

static struct zCodeRegionSliceFields code_region_from_bytes(
    const uint8_t *bytes, uint64_t len) {
  uint64_t off = 0;
  if (!bytes || !code_db_append_region(bytes, len, &off)) {
    off = 0;
    len = 0;
  }
  return code_region_value(off, len);
}

struct zCodeRegionSliceFields code_region_from_input(
    struct zStatelessInputSliceFields input) {
  const uint64_t off = input.zbytes;
  const uint64_t len = input.zlen;
  return code_region_from_bytes(evmsail_stateless_input_ptr(off, len), len);
}

struct zCodeRegionSliceFields code_region_from_memory(
    struct zEvmMemorySliceFields input) {
  const uint64_t off = input.zbytes;
  const uint64_t len = input.zlen;
  return code_region_from_bytes(evmsail_memory_ptr(off, len), len);
}

struct zCodeRegionSliceFields code_region_from_output(
    struct zOutputSliceFields input) {
  const uint64_t off = input.zbytes;
  const uint64_t len = input.zlen;
  return code_region_from_bytes(evmsail_output_ptr(off, len), len);
}

struct zCodeRegionSliceFields code_region_from_delegation(
    sail_fixed_bytes_20 address) {
  uint8_t bytes[23] = {0xef, 0x01, 0x00};
  evmsail_address_to_be_bytes(bytes + 3, address);
  return code_region_from_bytes(bytes, sizeof(bytes));
}

struct zCodeFields code_db_lookup(sail_fixed_bytes_32 hash) {
  struct zCodeFields out = {0};
  uint64_t off = 0, len = 0, jumpdest_ref = 0;
  if (!code_db_lookup_indexed(hash, &off, &len, &jumpdest_ref)) {
    return out;
  }
  out.zbytes = off;
  out.zlen = len;
  out.zjumpdests = jumpdest_ref;
  return out;
}
