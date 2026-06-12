/* ======================================================================== */
/* Runnable example: execute a one-transaction block (transfer + SSTORE +     */
/* withdrawal) against the host, and assert storage, gas, and balances.       */
/*                                                                           */
/* Contract code: PUSH1 0x2a; PUSH1 0x00; SSTORE; STOP   (store 42 at slot 0) */
/* Compile + run:  make run-example                                  */
/* ======================================================================== */

$include "../evm/evm.sail"
$include "evm_primitives.sail"
$include <string.sail>

function eoa()        -> address = 0x00000000000000000000000000000000000005e0
function contract()   -> address = 0x000000000000000000000000000000000000c0de
function coinbase_a() -> address = 0x000000000000000000000000000000000000c01b
function withdrawer() -> address = 0x0000000000000000000000000000000000004895

function contract_code() -> list(byte) = [| 0x60, 0x2a, 0x60, 0x00, 0x55, 0x00 |]

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

  print_endline(concat_str("  contract storage[0] = ", dec_str(unsigned(stored))));
  print_endline(concat_str("  total gas used      = ", dec_str(gas_used)));
  print_endline(concat_str("  withdrawal balance  = ", dec_str(unsigned(wbal))));
  print_endline(concat_str("  coinbase balance    = ", dec_str(unsigned(cbbal))));

  assert(unsigned(stored) == 42,  "storage slot 0 should be 42 (SSTORE 0x2a)");
  assert(unsigned(wbal) == 500,   "withdrawal should credit 500 wei");
  assert(gas_used == 43106,       "gas used should be 43106 (21000 + 6 + cold zero->nonzero SSTORE 22100)");
  match receipts {
    r :: _ => assert(r.rc_success, "the transaction receipt should be success"),
    [||]   => assert(false, "expected one receipt")
  };
  print_endline("");
  print_endline("host/transaction-kernel block execution: all assertions passed.")
}
