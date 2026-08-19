#pragma once

#include "evmsail/spec/primitives/gas.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// union LogData
enum kind_LogData { Kind_LogDataMemory, Kind_LogDataWord };

struct LogData {
  enum kind_LogData kind;
  union {
    struct { Bytes LogDataMemory; };
    struct { u256 LogDataWord; };
  } variants;
};

// union CalldataSlice
enum kind_CalldataSlice { Kind_InputCalldata, Kind_MemoryCalldata };

struct CalldataSlice {
  enum kind_CalldataSlice kind;
  union {
    struct { Bytes InputCalldata; };
    struct { Bytes MemoryCalldata; };
  } variants;
};

uint32_t calldata_slice_length(struct CalldataSlice s);

uint32_t stateless_input_slice_length(Bytes s);

void create_letbind_26(void);
void kill_letbind_26(void);


void create_letbind_27(void);
void kill_letbind_27(void);


void create_letbind_28(void);
void kill_letbind_28(void);


void create_letbind_29(void);
void kill_letbind_29(void);


void create_letbind_31(void);
void kill_letbind_31(void);


void create_letbind_32(void);
void kill_letbind_32(void);


Bytes stateless_input_sub_slice(Bytes s, uint32_t off, uint32_t len);

Bytes scratch_sub_slice(Bytes s, uint32_t off, uint32_t len);

Bytes memory_sub_slice(Bytes s, uint32_t off, uint32_t len);

Bytes stateless_input_slice_suffix(Bytes s, uint32_t off);

Bytes scratch_slice_suffix(Bytes s, uint32_t off);

uint8_t * code_db_analyze_indexed(Bytes /* arg_0 */, bool /* arg_1 */);

u256 ssz_u256(Bytes /* arg_0 */, uint32_t /* arg_1 */);

Bytes output_buffer_slice(uint32_t /* arg_0 */);

Bytes host_log_data_slice(uint64_t /* arg_0 */);

Bytes scratch_store_receipt_logs_bloom(uint32_t /* arg_0 */, uint64_t /* arg_1 */, uint64_t /* arg_2 */);

void receipt_table_push(uint32_t /* arg_0 */, Bytes /* arg_1 */);

bool block_logs_bloom_matches(Bytes /* arg_0 */);

void validate_block_access_list(Bytes /* arg_0 */, uint64_t /* arg_1 */);

void initialize_block_access_list_state(Bytes /* arg_0 */, bytes32 /* arg_1 */, uint32_t /* arg_2 */);

void initialize_block_access_list_state_(Bytes bytes, bytes32 parent_state_root, uint32_t transaction_count);

struct CalldataSlice calldata_sub_slice(struct CalldataSlice s, uint8_t off, uint8_t len);

Bytes evm_memory_slice(const uint8_t * off, uint32_t len);

Bytes log_data_sub_slice_Bytes_uint16_t_uint8_t_to_Bytes(Bytes s, uint16_t off, uint8_t len);

Bytes log_data_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(Bytes s, uint8_t off, uint8_t len);

Bytes memory_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(Bytes s, uint8_t off, uint8_t len);

Bytes scratch_slice_suffix_Bytes_uint8_t_to_Bytes(Bytes s, uint8_t off);

Bytes scratch_sub_slice_Bytes_uint8_t_uint32_t_to_Bytes(Bytes s, uint8_t off, uint32_t len);

Bytes scratch_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(Bytes s, uint8_t off, uint8_t len);

Bytes stateless_input_slice(const uint8_t * off, uint32_t len);

Bytes stateless_input_slice_suffix_Bytes_uint8_t_to_Bytes(Bytes s, uint8_t off);

Bytes stateless_input_sub_slice_Bytes_uint8_t_uint16_t_to_Bytes(Bytes s, uint8_t off, uint16_t len);

Bytes stateless_input_sub_slice_Bytes_uint8_t_uint32_t_to_Bytes(Bytes s, uint8_t off, uint32_t len);

Bytes stateless_input_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(Bytes s, uint8_t off, uint8_t len);

extern const uint8_t ADDRESS_BYTE_LENGTH;


extern const uint8_t WORD_BYTE_LENGTH;


extern const uint8_t EIGHT_BYTE_LENGTH;


extern const uint8_t DOUBLE_WORD_BYTE_LENGTH;


extern Bytes EMPTY_STATELESS_INPUT_SLICE;


extern Bytes EMPTY_SCRATCH_SLICE;


extern Bytes EMPTY_EVM_MEMORY_SLICE;


extern Bytes EMPTY_CODE_REGION_SLICE;


extern Bytes EMPTY_OUTPUT_SLICE;


extern struct CalldataSlice EMPTY_CALLDATA;



#ifdef __cplusplus
}
#endif
