/* Generated from sail/kernel/logs.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
unit zk_log(sail_fixed_bytes_20 za, zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics, struct zBytes zdata)
{
  unit z8zE223;
  z8zE223 = log_append_record(za, ztopics, zdata);
end_function_2799: ;
  return z8zE223;
end_block_exception_2800: ;

  return UNIT;
}

unit zk_emit_transfer_log(sail_fixed_bytes_20 zsrc, sail_fixed_bytes_20 zdst, sail_u256 zv)
{
  unit z8zE224;
  bool z2zE2501;
  {
    bool z2zE2500;
    z2zE2500 = zfork_lt(zk_fork, zAmsterdam);
    bool z3zE2403;
    if (z2zE2500) {  z3zE2403 = true;  } else {
      bool z2zE2499;
      z2zE2499 = zword_is_zzero(zv);
      bool z3zE2402;
      if (z2zE2499) {  z3zE2402 = true;  } else {  z3zE2402 = eq_fixed_bytes_20(zsrc, zdst);  }
      z3zE2403 = z3zE2402;
    }
    z2zE2501 = z3zE2403;
  }
  if (z2zE2501) {  z8zE224 = UNIT;  } else {
    zz5listz8z5structz0zz__sail_c_repr_u256z9 z2zE2505;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE2505);
    {
      sail_u256 z2zE2502;
      z2zE2502 = evmsail_address_to_word(zsrc);
      sail_u256 z2zE2503;
      z2zE2503 = evmsail_address_to_word(zdst);
      zconsz3z5structz0zz__sail_c_repr_u256(&z2zE2505, z2zE2503, z2zE2505);
      zconsz3z5structz0zz__sail_c_repr_u256(&z2zE2505, z2zE2502, z2zE2505);
      zconsz3z5structz0zz__sail_c_repr_u256(&z2zE2505, zEIP7708_TRANSFER_TOPIC, z2zE2505);
    }
    struct zBytes z2zE2506;
    CREATE(zBytes)(&z2zE2506);
    {
      zz5listz8z5bv8z9 z2zE2504;
      CREATE(zz5listz8z5bv8z9)(&z2zE2504);
      zword_to_bytes32(&z2zE2504, zv);
      zbytes_list(&z2zE2506, z2zE2504, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE2504);
    }
    z8zE224 = log_append_record(zEIP7708_SYSTEM_ADDRESS, z2zE2505, z2zE2506);
    KILL(zBytes)(&z2zE2506);
    KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE2505);
  }
end_function_2797: ;
  return z8zE224;
end_block_exception_2798: ;

  return UNIT;
}
