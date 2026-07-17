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
/* - Static G_* constants: the Yellow Paper Appendix G symbols, as          */
/* repriced by EIP-150 (SLOAD/CALL/SELFDESTRUCT), EIP-160 (G_expbyte),      */
/* and EIP-2929 (cold/warm).                                                */
/* - Memory-expansion gas (YP C_mem): quadratic in the high-water word      */
/* count, charged on any addressed touch.                                   */
/* - EIP-2929 access costs: cold/warm surcharges for account and storage    */
/* access (the kernel owns the warm set; the EVM only prices it).           */
/* - EIP-2200 / EIP-3529 SSTORE cost and refund, under the EIP-3529         */
/* refund cap (applied at tx scope, not here).                              */
/*   - Per-opcode dynamic gas: KECCAK256 / *COPY per-word, LOG per-         */
/* byte/topic, EXP per-byte, EIP-3860 per-initcode-word.                    */
/*   - Call gas: the EIP-150 all-but-one-64th forwarding cap and the value- */
/* transfer stipend.                                                        */
/* - Per-precompile gas, including the EIP-2565 / EIP-7883 MODEXP curve     */
/* and the EIP-2537 BLS12-381 MSM discount tables.                          */
/* ======================================================================== */

/* --- EIP-4844/EIP-7918 blob accounting (from excess_blob_gas) ---
 * blob_base_fee = fake_exponential(MIN_BLOB_BASE_FEE, excess_blob_gas,
 * BLOB_BASE_FEE_UPDATE_FRACTION). The active target, maximum, and update
 * fraction come from the validated SSZ chain config, including BPO1/BPO2. */
let MIN_BLOB_BASE_FEE : word = WORD_ONE
let GAS_PER_BLOB : blob_gas = 131072

/* The Taylor series is defined over exact non-negative integers, not the
 * EVM's modulo-2^256 word algebra. A valid header chain cannot reach a blob
 * price above an EVM word: long before that point, the fee for one blob would
 * exceed every word-sized sender balance, so no block could sustain the
 * excess needed to raise the price. The checked implementation below fails
 * closed if an unauthenticated or otherwise unreachable header violates that
 * invariant.
 *
 * Taylor terms and their running sum are scaled by `denominator`. Keeping the
 * quotient and remainder separately computes the exact word-sized result
 * without either modular intermediate arithmetic or a Sail bigint. */
struct ScaledBlobValue = {
    whole : word,
    remainder : protocol_quantity,
}

function scaled_blob_add(
    left : ScaledBlobValue,
    right : ScaledBlobValue,
    denominator : blob_fee_update_fraction,
) -> (
    option(ScaledBlobValue)
) = {
    if denominator == 0 then {
        return None()
    };
    let combined = left.remainder + right.remainder;
    let carry = denominator <= combined;
    let remainder = if carry then {
            combined - denominator
        } else {
            combined
        };
    if 2 ^ 64 - 1 < remainder then {
        return None()
    };
    let whole : word = match word_checked_add(left.whole, right.whole) {
        Some(value) => value,
        None() => return None(),
    };
    if carry then {
        match word_checked_add(whole, WORD_ONE) {
            Some(value) => Some(struct { whole = value, remainder = remainder }),
            None() => None(),
        }
    } else {
        Some(struct { whole = whole, remainder = remainder })
    }
}

/* Exact divmod of `value * factor + addend` by
 * `denominator * iteration`. Both divisor factors fit one fixed-width limb, so after first
 * dividing `value`, the residual product is below 2^192 and cannot wrap a
 * word. None means that the exact quotient is larger than WORD_ALL_ONES. */
function blob_product_divmod(
    value : word,
    factor : blob_gas,
    addend : protocol_quantity,
    denominator : blob_fee_update_fraction,
    iteration : item_index,
) -> (
    option(WordDivMod)
) = {
    let divisor = word_mul(word_of_nat(denominator), word_of_nat(iteration));
    if word_is_zero(divisor) then {
        return None()
    };
    let value_parts = word_divmod(value, divisor);
    let whole : word = match word_checked_mul(value_parts.quotient, factor) {
        Some(product) => product,
        None() => return None(),
    };
    let residual_product = word_mul(value_parts.remainder, word_of_nat(factor));
    let residual_sum : word = match word_checked_add(residual_product, word_of_nat(addend)) {
        Some(sum) => sum,
        None() => return None(),
    };
    let residual_parts = word_divmod(residual_sum, divisor);
    let quotient : word = match word_checked_add(whole, residual_parts.quotient) {
        Some(sum) => sum,
        None() => return None(),
    };
    Some(struct { quotient = quotient, remainder = residual_parts.remainder })
}

