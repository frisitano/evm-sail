/* Generated option(Code) adapters for the specification ABI. */
#include EVMSAIL_MODEL_H
#include "code_db.h"
#include "region_access.h"

#include <stdint.h>

static void code_region_value(struct zCodeRegionSliceFields *out, uint64_t off,
                              uint64_t len) {
  CREATE(sail_int)(&out->zoff);
  CREATE(sail_int)(&out->zlen);
  evmsail_byte_quantity_set(&out->zoff, off);
  evmsail_byte_quantity_set(&out->zlen, len);
}

static void code_region_from_bytes(struct zCodeRegionSliceFields *out,
                                   const uint8_t *bytes, uint64_t len) {
  uint64_t off = 0;
  if (!bytes || !code_db_append_region(bytes, len, &off)) {
    off = 0;
    len = 0;
  }
  code_region_value(out, off, len);
}

void code_region_from_input(struct zCodeRegionSliceFields *out,
                            struct zStatelessInputSliceFields input) {
  const uint64_t off = evmsail_byte_quantity_value(input.zoff);
  const uint64_t len = evmsail_byte_quantity_value(input.zlen);
  code_region_from_bytes(out, evmsail_stateless_input_ptr(off, len), len);
}

void code_region_from_memory(struct zCodeRegionSliceFields *out,
                             struct zMemorySliceFields input) {
  const uint64_t off = evmsail_byte_quantity_value(input.zoff);
  const uint64_t len = evmsail_byte_quantity_value(input.zlen);
  code_region_from_bytes(out, evmsail_memory_ptr(off, len), len);
}

void code_region_from_output(struct zCodeRegionSliceFields *out,
                             struct zOutputSliceFields input) {
  const uint64_t off = evmsail_byte_quantity_value(input.zoff);
  const uint64_t len = evmsail_byte_quantity_value(input.zlen);
  code_region_from_bytes(out, evmsail_output_ptr(off, len), len);
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
  CREATE(sail_int)(&code->zbytes.zoff);
  CREATE(sail_int)(&code->zbytes.zlen);
  evmsail_byte_quantity_set(&code->zbytes.zoff, off);
  evmsail_byte_quantity_set(&code->zbytes.zlen, len);
  code->zjumpdests = jumpdest_ref;
}
