
/*md
# The instruction set

The abstract syntax of EVM bytecode: the full defined opcode set through
the Osaka fork (Yellow Paper Appendix H, plus the EIPs that extend it —
`PUSH0` EIP-3855, `SHL`/`SHR`/`SAR` EIP-145, `CLZ` EIP-7939,
`TLOAD`/`TSTORE` EIP-1153, `MCOPY` EIP-5656, `BLOBHASH`/`BLOBBASEFEE`
EIP-4844/EIP-7516, and `DUPN`/`SWAPN`/`EXCHANGE` EIP-8024).
*/
/*! One constructor per opcode. Immediates are carried inline: `PUSH`
    holds its byte width (0–32) and value, `DUP`/`SWAP` hold the index
    `n`, `LOG` holds its topic count. The constructor groups are labelled
    with the opcode byte range they cover. Decoding code bytes into this
    AST is [fetch][]'s job; undefined bytes decode to `INVALID` and halt
    exceptionally when executed. */
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
  /* EIP-7843 (0x4b) */
  SLOTNUM : unit,

  /* 0x50, 0x5e: stack / memory / storage / flow */
  POP : unit, MLOAD : unit, MSTORE : unit, MSTORE8 : unit,
  SLOAD : unit, SSTORE : unit, JUMP : unit, JUMPI : unit,
  PC : unit, MSIZE : unit, GAS : unit, JUMPDEST : unit,
  TLOAD : unit, TSTORE : unit, MCOPY : unit,

  /* 0x5f, 0x7f: push (width 0..32, value) */
  PUSH : (push_width, word),

  /* 0x80, 0x9f: dup / swap (n) */
  DUP : stack_operation_index, SWAP : stack_operation_index,

  /* 0xa0, 0xa4: log (num topics) */
  LOG : log_topic_count,

  /* 0xe6, 0xe8: EIP-8024 deep-stack access (immediate byte) */
  DUPN : byte, SWAPN : byte, EXCHANGE : byte,

  /* 0xf0, 0xff: system */
  opcode_CREATE : unit, CALL : unit, CALLCODE : unit, RETURN : unit,
  DELEGATECALL : unit, CREATE2 : unit, STATICCALL : unit,
  REVERT : unit, INVALID : unit, SELFDESTRUCT : unit
}

/*! Decodes the immediate shared by EIP-8024 `DUPN` and `SWAPN` into
    their one-based deep-stack index (17–235). */
function decode_single_stack_index(immediate : byte) -> deep_stack_index = {
    assert(deep_stack_immediate_valid(immediate));
    let value : opcode = unsigned(immediate);
    if value <= 90 then {
        value + 145
    } else {
        assert(128 <= value);
        value - 111
    }
}

/*! Decodes the EIP-8024 `EXCHANGE` immediate into the two zero-based
    stack depths that it exchanges. */
function decode_exchange_stack_indices(immediate : byte) -> (stack_index, stack_index) = {
    assert(exchange_immediate_valid(immediate));
    let shifted : byte = xor_vec(immediate, 0x8f);
    let quotient : range(0, 15) = unsigned(shifted[7 .. 4]);
    let remainder : range(0, 15) = unsigned(shifted[3 .. 0]);
    if quotient < remainder then {
        (quotient + 1, remainder + 1)
    } else {
        (remainder + 1, 29 - quotient)
    }
}