function scaled_blob_next(
    term : ScaledBlobValue,
    numerator : blob_gas,
    denominator : blob_fee_update_fraction,
    iteration : item_index,
) -> (
    option(ScaledBlobValue)
) = {
    /* term = whole*denominator + remainder. Cancelling the common
     * denominator leaves an exact integer addend floor(remainder*n/d). */
    let remainder_product = word_mul(word_of_nat(term.remainder), word_of_nat(numerator));
    let addend_word = word_divmod(remainder_product, word_of_nat(denominator)).quotient;
    if addend_word[255 .. 64] != sail_zeros(192) then {
        return None()
    };
    let addend = unsigned(addend_word[63 .. 0]);
    let divided : WordDivMod = match blob_product_divmod(term.whole, numerator, addend, denominator, iteration) {
        Some(result) => result,
        None() => return None(),
    };
    let remainder_word = word_divmod(divided.remainder, word_of_nat(iteration)).quotient;
    if remainder_word[255 .. 64] != sail_zeros(192) then {
        return None()
    };
    let remainder = unsigned(remainder_word[63 .. 0]);
    Some(struct { whole = divided.quotient, remainder = remainder })
}

function fake_exponential(factor : word, numerator : blob_gas, denominator : blob_fee_update_fraction) -> word = {
    if denominator == 0 then {
        throw InvalidBlock(InvalidConfig)
    };
    var i : item_index = 1;
    var output : ScaledBlobValue = struct { whole = ZERO_WORD, remainder = 0 };
    var term : ScaledBlobValue = struct { whole = factor, remainder = 0 };
    while word_nonzero(term.whole) | (term.remainder != 0) termination_measure(2 ^ 64 - i) do {
        output = match scaled_blob_add(output, term, denominator) {
            Some(sum) => sum,
            None() => throw InvalidBlock(ExecutionInvalid),
        };
        term = match scaled_blob_next(term, numerator, denominator, i) {
            Some(next) => next,
            None() => throw InvalidBlock(ExecutionInvalid),
        };
        if i == 2 ^ 64 - 1 then {
            throw InvalidBlock(ExecutionInvalid)
        };
        i = item_count_increment(i)
    };
    output.whole
}

function blob_base_fee_update_fraction() -> blob_fee_update_fraction =
    k_blob_schedule.base_fee_update_fraction

function blob_base_fee(excess_blob_gas : blob_gas) -> word =
    fake_exponential(MIN_BLOB_BASE_FEE, excess_blob_gas, blob_base_fee_update_fraction())

/* EIP-7918 compares BLOB_BASE_COST*base_fee with GAS_PER_BLOB*blob_fee.
 * Their ratio is exactly 16, so compare base_fee with 16*blob_fee instead of
 * forming either potentially 257+-bit product. */
function blob_reserve_price_active(base_fee : word, price : word) -> bool =
    if price[255 .. 252] == sail_zeros(4) then {
        word_ult(sail_shiftleft(price, 4), base_fee)
    } else {
        false
    }

function blob_schedule_target() -> blob_count = k_blob_schedule.target

function blob_schedule_max() -> blob_count = k_blob_schedule.max

function blob_gas_for_count(count : blob_count) -> blob_gas = {
    let product = GAS_PER_BLOB * count;
    if product <= 2 ^ 64 - 1 then {
        product
    } else {
        throw InvalidBlock(InvalidConfig)
    }
}

function blob_target_gas_per_block() -> blob_gas =
    blob_gas_for_count(blob_schedule_target())

function blob_max_gas_per_block() -> blob_gas =
    blob_gas_for_count(blob_schedule_max())

function next_excess_blob_gas(
    parent_excess_blob_gas : blob_gas,
    parent_blob_gas_used : blob_gas,
    parent_base_fee_per_gas : word,
) -> (
    blob_gas
) = {
    let parent_blob_gas = parent_excess_blob_gas + parent_blob_gas_used;
    let target_blob_gas = blob_target_gas_per_block();
    if parent_blob_gas < target_blob_gas then {
        0
    } else {
        let price = blob_base_fee(parent_excess_blob_gas);
        if (k_fork >= Osaka) & blob_reserve_price_active(parent_base_fee_per_gas, price) then {
            let target = blob_schedule_target();
            let maximum = blob_schedule_max();
            if (maximum == 0) | (maximum < target) then {
                throw InvalidBlock(InvalidConfig)
            };
            let schedule_delta = maximum - target;
            let scaled = parent_blob_gas_used * schedule_delta;
            let share = exact_quotient(scaled, maximum);
            let next = parent_excess_blob_gas + share;
            if next <= 2 ^ 64 - 1 then {
                next
            } else {
                throw InvalidBlock(InvalidConfig)
            }
        } else {
            let next = parent_blob_gas - target_blob_gas;
            if next <= 2 ^ 64 - 1 then {
                next
            } else {
                throw InvalidBlock(InvalidConfig)
            }
        }
    }
}

