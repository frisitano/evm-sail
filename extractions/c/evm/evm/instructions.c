/* Generated from sail/evm/instructions.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
uint64_t zdecode_single_stack_index(uint64_t zimmediate)
{
  uint64_t z8zE413;
  bool z2zE2452;
  z2zE2452 = zdeep_stack_immediate_valid(zimmediate);
  unit z3zE2643;
  z3zE2643 = sail_assert(z2zE2452, "sail/evm/instructions.sail:72.48-72.49");
  uint64_t zvalue;
  zvalue = ((uint64_t) zimmediate);
  bool z2zE2453;
  z2zE2453 = (!(UINT64_C(90) < zvalue));
  if (z2zE2453) {
    {    z8zE413 = (zvalue + UINT64_C(145));
    }
  } else {
    bool z2zE2454;
    z2zE2454 = (!(zvalue < UINT64_C(128)));
    unit z3zE2644;
    z3zE2644 = sail_assert(z2zE2454, "sail/evm/instructions.sail:77.27-77.28");
    {    z8zE413 = (zvalue - UINT64_C(111));
    }
  }
end_function_2614: ;
  return z8zE413;
end_block_exception_2615: ;

  return UINT64_C(0xdeadc0de);
}

struct ztuple_z8z5u64zCz0z5u64z9 zdecode_exchange_stack_indices(uint64_t zimmediate)
{
  struct ztuple_z8z5u64zCz0z5u64z9 z8zE414;
  bool z2zE2444;
  z2zE2444 = zexchange_immediate_valid(zimmediate);
  unit z3zE2640;
  z3zE2640 = sail_assert(z2zE2444, "sail/evm/instructions.sail:85.46-85.47");
  uint64_t zshifted;
  zshifted = (zimmediate ^ UINT64_C(0x8F));
  uint64_t zquotient;
  {
    uint64_t z2zE2451;
    z2zE2451 = (safe_rshift(UINT64_MAX, 64 - 4) & (zshifted >> UINT64_C(4)));
    zquotient = ((uint64_t) z2zE2451);
  }
  uint64_t zremainder;
  {
    uint64_t z2zE2450;
    z2zE2450 = (safe_rshift(UINT64_MAX, 64 - 4) & (zshifted >> UINT64_C(0)));
    zremainder = ((uint64_t) z2zE2450);
  }
  bool z2zE2445;
  z2zE2445 = (zquotient < zremainder);
  if (z2zE2445) {
    uint64_t z2zE2446;
    {    z2zE2446 = (zquotient + UINT64_C(1));
    }
    uint64_t z2zE2447;
    {    z2zE2447 = (zremainder + UINT64_C(1));
    }
    struct ztuple_z8z5u64zCz0z5u64z9 z3zE2642;
    z3zE2642.ztup0 = z2zE2446;
    z3zE2642.ztup1 = z2zE2447;
    z8zE414 = z3zE2642;
  } else {
    uint64_t z2zE2448;
    {    z2zE2448 = (zremainder + UINT64_C(1));
    }
    uint64_t z2zE2449;
    {    z2zE2449 = (UINT64_C(29) - zquotient);
    }
    struct ztuple_z8z5u64zCz0z5u64z9 z3zE2641;
    z3zE2641.ztup0 = z2zE2448;
    z3zE2641.ztup1 = z2zE2449;
    z8zE414 = z3zE2641;
  }
end_function_2612: ;
  return z8zE414;
end_block_exception_2613: ;
  struct ztuple_z8z5u64zCz0z5u64z9 z8zE968 = { .ztup0 = UINT64_C(0xdeadc0de), .ztup1 = UINT64_C(0xdeadc0de) };
  return z8zE968;
}

