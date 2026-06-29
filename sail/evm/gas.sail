/* ======================================================================== */
/* THE GAS SCHEDULE                                                         */
/*                                                                          */
/* The complete fork-gated EVM gas schedule (Yellow Paper Appendix G plus   */
/* the gas-repricing EIPs). Gas meters execution: every step debits the     */
/* frame's gas counter, and a debit that would underflow raises an out-of-  */
/* gas exceptional halt. This module owns the cost groups; the static per-  */
/* opcode cost is attached to each opcode in execute.sail.                  */
/*                                                                          */
/* The cost groups, with their governing specifications:                    */
/*                                                                          */
/*   - Static G_* constants: the Yellow Paper Appendix G symbols, as        */
/*     repriced by EIP-150 (SLOAD/CALL/SELFDESTRUCT), EIP-160 (G_expbyte),  */
/*     and EIP-2929 (cold/warm).                                            */
/*   - Memory-expansion gas (YP C_mem): quadratic in the high-water word    */
/*     count, charged on any addressed touch.                               */
/*   - EIP-2929 access costs: cold/warm surcharges for account and storage  */
/*     access (the kernel owns the warm set; the EVM only prices it).       */
/*   - EIP-2200 / EIP-3529 SSTORE cost and refund, under the EIP-3529       */
/*     refund cap (applied at tx scope, not here).                          */
/*   - Per-opcode dynamic gas: KECCAK256 / *COPY per-word, LOG per-         */
/*     byte/topic, EXP per-byte, EIP-3860 per-initcode-word.                */
/*   - Call gas: the EIP-150 all-but-one-64th forwarding cap and the value- */
/*     transfer stipend.                                                    */
/*   - Per-precompile gas, including the EIP-2565 / EIP-7883 MODEXP curve   */
/*     and the EIP-2537 BLS12-381 MSM discount tables.                      */
/* ======================================================================== */

/* --- EIP-4844 blob base fee (from excess_blob_gas) ---
 * blob_base_fee = fake_exponential(MIN_BLOB_BASE_FEE, excess_blob_gas,
 * BLOB_BASE_FEE_UPDATE_FRACTION). fake_exponential approximates
 * factor * e^(numerator/denominator) by summing the Taylor terms until the
 * (integer-divided) term reaches 0. The Amsterdam update fraction is EIP-7691's
 * (5007716). Mirrors forks/amsterdam/vm/gas.py. */
let MIN_BLOB_BASE_FEE : int = 1
let BLOB_BASE_FEE_UPDATE_FRACTION : int = 5007716

function fake_exponential(factor : int, numerator : int, denominator : int) -> int = {
    var i : int = 1;
    var output : int = 0;
    var numerator_accum : int = factor * denominator;
    while numerator_accum > 0 do {
        output = output + numerator_accum;
        numerator_accum = ediv_int(numerator_accum * numerator, denominator * i);
        i = i + 1
    };
    ediv_int(output, denominator)
}

function blob_base_fee(excess_blob_gas : int) -> int =
    fake_exponential(MIN_BLOB_BASE_FEE, excess_blob_gas, BLOB_BASE_FEE_UPDATE_FRACTION)

/* --- static cost constants (Yellow Paper Appendix G symbols G_*, as
 *     repriced by EIP-150/160/2929) --- */
let G_zero : gas = 0 /* YP G_zero                                  */
let G_jumpdest : gas = 1 /* YP G_jumpdest                              */
let G_base : gas = 2 /* YP G_base                                  */
let G_verylow : gas = 3 /* YP G_verylow                               */
let G_low : gas = 5 /* YP G_low                                   */
let G_mid : gas = 8 /* YP G_mid                                   */
let G_high : gas = 10 /* YP G_high                                  */
let G_warm_access : gas = 100 /* EIP-2929 G_warmaccess (warm SLOAD/access)  */
let G_cold_sload : gas = 2100 /* EIP-2929 G_coldsload surcharge             */
let G_cold_account : gas = 2600 /* EIP-2929 G_coldaccountaccess surcharge     */
let G_keccak : gas = 30 /* YP G_keccak256 (KECCAK256 base)            */
let G_keccak_word : gas = 6 /* YP G_keccak256word (per 32-byte word)      */
let G_copy_word : gas = 3 /* YP G_copy (*COPY per word)                 */
let G_memory : gas = 3 /* YP G_memory (per word of expansion)        */
let G_log : gas = 375 /* YP G_log (LOG base)                        */
let G_logtopic : gas = 375 /* YP G_logtopic (per topic)                  */
let G_logdata : gas = 8 /* YP G_logdata (per byte)                    */
let G_exp : gas = 10 /* YP G_exp (EXP base)                        */
let G_expbyte : gas = 50 /* YP G_expbyte; raised 10->50 by EIP-160     */
let G_sset : gas = 20000 /* EIP-2200 G_sset (zero -> nonzero SSTORE)   */
let G_sreset : gas = 2900 /* EIP-2200/2929 G_sreset (5000 - 2100 cold)  */
let R_sclear : gas = 4800 /* EIP-3529 SSTORE-clears refund              */
let G_create : gas = 32000 /* YP G_create (CREATE/CREATE2 base)          */
let G_codedeposit : gas = 200 /* YP G_codedeposit (per deployed code byte)  */
let G_callvalue : gas = 9000 /* YP G_callvalue (CALL value-transfer)       */
let G_callstipend : gas = 2300 /* YP G_callstipend (free gas to a value call) */
let G_newaccount : gas = 25000 /* YP G_newaccount (CALL/SELFDESTRUCT to new) */
let G_selfdestruct : gas = 5000 /* EIP-150 R_selfdestruct base                */
let G_initcode_word : gas = 2 /* EIP-3860 (per word of initcode)            */