/* --- static cost constants (Yellow Paper Appendix G symbols G_*, as
 *     repriced by EIP-150/160/2929) --- */
let G_zero : gas_constant = GasConstant(0) /* YP G_zero                                  */
let G_jumpdest : gas_constant = GasConstant(1) /* YP G_jumpdest                              */
let G_base : gas_constant = GasConstant(2) /* YP G_base                                  */
let G_verylow : gas_constant = GasConstant(3) /* YP G_verylow                               */
let G_low : gas_constant = GasConstant(5) /* YP G_low                                   */
let G_mid : gas_constant = GasConstant(8) /* YP G_mid                                   */
let G_high : gas_constant = GasConstant(10) /* YP G_high                                  */
let G_warm_access : gas_constant = GasConstant(100) /* EIP-2929 G_warmaccess (warm SLOAD/access)  */
let G_cold_sload : gas_constant = GasConstant(2100) /* EIP-2929 G_coldsload surcharge             */
let G_cold_account : gas_constant = GasConstant(2600) /* EIP-2929 G_coldaccountaccess surcharge     */
let G_keccak : gas_constant = GasConstant(30) /* YP G_keccak256 (KECCAK256 base)            */
let G_keccak_word : gas_constant = GasConstant(6) /* YP G_keccak256word (per 32-byte word)      */
let G_copy_word : gas_constant = GasConstant(3) /* YP G_copy (*COPY per word)                 */
let G_memory : gas_constant = GasConstant(3) /* YP G_memory (per word of expansion)        */
let G_log : gas_constant = GasConstant(375) /* YP G_log (LOG base)                        */
let G_logtopic : gas_constant = GasConstant(375) /* YP G_logtopic (per topic)                  */
let G_logdata : gas_constant = GasConstant(8) /* YP G_logdata (per byte)                    */
let G_exp : gas_constant = GasConstant(10) /* YP G_exp (EXP base)                        */
let G_expbyte : gas_constant = GasConstant(50) /* YP G_expbyte; raised 10->50 by EIP-160     */
let G_sset : gas_constant = GasConstant(20000) /* EIP-2200 G_sset (zero -> nonzero SSTORE)   */
let G_sreset : gas_constant = GasConstant(2900) /* EIP-2200/2929 G_sreset (5000 - 2100 cold)  */
let R_sclear_pre_london : gas_constant = GasConstant(15000) /* EIP-2200 SSTORE-clears refund before EIP-3529 */
let R_sclear_london : gas_constant = GasConstant(4800) /* EIP-3529 SSTORE-clears refund                  */
let R_selfdestruct_pre_london : gas_constant = GasConstant(24000) /* SELFDESTRUCT refund removed by EIP-3529 */
let G_create : gas_constant = GasConstant(32000) /* YP G_create (CREATE/CREATE2 base)          */
let G_codedeposit : gas_constant = GasConstant(200) /* YP G_codedeposit (per deployed code byte)  */
let G_callvalue : gas_constant = GasConstant(9000) /* YP G_callvalue (CALL value-transfer)       */
let G_callstipend : gas = Gas(2300) /* YP G_callstipend (free gas to a value call) */
let G_newaccount : gas_constant = GasConstant(25000) /* YP G_newaccount (CALL/SELFDESTRUCT to new) */
let G_selfdestruct : gas_constant = GasConstant(5000) /* EIP-150 R_selfdestruct base                */
let G_initcode_word : gas_constant = GasConstant(2) /* EIP-3860 (per word of initcode)            */

/* --- protocol code-size limits --- */
/* EIP-170 deployed-code limit 24576, raised to 0x8000 = 32768 by EIP-7954 at
 * Amsterdam; the EIP-3860 initcode cap is always 2 * MAX_CODE_SIZE. Fork-
 * dependent so the shared interpreter serves both the pre-Amsterdam state-tests
 * (k_fork < Amsterdam) and the guest (k_fork defaults to 99). */
