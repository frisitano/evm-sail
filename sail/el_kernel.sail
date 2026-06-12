/* ======================================================================== */
/* EL-IR KERNEL (host space)                                                 */
/*                                                                           */
/* The kernel owns all privileged world state — accounts, storage, transient */
/* storage, the EIP-2929 warm sets, logs, the refund counter, world          */
/* snapshots, and the block/tx environment — and exposes it ONLY through the  */
/* EL syscall ISA (the `k_*` handlers). The kernel is pure mechanism: it      */
/* performs effects and answers queries, but holds NO gas counter and makes   */
/* NO policy decisions. Every syscall is appended to `el_trace`, which is the */
/* exact sequence of authenticated effects (= the AIR interface channels).    */
/*                                                                           */
/* The EVM (user space) decides whether/which syscall to issue; e.g. a no-op  */
/* SSTORE charges gas in the EVM but issues no EL_SStore here.                */
/* ======================================================================== */

/* ------------------------------ structs --------------------------------- */

struct Account = { acc_nonce : int, acc_balance : word, acc_code : list(byte) }
struct AccountEntry = { ae_addr : address, ae_acc : Account }
struct StorageEntry = { se_addr : address, se_slot : word, se_val : word }
struct SlotKey      = { sk_addr : address, sk_slot : word }
struct LogEntry     = { le_addr : address, le_topics : list(word), le_data : list(byte) }

enum EnvField = {
  F_Number, F_Timestamp, F_Coinbase, F_BaseFee, F_ChainId, F_GasLimit,
  F_PrevRandao, F_BlobBaseFee, F_Origin, F_GasPrice
}

struct BlockEnv = {
  be_number : word, be_timestamp : word, be_gas_limit : word, be_coinbase : address,
  be_base_fee : word, be_prev_randao : word, be_chain_id : word,
  be_blob_base_fee : word, be_excess_blob_gas : word
}
struct TxEnv = { txe_origin : address, txe_gas_price : word, txe_blob_hashes : list(word) }

/* Everything a reverting frame must roll back atomically. */
/* Journal-based revert: instead of deep-copying the whole world per call frame
 * (O(state) per snapshot), each mutation records a small undo entry; k_snapshot
 * pushes a checkpoint marker, k_revert replays undos back to it, k_commit drops
 * the marker (the frame's entries fold into the parent). O(changes), not O(state). */
union JEntry = {
  JCheck  : unit,                     /* call-frame checkpoint marker            */
  JAcct   : (address, Account),       /* restore an account to its prior value   */
  JStor   : (address, word, word),    /* restore storage slot to its prior value */
  JTran   : (address, word, word),    /* restore transient slot                  */
  JWarmA  : address,                  /* un-warm a newly-accessed address        */
  JWarmS  : (address, word),          /* un-warm a newly-accessed slot           */
  JLog    : unit,                     /* drop the most recent log                */
  JRefund : int,                      /* restore the refund counter              */
  JSelfD  : unit                      /* drop the most recent selfdestruct       */
}

/* ----------------------- the EL syscall trace --------------------------- */
/* One recorded entry per issued syscall. Byte buffers are summarized by      */
/* length to keep the trace a clean effect log.                              */
union ELOp = {
  EL_AccessAccount : address,
  EL_AccessSlot    : (address, word),
  EL_SLoad         : (address, word),
  EL_SStore        : (address, word, word),
  EL_TLoad         : (address, word),
  EL_TStore        : (address, word, word),
  EL_GetBalance    : address,
  EL_GetNonce      : address,
  EL_GetCode       : address,
  EL_GetCodeHash   : address,
  EL_Transfer      : (address, address, word),
  EL_BumpNonce     : address,
  EL_SetCode       : (address, int),
  EL_Env           : EnvField,
  EL_BlockHash     : word,
  EL_BlobHash      : int,
  EL_Keccak        : int,
  EL_Precompile    : (int, int),
  EL_Snapshot      : unit,
  EL_Commit        : unit,
  EL_Revert        : unit,
  EL_Refund        : int,
  EL_Log           : (address, int, int)
}

/* The EL-IR execution stream: USER-SPACE opcode steps interleaved with the
 * KERNEL syscalls each step issues. The TSyscall subset is the authenticated
 * effect stream (= the AIR interface channels); the TEvm steps are the opcode
 * rows. This is the inspectable EL-IR artifact. */
