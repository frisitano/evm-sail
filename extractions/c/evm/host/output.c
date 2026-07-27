/* Generated from sail/host/output.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
struct zByteSliceFields zoutput_buffer_slice(uint64_t zlen)
{
  struct zByteSliceFields z8zE202;
  bool z2zE2539;
  z2zE2539 = (zlen == UINT64_C(0));
  if (z2zE2539) {  z8zE202 = zEMPTY_SLICE;  } else {  z8zE202 = zbyte_slice(zOutputSource, UINT64_C(0), zlen);  }
end_function_2863: ;
  return z8zE202;
end_block_exception_2864: ;
  struct zByteSliceFields z8zE800 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE800;
}

struct zByteSliceFields zfreezze_output(struct zByteSliceFields zdata)
{
  struct zByteSliceFields z8zE203;
  uint64_t zlen;
  zlen = zdata.zlen;
  bool z2zE2537;
  z2zE2537 = (zlen == UINT64_C(0));
  if (z2zE2537) {  z8zE203 = zEMPTY_SLICE;  } else {
    bool z2zE2538;
    z2zE2538 = output_buffer_store(zdata);
    if (z2zE2538) {  z8zE203 = zoutput_buffer_slice(zlen);  } else {  z8zE203 = zEMPTY_SLICE;  }
  }
end_function_2861: ;
  return z8zE203;
end_block_exception_2862: ;
  struct zByteSliceFields z8zE801 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE801;
}

struct zByteSliceFields zoutput_buffer_word(sail_u256 zvalue)
{
  struct zByteSliceFields z8zE204;
  bool z2zE2536;
  z2zE2536 = output_buffer_store_word(zvalue);
  if (z2zE2536) {  z8zE204 = zoutput_buffer_slice(zWORD_BYTE_LENGTH);  } else {  z8zE204 = zEMPTY_SLICE;  }
end_function_2859: ;
  return z8zE204;
end_block_exception_2860: ;
  struct zByteSliceFields z8zE802 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE802;
}

struct zByteSliceFields zoutput_buffer_words(sail_u256 zfirst, sail_u256 zsecond)
{
  struct zByteSliceFields z8zE205;
  bool z2zE2535;
  z2zE2535 = output_buffer_store_words(zfirst, zsecond);
  if (z2zE2535) {  z8zE205 = zoutput_buffer_slice(zDOUBLE_WORD_BYTE_LENGTH);  } else {  z8zE205 = zEMPTY_SLICE;  }
end_function_2857: ;
  return z8zE205;
end_block_exception_2858: ;
  struct zByteSliceFields z8zE803 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE803;
}