function max_code_size() -> code_length =
    if k_fork >= Amsterdam then {
        ByteQuantity(32768)
    } else {
        ByteQuantity(24576)
    }
function initcode_size_allowed(size : nat) -> bool =
    (k_fork < Shanghai) | size <= 2 * max_code_size()
function sstore_clear_refund() -> gas_constant =
    if k_fork >= London then { R_sclear_london } else { R_sclear_pre_london }

/* Intermediate costs remain exact in the canonical model. Optimized C builds
 * splice the complete cost calculators that can exceed the live-gas range;
 * affordability is still decided only by charge or an explicit comparison. */
function gas_product_quotient(left : nat, right : nat, divisor : gas_divisor) -> gas_cost =
    GasCost(exact_quotient(left * right, divisor))

/* --------------------------- charging hook ------------------------------ */
/* Debit `amount` from the frame's gas counter. A debit that cannot be       */
/* covered raises an out-of-gas exceptional halt (YP: the step is invalid,   */
/* all remaining gas is consumed). A non-positive amount is a no-op.         */

function charge_cost(GasCost(amount) : gas_cost) -> unit =
    if not_bool(is_running()) | amount == 0 then {
        ()
    } else {
        let Gas(remaining) = gas_remaining;
        if amount <= remaining then {
            gas_remaining = Gas(remaining - amount)
        } else {
            exc_halt(OutOfGas)
        }
    }

function charge_constant(GasConstant(amount) : gas_constant) -> unit =
    charge_cost(GasCost(amount))

overload charge = {charge_cost, charge_constant}

/* Subtraction is the one gas arithmetic operation whose invalid case is
 * observable: attempting to spend more than remains exceptionally halts. The
 * inert zero mirrors pop() after stack underflow and is ignored once halted. */
function gas_sub_cost_or_oog(Gas(left) : gas, GasCost(right) : gas_cost) -> gas =
    if right <= left then {
        Gas(left - right)
    } else {
        exc_halt(OutOfGas);
        GAS_ZERO
    }

function gas_sub_gas_or_oog(Gas(left) : gas, Gas(right) : gas) -> gas =
    if right <= left then {
        Gas(left - right)
    } else {
        exc_halt(OutOfGas);
        GAS_ZERO
    }

overload gas_sub_or_oog = {gas_sub_cost_or_oog, gas_sub_gas_or_oog}

function refund_gas(Gas(amount) : gas) -> unit = {
    let Gas(remaining) = gas_remaining;
    if amount <= (2 ^ 63 - 1) - remaining then {
        gas_remaining = Gas(remaining + amount)
    } else {
        throw InvalidBlock(ExecutionInvalid)
    }
}

function memory_word_count(byte_len : nat) -> nat = exact_quotient(byte_len + 31, 32)

/* --------------------- memory-expansion gas ----------------------------- */
/* Memory is charged by its high-water mark in 32-byte words (YP C_mem):     */
/* C_mem(a) = G_memory * a + floor(a^2 / 512), a quadratic term that makes   */
/* large allocations progressively dearer. Expansion gas is the DIFFERENCE   */
/* C_mem(new) - C_mem(old) -- you pay only for the words you newly reach.    */

function mem_cost(words : nat) -> gas_cost =
    G_memory * words + gas_product_quotient(words, words, 512)

/* Gas to expand memory so that byte range [0 .. new_bytes) is addressable;
 * rounds up to a word boundary and is zero when the watermark already covers
 * the range. */
/* A zero-sized operand does not access memory, irrespective of its offset. */
function memory_required_size(start : word, size : word) -> nat =
    if word_is_zero(size) then {
        0
    } else {
        unsigned(start) + unsigned(size)
    }

function maximum_memory_required_size(
    left_start : word,
    left_size : word,
    right_start : word,
    right_size : word,
) -> (
    nat
) = {
    let left = memory_required_size(left_start, left_size);
    let right = memory_required_size(right_start, right_size);
    if left < right then { right } else { left }
}

function memory_expansion_gas(required_size : nat) -> gas_cost = {
    let new_words = memory_word_count(required_size);
    let ByteQuantity(old_size) = evm_memory_size();
    let old_words = memory_word_count(old_size);
    if new_words <= old_words then {
        GAS_COST_ZERO
    } else {
        mem_cost(new_words) - mem_cost(old_words)
    }
}

function expand_memory(required_size : nat) -> unit = {
    let expanded_size = memory_word_count(required_size) * 32;
    assert(nat_fits_limb(expanded_size));
    memory_expand_to(ByteQuantity(expanded_size))
}