union TraceEntry = {
  TEvm     : (int, int),   /* (pc, opcode byte) -- a user-space EVM opcode step */
  TSyscall : ELOp          /* a kernel syscall  -- an authenticated effect       */
}

/* create_address / create2_address are real (sail/keccak.sail). EVM precompiles
 * run through the C host accelerator in sail/precompile.sail (run_precompile). */

/* ----------------------------- kernel state ----------------------------- */

let ZERO_WORD : word = 0x0000000000000000000000000000000000000000000000000000000000000000
let ZERO_ADDR : address = 0x0000000000000000000000000000000000000000
let EMPTY_ACCOUNT : Account = struct { acc_nonce = 0, acc_balance = ZERO_WORD, acc_code = [||] }

register k_accounts    : list(AccountEntry) = [||]
register k_warm_addr   : list(address)      = [||]
register k_logs        : list(LogEntry)     = [||]

/* ---- C-backed (address, slot) maps (ffi/el_map.c): O(1) per access ----
 * id 0 = EIP-1153 transient storage, id 1 = the EIP-2929 warm-slot set.
 * Sail assoc lists made these O(n) per access -- O(n^2) over a TSTORE loop or a
 * large access-list prewarm. "Absent" == "value 0" for both (TLOAD of an unset
 * slot is 0; un-warm = 0), so journal undo just stores the prior value. */
val el_map_reset       = impure "el_map_reset"       : bits(64) -> unit
val el_map_push        = impure "el_map_push"        : bits(64) -> unit
val el_map_pop_commit  = impure "el_map_pop_commit"  : bits(64) -> unit
val el_map_pop_discard = impure "el_map_pop_discard" : bits(64) -> unit
val el_map_key   = impure "el_map_key"   : (bits(64), bits(64), bits(64), bits(64), bits(64), bits(64), bits(64), bits(64)) -> unit
val el_map_store = impure "el_map_store" : (bits(64), bits(64), bits(64), bits(64)) -> unit
val el_map_word  = impure "el_map_word"  : bits(64) -> bits(64)
val el_map_base_word = impure "el_map_base_word" : bits(64) -> bits(64)
val el_map_present   = impure "el_map_present"   : unit -> bits(64)
val el_map_has_addr  = impure "el_map_has_addr"  : (bits(64), bits(64), bits(64), bits(64)) -> bits(64)
val el_map_wipe_addr = impure "el_map_wipe_addr" : (bits(64), bits(64), bits(64), bits(64)) -> unit
val el_map_count  = impure "el_map_count"  : bits(64) -> bits(64)
val el_map_sel    = impure "el_map_sel"    : bits(64) -> unit
val el_map_it_key = impure "el_map_it_key" : bits(64) -> bits(64)
val el_map_it_val = impure "el_map_it_val" : bits(64) -> bits(64)

let MAP_TRANSIENT : int = 0
let MAP_WARM_SLOT : int = 1
let MAP_STORAGE   : int = 2   /* persistent storage: OVERLAY layers (push at snapshot,
                                 pop_commit/pop_discard at commit/revert); the base
                                 layer = tx-start state = EIP-2200 originals. */

