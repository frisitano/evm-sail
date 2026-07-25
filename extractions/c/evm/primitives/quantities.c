/* Generated from sail/primitives/quantities.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void zword_to_account_nonce(struct zoptionzIU64zK *z8zE73, sail_u256 zvalue)
{
  bool z2zE3918;
  z2zE3918 = (!u64_lt_u256(UINT64_C(18446744073709551615), zvalue));
  if (z2zE3918) {
    {
      struct zoptionzIR__sail_c_repr_u256zK z3zE3778;
      CREATE(zoptionzIR__sail_c_repr_u256zK)(&z3zE3778);
      zSomezIR__sail_c_repr_u256zK(&z3zE3778, zvalue);
    switch (z3zE3778.kind) {
    case Kind_zNonezIR__sail_c_repr_u256zK: {
      unit z8zE805;
        z8zE805 = z3zE3778.variants.zNonezIR__sail_c_repr_u256zK;
      zNonezIU64zK((*(&z8zE73)), z8zE805);

      break;
    }
    case Kind_zSomezIR__sail_c_repr_u256zK: {
      uint64_t z8zE806;
        z8zE806 = u256_to_u64(z3zE3778.variants.zSomezIR__sail_c_repr_u256zK);
      zSomezIU64zK((*(&z8zE73)), z8zE806);

      break;
    }
    }
      KILL(zoptionzIR__sail_c_repr_u256zK)(&z3zE3778);
    }
  } else {  zNonezIU64zK((*(&z8zE73)), UNIT);  }
end_function_3817: ;
  goto end_function_4080;
end_block_exception_3818: ;
  goto end_function_4080;
end_function_4080: ;
}

uint64_t zword_of_account_nonce(uint64_t zvalue)
{
  uint64_t z8zE74;
  z8zE74 = zvalue;
end_function_3815: ;
  return z8zE74;
end_block_exception_3816: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_of_withdrawal_amount(uint64_t zvalue)
{
  uint64_t z8zE75;
  z8zE75 = zvalue;
end_function_3813: ;
  return z8zE75;
end_block_exception_3814: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_of_slot_number(uint64_t zvalue)
{
  uint64_t z8zE76;
  z8zE76 = zvalue;
end_function_3811: ;
  return z8zE76;
end_block_exception_3812: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_of_block_number(uint64_t zvalue)
{
  uint64_t z8zE77;
  z8zE77 = zvalue;
end_function_3809: ;
  return z8zE77;
end_block_exception_3810: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_of_block_timestamp(uint64_t zvalue)
{
  uint64_t z8zE78;
  z8zE78 = zvalue;
end_function_3807: ;
  return z8zE78;
end_block_exception_3808: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_of_chain_identifier(uint64_t zvalue)
{
  uint64_t z8zE79;
  z8zE79 = zvalue;
end_function_3805: ;
  return z8zE79;
end_block_exception_3806: ;

  return UINT64_C(0xdeadc0de);
}

sail_fixed_bytes_20 zprecompile_id_to_address(uint64_t zvalue)
{
  sail_fixed_bytes_20 z8zE80;
  sail_fixed_bytes_20 zresult;
  zresult = zZERO_ADDRESS;
  uint64_t z2zE3916;
  z2zE3916 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(zvalue, UINT64_C(0)));
  zresult = fast_unsigned_vector_update_fixed_bytes_20(zresult, UINT64_C(0), z2zE3916);
  unit z3zE3690;
  z3zE3690 = UNIT;
  uint64_t z2zE3917;
  z2zE3917 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(zvalue, UINT64_C(8)));
  zresult = fast_unsigned_vector_update_fixed_bytes_20(zresult, UINT64_C(1), z2zE3917);
  unit z3zE3689;
  z3zE3689 = UNIT;
  z8zE80 = zresult;
end_function_3803: ;
  return z8zE80;
end_block_exception_3804: ;

  return fixed_bytes_20_zero();
}

uint64_t zaccount_nonce_increment(uint64_t zvalue)
{
  uint64_t z8zE81;
  bool z2zE3915;
  z2zE3915 = (zvalue < UINT64_C(18446744073709551615));
  unit z3zE3688;
  z3zE3688 = sail_assert(z2zE3915, "sail/primitives/quantities.sail:229.46-229.47");
  {    z8zE81 = (zvalue + UINT64_C(1));
  }
end_function_3801: ;
  return z8zE81;
end_block_exception_3802: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zframe_depth_increment(uint64_t zvalue)
{
  uint64_t z8zE82;
  bool z2zE3914;
  z2zE3914 = (zvalue < UINT64_C(1024));
  if (z2zE3914) {
    {    z8zE82 = (zvalue + UINT64_C(1));
    }
  } else {
    unit z3zE3687;
    z3zE3687 = sail_assert(false, "sail/primitives/quantities.sail:238.20-238.21");
    sail_match_failure("frame_depth_increment");
    /* unreachable after exit */
  }
end_function_3799: ;
  return z8zE82;
end_block_exception_3800: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t ztransaction_blob_count_decrement(uint64_t zvalue)
{
  uint64_t z8zE83;
  bool z2zE3913;
  z2zE3913 = (zvalue != UINT64_C(0));
  if (z2zE3913) {
    {    z8zE83 = (zvalue - UINT64_C(1));
    }
  } else {  z8zE83 = UINT64_C(0);  }
end_function_3797: ;
  return z8zE83;
end_block_exception_3798: ;

  return UINT64_C(0xdeadc0de);
}

struct zMemoryRangeFields zmemory_range(uint64_t zoff, uint64_t zlen)
{
  struct zMemoryRangeFields z8zE84;
  struct zMemoryRangeFields z3zE3686;
  z3zE3686.zlen = zlen;
  z3zE3686.zoff = zoff;
  z8zE84 = z3zE3686;
end_function_3795: ;
  return z8zE84;
end_block_exception_3796: ;
  struct zMemoryRangeFields z8zE807 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  return z8zE807;
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
end_function_3791: ;
  return z8zE85;
end_block_exception_3792: ;

  return u256_zero();
}

