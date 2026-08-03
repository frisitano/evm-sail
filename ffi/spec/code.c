/* Generated option(Code) adapters for the specification ABI. */
#include EVMSAIL_MODEL_H
#include "code_db.h"
#include "region_access.h"

#include <stdint.h>

static struct zCodeRegionSliceFields code_region_value(uint64_t off,
                                                       uint64_t len) {
  struct zCodeRegionSliceFields out;
  out.zoff = off;
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
  const uint64_t off = input.zoff;
  const uint64_t len = input.zlen;
  return code_region_from_bytes(evmsail_stateless_input_ptr(off, len), len);
}

struct zCodeRegionSliceFields code_region_from_memory(
    struct zEvmMemorySliceFields input) {
  const uint64_t off = input.zoff;
  const uint64_t len = input.zlen;
  return code_region_from_bytes(evmsail_memory_ptr(off, len), len);
}

struct zCodeRegionSliceFields code_region_from_output(
    struct zOutputSliceFields input) {
  const uint64_t off = input.zoff;
  const uint64_t len = input.zlen;
  return code_region_from_bytes(evmsail_output_ptr(off, len), len);
}

void code_db_lookup(struct zoptionzIRCodezK *out, sail_fixed_bytes_32 hash) {
  uint64_t off = 0, len = 0, jumpdest_ref = 0;
  if (!code_db_lookup_indexed(hash, &off, &len, &jumpdest_ref)) {
    out->kind = Kind_zNonezIRCodezK;
    out->variants.zNonezIRCodezK = UNIT;
    return;
  }
  out->kind = Kind_zSomezIRCodezK;
  struct zCode *code = &out->variants.zSomezIRCodezK;
  code->zbytes.zoff = off;
  code->zbytes.zlen = len;
  code->zjumpdests = jumpdest_ref;
}
