/* Generated from sail/evm/instructions.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
uint64_t zdecode_single_stack_index(uint64_t zimmediate)
{
  uint64_t z8zE416;
  bool z2zE2475;
  z2zE2475 = zdeep_stack_immediate_valid(zimmediate);
  unit z3zE2639;
  z3zE2639 = sail_assert(z2zE2475, "sail/evm/instructions.sail:72.48-72.49");
  uint64_t zvalue;
  zvalue = ((uint64_t) zimmediate);
  bool z2zE2476;
  z2zE2476 = (!(UINT64_C(90) < zvalue));
  if (z2zE2476) {
    {    z8zE416 = (zvalue + UINT64_C(145));
    }
  } else {
    bool z2zE2477;
    z2zE2477 = (!(zvalue < UINT64_C(128)));
    unit z3zE2640;
    z3zE2640 = sail_assert(z2zE2477, "sail/evm/instructions.sail:77.27-77.28");
    {    z8zE416 = (zvalue - UINT64_C(111));
    }
  }
end_function_2618: ;
  return z8zE416;
end_block_exception_2619: ;

  return UINT64_C(0xdeadc0de);
}

struct ztuple_z8z5u64zCz0z5u64z9 zdecode_exchange_stack_indices(uint64_t zimmediate)
{
  struct ztuple_z8z5u64zCz0z5u64z9 z8zE417;
  bool z2zE2467;
  z2zE2467 = zexchange_immediate_valid(zimmediate);
  unit z3zE2636;
  z3zE2636 = sail_assert(z2zE2467, "sail/evm/instructions.sail:85.46-85.47");
  uint64_t zshifted;
  zshifted = (zimmediate ^ UINT64_C(0x8F));
  uint64_t zquotient;
  {
    uint64_t z2zE2474;
    z2zE2474 = (safe_rshift(UINT64_MAX, 64 - 4) & (zshifted >> UINT64_C(4)));
    zquotient = ((uint64_t) z2zE2474);
  }
  uint64_t zremainder;
  {
    uint64_t z2zE2473;
    z2zE2473 = (safe_rshift(UINT64_MAX, 64 - 4) & (zshifted >> UINT64_C(0)));
    zremainder = ((uint64_t) z2zE2473);
  }
  bool z2zE2468;
  z2zE2468 = (zquotient < zremainder);
  if (z2zE2468) {
    uint64_t z2zE2469;
    {    z2zE2469 = (zquotient + UINT64_C(1));
    }
    uint64_t z2zE2470;
    {    z2zE2470 = (zremainder + UINT64_C(1));
    }
    struct ztuple_z8z5u64zCz0z5u64z9 z3zE2638;
    z3zE2638.ztup0 = z2zE2469;
    z3zE2638.ztup1 = z2zE2470;
    z8zE417 = z3zE2638;
  } else {
    uint64_t z2zE2471;
    {    z2zE2471 = (zremainder + UINT64_C(1));
    }
    uint64_t z2zE2472;
    {    z2zE2472 = (UINT64_C(29) - zquotient);
    }
    struct ztuple_z8z5u64zCz0z5u64z9 z3zE2637;
    z3zE2637.ztup0 = z2zE2471;
    z3zE2637.ztup1 = z2zE2472;
    z8zE417 = z3zE2637;
  }
end_function_2616: ;
  return z8zE417;
end_block_exception_2617: ;
  struct ztuple_z8z5u64zCz0z5u64z9 z8zE980 = { .ztup0 = UINT64_C(0xdeadc0de), .ztup1 = UINT64_C(0xdeadc0de) };
  return z8zE980;
}

