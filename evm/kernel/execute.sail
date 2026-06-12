/* ======================================================================== */
/* EVM USER-SPACE instruction semantics: execute(op)                         */
/*                                                                           */
/* Pure compute is done here; every world effect is a kernel syscall (k_*).   */
/* All gas/policy stays in the EVM: the EVM marks-and-prices access via the    */
/* kernel's returned warm bit, decides whether an effect happens, and issues  */
/* the syscall only for real effects (e.g. a no-op SSTORE charges gas but     */
/* issues no EL_SStore). Sub-calls/creates delegate to run_call/run_create.   */
/* ======================================================================== */

val run_call   : int  -> unit   /* 0=CALL 1=CALLCODE 2=DELEGATECALL 3=STATICCALL */
val run_create : bool -> unit   /* true = CREATE2 */

/* ----------------------------- helpers ---------------------------------- */

function self_addr() -> address = message.msg_address

function is_valid_jumpdest(c : list(byte), dest : int) -> bool = {
  let n = list_len(c);
  var i : int = 0;
  var result : bool = false;
  var done : bool = false;
  while (i < n) & not_bool(done) do {
    if i == dest then { result = (list_nth(c, i, 0x00) == 0x5b); done = true }
    else {
      let op = list_nth(c, i, 0x00);
      if (unsigned(op) >= 96) & (unsigned(op) <= 127) then i = i + 1 + (unsigned(op) - 95)
      else i = i + 1
    }
  };
  result
}

function guard_static() -> bool =
  if message.msg_is_static then { exc_halt(WriteProtection); true } else false

function do_jump(dest : int) -> unit =
  if is_valid_jumpdest(code, dest) then pc = dest else exc_halt(InvalidJump)

/* ------------------------------- execute -------------------------------- */

