/* Generated from sail/evm/instructions.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
uint64_t zdecode_single_stack_index(uint64_t zimmediate)
{
  uint64_t z8zE380;
  bool z2zE2015;
  z2zE2015 = zdeep_stack_immediate_valid(zimmediate);
  unit z3zE2030;
  z3zE2030 = sail_assert(z2zE2015, "sail/evm/instructions.sail:72.48-72.49");
  uint64_t zvalue;
  zvalue = ((uint64_t) zimmediate);
  bool z2zE2016;
  z2zE2016 = (!(UINT64_C(90) < zvalue));
  if (z2zE2016) {
    {    z8zE380 = (zvalue + UINT64_C(145));
    }
  } else {
    bool z2zE2017;
    z2zE2017 = (!(zvalue < UINT64_C(128)));
    unit z3zE2031;
    z3zE2031 = sail_assert(z2zE2017, "sail/evm/instructions.sail:77.27-77.28");
    {    z8zE380 = (zvalue - UINT64_C(111));
    }
  }
end_function_2342: ;
  return z8zE380;
end_block_exception_2343: ;

  return UINT64_C(0xdeadc0de);
}

struct ztuple_z8z5u64zCz0z5u64z9 zdecode_exchange_stack_indices(uint64_t zimmediate)
{
  struct ztuple_z8z5u64zCz0z5u64z9 z8zE381;
  bool z2zE2007;
  z2zE2007 = zexchange_immediate_valid(zimmediate);
  unit z3zE2027;
  z3zE2027 = sail_assert(z2zE2007, "sail/evm/instructions.sail:85.46-85.47");
  uint64_t zshifted;
  zshifted = (zimmediate ^ UINT64_C(0x8F));
  uint64_t zquotient;
  {
    uint64_t z2zE2014;
    z2zE2014 = (safe_rshift(UINT64_MAX, 64 - 4) & (zshifted >> UINT64_C(4)));
    zquotient = ((uint64_t) z2zE2014);
  }
  uint64_t zremainder;
  {
    uint64_t z2zE2013;
    z2zE2013 = (safe_rshift(UINT64_MAX, 64 - 4) & (zshifted >> UINT64_C(0)));
    zremainder = ((uint64_t) z2zE2013);
  }
  bool z2zE2008;
  z2zE2008 = (zquotient < zremainder);
  if (z2zE2008) {
    uint64_t z2zE2009;
    {    z2zE2009 = (zquotient + UINT64_C(1));
    }
    uint64_t z2zE2010;
    {    z2zE2010 = (zremainder + UINT64_C(1));
    }
    struct ztuple_z8z5u64zCz0z5u64z9 z3zE2029;
    z3zE2029.ztup0 = z2zE2009;
    z3zE2029.ztup1 = z2zE2010;
    z8zE381 = z3zE2029;
  } else {
    uint64_t z2zE2011;
    {    z2zE2011 = (zremainder + UINT64_C(1));
    }
    uint64_t z2zE2012;
    {    z2zE2012 = (UINT64_C(29) - zquotient);
    }
    struct ztuple_z8z5u64zCz0z5u64z9 z3zE2028;
    z3zE2028.ztup0 = z2zE2011;
    z3zE2028.ztup1 = z2zE2012;
    z8zE381 = z3zE2028;
  }
end_function_2340: ;
  return z8zE381;
end_block_exception_2341: ;
  struct ztuple_z8z5u64zCz0z5u64z9 z8zE881 = { .ztup0 = UINT64_C(0xdeadc0de), .ztup1 = UINT64_C(0xdeadc0de) };
  return z8zE881;
}

