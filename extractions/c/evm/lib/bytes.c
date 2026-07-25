/* Generated from sail/lib/bytes.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void zword_to_bytes32(zz5listz8z5bv8z9 *z8zE96, sail_u256 zvalue)
{
  sail_u256 zremaining;
  zremaining = zvalue;
  zz5listz8z5bv8z9 zout;
  CREATE(zz5listz8z5bv8z9)(&zout);
  {
  }
  int64_t z3zE3672;
  {    z3zE3672 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE3673;
  {    z3zE3673 = (int64_t)(UINT64_C(31));
  }
  int64_t z3zE3674;
  {    z3zE3674 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zbyte_index;
    zbyte_index = z3zE3672;
    unit z3zE3676;
  for_start_3767: ;
    {
      if ((z3zE3673 < zbyte_index)) goto for_end_3768;
      uint64_t z2zE3898;
      z2zE3898 = zword_low_byte(zremaining);
      zconsz3z5bv8(&zout, z2zE3898, zout);
      unit z3zE3675;
      z3zE3675 = UNIT;
      zremaining = zword_shift_right(zremaining, UINT64_C(8));
      z3zE3676 = UNIT;
      zbyte_index = (zbyte_index + z3zE3674);
      goto for_start_3767;
    }
  for_end_3768: ;
  }
  unit z3zE3677;
  z3zE3677 = UNIT;
  COPY(zz5listz8z5bv8z9)((*(&z8zE96)), zout);
  KILL(zz5listz8z5bv8z9)(&zout);
end_function_3769: ;
  goto end_function_4076;
end_block_exception_3770: ;
  goto end_function_4076;
end_function_4076: ;
}

void zhash_to_bytes32(zz5listz8z5bv8z9 *z8zE97, sail_fixed_bytes_32 zbytes)
{
  zz5listz8z5bv8z9 zout;
  CREATE(zz5listz8z5bv8z9)(&zout);
  {
  }
  int64_t z3zE3666;
  {    z3zE3666 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE3667;
  {    z3zE3667 = (int64_t)(UINT64_C(31));
  }
  int64_t z3zE3668;
  {    z3zE3668 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zk;
    zk = z3zE3666;
    unit z3zE3669;
  for_start_3763: ;
    {
      if ((z3zE3667 < zk)) goto for_end_3764;
      uint64_t z2zE3897;
      z2zE3897 = fast_vector_access_fixed_bytes_32(zbytes, zk);
      zconsz3z5bv8(&zout, z2zE3897, zout);
      z3zE3669 = UNIT;
      zk = (zk + z3zE3668);
      goto for_start_3763;
    }
  for_end_3764: ;
  }
  unit z3zE3670;
  z3zE3670 = UNIT;
  COPY(zz5listz8z5bv8z9)((*(&z8zE97)), zout);
  KILL(zz5listz8z5bv8z9)(&zout);
end_function_3765: ;
  goto end_function_4075;
end_block_exception_3766: ;
  goto end_function_4075;
end_function_4075: ;
}

void zaddress_to_bytes(zz5listz8z5bv8z9 *z8zE98, sail_fixed_bytes_20 zbytes)
{
  zz5listz8z5bv8z9 zout;
  CREATE(zz5listz8z5bv8z9)(&zout);
  {
  }
  int64_t z3zE3660;
  {    z3zE3660 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE3661;
  {    z3zE3661 = (int64_t)(UINT64_C(19));
  }
  int64_t z3zE3662;
  {    z3zE3662 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zk;
    zk = z3zE3660;
    unit z3zE3663;
  for_start_3759: ;
    {
      if ((z3zE3661 < zk)) goto for_end_3760;
      uint64_t z2zE3896;
      z2zE3896 = fast_vector_access_fixed_bytes_20(zbytes, zk);
      zconsz3z5bv8(&zout, z2zE3896, zout);
      z3zE3663 = UNIT;
      zk = (zk + z3zE3662);
      goto for_start_3759;
    }
  for_end_3760: ;
  }
  unit z3zE3664;
  z3zE3664 = UNIT;
  COPY(zz5listz8z5bv8z9)((*(&z8zE98)), zout);
  KILL(zz5listz8z5bv8z9)(&zout);
end_function_3761: ;
  goto end_function_4074;
end_block_exception_3762: ;
  goto end_function_4074;
end_function_4074: ;
}