function map_select(id : int, a : address, s : word) -> unit = {
  let aw : word = sail_zero_extend(a, 256);
  el_map_key(get_slice_int(64, id, 0),
             aw[191 .. 128], aw[127 .. 64], aw[63 .. 0],
             s[255 .. 192], s[191 .. 128], s[127 .. 64], s[63 .. 0])
}
function map_load(id : int, a : address, s : word) -> word = {
  map_select(id, a, s);
  append(el_map_word(get_slice_int(64, 3, 0)),
  append(el_map_word(get_slice_int(64, 2, 0)),
  append(el_map_word(get_slice_int(64, 1, 0)), el_map_word(get_slice_int(64, 0, 0)))))
}
function map_store(id : int, a : address, s : word, v : word) -> unit = {
  map_select(id, a, s);
  el_map_store(v[255 .. 192], v[191 .. 128], v[127 .. 64], v[63 .. 0])
}
/* value in the BASE layer only (the state at tx start -- EIP-2200 original) */
function map_base_load(id : int, a : address, s : word) -> word = {
  map_select(id, a, s);
  append(el_map_base_word(get_slice_int(64, 3, 0)),
  append(el_map_base_word(get_slice_int(64, 2, 0)),
  append(el_map_base_word(get_slice_int(64, 1, 0)), el_map_base_word(get_slice_int(64, 0, 0)))))
}
/* present in ANY layer (an explicit 0 is present; fail-closed stateless test) */
function map_present(id : int, a : address, s : word) -> bool = {
  map_select(id, a, s);
  unsigned(el_map_present()) == 1
}
function map_has_addr(id : int, a : address) -> bool = {
  let aw : word = sail_zero_extend(a, 256);
  unsigned(el_map_has_addr(get_slice_int(64, id, 0), aw[191 .. 128], aw[127 .. 64], aw[63 .. 0])) == 1
}
function map_wipe_addr(id : int, a : address) -> unit = {
  let aw : word = sail_zero_extend(a, 256);
  el_map_wipe_addr(get_slice_int(64, id, 0), aw[191 .. 128], aw[127 .. 64], aw[63 .. 0])
}
/* flattened storage enumeration (dump_state / state root; cold path) */
function storage_count() -> int = unsigned(el_map_count(get_slice_int(64, MAP_STORAGE, 0)))
function storage_entry(j : int) -> (address, word, word) = {
  el_map_sel(get_slice_int(64, j, 0));
  let aw : word = sail_zero_extend(
    append(el_map_it_key(get_slice_int(64, 0, 0)),
    append(el_map_it_key(get_slice_int(64, 1, 0)), el_map_it_key(get_slice_int(64, 2, 0)))), 256);
  let s : word =
    append(el_map_it_key(get_slice_int(64, 3, 0)),
    append(el_map_it_key(get_slice_int(64, 4, 0)),
    append(el_map_it_key(get_slice_int(64, 5, 0)), el_map_it_key(get_slice_int(64, 6, 0)))));
  let v : word =
    append(el_map_it_val(get_slice_int(64, 3, 0)),
    append(el_map_it_val(get_slice_int(64, 2, 0)),
    append(el_map_it_val(get_slice_int(64, 1, 0)), el_map_it_val(get_slice_int(64, 0, 0)))));
  (word_to_address(aw), s, v)
}
register k_refund      : int                = 0
register k_selfdestr   : list(address)      = [||]
register k_created     : list(address)      = [||]   /* accounts CREATEd this tx (EIP-6780).
                                                        NOT journaled: like EELS created_accounts,
                                                        the marker survives a frame revert. */
register k_journal     : list(JEntry)       = [||]   /* undo log (call-frame revert) */
register ir_trace      : list(TraceEntry)   = [||]   /* opcode steps + syscalls */
register k_trace_on    : bool               = true   /* record the EL-IR trace? off for bulk EEST runs */
register k_stateless   : bool               = false  /* stateless mode: storage reads must hit the witness working set */
register k_fork        : int                = 99     /* active fork level (higher = newer); set by the runner */

/* fork levels for fork-gated rules (chronological; only thresholds matter) */
let FORK_PRAGUE : int = 2   /* EIP-7623 calldata floor              */
let FORK_OSAKA  : int = 3   /* EIP-7883 modexp gas, EIP-7825 cap    */

register k_block : BlockEnv = struct {
  be_number = ZERO_WORD, be_timestamp = ZERO_WORD, be_gas_limit = ZERO_WORD,
  be_coinbase = ZERO_ADDR, be_base_fee = ZERO_WORD, be_prev_randao = ZERO_WORD,
  be_chain_id = ZERO_WORD, be_blob_base_fee = ZERO_WORD, be_excess_blob_gas = ZERO_WORD
}
register k_tx : TxEnv = struct {
  txe_origin = ZERO_ADDR, txe_gas_price = ZERO_WORD, txe_blob_hashes = [||]
}

/* O(1) prepend: the trace is built newest-first; reverse for display
 * (list_append would make recording O(n^2) over a long execution). */
function record(op : ELOp) -> unit = if k_trace_on then ir_trace = TSyscall(op) :: ir_trace else ()

/* record a user-space opcode step (called by the interpreter's run loop) */
function trace_evm(at : int, opcode : int) -> unit =
  if k_trace_on then ir_trace = TEvm(at, opcode) :: ir_trace else ()

/* ----------------------------- assoc helpers ---------------------------- */

function acc_lookup(es : list(AccountEntry), a : address) -> Account =
  match es { [||] => EMPTY_ACCOUNT, e :: r => if e.ae_addr == a then e.ae_acc else acc_lookup(r, a) }
