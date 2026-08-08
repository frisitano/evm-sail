#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

Bytes freeze_memory_output(Bytes data)
{
  if (data.len == UINT8_C(0)) {
    return EMPTY_OUTPUT_SLICE;
  }
  bool stored = output_buffer_store_memory(data);
  if (stored) {
    return output_buffer_slice_uint32_t_to_Bytes(data.len);
  }
  return EMPTY_OUTPUT_SLICE;
}

Bytes freeze_input_output(Bytes data)
{
  if (data.len == UINT8_C(0)) {
    return EMPTY_OUTPUT_SLICE;
  }
  bool stored = output_buffer_store_input(data);
  if (stored) {
    return output_buffer_slice_uint32_t_to_Bytes(data.len);
  }
  return EMPTY_OUTPUT_SLICE;
}

Bytes freeze_calldata_output(struct CalldataSlice data)
{
  switch (data.kind) {
  case Kind_InputCalldata:
    return freeze_input_output(data.variants.InputCalldata);
  case Kind_MemoryCalldata:
    return freeze_memory_output(data.variants.MemoryCalldata);
  }
}

Bytes output_buffer_word(u256 value)
{
  bool stored = output_buffer_store_word(value);
  if (stored) {
    return output_buffer_slice_uint8_t_to_Bytes(WORD_BYTE_LENGTH);
  }
  return EMPTY_OUTPUT_SLICE;
}

Bytes output_buffer_words(u256 first, u256 second)
{
  bool stored = output_buffer_store_words(first, second);
  if (stored) {
    return output_buffer_slice_uint8_t_to_Bytes(DOUBLE_WORD_BYTE_LENGTH);
  }
  return EMPTY_OUTPUT_SLICE;
}

Bytes output_buffer_slice_uint16_t_to_Bytes(uint16_t len)
{
  return output_buffer_slice((uint32_t)len);
}

Bytes output_buffer_slice_uint32_t_to_Bytes(uint32_t len)
{
  return output_buffer_slice(len);
}

Bytes output_buffer_slice_uint8_t_to_Bytes(uint8_t len)
{
  return output_buffer_slice((uint32_t)len);
}

