/* ======================================================================== */
/* INSTRUCTION SURFACE                                                      */
/*                                                                          */
/* The abstract syntax of EVM bytecode: the full defined opcode set through */
/* the Osaka fork (Yellow Paper Appendix H, plus the EIPs that extend it -- */
/* e.g. PUSH0 EIP-3855, SHL/SHR/SAR EIP-145, CLZ EIP-7939, TLOAD/TSTORE     */
/* EIP-1153, MCOPY EIP-5656, BLOBHASH/BLOBBASEFEE EIP-4844/7516).           */
/*                                                                          */
/* One `ast` constructor per opcode. Immediates are carried inline: PUSH    */
/* holds its byte width (0..32) and value, DUP/SWAP hold the index n, LOG   */
/* holds its topic count n. The constructor groups below are labelled with  */
/* the opcode byte range they cover. Decoding code bytes into this AST      */
/* lives in interpreter.sail (`fetch`); undefined bytes decode to INVALID   */
/* and halt exceptionally when executed.                                    */
/* ======================================================================== */

union ast = {
  /* 0x00, 0x0b: arithmetic */
  STOP : unit, ADD : unit, MUL : unit, SUB : unit, DIV : unit,
  SDIV : unit, MOD : unit, SMOD : unit, ADDMOD : unit,
  MULMOD : unit, EXP : unit, SIGNEXTEND : unit,

  /* 0x10, 0x1e: comparison / bitwise */
  LT : unit, GT : unit, SLT : unit, SGT : unit, EQ : unit,
  ISZERO : unit, AND : unit, OR : unit, XOR : unit, NOT : unit,
  BYTE : unit, SHL : unit, SHR : unit, SAR : unit, CLZ : unit,

  /* 0x20: keccak */
  KECCAK256 : unit,

  /* 0x30, 0x3f: environment / account */
  ADDRESS : unit, BALANCE : unit, ORIGIN : unit, CALLER : unit,
  CALLVALUE : unit, CALLDATALOAD : unit, CALLDATASIZE : unit,
  CALLDATACOPY : unit, CODESIZE : unit, CODECOPY : unit,
  GASPRICE : unit, EXTCODESIZE : unit, EXTCODECOPY : unit,
  RETURNDATASIZE : unit, RETURNDATACOPY : unit, EXTCODEHASH : unit,

  /* 0x40, 0x4a: block */
  BLOCKHASH : unit, COINBASE : unit, TIMESTAMP : unit, NUMBER : unit,
  PREVRANDAO : unit, GASLIMIT : unit, CHAINID : unit,
  SELFBALANCE : unit, BASEFEE : unit, BLOBHASH : unit, BLOBBASEFEE : unit,
  SLOTNUM : unit, /* EIP-7843 (0x4b) */

  /* 0x50, 0x5e: stack / memory / storage / flow */
  POP : unit, MLOAD : unit, MSTORE : unit, MSTORE8 : unit,
  SLOAD : unit, SSTORE : unit, JUMP : unit, JUMPI : unit,
  PC : unit, MSIZE : unit, GAS : unit, JUMPDEST : unit,
  TLOAD : unit, TSTORE : unit, MCOPY : unit,

  /* 0x5f, 0x7f: push (width 0..32, value) */
  PUSH : (int, word),

  /* 0x80, 0x9f: dup / swap (n) */
  DUP : int, SWAP : int,

  /* 0xa0, 0xa4: log (num topics) */
  LOG : int,

  /* 0xf0, 0xff: system */
  CREATE : unit, CALL : unit, CALLCODE : unit, RETURN : unit,
  DELEGATECALL : unit, CREATE2 : unit, STATICCALL : unit,
  REVERT : unit, INVALID : unit, SELFDESTRUCT : unit
}
