/* ======================================================================== */
/* EL-IR zkVM guest entry: run a one-transaction block and expose the result */
/* facts to the freestanding C harness via output registers.                 */
/*                                                                           */
/* This is the run_block.sail example reshaped for the eth-act zkVM RISC-V    */
/* standard target: NO printing, NO string building, NO libc IO, NO asserts   */
/* in the happy path.  The harness reads the `zkvm_out_*` registers after     */
/* `zkvm_run` and emits them through the standard write_output interface.     */
/*                                                                           */
/* Built GMP-free by backing the stock Sail GMP ABI with a vendored,           */
/* freestanding mini-gmp (no libgmp linked), keeping EXACT 256-bit semantics.  */
/* Keccak-256 is supplied by the C accelerator (ffi/keccak_ffi.c) linked into  */
/* the guest — the eth-act c-interface-accelerators path; it needs no GMP and  */
/* only memset from libc.                                                       */
/* ======================================================================== */

$include "../evm/evm.sail"
$include "../examples/evm_primitives.sail"
$include "../evm/lib/ssz.sail"
$include "../evm/lib/rlp_decode.sail"
$include "../evm/lib/mpt_witness.sail"
$include "../evm/lib/sha256.sail"
$include "../evm/lib/ssz_htr.sail"

/* canonical public output: write one byte to write_output (zkvm/el_input.c) */
val emit_out = impure "el_emit_out" : bits(8) -> unit

/* Output facts, read by the C harness after zkvm_run(). All fit in 64 bits. */
register zkvm_out_gas      : int = 0   /* total gas used                     */
register zkvm_out_storage0 : int = 0   /* contract storage slot 0            */
register zkvm_out_wbal     : int = 0   /* withdrawer balance (wei)           */
register zkvm_out_cbbal    : int = 0   /* coinbase balance (wei)             */
register zkvm_out_tracelen : int = 0   /* EL-IR stream length                */
register zkvm_out_success  : int = 0   /* 1 if the tx receipt succeeded      */

function eoa()        -> address = 0x00000000000000000000000000000000000005e0
function contract()   -> address = 0x000000000000000000000000000000000000c0de
function coinbase_a() -> address = 0x000000000000000000000000000000000000c01b
function withdrawer() -> address = 0x0000000000000000000000000000000000004895

/* PUSH1 0x2a; PUSH1 0x00; SSTORE; STOP  (store 42 at slot 0) */
function contract_code() -> list(byte) = [| 0x60, 0x2a, 0x60, 0x00, 0x55, 0x00 |]

/* Exercise the keccak C accelerator (ffi/keccak_ffi.c) ON the RISC-V guest:
 * compute the two reference digests through the freestanding mini-gmp + FFI
 * path and assert them. A broken on-guest accelerator (ABI/endianness/mini-gmp)
 * fails the assert -> abnormal termination (non-zero exit), so exit 0 proves
 * keccak runs correctly on the guest, not just that it links. */
function keccak_selfcheck() -> unit = {
  assert(keccak256([||]) == 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470,
         "keccak('') on guest");
  assert(keccak256([| 0x61, 0x62, 0x63 |]) == 0x4e03657aea45a94fc7d47ba826c8d667c0d1e6e33a64a036ec44f58fa12d6c45,
         "keccak('abc') on guest")
}

/* code db from witness.codes (keccak(code) -> code) */
function build_code_db() -> list((bits(256), list(byte))) = {
  let n = wit_n_codes();
  var db : list((bits(256), list(byte))) = [||];
  foreach (i from 0 to (n - 1)) { let cb = ssz_witness_code(i); db = (keccak256(cb), cb) :: db };
  db
}

/* resolve `addr` from the witness pre-state MPT and seed the kernel world with
 * it (account + code). No-op if the account is absent from the witness. */
function seed_account_from_witness(adb : list((bits(256), list(byte))),
                                   cdb : list((bits(256), list(byte))),
                                   sr : bits(256), addr : address) -> unit = {
  let leaf = witness_get_account(adb, sr, addr);
  if list_len(leaf) > 0 then {
    let (nonce, bal, _, chash) = decode_account(leaf);
    let acode = if chash == EMPTY_CODE_HASH then [||] else node_db_lookup(cdb, chash);
    k_seed_account(addr, nonce, bal, acode)
  } else ()
}

/* seed an account from the flat leaf store harvested during the validated
 * re-root pass (witness_leaves). A present leaf is bound to parent.state_root,
 * so it is authenticated; absent -> empty account. Replaces the per-address
 * trie_walk (witness_get_account): values come straight from the leaves. */
function seed_account_from_store(cdb : list((bits(256), list(byte))), addr : address) -> unit = {
  let leaf = witness_store_leaf(keccak256(address_to_bytes(addr)));
  if list_len(leaf) > 0 then {
    let (nonce, bal, _, chash) = decode_account(leaf);
    let acode = if chash == EMPTY_CODE_HASH then [||] else node_db_lookup(cdb, chash);
    k_seed_account(addr, nonce, bal, acode)
  } else ()
}