function acc_update(es : list(AccountEntry), a : address, v : Account) -> list(AccountEntry) =
  match es { [||] => [| struct { ae_addr = a, ae_acc = v } |],
             e :: r => if e.ae_addr == a then struct { ae_addr = a, ae_acc = v } :: r
                       else e :: acc_update(r, a, v) }
function addr_in(xs : list(address), a : address) -> bool =
  match xs { [||] => false, x :: r => if x == a then true else addr_in(r, a) }
function slot_in(xs : list(SlotKey), a : address, s : word) -> bool =
  match xs { [||] => false, x :: r => if x.sk_addr == a & x.sk_slot == s then true else slot_in(r, a, s) }

/* internal (unrecorded) account accessors used by syscall handlers */
function kget(a : address) -> Account = acc_lookup(k_accounts, a)
function kset(a : address, v : Account) -> unit = k_accounts = acc_update(k_accounts, a, v)

/* --- journal (undo log) helpers --- */
function jpush(e : JEntry) -> unit = k_journal = e :: k_journal
function jremove_addr(xs : list(address), a : address) -> list(address) =
  match xs { [||] => [||], x :: r => if x == a then r else x :: jremove_addr(r, a) }
function jremove_slot(xs : list(SlotKey), a : address, s : word) -> list(SlotKey) =
  match xs { [||] => [||], x :: r => if x.sk_addr == a & x.sk_slot == s then r else x :: jremove_slot(r, a, s) }
function jdrop_last_log(xs : list(LogEntry)) -> list(LogEntry) =
  match xs { [||] => [||], _ :: [||] => [||], x :: r => x :: jdrop_last_log(r) }
function jdrop_last_addr(xs : list(address)) -> list(address) =
  match xs { [||] => [||], _ :: [||] => [||], x :: r => x :: jdrop_last_addr(r) }
function apply_undo(e : JEntry) -> unit = match e {
  JCheck()       => (),
  JAcct(a, old)  => kset(a, old),
  JStor(_, _, _) => (),   /* storage undo is the overlay's pop_discard now */
  JTran(a, s, v) => map_store(MAP_TRANSIENT, a, s, v),
  JWarmA(a)      => k_warm_addr = jremove_addr(k_warm_addr, a),
  JWarmS(a, s)   => map_store(MAP_WARM_SLOT, a, s, ZERO_WORD),
  JLog()         => k_logs = jdrop_last_log(k_logs),
  JRefund(old)   => k_refund = old,
  JSelfD()       => k_selfdestr = jdrop_last_addr(k_selfdestr)
}

/* ========================= EL syscall handlers ========================== */
/* Each performs the effect / answers the query and records the syscall.      */

/* --- EIP-2929 access (mark warm, return PRIOR warm bit) --- */
function k_access_account(a : address) -> bool = {
  record(EL_AccessAccount(a));
  let prior = addr_in(k_warm_addr, a);
  if not_bool(prior) then { k_warm_addr = a :: k_warm_addr; jpush(JWarmA(a)) } else ();
  prior
}
function k_access_slot(a : address, s : word) -> bool = {
  record(EL_AccessSlot(a, s));
  let prior = word_nonzero(map_load(MAP_WARM_SLOT, a, s));
  if not_bool(prior) then { map_store(MAP_WARM_SLOT, a, s, word_of_int(1)); jpush(JWarmS(a, s)) } else ();
  prior
}

/* --- storage (C overlay map; layers paired with snapshot/commit/revert) --- */

/* stateless guard: a slot the EVM touches must be in the witness working set
 * (seeded from the BAL slice + witness pre-values). Absent => the prover did not
 * provision it => fail closed -- el_map_present distinguishes a witness-proven 0
 * from never-provisioned. Off by default, so the native path is unchanged. */
function k_stateless_on() -> unit = k_stateless = true
function storage_present(a : address, s : word) -> unit =
  if k_stateless then
    assert(map_present(MAP_STORAGE, a, s), "storage slot not in witness working set")
  else ()

function k_sload(a : address, s : word) -> word = {
  storage_present(a, s); record(EL_SLoad(a, s));
  map_load(MAP_STORAGE, a, s)
}
function k_sstore(a : address, s : word, v : word) -> unit = {
  storage_present(a, s);
  record(EL_SStore(a, s, v));
  map_store(MAP_STORAGE, a, s, v)   /* overlay write; revert = layer discard */
}
/* EIP-2200 original value = the BASE layer (the state at tx start; every prior
 * tx was pop_commit-merged into it). No per-tx snapshot copy needed. */
