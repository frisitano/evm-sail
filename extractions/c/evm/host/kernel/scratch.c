/* Generated from sail/host/kernel/scratch.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
uint64_t zscratch_begin(unit z3zE3673)
{
  uint64_t z8zE122;
  struct zByteSliceFields zarena;
  zarena = zscratch_arena;
  z8zE122 = zarena.zlen;
end_function_3706: ;
  return z8zE122;
end_block_exception_3707: ;

  return UINT64_C(0xdeadc0de);
}

unit zscratch_push_bytes(zz5listz8z5bv8z9 zdata, uint64_t zlen)
{
  unit z8zE123;
  bool z2zE3890;
  z2zE3890 = (zlen != UINT64_C(0));
  if (z2zE3890) {
    struct zByteSliceFields zarena;
    zarena = zscratch_arena;
    struct zByteRegionResult z2zE3892;
    CREATE(zByteRegionResult)(&z2zE3892);
    {
      uint64_t z2zE3891;
      z2zE3891 = zarena.zlen;
      scratch_store_bytes(&z2zE3892, z2zE3891, zdata, zlen);
    }
    unit z3zE3670;
    {
      if (z2zE3892.kind != Kind_zByteRegionReady) goto case_3703;
      struct zByteSliceFields zexpanded;
      zexpanded = z2zE3892.variants.zByteRegionReady;
      zscratch_arena = zexpanded;
      z3zE3670 = UNIT;
      goto finish_match_3701;
    }
  case_3703: ;
    {
      /* complete */
      z3zE3670 = sail_assert(false, "scratch byte append");
      goto finish_match_3701;
    }
  case_3702: ;
  finish_match_3701: ;
    z8zE123 = z3zE3670;
    KILL(zByteRegionResult)(&z2zE3892);
  } else {  z8zE123 = UNIT;  }
end_function_3704: ;
  return z8zE123;
end_block_exception_3705: ;

  return UNIT;
}

unit zscratch_push_slice(struct zByteSliceFields zdata)
{
  unit z8zE124;
  bool z2zE3887;
  {
    uint64_t z2zE3886;
    z2zE3886 = zdata.zlen;
    z2zE3887 = (z2zE3886 != UINT64_C(0));
  }
  if (z2zE3887) {
    struct zByteSliceFields zarena;
    zarena = zscratch_arena;
    struct zByteRegionResult z2zE3889;
    CREATE(zByteRegionResult)(&z2zE3889);
    {
      uint64_t z2zE3888;
      z2zE3888 = zarena.zlen;
      scratch_store_slice(&z2zE3889, z2zE3888, zdata);
    }
    unit z3zE3667;
    {
      if (z2zE3889.kind != Kind_zByteRegionReady) goto case_3698;
      struct zByteSliceFields zexpanded;
      zexpanded = z2zE3889.variants.zByteRegionReady;
      zscratch_arena = zexpanded;
      z3zE3667 = UNIT;
      goto finish_match_3696;
    }
  case_3698: ;
    {
      /* complete */
      z3zE3667 = sail_assert(false, "scratch slice append");
      goto finish_match_3696;
    }
  case_3697: ;
  finish_match_3696: ;
    z8zE124 = z3zE3667;
    KILL(zByteRegionResult)(&z2zE3889);
  } else {  z8zE124 = UNIT;  }
end_function_3699: ;
  return z8zE124;
end_block_exception_3700: ;

  return UNIT;
}

unit zscratch_push_b256(sail_fixed_bytes_32 zdata, uint64_t zlen)
{
  unit z8zE125;
  bool z2zE3883;
  z2zE3883 = (zlen != UINT64_C(0));
  if (z2zE3883) {
    struct zByteSliceFields zarena;
    zarena = zscratch_arena;
    struct zByteRegionResult z2zE3885;
    CREATE(zByteRegionResult)(&z2zE3885);
    {
      uint64_t z2zE3884;
      z2zE3884 = zarena.zlen;
      scratch_store_b256(&z2zE3885, z2zE3884, zdata, zlen);
    }
    unit z3zE3664;
    {
      if (z2zE3885.kind != Kind_zByteRegionReady) goto case_3693;
      struct zByteSliceFields zexpanded;
      zexpanded = z2zE3885.variants.zByteRegionReady;
      zscratch_arena = zexpanded;
      z3zE3664 = UNIT;
      goto finish_match_3691;
    }
  case_3693: ;
    {
      /* complete */
      z3zE3664 = sail_assert(false, "scratch fixed-byte append");
      goto finish_match_3691;
    }
  case_3692: ;
  finish_match_3691: ;
    z8zE125 = z3zE3664;
    KILL(zByteRegionResult)(&z2zE3885);
  } else {  z8zE125 = UNIT;  }
end_function_3694: ;
  return z8zE125;
end_block_exception_3695: ;

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
  bool z2zE3881;
  z2zE3881 = (!(zstop_offset < zstart_offset));
  if (z2zE3881) {
    uint64_t z2zE3882;
    {    z2zE3882 = (zstop_offset - zstart_offset);
    }
    z8zE126 = zsub_slice(zarena, zstart, z2zE3882);
  } else {
    unit z3zE3663;
    z3zE3663 = sail_assert(false, "scratch finish mark");
    sail_match_failure("scratch_finish");
    /* unreachable after exit */
  }
end_function_3689: ;
  return z8zE126;
end_block_exception_3690: ;
  struct zByteSliceFields z8zE834 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE834;
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
  bool z2zE3880;
  z2zE3880 = (!(zcursor_offset < zmark_offset));
  if (z2zE3880) {
    zscratch_arena = zsub_slice(zarena, UINT64_C(0), zmark);
    unit z3zE3662;
    z3zE3662 = UNIT;
    z8zE127 = scratch_truncate(zmark);
  } else {  z8zE127 = sail_assert(false, "scratch rewind mark");  }
end_function_3687: ;
  return z8zE127;
end_block_exception_3688: ;

  return UNIT;
}

unit zscratch_reset(unit z3zE3660)
{
  unit z8zE128;
  zscratch_arena = zbyte_slice(zScratchSource, UINT64_C(0), UINT64_C(0));
  unit z3zE3661;
  z3zE3661 = UNIT;
  z8zE128 = scratch_truncate(UINT64_C(0));
end_function_3685: ;
  return z8zE128;
end_block_exception_3686: ;

  return UNIT;
}

