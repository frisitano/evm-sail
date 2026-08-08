#pragma once

#include "evmsail/spec/evm/instructions.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct tuple_uint_32_uint_64
struct tuple_uint_32_uint_64 {
  uint32_t tup0;
  uint64_t tup1;
};

// struct tuple_LogTopics_bits_64
struct tuple_LogTopics_bits_64 {
  struct LogTopics tup0;
  uint64_t tup1;
};

// struct tuple_bits_64_uint_64
struct tuple_bits_64_uint_64 {
  uint64_t tup0;
  uint64_t tup1;
};

// struct tuple_bits_64_Bytes_uint_64
struct tuple_bits_64_Bytes_uint_64 {
  uint64_t tup0;
  Bytes tup1;
  uint64_t tup2;
};

// struct tuple_uint_32_bits_64_uint_64
struct tuple_uint_32_bits_64_uint_64 {
  uint32_t tup0;
  uint64_t tup1;
  uint64_t tup2;
};

// struct tuple_uint_32_bits_64_Bytes_uint_64
struct tuple_uint_32_bits_64_Bytes_uint_64 {
  uint32_t tup0;
  uint64_t tup1;
  Bytes tup2;
  uint64_t tup3;
};

bytes20 self_addr(void);

struct tuple_bool_uint_64 guard_static(uint64_t g);

struct tuple_uint_32_uint_64 do_jump(uint32_t pc_in, uint64_t g, u256 destination_value);

struct tuple_LogTopics_bits_64 pop_log_topics(uint8_t count, uint64_t top);

struct tuple_bits_64_uint_64 execute_add(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_mul(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_sub(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_div(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_sdiv(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_mod(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_smod(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_addmod(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_mulmod(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_exp(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_signextend(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_lt(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_gt(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_slt(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_sgt(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_eq(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_iszero(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_and(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_or(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_xor(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_not(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_byte(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_shl(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_shr(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_sar(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_clz(uint64_t top, uint64_t g);

struct tuple_bits_64_Bytes_uint_64 execute_keccak256(uint64_t top, Bytes mem, uint64_t g);

struct tuple_bits_64_uint_64 execute_address(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_origin(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_caller(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_callvalue(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_gasprice(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_calldatasize(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_calldataload(uint64_t top, uint64_t g);

struct tuple_bits_64_Bytes_uint_64 execute_calldatacopy(uint64_t top, Bytes mem, uint64_t g);

struct tuple_bits_64_uint_64 execute_codesize(uint64_t top, uint64_t g);

struct tuple_bits_64_Bytes_uint_64 execute_codecopy(uint64_t top, Bytes mem, uint64_t g);

struct tuple_bits_64_uint_64 execute_balance(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_selfbalance(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_extcodesize(uint64_t top, uint64_t g);

struct tuple_bits_64_Bytes_uint_64 execute_extcodecopy(uint64_t top, Bytes mem, uint64_t g);

struct tuple_bits_64_uint_64 execute_extcodehash(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_returndatasize(uint64_t top, uint64_t g);

struct tuple_bits_64_Bytes_uint_64 execute_returndatacopy(uint64_t top, Bytes mem, uint64_t g);

struct tuple_bits_64_uint_64 execute_blockhash(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_coinbase(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_timestamp(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_number(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_slotnum(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_prevrandao(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_gaslimit(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_chainid(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_basefee(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_blobbasefee(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_blobhash(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_pop(uint64_t top, uint64_t g);

struct tuple_bits_64_Bytes_uint_64 execute_mload(uint64_t top, Bytes mem, uint64_t g);

struct tuple_bits_64_Bytes_uint_64 execute_mstore(uint64_t top, Bytes mem, uint64_t g);

struct tuple_bits_64_Bytes_uint_64 execute_mstore8(uint64_t top, Bytes mem, uint64_t g);

struct tuple_bits_64_Bytes_uint_64 execute_msize(uint64_t top, Bytes mem, uint64_t g);

struct tuple_bits_64_Bytes_uint_64 execute_mcopy(uint64_t top, Bytes mem, uint64_t g);

struct tuple_bits_64_uint_64 execute_sload_(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_sstore_(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_tload(uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_tstore(uint64_t top, uint64_t g);

struct tuple_uint_32_bits_64_uint_64 execute_jump(uint32_t pc_in, uint64_t top, uint64_t g);

struct tuple_uint_32_bits_64_uint_64 execute_jumpi(uint32_t pc_in, uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_pc(uint32_t pc_in, uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_gas(uint64_t top, uint64_t g);

uint64_t execute_jumpdest(uint64_t g);

struct tuple_bits_64_uint_64 execute_push(uint8_t n, u256 v, uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_swap(uint8_t n, uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_dupn(uint64_t immediate, uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_swapn(uint64_t immediate, uint64_t top, uint64_t g);

struct tuple_bits_64_uint_64 execute_exchange(uint64_t immediate, uint64_t top, uint64_t g);

struct tuple_bits_64_Bytes_uint_64 execute_log(uint8_t n, uint64_t top, Bytes mem, uint64_t g);

void execute_stop(void);

struct tuple_bits_64_Bytes_uint_64 execute_return(uint64_t top, Bytes mem, uint64_t g);

struct tuple_bits_64_Bytes_uint_64 execute_revert(uint64_t top, Bytes mem, uint64_t g);

uint64_t execute_invalid(uint64_t g);

struct tuple_bits_64_uint_64 execute_selfdestruct(uint64_t top, uint64_t g);

struct tuple_uint_32_bits_64_Bytes_uint_64 run_create(enum CreateKind kind, uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g);

struct tuple_uint_32_bits_64_Bytes_uint_64 execute_create(uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g);

struct tuple_uint_32_bits_64_Bytes_uint_64 execute_create2(uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g);

struct tuple_uint_32_bits_64_Bytes_uint_64 run_call(enum CallKind kind, uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g);

struct tuple_uint_32_bits_64_Bytes_uint_64 execute_call(uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g);

struct tuple_uint_32_bits_64_Bytes_uint_64 execute_callcode(uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g);

struct tuple_uint_32_bits_64_Bytes_uint_64 execute_delegatecall(uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g);

struct tuple_uint_32_bits_64_Bytes_uint_64 execute_staticcall(uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g);

struct tuple_uint_16_uint_16 opcode_stack_effect(struct ast op);

struct tuple_uint_32_bits_64_Bytes_uint_64 execute_opcode(struct ast op, uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g);

struct tuple_uint_32_bits_64_Bytes_uint_64 execute(struct ast op, uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g);

struct tuple_bits_64_uint_64 execute_dup(uint8_t n, uint64_t top, uint64_t g);


#ifdef __cplusplus
}
#endif
