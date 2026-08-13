#pragma once

#include "evmsail/spec/primitives/ssz.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// enum DeepStackOperation
enum DeepStackOperation { DeepStackDuplicate, DeepStackSwap, DeepStackExchange, NotDeepStackOperation };

// struct CodeFields
struct CodeFields {
  const uint8_t * bytes;
  uint8_t * jumpdests;
  uint32_t len;
};

// struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes
struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes {
  uint32_t tup0;
  uint64_t tup1;
  bytes20 tup10;
  u256 tup11;
  uint64_t tup12;
  bool tup13;
  uint16_t tup14;
  struct CodeFields tup15;
  struct CalldataSlice tup16;
  Bytes tup17;
  uint64_t tup2;
  uint32_t tup3;
  __int128 tup4;
  struct FrameStatus tup5;
  StackPointer tup6;
  Bytes tup7;
  bytes20 tup8;
  bytes20 tup9;
};

// struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes
struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes {
  uint64_t tup0;
  uint64_t tup1;
  uint32_t tup2;
  __int128 tup3;
  struct FrameStatus tup4;
  Bytes tup5;
};

Bytes code_slice(Bytes bytes);

Bytes validated_code_slice(Bytes bytes);

void create_letbind_34(void);
void kill_letbind_34(void);


enum DeepStackOperation deep_stack_operation(uint8_t opcode);

bool deep_stack_immediate_valid(uint64_t immediate);

bool exchange_immediate_valid(uint64_t immediate);

bool deep_stack_operation_immediate_valid(enum DeepStackOperation operation, uint64_t immediate);

struct CodeFields analyzed_code(Bytes bytes, uint8_t * jumpdests);

Bytes code_bytes(struct CodeFields code);

void create_letbind_35(void);
void kill_letbind_35(void);


struct CodeFields code_db_resolve_indexed(bytes32 /* arg_0 */);

struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes threaded_interpret(uint64_t /* arg_0 */, uint64_t /* arg_1 */, uint32_t /* arg_2 */, __int128 /* arg_3 */, StackPointer /* arg_4 */, Bytes /* arg_5 */, bytes20 /* arg_6 */, bytes20 /* arg_7 */, bytes20 /* arg_8 */, u256 /* arg_9 */, uint64_t /* arg_10 */, bool /* arg_11 */, uint16_t /* arg_12 */, struct CodeFields /* arg_13 */, struct CalldataSlice /* arg_14 */, uint8_t /* arg_15 */, u256 /* arg_16 */);

extern uint8_t * const EMPTY_JUMP_TABLE;


extern Bytes EMPTY_CODE_SLICE;


extern struct CodeFields EMPTY_CODE;



#ifdef __cplusplus
}
#endif