function execute(op : ast) -> unit = match op {

  OP_STOP() => frame_status = Halted(HaltStop),

  /* ---- arithmetic (pure) ---- */
  OP_ADD()  => { charge(G_verylow); let a = pop(); let b = pop(); push(alu_add(a, b)) },
  OP_MUL()  => { charge(G_low);     let a = pop(); let b = pop(); push(alu_mul(a, b)) },
  OP_SUB()  => { charge(G_verylow); let a = pop(); let b = pop(); push(alu_sub(a, b)) },
  OP_DIV()  => { charge(G_low);     let a = pop(); let b = pop(); push(alu_div(a, b)) },
  OP_SDIV() => { charge(G_low);     let a = pop(); let b = pop(); push(alu_sdiv(a, b)) },
  OP_MOD()  => { charge(G_low);     let a = pop(); let b = pop(); push(alu_mod(a, b)) },
  OP_SMOD() => { charge(G_low);     let a = pop(); let b = pop(); push(alu_smod(a, b)) },
  OP_ADDMOD() => { charge(G_mid); let a = pop(); let b = pop(); let n = pop(); push(alu_addmod(a, b, n)) },
  OP_MULMOD() => { charge(G_mid); let a = pop(); let b = pop(); let n = pop(); push(alu_mulmod(a, b, n)) },
  OP_EXP()  => { let a = pop(); let e = pop(); charge(exp_gas(e)); push(alu_exp(a, e)) },
  OP_SIGNEXTEND() => { charge(G_low); let bi = pop(); let v = pop(); push(alu_signextend(bi, v)) },

  /* ---- comparison / bitwise (pure) ---- */
  OP_LT()  => { charge(G_verylow); let a = pop(); let b = pop(); push(alu_lt(a, b)) },
  OP_GT()  => { charge(G_verylow); let a = pop(); let b = pop(); push(alu_gt(a, b)) },
  OP_SLT() => { charge(G_verylow); let a = pop(); let b = pop(); push(alu_slt(a, b)) },
  OP_SGT() => { charge(G_verylow); let a = pop(); let b = pop(); push(alu_sgt(a, b)) },
  OP_EQ()  => { charge(G_verylow); let a = pop(); let b = pop(); push(alu_eq(a, b)) },
  OP_ISZERO() => { charge(G_verylow); let a = pop(); push(alu_iszero(a)) },
  OP_AND() => { charge(G_verylow); let a = pop(); let b = pop(); push(alu_and(a, b)) },
  OP_OR()  => { charge(G_verylow); let a = pop(); let b = pop(); push(alu_or(a, b)) },
  OP_XOR() => { charge(G_verylow); let a = pop(); let b = pop(); push(alu_xor(a, b)) },
  OP_NOT() => { charge(G_verylow); let a = pop(); push(alu_not(a)) },
  OP_BYTE() => { charge(G_verylow); let i = pop(); let x = pop(); push(alu_byte(i, x)) },
  OP_SHL() => { charge(G_verylow); let s = pop(); let v = pop(); push(alu_shl(s, v)) },
  OP_SHR() => { charge(G_verylow); let s = pop(); let v = pop(); push(alu_shr(s, v)) },
  OP_SAR() => { charge(G_verylow); let s = pop(); let v = pop(); push(alu_sar(s, v)) },
  OP_CLZ() => { charge(G_low); let x = pop(); push(alu_clz(x)) },

  /* ---- keccak: hash via kernel primitive ---- */
  OP_KECCAK256() => { let off = pop_int(); let len = pop_int();
                      charge(keccak_gas(len)); charge_expansion(off, len);
                      if is_running() then push(k_keccak(mem_read_range(off, len))) else () },

  /* ---- environment / account (kernel reads, EVM prices them) ---- */
  OP_ADDRESS()   => { charge(G_base); push(address_to_word(self_addr())) },
  OP_ORIGIN()    => { charge(G_base); push(k_env(F_Origin)) },
  OP_CALLER()    => { charge(G_base); push(address_to_word(message.msg_caller)) },
  OP_CALLVALUE() => { charge(G_base); push(message.msg_value) },
  OP_GASPRICE()  => { charge(G_base); push(k_env(F_GasPrice)) },
  OP_CALLDATASIZE() => { charge(G_base); push(word_of_int(list_len(message.msg_data))) },
  OP_CALLDATALOAD() => { charge(G_verylow); let off = pop_int();
                         push(bytes_load_word(message.msg_data, off)) },
  OP_CALLDATACOPY() => { charge(G_verylow); let dest = pop_int(); let off = pop_int();
                         let len = pop_int(); charge(copy_gas(len)); charge_expansion(dest, len);
                         if is_running() then mem_write_range(dest, bytes_slice(message.msg_data, off, len)) else () },
  OP_CODESIZE()  => { charge(G_base); push(word_of_int(code_len)) },
  OP_CODECOPY()  => { charge(G_verylow); let dest = pop_int(); let off = pop_int();
                      let len = pop_int(); charge(copy_gas(len)); charge_expansion(dest, len);
                      if is_running() then mem_write_range(dest, bytes_slice(code, off, len)) else () },
  OP_BALANCE()   => { let a = word_to_address(pop()); let warm = k_access_account(a);
                      charge(account_cost(warm)); push(k_get_balance(a)) },
  OP_SELFBALANCE() => { charge(G_low); push(k_get_balance(self_addr())) },
  OP_EXTCODESIZE() => { let a = word_to_address(pop()); let warm = k_access_account(a);
                        charge(account_cost(warm)); push(word_of_int(list_len(k_get_code(a)))) },
  OP_EXTCODECOPY() => { let a = word_to_address(pop()); let dest = pop_int();
                        let off = pop_int(); let len = pop_int();
                        let warm = k_access_account(a); charge(account_cost(warm));
                        charge(copy_gas(len)); charge_expansion(dest, len);
                        if is_running() then mem_write_range(dest, bytes_slice(k_get_code(a), off, len)) else () },
  OP_EXTCODEHASH() => { let a = word_to_address(pop()); let warm = k_access_account(a);
                        charge(account_cost(warm)); push(k_get_codehash(a)) },
  OP_RETURNDATASIZE() => { charge(G_base); push(word_of_int(list_len(returndata))) },
  OP_RETURNDATACOPY() => { charge(G_verylow); let dest = pop_int(); let off = pop_int();
                           let len = pop_int(); charge(copy_gas(len)); charge_expansion(dest, len);
                           if off + len > list_len(returndata) then exc_halt(InvalidOpcode)
                           else if is_running() then mem_write_range(dest, bytes_slice(returndata, off, len)) else () },

  /* ---- block (kernel reads) ---- */
  OP_BLOCKHASH()  => { charge(20); let n = pop(); push(k_blockhash(n)) },
  OP_COINBASE()   => { charge(G_base); push(k_env(F_Coinbase)) },
  OP_TIMESTAMP()  => { charge(G_base); push(k_env(F_Timestamp)) },
  OP_NUMBER()     => { charge(G_base); push(k_env(F_Number)) },
  OP_PREVRANDAO() => { charge(G_base); push(k_env(F_PrevRandao)) },
  OP_GASLIMIT()   => { charge(G_base); push(k_env(F_GasLimit)) },
  OP_CHAINID()    => { charge(G_base); push(k_env(F_ChainId)) },
  OP_BASEFEE()    => { charge(G_base); push(k_env(F_BaseFee)) },
  OP_BLOBBASEFEE()=> { charge(G_base); push(k_env(F_BlobBaseFee)) },
  OP_BLOBHASH()   => { charge(G_verylow); let i = pop_int(); push(k_blobhash(i)) },

  /* ---- stack / memory (pure, user-space) ---- */
  OP_POP()    => { charge(G_base); let _ = pop() },
  OP_MLOAD()  => { charge(G_verylow); let off = pop_int(); charge_expansion(off, 32); push(mem_load(off)) },
  OP_MSTORE() => { charge(G_verylow); let off = pop_int(); let v = pop();
                   charge_expansion(off, 32); mem_store(off, v) },
  OP_MSTORE8()=> { charge(G_verylow); let off = pop_int(); let v = pop();
                   charge_expansion(off, 1); mem_store_byte(off, v) },
  OP_MSIZE()  => { charge(G_base); push(word_of_int(mem_size_words() * 32)) },
  OP_MCOPY()  => { charge(G_verylow); let dest = pop_int(); let src = pop_int(); let len = pop_int();
                   charge(copy_gas(len)); let hi = if dest > src then dest else src;
                   charge_expansion(hi, len);
                   if is_running() then mem_write_range(dest, mem_read_range(src, len)) else () },

  /* ---- storage: gas/decision in EVM, effect in kernel ---- */
  OP_SLOAD()  => { let s = pop(); let warm = k_access_slot(self_addr(), s);
                   charge(sload_cost(warm)); push(k_sload(self_addr(), s)) },
  OP_SSTORE() => { if guard_static() then () else {
                     if gas_remaining <= G_callstipend then exc_halt(OutOfGas)  /* EIP-1706 */
                     else {
                       let s = pop(); let v = pop();
                       let warm = k_access_slot(self_addr(), s);   /* kernel: access accounting */
                       let current = k_sload(self_addr(), s);      /* kernel: read current      */
                       let original = k_orig(self_addr(), s);      /* kernel: tx-original        */
                       let (cost, refund) = sstore_gas(original, current, v, not_bool(warm)); /* USER policy */
                       charge(cost);
                       if refund != 0 then k_refund_add(refund) else ();
                       if current != v then k_sstore(self_addr(), s, v) else ()  /* effect ONLY if it changes */
                     }
                   } },
  OP_TLOAD()  => { charge(G_warm_access); let s = pop(); push(k_tload(self_addr(), s)) },
  OP_TSTORE() => { if guard_static() then () else {
                     charge(G_warm_access); let s = pop(); let v = pop();
                     k_tstore(self_addr(), s, v) } },

  /* ---- flow (pure over own code) ---- */
  OP_JUMP()   => { charge(G_mid); let dest = pop_int(); do_jump(dest) },
  OP_JUMPI()  => { charge(G_high); let dest = pop_int(); let cond = pop();
                   if word_is_zero(cond) then () else do_jump(dest) },
  OP_PC()     => { charge(G_base); push(word_of_int(pc - 1)) },
  OP_GAS()    => { charge(G_base); push(word_of_int(gas_remaining)) },
  OP_JUMPDEST() => charge(G_jumpdest),

  /* ---- push / dup / swap (pure) ---- */
  OP_PUSH(n, v) => { if n == 0 then charge(G_base) else charge(G_verylow); push(v) },
  OP_DUP(n)     => { charge(G_verylow); push(peek(n - 1)) },
  OP_SWAP(n)    => { charge(G_verylow); let top = peek(0); let other = peek(n);
                     stack = list_set(list_set(stack, 0, other), n, top) },

  /* ---- log: EVM checks/prices, kernel records ---- */
  OP_LOG(n) => { if guard_static() then () else {
                   let off = pop_int(); let len = pop_int();
                   var topics : list(word) = [||];
                   foreach (k from 0 to (n - 1)) { topics = list_append(topics, [| pop() |]) };
                   charge(log_gas(n, len)); charge_expansion(off, len);
                   if is_running() then k_log(self_addr(), topics, mem_read_range(off, len)) else () } },

  /* ---- system ---- */
  OP_CREATE()       => run_create(false),
  OP_CREATE2()      => run_create(true),
  OP_CALL()         => run_call(0),
  OP_CALLCODE()     => run_call(1),
  OP_DELEGATECALL() => run_call(2),
  OP_STATICCALL()   => run_call(3),

  OP_RETURN() => { let off = pop_int(); let len = pop_int(); charge_expansion(off, len);
                   if is_running() then { frame_output = mem_read_range(off, len); frame_status = Halted(HaltReturn) } else () },
  OP_REVERT() => { let off = pop_int(); let len = pop_int(); charge_expansion(off, len);
                   if is_running() then { frame_output = mem_read_range(off, len); frame_status = Halted(HaltRevert) } else () },
  OP_INVALID() => { gas_remaining = 0; exc_halt(InvalidOpcode) },

  OP_SELFDESTRUCT() => { if guard_static() then () else {
                           let beneficiary = word_to_address(pop());
                           let bal = k_get_balance(self_addr());
                           let warm = k_access_account(beneficiary);
                           charge(G_selfdestruct);
                           if not_bool(warm) then charge(G_cold_account) else ();   /* EIP-2929 */
                           if word_nonzero(bal) & k_account_is_empty(beneficiary)     /* new account */
                           then charge(G_newaccount) else ();
                           k_transfer(self_addr(), beneficiary, bal);
                           /* EIP-6780: only a created-this-tx originator is registered for
                            * deletion, and its balance is forced to 0 after the transfer
                            * (burning a self-beneficiary transfer). A pre-existing account
                            * just moves its balance and persists. */
                           if k_was_created(self_addr()) then {
                             k_zero_balance(self_addr());
                             k_selfdestruct(self_addr())
                           } else ();
                           frame_status = Halted(HaltSelfDestruct) } }
}
