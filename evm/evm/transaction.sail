/* ======================================================================== */
/* EVM USER-SPACE transaction processing                                     */
/*                                                                           */
/* Intrinsic gas + the upfront-gas/nonce state transition + EIP-3529 refunds  */
/* are user-space policy; all state changes are kernel syscalls. The tx-level */
/* snapshot/commit/revert bound the whole transaction's world effects.       */
/* ======================================================================== */

/* EIP-7702 authorization tuple; the runner pre-recovers the authority + validates
 * the signature (s<=n/2, y_parity, r) into au_valid_sig. */
struct Authorization = {
  au_valid_sig : bool, au_authority : address, au_address : address,
  au_nonce : int, au_chain_id : int
}
struct Transaction = {
  t_sender : address, t_nonce : int, t_gas_price : word, t_priority_fee : word,
  t_gas_limit : gas, t_is_create : bool, t_to : address, t_value : word,
  t_index : int,                /* transaction index: its input bytes live in memory (txin) */
  t_al_addrs : list(address),   /* EIP-2930 access-list addresses (one per entry) */
  t_al_keys  : list(SlotKey),   /* EIP-2930 access-list (address, slot) pairs      */
  t_blob_count : int,           /* EIP-4844 number of blob versioned hashes         */
  t_max_fee : word,             /* EIP-1559 max_fee_per_gas (cap, for validity)     */
  t_max_blob_fee : word,        /* EIP-4844 max_fee_per_blob_gas (cap, for validity)*/
  t_max_priority : word,        /* EIP-1559 max_priority_fee_per_gas (cap, for validity) */
  t_is_setcode : bool,          /* type-4 (EIP-7702) envelope (even with 0 tuples)  */
  t_auth_list : list(Authorization), /* EIP-7702 set-code authorization tuples      */
  t_is_blob : bool,             /* EIP-4844 type-3 marker (blob validity rules)     */
  t_blob_hashes : list(word)    /* EIP-4844 versioned hashes (BLOBHASH operands)    */
}
struct Receipt = { rc_success : bool, rc_gas_used : gas, rc_logs : list(LogEntry) }

let G_transaction : gas = 21000
let G_txcreate    : gas = 32000
let G_txdatazero  : gas = 4
let G_txdatanonzero : gas = 16
let G_al_address  : gas = 2400   /* EIP-2930 ACCESS_LIST_ADDRESS_COST     */
let G_al_key      : gas = 1900   /* EIP-2930 ACCESS_LIST_STORAGE_KEY_COST */
let GAS_PER_BLOB  : int = 131072 /* EIP-4844 GAS_PER_BLOB                  */
let PER_AUTH_BASE    : gas = 12500  /* EIP-7702 PER_AUTH_BASE_COST          */
let PER_EMPTY_ACCOUNT : gas = 25000 /* EIP-7702 PER_EMPTY_ACCOUNT_COST      */

function calldata_cost() -> gas = {
  /* EIP-2028: 4 gas per zero byte, 16 per non-zero, over the loaded tx input */
  let n = txdata_size();
  var g : int = 0;
  foreach (i from 0 to (n - 1)) {
    g = g + (if txdata_byte(i) == 0x00 then G_txdatazero else G_txdatanonzero)
  };
  to_gas(g)
}

/* EIP-7623 calldata tokens: each zero byte = 1 token, each nonzero byte = 4. */
function calldata_tokens() -> int = {
  let n = txdata_size();
  var t : int = 0;
  foreach (i from 0 to (n - 1)) { t = t + (if txdata_byte(i) == 0x00 then 1 else 4) };
  t
}

/* EIP-4844: every blob versioned hash must carry VERSIONED_HASH_VERSION_KZG (0x01). */
function blob_hashes_versioned(xs : list(word)) -> bool =
  match xs {
    [||]   => true,
    h :: r => if h[255 .. 248] == 0x01 then blob_hashes_versioned(r) else false
  }

/* intrinsic gas: tx input already loaded into the C buffer (txdata_load), so
 * calldata cost and the EIP-3860 initcode words read it via txdata_size. */