/* --- protocol code-size limits --- */
/* EIP-170 deployed-code limit 24576, raised to 0x8000 = 32768 by EIP-7954 at
 * Amsterdam; the EIP-3860 initcode cap is always 2 * MAX_CODE_SIZE. Fork-
 * dependent so the shared interpreter serves both the pre-Amsterdam state-tests
 * (k_fork < FORK_AMSTERDAM) and the guest (k_fork defaults to 99). */
function max_code_size() -> int = if k_fork >= FORK_AMSTERDAM then { 32768 } else { 24576 }
function max_initcode_size() -> int = 2 * max_code_size()

/* --------------------------- charging hook ------------------------------ */
/* Debit `amount` from the frame's gas counter. A debit that cannot be       */
/* covered raises an out-of-gas exceptional halt (YP: the step is invalid,   */
/* all remaining gas is consumed). A non-positive amount is a no-op.         */

function charge(amount : gas) -> unit =
    if amount <= 0 then {
        ()
    } else {
        if gas_remaining < amount then {
            exc_halt(OutOfGas)
        } else {
            /* a >= b >= 0 proven above, so gas_remaining - amount is a valid
               `gas`: use native int64 subtraction instead of to_gas(a - b),
               which would round-trip through sail_int (see gas_sub). */
            gas_remaining = gas_sub(gas_remaining, amount)
        }
    }

/* --------------------- memory-expansion gas ----------------------------- */
/* Memory is charged by its high-water mark in 32-byte words (YP C_mem):     */
/* C_mem(a) = G_memory * a + floor(a^2 / 512), a quadratic term that makes   */
/* large allocations progressively dearer. Expansion gas is the DIFFERENCE   */
/* C_mem(new) - C_mem(old) -- you pay only for the words you newly reach.    */

function mem_cost(words : int) -> int = G_memory * words + ediv_int(words * words, 512)

/* Gas to expand memory so that byte range [0 .. new_bytes) is addressable;
 * rounds up to a word boundary and is zero when the watermark already covers
 * the range. */
function expansion_gas(new_bytes : int) -> gas = {
    let new_words = ediv_int(new_bytes + 31, 32);
    let old_words = mem_size_words();
    if new_words <= old_words then {
        0
    } else {
        to_gas(mem_cost(new_words) - mem_cost(old_words))
    }
}

/* Charge expansion to cover [off, off+len) when len > 0. EVM memory expands
 * for ANY charged touch (reads too -- MLOAD/KECCAK/MCOPY src/RETURN), so the
 * MSIZE watermark grows here, word-aligned, once the gas is paid. */
function charge_expansion(off : int, len : int) -> unit =
    if len <= 0 then {
        ()
    } else {
        charge(expansion_gas(off + len));
        if is_running() then {
            let new_bytes = 32 * ediv_int(off + len + 31, 32);
            if new_bytes > memory_size then {
                memory_size = new_bytes
            }
        }
    }

/* ----------------------- EIP-2929 access costs -------------------------- */
/* First touch of an account or storage slot in a transaction is COLD and    */
/* dear; a repeat touch is WARM and cheap (EIP-2929 mitigates state-access    */
/* DoS). The warm set is kernel state: the kernel marks warm (k_access_*)     */
/* and returns the PRIOR warm bit, and these helpers price that bit. No       */
/* state is mutated here -- this is pure cost policy.                         */

