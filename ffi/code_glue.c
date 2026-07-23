/* Code lookup aggregate glue. option(Code) is generated, so this file compiles
 * per build against the generated model header rather than mirroring its
 * layout. JUMPDEST chunks cross directly as fixed-width values. */
#include EVMSAIL_MODEL_H
#include "code_db.h"
#include <stdint.h>

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
  code->zbytes.zsource = zCodeSource;
  evmsail_byte_quantity_set(&code->zbytes.zoff, off);
  evmsail_byte_quantity_set(&code->zbytes.zlen, len);
  code->zjumpdests = jumpdest_ref;
}
