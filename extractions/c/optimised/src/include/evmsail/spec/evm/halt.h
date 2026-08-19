#pragma once

#include "evmsail/spec/exceptions.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// union HaltKind
enum kind_HaltKind { Kind_HaltReturn, Kind_HaltRevert, Kind_HaltSelfDestruct, Kind_HaltStop };

struct HaltKind {
  enum kind_HaltKind kind;
  union {
    struct { Bytes HaltReturn; };
    struct { Bytes HaltRevert; };
    struct { unit HaltSelfDestruct; };
    struct { unit HaltStop; };
  } variants;
};

// union FrameStatus
enum kind_FrameStatus { Kind_Exceptional, Kind_Halted, Kind_Running };

struct FrameStatus {
  enum kind_FrameStatus kind;
  union {
    struct { enum ExceptionKind Exceptional; };
    struct { struct HaltKind Halted; };
    struct { unit Running; };
  } variants;
};

// struct InterpreterCompletion
struct InterpreterCompletion {
  uint64_t gas_remaining;
  Bytes output;
  __int128 refund;
  uint64_t state_gas_remaining;
  uint32_t state_gas_spilled;
  struct FrameStatus status;
};


#ifdef __cplusplus
}
#endif