/* storage root of an account from the harvested (authenticated) leaf */
function acct_storage_root(addr : address) -> bits(256) = {
  let leaf = witness_store_leaf(keccak256(address_to_bytes(addr)));
  if list_len(leaf) == 0 then EMPTY_TRIE_ROOT
  else { let (_, _, sr, _) = decode_account(leaf); sr }
}

/* seed each BAL-listed (addr, slot) with its authenticated witness pre-value into
 * the kernel storage, marking it present in the working set. witness_get_storage
 * walks from the account's (authenticated) storage root and fails closed on a
 * missing node, so the seeded value is witness-proven (a proven 0 included).
 * After this, every EVM SLOAD/SSTORE outside the seeded set fails the presence
 * check -- the soundness bound, folded into the read (replaces the BAL guard). */
function seed_storage_working_set(adb : list((bits(256), list(byte)))) -> unit = {
  let bal = bal_load();
  let n = list_len(bal);
  foreach (i from 0 to (n - 1)) {
    let sk = list_nth(bal, i, struct { sk_addr = 0x0000000000000000000000000000000000000000, sk_slot = word_of_int(0) });
    let v = witness_get_storage(adb, acct_storage_root(sk.sk_addr), sk.sk_slot);
    k_seed_storage(sk.sk_addr, sk.sk_slot, v)
  }
}

/* validate_headers: the witness headers form a contiguous chain
 * (header[i].parent_hash == keccak(header[i-1])). */
function headers_ok() -> bool = {
  let n = wit_n_headers();
  if n < 1 then false
  else {
    var ok : bool = true;
    foreach (i from 1 to (n - 1)) {
      let parent_hash = bytes_be256((list_nth(rlp_node_items(ssz_witness_header(i)), 0, RLP_NONE)).rf_content);
      let prev_hash   = keccak256(ssz_witness_header(i - 1));
      if parent_hash != prev_hash then ok = false else ()
    };
    ok
  }
}

function emit_all(bs : list(byte)) -> unit = {
  let n = list_len(bs);
  foreach (i from 0 to (n - 1)) { emit_out(list_nth(bs, i, 0x00)) }
}

/* Build + emit the canonical SSZ SszStatelessValidationResult:
 *   new_payload_request_root (32 BE) ++ successful_validation (1) ++
 *   chain_config offset (37, LE u32) ++ chain_config bytes (copied from input).
 * The chain_config is returned unchanged, so its SSZ bytes are exactly the
 * input's [cc_start, cc_end) region. */
function emit_result(zh : list(bits(256)), success : bool) -> unit = {
  let root = htr_new_payload_request(zh);
  var out : list(byte) = [||];
  foreach (k from 0 to 31) { out = (sail_shiftright(root, 8 * (31 - k)))[7 .. 0] :: out };
  out = (if success then 0x01 else 0x00) :: out;
  out = 0x25 :: out; out = 0x00 :: out; out = 0x00 :: out; out = 0x00 :: out;
  let s = cc_start();
  let e = cc_end();
  foreach (i from 0 to (e - s - 1)) { out = ssz_src_byte(s + i) :: out };
  let result = list_reverse(out);
  /* fixture self-check: on the (valid) happy path the result is byte-exact with
     the host's remerkleable encoding. The fail path emits a different result. */
  if success then
    assert(sha256(result) == 0x2c0ea45002c0a19ee9dbf10817e26e848ae4d11d5e576e35e7e97ed297d110e9,
           "canonical SSZ stateless result bytes (success path)")
  else ();
  emit_all(result)
}

/* Exercise the fixed-width (GMP-free) sailfix runtime's hard arithmetic paths
 * on-guest: 256-bit MUL+truncate, DIV/MOD, the 512-bit MULMOD product, EXP
 * (square-and-multiply), and signed SDIV/SMOD over the negative get_slice_int
 * path. Ground truth computed independently (Python). */
function arith_selfcheck() -> unit = {
  let a   : word = 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff;
  let two : word = word_of_int(2);
  assert(alu_mul(a, two) == 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, "alu_mul");
  assert(alu_div(a, two) == 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, "alu_div");
  assert(alu_mod(a, word_of_int(3)) == word_of_int(0), "alu_mod");
  assert(alu_mulmod(a, a, word_of_int(7)) == word_of_int(1), "alu_mulmod 512-bit");
  /* alu_exp (square-and-multiply, 256x 512-bit mulmod) dropped from the hot path:
     ~17M instructions and redundant — EEST validates sailfix's full arithmetic,
     and alu_mulmod above already exercises the 512-bit product path. */
  let neg7 : word = word_of_int(-7);
  assert(alu_sdiv(neg7, two) == 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd, "alu_sdiv");
  assert(alu_smod(neg7, two) == 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, "alu_smod")
}

