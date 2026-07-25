/* Generated from sail/primitives/code.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_51(void) {

  sail_u256 z3zE54;
  z3zE54 = ((sail_u256){{UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000)}});
  zEMPTY_JUMPDEST_CHUNK = z3zE54;
let_end_146: ;
}
void kill_letbind_51(void) {
}

void create_letbind_52(void) {

  uint64_t z3zE55;
  z3zE55 = UINT64_C(0x0000000000000000);
  zEMPTY_JUMPDEST_REF = z3zE55;
let_end_147: ;
}
void kill_letbind_52(void) {
}

struct zByteSliceFields zcode_slice(struct zByteSliceFields zbytes)
{
  struct zByteSliceFields z8zE92;
  z8zE92 = zbytes;
end_function_3777: ;
  return z8zE92;
end_block_exception_3778: ;
  struct zByteSliceFields z8zE811 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE811;
}

struct zByteSliceFields zvalidated_code_slice(struct zByteSliceFields zbytes)
{
  struct zByteSliceFields z8zE93;
  uint64_t zlength;
  zlength = zbytes.zlen;
  bool z2zE3902;
  {
    sail_u128 z2zE3901;
    z2zE3901 = u128_add_u64_u64(zlength, UINT64_C(32));
    z2zE3902 = (!u64_lt_u128(UINT64_C(18446744073709551615), z2zE3901));
  }
  if (z2zE3902) {  z8zE93 = zcode_slice(zbytes);  } else {
    unit z3zE3680;
    z3zE3680 = sail_assert(false, "executable code cursor headroom");
    sail_match_failure("validated_code_slice");
    /* unreachable after exit */
  }
end_function_3775: ;
  return z8zE93;
end_block_exception_3776: ;
  struct zByteSliceFields z8zE812 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE812;
}

void create_letbind_53(void) {

  struct zByteSliceFields z3zE56;
  struct zByteSliceFields z2zE0;
  z2zE0 = zbyte_slice(zStatelessInputSource, UINT64_C(0), UINT64_C(0));
  z3zE56 = zcode_slice(z2zE0);
  zEMPTY_CODE_SLICE = z3zE56;
let_end_150: ;
}
void kill_letbind_53(void) {
}

bool zdeep_stack_immediate_valid(uint64_t zimmediate)
{
  bool z8zE94;
  uint64_t zvalue;
  zvalue = ((uint64_t) zimmediate);
  bool z2zE3900;
  z2zE3900 = (!(UINT64_C(90) < zvalue));
  bool z3zE3679;
  if (z2zE3900) {  z3zE3679 = true;  } else {  z3zE3679 = (!(zvalue < UINT64_C(128)));  }
  z8zE94 = z3zE3679;
end_function_3773: ;
  return z8zE94;
end_block_exception_3774: ;

  return false;
}

bool zexchange_immediate_valid(uint64_t zimmediate)
{
  bool z8zE95;
  uint64_t zvalue;
  zvalue = ((uint64_t) zimmediate);
  bool z2zE3899;
  z2zE3899 = (!(UINT64_C(81) < zvalue));
  bool z3zE3678;
  if (z2zE3899) {  z3zE3678 = true;  } else {  z3zE3678 = (!(zvalue < UINT64_C(128)));  }
  z8zE95 = z3zE3678;
end_function_3771: ;
  return z8zE95;
end_block_exception_3772: ;

  return false;
}

void create_letbind_54(void) {

  struct zCode z3zE58;
  struct zCode z3zE57;
  z3zE57.zbytes = zEMPTY_CODE_SLICE;
  z3zE57.zjumpdests = zEMPTY_JUMPDEST_REF;
  z3zE58 = z3zE57;
  zEMPTY_CODE = z3zE58;
let_end_153: ;
}
void kill_letbind_54(void) {
}

