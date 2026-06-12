/* ======================================================================== */
/* EL-IR runnable example: execute a one-transaction block, then show the     */
/* EL syscall trace the EVM (user space) issued into the kernel (host space).  */
/*                                                                           */
/* Contract code: PUSH1 0x2a; PUSH1 0x00; SSTORE; STOP   (store 42 at slot 0) */
/* Compile + run:  make -C el-ir run-example                                  */
/* ======================================================================== */

$include "../evm/el_ir.sail"
$include "evm_primitives.sail"
$include <string.sail>

function eoa()        -> address = 0x00000000000000000000000000000000000005e0
function contract()   -> address = 0x000000000000000000000000000000000000c0de
function coinbase_a() -> address = 0x000000000000000000000000000000000000c01b
function withdrawer() -> address = 0x0000000000000000000000000000000000004895

function contract_code() -> list(byte) = [| 0x60, 0x2a, 0x60, 0x00, 0x55, 0x00 |]

/* render an EL syscall for display */
function envf_str(f : EnvField) -> string = match f {
  F_Number => "NUMBER", F_Timestamp => "TIMESTAMP", F_Coinbase => "COINBASE",
  F_BaseFee => "BASEFEE", F_ChainId => "CHAINID", F_GasLimit => "GASLIMIT",
  F_PrevRandao => "PREVRANDAO", F_BlobBaseFee => "BLOBBASEFEE",
  F_Origin => "ORIGIN", F_GasPrice => "GASPRICE"
}
function elop_str(op : ELOp) -> string = match op {
  EL_AccessAccount(_) => "AccessAccount",
  EL_AccessSlot(_, s) => concat_str("AccessSlot slot=", dec_str(unsigned(s))),
  EL_SLoad(_, s)      => concat_str("SLoad slot=", dec_str(unsigned(s))),
  EL_SStore(_, s, v)  => concat_str("SStore slot=", concat_str(dec_str(unsigned(s)),
                            concat_str(" <- ", dec_str(unsigned(v))))),
  EL_TLoad(_, _)      => "TLoad",  EL_TStore(_, _, _) => "TStore",
  EL_GetBalance(_)    => "GetBalance", EL_GetNonce(_) => "GetNonce",
  EL_GetCode(_)       => "GetCode",   EL_GetCodeHash(_) => "GetCodeHash",
  EL_Transfer(_, _, v)=> concat_str("Transfer amount=", dec_str(unsigned(v))),
  EL_BumpNonce(_)     => "BumpNonce", EL_SetCode(_, n) => concat_str("SetCode len=", dec_str(n)),
  EL_Env(f)           => concat_str("Env ", envf_str(f)),
  EL_BlockHash(_)     => "BlockHash", EL_BlobHash(_) => "BlobHash",
  EL_Keccak(n)        => concat_str("Keccak len=", dec_str(n)),
  EL_Precompile(p, _) => concat_str("Precompile #", dec_str(p)),
  EL_Snapshot()       => "Snapshot", EL_Commit() => "Commit", EL_Revert() => "Revert",
  EL_Refund(d)        => concat_str("Refund ", dec_str(d)),
  EL_Log(_, t, d)     => concat_str("Log topics=", concat_str(dec_str(t),
                            concat_str(" bytes=", dec_str(d))))
}
/* opcode byte -> mnemonic (the subset that appears here; default OP_xx). */
function opcode_name(b : int) -> string = match b {
  0 => "STOP", 1 => "ADD", 2 => "MUL", 3 => "SUB", 16 => "LT", 20 => "EQ",
  21 => "ISZERO", 22 => "AND", 84 => "SLOAD", 85 => "SSTORE", 80 => "POP",
  81 => "MLOAD", 82 => "MSTORE", 83 => "MSTORE8", 86 => "JUMP", 87 => "JUMPI",
  91 => "JUMPDEST", 96 => "PUSH1", 97 => "PUSH2", 241 => "CALL", 243 => "RETURN",
  253 => "REVERT", 255 => "SELFDESTRUCT", 67 => "NUMBER", 32 => "KECCAK256",
  _ => concat_str("OP_", hex_str(b))
}