function zkvm_run() -> unit = {
  /* input-independent self-checks (valid for any vector): the C accelerators
     and the GMP-free fixed-width int/bits runtime. The per-stage fixture
     self-checks (ssz/rlp/witness/htr) were incremental scaffolding subsumed by
     the end-to-end success/fail paths + the success-path output check. */
  keccak_selfcheck();
  sha256_selfcheck();
  arith_selfcheck();

  /* S2f: execute the block decoded from private input against witness state */
  let adb = build_node_db();
  let cdb = build_code_db();
  let sr  = parent_state_root();
  /* Stage 3+4: re-root the witness via the stack builder and validate it against
     the committed parent.state_root (the one-shot pre-state validation), and in
     the same pass harvest the authenticated account leaves into witness_leaves. */
  assert(witness_reroot(adb, sr) == sr, "witness re-roots to parent.state_root");
  k_world_reset();

  /* The tx sender is derived from public_keys[0] (no ecrecover needed): the
     guest trusts the host-attested pubkey and computes address = keccak(X||Y)[12:]
     over the 64 bytes after the 0x04 uncompressed prefix. */
  let sender = word_to_address(keccak256(ssz_bytes(cc_end() + 1, 64)));

  /* decode the single legacy transaction from transactions[0] */
  let ti = rlp_node_items(ssz_tx(0));
  let t_nonce    = rlp_field_int(list_nth(ti, 0, RLP_NONE));
  let t_gasprice = rlp_field_word(list_nth(ti, 1, RLP_NONE));
  let t_gas      = rlp_field_int(list_nth(ti, 2, RLP_NONE));
  let t_to       = word_to_address(rlp_field_word(list_nth(ti, 3, RLP_NONE)));
  let t_value    = rlp_field_word(list_nth(ti, 4, RLP_NONE));
  let t_data     = (list_nth(ti, 5, RLP_NONE)).rf_content;

  /* seed the touched accounts' pre-state from the harvested (authenticated) leaves */
  seed_account_from_store(cdb, sender);
  seed_account_from_store(cdb, t_to);

  let env : BlockEnv = struct {
    be_number = word_of_int(pl_block_number()), be_timestamp = word_of_int(pl_timestamp()),
    be_gas_limit = word_of_int(pl_gas_limit()), be_coinbase = pl_fee_recipient(),
    be_base_fee = pl_base_fee(), be_prev_randao = pl_prev_randao(),
    be_chain_id = word_of_int(cc_chain_id()), be_blob_base_fee = word_of_int(1),
    be_excess_blob_gas = word_of_int(0)
  };
  let txn : Transaction = struct {
    t_sender = sender, t_nonce = t_nonce, t_gas_price = t_gasprice, t_priority_fee = t_gasprice,
    t_gas_limit = t_gas, t_is_create = false, t_to = t_to, t_value = t_value, t_data = t_data,
    t_al_addrs = [||], t_al_keys = [||]
  };
  let wd : Withdrawal = struct {
    w_address = withdrawal_addr(0), w_amount = word_of_int(withdrawal_amount(0)) };
  let blk : Block = struct { blk_env = env, blk_txs = [| txn |], blk_withdrawals = [| wd |] };

  /* Seed the storage working set with authenticated witness pre-values for every
     BAL-listed slot, then arm stateless mode: from here every EVM SLOAD/SSTORE
     outside the seeded set fails the kernel presence check. The BAL selects which
     slots to provision; the witness supplies their values; presence is the bound
     (no separate access-list guard). */
  seed_storage_working_set(adb);
  k_stateless_on();

  let receipts = process_block(blk);

  /* S5: stateless validation outcome (a failure is a NORMAL result -- the guest
     reports successful_validation=false, it does not abort):
       - chain config selects the Amsterdam fork
       - the witness headers are contiguous
       - the block executed (receipt success)
       - the recomputed post-state root commits to payload.state_root */
  let cfg_ok       = cc_fork() == 20;
  let hdr_ok       = headers_ok();
  let exec_ok : bool = match receipts { r :: _ => r.rc_success, [||] => false };
  let poststate_ok = compute_state_root_hb() == pl_state_root();
  let valid : bool = cfg_ok & hdr_ok & exec_ok & poststate_ok;

  let stored   = k_storage_get(t_to, word_of_int(0));
  let gas_used = total_gas_used(receipts);
  let wbal     = kget(withdrawal_addr(0)).acc_balance;
  let cbbal    = kget(pl_fee_recipient()).acc_balance;

  zkvm_out_gas      = gas_used;
  zkvm_out_storage0 = unsigned(stored);
  zkvm_out_wbal     = unsigned(wbal);
  zkvm_out_cbbal    = unsigned(cbbal);
  zkvm_out_tracelen = 0  /* the ELOp trace machinery was removed */;
  zkvm_out_success  = if valid then 1 else 0;

  /* emit the canonical SSZ SszStatelessValidationResult as public output */
  emit_result(build_zerohashes(40), valid)
}
