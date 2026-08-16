#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint8_t SSZ_UINT_BYTES = UINT8_C(8);


uint32_t ssz_u32_at(Bytes input, uint32_t offset)
{
  uint64_t byte0 = stateless_input_slice_byte(input, offset);
  uint32_t offset1 = ssz_field_offset_uint32_t_uint8_t_to_uint32_t(offset, UINT8_C(1));
  uint64_t byte1 = stateless_input_slice_byte(input, offset1);
  uint32_t offset2 = ssz_field_offset_uint32_t_uint8_t_to_uint32_t(offset, UINT8_C(2));
  uint64_t byte2 = stateless_input_slice_byte(input, offset2);
  uint32_t offset3 = ssz_field_offset_uint32_t_uint8_t_to_uint32_t(offset, UINT8_C(3));
  uint64_t byte3 = stateless_input_slice_byte(input, offset3);
  return (uint32_t)(byte0 | (((byte1 << UINT64_C(8)) & UINT64_C(0xFFFFFFFF)) | (((byte2 << UINT64_C(16)) & UINT64_C(0xFFFFFFFF)) | ((byte3 << UINT64_C(24)) & UINT64_C(0xFFFFFFFF)))));
}

uint32_t ssz_u32_in_slice(Bytes input, uint32_t offset)
{
  if ((offset <= input.len) && (UINT8_C(4) <= ((int64_t)input.len - (int64_t)offset))) {
    return ssz_u32_at(input, offset);
  }
  fatal_error(InvalidConfig);
}

uint32_t ssz_offset_to_source_pointer(uint32_t value)
{
  return value;
}

uint64_t decode_ssz_uint(Bytes input, uint32_t offset)
{
  uint64_t byte0 = stateless_input_slice_byte(input, offset);
  uint32_t offset1 = ssz_field_offset_uint32_t_uint8_t_to_uint32_t(offset, UINT8_C(1));
  uint64_t byte1 = stateless_input_slice_byte(input, offset1);
  uint32_t offset2 = ssz_field_offset_uint32_t_uint8_t_to_uint32_t(offset, UINT8_C(2));
  uint64_t byte2 = stateless_input_slice_byte(input, offset2);
  uint32_t offset3 = ssz_field_offset_uint32_t_uint8_t_to_uint32_t(offset, UINT8_C(3));
  uint64_t byte3 = stateless_input_slice_byte(input, offset3);
  uint32_t offset4 = ssz_field_offset_uint32_t_uint8_t_to_uint32_t(offset, UINT8_C(4));
  uint64_t byte4 = stateless_input_slice_byte(input, offset4);
  uint32_t offset5 = ssz_field_offset_uint32_t_uint8_t_to_uint32_t(offset, UINT8_C(5));
  uint64_t byte5 = stateless_input_slice_byte(input, offset5);
  uint32_t offset6 = ssz_field_offset_uint32_t_uint8_t_to_uint32_t(offset, UINT8_C(6));
  uint64_t byte6 = stateless_input_slice_byte(input, offset6);
  uint32_t offset7 = ssz_field_offset_uint32_t_uint8_t_to_uint32_t(offset, UINT8_C(7));
  uint64_t byte7 = stateless_input_slice_byte(input, offset7);
  return ((UINT64_C(72057594037927936) * (uint64_t)(uint8_t)byte7) + ((UINT64_C(281474976710656) * (uint64_t)(uint8_t)byte6) + ((UINT64_C(1099511627776) * (uint64_t)(uint8_t)byte5) + ((UINT64_C(4294967296) * (uint64_t)(uint8_t)byte4) + (((uint64_t)UINT32_C(16777216) * (uint64_t)(uint32_t)(uint8_t)byte3) + (((uint64_t)UINT32_C(65536) * (uint64_t)(uint32_t)(uint8_t)byte2) + (uint64_t)(((uint32_t)UINT16_C(256) * (uint32_t)(uint16_t)(uint8_t)byte1) + (uint32_t)(uint16_t)(uint8_t)byte0)))))));
}

