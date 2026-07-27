/* Generated from sail/primitives/code.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_75(void) {

  sail_u256 z3zE78;
  z3zE78 = ((sail_u256){{UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000)}});
  zEMPTY_JUMPDEST_CHUNK = z3zE78;
let_end_171: ;
}
void kill_letbind_75(void) {
}

void create_letbind_76(void) {

  uint64_t z3zE79;
  z3zE79 = UINT64_C(0x0000000000000000);
  zEMPTY_JUMPDEST_REF = z3zE79;
let_end_172: ;
}
void kill_letbind_76(void) {
}

struct zByteSliceFields zcode_slice(struct zByteSliceFields zbytes)
{
  struct zByteSliceFields z8zE93;
  z8zE93 = zbytes;
end_function_3361: ;
  return z8zE93;
end_block_exception_3362: ;
  struct zByteSliceFields z8zE775 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE775;
}

struct zByteSliceFields zvalidated_code_slice(struct zByteSliceFields zbytes)
{
  struct zByteSliceFields z8zE94;
  uint64_t zlength;
  zlength = zbytes.zlen;
  bool z2zE3194;
  {
    sail_u128 z2zE3193;
    z2zE3193 = u128_add_u64_u64(zlength, UINT64_C(32));
    z2zE3194 = (!u64_lt_u128(UINT64_C(18446744073709551615), z2zE3193));
  }
  if (z2zE3194) {  z8zE94 = zcode_slice(zbytes);  } else {
    unit z3zE2900;
    z3zE2900 = sail_assert(false, "executable code cursor headroom");
    sail_match_failure("validated_code_slice");
    /* unreachable after exit */
  }
end_function_3359: ;
  return z8zE94;
end_block_exception_3360: ;
  struct zByteSliceFields z8zE776 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE776;
}

void create_letbind_77(void) {

  struct zByteSliceFields z3zE80;
  struct zByteSliceFields z2zE0;
  z2zE0 = zbyte_slice(zStatelessInputSource, UINT64_C(0), UINT64_C(0));
  z3zE80 = zcode_slice(z2zE0);
  zEMPTY_CODE_SLICE = z3zE80;
let_end_175: ;
}
void kill_letbind_77(void) {
}

bool zdeep_stack_immediate_valid(uint64_t zimmediate)
{
  bool z8zE95;
  uint64_t zvalue;
  zvalue = ((uint64_t) zimmediate);
  bool z2zE3192;
  z2zE3192 = (!(UINT64_C(90) < zvalue));
  bool z3zE2899;
  if (z2zE3192) {  z3zE2899 = true;  } else {  z3zE2899 = (!(zvalue < UINT64_C(128)));  }
  z8zE95 = z3zE2899;
end_function_3357: ;
  return z8zE95;
end_block_exception_3358: ;

  return false;
}

bool zexchange_immediate_valid(uint64_t zimmediate)
{
  bool z8zE96;
  uint64_t zvalue;
  zvalue = ((uint64_t) zimmediate);
  bool z2zE3191;
  z2zE3191 = (!(UINT64_C(81) < zvalue));
  bool z3zE2898;
  if (z2zE3191) {  z3zE2898 = true;  } else {  z3zE2898 = (!(zvalue < UINT64_C(128)));  }
  z8zE96 = z3zE2898;
end_function_3355: ;
  return z8zE96;
end_block_exception_3356: ;

  return false;
}

void create_letbind_78(void) {

  struct zCode z3zE82;
  struct zCode z3zE81;
  z3zE81.zbytes = zEMPTY_CODE_SLICE;
  z3zE81.zjumpdests = zEMPTY_JUMPDEST_REF;
  z3zE82 = z3zE81;
  zEMPTY_CODE = z3zE82;
let_end_178: ;
}
void kill_letbind_78(void) {
}