/* Charge before materializing the range. Exact arithmetic makes an
 * unaffordable transaction-controlled range out-of-gas before it reaches the
 * host memory boundary. */
function charge_memory_expansion(start : word, size : word) -> unit =
    if is_running() then {
        let required_size = memory_required_size(start, size);
        charge(memory_expansion_gas(required_size));
        if is_running() then {
            expand_memory(required_size)
        }
    }

/* Apply an already-charged range, used by CALL where memory gas is included
 * in the single EIP-150 message-call charge. */
function expand_charged_memory(required_size : nat) -> unit =
    if is_running() then {
        expand_memory(required_size)
    }

/* ----------------------- EIP-2929 access costs -------------------------- */
/* First touch of an account or storage slot in a transaction is COLD and    */
/* dear; a repeat touch is WARM and cheap (EIP-2929 mitigates state-access    */
/* DoS). The warm set is kernel state: the kernel marks warm (k_access_*)     */
/* and returns the PRIOR warm bit, and these helpers price that bit. No       */
/* state is mutated here -- this is pure cost policy.                         */

function account_cost(warm : bool) -> gas_constant =
    if warm then { G_warm_access } else { G_cold_account }

function sload_cost(warm : bool) -> gas_constant =
    if warm then { G_warm_access } else { G_cold_sload } /* EIP-2929: cold SLOAD = 2100 */

/* Precompile gas inspects the same ByteSlice that execution consumes. Bytes
 * past its declared length read as zero, matching buffer_read semantics. */

/* At most 32 big-endian bytes from precompile input. The bounded construction
 * stays in the EVM word algebra; out-of-range input bytes are zero. */
function pc_word(input : ByteSlice, start : nat, byte_count : nat) -> word = {
    var value : word = ZERO_WORD;
    let ByteQuantity(input_len) = input.len;
    foreach (byte_index from 0 to 31) {
        if byte_index < byte_count then {
            let cursor = start + byte_index;
            let next_byte = if cursor < input_len then {
                    slice_byte(input, ByteQuantity(cursor))
                } else {
                    0x00
                };
            value = or_vec(sail_shiftleft(value, 8), sail_zero_extend(next_byte, 256))
        }
    };
    value
}

function pc_blake2_rounds(input : ByteSlice) -> blake2_rounds =
    unsigned(pc_word(input, 0, 4)[31 .. 0])

/* MODEXP (precompile 0x05) gas, EIP-2565 as amended by EIP-7883 (Osaka).
 * Input layout: bsize[32] esize[32] msize[32] base[bsize] exp[esize] mod[msize].
 * Gas grows with the multiplication-complexity of the larger of base/mod and
 * with an iteration count derived from the exponent's bit length. */
function modexp_gas(input : ByteSlice) -> option(gas_cost) = {
    let bl_word = pc_word(input, 0, 32);
    let el_word = pc_word(input, 32, 32);
    let ml_word = pc_word(input, 64, 32);
    let bl : nat = unsigned(bl_word);
    let el : nat = unsigned(el_word);
    let ml : nat = unsigned(ml_word);
    let osaka = k_fork >= Osaka;

    /* EIP-7823 rejects the call before charging or executing MODEXP when any
     * length exceeds 1024. This is structural invalidity, distinct from an
     * unaffordable but otherwise valid gas expression. */
    if osaka & (bl > 1024 | el > 1024 | ml > 1024) then {
        return None()
    };

    /* Before Osaka the multiplication complexity is zero when both base and
     * modulus lengths are zero. The minimum cost is then exact even if the
     * unused exponent length does not fit a host quantity. */
    if not_bool(osaka) & word_is_zero(bl_word) & word_is_zero(ml_word) then {
        return Some(GasCost(200))
    };

    let maxlen = if ml < bl then { bl } else { ml };
    let words = exact_quotient(maxlen + 7, 8); /* length of base/mod in 8-byte words */

    /* multiplication complexity: EIP-2565 uses words^2; EIP-7883 (Osaka)
       uses a flat 16 for inputs <= 32 bytes, else 2*words^2. */
    let words_squared = words * words;
    let mult : nat = if osaka & maxlen <= 32 then {
            16
        } else {
            if osaka then {
                2 * words_squared
            } else {
                words_squared
            }
        };
    let big_mul : nat = if osaka then { 16 } else { 8 };
    let exp_off = 96 + bl; /* exponent begins after lengths + base */

    /* iteration count: for a short exponent, its bit length minus one; for a
       long exponent, big_mul per extra word past the first 32 bytes plus the
       leading word's bit length. */
    let iterations : nat = if el <= 32 then {
            let exponent_bits = word_bit_length(pc_word(input, exp_off, el));
            if exponent_bits == 0 then {
                1
            } else {
                let count = exponent_bits - 1;
                if count == 0 then { 1 } else { count }
            }
        } else {
            let head_bits = word_bit_length(pc_word(input, exp_off, 32));
            let high_bits = if head_bits != 0 then {
                    head_bits - 1
                } else {
                    0
                };
            let count = big_mul * (el - 32) + high_bits;
            if count == 0 then { 1 } else { count }
        };

    /* EIP-7883: gas = max(500, mult*it) (no /3); EIP-2565: max(200, mult*it/3) */
    let calculated : nat = if osaka then {
            mult * iterations
        } else {
            exact_quotient(mult * iterations, 3)
        };
    let minimum : nat = if osaka then { 500 } else { 200 };
    Some(GasCost(if calculated < minimum then { minimum } else { calculated }))
}

