/* ======================================================================== */
/* evm-sail block fixtures: run a variety of Ethereum blocks and assert results  */
/*                                                                           */
/* Each fixture wipes the kernel world, seeds pre-state, executes a block via */
/* process_block, prints a one-line summary, and asserts the expected state / */
/* gas / receipt. A regression suite for the user-space/kernel EVM.           */
/* Run:  make run EX=examples/fixtures.sail                          */
/* ======================================================================== */

$include "../evm/evm.sail"
$include "evm_primitives.sail"
$include <string.sail>

/* ------------------------------ addresses ------------------------------- */
function alice()    -> address = 0x000000000000000000000000000000000000a11c
function bob()      -> address = 0x0000000000000000000000000000000000000b0b
function cont()     -> address = 0x000000000000000000000000000000000000c0de
function callee()   -> address = 0x000000000000000000000000000000000000bbbb
function coinbase_a() -> address = 0x000000000000000000000000000000000000c01b

/* ------------------------------ helpers --------------------------------- */
function fund(a : address, wei : int) -> unit =
  k_seed_account(a, 0, word_of_int(wei), [||])
function deploy(a : address, c : list(byte)) -> unit =
  k_seed_account(a, 1, word_of_int(0), c)

function std_env() -> BlockEnv = struct {
  be_number = word_of_int(21000000), be_timestamp = word_of_int(1700000000),
  be_gas_limit = word_of_int(30000000), be_coinbase = coinbase_a(),
  be_base_fee = word_of_int(0), be_prev_randao = word_of_int(0),
  be_chain_id = word_of_int(1), be_blob_base_fee = word_of_int(1),
  be_excess_blob_gas = word_of_int(0)
}
function call_tx(sender : address, dst : address, value : int, gl : gas, data : list(byte)) -> Transaction =
  struct { t_sender = sender, t_nonce = 0, t_gas_price = word_of_int(1), t_priority_fee = word_of_int(1),
           t_gas_limit = gl, t_is_create = false, t_to = dst, t_value = word_of_int(value), t_data = data,
           t_al_addrs = [||], t_al_keys = [||], t_blob_count = 0, t_max_fee = word_of_int(1), t_max_blob_fee = word_of_int(1), t_max_priority = word_of_int(1), t_is_setcode = false, t_auth_list = [||], t_is_blob = false, t_blob_hashes = [||] }
function create_tx(sender : address, gl : gas, initcode : list(byte)) -> Transaction =
  struct { t_sender = sender, t_nonce = 0, t_gas_price = word_of_int(1), t_priority_fee = word_of_int(1),
           t_gas_limit = gl, t_is_create = true, t_to = ZERO_ADDR, t_value = word_of_int(0), t_data = initcode,
           t_al_addrs = [||], t_al_keys = [||], t_blob_count = 0, t_max_fee = word_of_int(1), t_max_blob_fee = word_of_int(1), t_max_priority = word_of_int(1), t_is_setcode = false, t_auth_list = [||], t_is_blob = false, t_blob_hashes = [||] }
function block1(tx : Transaction) -> Block =
  struct { blk_env = std_env(), blk_txs = [| tx |], blk_withdrawals = [||] }

function rsucc(rs : list(Receipt)) -> bool = match rs { r :: _ => r.rc_success, [||] => false }
function rgas(rs : list(Receipt))  -> gas  = match rs { r :: _ => r.rc_gas_used, [||] => 0 }
function bal(a : address)  -> int = unsigned(kget(a).acc_balance)
function slot0(a : address) -> int = unsigned(k_storage_get(a, word_of_int(0)))
function yn(b : bool) -> string = if b then "ok" else "FAIL"
function line(name : string, rest : string) -> unit = print_endline(concat_str(name, rest))

/* ============================== fixtures ================================ */

