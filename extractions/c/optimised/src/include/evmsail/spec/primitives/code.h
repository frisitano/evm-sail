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

struct InterpreterCompletion threaded_interpret(uint64_t /* arg_0 */, uint64_t /* arg_1 */, uint32_t /* arg_2 */, __int128 /* arg_3 */, StackPointer /* arg_4 */, uint32_t /* arg_5 */, uint32_t /* arg_6 */, bytes20 /* arg_7 */, bytes20 /* arg_8 */, bytes20 /* arg_9 */, u256 /* arg_10 */, uint64_t /* arg_11 */, bool /* arg_12 */, uint16_t /* arg_13 */, struct CodeFields /* arg_14 */, struct CalldataSlice /* arg_15 */, uint8_t /* arg_16 */, u256 /* arg_17 */);

extern uint8_t * const EMPTY_JUMP_TABLE;


extern Bytes EMPTY_CODE_SLICE;


extern struct CodeFields EMPTY_CODE;



#ifdef __cplusplus
}
#endif