function k_orig(a : address, s : word) -> word = map_base_load(MAP_STORAGE, a, s)
/* unrecorded storage read (drivers/examples; same layered view as SLOAD) */
function k_storage_get(a : address, s : word) -> word = map_load(MAP_STORAGE, a, s)
function k_tload(a : address, s : word) -> word = { record(EL_TLoad(a, s)); map_load(MAP_TRANSIENT, a, s) }
function k_tstore(a : address, s : word, v : word) -> unit = {
  record(EL_TStore(a, s, v));
  jpush(JTran(a, s, map_load(MAP_TRANSIENT, a, s)));
  map_store(MAP_TRANSIENT, a, s, v)
}

/* --- accounts --- */
function k_get_balance(a : address) -> word = { record(EL_GetBalance(a)); kget(a).acc_balance }
function k_get_nonce(a : address) -> int = { record(EL_GetNonce(a)); kget(a).acc_nonce }
function k_get_code(a : address) -> list(byte) = { record(EL_GetCode(a)); kget(a).acc_code }
function k_get_codehash(a : address) -> hash = {
  record(EL_GetCodeHash(a));
  let acc = kget(a);
  match acc.acc_code { [||] => if acc.acc_nonce == 0 & word_is_zero(acc.acc_balance)
                               then word_of_int(0) else keccak256([||]),
                       _ => keccak256(acc.acc_code) }
}
function k_account_is_empty(a : address) -> bool = {   /* unrecorded query */
  let acc = kget(a);
  match acc.acc_code { [||] => acc.acc_nonce == 0 & word_is_zero(acc.acc_balance), _ => false }
}
/* CREATE/CREATE2/create-tx address-collision test (unrecorded query): the
 * target is occupied if it has code OR a nonzero nonce OR any storage
 * (EELS account_has_code_or_nonce || account_has_storage). */
function k_account_occupied(a : address) -> bool = {
  let acc = kget(a);
  let has_code : bool = match acc.acc_code { [||] => false, _ => true };
  has_code | (acc.acc_nonce != 0) | map_has_addr(MAP_STORAGE, a)
}
function k_transfer(src : address, dst : address, v : word) -> unit = {
  record(EL_Transfer(src, dst, v));
  jpush(JAcct(src, kget(src))); jpush(JAcct(dst, kget(dst)));
  kset(src, { kget(src) with acc_balance = alu_sub(kget(src).acc_balance, v) });
  kset(dst, { kget(dst) with acc_balance = alu_add(kget(dst).acc_balance, v) })
}
function k_bump_nonce(a : address) -> unit = {
  record(EL_BumpNonce(a)); jpush(JAcct(a, kget(a)));
  kset(a, { kget(a) with acc_nonce = kget(a).acc_nonce + 1 })
}
function k_set_code(a : address, c : list(byte)) -> unit = {
  record(EL_SetCode(a, list_len(c))); jpush(JAcct(a, kget(a)));
  kset(a, { kget(a) with acc_code = c })
}

/* ---- EIP-7702 delegation designation (0xef0100 || address) ---- */
function addr_bytes(a : address) -> list(byte) = {
  let w = address_to_word(a);
  var out : list(byte) = [||];
  foreach (k from 0 to 19) { out = list_append(out, [| (sail_shiftright(w, 8 * (19 - k)))[7 .. 0] |]) };
  out
}
function addr_from_bytes(bs : list(byte)) -> address = {
  var w : word = word_of_int(0);
  foreach (k from 0 to 19) { w = or_vec(sail_shiftleft(w, 8), sail_zero_extend(list_nth(bs, k, 0x00), 256)) };
  word_to_address(w)
}
function delegation_code(a : address) -> list(byte) = 0xef :: 0x01 :: 0x00 :: addr_bytes(a)

/* (is_delegation, target) for a 23-byte 0xef0100 designation; else (false, _) */
function deleg_target(c : list(byte)) -> (bool, address) =
  if list_len(c) == 23 then
    match c { 0xef :: 0x01 :: 0x00 :: rest => (true, addr_from_bytes(rest)), _ => (false, ZERO_ADDR) }
  else (false, ZERO_ADDR)

/* code to EXECUTE when calling addr: follow one delegation hop; a delegation to
 * a precompile (or empty) executes as empty code (EIP-7702). EXTCODE / CODE ops
 * keep the raw designation via k_get_code. */
