/* ======================================================================== */
/* EVM USER-SPACE interpreter: fetch/decode, run loop, sub-call/create        */
/*                                                                           */
/* run_call/run_create save USER-SPACE frame registers and use kernel         */
/* syscalls for all world effects: k_snapshot on entry, k_get_code to fetch   */
/* the callee code, k_transfer for value, and k_commit / k_revert on exit     */
/* (the kernel rolls the world back atomically on a reverting child).         */
/* ======================================================================== */

let DEPTH_LIMIT : int = 1024

/* ------------------------------ fetch ----------------------------------- */

function read_push(n : int) -> word = {
  var acc : word = word_of_int(0);
  foreach (k from 0 to (n - 1)) {
    acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(code_byte(pc + k), 256))
  };
  acc
}

function decode_simple(b : int) -> ast =
  if      b >= 128 & b <= 143 then DUP(b - 127)
  else if b >= 144 & b <= 159 then SWAP(b - 143)
  else if b >= 160 & b <= 164 then LOG(b - 160)
  else match b {
    0   => STOP(),   1  => ADD(),    2  => MUL(),    3  => SUB(),
    4   => DIV(),    5  => SDIV(),   6  => MOD(),    7  => SMOD(),
    8   => ADDMOD(), 9  => MULMOD(), 10 => EXP(),    11 => SIGNEXTEND(),
    16  => LT(),     17 => GT(),     18 => SLT(),    19 => SGT(),
    20  => EQ(),     21 => ISZERO(), 22 => AND(),    23 => OR(),
    24  => XOR(),    25 => NOT(),    26 => BYTE(),   27 => SHL(),
    28  => SHR(),    29 => SAR(),    30 => CLZ(),
    32  => KECCAK256(),
    48  => ADDRESS(),    49 => BALANCE(),    50 => ORIGIN(),
    51  => CALLER(),     52 => CALLVALUE(),  53 => CALLDATALOAD(),
    54  => CALLDATASIZE(),55 => CALLDATACOPY(),56 => CODESIZE(),
    57  => CODECOPY(),   58 => GASPRICE(),   59 => EXTCODESIZE(),
    60  => EXTCODECOPY(),61 => RETURNDATASIZE(),62 => RETURNDATACOPY(),
    63  => EXTCODEHASH(),
    64  => BLOCKHASH(),  65 => COINBASE(),   66 => TIMESTAMP(),
    67  => NUMBER(),     68 => PREVRANDAO(), 69 => GASLIMIT(),
    70  => CHAINID(),    71 => SELFBALANCE(),72 => BASEFEE(),
    73  => BLOBHASH(),   74 => BLOBBASEFEE(),
    80  => POP(),        81 => MLOAD(),      82 => MSTORE(),
    83  => MSTORE8(),    84 => SLOAD(),      85 => SSTORE(),
    86  => JUMP(),       87 => JUMPI(),      88 => PC(),
    89  => MSIZE(),      90 => GAS(),        91 => JUMPDEST(),
    92  => TLOAD(),      93 => TSTORE(),     94 => MCOPY(),
    240 => CREATE(),     241 => CALL(),      242 => CALLCODE(),
    243 => RETURN(),     244 => DELEGATECALL(),245 => CREATE2(),
    250 => STATICCALL(), 253 => REVERT(),    255 => SELFDESTRUCT(),
    _   => INVALID()
  }

function fetch() -> ast = {
  if pc >= code_len then { pc = pc + 1; STOP() }
  else {
    let b = unsigned(code_byte(pc));
    pc = pc + 1;
    if b >= 95 & b <= 127 then { let n = b - 95; let v = read_push(n); pc = pc + n; PUSH(n, v) }
    else decode_simple(b)
  }
}

function interpret() -> unit =
  while is_running() do {
    let op = fetch();
    execute(op)
  }

function frame_succeeded() -> bool =
  match frame_status {
    Halted(HaltRevert) => false, Halted(_) => true, Running() => true, Exceptional(_) => false
  }

/* ------------------------------ run_call -------------------------------- */

