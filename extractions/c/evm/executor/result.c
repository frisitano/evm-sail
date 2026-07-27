/* Generated from sail/executor/result.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_382(void) {

  uint64_t z3zE408;
  z3zE408 = UINT64_C(5);
  zRESULT_METADATA_LENGTH = z3zE408;
let_end_1104: ;
}
void kill_letbind_382(void) {
}

unit zresult_prefix(sail_fixed_bytes_32 zroot, bool zsuccess)
{
  unit z8zE694;
  zz5listz8z5bv8z9 z2zE9;
  CREATE(zz5listz8z5bv8z9)(&z2zE9);
  zhash_to_bytes32(&z2zE9, zroot);
  unit z3zE436;
  z3zE436 = zscratch_push_bytes(z2zE9, zWORD_BYTE_LENGTH);
  KILL(zz5listz8z5bv8z9)(&z2zE9);
  zz5listz8z5bv8z9 z2zE11;
  CREATE(zz5listz8z5bv8z9)(&z2zE11);
  {
    uint64_t z2zE10;
    if (zsuccess) {  z2zE10 = UINT64_C(0x01);  } else {  z2zE10 = UINT64_C(0x00);  }
    zconsz3z5bv8(&z2zE11, UINT64_C(0x00), z2zE11);
    zconsz3z5bv8(&z2zE11, UINT64_C(0x00), z2zE11);
    zconsz3z5bv8(&z2zE11, UINT64_C(0x00), z2zE11);
    zconsz3z5bv8(&z2zE11, UINT64_C(0x25), z2zE11);
    zconsz3z5bv8(&z2zE11, z2zE10, z2zE11);
  }
  z8zE694 = zscratch_push_bytes(z2zE11, zRESULT_METADATA_LENGTH);
  KILL(zz5listz8z5bv8z9)(&z2zE11);
end_function_1130: ;
  return z8zE694;
end_block_exception_1131: ;

  return UNIT;
}

unit zcommit_validation_result(sail_fixed_bytes_32 zroot, bool zsuccess, struct zByteSliceFields zchain_config)
{
  unit z8zE695;
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  unit z3zE435;
  z3zE435 = zresult_prefix(zroot, zsuccess);
  unit z3zE434;
  z3zE434 = zscratch_push_slice(zchain_config);
  bool z2zE8;
  {
    struct zByteSliceFields z2zE7;
    z2zE7 = zscratch_finish(zstart);
    z2zE8 = public_output_write(z2zE7);
  }
  z8zE695 = sail_assert(z2zE8, "public output write");
end_function_1128: ;
  return z8zE695;
end_block_exception_1129: ;

  return UNIT;
}

unit zwrite_validation_result(struct zStatelessInputRef zinput_ref, bool zsuccess)
{
  unit z8zE696;
  unit z3zE430;
  z3zE430 = zcycle_scope_start(zSCOPE_COMPUTE_OUTPUT_ROOT);
  sail_fixed_bytes_32 zroot;
  zroot = zhtr_new_payload_request(zinput_ref);
  unit z3zE433;
  z3zE433 = zcycle_scope_end(zSCOPE_COMPUTE_OUTPUT_ROOT);
  unit z3zE432;
  z3zE432 = zcycle_scope_start(zSCOPE_SERIALIZE_OUTPUT);
  struct zByteSliceFields z2zE6;
  z2zE6 = zinput_ref.zchain_config;
  unit z3zE431;
  z3zE431 = zcommit_validation_result(zroot, zsuccess, z2zE6);
  z8zE696 = zcycle_scope_end(zSCOPE_SERIALIZE_OUTPUT);
end_function_1126: ;
  return z8zE696;
end_block_exception_1127: ;

  return UNIT;
}

unit zwrite_invalid_result(unit z3zE425)
{
  unit z8zE697;
  unit z3zE426;
  z3zE426 = zcycle_scope_start(zSCOPE_SERIALIZE_OUTPUT);
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  zz5listz8z5bv8z9 z3zE427;
  CREATE(zz5listz8z5bv8z9)(&z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x08), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x08), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x04), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x0C), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  zconsz3z5bv8(&z3zE427, UINT64_C(0x00), z3zE427);
  unit z3zE428;
  z3zE428 = zscratch_push_bytes(z3zE427, UINT64_C(24));
  KILL(zz5listz8z5bv8z9)(&z3zE427);
  struct zByteSliceFields zchain_config;
  zchain_config = zscratch_finish(zstart);
  unit z3zE429;
  z3zE429 = zcommit_validation_result(zZERO_HASH, false, zchain_config);
  z8zE697 = zcycle_scope_end(zSCOPE_SERIALIZE_OUTPUT);
end_function_1124: ;
  return z8zE697;
end_block_exception_1125: ;

  return UNIT;
}

