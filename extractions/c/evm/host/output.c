/* Generated from sail/host/output.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
struct zByteSliceFields zoutput_buffer_slice(uint64_t zlen)
{
  struct zByteSliceFields z8zE209;
  bool z2zE3262;
  z2zE3262 = (zlen == UINT64_C(0));
  if (z2zE3262) {  z8zE209 = zEMPTY_SLICE;  } else {  z8zE209 = zbyte_slice(zOutputSource, UINT64_C(0), zlen);  }
end_function_3272: ;
  return z8zE209;
end_block_exception_3273: ;
  struct zByteSliceFields z8zE863 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE863;
}

struct zByteSliceFields zfreezze_output(struct zByteSliceFields zdata)
{
  struct zByteSliceFields z8zE210;
  uint64_t zlen;
  zlen = zdata.zlen;
  bool z2zE3260;
  z2zE3260 = (zlen == UINT64_C(0));
  if (z2zE3260) {  z8zE210 = zEMPTY_SLICE;  } else {
    bool z2zE3261;
    z2zE3261 = output_buffer_store(zdata);
    if (z2zE3261) {  z8zE210 = zoutput_buffer_slice(zlen);  } else {  z8zE210 = zEMPTY_SLICE;  }
  }
end_function_3270: ;
  return z8zE210;
end_block_exception_3271: ;
  struct zByteSliceFields z8zE864 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE864;
}

struct zByteSliceFields zoutput_buffer_word(sail_u256 zvalue)
{
  struct zByteSliceFields z8zE211;
  bool z2zE3259;
  z2zE3259 = output_buffer_store_word(zvalue);
  if (z2zE3259) {  z8zE211 = zoutput_buffer_slice(zWORD_BYTE_LENGTH);  } else {  z8zE211 = zEMPTY_SLICE;  }
end_function_3268: ;
  return z8zE211;
end_block_exception_3269: ;
  struct zByteSliceFields z8zE865 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE865;
}

struct zByteSliceFields zoutput_buffer_words(sail_u256 zfirst, sail_u256 zsecond)
{
  struct zByteSliceFields z8zE212;
  bool z2zE3258;
  z2zE3258 = output_buffer_store_words(zfirst, zsecond);
  if (z2zE3258) {  z8zE212 = zoutput_buffer_slice(zDOUBLE_WORD_BYTE_LENGTH);  } else {  z8zE212 = zEMPTY_SLICE;  }
end_function_3266: ;
  return z8zE212;
end_block_exception_3267: ;
  struct zByteSliceFields z8zE866 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE866;
}

