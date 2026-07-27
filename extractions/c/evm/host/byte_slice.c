/* Generated from sail/host/byte_slice.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
uint64_t zslice_byte(struct zByteSliceFields zs, uint64_t zoff)
{
  uint64_t z8zE113;
  uint64_t zoffset;
  zoffset = zoff;
  uint64_t zlength;
  zlength = zs.zlen;
  bool z2zE3155;
  z2zE3155 = (zoffset < zlength);
  if (z2zE3155) {  z8zE113 = slice_byte_at(zs, zoff);  } else {  z8zE113 = UINT64_C(0x00);  }
end_function_3291: ;
  return z8zE113;
end_block_exception_3292: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zslice_count_nonzzero(struct zByteSliceFields zs)
{
  uint64_t z8zE114;
  z8zE114 = slice_count_nonzero(zs);
end_function_3289: ;
  return z8zE114;
end_block_exception_3290: ;

  return UINT64_C(0xdeadc0de);
}

bool zslice_strided_zzero(struct zByteSliceFields zs, uint64_t zstart, uint64_t zstride, uint64_t zwidth, uint64_t zcount)
{
  bool z8zE115;
  z8zE115 = slice_strided_zero(zs, zstart, zstride, zwidth, zcount);
end_function_3287: ;
  return z8zE115;
end_block_exception_3288: ;

  return false;
}

sail_u256 zslice_load(struct zByteSliceFields zs, uint64_t zoff)
{
  sail_u256 z8zE116;
  uint64_t zoffset;
  zoffset = zoff;
  uint64_t zlength;
  zlength = zs.zlen;
  bool z2zE3154;
  z2zE3154 = (zoffset < zlength);
  if (z2zE3154) {  z8zE116 = slice_load_word(zs, zoff);  } else {  z8zE116 = zZERO_WORD;  }
end_function_3285: ;
  return z8zE116;
end_block_exception_3286: ;

  return u256_zero();
}

sail_u256 zslice_load_word_offset(struct zByteSliceFields zs, sail_u256 zoff)
{
  sail_u256 z8zE117;
  uint64_t zslice_len;
  zslice_len = zs.zlen;
  bool z2zE3153;
  z2zE3153 = u256_lt_u64(zoff, zslice_len);
  if (z2zE3153) {
    {
      uint64_t z3zE3014;
      z3zE3014 = u256_to_u64(zoff);
      z8zE117 = zslice_load(zs, z3zE3014);
    }
  } else {  z8zE117 = zZERO_WORD;  }
end_function_3283: ;
  return z8zE117;
end_block_exception_3284: ;

  return u256_zero();
}

sail_u256 zslice_load_n(struct zByteSliceFields zs, uint64_t zoff, uint64_t zn)
{
  sail_u256 z8zE118;
  uint64_t zoffset;
  zoffset = zoff;
  uint64_t zlength;
  zlength = zs.zlen;
  bool z2zE3152;
  z2zE3152 = (zoffset < zlength);
  if (z2zE3152) {  z8zE118 = slice_load_n_word(zs, zoff, zn);  } else {  z8zE118 = zZERO_WORD;  }
end_function_3281: ;
  return z8zE118;
end_block_exception_3282: ;

  return u256_zero();
}

unit zslice_copy(struct zByteSliceFields zs, uint64_t zdst, uint64_t zoff, uint64_t zlen)
{
  unit z8zE119;
  bool z2zE3150;
  z2zE3150 = (zlen != UINT64_C(0));
  if (z2zE3150) {
    uint64_t zoffset;
    zoffset = zoff;
    uint64_t zsource_length;
    zsource_length = zs.zlen;
    bool z2zE3151;
    z2zE3151 = (zoffset < zsource_length);
    if (z2zE3151) {  z8zE119 = slice_copy_to_memory(zs, zdst, zoff, zlen);  } else {
      z8zE119 = slice_copy_to_memory(zEMPTY_SLICE, zdst, UINT64_C(0), zlen);
    }
  } else {  z8zE119 = UNIT;  }
end_function_3279: ;
  return z8zE119;
end_block_exception_3280: ;

  return UNIT;
}

unit zslice_copy_word_offset(struct zByteSliceFields zs, uint64_t zdst, sail_u256 zoff, uint64_t zlen)
{
  unit z8zE120;
  uint64_t zslice_len;
  zslice_len = zs.zlen;
  bool z2zE3149;
  z2zE3149 = u256_lt_u64(zoff, zslice_len);
  if (z2zE3149) {
    {
      uint64_t z3zE3015;
      z3zE3015 = u256_to_u64(zoff);
      z8zE120 = zslice_copy(zs, zdst, z3zE3015, zlen);
    }
  } else {  z8zE120 = zslice_copy(zEMPTY_SLICE, zdst, UINT64_C(0), zlen);  }
end_function_3277: ;
  return z8zE120;
end_block_exception_3278: ;

  return UNIT;
}