function run_call(mode : int) -> unit = {
  let caller = self_addr();
  let gas_req = pop_int();
  let target  = word_to_address(pop());
  let value   = if mode == 0 | mode == 1 then pop() else word_of_int(0);
  let args_off = pop_int(); let args_len = pop_int();
  let ret_off  = pop_int(); let ret_len  = pop_int();

  /* memory is expanded once to cover the larger of the args / return ranges */
  let m_args = if args_len > 0 then args_off + args_len else 0;
  let m_ret  = if ret_len  > 0 then ret_off  + ret_len  else 0;
  let m_max  = if m_args > m_ret then m_args else m_ret;
  if m_max > 0 then {
    charge(expansion_gas(m_max));
    if m_max > memory_size then memory_size = m_max else ()
  } else ();
  let warm = k_access_account(target);
  charge(account_cost(warm));
  /* EIP-7702: resolving a delegation designation charges the delegate's access */
  let (tg_deleg, tg_target) = deleg_target(k_get_code(target));
  if tg_deleg then { let dw = k_access_account(tg_target); charge(account_cost(dw)) } else ();
  if word_nonzero(value) then {
    charge(G_callvalue);
    if (mode == 0) & k_account_is_empty(target) then charge(G_newaccount) else ()
  } else ();

  let avail = gas_remaining;
  let base_child = call_gas_cap(avail, gas_req);
  let stipend = if word_nonzero(value) then G_callstipend else 0;
  gas_remaining = to_gas(avail - base_child);
  let child_gas = base_child + stipend;

  let args = if is_running() then mem_read_range(args_off, args_len) else [||];
  let an = unsigned(target);
  let depth_ok : bool = call_depth + 1 <= DEPTH_LIMIT;
  let balance_ok : bool =
    if (mode == 0 | mode == 1) & word_nonzero(value)
    then unsigned(k_get_balance(caller)) >= unsigned(value) else true;

  if not_bool(depth_ok) | not_bool(balance_ok) then {
    returndata = [||]; gas_remaining = to_gas(gas_remaining + child_gas); push(word_of_int(0))
  } else if (an >= 1 & an <= 17) | (an == 256) then {   /* 256 = 0x100 P256VERIFY (EIP-7951) */
    let used = precompile_gas(an, args);
    /* gas check FIRST: an out-of-gas precompile call must not execute (e.g. a
       blake2f with a huge round count would otherwise run before failing). */
    let (ok, out) : (bool, list(byte)) =
      if used <= child_gas then k_precompile(an, args) else (false, [||]);
    if ok & (used <= child_gas) then {
      returndata = out;
      /* the value transfer is part of the (successful) call */
      if (mode == 0) & word_nonzero(value) then k_transfer(caller, target, value) else ();
      let pout_n = if ret_len < list_len(out) then ret_len else list_len(out);
      mem_write_range(ret_off, bytes_slice(out, 0, pout_n));
      gas_remaining = to_gas(gas_remaining + (child_gas - used));
      push(word_of_int(1))
    } else {
      /* precompile failure (bad input) or OOG is a non-revert error: it consumes
         ALL the call's gas (no refund of child_gas), CALL returns 0, and the
         RETURNDATA buffer is EMPTY (no output escapes a failed precompile). */
      returndata = [||];
      push(word_of_int(0))
    }
  } else {
    k_snapshot();
    if (mode == 0) & word_nonzero(value) then k_transfer(caller, target, value) else ();

    let s_pc = pc; let s_gas = gas_remaining; let s_stack = stack; let s_code = code;
    let s_out = frame_output; let s_msg = message; let s_status = frame_status;
    let s_msize = memory_size; let s_depth = call_depth;

    let child_addr : address = if mode == 0 | mode == 3 then target else caller;
    let child_caller : address = if mode == 2 then s_msg.msg_caller else caller;
    let child_value : word = if mode == 2 then s_msg.msg_value else value;
    let child_static : bool = if mode == 3 then true else s_msg.msg_is_static;

    message = struct {
      msg_caller = child_caller, msg_address = child_addr, msg_code_address = target,
      msg_value = child_value, msg_data = args, msg_is_static = child_static, msg_depth = s_depth + 1
    };
    pc = 0; gas_remaining = to_gas(child_gas); stack = [||]; set_code(k_get_exec_code(target));  /* EIP-7702 delegation */
    returndata = [||]; frame_output = [||]; frame_status = Running(); mem_frame_enter(); memory_size = 0;
    call_depth = s_depth + 1;

    interpret();

    let succeeded = frame_succeeded();
    let child_out = frame_output; let child_left = gas_remaining;

    pc = s_pc; stack = s_stack; set_code(s_code); frame_output = s_out; message = s_msg;
    frame_status = s_status; mem_frame_leave(); memory_size = s_msize; call_depth = s_depth;
    returndata = child_out; gas_remaining = to_gas(s_gas + child_left);

    /* the output region is written on success AND revert (EELS generic_call) --
       and only min(ret_len, |output|) bytes: the rest of the region is untouched.
       An exceptionally-halted child has empty output, so nothing is written. */
    let out_n = if ret_len < list_len(child_out) then ret_len else list_len(child_out);
    mem_write_range(ret_off, bytes_slice(child_out, 0, out_n));
    if succeeded then { k_commit(); push(word_of_int(1)) }
    else { k_revert(); push(word_of_int(0)) }
  }
}

