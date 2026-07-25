/* Generated from sail/host/output.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
struct zByteSliceFields zoutput_buffer_slice(uint64_t zlen)
{
  struct zByteSliceFields z8zE205;
  bool z2zE3238;
  z2zE3238 = (zlen == UINT64_C(0));
  if (z2zE3238) {  z8zE205 = zEMPTY_SLICE;  } else {  z8zE205 = zbyte_slice(zOutputSource, UINT64_C(0), zlen);  }
end_function_3267: ;
  return z8zE205;
end_block_exception_3268: ;
  struct zByteSliceFields z8zE853 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE853;
}

struct zByteSliceFields zfreezze_output(struct zByteSliceFields zdata)
{
  struct zByteSliceFields z8zE206;
  uint64_t zlen;
  zlen = zdata.zlen;
  bool z2zE3236;
  z2zE3236 = (zlen == UINT64_C(0));
  if (z2zE3236) {  z8zE206 = zEMPTY_SLICE;  } else {
    bool z2zE3237;
    z2zE3237 = output_buffer_store(zdata);
    if (z2zE3237) {  z8zE206 = zoutput_buffer_slice(zlen);  } else {  z8zE206 = zEMPTY_SLICE;  }
  }
end_function_3265: ;
  return z8zE206;
end_block_exception_3266: ;
  struct zByteSliceFields z8zE854 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE854;
}

struct zByteSliceFields zoutput_buffer_word(sail_u256 zvalue)
{
  struct zByteSliceFields z8zE207;
  bool z2zE3235;
  z2zE3235 = output_buffer_store_word(zvalue);
  if (z2zE3235) {  z8zE207 = zoutput_buffer_slice(zWORD_BYTE_LENGTH);  } else {  z8zE207 = zEMPTY_SLICE;  }
end_function_3263: ;
  return z8zE207;
end_block_exception_3264: ;
  struct zByteSliceFields z8zE855 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE855;
}

struct zByteSliceFields zoutput_buffer_words(sail_u256 zfirst, sail_u256 zsecond)
{
  struct zByteSliceFields z8zE208;
  bool z2zE3234;
  z2zE3234 = output_buffer_store_words(zfirst, zsecond);
  if (z2zE3234) {  z8zE208 = zoutput_buffer_slice(zDOUBLE_WORD_BYTE_LENGTH);  } else {  z8zE208 = zEMPTY_SLICE;  }
end_function_3261: ;
  return z8zE208;
end_block_exception_3262: ;
  struct zByteSliceFields z8zE856 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE856;
}

