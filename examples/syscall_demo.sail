/* ======================================================================== */
/* EL-IR user/kernel demo: EVM (user space) issues EL syscalls (kernel)      */
/*                                                                           */
/*   EVM  = user space : pure compute + ALL policy (gas schedule, warm/cold, */
/*                       EIP-2200/2929 accounting, the DECISION of whether    */
/*                       an effect happens). The gas counter lives here.      */
/*   EL   = kernel/host: privileged primitives only (read/write a slot,       */
/*                       mark-warm, keccak, log). No gas, no policy.          */
/*                                                                           */
/* The EVM emits an EL syscall ONLY for a real effect. Key behaviour shown:   */
/* an SSTORE that writes the value already present charges gas in user space  */
/* but issues NO kernel write (no EL_SStore) -- "SSTORE need not store".      */
/* ======================================================================== */

/* prelude_evm.sail sets `default Order dec` + pulls <prelude.sail>/<smt.sail>. */
$include "../sail/prelude_evm.sail"
$include <string.sail>

/* ----------------------- EL kernel ISA (the syscalls) ------------------- */
/* Each is a privileged primitive. They are RECORDED so the EL trace is the  */
/* exact sequence of authenticated effects (= the AIR channel sends).        */
union ELOp = {
  EL_AccessSlot : word,          /* mark a slot warm; return prior warm bit  */
  EL_SLoad      : word,          /* read a slot                              */
  EL_SStore     : (word, word),  /* write a slot (emitted ONLY on real write)*/
  EL_Keccak     : word,
  EL_Log        : word
}

/* ------------------------------ kernel state ---------------------------- */
struct SCell = { sc_slot : word, sc_val : word }
register k_storage : list(SCell) = [||]
register k_warm    : list(word)  = [||]
register el_trace  : list(ELOp)  = [||]
register el_writes : int         = 0    /* count of real EL_SStore syscalls  */

function s_lookup(cs : list(SCell), s : word) -> word =
  match cs { [||] => word_of_int(0), c :: r => if c.sc_slot == s then c.sc_val else s_lookup(r, s) }
function s_update(cs : list(SCell), s : word, v : word) -> list(SCell) =
  match cs { [||] => [| struct { sc_slot = s, sc_val = v } |],
             c :: r => if c.sc_slot == s then struct { sc_slot = s, sc_val = v } :: r
                       else c :: s_update(r, s, v) }
function warm_in(cs : list(word), s : word) -> bool =
  match cs { [||] => false, x :: r => if x == s then true else warm_in(r, s) }

/* kernel syscall handlers: mechanism only, each records to the EL trace */
function k_access_slot(s : word) -> bool = {
  el_trace = list_append(el_trace, [| EL_AccessSlot(s) |]);
  let prior = warm_in(k_warm, s);
  if not_bool(prior) then k_warm = s :: k_warm else ();
  prior
}
function k_sload(s : word) -> word = {
  el_trace = list_append(el_trace, [| EL_SLoad(s) |]);
  s_lookup(k_storage, s)
}
function k_sstore(s : word, v : word) -> unit = {
  el_trace = list_append(el_trace, [| EL_SStore(s, v) |]);
  el_writes = el_writes + 1;
  k_storage = s_update(k_storage, s, v)
}
/* the kernel current value, folded into SSTORE's effect (not a separate read) */
function k_peek(s : word) -> word = s_lookup(k_storage, s)

/* ----------------------- EVM user space (policy) ------------------------ */
register stack    : list(word) = [||]
register gas_used : gas        = 0

function push(w : word) -> unit = stack = w :: stack
function pop() -> word = match stack { [||] => word_of_int(0), w :: r => { stack = r; w } }
function charge(g : gas) -> unit = gas_used = gas_used + g

union Op = { O_STOP : unit, O_ADD : unit, O_PUSH : word, O_SLOAD : unit, O_SSTORE : unit }

/* The EVM opcode semantics. SLOAD/SSTORE do their accounting in USER SPACE
 * and call kernel syscalls only for the actual effect. */