function k_get_exec_code(addr : address) -> list(byte) = {
  let c = k_get_code(addr);
  let (deleg, t) = deleg_target(c);
  if not_bool(deleg) then c
  else { let n = unsigned(address_to_word(t)); if (n >= 1 & n <= 17) | (n == 256) then [||] else k_get_code(t) }
}
/* used only by the tx driver to seed pre-state (no trace entry) */
function k_seed_account(a : address, acc : Account) -> unit = kset(a, acc)
function k_seed_storage(a : address, s : word, v : word) -> unit =
  map_store(MAP_STORAGE, a, s, v)   /* seeds the base layer (only layer live here) */

/* single-account balance deltas (gas fees / withdrawals); recorded as a
 * transfer to/from the zero-address fee sink so the flow is in the trace. */
function k_add_balance(a : address, v : word) -> unit = {
  record(EL_Transfer(ZERO_ADDR, a, v)); jpush(JAcct(a, kget(a)));
  kset(a, { kget(a) with acc_balance = alu_add(kget(a).acc_balance, v) })
}
function k_sub_balance(a : address, v : word) -> unit = {
  record(EL_Transfer(a, ZERO_ADDR, v)); jpush(JAcct(a, kget(a)));
  kset(a, { kget(a) with acc_balance = alu_sub(kget(a).acc_balance, v) })
}

/* --- environment --- */
function k_env(f : EnvField) -> word = {
  record(EL_Env(f));
  match f {
    F_Number => k_block.be_number, F_Timestamp => k_block.be_timestamp,
    F_Coinbase => address_to_word(k_block.be_coinbase), F_BaseFee => k_block.be_base_fee,
    F_ChainId => k_block.be_chain_id, F_GasLimit => k_block.be_gas_limit,
    F_PrevRandao => k_block.be_prev_randao, F_BlobBaseFee => k_block.be_blob_base_fee,
    F_Origin => address_to_word(k_tx.txe_origin), F_GasPrice => k_tx.txe_gas_price
  }
}
function k_coinbase() -> address = k_block.be_coinbase
function k_blockhash(n : word) -> hash = { record(EL_BlockHash(n)); word_of_int(0) }
function k_blobhash(i : int) -> word = {
  record(EL_BlobHash(i)); list_nth(k_tx.txe_blob_hashes, i, word_of_int(0))
}

/* --- primitives --- */
function k_keccak(bs : list(byte)) -> hash = { record(EL_Keccak(list_len(bs))); keccak256(bs) }
function k_create_addr(a : address, nonce : int) -> address = create_address(a, nonce)
function k_create2_addr(a : address, salt : word, inithash : hash) -> address =
  create2_address(a, salt, inithash)
/* precompile: returns (success, output) via the C host accelerator
 * (sail/precompile.sail). Gas is computed by the EVM (policy, sail/gas.sail). */
function k_precompile(num : int, input : list(byte)) -> (bool, list(byte)) = {
  record(EL_Precompile(num, list_len(input)));
  if (num >= 1 & num <= 17) | (num == 256) then run_precompile(num, input) else (false, [||])
}

/* --- logs + refund --- */
function k_log(a : address, topics : list(word), data : list(byte)) -> unit = {
  record(EL_Log(a, list_len(topics), list_len(data))); jpush(JLog());
  k_logs = list_append(k_logs, [| struct { le_addr = a, le_topics = topics, le_data = data } |])
}
function k_refund_add(delta : int) -> unit = {
  record(EL_Refund(delta)); jpush(JRefund(k_refund)); k_refund = k_refund + delta
}
function k_selfdestruct(a : address) -> unit = { jpush(JSelfD()); k_selfdestr = list_append(k_selfdestr, [| a |]) }

/* --- EIP-6780 created-this-tx tracking --- */
/* Mark an address as CREATEd in the current tx. Like EELS created_accounts the
 * marker is NOT removed when the creating frame reverts (harmless: the account
 * cannot selfdestruct later without being re-created, which re-marks it). */
function k_mark_created(a : address) -> unit =
  if addr_in(k_created, a) then () else k_created = a :: k_created
function k_was_created(a : address) -> bool = addr_in(k_created, a)

/* EIP-6780 (selfdestruct of a created-this-tx account): the originator's
 * balance is forced to 0 after the transfer, so a self-beneficiary transfer
 * burns the ether (journaled; a frame revert restores it). */
