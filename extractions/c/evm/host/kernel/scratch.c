/* Generated from sail/host/kernel/scratch.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
uint64_t zscratch_begin(unit z3zE3602)
{
  uint64_t z8zE122;
  struct zByteSliceFields zarena;
  zarena = zscratch_arena;
  z8zE122 = zarena.zlen;
end_function_3687: ;
  return z8zE122;
end_block_exception_3688: ;

  return UINT64_C(0xdeadc0de);
}

unit zscratch_push_bytes(zz5listz8z5bv8z9 zdata, uint64_t zlen)
{
  unit z8zE123;
  bool z2zE3851;
  z2zE3851 = (zlen != UINT64_C(0));
  if (z2zE3851) {
    struct zByteSliceFields zarena;
    zarena = zscratch_arena;
    struct zByteRegionResult z2zE3853;
    CREATE(zByteRegionResult)(&z2zE3853);
    {
      uint64_t z2zE3852;
      z2zE3852 = zarena.zlen;
      scratch_store_bytes(&z2zE3853, z2zE3852, zdata, zlen);
    }
    unit z3zE3599;
    {
      if (z2zE3853.kind != Kind_zByteRegionReady) goto case_3684;
      struct zByteSliceFields zexpanded;
      zexpanded = z2zE3853.variants.zByteRegionReady;
      zscratch_arena = zexpanded;
      z3zE3599 = UNIT;
      goto finish_match_3682;
    }
  case_3684: ;
    {
      /* complete */
      z3zE3599 = sail_assert(false, "scratch byte append");
      goto finish_match_3682;
    }
  case_3683: ;
  finish_match_3682: ;
    z8zE123 = z3zE3599;
    KILL(zByteRegionResult)(&z2zE3853);
  } else {  z8zE123 = UNIT;  }
end_function_3685: ;
  return z8zE123;
end_block_exception_3686: ;

  return UNIT;
}

unit zscratch_push_slice(struct zByteSliceFields zdata)
{
  unit z8zE124;
  bool z2zE3848;
  {
    uint64_t z2zE3847;
    z2zE3847 = zdata.zlen;
    z2zE3848 = (z2zE3847 != UINT64_C(0));
  }
  if (z2zE3848) {
    struct zByteSliceFields zarena;
    zarena = zscratch_arena;
    struct zByteRegionResult z2zE3850;
    CREATE(zByteRegionResult)(&z2zE3850);
    {
      uint64_t z2zE3849;
      z2zE3849 = zarena.zlen;
      scratch_store_slice(&z2zE3850, z2zE3849, zdata);
    }
    unit z3zE3596;
    {
      if (z2zE3850.kind != Kind_zByteRegionReady) goto case_3679;
      struct zByteSliceFields zexpanded;
      zexpanded = z2zE3850.variants.zByteRegionReady;
      zscratch_arena = zexpanded;
      z3zE3596 = UNIT;
      goto finish_match_3677;
    }
  case_3679: ;
    {
      /* complete */
      z3zE3596 = sail_assert(false, "scratch slice append");
      goto finish_match_3677;
    }
  case_3678: ;
  finish_match_3677: ;
    z8zE124 = z3zE3596;
    KILL(zByteRegionResult)(&z2zE3850);
  } else {  z8zE124 = UNIT;  }
end_function_3680: ;
  return z8zE124;
end_block_exception_3681: ;

  return UNIT;
}

unit zscratch_push_b256(sail_fixed_bytes_32 zdata, uint64_t zlen)
{
  unit z8zE125;
  bool z2zE3844;
  z2zE3844 = (zlen != UINT64_C(0));
  if (z2zE3844) {
    struct zByteSliceFields zarena;
    zarena = zscratch_arena;
    struct zByteRegionResult z2zE3846;
    CREATE(zByteRegionResult)(&z2zE3846);
    {
      uint64_t z2zE3845;
      z2zE3845 = zarena.zlen;
      scratch_store_b256(&z2zE3846, z2zE3845, zdata, zlen);
    }
    unit z3zE3593;
    {
      if (z2zE3846.kind != Kind_zByteRegionReady) goto case_3674;
      struct zByteSliceFields zexpanded;
      zexpanded = z2zE3846.variants.zByteRegionReady;
      zscratch_arena = zexpanded;
      z3zE3593 = UNIT;
      goto finish_match_3672;
    }
  case_3674: ;
    {
      /* complete */
      z3zE3593 = sail_assert(false, "scratch fixed-byte append");
      goto finish_match_3672;
    }
  case_3673: ;
  finish_match_3672: ;
    z8zE125 = z3zE3593;
    KILL(zByteRegionResult)(&z2zE3846);
  } else {  z8zE125 = UNIT;  }
end_function_3675: ;
  return z8zE125;
end_block_exception_3676: ;

  return UNIT;
}

struct zByteSliceFields zscratch_finish(uint64_t zstart)
{
  struct zByteSliceFields z8zE126;
  uint64_t zstart_offset;
  zstart_offset = zstart;
  struct zByteSliceFields zarena;
  zarena = zscratch_arena;
  uint64_t zstop_offset;
  zstop_offset = zarena.zlen;
  bool z2zE3842;
  z2zE3842 = (!(zstop_offset < zstart_offset));
  if (z2zE3842) {
    uint64_t z2zE3843;
    {    z2zE3843 = (zstop_offset - zstart_offset);
    }
    z8zE126 = zsub_slice(zarena, zstart, z2zE3843);
  } else {
    unit z3zE3592;
    z3zE3592 = sail_assert(false, "scratch finish mark");
    sail_match_failure("scratch_finish");
    /* unreachable after exit */
  }
end_function_3670: ;
  return z8zE126;
end_block_exception_3671: ;
  struct zByteSliceFields z8zE831 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE831;
}

unit zscratch_rewind(uint64_t zmark)
{
  unit z8zE127;
  uint64_t zmark_offset;
  zmark_offset = zmark;
  struct zByteSliceFields zarena;
  zarena = zscratch_arena;
  uint64_t zcursor_offset;
  zcursor_offset = zarena.zlen;
  bool z2zE3841;
  z2zE3841 = (!(zcursor_offset < zmark_offset));
  if (z2zE3841) {
    zscratch_arena = zsub_slice(zarena, UINT64_C(0), zmark);
    unit z3zE3591;
    z3zE3591 = UNIT;
    z8zE127 = scratch_truncate(zmark);
  } else {  z8zE127 = sail_assert(false, "scratch rewind mark");  }
end_function_3668: ;
  return z8zE127;
end_block_exception_3669: ;

  return UNIT;
}

unit zscratch_reset(unit z3zE3589)
{
  unit z8zE128;
  zscratch_arena = zbyte_slice(zScratchSource, UINT64_C(0), UINT64_C(0));
  unit z3zE3590;
  z3zE3590 = UNIT;
  z8zE128 = scratch_truncate(UINT64_C(0));
end_function_3666: ;
  return z8zE128;
end_block_exception_3667: ;

  return UNIT;
}

