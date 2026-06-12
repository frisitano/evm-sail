/* ======================================================================== */
/* EL-IR gas schedule (Cancun/Prague-era constants)                          */
/*                                                                           */
/* Static per-opcode costs are attached in execute.sail; this module owns    */
/* the charging hook, memory-expansion gas, the EIP-2929 access costs, the   */
/* EIP-2200/3529 SSTORE cost+refund, and the keccak/copy/log/exp/call/create */
/* dynamic-gas helpers.                                                      */
/* ======================================================================== */

/* --- static cost constants --- */
let G_zero               : gas = 0
let G_jumpdest           : gas = 1
let G_base               : gas = 2
let G_verylow            : gas = 3
let G_low                : gas = 5
let G_mid                : gas = 8
let G_high               : gas = 10
let G_warm_access        : gas = 100
let G_cold_sload         : gas = 2100
let G_cold_account       : gas = 2600
let G_keccak             : gas = 30
let G_keccak_word        : gas = 6
let G_copy_word          : gas = 3
let G_memory             : gas = 3
let G_log                : gas = 375
let G_logtopic           : gas = 375
let G_logdata            : gas = 8
let G_exp                : gas = 10
let G_expbyte            : gas = 50
let G_sset               : gas = 20000
let G_sreset             : gas = 2900   /* 5000 - 2100 cold (charged separately) */
let R_sclear             : gas = 4800   /* EIP-3529 SSTORE clears refund         */
let G_create             : gas = 32000
let G_codedeposit        : gas = 200
let G_callvalue          : gas = 9000
let G_callstipend        : gas = 2300
let G_newaccount         : gas = 25000
let G_selfdestruct       : gas = 5000
let G_initcode_word      : gas = 2      /* EIP-3860 */
let MAX_INITCODE_SIZE    : int = 49152  /* EIP-3860: 2 * MAX_CODE_SIZE */
let MAX_CODE_SIZE        : int = 24576

/* --------------------------- charging hook ------------------------------ */

function charge(amount : gas) -> unit =
  if amount <= 0 then ()
  else if gas_remaining < amount then exc_halt(OutOfGas)
  else gas_remaining = gas_remaining - amount

/* --------------------- memory-expansion gas ----------------------------- */

function mem_cost(words : int) -> gas = G_memory * words + ediv_int(words * words, 512)

/* Gas to expand memory so that byte range [0 .. new_bytes) is addressable. */
function expansion_gas(new_bytes : int) -> gas = {
  let new_words = ediv_int(new_bytes + 31, 32);
  let old_words = mem_size_words();
  if new_words <= old_words then 0
  else mem_cost(new_words) - mem_cost(old_words)
}

/* Charge expansion to cover [off, off+len) when len > 0. EVM memory expands
 * for ANY charged touch (reads too -- MLOAD/KECCAK/MCOPY src/RETURN), so the
 * MSIZE watermark grows here, word-aligned, once the gas is paid. */
function charge_expansion(off : int, len : int) -> unit =
  if len <= 0 then () else {
    charge(expansion_gas(off + len));
    if is_running() then {
      let new_bytes = 32 * ediv_int(off + len + 31, 32);
      if new_bytes > memory_size then memory_size = new_bytes else ()
    } else ()
  }

/* ----------------------- EIP-2929 access costs -------------------------- */
/* PURE user-space policy: the kernel marks warm (k_access_*) and returns the */
/* PRIOR warm bit; the EVM computes the cost from it. No state mutation here.  */

function account_cost(warm : bool) -> gas =
  if warm then G_warm_access else G_cold_account

function sload_cost(warm : bool) -> gas =
  if warm then G_warm_access else G_cold_sload   /* EIP-2929: COLD_SLOAD_COST = 2100 total */

/* big-endian int over n bytes of `args` starting at off (0 beyond end) */
function pc_be(args : list(byte), off : int, n : int) -> int = {
  var v : int = 0;
  foreach (i from 0 to (n - 1)) { v = v * 256 + unsigned(list_nth(args, off + i, 0x00)) };
  v
}
/* number of bits in v (bit_length): bit_length(0)=0, bit_length(1)=1, ... */
function bit_length(v : int) -> int = {
  var n : int = 0; var x : int = v;
  while x > 0 do { n = n + 1; x = ediv_int(x, 2) };
  n
}

