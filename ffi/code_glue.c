/* Code lookup aggregate glue. option(Code) is generated, so this file compiles
 * per build against the generated model header rather than mirroring its
 * layout. JUMPDEST chunks cross directly as fixed-width values. */
#include EVMSAIL_MODEL_H
#include "region_access.h"
#include "code_db.h"
#include <stdint.h>

static void code_region_value(struct zCodeRegionSliceFields *out, uint64_t off,
                              uint64_t len) {
#ifdef EVMSAIL_STANDARD_ABI
  CREATE(sail_int)(&out->zoff);
  CREATE(sail_int)(&out->zlen);
#endif
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

static void code_region_borrow_input(struct zCodeRegionSliceFields *out,
                                     const uint8_t *bytes, uint64_t len) {
#if defined(EVMSAIL_POINTER_ABI) || defined(EVMSAIL_CAPACITY_MEASURE)
  code_region_value(out, len ? (uint64_t)(uintptr_t)bytes : 0, len);
#else
  code_region_from_bytes(out, bytes, len);
#endif
}

#ifdef EVMSAIL_STANDARD_ABI
void code_region_from_input(struct zCodeRegionSliceFields *out,
                            struct zStatelessInputSliceFields input) {
  const uint64_t off = evmsail_byte_quantity_value(input.zoff);
  const uint64_t len = evmsail_byte_quantity_value(input.zlen);
  code_region_borrow_input(out, evmsail_stateless_input_ptr(off, len), len);
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
#else
#define DEFINE_CODE_REGION_FROM(name, slice_type, resolver, constructor)     \
  struct zCodeRegionSliceFields name(struct slice_type input) {              \
    struct zCodeRegionSliceFields out;                                       \
    const uint64_t off = evmsail_byte_quantity_value(input.zoff);            \
    const uint64_t len = evmsail_byte_quantity_value(input.zlen);            \
    constructor(&out, resolver(off, len), len);                               \
    return out;                                                               \
  }

DEFINE_CODE_REGION_FROM(code_region_from_input, zStatelessInputSliceFields,
                        evmsail_stateless_input_ptr, code_region_borrow_input)
DEFINE_CODE_REGION_FROM(code_region_from_memory, zMemorySliceFields,
                        evmsail_memory_ptr, code_region_from_bytes)
DEFINE_CODE_REGION_FROM(code_region_from_output, zOutputSliceFields,
                        evmsail_output_ptr, code_region_from_bytes)

#undef DEFINE_CODE_REGION_FROM
#endif

void code_db_lookup(struct zoptionzIRCodezK *out, sail_hash h) {
  uint64_t off = 0, len = 0, jumpdest_ref = 0;
  if (!code_db_lookup_indexed(h, &off, &len, &jumpdest_ref)) {
    out->kind = Kind_zNonezIRCodezK;
    out->variants.zNonezIRCodezK = UNIT;
    return;
  }
  out->kind = Kind_zSomezIRCodezK;
  struct zCode *code = &out->variants.zSomezIRCodezK;
#ifdef EVMSAIL_STANDARD_ABI
  /* The generated option starts in None, so selecting Some must construct the
     GMP-backed fields of Code before they are assigned. */
  CREATE(sail_int)(&code->zbytes.zoff);
  CREATE(sail_int)(&code->zbytes.zlen);
#endif
  evmsail_byte_quantity_set(&code->zbytes.zoff, off);
  evmsail_byte_quantity_set(&code->zbytes.zlen, len);
  code->zjumpdests = jumpdest_ref;
}