uint64_t decode_ssz_uint_Bytes_uint16_t_to_uint64_t(Bytes input, uint16_t offset)
{
  uint64_t byte0 = stateless_input_slice_byte_Bytes_uint16_t_to_uint64_t(input, offset);
  uint32_t offset1 = ssz_field_offset_uint16_t_uint8_t_to_uint32_t(offset, UINT8_C(1));
  uint64_t byte1 = stateless_input_slice_byte(input, offset1);
  uint32_t offset2 = ssz_field_offset_uint16_t_uint8_t_to_uint32_t(offset, UINT8_C(2));
  uint64_t byte2 = stateless_input_slice_byte(input, offset2);
  uint32_t offset3 = ssz_field_offset_uint16_t_uint8_t_to_uint32_t(offset, UINT8_C(3));
  uint64_t byte3 = stateless_input_slice_byte(input, offset3);
  uint32_t offset4 = ssz_field_offset_uint16_t_uint8_t_to_uint32_t(offset, UINT8_C(4));
  uint64_t byte4 = stateless_input_slice_byte(input, offset4);
  uint32_t offset5 = ssz_field_offset_uint16_t_uint8_t_to_uint32_t(offset, UINT8_C(5));
  uint64_t byte5 = stateless_input_slice_byte(input, offset5);
  uint32_t offset6 = ssz_field_offset_uint16_t_uint8_t_to_uint32_t(offset, UINT8_C(6));
  uint64_t byte6 = stateless_input_slice_byte(input, offset6);
  uint32_t offset7 = ssz_field_offset_uint16_t_uint8_t_to_uint32_t(offset, UINT8_C(7));
  uint64_t byte7 = stateless_input_slice_byte(input, offset7);
  return ((UINT64_C(72057594037927936) * (uint64_t)(uint8_t)byte7) + ((UINT64_C(281474976710656) * (uint64_t)(uint8_t)byte6) + ((UINT64_C(1099511627776) * (uint64_t)(uint8_t)byte5) + ((UINT64_C(4294967296) * (uint64_t)(uint8_t)byte4) + (((uint64_t)UINT32_C(16777216) * (uint64_t)(uint32_t)(uint8_t)byte3) + (((uint64_t)UINT32_C(65536) * (uint64_t)(uint32_t)(uint8_t)byte2) + (uint64_t)(((uint32_t)UINT16_C(256) * (uint32_t)(uint16_t)(uint8_t)byte1) + (uint32_t)(uint16_t)(uint8_t)byte0)))))));
}

uint64_t decode_ssz_uint_Bytes_uint8_t_to_uint64_t(Bytes input, uint8_t offset)
{
  uint64_t byte0 = stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(input, offset);
  uint32_t offset1 = ssz_field_offset_uint8_t_uint8_t_to_uint32_t(offset, UINT8_C(1));
  uint64_t byte1 = stateless_input_slice_byte(input, offset1);
  uint32_t offset2 = ssz_field_offset_uint8_t_uint8_t_to_uint32_t(offset, UINT8_C(2));
  uint64_t byte2 = stateless_input_slice_byte(input, offset2);
  uint32_t offset3 = ssz_field_offset_uint8_t_uint8_t_to_uint32_t(offset, UINT8_C(3));
  uint64_t byte3 = stateless_input_slice_byte(input, offset3);
  uint32_t offset4 = ssz_field_offset_uint8_t_uint8_t_to_uint32_t(offset, UINT8_C(4));
  uint64_t byte4 = stateless_input_slice_byte(input, offset4);
  uint32_t offset5 = ssz_field_offset_uint8_t_uint8_t_to_uint32_t(offset, UINT8_C(5));
  uint64_t byte5 = stateless_input_slice_byte(input, offset5);
  uint32_t offset6 = ssz_field_offset_uint8_t_uint8_t_to_uint32_t(offset, UINT8_C(6));
  uint64_t byte6 = stateless_input_slice_byte(input, offset6);
  uint32_t offset7 = ssz_field_offset_uint8_t_uint8_t_to_uint32_t(offset, UINT8_C(7));
  uint64_t byte7 = stateless_input_slice_byte(input, offset7);
  return ((UINT64_C(72057594037927936) * (uint64_t)(uint8_t)byte7) + ((UINT64_C(281474976710656) * (uint64_t)(uint8_t)byte6) + ((UINT64_C(1099511627776) * (uint64_t)(uint8_t)byte5) + ((UINT64_C(4294967296) * (uint64_t)(uint8_t)byte4) + (((uint64_t)UINT32_C(16777216) * (uint64_t)(uint32_t)(uint8_t)byte3) + (((uint64_t)UINT32_C(65536) * (uint64_t)(uint32_t)(uint8_t)byte2) + (uint64_t)(((uint32_t)UINT16_C(256) * (uint32_t)(uint16_t)(uint8_t)byte1) + (uint32_t)(uint16_t)(uint8_t)byte0)))))));
}

