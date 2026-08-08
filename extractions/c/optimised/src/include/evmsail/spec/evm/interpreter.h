#pragma once

#include "evmsail/spec/evm/execute.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct CreateSemantics
struct CreateSemantics {bool uses_salt;};

// struct CallSemantics
struct CallSemantics {
  bool enters_static_context;
  bool inherits_caller_and_value;
  bool takes_value;
  bool transfers_value;
  bool uses_target_address;
};

// struct tuple_uint_8_bits_64_Bytes_uint_64
struct tuple_uint_8_bits_64_Bytes_uint_64 {
  uint8_t tup0;
  uint64_t tup1;
  Bytes tup2;
  uint64_t tup3;
};

struct CreateSemantics create_semantics(enum CreateKind kind);

struct CallSemantics call_semantics(enum CallKind kind);

struct CodeFields executable_code(bytes20 target, bool dele, bytes20 dtgt);

Bytes interpret_(void);

bool frame_succeeded(void);

void resume_call(struct CallContinuation continuation, Bytes output);

void resume_create(struct CreateContinuation continuation, Bytes output);

void resume_frame(struct FrameContinuation continuation, Bytes output);


#ifdef __cplusplus
}
#endif
