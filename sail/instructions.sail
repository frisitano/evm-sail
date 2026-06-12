/* ======================================================================== */
/* EL-IR instruction surface: the full EVM opcode set (Cancun/Prague)        */
/*                                                                           */
/* One `ast` constructor per opcode. Immediates are carried inline (PUSH      */
/* holds its width + value; DUP/SWAP/LOG hold n). Decoding from code bytes    */
/* lives in interpreter.sail (`fetch`). Undefined bytes decode to OP_INVALID. */
/* ======================================================================== */

union ast = {
  /* 0x00, 0x0b: arithmetic */
  OP_STOP : unit, OP_ADD : unit, OP_MUL : unit, OP_SUB : unit, OP_DIV : unit,
  OP_SDIV : unit, OP_MOD : unit, OP_SMOD : unit, OP_ADDMOD : unit,
  OP_MULMOD : unit, OP_EXP : unit, OP_SIGNEXTEND : unit,

  /* 0x10, 0x1e: comparison / bitwise */
  OP_LT : unit, OP_GT : unit, OP_SLT : unit, OP_SGT : unit, OP_EQ : unit,
  OP_ISZERO : unit, OP_AND : unit, OP_OR : unit, OP_XOR : unit, OP_NOT : unit,
  OP_BYTE : unit, OP_SHL : unit, OP_SHR : unit, OP_SAR : unit, OP_CLZ : unit,

  /* 0x20: keccak */
  OP_KECCAK256 : unit,

  /* 0x30, 0x3f: environment / account */
  OP_ADDRESS : unit, OP_BALANCE : unit, OP_ORIGIN : unit, OP_CALLER : unit,
  OP_CALLVALUE : unit, OP_CALLDATALOAD : unit, OP_CALLDATASIZE : unit,
  OP_CALLDATACOPY : unit, OP_CODESIZE : unit, OP_CODECOPY : unit,
  OP_GASPRICE : unit, OP_EXTCODESIZE : unit, OP_EXTCODECOPY : unit,
  OP_RETURNDATASIZE : unit, OP_RETURNDATACOPY : unit, OP_EXTCODEHASH : unit,

  /* 0x40, 0x4a: block */
  OP_BLOCKHASH : unit, OP_COINBASE : unit, OP_TIMESTAMP : unit, OP_NUMBER : unit,
  OP_PREVRANDAO : unit, OP_GASLIMIT : unit, OP_CHAINID : unit,
  OP_SELFBALANCE : unit, OP_BASEFEE : unit, OP_BLOBHASH : unit, OP_BLOBBASEFEE : unit,

  /* 0x50, 0x5e: stack / memory / storage / flow */
  OP_POP : unit, OP_MLOAD : unit, OP_MSTORE : unit, OP_MSTORE8 : unit,
  OP_SLOAD : unit, OP_SSTORE : unit, OP_JUMP : unit, OP_JUMPI : unit,
  OP_PC : unit, OP_MSIZE : unit, OP_GAS : unit, OP_JUMPDEST : unit,
  OP_TLOAD : unit, OP_TSTORE : unit, OP_MCOPY : unit,

  /* 0x5f, 0x7f: push (width 0..32, value) */
  OP_PUSH : (int, word),

  /* 0x80, 0x9f: dup / swap (n) */
  OP_DUP : int, OP_SWAP : int,

  /* 0xa0, 0xa4: log (num topics) */
  OP_LOG : int,

  /* 0xf0, 0xff: system */
  OP_CREATE : unit, OP_CALL : unit, OP_CALLCODE : unit, OP_RETURN : unit,
  OP_DELEGATECALL : unit, OP_CREATE2 : unit, OP_STATICCALL : unit,
  OP_REVERT : unit, OP_INVALID : unit, OP_SELFDESTRUCT : unit
}
