/* Generated from sail/primitives/quantities.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void zword_to_account_nonce(struct zoptionzIU64zK *z8zE74, sail_u256 zvalue)
{
  bool z2zE3210;
  z2zE3210 = (!u64_lt_u256(UINT64_C(18446744073709551615), zvalue));
  if (z2zE3210) {
    {
      struct zoptionzIR__sail_c_repr_u256zK z3zE3010;
      CREATE(zoptionzIR__sail_c_repr_u256zK)(&z3zE3010);
      zSomezIR__sail_c_repr_u256zK(&z3zE3010, zvalue);
    switch (z3zE3010.kind) {
    case Kind_zNonezIR__sail_c_repr_u256zK: {
      unit z8zE769;
        z8zE769 = z3zE3010.variants.zNonezIR__sail_c_repr_u256zK;
      zNonezIU64zK((*(&z8zE74)), z8zE769);

      break;
    }
    case Kind_zSomezIR__sail_c_repr_u256zK: {
      uint64_t z8zE770;
        z8zE770 = u256_to_u64(z3zE3010.variants.zSomezIR__sail_c_repr_u256zK);
      zSomezIU64zK((*(&z8zE74)), z8zE770);

      break;
    }
    }
      KILL(zoptionzIR__sail_c_repr_u256zK)(&z3zE3010);
    }
  } else {  zNonezIU64zK((*(&z8zE74)), UNIT);  }
end_function_3401: ;
  goto end_function_3661;
end_block_exception_3402: ;
  goto end_function_3661;
end_function_3661: ;
}

uint64_t zword_of_account_nonce(uint64_t zvalue)
{
  uint64_t z8zE75;
  z8zE75 = zvalue;
end_function_3399: ;
  return z8zE75;
end_block_exception_3400: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_of_withdrawal_amount(uint64_t zvalue)
{
  uint64_t z8zE76;
  z8zE76 = zvalue;
end_function_3397: ;
  return z8zE76;
end_block_exception_3398: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_of_slot_number(uint64_t zvalue)
{
  uint64_t z8zE77;
  z8zE77 = zvalue;
end_function_3395: ;
  return z8zE77;
end_block_exception_3396: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_of_block_number(uint64_t zvalue)
{
  uint64_t z8zE78;
  z8zE78 = zvalue;
end_function_3393: ;
  return z8zE78;
end_block_exception_3394: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_of_block_timestamp(uint64_t zvalue)
{
  uint64_t z8zE79;
  z8zE79 = zvalue;
end_function_3391: ;
  return z8zE79;
end_block_exception_3392: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_of_chain_identifier(uint64_t zvalue)
{
  uint64_t z8zE80;
  z8zE80 = zvalue;
end_function_3389: ;
  return z8zE80;
end_block_exception_3390: ;

  return UINT64_C(0xdeadc0de);
}

sail_fixed_bytes_20 zprecompile_id_to_address(uint64_t zvalue)
{
  sail_fixed_bytes_20 z8zE81;
  sail_fixed_bytes_20 zresult;
  zresult = zZERO_ADDRESS;
  uint64_t z2zE3208;
  z2zE3208 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(zvalue, UINT64_C(0)));
  zresult = fast_unsigned_vector_update_fixed_bytes_20(zresult, UINT64_C(19), z2zE3208);
  unit z3zE2910;
  z3zE2910 = UNIT;
  uint64_t z2zE3209;
  z2zE3209 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(zvalue, UINT64_C(8)));
  zresult = fast_unsigned_vector_update_fixed_bytes_20(zresult, UINT64_C(18), z2zE3209);
  unit z3zE2909;
  z3zE2909 = UNIT;
  z8zE81 = zresult;
end_function_3387: ;
  return z8zE81;
end_block_exception_3388: ;

  return fixed_bytes_20_zero();
}

uint64_t zaccount_nonce_increment(uint64_t zvalue)
{
  uint64_t z8zE82;
  bool z2zE3207;
  z2zE3207 = (zvalue < UINT64_C(18446744073709551615));
  unit z3zE2908;
  z3zE2908 = sail_assert(z2zE3207, "sail/primitives/quantities.sail:229.46-229.47");
  {    z8zE82 = (zvalue + UINT64_C(1));
  }
end_function_3385: ;
  return z8zE82;
end_block_exception_3386: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zframe_depth_increment(uint64_t zvalue)
{
  uint64_t z8zE83;
  bool z2zE3206;
  z2zE3206 = (zvalue < UINT64_C(1024));
  if (z2zE3206) {
    {    z8zE83 = (zvalue + UINT64_C(1));
    }
  } else {
    unit z3zE2907;
    z3zE2907 = sail_assert(false, "sail/primitives/quantities.sail:238.20-238.21");
    sail_match_failure("frame_depth_increment");
    /* unreachable after exit */
  }
end_function_3383: ;
  return z8zE83;
end_block_exception_3384: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t ztransaction_blob_count_decrement(uint64_t zvalue)
{
  uint64_t z8zE84;
  bool z2zE3205;
  z2zE3205 = (zvalue != UINT64_C(0));
  if (z2zE3205) {
    {    z8zE84 = (zvalue - UINT64_C(1));
    }
  } else {  z8zE84 = UINT64_C(0);  }
end_function_3381: ;
  return z8zE84;
end_block_exception_3382: ;

  return UINT64_C(0xdeadc0de);
}

struct zMemoryRangeFields zmemory_range(uint64_t zoff, uint64_t zlen)
{
  struct zMemoryRangeFields z8zE85;
  struct zMemoryRangeFields z3zE2906;
  z3zE2906.zlen = zlen;
  z3zE2906.zoff = zoff;
  z8zE85 = z3zE2906;
end_function_3379: ;
  return z8zE85;
end_block_exception_3380: ;
  struct zMemoryRangeFields z8zE771 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  return z8zE771;
}

void create_letbind_59(void) {

  struct zMemoryRangeFields z3zE62;
  struct zMemoryRangeFields z3zE61;
  z3zE61.zlen = UINT64_C(0);
  z3zE61.zoff = UINT64_C(0);
  z3zE62 = z3zE61;
  zEMPTY_MEMORY_RANGE = z3zE62;
let_end_147: ;
}
void kill_letbind_59(void) {
}

sail_u256 zword_of_source_byte_count(uint64_t zvalue)
{
  sail_u256 z8zE86;
  z8zE86 = zword_of_nat_byte_countzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zvalue);
end_function_3375: ;
  return z8zE86;
end_block_exception_3376: ;

  return u256_zero();
}