function account_cost(warm : bool) -> gas =
    if warm then { G_warm_access } else { G_cold_account }

function sload_cost(warm : bool) -> gas =
    if warm then { G_warm_access } else { G_cold_sload } /* EIP-2929: cold SLOAD = 2100 */

/* Big-endian integer from the STAGED precompile input, reading n bytes at
 * off and treating bytes past the end as zero (matching the execution-specs
 * buffer_read semantics used to parse precompile input headers). */
function pc_be(off : int, n : int) -> int = {
    var v : int = 0;
    foreach (i from 0 to (n - 1)) {
        v = v * 256 + unsigned(precompile_input_byte(off + i))
    };
    v
}

/* Number of significant bits in v: bit_length(0)=0, bit_length(1)=1, ... */
function bit_length(v : int) -> int = {
    var n : int = 0;
    var x : int = v;
    while x > 0 do {
        n = n + 1;
        x = ediv_int(x, 2)
    };
    n
}

/* MODEXP (precompile 0x05) gas, EIP-2565 as amended by EIP-7883 (Osaka).
 * Input layout: bsize[32] esize[32] msize[32] base[bsize] exp[esize] mod[msize].
 * Gas grows with the multiplication-complexity of the larger of base/mod and
 * with an iteration count derived from the exponent's bit length. */
function modexp_gas() -> gas = {
    let bl = pc_be(0, 32); /* the length fields are read at FULL 32-byte  */
    let el = pc_be(32, 32); /* width: an oversized length (e.g. 2^64) must */
    let ml = pc_be(64, 32); /* price OOG, not wrap into the low 4 bytes    */

    /* EIP-7823 (Osaka) caps base/exp/mod at 1024 bytes each; a larger field
     * fails the call. Modeled by returning a cost beyond any forwardable child
     * gas, so run_call's used <= child_gas test rejects it. */
    if (k_fork >= FORK_OSAKA) & ((bl > 1024) | (el > 1024) | (ml > 1024)) then {
        return 1000000000000
    };

    let maxlen = if bl > ml then { bl } else { ml };
    let words = ediv_int(maxlen + 7, 8); /* length of base/mod in 8-byte words */

    /* multiplication complexity: EIP-2565 uses words^2; EIP-7883 (Osaka)
       uses a flat 16 for inputs <= 32 bytes, else 2*words^2. */
    let osaka = k_fork >= FORK_OSAKA;
    let mult = if osaka then {
            if maxlen <= 32 then {
                16
            } else {
                2 * words * words
            }
        } else {
            words * words
        };
    let big_mul = if osaka then { 16 } else { 8 }; /* per-word weight of the high exponent words */
    let exp_off = 96 + bl; /* exponent begins after the 3 length fields + base */

    /* iteration count: for a short exponent, its bit length minus one; for a
       long exponent, big_mul per extra word past the first 32 bytes plus the
       leading word's bit length. */
    let iter : int = if el <= 32 then {
            let ev = pc_be(exp_off, el);
            if ev == 0 then {
                0
            } else {
                bit_length(ev) - 1
            }
        } else {
            let head = pc_be(exp_off, 32); /* first 32 bytes of exponent */
            let hb = if bit_length(head) > 0 then {
                    bit_length(head) - 1
                } else {
                    0
                };
            big_mul * (el - 32) + hb
        };
    let it = if iter < 1 then { 1 } else { iter };

    /* EIP-7883: gas = max(500, mult*it) (no /3); EIP-2565: max(200, mult*it/3) */
    let g = if osaka then {
            mult * it
        } else {
            ediv_int(mult * it, 3)
        };
    to_gas(if osaka then if g < 500 then 500 else g else if g < 200 then 200 else g)
}

/* EIP-2537 BLS12-381 multi-scalar-multiplication discount tables. Entry k-1
 * is the per-pair discount (in thousandths) applied to a k-pair MSM, so a
 * larger MSM is progressively cheaper per pair; the discount is constant for
 * k >= 128. Indexed k = 1 .. 128 (G1 and G2 use distinct curves). */
