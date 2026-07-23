/* Generated from sail/host/kernel/logs.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
unit zk_log(sail_fixed_bytes_20 za, zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics, struct zBytes zdata)
{
  unit z8zE248;
  z8zE248 = log_append_record(za, ztopics, zdata);
end_function_3110: ;
  return z8zE248;
end_block_exception_3111: ;

  return UNIT;
}

unit zk_emit_transfer_log(sail_fixed_bytes_20 zsrc, sail_fixed_bytes_20 zdst, sail_u256 zv)
{
  unit z8zE249;
  bool z2zE3090;
  {
    bool z2zE3089;
    z2zE3089 = zfork_lt(zk_fork, zAmsterdam);
    bool z3zE3065;
    if (z2zE3089) {  z3zE3065 = true;  } else {
      bool z2zE3088;
      z2zE3088 = zword_is_zzero(zv);
      bool z3zE3064;
      if (z2zE3088) {  z3zE3064 = true;  } else {  z3zE3064 = eq_fixed_bytes_20(zsrc, zdst);  }
      z3zE3065 = z3zE3064;
    }
    z2zE3090 = z3zE3065;
  }
  if (z2zE3090) {  z8zE249 = UNIT;  } else {
    zz5listz8z5structz0zz__sail_c_repr_u256z9 z2zE3094;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE3094);
    {
      sail_u256 z2zE3091;
      z2zE3091 = zaddress_to_word(zsrc);
      sail_u256 z2zE3092;
      z2zE3092 = zaddress_to_word(zdst);
      zconsz3z5structz0zz__sail_c_repr_u256(&z2zE3094, z2zE3092, z2zE3094);
      zconsz3z5structz0zz__sail_c_repr_u256(&z2zE3094, z2zE3091, z2zE3094);
      zconsz3z5structz0zz__sail_c_repr_u256(&z2zE3094, zEIP7708_TRANSFER_TOPIC, z2zE3094);
    }
    struct zBytes z2zE3095;
    CREATE(zBytes)(&z2zE3095);
    {
      zz5listz8z5bv8z9 z2zE3093;
      CREATE(zz5listz8z5bv8z9)(&z2zE3093);
      zword_to_bytes32(&z2zE3093, zv);
      zbytes_list(&z2zE3095, z2zE3093, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE3093);
    }
    z8zE249 = log_append_record(zEIP7708_SYSTEM_ADDRESS, z2zE3094, z2zE3095);
    KILL(zBytes)(&z2zE3095);
    KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE3094);
  }
end_function_3108: ;
  return z8zE249;
end_block_exception_3109: ;

  return UNIT;
}