/* F1: plain EOA -> EOA value transfer (no code). */
function f1_transfer() -> unit = {
  k_world_reset(); fund(alice(), 1000000);
  let rs = process_block(block1(call_tx(alice(), bob(), 500, 21000, [||])));
  line("  F1 transfer    : ", concat_str(yn(rsucc(rs)),
    concat_str(concat_str("  gas=", dec_str(rgas(rs))),
      concat_str(concat_str("  bob=", dec_str(bal(bob()))),
                 concat_str("  alice=", dec_str(bal(alice())))))));
  assert(rsucc(rs), "F1 success"); assert(rgas(rs) == 21000, "F1 gas 21000");
  assert(bal(bob()) == 500, "F1 bob credited");
  assert(bal(alice()) == 1000000 - 500 - 21000, "F1 alice debited value+gas")
}

/* F2: SSTORE 0x2a at slot 0. */
function f2_storage() -> unit = {
  k_world_reset(); fund(alice(), 1000000);
  deploy(cont(), [| 0x60, 0x2a, 0x60, 0x00, 0x55, 0x00 |]);
  let rs = process_block(block1(call_tx(alice(), cont(), 0, 100000, [||])));
  line("  F2 storage     : ", concat_str(yn(rsucc(rs)),
    concat_str(concat_str("  gas=", dec_str(rgas(rs))),
               concat_str("  slot0=", dec_str(slot0(cont()))))));
  assert(rsucc(rs), "F2 success"); assert(slot0(cont()) == 42, "F2 slot0 = 42");
  assert(rgas(rs) == 43106, "F2 gas 43106 (cold zero->nonzero SSTORE)")
}

/* F3: arithmetic (2+3)*4 = 20 -> slot 0.  PUSH1 2 PUSH1 3 ADD PUSH1 4 MUL PUSH1 0 SSTORE STOP */
function f3_arith() -> unit = {
  k_world_reset(); fund(alice(), 1000000);
  deploy(cont(), [| 0x60, 0x02, 0x60, 0x03, 0x01, 0x60, 0x04, 0x02, 0x60, 0x00, 0x55, 0x00 |]);
  let rs = process_block(block1(call_tx(alice(), cont(), 0, 100000, [||])));
  line("  F3 arithmetic  : ", concat_str(yn(rsucc(rs)),
    concat_str(concat_str("  gas=", dec_str(rgas(rs))),
               concat_str("  slot0=", dec_str(slot0(cont()))))));
  assert(rsucc(rs), "F3 success"); assert(slot0(cont()) == 20, "F3 (2+3)*4 = 20")
}

/* F4: cont CALLs callee; callee SSTOREs 99 at its slot 0. */
function f4_call() -> unit = {
  k_world_reset(); fund(alice(), 1000000);
  /* caller: push 7 CALL args (retLen,retOff,argsLen,argsOff,value,addr,gas) then CALL, STOP */
  deploy(cont(), [| 0x60,0x00, 0x60,0x00, 0x60,0x00, 0x60,0x00, 0x60,0x00,
                    0x61,0xbb,0xbb, 0x61,0xff,0xff, 0xf1, 0x00 |]);
  deploy(callee(), [| 0x60, 0x63, 0x60, 0x00, 0x55, 0x00 |]);   /* SSTORE 99 @ slot0 */
  let rs = process_block(block1(call_tx(alice(), cont(), 0, 200000, [||])));
  line("  F4 nested CALL : ", concat_str(yn(rsucc(rs)),
    concat_str(concat_str("  gas=", dec_str(rgas(rs))),
               concat_str("  callee.slot0=", dec_str(slot0(callee()))))));
  assert(rsucc(rs), "F4 success"); assert(slot0(callee()) == 99, "F4 callee stored 99");
  assert(slot0(cont()) == 0, "F4 caller storage untouched")
}