/* EIP-2565 modexp gas: input = bsize[32] esize[32] msize[32] base exp mod. */
function modexp_gas(args : list(byte)) -> gas = {
  let bl = pc_be(args, 0, 32);   /* FULL 32-byte length fields: oversized      */
  let el = pc_be(args, 32, 32);  /* lengths (e.g. 2^64) must price the call    */
  let ml = pc_be(args, 64, 32);  /* out of gas, not alias into the low 4 bytes */
  /* EIP-7823 (Osaka): base/exp/mod each <= 1024 bytes, else the call fails (priced
   * beyond any forwardable gas so run_call's used<=child_gas test rejects it). */
  if (k_fork >= FORK_OSAKA) & ((bl > 1024) | (el > 1024) | (ml > 1024))
  then return 1000000000000   /* > any forwardable child gas -> the modexp call fails */
  else ();
  let maxlen = if bl > ml then bl else ml;
  let words  = ediv_int(maxlen + 7, 8);
  /* EIP-7883 (Osaka): mult = 16 for <=32B inputs, else 2*words^2; EIP-2565: words^2 */
  let osaka  = k_fork >= FORK_OSAKA;
  let mult   = if osaka then (if maxlen <= 32 then 16 else 2 * words * words)
               else words * words;
  let big_mul = if osaka then 16 else 8;            /* large-exponent multiplier */
  let exp_off = 96 + bl;
  let iter : int =
    if el <= 32 then {
      let ev = pc_be(args, exp_off, el);
      if ev == 0 then 0 else bit_length(ev) - 1
    } else {
      let head = pc_be(args, exp_off, 32);           /* first 32 bytes of exponent */
      let hb = if bit_length(head) > 0 then bit_length(head) - 1 else 0;
      big_mul * (el - 32) + hb
    };
  let it = if iter < 1 then 1 else iter;
  /* EIP-7883: gas = max(500, mult*it) (no /3); EIP-2565: max(200, mult*it/3) */
  let g  = if osaka then mult * it else ediv_int(mult * it, 3);
  if osaka then (if g < 500 then 500 else g) else (if g < 200 then 200 else g)
}

/* EIP-2537 BLS12-381 MSM discount tables (per-pair discount, /1000), index k=1..128. */
let BLS_G1_DISCOUNT : list(int) = [|
  1000,949,848,797,764,750,738,728, 719,712,705,698,692,687,682,677,
  673,669,665,661,658,654,651,648,  645,642,640,637,635,632,630,627,
  625,623,621,619,617,615,613,611,  609,608,606,604,603,601,599,598,
  596,595,593,592,591,589,588,586,  585,584,582,581,580,579,577,576,
  575,574,573,572,570,569,568,567,  566,565,564,563,562,561,560,559,
  558,557,556,555,554,553,552,551,  550,549,548,547,547,546,545,544,
  543,542,541,540,540,539,538,537,  536,536,535,534,533,532,532,531,
  530,529,528,528,527,526,525,525,  524,523,522,522,521,520,520,519 |]
let BLS_G2_DISCOUNT : list(int) = [|
  1000,1000,923,884,855,832,812,796, 782,770,759,749,740,732,724,717,
  711,704,699,693,688,683,679,674,   670,666,663,659,655,652,649,646,
  643,640,637,634,632,629,627,624,   622,620,618,615,613,611,609,607,
  606,604,602,600,598,597,595,593,   592,590,589,587,586,584,583,582,
  580,579,578,576,575,574,573,571,   570,569,568,567,566,565,563,562,
  561,560,559,558,557,556,555,554,   553,552,552,551,550,549,548,547,
  546,545,545,544,543,542,541,541,   540,539,538,537,537,536,535,535,
  534,533,532,532,531,530,530,529,   528,528,527,526,526,525,524,524 |]

/* EIP-2537 MSM: gas = (k * base * discount(k)) / 1000; discount clamps at k=128. */
function bls_msm_gas(table : list(int), base : gas, maxd : int, k : int) -> gas = {
  if k <= 0 then 0
  else {
    let d = if k >= 128 then maxd else list_nth(table, k - 1, maxd);
    ediv_int(k * base * d, 1000)
  }
}

/* Precompile gas (user-space policy; the C host returns only output). Dynamic
 * costs (modexp EIP-2565, bn254 pairing, blake2f rounds, EIP-2537 BLS) read input. */
function precompile_gas(num : int, args : list(byte)) -> gas = {
  let input_len = list_len(args);
  let words = ediv_int(input_len + 31, 32);
  match num {
    1 => 3000, 2 => 60 + 12 * words, 3 => 600 + 120 * words, 4 => 15 + 3 * words,
    5 => modexp_gas(args),
    6 => 150, 7 => 6000,
    8 => 45000 + 34000 * ediv_int(input_len, 192),   /* EIP-1108 */
    9 => pc_be(args, 0, 4),                           /* blake2f: gas = rounds */
    10 => 50000,
    /* EIP-2537 BLS12-381 */
    11 => 375,                                                  /* G1ADD            */
    12 => bls_msm_gas(BLS_G1_DISCOUNT, 12000, 519, ediv_int(input_len, 160)),  /* G1MSM */
    13 => 600,                                                  /* G2ADD            */
    14 => bls_msm_gas(BLS_G2_DISCOUNT, 22500, 524, ediv_int(input_len, 288)),  /* G2MSM */
    15 => 37700 + 32600 * ediv_int(input_len, 384),            /* PAIRING_CHECK    */
    16 => 5500,                                                 /* MAP_FP_TO_G1     */
    17 => 23800,                                                /* MAP_FP2_TO_G2    */
    256 => 6900,                                                /* P256VERIFY (EIP-7951, 0x100) */
    _ => 0
  }
}