let BLS_G1_DISCOUNT : list(int) = [|
        1000,
        949,
        848,
        797,
        764,
        750,
        738,
        728,
        719,
        712,
        705,
        698,
        692,
        687,
        682,
        677,
        673,
        669,
        665,
        661,
        658,
        654,
        651,
        648,
        645,
        642,
        640,
        637,
        635,
        632,
        630,
        627,
        625,
        623,
        621,
        619,
        617,
        615,
        613,
        611,
        609,
        608,
        606,
        604,
        603,
        601,
        599,
        598,
        596,
        595,
        593,
        592,
        591,
        589,
        588,
        586,
        585,
        584,
        582,
        581,
        580,
        579,
        577,
        576,
        575,
        574,
        573,
        572,
        570,
        569,
        568,
        567,
        566,
        565,
        564,
        563,
        562,
        561,
        560,
        559,
        558,
        557,
        556,
        555,
        554,
        553,
        552,
        551,
        550,
        549,
        548,
        547,
        547,
        546,
        545,
        544,
        543,
        542,
        541,
        540,
        540,
        539,
        538,
        537,
        536,
        536,
        535,
        534,
        533,
        532,
        532,
        531,
        530,
        529,
        528,
        528,
        527,
        526,
        525,
        525,
        524,
        523,
        522,
        522,
        521,
        520,
        520,
        519,
    |]
let BLS_G2_DISCOUNT : list(int) = [|
        1000,
        1000,
        923,
        884,
        855,
        832,
        812,
        796,
        782,
        770,
        759,
        749,
        740,
        732,
        724,
        717,
        711,
        704,
        699,
        693,
        688,
        683,
        679,
        674,
        670,
        666,
        663,
        659,
        655,
        652,
        649,
        646,
        643,
        640,
        637,
        634,
        632,
        629,
        627,
        624,
        622,
        620,
        618,
        615,
        613,
        611,
        609,
        607,
        606,
        604,
        602,
        600,
        598,
        597,
        595,
        593,
        592,
        590,
        589,
        587,
        586,
        584,
        583,
        582,
        580,
        579,
        578,
        576,
        575,
        574,
        573,
        571,
        570,
        569,
        568,
        567,
        566,
        565,
        563,
        562,
        561,
        560,
        559,
        558,
        557,
        556,
        555,
        554,
        553,
        552,
        552,
        551,
        550,
        549,
        548,
        547,
        546,
        545,
        545,
        544,
        543,
        542,
        541,
        541,
        540,
        539,
        538,
        537,
        537,
        536,
        535,
        535,
        534,
        533,
        532,
        532,
        531,
        530,
        530,
        529,
        528,
        528,
        527,
        526,
        526,
        525,
        524,
        524,
    |]

/* EIP-2537 MSM gas: gas = (k * base * discount(k)) / 1000, where discount(k)
 * is read from `table` and clamps to `maxd` (the k=128 entry) for k >= 128. */
function bls_msm_gas(table : list(int), base : gas, maxd : int, k : int) -> gas = {
    if k <= 0 then {
        0
    } else {
        let d = if k >= 128 then {
                maxd
            } else {
                list_nth(table, k - 1, maxd)
            };
        to_gas(ediv_int(k * base * d, 1000))
    }
}

/* Gas for the precompile at address `num`, by input length (the host returns
 * only output; gas is pure EVM policy). Length-only costs are computed from
 * `words`; the two input-dependent curves (MODEXP, BLAKE2F rounds) inspect the
 * STAGED input bytes in O(1). The cases below are the precompile catalog with
 * their addresses and pricing EIPs. */
function precompile_gas(num : int, input_len : int) -> gas = {
    let words = ediv_int(input_len + 31, 32);
    let g : int = match num {
        1 => 3000, /* ECRECOVER (0x01) */
        2 => 60 + 12 * words, /* SHA-256 (0x02) */
        3 => 600 + 120 * words, /* RIPEMD-160 (0x03) */
        4 => 15 + 3 * words, /* IDENTITY (0x04) */
        5 => modexp_gas(), /* MODEXP (0x05), EIP-2565/7883 */
        6 => 150, /* BN_ADD (0x06), EIP-1108 */
        7 => 6000, /* BN_MUL (0x07), EIP-1108 */
        8 => 45000 + 34000 * ediv_int(input_len, 192), /* BN_PAIRING (0x08), EIP-1108 */
        9 => pc_be(0, 4), /* BLAKE2F (0x09): gas = round count */
        10 => 50000, /* POINT_EVALUATION (0x0a), EIP-4844 */
        /* EIP-2537 BLS12-381 */
        11 => 375, /* G1ADD            */
        12 => bls_msm_gas(BLS_G1_DISCOUNT, 12000, 519, ediv_int(input_len, 160)), /* G1MSM */
        13 => 600, /* G2ADD            */
        14 => bls_msm_gas(BLS_G2_DISCOUNT, 22500, 524, ediv_int(input_len, 288)), /* G2MSM */
        15 => 37700 + 32600 * ediv_int(input_len, 384), /* PAIRING_CHECK    */
        16 => 5500, /* MAP_FP_TO_G1     */
        17 => 23800, /* MAP_FP2_TO_G2    */
        256 => 6900, /* P256VERIFY (EIP-7951, 0x100) */
        _ => 0,
    };
    to_gas(g)
}

