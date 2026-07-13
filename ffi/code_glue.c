/* Code aggregate glue. JumpdestBitmap and option(Code) are
 * GENERATED types, so this file compiles per build against the generated model
 * header rather than mirroring Sail's layouts.
 *
 * Sail performs the complete PUSH-aware scan before either function is
 * called. The glue flattens its immutable bitmap into scalar words for
 * code_db.c and assembles a typed lookup result; opcode analysis never crosses
 * into C. */
#include EVMSAIL_MODEL_H
#include "code_db.h"
#include "lbits_convert.h"
#include <stdint.h>
#include <stdlib.h>

static int unpack_jumpdest_bitmap(uint64_t **out, uint64_t expected,
                                  zz5listz8z5bvz9 bitmap) {
  *out = NULL;
  if (expected == 0) return bitmap == NULL;
  if (expected > SIZE_MAX / sizeof(uint64_t)) return 0;

  uint64_t *words = (uint64_t *)malloc((size_t)expected * sizeof(*words));
  if (!words) return 0;

  zz5listz8z5bvz9 node = bitmap;
  uint64_t i = 0;
  while (node && i < expected) {
    words[i++] = lbits_to_u64(node->hd);
    node = node->tl;
  }
  if (i != expected || node) {
    free(words);
    return 0;
  }
  *out = words;
  return 1;
}

void code_db_lookup(struct zoptionzIRCodezK *out, const lbits h) {
  uint64_t off = 0, len = 0, jumpdest_ref = 0;
  if (!code_db_lookup_indexed(h, &off, &len, &jumpdest_ref)) {
    out->kind = Kind_zNonezIRCodezK;
    out->variants.zNonezIRCodezK = UNIT;
    return;
  }
  out->kind = Kind_zSomezIRCodezK;
  struct zCode *code = &out->variants.zSomezIRCodezK;
  code->zbytes.zsource = zCodeSource;
  code->zbytes.zoff = off;
  code->zbytes.zlen = len;
  code->zjumpdests = jumpdest_ref;
}

void code_db_store_indexed_source(lbits *rop, uint64_t source_kind,
                                  uint64_t off, uint64_t len,
                                  zz5listz8z5bvz9 jumpdests) {
  uint64_t expected = len <= UINT32_MAX ? (len + 63) / 64 : UINT64_MAX;
  uint64_t *words = NULL;
  if (!unpack_jumpdest_bitmap(&words, expected, jumpdests)) {
    code_db_store_indexed_words(rop, source_kind, off, len, NULL, UINT64_MAX);
    return;
  }
  code_db_store_indexed_words(rop, source_kind, off, len, words, expected);
  free(words);
}

void code_intern_indexed_delegation(lbits *rop, const lbits addr,
                                    zz5listz8z5bvz9 jumpdests) {
  uint64_t *words = NULL;
  if (!unpack_jumpdest_bitmap(&words, 1, jumpdests)) {
    code_intern_indexed_delegation_words(rop, addr, NULL, UINT64_MAX);
    return;
  }
  code_intern_indexed_delegation_words(rop, addr, words, 1);
  free(words);
}
