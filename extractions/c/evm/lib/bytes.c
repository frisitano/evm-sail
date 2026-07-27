/* Generated from sail/lib/bytes.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void zword_to_bytes32(zz5listz8z5bv8z9 *z8zE97, sail_u256 zvalue)
{
  sail_u256 zremaining;
  zremaining = zvalue;
  zz5listz8z5bv8z9 zout;
  CREATE(zz5listz8z5bv8z9)(&zout);
  {
  }
  int64_t z3zE2892;
  {    z3zE2892 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE2893;
  {    z3zE2893 = (int64_t)(UINT64_C(31));
  }
  int64_t z3zE2894;
  {    z3zE2894 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zbyte_index;
    zbyte_index = z3zE2892;
    unit z3zE2896;
  for_start_3351: ;
    {
      if ((z3zE2893 < zbyte_index)) goto for_end_3352;
      uint64_t z2zE3190;
      z2zE3190 = zword_low_byte(zremaining);
      zconsz3z5bv8(&zout, z2zE3190, zout);
      unit z3zE2895;
      z3zE2895 = UNIT;
      zremaining = zword_shift_right(zremaining, UINT64_C(8));
      z3zE2896 = UNIT;
      zbyte_index = (zbyte_index + z3zE2894);
      goto for_start_3351;
    }
  for_end_3352: ;
  }
  unit z3zE2897;
  z3zE2897 = UNIT;
  COPY(zz5listz8z5bv8z9)((*(&z8zE97)), zout);
  KILL(zz5listz8z5bv8z9)(&zout);
end_function_3353: ;
  goto end_function_3657;
end_block_exception_3354: ;
  goto end_function_3657;
end_function_3657: ;
}

void zhash_to_bytes32(zz5listz8z5bv8z9 *z8zE98, sail_fixed_bytes_32 zbytes)
{
  zz5listz8z5bv8z9 zout;
  CREATE(zz5listz8z5bv8z9)(&zout);
  {
  }
  int64_t z3zE2886;
  {    z3zE2886 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE2887;
  {    z3zE2887 = (int64_t)(UINT64_C(31));
  }
  int64_t z3zE2888;
  {    z3zE2888 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zk;
    zk = z3zE2886;
    unit z3zE2889;
  for_start_3347: ;
    {
      if ((z3zE2887 < zk)) goto for_end_3348;
      uint64_t z2zE3189;
      {
        uint64_t z2zE3188;
        {
          uint64_t z3zE3011;
          z3zE3011 = (uint64_t)(zk);
          z2zE3188 = zhash_little_endian_index(z3zE3011);
        }
        z2zE3189 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, z2zE3188);
      }
      zconsz3z5bv8(&zout, z2zE3189, zout);
      z3zE2889 = UNIT;
      zk = (zk + z3zE2888);
      goto for_start_3347;
    }
  for_end_3348: ;
  }
  unit z3zE2890;
  z3zE2890 = UNIT;
  COPY(zz5listz8z5bv8z9)((*(&z8zE98)), zout);
  KILL(zz5listz8z5bv8z9)(&zout);
end_function_3349: ;
  goto end_function_3656;
end_block_exception_3350: ;
  goto end_function_3656;
end_function_3656: ;
}

void zaddress_to_bytes(zz5listz8z5bv8z9 *z8zE99, sail_fixed_bytes_20 zbytes)
{
  zz5listz8z5bv8z9 zout;
  CREATE(zz5listz8z5bv8z9)(&zout);
  {
  }
  int64_t z3zE2880;
  {    z3zE2880 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE2881;
  {    z3zE2881 = (int64_t)(UINT64_C(19));
  }
  int64_t z3zE2882;
  {    z3zE2882 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zk;
    zk = z3zE2880;
    unit z3zE2883;
  for_start_3343: ;
    {
      if ((z3zE2881 < zk)) goto for_end_3344;
      uint64_t z2zE3187;
      {
        uint64_t z2zE3186;
        {
          uint64_t z3zE3012;
          z3zE3012 = (uint64_t)(zk);
          z2zE3186 = zaddress_little_endian_index(z3zE3012);
        }
        z2zE3187 = fast_unsigned_vector_access_fixed_bytes_20(zbytes, z2zE3186);
      }
      zconsz3z5bv8(&zout, z2zE3187, zout);
      z3zE2883 = UNIT;
      zk = (zk + z3zE2882);
      goto for_start_3343;
    }
  for_end_3344: ;
  }
  unit z3zE2884;
  z3zE2884 = UNIT;
  COPY(zz5listz8z5bv8z9)((*(&z8zE99)), zout);
  KILL(zz5listz8z5bv8z9)(&zout);
end_function_3345: ;
  goto end_function_3655;
end_block_exception_3346: ;
  goto end_function_3655;
end_function_3655: ;
}

