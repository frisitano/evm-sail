/* Generated from sail/kernel/scratch.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
uint64_t zscratch_begin(unit z3zE2824)
{
  uint64_t z8zE121;
  struct zByteSliceFields zarena;
  zarena = zscratch_arena;
  z8zE121 = zarena.zlen;
end_function_3275: ;
  return z8zE121;
end_block_exception_3276: ;

  return UINT64_C(0xdeadc0de);
}

unit zscratch_push_bytes(zz5listz8z5bv8z9 zdata, uint64_t zlen)
{
  unit z8zE122;
  bool z2zE3146;
  z2zE3146 = (zlen != UINT64_C(0));
  if (z2zE3146) {
    struct zByteSliceFields zarena;
    zarena = zscratch_arena;
    struct zByteRegionResult z2zE3148;
    CREATE(zByteRegionResult)(&z2zE3148);
    {
      uint64_t z2zE3147;
      z2zE3147 = zarena.zlen;
      scratch_store_bytes(&z2zE3148, z2zE3147, zdata, zlen);
    }
    unit z3zE2821;
    {
      if (z2zE3148.kind != Kind_zByteRegionReady) goto case_3272;
      struct zByteSliceFields zexpanded;
      zexpanded = z2zE3148.variants.zByteRegionReady;
      zscratch_arena = zexpanded;
      z3zE2821 = UNIT;
      goto finish_match_3270;
    }
  case_3272: ;
    {
      /* complete */
      z3zE2821 = sail_assert(false, "scratch byte append");
      goto finish_match_3270;
    }
  case_3271: ;
  finish_match_3270: ;
    z8zE122 = z3zE2821;
    KILL(zByteRegionResult)(&z2zE3148);
  } else {  z8zE122 = UNIT;  }
end_function_3273: ;
  return z8zE122;
end_block_exception_3274: ;

  return UNIT;
}

unit zscratch_push_slice(struct zByteSliceFields zdata)
{
  unit z8zE123;
  bool z2zE3143;
  {
    uint64_t z2zE3142;
    z2zE3142 = zdata.zlen;
    z2zE3143 = (z2zE3142 != UINT64_C(0));
  }
  if (z2zE3143) {
    struct zByteSliceFields zarena;
    zarena = zscratch_arena;
    struct zByteRegionResult z2zE3145;
    CREATE(zByteRegionResult)(&z2zE3145);
    {
      uint64_t z2zE3144;
      z2zE3144 = zarena.zlen;
      scratch_store_slice(&z2zE3145, z2zE3144, zdata);
    }
    unit z3zE2818;
    {
      if (z2zE3145.kind != Kind_zByteRegionReady) goto case_3267;
      struct zByteSliceFields zexpanded;
      zexpanded = z2zE3145.variants.zByteRegionReady;
      zscratch_arena = zexpanded;
      z3zE2818 = UNIT;
      goto finish_match_3265;
    }
  case_3267: ;
    {
      /* complete */
      z3zE2818 = sail_assert(false, "scratch slice append");
      goto finish_match_3265;
    }
  case_3266: ;
  finish_match_3265: ;
    z8zE123 = z3zE2818;
    KILL(zByteRegionResult)(&z2zE3145);
  } else {  z8zE123 = UNIT;  }
end_function_3268: ;
  return z8zE123;
end_block_exception_3269: ;

  return UNIT;
}

unit zscratch_push_b256(sail_fixed_bytes_32 zdata, uint64_t zlen)
{
  unit z8zE124;
  bool z2zE3139;
  z2zE3139 = (zlen != UINT64_C(0));
  if (z2zE3139) {
    struct zByteSliceFields zarena;
    zarena = zscratch_arena;
    struct zByteRegionResult z2zE3141;
    CREATE(zByteRegionResult)(&z2zE3141);
    {
      uint64_t z2zE3140;
      z2zE3140 = zarena.zlen;
      scratch_store_b256(&z2zE3141, z2zE3140, zdata, zlen);
    }
    unit z3zE2815;
    {
      if (z2zE3141.kind != Kind_zByteRegionReady) goto case_3262;
      struct zByteSliceFields zexpanded;
      zexpanded = z2zE3141.variants.zByteRegionReady;
      zscratch_arena = zexpanded;
      z3zE2815 = UNIT;
      goto finish_match_3260;
    }
  case_3262: ;
    {
      /* complete */
      z3zE2815 = sail_assert(false, "scratch fixed-byte append");
      goto finish_match_3260;
    }
  case_3261: ;
  finish_match_3260: ;
    z8zE124 = z3zE2815;
    KILL(zByteRegionResult)(&z2zE3141);
  } else {  z8zE124 = UNIT;  }
end_function_3263: ;
  return z8zE124;
end_block_exception_3264: ;

  return UNIT;
}

struct zByteSliceFields zscratch_finish(uint64_t zstart)
{
  struct zByteSliceFields z8zE125;
  uint64_t zstart_offset;
  zstart_offset = zstart;
  struct zByteSliceFields zarena;
  zarena = zscratch_arena;
  uint64_t zstop_offset;
  zstop_offset = zarena.zlen;
  bool z2zE3137;
  z2zE3137 = (!(zstop_offset < zstart_offset));
  if (z2zE3137) {
    uint64_t z2zE3138;
    {    z2zE3138 = (zstop_offset - zstart_offset);
    }
    z8zE125 = zsub_slice(zarena, zstart, z2zE3138);
  } else {
    unit z3zE2814;
    z3zE2814 = sail_assert(false, "scratch finish mark");
    sail_match_failure("scratch_finish");
    /* unreachable after exit */
  }
end_function_3258: ;
  return z8zE125;
end_block_exception_3259: ;
  struct zByteSliceFields z8zE781 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE781;
}

unit zscratch_rewind(uint64_t zmark)
{
  unit z8zE126;
  uint64_t zmark_offset;
  zmark_offset = zmark;
  struct zByteSliceFields zarena;
  zarena = zscratch_arena;
  uint64_t zcursor_offset;
  zcursor_offset = zarena.zlen;
  bool z2zE3136;
  z2zE3136 = (!(zcursor_offset < zmark_offset));
  if (z2zE3136) {
    zscratch_arena = zsub_slice(zarena, UINT64_C(0), zmark);
    unit z3zE2813;
    z3zE2813 = UNIT;
    z8zE126 = scratch_truncate(zmark);
  } else {  z8zE126 = sail_assert(false, "scratch rewind mark");  }
end_function_3256: ;
  return z8zE126;
end_block_exception_3257: ;

  return UNIT;
}

unit zscratch_reset(unit z3zE2811)
{
  unit z8zE127;
  zscratch_arena = zbyte_slice(zScratchSource, UINT64_C(0), UINT64_C(0));
  unit z3zE2812;
  z3zE2812 = UNIT;
  z8zE127 = scratch_truncate(UINT64_C(0));
end_function_3254: ;
  return z8zE127;
end_block_exception_3255: ;

  return UNIT;
}
