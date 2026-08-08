#pragma once

#include "evmsail/spec/primitives/bytes.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// enum FatalError
enum FatalError { InvalidConfig, HeaderChainBroken, RlpDecode, InvalidSignature, InvalidGasLimit, GasUsedExceedsLimit, BlobGasLimitExceeded, ExecutionInvalid, InvalidGasUsed, InvalidBlobGasUsed, InvalidExcessBlobGas, InvalidStateRoot, InvalidReceiptsRoot, InvalidLogsBloom, InvalidBlockHash, InvalidParentHash, BlockAccessListTooLarge, InvalidBlockAccessList, InvalidExecutionRequests, WitnessDeficient, NumericOverflow };

// enum ExceptionKind
enum ExceptionKind { StackUnderflow, StackOverflow, OutOfGas, InvalidOpcode, InvalidJump, StaticViolation, CallDepthExceeded, InsufficientBalance, WriteProtection, InitCodeTooLarge, NonceOverflow, AddressCollision };

// struct tuple_bool_uint_32_uint_64
struct tuple_bool_uint_32_uint_64 {
  bool tup0;
  uint32_t tup1;
  uint64_t tup2;
};

_Noreturn void fatal_error(enum FatalError _reason);


#ifdef __cplusplus
}
#endif
