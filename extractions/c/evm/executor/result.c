/* Generated from sail/executor/result.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_361(void) {

  uint64_t z3zE444;
  z3zE444 = UINT64_C(5);
  zRESULT_METADATA_LENGTH = z3zE444;
let_end_1115: ;
}
void kill_letbind_361(void) {
}

unit zresult_prefix(sail_fixed_bytes_32 zroot, bool zsuccess)
{
  unit z8zE716;
  zz5listz8z5bv8z9 z2zE10;
  CREATE(zz5listz8z5bv8z9)(&z2zE10);
  zhash_to_bytes32(&z2zE10, zroot);
  unit z3zE472;
  z3zE472 = zscratch_push_bytes(z2zE10, zWORD_BYTE_LENGTH);
  KILL(zz5listz8z5bv8z9)(&z2zE10);
  zz5listz8z5bv8z9 z2zE12;
  CREATE(zz5listz8z5bv8z9)(&z2zE12);
  {
    uint64_t z2zE11;
    if (zsuccess) {  z2zE11 = UINT64_C(0x01);  } else {  z2zE11 = UINT64_C(0x00);  }
    zconsz3z5bv8(&z2zE12, UINT64_C(0x00), z2zE12);
    zconsz3z5bv8(&z2zE12, UINT64_C(0x00), z2zE12);
    zconsz3z5bv8(&z2zE12, UINT64_C(0x00), z2zE12);
    zconsz3z5bv8(&z2zE12, UINT64_C(0x25), z2zE12);
    zconsz3z5bv8(&z2zE12, z2zE11, z2zE12);
  }
  z8zE716 = zscratch_push_bytes(z2zE12, zRESULT_METADATA_LENGTH);
  KILL(zz5listz8z5bv8z9)(&z2zE12);
end_function_1141: ;
  return z8zE716;
end_block_exception_1142: ;

  return UNIT;
}

unit zcommit_validation_result(sail_fixed_bytes_32 zroot, bool zsuccess, struct zByteSliceFields zchain_config)
{
  unit z8zE717;
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  unit z3zE471;
  z3zE471 = zresult_prefix(zroot, zsuccess);
  unit z3zE470;
  z3zE470 = zscratch_push_slice(zchain_config);
  bool z2zE9;
  {
    struct zByteSliceFields z2zE8;
    z2zE8 = zscratch_finish(zstart);
    z2zE9 = public_output_write(z2zE8);
  }
  z8zE717 = sail_assert(z2zE9, "public output write");
end_function_1139: ;
  return z8zE717;
end_block_exception_1140: ;

  return UNIT;
}

unit zwrite_validation_result(struct zStatelessInputRef zinput_ref, bool zsuccess)
{
  unit z8zE718;
  unit z3zE466;
  z3zE466 = zcycle_scope_start(zSCOPE_COMPUTE_OUTPUT_ROOT);
  sail_fixed_bytes_32 zroot;
  {
    zroot = zhtr_new_payload_request(zinput_ref);
    if (have_exception) {  goto end_block_exception_1138;  }
  }
  unit z3zE469;
  z3zE469 = zcycle_scope_end(zSCOPE_COMPUTE_OUTPUT_ROOT);
  unit z3zE468;
  z3zE468 = zcycle_scope_start(zSCOPE_SERIALIZE_OUTPUT);
  struct zByteSliceFields z2zE7;
  z2zE7 = zinput_ref.zchain_config;
  unit z3zE467;
  z3zE467 = zcommit_validation_result(zroot, zsuccess, z2zE7);
  z8zE718 = zcycle_scope_end(zSCOPE_SERIALIZE_OUTPUT);
end_function_1137: ;
  return z8zE718;
end_block_exception_1138: ;

  return UNIT;
}

unit zwrite_invalid_result(unit z3zE461)
{
  unit z8zE719;
  unit z3zE462;
  z3zE462 = zcycle_scope_start(zSCOPE_SERIALIZE_OUTPUT);
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  zz5listz8z5bv8z9 z3zE463;
  CREATE(zz5listz8z5bv8z9)(&z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x08), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x08), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x04), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x0C), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  zconsz3z5bv8(&z3zE463, UINT64_C(0x00), z3zE463);
  unit z3zE464;
  z3zE464 = zscratch_push_bytes(z3zE463, UINT64_C(24));
  KILL(zz5listz8z5bv8z9)(&z3zE463);
  struct zByteSliceFields zchain_config;
  zchain_config = zscratch_finish(zstart);
  unit z3zE465;
  z3zE465 = zcommit_validation_result(zZERO_HASH, false, zchain_config);
  z8zE719 = zcycle_scope_end(zSCOPE_SERIALIZE_OUTPUT);
end_function_1135: ;
  return z8zE719;
end_block_exception_1136: ;

  return UNIT;
}

