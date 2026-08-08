#pragma once

#include "evmsail/spec/primitives/ssz.h"

#include "evmsail/spec/abi.h"
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

void create_letbind_33(void);
void kill_letbind_33(void);


bool deep_stack_immediate_valid(uint64_t immediate);

bool exchange_immediate_valid(uint64_t immediate);

struct CodeFields analyzed_code(Bytes bytes, uint8_t * jumpdests);

Bytes code_bytes(struct CodeFields code);

void create_letbind_34(void);
void kill_letbind_34(void);


struct CodeFields code_db_resolve_indexed(bytes32 /* arg_0 */);

extern uint8_t * const EMPTY_JUMP_TABLE;


extern Bytes EMPTY_CODE_SLICE;


extern struct CodeFields EMPTY_CODE;



#ifdef __cplusplus
}
#endif
