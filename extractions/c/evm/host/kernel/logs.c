/* Generated from sail/host/kernel/logs.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
unit zk_log(sail_fixed_bytes_20 za, zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics, struct zBytes zdata)
{
  unit z8zE245;
  z8zE245 = log_append_record(za, ztopics, zdata);
end_function_3106: ;
  return z8zE245;
end_block_exception_3107: ;

  return UNIT;
}

unit zk_emit_transfer_log(sail_fixed_bytes_20 zsrc, sail_fixed_bytes_20 zdst, sail_u256 zv)
{
  unit z8zE246;
  bool z2zE3067;
  {
    bool z2zE3066;
    z2zE3066 = zfork_lt(zk_fork, zAmsterdam);
    bool z3zE3069;
    if (z2zE3066) {  z3zE3069 = true;  } else {
      bool z2zE3065;
      z2zE3065 = zword_is_zzero(zv);
      bool z3zE3068;
      if (z2zE3065) {  z3zE3068 = true;  } else {  z3zE3068 = eq_fixed_bytes_20(zsrc, zdst);  }
      z3zE3069 = z3zE3068;
    }
    z2zE3067 = z3zE3069;
  }
  if (z2zE3067) {  z8zE246 = UNIT;  } else {
    zz5listz8z5structz0zz__sail_c_repr_u256z9 z2zE3071;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE3071);
    {
      sail_u256 z2zE3068;
      z2zE3068 = zaddress_to_word(zsrc);
      sail_u256 z2zE3069;
      z2zE3069 = zaddress_to_word(zdst);
      zconsz3z5structz0zz__sail_c_repr_u256(&z2zE3071, z2zE3069, z2zE3071);
      zconsz3z5structz0zz__sail_c_repr_u256(&z2zE3071, z2zE3068, z2zE3071);
      zconsz3z5structz0zz__sail_c_repr_u256(&z2zE3071, zEIP7708_TRANSFER_TOPIC, z2zE3071);
    }
    struct zBytes z2zE3072;
    CREATE(zBytes)(&z2zE3072);
    {
      zz5listz8z5bv8z9 z2zE3070;
      CREATE(zz5listz8z5bv8z9)(&z2zE3070);
      zword_to_bytes32(&z2zE3070, zv);
      zbytes_list(&z2zE3072, z2zE3070, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE3070);
    }
    z8zE246 = log_append_record(zEIP7708_SYSTEM_ADDRESS, z2zE3071, z2zE3072);
    KILL(zBytes)(&z2zE3072);
    KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE3071);
  }
end_function_3104: ;
  return z8zE246;
end_block_exception_3105: ;

  return UNIT;
}