/* EIP-2537 BLS12-381 multi-scalar-multiplication discount tables. Entry k-1
 * is the per-pair discount (in thousandths) applied to a k-pair MSM, so a
 * larger MSM is progressively cheaper per pair; the discount is constant for
 * k >= 128. Indexed k = 1 .. 128 (G1 and G2 use distinct curves). */
let BLS_G1_DISCOUNT : vector(128, inc, bits(16)) = [
        0x03E8,
        0x03B5,
        0x0350,
        0x031D,
        0x02FC,
        0x02EE,
        0x02E2,
        0x02D8,
        0x02CF,
        0x02C8,
        0x02C1,
        0x02BA,
        0x02B4,
        0x02AF,
        0x02AA,
        0x02A5,
        0x02A1,
        0x029D,
        0x0299,
        0x0295,
        0x0292,
        0x028E,
        0x028B,
        0x0288,
        0x0285,
        0x0282,
        0x0280,
        0x027D,
        0x027B,
        0x0278,
        0x0276,
        0x0273,
        0x0271,
        0x026F,
        0x026D,
        0x026B,
        0x0269,
        0x0267,
        0x0265,
        0x0263,
        0x0261,
        0x0260,
        0x025E,
        0x025C,
        0x025B,
        0x0259,
        0x0257,
        0x0256,
        0x0254,
        0x0253,
        0x0251,
        0x0250,
        0x024F,
        0x024D,
        0x024C,
        0x024A,
        0x0249,
        0x0248,
        0x0246,
        0x0245,
        0x0244,
        0x0243,
        0x0241,
        0x0240,
        0x023F,
        0x023E,
        0x023D,
        0x023C,
        0x023A,
        0x0239,
        0x0238,
        0x0237,
        0x0236,
        0x0235,
        0x0234,
        0x0233,
        0x0232,
        0x0231,
        0x0230,
        0x022F,
        0x022E,
        0x022D,
        0x022C,
        0x022B,
        0x022A,
        0x0229,
        0x0228,
        0x0227,
        0x0226,
        0x0225,
        0x0224,
        0x0223,
        0x0223,
        0x0222,
        0x0221,
        0x0220,
        0x021F,
        0x021E,
        0x021D,
        0x021C,
        0x021C,
        0x021B,
        0x021A,
        0x0219,
        0x0218,
        0x0218,
        0x0217,
        0x0216,
        0x0215,
        0x0214,
        0x0214,
        0x0213,
        0x0212,
        0x0211,
        0x0210,
        0x0210,
        0x020F,
        0x020E,
        0x020D,
        0x020D,
        0x020C,
        0x020B,
        0x020A,
        0x020A,
        0x0209,
        0x0208,
        0x0208,
        0x0207,
    ]