bytes20 ssz_addr(Bytes input, uint8_t offset)
{
  u256 value = stateless_input_slice_load_n_Bytes_uint8_t_uint8_t_to_u256(input, offset, ADDRESS_BYTE_LENGTH);
  return word_to_address(value);
}

bytes32 ssz_bytes32_Bytes_uint16_t_to_bytes32(Bytes input, uint16_t offset)
{
  u256 value = stateless_input_slice_load_Bytes_uint16_t_to_u256(input, offset);
  return word_to_hash(value);
}

bytes32 ssz_bytes32_Bytes_uint8_t_to_bytes32(Bytes input, uint8_t offset)
{
  u256 value = stateless_input_slice_load_Bytes_uint8_t_to_u256(input, offset);
  return word_to_hash(value);
}

uint32_t ssz_field_offset_uint16_t_uint8_t_to_uint32_t(uint16_t base, uint8_t delta)
{
  return ((uint32_t)base + (uint32_t)(uint16_t)delta);
}

uint32_t ssz_field_offset_uint32_t_uint8_t_to_uint32_t(uint32_t base, uint8_t delta)
{
  return (base + (uint32_t)delta);
}

uint32_t ssz_field_offset_uint8_t_uint8_t_to_uint32_t(uint8_t base, uint8_t delta)
{
  return ((uint32_t)delta + (uint32_t)base);
}

uint8_t ssz_field_offset_uint8_t_uint8_t_to_uint8_t(uint8_t base, uint8_t delta)
{
  return ((uint8_t)((uint32_t)delta + (uint32_t)base));
}

u256 ssz_u256_(Bytes input, uint16_t offset)
{
  return ssz_u256(input, (uint32_t)offset);
}

uint32_t ssz_u32(Bytes input, uint8_t offset)
{
  return ssz_u32_at_Bytes_uint8_t_to_uint32_t(input, offset);
}

uint32_t ssz_u32_at_Bytes_uint8_t_to_uint32_t(Bytes input, uint8_t offset)
{
  uint64_t byte0 = stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(input, offset);
  uint32_t offset1 = ssz_field_offset_uint8_t_uint8_t_to_uint32_t(offset, UINT8_C(1));
  uint64_t byte1 = stateless_input_slice_byte(input, offset1);
  uint32_t offset2 = ssz_field_offset_uint8_t_uint8_t_to_uint32_t(offset, UINT8_C(2));
  uint64_t byte2 = stateless_input_slice_byte(input, offset2);
  uint32_t offset3 = ssz_field_offset_uint8_t_uint8_t_to_uint32_t(offset, UINT8_C(3));
  uint64_t byte3 = stateless_input_slice_byte(input, offset3);
  return (uint32_t)(byte0 | (((byte1 << UINT64_C(8)) & UINT64_C(0xFFFFFFFF)) | (((byte2 << UINT64_C(16)) & UINT64_C(0xFFFFFFFF)) | ((byte3 << UINT64_C(24)) & UINT64_C(0xFFFFFFFF)))));
}

