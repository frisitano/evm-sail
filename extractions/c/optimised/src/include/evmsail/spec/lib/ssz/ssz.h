#pragma once

#include "evmsail/spec/host/nodes.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

uint32_t ssz_u32_at(Bytes input, uint32_t offset);

uint32_t ssz_u32_in_slice(Bytes input, uint32_t offset);

uint32_t ssz_offset_to_source_pointer(uint32_t value);

uint64_t decode_ssz_uint(Bytes input, uint32_t offset);

uint64_t decode_ssz_uint_Bytes_uint16_t_to_uint64_t(Bytes input, uint16_t offset);

uint64_t decode_ssz_uint_Bytes_uint8_t_to_uint64_t(Bytes input, uint8_t offset);

bytes20 ssz_addr(Bytes input, uint8_t offset);

bytes32 ssz_bytes32_Bytes_uint16_t_to_bytes32(Bytes input, uint16_t offset);

bytes32 ssz_bytes32_Bytes_uint8_t_to_bytes32(Bytes input, uint8_t offset);

uint32_t ssz_field_offset_uint16_t_uint8_t_to_uint32_t(uint16_t base, uint8_t delta);

uint32_t ssz_field_offset_uint32_t_uint8_t_to_uint32_t(uint32_t base, uint8_t delta);

uint32_t ssz_field_offset_uint8_t_uint8_t_to_uint32_t(uint8_t base, uint8_t delta);

uint8_t ssz_field_offset_uint8_t_uint8_t_to_uint8_t(uint8_t base, uint8_t delta);

u256 ssz_u256_(Bytes input, uint16_t offset);

uint32_t ssz_u32(Bytes input, uint8_t offset);

uint32_t ssz_u32_at_Bytes_uint8_t_to_uint32_t(Bytes input, uint8_t offset);

extern const uint8_t SSZ_UINT_BYTES;



#ifdef __cplusplus
}
#endif