function intrinsic_gas(tx : Transaction) -> gas = {
  let al = G_al_address * list_len(tx.t_al_addrs) + G_al_key * list_len(tx.t_al_keys);
  let auth = PER_EMPTY_ACCOUNT * list_len(tx.t_auth_list);   /* EIP-7702 */
  let base = G_transaction + calldata_cost() + al + auth;
  to_gas(if tx.t_is_create then base + G_txcreate + initcode_gas(txdata_size()) else base)
}

/* EIP-7702: apply one authorization (validate against current state, set/clear the
 * delegation, bump the authority nonce; refund if the authority already existed). */
function process_auth(au : Authorization) -> unit = {
  let authority = au.au_authority;
  let chain_ok = (au.au_chain_id == 0) | (au.au_chain_id == unsigned(k_block.be_chain_id));
  let acode = k_get_code(authority);
  let (is_deleg, _) = deleg_target(acode);
  let code_ok = code_is_nil(acode) | is_deleg;
  let nonce_ok = k_get_nonce(authority) == au.au_nonce;
  /* step 5: warm the authority once sig + chain id pass, BEFORE the code/nonce
   * checks -- so a tuple later skipped for code/nonce still warms its authority. */
  if au.au_valid_sig & chain_ok then { let _ = k_access_account(authority); () } else ();
  if au.au_valid_sig & chain_ok & code_ok & nonce_ok then {
    let existed = not_bool(k_account_is_empty(authority));
    if unsigned(address_to_word(au.au_address)) == 0
    then k_set_code(authority, [||])                          /* clear delegation */
    else k_set_code(authority, delegation_code(au.au_address));
    k_bump_nonce(authority);
    if existed then k_refund_add(PER_EMPTY_ACCOUNT - PER_AUTH_BASE) else ()
  } else ()
}
function process_auth_list(xs : list(Authorization)) -> unit =
  match xs { [||] => (), a :: r => { process_auth(a); process_auth_list(r) } }

function warm_al_addrs(xs : list(address)) -> unit =
  match xs { [||] => (), a :: r => { let _ = k_access_account(a); warm_al_addrs(r) } }
function warm_al_keys(xs : list(SlotKey)) -> unit =
  match xs { [||] => (), k :: r => { let _ = k_access_slot(k.sk_addr, k.sk_slot); warm_al_keys(r) } }

/* EIP-2929 + EIP-3651 + EIP-2930: pre-warm sender, target, coinbase, precompiles,
 * and the transaction's access list. */
function prewarm(tx : Transaction) -> unit = {
  let _ = k_access_account(tx.t_sender);
  let _ = if tx.t_is_create then false else k_access_account(tx.t_to);
  let _ = k_access_account(k_coinbase());
  foreach (p from 1 to 17) { let _ = k_access_account(word_to_address(word_of_int(p))) };  /* precompiles pre-warmed (EIP-2929; 0x0b-0x11 are EIP-2537 BLS) */
  let _ = k_access_account(word_to_address(word_of_int(256)));  /* 0x100 P256VERIFY (EIP-7951) */
  warm_al_addrs(tx.t_al_addrs);
  warm_al_keys(tx.t_al_keys)
}