/* ----------------- EIP-2200/2929/3529 SSTORE cost+refund ---------------- */
/* original = value at start of tx, current = present value, new = stored.
 * Returns (gas_cost, refund_delta). Cold surcharge is added when the slot
 * was not yet warm (the caller passes `cold`). */
function sstore_gas(original : word, current : word, new : word, cold : bool)
                   -> (gas, int) = {
  let cold_cost : gas = if cold then G_cold_sload else 0;
  let base : gas =
    if current == new then G_warm_access
    else if original == current then {
      if word_is_zero(original) then G_sset else G_sreset
    } else G_warm_access;
  var refund : int = 0;
  if current != new then {
    if original == current then {
      /* clean slot being changed: refund only when clearing a non-zero slot */
      if not_bool(word_is_zero(original)) & word_is_zero(new)
      then refund = refund + R_sclear else ()
    } else {
      /* dirty slot */
      if not_bool(word_is_zero(original)) then {
        if word_is_zero(current) then refund = refund - R_sclear else ();  /* un-clear */
        if word_is_zero(new)     then refund = refund + R_sclear else ()   /* re-clear */
      } else ();
      if original == new then {                                            /* reset to original */
        if word_is_zero(original)
        then refund = refund + (G_sset - G_warm_access)                    /* 19900 */
        else refund = refund + (G_sreset - G_warm_access)                  /* 2800 */
      } else ()
    }
  } else ();
  (base + cold_cost, refund)
}

/* ------------------------- per-op dynamic gas --------------------------- */

function keccak_gas(byte_len : int) -> gas =
  G_keccak + G_keccak_word * ediv_int(byte_len + 31, 32)

function copy_gas(byte_len : int) -> gas =
  G_copy_word * ediv_int(byte_len + 31, 32)

function log_gas(num_topics : int, byte_len : int) -> gas =
  G_log + G_logtopic * num_topics + G_logdata * byte_len

/* significant byte length of a non-negative integer (for EXP). */
function byte_length(n : int) -> int =
  if n <= 0 then 0 else 1 + byte_length(ediv_int(n, 256))
termination_measure byte_length(n) = n

function exp_gas(exponent : word) -> gas =
  G_exp + G_expbyte * byte_length(unsigned(exponent))

function initcode_gas(byte_len : int) -> gas =
  G_initcode_word * ediv_int(byte_len + 31, 32)

/* ----------------------- call gas (EIP-150 63/64) ----------------------- */
/* The gas actually forwarded to a sub-call: min(requested, all-but-1/64). */
function call_gas_cap(available : gas, requested : gas) -> gas = {
  let all_but_64th = available - ediv_int(available, 64);
  if requested < all_but_64th then requested else all_but_64th
}

/* ======================================================================== */
/* The gas instruction set                                                   */
/*                                                                           */
/* Every gas charge is one named GasOp; gas_of is the single, spec-auditable */
/* source of truth for the schedule. Each opcode charges a short sequence of */
/* these, so the gas rules live in one place (instead of scattered inline    */
/* charges) and can be checked against the yellow paper / EIPs one by one.   */
/* ======================================================================== */

union GasOp = {
  Gconst        : gas,                      /* fixed opcode cost                       */
  Gmem          : int,                      /* memory expansion to cover [.. nbytes)   */
  Gacct         : bool,                     /* EIP-2929 account access (prior-warm bit)*/
  Gslot         : bool,                     /* EIP-2929 storage-slot access            */
  Gsstore       : (word, word, word, bool), /* EIP-2200/3529 (orig, cur, new, cold)    */
  Gkeccak       : int,                      /* KECCAK256 over nbytes                   */
  Gcopy         : int,                      /* copy nbytes                             */
  Glog          : (int, int),               /* LOG ntopics, nbytes                     */
  Gexp          : word,                     /* EXP exponent                            */
  Gcallvalue    : unit,                     /* CALL value-transfer surcharge           */
  Gnewacct      : unit,                     /* new-account surcharge                   */
  Gcreatebase   : unit,                     /* CREATE base                             */
  Ginitcode     : int,                      /* EIP-3860 init-code words                 */
  Gselfdestructx: unit
}

function gas_of(g : GasOp) -> gas = match g {
  Gconst(n)           => n,
  Gmem(nbytes)        => expansion_gas(nbytes),
  Gacct(warm)         => account_cost(warm),
  Gslot(warm)         => sload_cost(warm),
  Gsstore(o, c, n, cold) => { let (cost, _) = sstore_gas(o, c, n, cold); cost },
  Gkeccak(len)        => keccak_gas(len),
  Gcopy(len)          => copy_gas(len),
  Glog(t, len)        => log_gas(t, len),
  Gexp(e)             => exp_gas(e),
  Gcallvalue()        => G_callvalue,
  Gnewacct()          => G_newaccount,
  Gcreatebase()       => G_create,
  Ginitcode(len)      => initcode_gas(len),
  Gselfdestructx()    => G_selfdestruct
}

/* charge one gas op (the only place gas leaves the EVM's counter) */
function charge_g(g : GasOp) -> unit = charge(gas_of(g))
