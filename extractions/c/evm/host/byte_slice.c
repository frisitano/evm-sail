/* Generated from sail/host/byte_slice.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
uint64_t zslice_byte(struct zByteSliceFields zs, uint64_t zoff)
{
  uint64_t z8zE114;
  uint64_t zoffset;
  zoffset = zoff;
  uint64_t zlength;
  zlength = zs.zlen;
  bool z2zE3860;
  z2zE3860 = (zoffset < zlength);
  if (z2zE3860) {  z8zE114 = slice_byte_at(zs, zoff);  } else {  z8zE114 = UINT64_C(0x00);  }
end_function_3703: ;
  return z8zE114;
end_block_exception_3704: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zslice_count_nonzzero(struct zByteSliceFields zs)
{
  uint64_t z8zE115;
  z8zE115 = slice_count_nonzero(zs);
end_function_3701: ;
  return z8zE115;
end_block_exception_3702: ;

  return UINT64_C(0xdeadc0de);
}

bool zslice_strided_zzero(struct zByteSliceFields zs, uint64_t zstart, uint64_t zstride, uint64_t zwidth, uint64_t zcount)
{
  bool z8zE116;
  z8zE116 = slice_strided_zero(zs, zstart, zstride, zwidth, zcount);
end_function_3699: ;
  return z8zE116;
end_block_exception_3700: ;

  return false;
}

sail_u256 zslice_load(struct zByteSliceFields zs, uint64_t zoff)
{
  sail_u256 z8zE117;
  uint64_t zoffset;
  zoffset = zoff;
  uint64_t zlength;
  zlength = zs.zlen;
  bool z2zE3859;
  z2zE3859 = (zoffset < zlength);
  if (z2zE3859) {  z8zE117 = slice_load_word(zs, zoff);  } else {  z8zE117 = zZERO_WORD;  }
end_function_3697: ;
  return z8zE117;
end_block_exception_3698: ;

  return u256_zero();
}

sail_u256 zslice_load_word_offset(struct zByteSliceFields zs, sail_u256 zoff)
{
  sail_u256 z8zE118;
  uint64_t zslice_len;
  zslice_len = zs.zlen;
  bool z2zE3858;
  z2zE3858 = u256_lt_u64(zoff, zslice_len);
  if (z2zE3858) {
    {
      uint64_t z3zE3779;
      z3zE3779 = u256_to_u64(zoff);
      z8zE118 = zslice_load(zs, z3zE3779);
    }
  } else {  z8zE118 = zZERO_WORD;  }
end_function_3695: ;
  return z8zE118;
end_block_exception_3696: ;

  return u256_zero();
}

sail_u256 zslice_load_n(struct zByteSliceFields zs, uint64_t zoff, uint64_t zn)
{
  sail_u256 z8zE119;
  uint64_t zoffset;
  zoffset = zoff;
  uint64_t zlength;
  zlength = zs.zlen;
  bool z2zE3857;
  z2zE3857 = (zoffset < zlength);
  if (z2zE3857) {  z8zE119 = slice_load_n_word(zs, zoff, zn);  } else {  z8zE119 = zZERO_WORD;  }
end_function_3693: ;
  return z8zE119;
end_block_exception_3694: ;

  return u256_zero();
}

unit zslice_copy(struct zByteSliceFields zs, uint64_t zdst, uint64_t zoff, uint64_t zlen)
{
  unit z8zE120;
  bool z2zE3855;
  z2zE3855 = (zlen != UINT64_C(0));
  if (z2zE3855) {
    uint64_t zoffset;
    zoffset = zoff;
    uint64_t zsource_length;
    zsource_length = zs.zlen;
    bool z2zE3856;
    z2zE3856 = (zoffset < zsource_length);
    if (z2zE3856) {  z8zE120 = slice_copy_to_memory(zs, zdst, zoff, zlen);  } else {
      z8zE120 = slice_copy_to_memory(zEMPTY_SLICE, zdst, UINT64_C(0), zlen);
    }
  } else {  z8zE120 = UNIT;  }
end_function_3691: ;
  return z8zE120;
end_block_exception_3692: ;

  return UNIT;
}

unit zslice_copy_word_offset(struct zByteSliceFields zs, uint64_t zdst, sail_u256 zoff, uint64_t zlen)
{
  unit z8zE121;
  uint64_t zslice_len;
  zslice_len = zs.zlen;
  bool z2zE3854;
  z2zE3854 = u256_lt_u64(zoff, zslice_len);
  if (z2zE3854) {
    {
      uint64_t z3zE3780;
      z3zE3780 = u256_to_u64(zoff);
      z8zE121 = zslice_copy(zs, zdst, z3zE3780, zlen);
    }
  } else {  z8zE121 = zslice_copy(zEMPTY_SLICE, zdst, UINT64_C(0), zlen);  }
end_function_3689: ;
  return z8zE121;
end_block_exception_3690: ;

  return UNIT;
}