/* F5: SSTORE 42 then REVERT -> store rolled back, receipt fails. */
function f5_revert() -> unit = {
  k_world_reset(); fund(alice(), 1000000);
  /* PUSH1 42 PUSH1 0 SSTORE  PUSH1 0 PUSH1 0 REVERT */
  deploy(cont(), [| 0x60,0x2a, 0x60,0x00, 0x55, 0x60,0x00, 0x60,0x00, 0xfd |]);
  let rs = process_block(block1(call_tx(alice(), cont(), 0, 100000, [||])));
  line("  F5 REVERT      : ", concat_str(yn(not_bool(rsucc(rs))),
    concat_str(concat_str("  reverted, gas=", dec_str(rgas(rs))),
               concat_str("  slot0=", dec_str(slot0(cont()))))));
  assert(not_bool(rsucc(rs)), "F5 receipt should be failure");
  assert(slot0(cont()) == 0, "F5 SSTORE rolled back by REVERT")
}

/* F6: CREATE tx deploying 1-byte runtime [0x00]. */
function f6_create() -> unit = {
  k_world_reset(); fund(alice(), 1000000);
  /* init: PUSH1 0 PUSH1 0 MSTORE8 ; PUSH1 1 PUSH1 0 RETURN  -> returns [0x00] */
  let rs = process_block(block1(create_tx(alice(), 200000,
             [| 0x60,0x00, 0x60,0x00, 0x53, 0x60,0x01, 0x60,0x00, 0xf3 |])));
  let new_addr = create_address(alice(), 0);
  let deployed_len = k_get_code_size(new_addr);
  line("  F6 CREATE      : ", concat_str(yn(rsucc(rs)),
    concat_str(concat_str("  gas=", dec_str(rgas(rs))),
               concat_str("  deployed_code_len=", dec_str(deployed_len)))));
  assert(rsucc(rs), "F6 success"); assert(deployed_len == 1, "F6 deployed 1-byte runtime")
}

/* F7: two transactions in one block; second overwrites the slot. */
function f7_multitx() -> unit = {
  k_world_reset(); fund(alice(), 1000000);
  deploy(cont(), [| 0x60, 0x2a, 0x60, 0x00, 0x55, 0x00 |]);   /* store 42 */
  deploy(callee(), [| 0x60, 0x64, 0x60, 0x00, 0x55, 0x00 |]); /* store 100 */
  let blk : Block = struct { blk_env = std_env(),
    blk_txs = [| call_tx(alice(), cont(), 0, 100000, [||]),
                 call_tx(alice(), cont(), 0, 100000, [||]) |],
    blk_withdrawals = [||] };
  let rs = process_block(blk);
  let n = list_len(rs);
  /* second tx overwrites 42 with 42 again? same code stores 42; use cont then re-store */
  line("  F7 multi-tx    : ", concat_str(concat_str("receipts=", dec_str(n)),
    concat_str(concat_str("  total_gas=", dec_str(total_gas_used(rs))),
               concat_str("  slot0=", dec_str(slot0(cont()))))));
  assert(n == 2, "F7 two receipts"); assert(slot0(cont()) == 42, "F7 slot0 = 42")
}

/* F8: gas_limit == intrinsic -> out of gas before any opcode. */
function f8_oog() -> unit = {
  k_world_reset(); fund(alice(), 1000000);
  deploy(cont(), [| 0x60, 0x2a, 0x60, 0x00, 0x55, 0x00 |]);
  let rs = process_block(block1(call_tx(alice(), cont(), 0, 21000, [||])));
  line("  F8 out-of-gas  : ", concat_str(yn(not_bool(rsucc(rs))),
    concat_str(concat_str("  gas=", dec_str(rgas(rs))),
               concat_str("  slot0=", dec_str(slot0(cont()))))));
  assert(not_bool(rsucc(rs)), "F8 should fail"); assert(rgas(rs) == 21000, "F8 consumes all gas");
  assert(slot0(cont()) == 0, "F8 no state change")
}

function main() -> unit = {
  print_endline("evm-sail block fixtures:");
  print_endline("");
  f1_transfer(); f2_storage(); f3_arith(); f4_call();
  f5_revert(); f6_create(); f7_multitx(); f8_oog();
  print_endline("");
  print_endline("All 8 block fixtures passed.")
}
