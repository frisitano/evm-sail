#pragma once

#include "evmsail/spec/prelude.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// enum PrecompileId
enum PrecompileId { NotPrecompile, Ecrecover, Sha256, Ripemd160, Identity, Modexp, Bn254Add, Bn254Mul, Bn254Pairing, Blake2f, KzgPointEvaluation, BlsG1Add, BlsG1Msm, BlsG2Add, BlsG2Msm, BlsPairing, BlsMapFpToG1, BlsMapFp2ToG2, P256Verify };

// struct MemoryRangeFields
struct MemoryRangeFields {
  uint32_t len;
  uint32_t off;
};

// struct MemoryAccessFields
struct MemoryAccessFields {
  struct MemoryRangeFields range;
  uint32_t required_size;
};

bool neq_anything_EPrecompileId__(enum PrecompileId x, enum PrecompileId y);

uint64_t word_of_account_nonce(uint64_t value);

uint64_t word_of_withdrawal_amount(uint64_t value);

uint64_t word_of_slot_number(uint64_t value);

uint64_t word_of_block_number(uint64_t value);

uint64_t word_of_block_timestamp(uint64_t value);

uint64_t word_of_chain_identifier(uint64_t value);

struct MemoryRangeFields memory_range(uint32_t off, uint32_t len);

void create_letbind_7(void);
void kill_letbind_7(void);


void create_letbind_8(void);
void kill_letbind_8(void);


u256 word_of_source_byte_count(uint32_t value);

struct MemoryRangeFields memory_range_uint8_t_uint8_t_to_struct_MemoryRangeFields(uint8_t off, uint8_t len);

u256 word_of_nat_byte_count_u128_to_u256(u128 value);

u256 word_of_nat_byte_count_uint16_t_to_u256(uint16_t value);

u256 word_of_nat_byte_count_uint32_t_to_u256(uint32_t value);

u256 word_of_nat_byte_count_uint64_t_to_u256(uint64_t value);

u256 word_of_nat_byte_count_uint8_t_to_u256(uint8_t value);

extern struct MemoryRangeFields EMPTY_MEMORY_RANGE;


extern struct MemoryAccessFields EMPTY_MEMORY_ACCESS;



#ifdef __cplusplus
}
#endif