/* ----------------------------- run_create ------------------------------- */

function run_create(is2 : bool) -> unit = {
  let creator = self_addr();
  let value = pop(); let off = pop_int(); let len = pop_int();
  let salt  = if is2 then pop() else word_of_int(0);

  charge_expansion(off, len);
  charge(G_create); charge(initcode_gas(len));
  if is2 then charge(to_gas(G_keccak_word * ediv_int(len + 31, 32))) else ();
  let initcode = if is_running() then mem_read_range(off, len) else [||];

  if len > MAX_INITCODE_SIZE then exc_halt(InitCodeTooLarge)
  else {
    let nonce = k_get_nonce(creator);
    let new_addr : address =
      if is2 then k_create2_addr(creator, salt, k_keccak(initcode))
      else k_create_addr(creator, nonce);

    let avail = gas_remaining; let child_gas = avail - ediv_int(avail, 64);
    gas_remaining = to_gas(avail - child_gas);
    let depth_ok : bool = call_depth + 1 <= DEPTH_LIMIT;
    let balance_ok : bool = unsigned(k_get_balance(creator)) >= unsigned(value);

    if not_bool(depth_ok) | not_bool(balance_ok) then {
      returndata = [||]; gas_remaining = to_gas(gas_remaining + child_gas); push(word_of_int(0))
    } else {
      /* warm the target + bump the creator nonce BEFORE the child snapshot:
       * both persist even when the initcode reverts (EELS generic_create runs
       * them in the parent frame, before process_create_message). */
      let _ = k_access_account(new_addr);
      let occupied : bool = k_account_occupied(new_addr);
      returndata = [||];
      k_bump_nonce(creator);
      if occupied then {
        /* CREATE address collision (code, nonzero nonce, or storage at the
         * target): no initcode runs, 0 is pushed, and the child gas is NOT
         * returned (EELS generic_create keeps it deducted). */
        push(word_of_int(0))
      } else {
      k_snapshot();
      k_mark_created(new_addr);        /* EIP-6780: created this tx */
      k_bump_nonce(new_addr);          /* EIP-161: a created contract starts at nonce 1 */
      k_transfer(creator, new_addr, value);

      let s_pc = pc; let s_gas = gas_remaining; let s_stack = stack; let s_code = code;
      let s_out = frame_output; let s_msg = message; let s_status = frame_status;
      let s_msize = memory_size; let s_depth = call_depth;

      message = struct {
        msg_caller = creator, msg_address = new_addr, msg_code_address = new_addr,
        msg_value = value, msg_data = [||], msg_is_static = s_msg.msg_is_static, msg_depth = s_depth + 1
      };
      pc = 0; gas_remaining = to_gas(child_gas); stack = [||]; set_code(initcode);
      returndata = [||]; frame_output = [||]; frame_status = Running(); mem_frame_enter(); memory_size = 0;
      call_depth = s_depth + 1;

      interpret();

      let succeeded = frame_succeeded();
      let deployed = frame_output; let child_left = gas_remaining;
      let deposit = G_codedeposit * list_len(deployed);
      /* EIP-170 size limit + EIP-3541 (reject code starting with 0xEF) */
      let code_ok = (list_len(deployed) <= MAX_CODE_SIZE)
                  & (match deployed { b :: _ => b != 0xef, [||] => true });

      pc = s_pc; stack = s_stack; set_code(s_code); frame_output = s_out; message = s_msg;
      frame_status = s_status; mem_frame_leave(); memory_size = s_msize; call_depth = s_depth;
      returndata = if succeeded then [||] else deployed;
      gas_remaining = to_gas(s_gas + child_left);

      if succeeded & code_ok & (child_left >= deposit) then {
        gas_remaining = to_gas(gas_remaining - deposit); k_set_code(new_addr, deployed); k_commit();
        push(address_to_word(new_addr))
      } else {
        /* a deposit-stage failure (0xEF prefix, oversize code, deposit OOG)
         * is an exceptional halt: ALL child gas is consumed (EELS
         * process_create_message sets gas_left = 0). A reverting/failing
         * initcode keeps child_left as already folded in above. */
        if succeeded then gas_remaining = s_gas else ();
        k_revert(); push(word_of_int(0))
      }
      }
    }
  }
}
