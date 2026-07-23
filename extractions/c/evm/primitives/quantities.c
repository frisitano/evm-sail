/* Generated from sail/primitives/quantities.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void zword_to_account_nonce(struct zoptionzIU64zK *z8zE73, sail_u256 zvalue)
{
  bool z2zE3957;
  z2zE3957 = (!u64_lt_u256(UINT64_C(18446744073709551615), zvalue));
  if (z2zE3957) {
    {
      struct zoptionzIR__sail_c_repr_u256zK z3zE3849;
      CREATE(zoptionzIR__sail_c_repr_u256zK)(&z3zE3849);
      zSomezIR__sail_c_repr_u256zK(&z3zE3849, zvalue);
    switch (z3zE3849.kind) {
    case Kind_zNonezIR__sail_c_repr_u256zK: {
      unit z8zE808;
        z8zE808 = z3zE3849.variants.zNonezIR__sail_c_repr_u256zK;
      zNonezIU64zK((*(&z8zE73)), z8zE808);

      break;
    }
    case Kind_zSomezIR__sail_c_repr_u256zK: {
      uint64_t z8zE809;
        z8zE809 = u256_to_u64(z3zE3849.variants.zSomezIR__sail_c_repr_u256zK);
      zSomezIU64zK((*(&z8zE73)), z8zE809);

      break;
    }
    }
      KILL(zoptionzIR__sail_c_repr_u256zK)(&z3zE3849);
    }
  } else {  zNonezIU64zK((*(&z8zE73)), UNIT);  }
end_function_3836: ;
  goto end_function_4092;
end_block_exception_3837: ;
  goto end_function_4092;
end_function_4092: ;
}

uint64_t zword_of_account_nonce(uint64_t zvalue)
{
  uint64_t z8zE74;
  z8zE74 = zvalue;
end_function_3834: ;
  return z8zE74;
end_block_exception_3835: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_of_withdrawal_amount(uint64_t zvalue)
{
  uint64_t z8zE75;
  z8zE75 = zvalue;
end_function_3832: ;
  return z8zE75;
end_block_exception_3833: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_of_slot_number(uint64_t zvalue)
{
  uint64_t z8zE76;
  z8zE76 = zvalue;
end_function_3830: ;
  return z8zE76;
end_block_exception_3831: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_of_block_number(uint64_t zvalue)
{
  uint64_t z8zE77;
  z8zE77 = zvalue;
end_function_3828: ;
  return z8zE77;
end_block_exception_3829: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_of_block_timestamp(uint64_t zvalue)
{
  uint64_t z8zE78;
  z8zE78 = zvalue;
end_function_3826: ;
  return z8zE78;
end_block_exception_3827: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_of_chain_identifier(uint64_t zvalue)
{
  uint64_t z8zE79;
  z8zE79 = zvalue;
end_function_3824: ;
  return z8zE79;
end_block_exception_3825: ;

  return UINT64_C(0xdeadc0de);
}

sail_fixed_bytes_20 zprecompile_id_to_address(uint64_t zvalue)
{
  sail_fixed_bytes_20 z8zE80;
  sail_fixed_bytes_20 zresult;
  zresult = zZERO_ADDRESS;
  uint64_t z2zE3955;
  z2zE3955 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(zvalue, UINT64_C(0)));
  zresult = fast_unsigned_vector_update_fixed_bytes_20(zresult, UINT64_C(0), z2zE3955);
  unit z3zE3761;
  z3zE3761 = UNIT;
  uint64_t z2zE3956;
  z2zE3956 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(zvalue, UINT64_C(8)));
  zresult = fast_unsigned_vector_update_fixed_bytes_20(zresult, UINT64_C(1), z2zE3956);
  unit z3zE3760;
  z3zE3760 = UNIT;
  z8zE80 = zresult;
end_function_3822: ;
  return z8zE80;
end_block_exception_3823: ;

  return fixed_bytes_20_zero();
}

uint64_t zaccount_nonce_increment(uint64_t zvalue)
{
  uint64_t z8zE81;
  bool z2zE3954;
  z2zE3954 = (zvalue < UINT64_C(18446744073709551615));
  unit z3zE3759;
  z3zE3759 = sail_assert(z2zE3954, "sail/primitives/quantities.sail:241.46-241.47");
  {    z8zE81 = (zvalue + UINT64_C(1));
  }
end_function_3820: ;
  return z8zE81;
end_block_exception_3821: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zframe_depth_increment(uint64_t zvalue)
{
  uint64_t z8zE82;
  bool z2zE3953;
  z2zE3953 = (zvalue < UINT64_C(1024));
  if (z2zE3953) {
    {    z8zE82 = (zvalue + UINT64_C(1));
    }
  } else {
    unit z3zE3758;
    z3zE3758 = sail_assert(false, "sail/primitives/quantities.sail:250.20-250.21");
    sail_match_failure("frame_depth_increment");
    /* unreachable after exit */
  }
end_function_3818: ;
  return z8zE82;
end_block_exception_3819: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t ztransaction_blob_count_decrement(uint64_t zvalue)
{
  uint64_t z8zE83;
  bool z2zE3952;
  z2zE3952 = (zvalue != UINT64_C(0));
  if (z2zE3952) {
    {    z8zE83 = (zvalue - UINT64_C(1));
    }
  } else {  z8zE83 = UINT64_C(0);  }
end_function_3816: ;
  return z8zE83;
end_block_exception_3817: ;

  return UINT64_C(0xdeadc0de);
}

struct zMemoryRangeFields zmemory_range(uint64_t zoff, uint64_t zlen)
{
  struct zMemoryRangeFields z8zE84;
  struct zMemoryRangeFields z3zE3757;
  z3zE3757.zlen = zlen;
  z3zE3757.zoff = zoff;
  z8zE84 = z3zE3757;
end_function_3814: ;
  return z8zE84;
end_block_exception_3815: ;
  struct zMemoryRangeFields z8zE810 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  return z8zE810;
}

void create_letbind_35(void) {

  struct zMemoryRangeFields z3zE38;
  struct zMemoryRangeFields z3zE37;
  z3zE37.zlen = UINT64_C(0);
  z3zE37.zoff = UINT64_C(0);
  z3zE38 = z3zE37;
  zEMPTY_MEMORY_RANGE = z3zE38;
let_end_122: ;
}
void kill_letbind_35(void) {
}

sail_u256 zword_of_source_byte_count(uint64_t zvalue)
{
  sail_u256 z8zE85;
  z8zE85 = zword_of_nat_byte_countzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zvalue);
end_function_3810: ;
  return z8zE85;
end_block_exception_3811: ;

  return u256_zero();
}