let BLS_G2_DISCOUNT : vector(128, inc, bits(16)) = [
        0x03E8,
        0x03E8,
        0x039B,
        0x0374,
        0x0357,
        0x0340,
        0x032C,
        0x031C,
        0x030E,
        0x0302,
        0x02F7,
        0x02ED,
        0x02E4,
        0x02DC,
        0x02D4,
        0x02CD,
        0x02C7,
        0x02C0,
        0x02BB,
        0x02B5,
        0x02B0,
        0x02AB,
        0x02A7,
        0x02A2,
        0x029E,
        0x029A,
        0x0297,
        0x0293,
        0x028F,
        0x028C,
        0x0289,
        0x0286,
        0x0283,
        0x0280,
        0x027D,
        0x027A,
        0x0278,
        0x0275,
        0x0273,
        0x0270,
        0x026E,
        0x026C,
        0x026A,
        0x0267,
        0x0265,
        0x0263,
        0x0261,
        0x025F,
        0x025E,
        0x025C,
        0x025A,
        0x0258,
        0x0256,
        0x0255,
        0x0253,
        0x0251,
        0x0250,
        0x024E,
        0x024D,
        0x024B,
        0x024A,
        0x0248,
        0x0247,
        0x0246,
        0x0244,
        0x0243,
        0x0242,
        0x0240,
        0x023F,
        0x023E,
        0x023D,
        0x023B,
        0x023A,
        0x0239,
        0x0238,
        0x0237,
        0x0236,
        0x0235,
        0x0233,
        0x0232,
        0x0231,
        0x0230,
        0x022F,
        0x022E,
        0x022D,
        0x022C,
        0x022B,
        0x022A,
        0x0229,
        0x0228,
        0x0228,
        0x0227,
        0x0226,
        0x0225,
        0x0224,
        0x0223,
        0x0222,
        0x0221,
        0x0221,
        0x0220,
        0x021F,
        0x021E,
        0x021D,
        0x021D,
        0x021C,
        0x021B,
        0x021A,
        0x0219,
        0x0219,
        0x0218,
        0x0217,
        0x0217,
        0x0216,
        0x0215,
        0x0214,
        0x0214,
        0x0213,
        0x0212,
        0x0212,
        0x0211,
        0x0210,
        0x0210,
        0x020F,
        0x020E,
        0x020E,
        0x020D,
        0x020C,
        0x020C,
    ]

/* EIP-2537 MSM gas: gas = (k * base * discount(k)) / 1000, where discount(k)
 * is read from `table` and clamps to `maxd` (the k=128 entry) for k >= 128. */
function bls_msm_gas(table : vector(128, inc, bits(16)), base : gas_constant, maxd : bls_discount, k : nat) -> gas_cost = {
    if k == 0 then {
        GAS_COST_ZERO
    } else {
        var discount : bls_discount = maxd;
        if k < 128 then {
            let target = 128 - k;
            var cursor : nat = 0;
            foreach (index from 0 to 127) {
                if cursor == target then {
                    discount = unsigned(table[index])
                };
                cursor = cursor + 1
            }
        };
        ((base * discount) * k) / 1000
    }
}

function linear_gas(base : gas_constant, per_unit : gas_constant, units : nat) -> gas_cost =
    per_unit * units + base

/* Gas for the precompile at address `num`, by input length (the host returns
 * only output; gas is pure EVM policy). Length-only costs are computed from
 * `words`; the two input-dependent curves (MODEXP, BLAKE2F rounds) read the
 * input in place through pc_be. The cases below are the precompile catalog
 * with their addresses and pricing EIPs. */