/* ----------------- EIP-2200/2929/3529 SSTORE cost+refund ---------------- */
/* SSTORE pricing is a three-way comparison (EIP-2200) of: original = the     */
/* slot's value at the start of the transaction, current = its present value, */
/* and new = the value being stored. Writing the same value, or dirtying an   */
/* already-dirty slot, is cheap (warm-access only); a clean slot going        */
/* zero<->nonzero pays G_sset / G_sreset. Returns (gas_cost, refund_delta);   */
/* refunds track clearing/un-clearing and resetting a slot to its original    */
/* value (EIP-3529 rates). The cold surcharge (EIP-2929) is added when the    */
/* caller reports the slot was not yet warm. The refund_delta is summed and   */
/* capped at tx scope (EIP-3529 gas_used/5), not here. */
function sstore_gas(original : word, current : word, new : word, cold : bool) -> (gas, int) = {
    let cold_cost : gas = if cold then { G_cold_sload } else { 0 };
    let base : gas = if current == new then {
            G_warm_access
        } else {
            if original == current then {
                if word_is_zero(original) then { G_sset } else { G_sreset }
            } else {
                G_warm_access
            }
        };
    var refund : int = 0;
    if current != new then {
        if original == current then {
            /* clean slot being changed: refund only when clearing a non-zero slot */
            if not_bool(word_is_zero(original)) & word_is_zero(new) then {
                refund = refund + R_sclear
            }
        } else {
            /* dirty slot */
            if not_bool(word_is_zero(original)) then {
                if word_is_zero(current) then {
                    refund = refund - R_sclear
                }; /* un-clear */
                if word_is_zero(new) then {
                    refund = refund + R_sclear
                } /* re-clear */
            };
            if original == new then {
                /* reset to original */
                if word_is_zero(original) then {
                    refund = refund + (G_sset - G_warm_access)
                    /* 19900 */
                } else {
                    refund = refund + (G_sreset - G_warm_access)
                } /* 2800 */
            }
        }
    };
    (to_gas(base + cold_cost), refund)
}

/* ------------------------- per-op dynamic gas --------------------------- */
/* Operand-sized costs that ride on top of an opcode's static cost. All are   */
/* word-counted via ceil(byte_len / 32) where the spec charges per 32-byte    */
/* word (YP Appendix G). */

/* KECCAK256: base + per-word hashing cost. */
function keccak_gas(byte_len : int) -> gas =
    to_gas(G_keccak + G_keccak_word * ediv_int(byte_len + 31, 32))

/* *COPY (CALLDATA/CODE/RETURNDATA/EXTCODECOPY, MCOPY): per-word copy cost. */
function copy_gas(byte_len : int) -> gas =
    to_gas(G_copy_word * ediv_int(byte_len + 31, 32))

/* LOG0..LOG4: base + per-topic + per-data-byte. */
function log_gas(num_topics : int, byte_len : int) -> gas =
    to_gas(G_log + G_logtopic * num_topics + G_logdata * byte_len)

/* Significant byte length of a non-negative integer (the EXP exponent size). */
function byte_length(n : int) -> int =
    if n <= 0 then {
        0
    } else {
        1 + byte_length(ediv_int(n, 256))
    }
termination_measure byte_length(n) = n

/* EXP: base + G_expbyte per significant byte of the exponent (EIP-160). */
function exp_gas(exponent : word) -> gas =
    to_gas(G_exp + G_expbyte * byte_length(unsigned(exponent)))

/* EIP-3860 initcode word cost, charged by CREATE/CREATE2 and tx-deploy. */
function initcode_gas(byte_len : int) -> gas =
    to_gas(G_initcode_word * ediv_int(byte_len + 31, 32))

/* ----------------------- call gas (EIP-150 63/64) ----------------------- */
/* EIP-150 forwards at most all-but-one-64th of the available gas to a        */
/* sub-call, so a caller always retains floor(available/64) to finish its     */
/* own frame. The forwarded amount is min(requested, available - available/64). */
function call_gas_cap(available : gas, requested : int) -> gas = {
    let all_but_64th = available - ediv_int(available, 64);
    if requested < all_but_64th then {
        to_gas(requested)
    } else {
        to_gas(all_but_64th)
    }
}