function k_zero_balance(a : address) -> unit = {
  record(EL_Transfer(a, ZERO_ADDR, kget(a).acc_balance)); jpush(JAcct(a, kget(a)));
  kset(a, { kget(a) with acc_balance = ZERO_WORD })
}

/* delete an account outright: drop it from the account table and wipe its
 * storage. Used by the EIP-6780 tx-end finalization (created AND selfdestructed
 * in the same tx). Unrecorded: the tx-end reduction is host bookkeeping. */
function acc_remove(es : list(AccountEntry), a : address) -> list(AccountEntry) =
  match es { [||] => [||], e :: r => if e.ae_addr == a then acc_remove(r, a) else e :: acc_remove(r, a) }
function k_account_delete(a : address) -> unit = {
  k_accounts = acc_remove(k_accounts, a);
  map_wipe_addr(MAP_STORAGE, a)
}

/* EIP-6780 tx-end finalization: an account is actually DELETED only if it was
 * both CREATEd and SELFDESTRUCTed in this same tx (EELS: destroy_account per
 * tx_output.accounts_to_delete). The balance already moved at SELFDESTRUCT
 * time. Call AFTER the tx-level commit, so only surviving (committed)
 * selfdestructs are in k_selfdestr; a pre-existing selfdestructed account is
 * left in place (code/nonce/storage kept). */
function k_finalize_selfdestructs() -> unit = {
  var xs : list(address) = k_selfdestr;
  var more : bool = true;
  while more do {
    match xs {
      a :: r => { if k_was_created(a) then k_account_delete(a) else (); xs = r },
      [||]   => more = false
    }
  }
}

/* --- world snapshots (atomic revert across a frame) --- */
function k_snapshot() -> unit = {
  record(EL_Snapshot()); jpush(JCheck());
  el_map_push(get_slice_int(64, MAP_STORAGE, 0))     /* open the frame's storage overlay */
}
/* full world wipe between independent blocks/fixtures (driver/test only) */
function k_world_reset() -> unit = {
  k_accounts = [||]; el_map_reset(get_slice_int(64, MAP_STORAGE, 0)); el_map_reset(get_slice_int(64, MAP_TRANSIENT, 0));
  k_warm_addr = [||]; el_map_reset(get_slice_int(64, MAP_WARM_SLOT, 0)); k_logs = [||]; k_refund = 0;
  k_selfdestr = [||]; k_created = [||]; k_journal = [||]; ir_trace = [||];
  k_stateless = false
}

/* --- tx-lifecycle / driver helpers (system layer, not EVM user space) --- */
function k_set_block(env : BlockEnv) -> unit = k_block = env
function k_set_tx(env : TxEnv) -> unit = k_tx = env
function k_tx_reset() -> unit = {   /* per-tx: clear ephemeral state, snapshot tx-original storage */
  k_warm_addr = [||]; el_map_reset(get_slice_int(64, MAP_WARM_SLOT, 0));
  el_map_reset(get_slice_int(64, MAP_TRANSIENT, 0));
  k_logs = [||]; k_refund = 0; k_selfdestr = [||]; k_created = [||]; k_journal = [||]
  /* no per-tx original-storage snapshot: the overlay's base layer IS tx-start state */
}

/* commit: drop the most recent checkpoint; the frame's undo entries fold into
 * the parent frame (so a later parent revert also undoes them). */
function k_commit() -> unit = {
  record(EL_Commit());
  el_map_pop_commit(get_slice_int(64, MAP_STORAGE, 0));   /* merge the frame's storage overlay */
  var pre : list(JEntry) = [||];     /* frame's entries before the checkpoint (reversed) */
  var j   : list(JEntry) = k_journal;
  var done : bool = false;
  while not_bool(done) do {
    match j {
      [||]          => done = true,
      JCheck() :: r => { j = r; done = true },
      e :: r        => { pre = e :: pre; j = r }
    }
  };
  k_journal = rev_onto(pre, j)
}
/* revert: replay undo entries back to (and dropping) the most recent checkpoint. */
function k_revert() -> unit = {
  record(EL_Revert());
  el_map_pop_discard(get_slice_int(64, MAP_STORAGE, 0));  /* drop the frame's storage overlay */
  var j : list(JEntry) = k_journal;
  var done : bool = false;
  while not_bool(done) do {
    match j {
      [||]          => done = true,
      JCheck() :: r => { j = r; done = true },
      e :: r        => { apply_undo(e); j = r }
    }
  };
  k_journal = j
}