/* print the interleaved EL-IR stream: [EVM] opcode steps, [EL] syscalls. */
function print_trace(es : list(TraceEntry)) -> unit = match es {
  [||]      => (),
  e :: rest => {
    match e {
      TSyscall(op) => print_endline(concat_str("  [EL ] ", elop_str(op))),
      TEvm(at, b)  => print_endline(concat_str("      [EVM] pc=",
                        concat_str(dec_str(at), concat_str("  ", opcode_name(b)))))
    };
    print_trace(rest)
  }
}

function main() -> unit = {
  /* host seeds pre-state */
  k_seed_account(eoa(), struct {
    acc_nonce = 0, acc_balance = word_of_int(1000000000000000000), acc_code = [||] });
  k_seed_account(contract(), struct {
    acc_nonce = 1, acc_balance = word_of_int(0), acc_code = contract_code() });

  let env : BlockEnv = struct {
    be_number = word_of_int(21000000), be_timestamp = word_of_int(1700000000),
    be_gas_limit = word_of_int(30000000), be_coinbase = coinbase_a(),
    be_base_fee = word_of_int(0), be_prev_randao = word_of_int(0),
    be_chain_id = word_of_int(1), be_blob_base_fee = word_of_int(1),
    be_excess_blob_gas = word_of_int(0)
  };
  let tx : Transaction = struct {
    t_sender = eoa(), t_nonce = 0, t_gas_price = word_of_int(2), t_priority_fee = word_of_int(1),
    t_gas_limit = 100000, t_is_create = false, t_to = contract(), t_value = word_of_int(0), t_data = [||],
    t_al_addrs = [||], t_al_keys = [||], t_blob_count = 0, t_max_fee = word_of_int(2), t_max_blob_fee = word_of_int(0), t_max_priority = word_of_int(1), t_is_setcode = false, t_auth_list = [||], t_is_blob = false, t_blob_hashes = [||]
  };
  let wd : Withdrawal = struct { w_address = withdrawer(), w_amount = word_of_int(500) };
  let blk : Block = struct { blk_env = env, blk_txs = [| tx |], blk_withdrawals = [| wd |] };

  let receipts = process_block(blk);

  let stored   = k_storage_get(contract(), word_of_int(0));
  let gas_used = total_gas_used(receipts);
  let wbal     = kget(withdrawer()).acc_balance;
  let cbbal    = kget(coinbase_a()).acc_balance;

  print_endline("EL-IR stream: [EVM] user-space opcode steps + [EL] kernel syscalls:");
  print_endline("");
  print_trace(list_reverse(ir_trace));
  print_endline("");
  print_endline(concat_str("  contract storage[0] = ", dec_str(unsigned(stored))));
  print_endline(concat_str("  total gas used      = ", dec_str(gas_used)));
  print_endline(concat_str("  withdrawal balance  = ", dec_str(unsigned(wbal))));
  print_endline(concat_str("  coinbase balance    = ", dec_str(unsigned(cbbal))));
  print_endline(concat_str("  EL-IR stream length = ", dec_str(list_len(ir_trace))));

  assert(unsigned(stored) == 42,  "storage slot 0 should be 42 (SSTORE 0x2a)");
  assert(unsigned(wbal) == 500,   "withdrawal should credit 500 wei");
  assert(gas_used == 43106,       "gas used should be 43106 (21000 + 6 + cold zero->nonzero SSTORE 22100)");
  match receipts {
    r :: _ => assert(r.rc_success, "the transaction receipt should be success"),
    [||]   => assert(false, "expected one receipt")
  };
  print_endline("");
  print_endline("EL-IR user/kernel block execution: all assertions passed.")
}
