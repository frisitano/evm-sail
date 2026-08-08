#pragma once

#include "evmsail/spec/evm/precompiles.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct tuple_uint_8_u256
struct tuple_uint_8_u256 {
  uint8_t tup0;
  u256 tup1;
};

// union ast
enum kind_ast { Kind_ADD, Kind_ADDMOD, Kind_ADDRESS, Kind_AND, Kind_BALANCE, Kind_BASEFEE, Kind_BLOBBASEFEE, Kind_BLOBHASH, Kind_BLOCKHASH, Kind_BYTE, Kind_CALL, Kind_CALLCODE, Kind_CALLDATACOPY, Kind_CALLDATALOAD, Kind_CALLDATASIZE, Kind_CALLER, Kind_CALLVALUE, Kind_CHAINID, Kind_CLZ, Kind_CODECOPY, Kind_CODESIZE, Kind_COINBASE, Kind_CREATE2, Kind_DELEGATECALL, Kind_DIV, Kind_DUP, Kind_DUPN, Kind_EQ, Kind_EXCHANGE, Kind_EXP, Kind_EXTCODECOPY, Kind_EXTCODEHASH, Kind_EXTCODESIZE, Kind_GAS, Kind_GASLIMIT, Kind_GASPRICE, Kind_GT, Kind_INVALID, Kind_ISZERO, Kind_JUMP, Kind_JUMPDEST, Kind_JUMPI, Kind_KECCAK256, Kind_LOG, Kind_LT, Kind_MCOPY, Kind_MLOAD, Kind_MOD, Kind_MSIZE, Kind_MSTORE, Kind_MSTORE8, Kind_MUL, Kind_MULMOD, Kind_NOT, Kind_NUMBER, Kind_OR, Kind_ORIGIN, Kind_PC, Kind_POP, Kind_PREVRANDAO, Kind_PUSH, Kind_RETURN, Kind_RETURNDATACOPY, Kind_RETURNDATASIZE, Kind_REVERT, Kind_SAR, Kind_SDIV, Kind_SELFBALANCE, Kind_SELFDESTRUCT, Kind_SGT, Kind_SHL, Kind_SHR, Kind_SIGNEXTEND, Kind_SLOAD, Kind_SLOTNUM, Kind_SLT, Kind_SMOD, Kind_SSTORE, Kind_STATICCALL, Kind_STOP, Kind_SUB, Kind_SWAP, Kind_SWAPN, Kind_TIMESTAMP, Kind_TLOAD, Kind_TSTORE, Kind_XOR, Kind_opcode_CREATE };

struct ast {
  enum kind_ast kind;
  union {
    struct { unit ADD; };
    struct { unit ADDMOD; };
    struct { unit ADDRESS; };
    struct { unit AND; };
    struct { unit BALANCE; };
    struct { unit BASEFEE; };
    struct { unit BLOBBASEFEE; };
    struct { unit BLOBHASH; };
    struct { unit BLOCKHASH; };
    struct { unit BYTE; };
    struct { unit CALL; };
    struct { unit CALLCODE; };
    struct { unit CALLDATACOPY; };
    struct { unit CALLDATALOAD; };
    struct { unit CALLDATASIZE; };
    struct { unit CALLER; };
    struct { unit CALLVALUE; };
    struct { unit CHAINID; };
    struct { unit CLZ; };
    struct { unit CODECOPY; };
    struct { unit CODESIZE; };
    struct { unit COINBASE; };
    struct { unit CREATE2; };
    struct { unit DELEGATECALL; };
    struct { unit DIV; };
    struct { uint8_t DUP; };
    struct { uint64_t DUPN; };
    struct { unit EQ; };
    struct { uint64_t EXCHANGE; };
    struct { unit EXP; };
    struct { unit EXTCODECOPY; };
    struct { unit EXTCODEHASH; };
    struct { unit EXTCODESIZE; };
    struct { unit GAS; };
    struct { unit GASLIMIT; };
    struct { unit GASPRICE; };
    struct { unit GT; };
    struct { unit INVALID; };
    struct { unit ISZERO; };
    struct { unit JUMP; };
    struct { unit JUMPDEST; };
    struct { unit JUMPI; };
    struct { unit KECCAK256; };
    struct { uint8_t LOG; };
    struct { unit LT; };
    struct { unit MCOPY; };
    struct { unit MLOAD; };
    struct { unit MOD; };
    struct { unit MSIZE; };
    struct { unit MSTORE; };
    struct { unit MSTORE8; };
    struct { unit MUL; };
    struct { unit MULMOD; };
    struct { unit NOT; };
    struct { unit NUMBER; };
    struct { unit OR; };
    struct { unit ORIGIN; };
    struct { unit PC; };
    struct { unit POP; };
    struct { unit PREVRANDAO; };
    struct { struct tuple_uint_8_u256 PUSH; };
    struct { unit RETURN; };
    struct { unit RETURNDATACOPY; };
    struct { unit RETURNDATASIZE; };
    struct { unit REVERT; };
    struct { unit SAR; };
    struct { unit SDIV; };
    struct { unit SELFBALANCE; };
    struct { unit SELFDESTRUCT; };
    struct { unit SGT; };
    struct { unit SHL; };
    struct { unit SHR; };
    struct { unit SIGNEXTEND; };
    struct { unit SLOAD; };
    struct { unit SLOTNUM; };
    struct { unit SLT; };
    struct { unit SMOD; };
    struct { unit SSTORE; };
    struct { unit STATICCALL; };
    struct { unit STOP; };
    struct { unit SUB; };
    struct { uint8_t SWAP; };
    struct { uint64_t SWAPN; };
    struct { unit TIMESTAMP; };
    struct { unit TLOAD; };
    struct { unit TSTORE; };
    struct { unit XOR; };
    struct { unit opcode_CREATE; };
  } variants;
};

// struct tuple_uint_16_uint_16
struct tuple_uint_16_uint_16 {
  uint16_t tup0;
  uint16_t tup1;
};

uint8_t decode_single_stack_index(uint64_t immediate);

struct tuple_uint_16_uint_16 decode_exchange_stack_indices(uint64_t immediate);


#ifdef __cplusplus
}
#endif
