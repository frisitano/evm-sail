#pragma once

#include "evmsail/spec/primitives/block.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct TxFrameGasSnapshotFields
struct TxFrameGasSnapshotFields {
  uint64_t admitted_limit;
  uint64_t calldata_floor;
  uint64_t regular_limit;
  uint64_t remaining;
  uint64_t state_used;
};

// struct TxFrameResultFields
struct TxFrameResultFields {
  struct TxFrameGasSnapshotFields gas;
  __int128 refund;
  bool success;
};

// struct TransactionInitialGasFields
struct TransactionInitialGasFields {
  uint64_t admitted_limit;
  uint64_t calldata_floor;
  uint64_t execution_remaining;
  uint64_t intrinsic_execution;
  uint64_t intrinsic_state;
  uint64_t regular_limit;
  uint64_t state_remaining;
};

// struct TxValidityFields
struct TxValidityFields {
  u256 blob_fee;
  struct TransactionInitialGasFields gas;
  u256 gas_price;
  uint64_t nonce_before;
  u256 priority_fee;
  bytes20 sender;
};

// struct TransactionGasAllowanceFields
struct TransactionGasAllowanceFields {
  uint64_t regular;
  uint64_t total;
};

// struct Message
struct Message {
  bytes20 address;
  bytes20 caller;
  bytes20 code_address;
  uint16_t depth;
  bool is_static;
  uint64_t state_gas_reservoir;
  u256 value;
};

// union OpcodeOutcome
enum kind_OpcodeOutcome { Kind_Continue, Kind_Failed };

struct OpcodeOutcome {
  enum kind_OpcodeOutcome kind;
  union {
    struct { unit Continue; };
    struct { enum ExceptionKind Failed; };
  } variants;
};

// struct ExceptionalStateTransition
struct ExceptionalStateTransition {
  uint64_t state_gas_remaining;
  uint32_t state_gas_spilled;
  struct FrameStatus status;
};

// enum CreateKind
enum CreateKind { CreateByNonce, CreateBySalt };

// struct FrameTransition
struct FrameTransition {
  struct CalldataSlice calldata;
  struct CodeFields code;
  uint64_t gas_remaining;
  uint32_t memory_base;
  uint32_t memory_height;
  struct Message message;
  uint32_t pc;
  __int128 refund;
  Bytes returndata;
  StackPointer stack_top;
  uint64_t state_gas_remaining;
  uint32_t state_gas_spilled;
  struct FrameStatus status;
};

// struct FrameCheckpoint
struct FrameCheckpoint {
  struct CalldataSlice calldata;
  struct CodeFields code;
  uint64_t gas_remaining;
  uint32_t memory_height;
  struct Message message;
  uint32_t pc;
  __int128 refund;
  StackPointer stack_top;
  uint64_t state_gas_remaining;
  uint32_t state_gas_spilled;
  struct FrameStatus status;
};

// struct CreateContinuation
struct CreateContinuation {
  bytes20 address;
  struct FrameCheckpoint checkpoint;
  bool new_account_charged;
};

// enum CallKind
enum CallKind { Call, CallCode, DelegateCall, StaticCall };

// struct CallContinuation
struct CallContinuation {
  struct FrameCheckpoint checkpoint;
  bool new_account_charged;
  uint32_t return_length;
  uint32_t return_offset;
};

// union FrameContinuation
enum kind_FrameContinuation { Kind_Empty, Kind_ResumeCall, Kind_ResumeCreate };

struct FrameContinuation {
  enum kind_FrameContinuation kind;
  union {
    struct { unit Empty; };
    struct { struct CallContinuation ResumeCall; };
    struct { struct CreateContinuation ResumeCreate; };
  } variants;
};

// struct TxEnvFields
struct TxEnvFields {
  struct BlobHashesFields blob_hashes;
  u256 gas_price;
  bytes20 origin;
};

struct TxEnvFields tx_env(bytes20 origin, u256 gas_price, struct BlobHashesFields blob_hashes);

struct TransactionInitialGasFields transaction_initial_gas_fields(uint64_t total, uint64_t regular, uint64_t intrinsic_execution, uint64_t intrinsic_state, uint64_t calldata_floor, uint64_t execution, uint64_t state);

struct TxValidityFields tx_validity_fields(bytes20 sender, uint64_t nonce_before, struct TransactionInitialGasFields gas, u256 blob_fee, u256 gas_price, u256 priority_fee);

struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_fields(uint64_t limit, uint64_t regular, uint64_t calldata_floor, uint64_t remaining, uint64_t state_used);

struct TransactionInitialGasFields transaction_initial_gas_fields_uint64_t_uint64_t_uint64_t_uint64_t_uint64_t_uint64_t_uint8_t_to_struct_TransactionInitialGasFields(uint64_t total, uint64_t regular, uint64_t intrinsic_execution, uint64_t intrinsic_state, uint64_t calldata_floor, uint64_t execution, uint8_t state);

struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_fields_uint64_t_uint64_t_uint64_t_uint64_t_uint8_t_to_struct_TxFrameGasSnapshotFields(uint64_t limit, uint64_t regular, uint64_t calldata_floor, uint64_t remaining, uint8_t state_used);


#ifdef __cplusplus
}
#endif