function step(op : Op) -> unit = match op {
  O_STOP()  => (),
  O_PUSH(v) => { charge(3); push(v) },
  O_ADD()   => { charge(3); let a = pop(); let b = pop(); push(alu_add(a, b)) },

  O_SLOAD() => {
    let s = pop();
    let warm = k_access_slot(s);                 /* kernel: mark/return warm */
    charge(if warm then 100 else 2100);          /* USER-SPACE gas policy    */
    push(k_sload(s))                             /* kernel: the actual read  */
  },

  O_SSTORE() => {
    let s = pop();
    let v = pop();
    let warm = k_access_slot(s);                 /* kernel: access accounting */
    let current = k_peek(s);                     /* kernel current value      */
    /* USER-SPACE EIP-2200/2929 gas policy */
    let base : gas = if current == v then 100
                     else if word_is_zero(current) then 20000 else 2900;
    charge(base + (if warm then 0 else 2100));
    /* USER-SPACE DECISION: only issue the kernel write if it actually changes */
    if current == v then ()                      /* no EL_SStore syscall      */
    else k_sstore(s, v)                          /* kernel: the actual write  */
  }
}

/* ------------------------------ run + print ----------------------------- */
function op_name(op : Op) -> string = match op {
  O_STOP() => "STOP", O_ADD() => "ADD",
  O_PUSH(v) => concat_str("PUSH ", dec_str(unsigned(v))),
  O_SLOAD() => "SLOAD", O_SSTORE() => "SSTORE"
}
function elop_name(e : ELOp) -> string = match e {
  EL_AccessSlot(s) => concat_str("EL_AccessSlot ", dec_str(unsigned(s))),
  EL_SLoad(s)      => concat_str("EL_SLoad ", dec_str(unsigned(s))),
  EL_SStore(s, v)  => concat_str("EL_SStore ", concat_str(dec_str(unsigned(s)),
                        concat_str(" <- ", dec_str(unsigned(v))))),
  EL_Keccak(_)     => "EL_Keccak",
  EL_Log(_)        => "EL_Log"
}
function print_el_from(idx : int) -> unit = {
  let n = list_len(el_trace);
  var i : int = idx;
  while i < n do {
    print_endline(concat_str("              syscall-> ",
      elop_name(list_nth(el_trace, i, EL_Log(word_of_int(0))))));
    i = i + 1
  }
}
function run(prog : list(Op)) -> unit = match prog {
  [||]      => (),
  op :: rest => {
    let before = list_len(el_trace);
    print_endline(concat_str("  [EVM] ", op_name(op)));
    step(op);
    print_el_from(before);
    run(rest)
  }
}

function main() -> unit = {
  k_storage = [| struct { sc_slot = word_of_int(0), sc_val = word_of_int(41) } |];
  k_warm = [||]; el_trace = [||]; el_writes = 0; stack = [||]; gas_used = 0;

  /* PUSH 0; SLOAD; PUSH 1; ADD; PUSH 0; SSTORE   (41 -> 42, REAL write)
   * PUSH 42; PUSH 0; SSTORE                       (42 -> 42, NO write)
   * STOP */
  let prog : list(Op) = [|
    O_PUSH(word_of_int(0)), O_SLOAD(),
    O_PUSH(word_of_int(1)), O_ADD(),
    O_PUSH(word_of_int(0)), O_SSTORE(),
    O_PUSH(word_of_int(42)), O_PUSH(word_of_int(0)), O_SSTORE(),
    O_STOP()
  |];

  print_endline("EVM (user space) executes; emits EL syscalls only for real effects:");
  print_endline("");
  run(prog);
  print_endline("");
  print_endline(concat_str("  storage[0] after      = ", dec_str(unsigned(k_peek(word_of_int(0))))));
  print_endline(concat_str("  gas used (in EVM)     = ", dec_str(gas_used)));
  print_endline(concat_str("  real EL_SStore writes = ", dec_str(el_writes)));
  print_endline("");
  print_endline("  Note: TWO SSTOREs ran, both charged gas in the EVM, but only the");
  print_endline("  first issued a kernel write. The no-op SSTORE emitted no EL_SStore.");

  assert(unsigned(k_peek(word_of_int(0))) == 42, "slot 0 should be 42");
  /* PUSH 3 + SLOAD cold 2100 + PUSH 3 + ADD 3 + PUSH 3 + SSTORE warm-dirty 2900
   * + PUSH 3 + PUSH 3 + SSTORE warm no-op 100 = 5118 */
  assert(gas_used == 5118, "user-space gas should total 5118");
  assert(el_writes == 1, "only ONE real kernel write despite two SSTOREs");
  print_endline("");
  print_endline("User/kernel (EVM/EL) split: all assertions passed.")
}