function process_transaction(tx : Transaction) -> Receipt = {
  k_tx_reset();
  k_set_tx(struct { txe_origin = tx.t_sender, txe_gas_price = tx.t_gas_price, txe_blob_hashes = tx.t_blob_hashes });
  hr_reset();
  /* select this tx's input (already loaded into memory at block setup); gas,
     validity, calldata and initcode all read it from there by index. */
  let dlen = txdata_activate(tx.t_index);

  let sender = tx.t_sender;
  let nonce_before = k_get_nonce(sender);
  let intrinsic = intrinsic_gas(tx);
  let blob_fee = tx.t_blob_count * GAS_PER_BLOB * unsigned(k_block.be_blob_base_fee);

  /* Transaction validity: an invalid tx is REJECTED with no state change at all (no
     nonce bump, no gas charge). The upfront-balance test uses the fee CAPS
     (max_fee_per_gas, max_fee_per_blob_gas), not the effective prices. */
  let blob_gas = tx.t_blob_count * GAS_PER_BLOB;
  let upfront = tx.t_gas_limit * unsigned(tx.t_max_fee)
              + unsigned(tx.t_value)
              + blob_gas * unsigned(tx.t_max_blob_fee);
  let scode = k_get_code(sender);
  let (sender_deleg, _) = deleg_target(scode);
  /* EIP-4844 / EIP-7691 / EIP-7594 blob-count bounds: Cancun max 6 per block,
     Prague max 9 per block, Osaka adds MAX_BLOBS_PER_TX = 6. A type-3 tx must
     carry >= 1 blob, must not create, and every versioned hash must be 0x01-tagged. */
  let max_blobs : int = if k_fork >= FORK_OSAKA then 6
                        else if k_fork >= FORK_PRAGUE then 9 else 6;
  let blob_ok : bool =
    if tx.t_is_blob then
        (tx.t_blob_count >= 1) & (tx.t_blob_count <= max_blobs)
      & not_bool(tx.t_is_create)
      & blob_hashes_versioned(tx.t_blob_hashes)
    else true;
  /* EIP-7623 (Prague+): the calldata floor is ALSO a validity bound -- a tx whose
     gas limit cannot cover 21000 + 10*tokens is rejected outright. */
  let floor_ok : bool =
    (k_fork < FORK_PRAGUE)
    | (tx.t_gas_limit >= G_transaction + 10 * calldata_tokens());
  let valid =
        (unsigned(k_get_balance(sender)) >= upfront)        /* covers upfront cost      */
      & (code_is_nil(scode) | sender_deleg)                 /* EIP-3607 (7702 exempts delegations) */
      & (tx.t_gas_limit >= intrinsic)                       /* gas covers intrinsic     */
      & floor_ok                                            /* EIP-7623 floor coverage  */
      & (unsigned(tx.t_max_fee) >= unsigned(k_block.be_base_fee))          /* >= base fee */
      & ((tx.t_blob_count == 0)                             /* blob fee cap >= price    */
         | (unsigned(tx.t_max_blob_fee) >= unsigned(k_block.be_blob_base_fee)))
      & blob_ok
      & (not_bool(tx.t_is_create) | (dlen <= MAX_INITCODE_SIZE))  /* EIP-3860 */
      & ((k_fork < FORK_OSAKA) | (tx.t_gas_limit <= 16777216))   /* EIP-7825: gas cap 2^24 */
      & (unsigned(tx.t_max_priority) <= unsigned(tx.t_max_fee))   /* EIP-1559: prio cap <= fee cap */
      & (not_bool(tx.t_is_setcode) | not_bool(tx.t_is_create))    /* EIP-7702: type-4 has to != nil */
      & (not_bool(tx.t_is_setcode) | (list_len(tx.t_auth_list) > 0)); /* EIP-7702: non-empty auth list */
  if not_bool(valid) then return struct { rc_success = false, rc_gas_used = 0, rc_logs = [||] }
  else ();

  /* upfront gas + nonce bump (persist across revert; taken before the snapshot) */
  k_sub_balance(sender, word_of_int(tx.t_gas_limit * unsigned(tx.t_gas_price)));
  /* EIP-4844: burn the blob fee upfront (blob_gas_used * blob_gas_price), no refund */
  if blob_fee > 0 then k_sub_balance(sender, word_of_int(blob_fee)) else ();
  k_bump_nonce(sender);
  prewarm(tx);
  process_auth_list(tx.t_auth_list);   /* EIP-7702: set delegations before execution (persist) */
  k_snapshot();

  pc = 0; call_depth = 0; gas_remaining = to_gas(tx.t_gas_limit - intrinsic);
  stack_clear(); mem_clear(); hr_reset(); memory_size = 0;
  frame_status = Running();

  if tx.t_is_create then {
    let new_addr = k_create_addr(sender, nonce_before);
    let _ = k_access_account(new_addr);
    /* create-tx address collision (code, nonzero nonce, or storage at the
     * target): the tx fails outright consuming ALL gas, no initcode runs
     * (EELS process_message_call -> AddressCollision, gas_left = 0). */
    if k_account_occupied(new_addr) then exc_halt(AddressCollision)
    else {
    k_mark_created(new_addr);          /* EIP-6780: created this tx */
    k_bump_nonce(new_addr);
    if word_nonzero(tx.t_value) then k_transfer(sender, new_addr, tx.t_value) else ();
    message = struct {
      msg_caller = sender, msg_address = new_addr, msg_code_address = new_addr,
      msg_value = tx.t_value, msg_data_len = 0, msg_is_static = false, msg_depth = 0
    };
    frame_code_txd();
    interpret();
    if frame_succeeded() then {
      let dep_len = unsigned(hr_pending_len());
      let deposit = G_codedeposit * dep_len;
      let code_ok = (dep_len <= MAX_CODE_SIZE)
                  & ((dep_len == 0) | (hr_pending_byte(get_slice_int(64, 0, 0)) != 0xef));  /* EIP-3541 */
      if code_ok & (gas_remaining >= deposit)
      then { gas_remaining = to_gas(gas_remaining - deposit);
             k_set_code(new_addr, pending_output_list()); hr_discard() }
      else { hr_discard(); exc_halt(OutOfGas) }     /* failed deploy */
    } else ()
    }
  } else {
    if word_nonzero(tx.t_value) then k_transfer(sender, tx.t_to, tx.t_value) else ();
    let an = unsigned(tx.t_to);
    if (an >= 1 & an <= 17) | (an == 256) then {
      /* tx directly to a precompile: run it as the top-level frame, gas-checked
         FIRST (an OOG precompile must not execute). Failure or OOG is an
         exceptional halt (all gas consumed, value transfer reverted). */
      stage_precompile_txd(an);
      let used = precompile_gas(an, dlen);
      let ok : bool =
        if used <= gas_remaining then k_precompile(an) else false;
      if ok & (used <= gas_remaining) then {
        gas_remaining = to_gas(gas_remaining - used);
        hr_capture_acc(); hr_adopt(); frame_status = Halted(HaltReturn)
      } else exc_halt(OutOfGas)
    } else {
      cd_set_tx();
      message = struct {
        msg_caller = sender, msg_address = tx.t_to, msg_code_address = tx.t_to,
        msg_value = tx.t_value, msg_data_len = dlen, msg_is_static = false, msg_depth = 0
      };
      /* EIP-7702: at the tx level the delegate is warmed but not separately charged */
      let (tx_deleg, tx_dtgt) = k_deleg_target(tx.t_to);
      if tx_deleg then { let _ = k_access_account(tx_dtgt); () } else ();
      frame_code_exec(tx.t_to, tx_deleg, tx_dtgt);   /* follow a delegation designation */
      interpret()
    }
  };

  let success = frame_succeeded();
  if success then k_commit() else k_revert();

  /* EIP-3529 refund, capped at gas_used / 5 (user-space policy) */
  let gas_left0 = gas_remaining;
  let gas_used0 = tx.t_gas_limit - gas_left0;
  let refund_cap = ediv_int(gas_used0, 5);
  let refund = if k_refund < refund_cap then k_refund else refund_cap;
  let refund_pos = if refund < 0 then 0 else refund;
  let gas_left1 = gas_left0 + refund_pos;
  let gas_used1 = tx.t_gas_limit - gas_left1;
  /* EIP-7623 (Prague+): a tx pays at least the calldata floor 21000 + 10*tokens */
  let gas_used =
    if k_fork >= FORK_PRAGUE then {
      let floor = G_transaction + 10 * calldata_tokens();
      if gas_used1 < floor then floor else gas_used1
    } else gas_used1;
  let gas_left = tx.t_gas_limit - gas_used;

  /* return unused gas to sender; pay coinbase the priority fee */
  k_add_balance(sender, word_of_int(gas_left * unsigned(tx.t_gas_price)));
  k_add_balance(k_coinbase(), word_of_int(gas_used * unsigned(tx.t_priority_fee)));

  /* EIP-6780 tx-end reduction: delete accounts created AND selfdestructed in
   * this tx (EELS destroys tx_output.accounts_to_delete after the fee
   * transfers; a failed tx has no surviving selfdestructs to apply). */
  if success then k_finalize_selfdestructs() else ();

  struct { rc_success = success, rc_gas_used = to_gas(gas_used), rc_logs = k_logs }
}