function precompile_gas(num : precompile_id, input : ByteSlice) -> option(gas_cost) = {
    let ByteQuantity(input_len) = input.len;
    let words = memory_word_count(input_len);
    match num {
        1 => Some(GasCost(3000)), /* ECRECOVER (0x01) */
        2 => Some(linear_gas(GasConstant(60), GasConstant(12), words)),
        3 => Some(linear_gas(GasConstant(600), GasConstant(120), words)),
        4 => Some(linear_gas(GasConstant(15), GasConstant(3), words)),
        5 => modexp_gas(input), /* MODEXP (0x05), EIP-2565/7883 */
        6 => Some(GasCost(150)), /* BN_ADD (0x06), EIP-1108 */
        7 => Some(GasCost(6000)), /* BN_MUL (0x07), EIP-1108 */
        8 => Some(linear_gas(GasConstant(45000), GasConstant(34000), exact_quotient(input_len, 192))), /* BN_PAIRING (0x08), EIP-1108 */
        9 => Some(GasCost(pc_blake2_rounds(input))), /* BLAKE2F */
        10 => Some(GasCost(50000)), /* POINT_EVALUATION */
        /* EIP-2537 BLS12-381 */
        11 => Some(GasCost(375)), /* G1ADD */
        12 => Some(bls_msm_gas(BLS_G1_DISCOUNT, GasConstant(12000), 519, exact_quotient(input_len, 160))),
        13 => Some(GasCost(600)), /* G2ADD */
        14 => Some(bls_msm_gas(BLS_G2_DISCOUNT, GasConstant(22500), 524, exact_quotient(input_len, 288))),
        15 => Some(linear_gas(GasConstant(37700), GasConstant(32600), exact_quotient(input_len, 384))), /* PAIRING_CHECK */
        16 => Some(GasCost(5500)), /* MAP_FP_TO_G1 */
        17 => Some(GasCost(23800)), /* MAP_FP2_TO_G2 */
        256 => Some(GasCost(6900)), /* P256VERIFY */
        _ => Some(GAS_COST_ZERO),
    }
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
function sstore_gas(original : word, current : word, new : word, cold : bool) -> (gas_cost, gas_refund) = {
    let cold_cost : gas_constant = if cold then { G_cold_sload } else { GAS_CONSTANT_ZERO };
    let clear_refund : gas_refund = gas_constant_to_refund(sstore_clear_refund());
    let base : gas_constant = if current == new then {
            G_warm_access
        } else {
            if original == current then {
                if word_is_zero(original) then { G_sset } else { G_sreset }
            } else {
                G_warm_access
            }
        };
    var refund : gas_refund = GAS_REFUND_ZERO;
    if current != new then {
        if original == current then {
            /* clean slot being changed: refund only when clearing a non-zero slot */
            if not_bool(word_is_zero(original)) & word_is_zero(new) then {
                refund = refund + clear_refund
            }
        } else {
            /* dirty slot */
            if not_bool(word_is_zero(original)) then {
                if word_is_zero(current) then {
                    refund = refund - clear_refund
                }; /* un-clear */
                if word_is_zero(new) then {
                    refund = refund + clear_refund
                } /* re-clear */
            };
            if original == new then {
                /* reset to original */
                if word_is_zero(original) then {
                    refund = refund + gas_constant_to_refund(G_sset) - gas_constant_to_refund(G_warm_access)
                    /* 19900 */
                } else {
                    refund = refund + gas_constant_to_refund(G_sreset) - gas_constant_to_refund(G_warm_access)
                } /* 2800 */
            }
        }
    };
    let cost = base + cold_cost;
    (cost, refund)
}

/* ------------------------- per-op dynamic gas --------------------------- */
/* Operand-sized costs that ride on top of an opcode's static cost. All are   */
/* word-counted via ceil(byte_len / 32) where the spec charges per 32-byte    */
/* word (YP Appendix G). */

/* Per-word memory cost for an operand. Empty operands retain the opcode base. */
function charge_memory_word_gas(base : gas_constant, per_word : gas_constant, size : nat) -> unit =
    charge(per_word * memory_word_count(size) + base)

/* KECCAK256: base + per-word hashing cost. */
function charge_keccak_gas(size : nat) -> unit =
    charge_memory_word_gas(G_keccak, G_keccak_word, size)

/* *COPY (CALLDATA/CODE/RETURNDATA/EXTCODECOPY, MCOPY): per-word copy cost. */
function charge_copy_gas(size : nat) -> unit =
    charge_memory_word_gas(GAS_CONSTANT_ZERO, G_copy_word, size)

/* LOG0..LOG4: base + per-topic + per-data-byte. */
function charge_log_gas(num_topics : log_topic_count, size : nat) -> unit =
    charge(G_logtopic * num_topics + G_logdata * size + G_log)

/* EXP: base + G_expbyte per significant byte of the exponent (EIP-160). */
function exp_gas(exponent : word) -> gas_cost = {
    let significant_bits = word_bit_length(exponent);
    let byte_count = exact_quotient(significant_bits + 7, 8);
    let significant_bytes : range(0, 32) = if byte_count <= 32 then {
            byte_count
        } else {
            assert(false);
            0
        };
    G_expbyte * significant_bytes + G_exp
}

/* EIP-3860 initcode word cost, charged by CREATE/CREATE2 and tx-deploy. */
function initcode_gas(byte_len : nat) -> gas_cost =
    if k_fork >= Shanghai then {
        G_initcode_word * memory_word_count(byte_len)
    } else {
        GAS_COST_ZERO
    }

/* ----------------------- call gas (EIP-150 63/64) ----------------------- */
/* EIP-150 forwards at most all-but-one-64th of the available gas to a        */
/* sub-call, so a caller always retains floor(available/64) to finish its     */
/* own frame. The forwarded amount is min(requested, available - available/64). */
function call_gas_cap_word(available : gas, requested : word) -> gas = {
    let retained = gas_quotient(available, 64);
    let all_but_64th = gas_sub_or_oog(available, retained);
    match word_to_gas(requested) {
        Some(requested_gas) => {
            if requested_gas < all_but_64th then { requested_gas } else { all_but_64th }
        },
        _ => all_but_64th,
    }
}
