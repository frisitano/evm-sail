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
  int64_t z3zE3743;
  {    z3zE3743 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE3744;
  {    z3zE3744 = (int64_t)(UINT64_C(31));
  }
  int64_t z3zE3745;
  {    z3zE3745 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zbyte_index;
    zbyte_index = z3zE3743;
    unit z3zE3747;
  for_start_3786: ;
    {
      if ((z3zE3744 < zbyte_index)) goto for_end_3787;
      uint64_t z2zE3937;
      z2zE3937 = zword_low_byte(zremaining);
      zconsz3z5bv8(&zout, z2zE3937, zout);
      unit z3zE3746;
      z3zE3746 = UNIT;
      zremaining = zword_shift_right(zremaining, UINT64_C(8));
      z3zE3747 = UNIT;
      zbyte_index = (zbyte_index + z3zE3745);
      goto for_start_3786;
    }
  for_end_3787: ;
  }
  unit z3zE3748;
  z3zE3748 = UNIT;
  COPY(zz5listz8z5bv8z9)((*(&z8zE96)), zout);
  KILL(zz5listz8z5bv8z9)(&zout);
end_function_3788: ;
  goto end_function_4088;
end_block_exception_3789: ;
  goto end_function_4088;
end_function_4088: ;
}

void zhash_to_bytes32(zz5listz8z5bv8z9 *z8zE97, sail_fixed_bytes_32 zbytes)
{
  zz5listz8z5bv8z9 zout;
  CREATE(zz5listz8z5bv8z9)(&zout);
  {
  }
  int64_t z3zE3737;
  {    z3zE3737 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE3738;
  {    z3zE3738 = (int64_t)(UINT64_C(31));
  }
  int64_t z3zE3739;
  {    z3zE3739 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zk;
    zk = z3zE3737;
    unit z3zE3740;
  for_start_3782: ;
    {
      if ((z3zE3738 < zk)) goto for_end_3783;
      uint64_t z2zE3936;
      z2zE3936 = fast_vector_access_fixed_bytes_32(zbytes, zk);
      zconsz3z5bv8(&zout, z2zE3936, zout);
      z3zE3740 = UNIT;
      zk = (zk + z3zE3739);
      goto for_start_3782;
    }
  for_end_3783: ;
  }
  unit z3zE3741;
  z3zE3741 = UNIT;
  COPY(zz5listz8z5bv8z9)((*(&z8zE97)), zout);
  KILL(zz5listz8z5bv8z9)(&zout);
end_function_3784: ;
  goto end_function_4087;
end_block_exception_3785: ;
  goto end_function_4087;
end_function_4087: ;
}

void zaddress_to_bytes(zz5listz8z5bv8z9 *z8zE98, sail_fixed_bytes_20 zbytes)
{
  zz5listz8z5bv8z9 zout;
  CREATE(zz5listz8z5bv8z9)(&zout);
  {
  }
  int64_t z3zE3731;
  {    z3zE3731 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE3732;
  {    z3zE3732 = (int64_t)(UINT64_C(19));
  }
  int64_t z3zE3733;
  {    z3zE3733 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zk;
    zk = z3zE3731;
    unit z3zE3734;
  for_start_3778: ;
    {
      if ((z3zE3732 < zk)) goto for_end_3779;
      uint64_t z2zE3935;
      z2zE3935 = fast_vector_access_fixed_bytes_20(zbytes, zk);
      zconsz3z5bv8(&zout, z2zE3935, zout);
      z3zE3734 = UNIT;
      zk = (zk + z3zE3733);
      goto for_start_3778;
    }
  for_end_3779: ;
  }
  unit z3zE3735;
  z3zE3735 = UNIT;
  COPY(zz5listz8z5bv8z9)((*(&z8zE98)), zout);
  KILL(zz5listz8z5bv8z9)(&zout);
end_function_3780: ;
  goto end_function_4086;
end_block_exception_3781: ;
  goto end_function_4086;
end_function_4086: ;
}

