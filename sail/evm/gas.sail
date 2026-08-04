
/*md
# The gas schedule

The complete fork-gated EVM gas schedule (Yellow Paper Appendix G plus the
gas-repricing EIPs). Gas meters execution: every step debits the frame's
gas counter, and a debit that would underflow raises an out-of-gas
exceptional halt. This module owns the cost groups; the static per-opcode
cost is attached to each opcode in the dispatch module.

The cost groups, with their governing specifications:

- **Static `G_*` constants**: the YP Appendix G symbols, as repriced by
  EIP-150 (`SLOAD`/`CALL`/`SELFDESTRUCT`), EIP-160 (`G_expbyte`), and
  EIP-2929 (cold/warm).
- **Memory-expansion gas** (YP C_mem): quadratic in the high-water word
  count, charged on any addressed touch.
- **EIP-2929 access costs**: cold/warm surcharges for account and storage
  access (the kernel owns the warm set; the EVM only prices it).
- **EIP-2200 / EIP-3529 `SSTORE` cost and refund**, under the EIP-3529
  refund cap (applied at transaction scope, not here).
- **Per-opcode dynamic gas**: `KECCAK256` / `*COPY` per word, `LOG` per
  byte/topic, `EXP` per byte, EIP-3860 per initcode word.
- **Call gas**: the EIP-150 all-but-one-64th forwarding cap and the
  value-transfer stipend.
- **Per-precompile gas**, including the EIP-2565 / EIP-7883 `MODEXP`
  curve and the EIP-2537 BLS12-381 MSM discount tables.

## The blob-gas accounting

`blob_base_fee = fake_exponential(excess_blob_gas,
BLOB_BASE_FEE_UPDATE_FRACTION)`. The active target, maximum, and update
fraction come from the validated SSZ chain config, including BPO1/BPO2.
*/
let GAS_PER_BLOB : range(gas_per_blob_value, gas_per_blob_value) = sizeof(gas_per_blob_value)

/*! Narrows a protocol natural to an EVM word at the single boundary where
    word overflow changes validity. Intermediate arithmetic remains
    mathematical rather than acquiring modular EVM semantics. */
val protocol_word : forall 'value, 0 <= 'value & 'value < 2 ^ 256. int('value) -> word
function protocol_word(value) = u256(value)

/*! Evaluates EIP-4844's integer fake exponential at its observable EVM word
    boundary. The result is exact while it fits; once the monotonically
    increasing scaled sum proves that the final quotient is at least `2^256`,
    the checked conversion raises the block-validity exception.

    The schedule is an explicit argument so its correlated target, maximum,
    and update fraction remain visible to specialization. Every mutable value
    is bounded by equations over those singleton parameters. The one-word
    observation is semantic: execution-specs likewise converts the arbitrary
    precision result with a checked `u256` constructor. */
val fake_exponential_word :
    forall 'target 'maximum 'denominator 'numerator,
    blob_schedule_parameters('target, 'maximum, 'denominator)
    & 0 <= 'numerator
    & 'numerator
        <=   blob_fee_word_exponent_limit * 'denominator
           + ('maximum - 'target) * gas_per_blob_value.
    (
        BlobScheduleFields('target, 'maximum, 'denominator),
        int('numerator),
    ) -> word
function fake_exponential_word(schedule, numerator) = {
    let denominator : int('denominator) =
        schedule.base_fee_update_fraction;
    let scaled_limit : int('denominator * word_modulus) =
        denominator * sizeof(word_modulus);
    var term_index : range(1, 'denominator * word_modulus) = 1;
    var output : range(0, 'denominator * word_modulus - 1) = 0;
    var numerator_accum : range(
        0,
        'denominator * word_modulus * ('numerator + 1),
    ) = denominator;
    while
        numerator_accum > 0
        termination_measure(scaled_limit - output)
    do {
        let current_accum = numerator_accum;
        if scaled_limit <= current_accum then {
            throw Overflow()
        } else {
            let bounded_accum : range(
                0,
                'denominator * word_modulus - 1,
            ) = current_accum;
            let next_output = output + bounded_accum;
            if scaled_limit <= next_output then {
                throw Overflow()
            } else {
                output = next_output;
                numerator_accum =
                    (bounded_accum * numerator)
                    / (denominator * term_index);
                let current_index = term_index;
                if current_index < scaled_limit then {
                    term_index = current_index + 1
                } else {
                    /* Each preceding non-zero term increased `output` by at
                       least one, so this branch is unreachable before
                       `scaled_limit`. Retaining the finite guard makes that
                       invariant explicit to executable backends. */
                    throw Overflow()
                }
            }
        }
    };
    let price = output / denominator;
    if price < sizeof(word_modulus) then {
        protocol_word(price)
    } else {
        throw Overflow()
    }
}

/*! Evaluates the word result for one complete correlated profile.

    `ProtocolProfile` existentially packages the selected schedule. Dispatching
    on its fork opens that finite set at this boundary, so every call to the
    single generic recurrence carries a concrete schedule and can be lowered
    with fixed-width intermediates by optimized backends. */
function blob_base_fee(
    profile : ProtocolProfile,
    excess_blob_gas : excess_blob_gas,
) -> word = {
    if profile.fork == Cancun then {
        let schedule : cancun_blob_schedule = profile.blob_schedule;
        let limit = profile.excess_blob_gas_limit;
        if excess_blob_gas <= limit then {
            fake_exponential_word(schedule, excess_blob_gas)
        } else {
            throw InvalidBlock(InvalidConfig)
        }
    } else if profile.fork == Prague then {
        let schedule : prague_blob_schedule = profile.blob_schedule;
        let limit = profile.excess_blob_gas_limit;
        if excess_blob_gas <= limit then {
            fake_exponential_word(schedule, excess_blob_gas)
        } else {
            throw InvalidBlock(InvalidConfig)
        }
    } else if profile.fork == Osaka then {
        let schedule : osaka_blob_schedule = profile.blob_schedule;
        let limit = profile.excess_blob_gas_limit;
        if excess_blob_gas <= limit then {
            fake_exponential_word(schedule, excess_blob_gas)
        } else {
            throw InvalidBlock(InvalidConfig)
        }
    } else if profile.fork == BPO1 then {
        let schedule : bpo1_blob_schedule = profile.blob_schedule;
        let limit = profile.excess_blob_gas_limit;
        if excess_blob_gas <= limit then {
            fake_exponential_word(schedule, excess_blob_gas)
        } else {
            throw InvalidBlock(InvalidConfig)
        }
    } else if profile.fork == BPO2 then {
        let schedule : bpo2_blob_schedule = profile.blob_schedule;
        let limit = profile.excess_blob_gas_limit;
        if excess_blob_gas <= limit then {
            fake_exponential_word(schedule, excess_blob_gas)
        } else {
            throw InvalidBlock(InvalidConfig)
        }
    } else if profile.fork == Amsterdam then {
        let schedule : amsterdam_blob_schedule = profile.blob_schedule;
        let limit = profile.excess_blob_gas_limit;
        if excess_blob_gas <= limit then {
            fake_exponential_word(schedule, excess_blob_gas)
        } else {
            throw InvalidBlock(InvalidConfig)
        }
    } else {
        /* No pre-Cancun rule can observe a blob base fee. */
        throw InvalidBlock(InvalidConfig)
    }
}

/* Add transaction blob gas under the active block-wide schedule bound.  The
 * nested guards make the range proof available at the addition itself. */
/*! Adds transaction blob gas to the block total, rejecting the active
    schedule limit. A successful result is the exact sum, not merely a value
    somewhere inside the block blob-gas range. */
val block_blob_gas_add : forall 'accumulated 'transaction,
    0 <= 'accumulated
    & 'accumulated <= bpo2_blob_max_count * gas_per_blob_value
    & 0 <= 'transaction
    & 'transaction <= prague_blob_max_count * gas_per_blob_value.
    (
        blob_schedule_max_count,
        int('accumulated),
        int('transaction),
    ) -> {'result,
        'result == 'accumulated + 'transaction
        & 0 <= 'result
        & 'result <= bpo2_blob_max_count * gas_per_blob_value.
        int('result)}
function block_blob_gas_add(maximum_count, accumulated, transaction) = {
    let maximum = sizeof(gas_per_blob_value) * maximum_count;
    if accumulated <= maximum then {
        if transaction <= maximum - accumulated then {
            accumulated + transaction
        } else {
            throw InvalidBlock(BlobGasLimitExceeded)
        }
    } else {
        throw InvalidBlock(BlobGasLimitExceeded)
    }
}

/*! The header `excess_blob_gas` rule: decreases toward zero when the
    parent underused blobs, otherwise accumulates; from Osaka, EIP-7918
    substitutes the reserve-price form when the execution base fee
    dominates. */
function next_excess_blob_gas(
    profile : ProtocolProfile,
    parent_excess_blob_gas : excess_blob_gas,
    parent_blob_gas_used : blob_gas_used,
    parent_base_fee_per_gas : word,
) -> (
    excess_blob_gas
) = {
    let parent_blob_gas = parent_excess_blob_gas + parent_blob_gas_used;
    let target_blob_gas =
        sizeof(gas_per_blob_value) * profile.blob_schedule.target;
    if parent_blob_gas < target_blob_gas then {
        0
    } else {
        /* EIP-7918 compares BLOB_BASE_COST*base_fee with
           GAS_PER_BLOB*blob_fee. Their ratio is exactly 16, so this avoids
           forming either potentially 257+-bit product. */
        if
            (profile.fork >= Osaka)
            & (
                16 * blob_base_fee(profile, parent_excess_blob_gas)
                < parent_base_fee_per_gas
            )
        then {
            let maximum = profile.blob_schedule.max;
            if maximum == 0 then {
                throw InvalidBlock(InvalidConfig)
            };
            let share =
                (parent_blob_gas_used
                    * (maximum - profile.blob_schedule.target))
                / maximum;
            let next = parent_excess_blob_gas + share;
            let limit = profile.excess_blob_gas_limit;
            if next <= limit then {
                next
            } else {
                throw InvalidBlock(InvalidConfig)
            }
        } else {
            let next = parent_blob_gas - target_blob_gas;
            let limit = profile.excess_blob_gas_limit;
            if next <= limit then {
                next
            } else {
                throw InvalidBlock(InvalidConfig)
            }
        }
    }
}

/*md
## Constants

The Yellow Paper Appendix G symbols, as repriced by EIP-150, EIP-160, and
EIP-2929. The definitions below are normative; this table summarizes them.

| Name | Value | Description |
| ---- | ----- | ----------- |
| `G_zero` | `0` | `STOP`, `RETURN`, `REVERT` |
| `G_jumpdest` | `1` | `JUMPDEST` |
| `G_base` | `2` | Base-tier opcodes |
| `G_verylow` | `3` | Very-low-tier opcodes |
| `G_low` | `5` | Low-tier opcodes |
| `G_mid` | `8` | Mid-tier opcodes |
| `G_high` | `10` | High-tier opcodes (`JUMPI`) |
| `G_warm_access` | `100` | Warm account/storage access (EIP-2929) |
| `G_cold_sload` | `2100` | Cold `SLOAD` surcharge (EIP-2929) |
| `G_cold_account` | `2600` | Cold account-access surcharge (EIP-2929) |
| `G_keccak` | `30` | `KECCAK256` base |
| `G_keccak_word` | `6` | `KECCAK256` per 32-byte word |
| `G_copy_word` | `3` | `*COPY` per word |
| `G_memory` | `3` | Memory expansion, per word |
| `G_log` | `375` | `LOG` base |
| `G_logtopic` | `375` | `LOG` per topic |
| `G_logdata` | `8` | `LOG` per data byte |
| `G_exp` | `10` | `EXP` base |
| `G_expbyte` | `50` | `EXP` per exponent byte (EIP-160) |
| `G_sset` | `20000` | `SSTORE` zero → nonzero (EIP-2200) |
| `G_sreset` | `2900` | `SSTORE` reset (EIP-2200/EIP-2929) |
| `R_sclear_london` | `4800` | `SSTORE`-clears refund (EIP-3529) |
| `G_create` | `32000` | `CREATE`/`CREATE2` base |
| `G_codedeposit` | `200` | Per deployed code byte |
| `G_callvalue` | `9000` | `CALL` value transfer |
| `G_callstipend` | `2300` | Stipend to a value-receiving call |
| `G_newaccount` | `25000` | `CALL`/`SELFDESTRUCT` to a new account |
| `G_selfdestruct` | `5000` | `SELFDESTRUCT` base (EIP-150) |
| `G_initcode_word` | `2` | Per initcode word (EIP-3860) |
*/
let G_zero : gas_constant = 0 /* YP G_zero                                  */
let G_jumpdest : gas_constant = 1 /* YP G_jumpdest                              */
let G_base : gas_constant = 2 /* YP G_base                                  */
let G_verylow : gas_constant = 3 /* YP G_verylow                               */
let G_low : gas_constant = 5 /* YP G_low                                   */
let G_mid : gas_constant = 8 /* YP G_mid                                   */
let G_high : gas_constant = 10 /* YP G_high                                  */
let G_warm_access : gas_constant = 100 /* EIP-2929 G_warmaccess (warm SLOAD/access)  */
let G_cold_sload : gas_constant = 2100 /* EIP-2929 G_coldsload surcharge             */
let G_cold_account : gas_constant = 2600 /* EIP-2929 G_coldaccountaccess surcharge     */
let G_keccak : gas_constant = 30 /* YP G_keccak256 (KECCAK256 base)            */
let G_keccak_word : int(6) = 6 /* YP G_keccak256word (per 32-byte word)      */
let G_copy_word : gas_constant = 3 /* YP G_copy (*COPY per word)                 */

/* Its singleton type records the Yellow Paper's strictly-positive divisor,
 * allowing the type checker to preserve natural-number division. */
let G_memory : int(3) = 3 /* YP G_memory (per word of expansion)        */
let G_log : gas_constant = 375 /* YP G_log (LOG base)                        */
let G_logtopic : gas_constant = 375 /* YP G_logtopic (per topic)                  */
let G_logdata : gas_constant = 8 /* YP G_logdata (per byte)                    */
let G_exp : gas_constant = 10 /* YP G_exp (EXP base)                        */
let G_expbyte : gas_constant = 50 /* YP G_expbyte; raised 10->50 by EIP-160     */
let G_sset : gas_constant = 20000 /* EIP-2200 G_sset (zero -> nonzero SSTORE)   */
let G_sreset : gas_constant = 2900 /* EIP-2200/2929 G_sreset (5000 - 2100 cold)  */
let R_sclear_pre_london : gas_constant = 15000 /* EIP-2200 SSTORE-clears refund before EIP-3529 */
let R_sclear_london : gas_constant = 4800 /* EIP-3529 SSTORE-clears refund                  */
let R_selfdestruct_pre_london : gas_constant = 24000 /* SELFDESTRUCT refund removed by EIP-3529 */
let G_create : gas_constant = 32000 /* YP G_create (CREATE/CREATE2 base)          */
let G_codedeposit : int(200) = 200 /* YP G_codedeposit (per deployed code byte)  */
let G_callvalue : gas_constant = 9000 /* YP G_callvalue (CALL value-transfer)       */
let G_callstipend : gas = 2300 /* YP G_callstipend (free gas to a value call) */
let G_newaccount : gas_constant = 25000 /* YP G_newaccount (CALL/SELFDESTRUCT to new) */
let G_selfdestruct : gas_constant = 5000 /* EIP-150 R_selfdestruct base                */
let G_initcode_word : int(2) = 2 /* EIP-3860 (per word of initcode)            */

/* Amsterdam separates execution work from persistent-state growth. These
 * constants retain their unit in the name: execution costs debit
 * `gas_remaining`; state costs debit the frame's state-gas reservoir. */
let G_amsterdam_cold_account_access : gas_constant = 3000
let G_amsterdam_cold_storage_access : gas_constant = 3000
let G_amsterdam_storage_write : gas_constant = 10000
let G_amsterdam_account_write : gas_constant = 8000
let G_amsterdam_call_value : gas_constant = 10300
let G_amsterdam_create_access : gas_constant = 11000
let G_amsterdam_state_byte : gas_constant = 1530
let G_amsterdam_state_storage_set : state_gas_spill = 97920
let G_amsterdam_state_new_account : state_gas_spill = 183600
let G_amsterdam_state_auth_base : state_gas_spill = 35190
let R_amsterdam_storage_clear : gas_constant = 12480
let G_sstore_sentry : gas_cost = 2301

/*md
## The code-size limits
*/
/*! Whether returned runtime code may be deployed at the active fork. Every
    represented protocol profile has a deployed-code limit. */
val deployed_code_size_allowed : forall 'size, 0 <= 'size. int('size) -> bool
function deployed_code_size_allowed(size) =
{
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    size <= profile.deployed_code_size_limit
}

/*! Whether creation initcode may enter a frame at the active fork. EIP-3860
    activates at Shanghai and EIP-7954 raises its limit at Amsterdam. A zero
    profile value denotes that the rule is inactive. */
val initcode_size_allowed : forall 'size, 0 <= 'size. int('size) -> bool
function initcode_size_allowed(size) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let limit = profile.initcode_size_limit;
    (limit == 0) | (size <= limit)
}

/*! The `SSTORE`-clears refund: 4800 from London (EIP-3529), 15000
    before. */
function sstore_clear_refund() -> gas_constant =
{
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if profile.fork >= London then { R_sclear_london } else { R_sclear_pre_london }
}

/* --------------------------- charging hook ------------------------------ */
/* Debit `amount` from the frame's gas counter. A debit that cannot be       */
/* covered raises an out-of-gas exceptional halt (YP: the step is invalid,   */
/* all remaining gas is consumed). A non-positive amount is a no-op.         */

/*! Debits an exact cost from the running frame. Returns false after entering
    an out-of-gas exceptional halt, allowing the current opcode body to return
    immediately to the interpreter loop without polling [is_running][]. */
val charge : forall 'amount, 0 <= 'amount. int('amount) -> bool
function charge(amount) =
    if not_bool(is_running()) then {
        false
    } else if amount == 0 then {
        true
    } else {
        let remaining = gas_remaining;
        if amount <= remaining then {
            gas_remaining = remaining - amount;
            true
        } else {
            exc_halt(OutOfGas);
            false
        }
    }

/*! Checks that the frame can pay an execution-gas cost without debiting it.
    State reads that affect the block access list call this before touching
    authenticated state. */
val check_execution_gas : forall 'amount, 0 <= 'amount. int('amount) -> unit
function check_execution_gas(amount) = {
    let remaining = gas_remaining;
    if is_running() & remaining < amount then {
        exc_halt(OutOfGas)
    }
}

/*! Adds state-gas spill while preserving EIP-8037's regular-gas cap. */
val state_gas_spill_room : forall 'left, 0 <= 'left & 'left <= transaction_execution_gas_limit_value. int('left) -> int(
        transaction_execution_gas_limit_value - 'left,
    )
function state_gas_spill_room(left) = sizeof(transaction_execution_gas_limit_value) - left

val state_gas_spill_add : forall 'right, 0 <= 'right. (state_gas_spill, int('right)) -> state_gas_spill
/*! Adds a nonnegative spill amount or rejects a value beyond the transaction cap. */
function state_gas_spill_add(left, right) =
    if right <= state_gas_spill_room(left) then {
        left + right
    } else {
        throw InvalidBlock(ExecutionInvalid)
    }

/*! Debits Amsterdam state gas from the frame reservoir first and spills the
    remainder into execution gas. Returns false when the two pools cannot
    cover the charge. */
val debit_state_gas : forall 'amount, 0 <= 'amount. int('amount) -> bool
function debit_state_gas(amount) = {
    if amount == 0 then {
        return true
    };
    let state_left = state_gas_remaining;
    let execution_left = gas_remaining;
    if amount <= state_left then {
        state_gas_remaining = state_left - amount;
        true
    } else {
        let remainder = amount - state_left;
        if remainder <= execution_left then {
            let spilled = state_gas_spilled;
            state_gas_remaining = GAS_ZERO;
            gas_remaining = execution_left - remainder;
            state_gas_spilled = state_gas_spill_add(spilled, remainder);
            true
        } else {
            false
        }
    }
}

/*! Charges state gas while an opcode frame is running. */
val charge_state_gas : forall 'amount, 0 <= 'amount. int('amount) -> unit
function charge_state_gas(amount) = {
    if is_running() & not_bool(debit_state_gas(amount)) then {
        exc_halt(OutOfGas)
    }
}

/*! Charges state gas for code returned by already-halted successful initcode.
    This is the create-message post-execution hook, not another opcode step. */
val charge_deployment_state_gas : forall 'amount, 0 <= 'amount. int('amount) -> unit
function charge_deployment_state_gas(amount) = {
    if not_bool(debit_state_gas(amount)) then {
        exc_halt(OutOfGas)
    }
}

/*! Credits a state-gas charge in exact reverse order: execution spill first,
    then the state reservoir. */
function credit_state_gas_refund(amount : state_gas_spill) -> unit = {
    let spilled = state_gas_spilled;
    if amount <= spilled then {
        if amount != 0 then {
            gas_remaining = conserved_gas_add(gas_remaining, amount);
            state_gas_spilled = spilled - amount
        }
    } else {
        if spilled != 0 then {
            gas_remaining = conserved_gas_add(gas_remaining, spilled);
            state_gas_spilled = STATE_GAS_SPILL_ZERO
        };
        let to_state : state_gas_spill = amount - spilled;
        state_gas_remaining = conserved_gas_add(state_gas_remaining, to_state)
    }
}

/*! Returns a completed child's remaining state reservoir to its parent and
    carries forward any state gas that the child drew from execution gas. */
function return_child_state_gas(child_remaining : gas, child_spilled : state_gas_spill) -> unit = {
    state_gas_remaining = conserved_gas_add(state_gas_remaining, child_remaining);
    state_gas_spilled = state_gas_spill_add(state_gas_spilled, child_spilled)
}

/* Subtraction is the one gas arithmetic operation whose invalid case is
 * observable: attempting to spend more than remains exceptionally halts. The
 * inert zero mirrors pop() after stack underflow and is ignored once halted. */
/*! Subtracts an exact cost from gas or records an out-of-gas halt. */
function gas_sub_or_oog(left : gas, right : gas_cost) -> gas =
    if right <= left then {
        left - right
    } else {
        exc_halt(OutOfGas);
        GAS_ZERO
    }

/*! Returns unused child gas to the parent frame. */
function refund_gas(amount : gas) -> unit = {
    gas_remaining = conserved_gas_add(gas_remaining, amount)
}

/*! Returns the number of 32-byte words covering a byte length. Besides the
    exact ceiling division, the result exposes its enclosing byte interval so
    affordability proofs can establish host-range bounds without a second
    runtime size check. */
type memory_word_count_relation('byte_len : Int, 'words : Int) -> Bool =
    0 <= 'byte_len
    & 'words == div('byte_len + 31, 32)
    & 'byte_len <= 32 * 'words
    & 32 * 'words <= 'byte_len + 31

val memory_word_count : forall 'byte_len,
    0 <= 'byte_len.
    int('byte_len) -> {
        'words,
        memory_word_count_relation('byte_len, 'words).
        int('words)
    }
function memory_word_count(byte_len) = {
    let quotient = tdiv_nat(byte_len, 32);
    if tmod_nat(byte_len, 32) == 0 then {
        quotient
    } else {
        quotient + 1
    }
}

/*! Returns the number of words covering a word-sized byte length without
    forming `byte_len + 31`, whose mathematical intermediate can require 257
    bits even though the final quotient remains an EVM word. */
function memory_word_count_word(byte_len : word) -> word = {
    let quotient = word_div(byte_len, u256(32));
    if word_mod(byte_len, u256(32)) == WORD_ZERO then {
        quotient
    } else {
        word_add(quotient, WORD_ONE)
    }
}

/*md
## The memory-expansion cost

Memory is charged by its high-water mark in 32-byte words (YP C_mem):
`C_mem(a) = G_memory·a + ⌊a²/512⌋`, a quadratic term that makes large
allocations progressively dearer. Expansion gas is the difference
`C_mem(new) − C_mem(old)` — only newly reached words are paid for.
*/
/*! `C_mem` (YP §9.4.1): the cumulative memory cost of `words` words. Its
    singleton result makes the gas-derived memory bound available to the type
    checker without introducing a separate memory-size constant. */
type memory_cost_relation('words : Int, 'cost : Int) -> Bool =
    0 <= 'words
    & 'cost == 3 * 'words + div('words * 'words, 512)

val mem_cost : forall 'words,
    memory_cost_input('words).
    int('words) -> {
        'cost,
        memory_cost_relation('words, 'cost).
        int('cost)
    }
function mem_cost(words) = {
    let linear : int(3 * 'words) = G_memory * words;
    let square : int('words * 'words) = words * words;
    let quadratic : int(div('words * 'words, 512)) = square / 512;
    linear + quadratic
}

/* A zero-sized operand does not access memory, irrespective of its offset. */
/*! Canonicalizes one operand and retains its exact exclusive endpoint. */
val memory_access : forall 'start 'size,
    0 <= 'start
    & 'start < 2 ^ 256
    & 0 <= 'size
    & 'size < 2 ^ 256.
    (
        int('start),
        int('size),
    ) -> MemoryAccess
function memory_access(start, size) =
    if size == 0 then {
        EMPTY_MEMORY_ACCESS
    } else {
        if start <= sizeof(memory_region_bound) then {
            let (bounded_start as 'bounded_start) :
                    {'bounded_start,
                     'bounded_start == 'start & 0 <= 'bounded_start & 'bounded_start <= memory_region_bound.
                     int('bounded_start)} = start;
            if size <= sizeof(memory_region_bound) - bounded_start then {
                let (bounded_size as 'bounded_size) :
                        {'bounded_size,
                            'bounded_size
                         == 'size
                         &  0
                         <  'bounded_size
                         &  'bounded_size
                         <= memory_region_bound
                         -  'bounded_start.
                         int('bounded_size)} = size;
                struct {
                    range = memory_range(bounded_start, bounded_size),
                    required_size = bounded_start + bounded_size,
                } :
                    MemoryAccessFields('bounded_start, 'bounded_size, 'bounded_start + 'bounded_size)
            } else {
                exc_halt(OutOfGas);
                EMPTY_MEMORY_ACCESS
            }
        } else {
            exc_halt(OutOfGas);
            EMPTY_MEMORY_ACCESS
        }
    }

/*! Computes an affordable incremental cost for reaching a memory size.
    The result type records that the exact `C_mem` delta fits the live gas
    supplied by the caller. An unaffordable expansion halts before any host
    memory is materialized. */
val memory_expansion_cost : forall 'required 'available,
    memory_expansion_endpoint('required)
    & live_gas_valid('available).
    (
        int('required),
        int('available),
    ) -> range(0, 'available)

function memory_expansion_cost(required_size, available) = {
    let new_words = memory_word_count(required_size);
    let old_words = memory_word_count(evm_memory_high_water());
    if new_words <= old_words then {
        0
    } else {
        let old_cost = mem_cost(old_words);
        let new_cost = mem_cost(new_words);
        if old_cost <= new_cost then {
            let cost = new_cost - old_cost;
            if cost <= available then {
                cost
            } else {
                exc_halt(OutOfGas);
                0
            }
        } else {
            exc_halt(OutOfGas);
            0
        }
    }
}

/*! The invariant retained by an affordable expansion plan. */
type memory_expansion_relation(
    'required : Int,
    'available : Int,
    'cost : Int,
) -> Bool =
    memory_expansion_endpoint('required)
    & live_gas_valid('available)
    & 0 <= 'cost
    & 'cost <= 'available

/*! A shared memory expansion plan. Logical operands retain their own
    `MemoryAccess` values; this value contains only the maximum endpoint and
    the one incremental cost paid for reaching it. */
struct MemoryExpansionFields(
    'required : Int,
    'available : Int,
    'cost : Int,
), memory_expansion_relation('required, 'available, 'cost) = {
    required_size : int('required),
    cost : int('cost),
}

type MemoryExpansion('available : Int) = {
    'required 'cost,
    memory_expansion_relation('required, 'available, 'cost).
    MemoryExpansionFields('required, 'available, 'cost)
}

/*! Plans the exact expansion required by a maximum exclusive endpoint. */
val memory_expansion : forall 'required 'available,
    memory_expansion_endpoint('required)
    & live_gas_valid('available).
    (
        int('required),
        int('available),
    ) -> MemoryExpansion('available)

function memory_expansion(required_size, available) = {
    let (cost as 'cost) = memory_expansion_cost(required_size, available);
    (struct {
        required_size = required_size,
        cost = cost,
    } : MemoryExpansionFields('required, 'available, 'cost))
}

/*! Materializes the exact byte high-water mark after gas has been charged. */
function expand_memory(required_size : memory_length) -> unit = {
    let _ = memory_expand_to(required_size);
    ()
}

/* Charge before materializing the range. Exact arithmetic makes an
 * unaffordable transaction-controlled range out-of-gas before it reaches the
 * host memory boundary. */
/*! Materializes an already-charged expansion plan. */
val apply_memory_expansion : forall 'available,
    live_gas_valid('available).
    MemoryExpansion('available) -> unit

function apply_memory_expansion(plan : MemoryExpansion('available)) ->
    unit = expand_memory(plan.required_size)

/*! Charges and materializes the expansion implied by one memory range. */
val charge_memory_range : forall 'size, 0 <= 'size & 'size < 2 ^ 256. (word, int('size)) -> MemoryRange
function charge_memory_range(start, size) =
    if is_running() then {
        let available = gas_remaining;
        let access = memory_access(start, size);
        let plan = memory_expansion(access.required_size, available);
        if charge(plan.cost) then {
            apply_memory_expansion(plan);
            access.range
        } else {
            EMPTY_MEMORY_RANGE
        }
    } else {
        EMPTY_MEMORY_RANGE
    }

/*md
## The access costs

The first touch of an account or storage slot in a transaction is cold
and dear; a repeat touch is warm and cheap (EIP-2929 mitigates
state-access DoS). The warm set is kernel state: the kernel marks warm
and returns the prior warm bit, and these helpers price that bit — pure
cost policy, no state mutation.
*/
/*! The account-access cost for a prior warm bit. */
function account_cost(warm : bool) -> gas_constant =
{
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if warm then {
        G_warm_access
    } else {
        if profile.fork >= Amsterdam then { G_amsterdam_cold_account_access } else { G_cold_account }
    }
}

/*! The second database read performed by `EXTCODESIZE` and `EXTCODECOPY`.
    EIP-8038 prices the code-store read as one warm access at Amsterdam. */
function external_code_read_cost() -> gas_constant =
{
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if profile.fork >= Amsterdam then { G_warm_access } else { G_zero }
}

/*! The `SLOAD` cost for a prior warm bit (cold = 2100, EIP-2929). */
function sload_cost(warm : bool) -> gas_constant =
{
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if warm then {
        G_warm_access
    } else {
        if profile.fork >= Amsterdam then { G_amsterdam_cold_storage_access } else { G_cold_sload }
    }
}

/*! The execution-gas component of a value-bearing CALL/CALLCODE. Amsterdam
    reprices the account write while retaining the 2300 child stipend. */
function call_value_cost() -> gas_constant =
{
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if profile.fork >= Amsterdam then { G_amsterdam_call_value } else { G_callvalue }
}

/*! The CREATE/CREATE2 execution-access charge. Persistent account growth is
    charged separately as state gas at Amsterdam. */
function create_access_cost() -> gas_constant =
{
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if profile.fork >= Amsterdam then { G_amsterdam_create_access } else { G_create }
}

/*! Returns the affordable execution-gas charge after successful initcode.
    Legacy forks charge per byte; Amsterdam charges the keccak word cost. The
    affordability guard bounds the native product without imposing a protocol
    code-size limit on Frontier or Homestead. */
function code_deployment_execution_cost(byte_len : code_length, available : gas) -> option(gas_cost) =
{
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if profile.fork >= Amsterdam then {
        if byte_len <= profile.deployed_code_size_limit then {
            let words = memory_word_count(byte_len);
            if words <= available / G_keccak_word then {
                let cost : gas_cost = G_keccak_word * words;
                Some(cost)
            } else {
                None()
            }
        } else {
            None()
        }
    } else {
        if byte_len <= available / G_codedeposit then {
            let cost : gas_cost = G_codedeposit * byte_len;
            Some(cost)
        } else {
            None()
        }
    }
}

/*! Amsterdam state gas charged for each byte of newly deployed code. */
function code_deployment_state_cost(byte_len : code_length) -> gas_cost =
{
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if profile.fork >= Amsterdam then {
        if byte_len <= profile.deployed_code_size_limit then {
            G_amsterdam_state_byte * byte_len
        } else {
            throw InvalidBlock(ExecutionInvalid)
        }
    } else {
        GAS_COST_ZERO
    }
}

/* At most 32 big-endian bytes from precompile input. The bounded construction
 * stays in the EVM word algebra; out-of-range input bytes are zero. */
/*md
## The precompile costs

Precompile gas inspects the same byte slice that execution consumes;
bytes past its declared length read as zero, matching
[calldata_slice_byte][] semantics.
*/
/*! Reads up to 32 big-endian precompile input bytes with zero padding. */
val pc_word : forall 'start 'byte_count, source_valid_length('start) & 0 <= 'byte_count & 'byte_count <= 32. (
        CalldataSlice,
        int('start),
        int('byte_count),
    ) -> word
function pc_word(input, start, byte_count) = {
    var value : word = ZERO_WORD;
    let start_offset = start;
    let count = byte_count;
    let input_length = calldata_slice_length(input);
    foreach (byte_index from 0 to 31) {
        let offset : range(0, 31) = byte_index;
        if offset < count then {
            let next_byte = if start_offset < input_length then {
                    let remaining = input_length - start_offset;
                    if offset < remaining then {
                        let cursor = start_offset + offset;
                        slice_byte(input, cursor)
                    } else {
                        0x00
                    }
                } else {
                    0x00
                };
            value = word_add(word_shift_left(value, 8), unsigned(next_byte))
        }
    };
    value
}

/*! Reads a precompile word after a fixed header and a 256-bit declared field.
    The declared length remains in the EVM word domain until the resulting
    position is known to lie inside `input`; an out-of-range position has the
    same all-zero result as an ordinary padded read. */
val pc_word_after_declared_field : forall 'prefix 'byte_count,
                                     source_valid_length('prefix) & 0 <= 'byte_count & 'byte_count <= 32. (
        CalldataSlice,
        int('prefix),
        word,
        int('byte_count),
    ) -> word
function pc_word_after_declared_field(input, prefix, declared_length, byte_count) = {
    let input_length = calldata_slice_length(input);
    let (prefix_offset as 'prefix_offset) = prefix;
    if prefix_offset < input_length then {
        let (suffix_length as 'suffix_length) = input_length - prefix_offset;
        if declared_length < suffix_length then {
            let (field_length as 'field_length) :
                    {'field_length,
                     0 <= 'field_length & 'field_length < 'suffix_length.
                     int('field_length)} = declared_length;
            let start : int('prefix_offset + 'field_length) = prefix_offset + field_length;
            pc_word(input, start, byte_count)
        } else {
            ZERO_WORD
        }
    } else {
        ZERO_WORD
    }
}

function pc_blake2_rounds(input : CalldataSlice) -> blake2_rounds =
    tmod_nat(pc_word(input, 0, 4), 2 ^ 32)

/*! `MODEXP` (precompile `0x05`) gas: EIP-2565 as amended by EIP-7883
    (Osaka). Input layout: `bsize[32] esize[32] msize[32] base exp mod`.
    Gas grows with the multiplication complexity of the larger of base
    and modulus, and with an iteration count derived from the exponent's
    bit length; EIP-7823 (Osaka) caps each field at 1024 bytes, modeled
    as an unforwardable cost. Before Osaka the exact expression is below
    `2^768`, but an EVM caller can observe only whether that expression fits
    its available gas. The staged affordability checks below therefore keep
    every materialized intermediate at most 256 bits in optimized builds. */
function modexp_gas(input : CalldataSlice, available : gas) -> option(gas_cost) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let bl_word = pc_word(input, 0, 32);
    let el_word = pc_word(input, 32, 32);
    let ml_word = pc_word(input, 64, 32);
    let bl : word = bl_word;
    let el : word = el_word;
    let ml : word = ml_word;
    let osaka = profile.fork >= Osaka;

    /* EIP-7823 rejects the call before charging or executing MODEXP when any
     * length exceeds 1024. This is structural invalidity, distinct from an
     * unaffordable but otherwise valid gas expression. */
    if osaka & (bl > 1024 | el > 1024 | ml > 1024) then {
        return None()
    };

    let minimum : gas_constant = if osaka then { 500 } else { 200 };
    if available < minimum then {
        return None()
    };

    /* Before Osaka the multiplication complexity is zero when both base and
     * modulus lengths are zero. The minimum cost is then exact even if the
     * unused exponent length does not fit a host quantity. */
    if not_bool(osaka) & word_is_zero(bl_word) & word_is_zero(ml_word) then {
        return Some(200)
    };

    let maxlen = if ml < bl then { bl } else { ml };
    let whole_words = maxlen / 8;
    let words = whole_words + (if tmod_nat(maxlen, 8) == 0 then { 0 } else { 1 });

    /* For EIP-2565, floor(product / 3) is affordable exactly when product is
     * at most 3*available+2. Bounding each factor by this threshold before
     * multiplication avoids constructing the protocol's enormous exact cost. */
    let product_limit_value : modexp_factor = if osaka then {
            available
        } else {
            3 * available + 2
        };
    let (product_limit as 'product_limit) = product_limit_value;
    let words_unaffordable = product_limit < words;
    let bounded_words : modexp_factor = if words_unaffordable then { product_limit } else { words };
    if words_unaffordable then {
        return None()
    };

    /* The clamped factors are at most 66 bits, so their exact square remains
       a bounded native value.  Narrow only after comparing it with the
       affordability limit. */
    let words_squared : modexp_product = bounded_words * bounded_words;
    let wide_product_limit : int('product_limit) = product_limit;
    let square_unaffordable = wide_product_limit < words_squared;
    let affordable_square : modexp_factor = if square_unaffordable then { product_limit } else { words_squared };
    if square_unaffordable then {
        return None()
    };

    /* multiplication complexity: EIP-2565 uses words^2; EIP-7883 (Osaka)
       uses a flat 16 for inputs <= 32 bytes, else 2*words^2. */
    let mult_value : modexp_factor = if osaka & maxlen <= 32 then {
            16
        } else {
            if osaka then {
                let doubled_limit_value : gas_cost = available / 2;
                let (doubled_limit as 'doubled_limit) = doubled_limit_value;
                let wide_doubled_limit : int('doubled_limit) = doubled_limit;
                let doubled_unaffordable = words_squared > wide_doubled_limit;
                let bounded_square : gas_cost = if doubled_unaffordable then { 0 } else { words_squared };
                if doubled_unaffordable then {
                    return None()
                };
                2 * bounded_square
            } else {
                affordable_square
            }
        };
    let (mult as 'mult) = mult_value;
    if product_limit < mult then {
        return None()
    };

    /* iteration count: for a short exponent, its bit length minus one; for a
       long exponent, big_mul per extra word past the first 32 bytes plus the
       leading word's bit length. */
    let iterations : modexp_factor = if el <= 32 then {
            let exponent_bits = word_bit_length(pc_word_after_declared_field(input, 96, bl, el));
            if exponent_bits == 0 then {
                1
            } else {
                let count = exponent_bits - 1;
                if count == 0 then { 1 } else { count }
            }
        } else {
            let head_bits = word_bit_length(pc_word_after_declared_field(input, 96, bl, 32));
            let high_bits = if head_bits != 0 then {
                    head_bits - 1
                } else {
                    0
                };
            let extra = el - 32;
            let count : modexp_factor = if osaka then {
                    let extra_limit : modexp_osaka_extra = available / 16;

                    /* The Osaka structural check above has already rejected
                       an exponent length above 1024.  Keep the successful
                       subtraction in its exact 992-byte host domain. */
                    let osaka_extra : range(0, 992) = if word_greater_than_word(extra, u256(992)) then {
                            992
                        } else {
                            extra
                        };
                    let extra_unaffordable = osaka_extra > extra_limit;
                    let bounded_extra : modexp_osaka_extra = if extra_unaffordable then {
                            extra_limit
                        } else {
                            osaka_extra
                        };
                    if extra_unaffordable then {
                        return None()
                    };
                    16 * bounded_extra + high_bits
                } else {
                    let pre_osaka_limit = 3 * available + 2;
                    let extra_limit : modexp_pre_osaka_extra = pre_osaka_limit / 8;
                    let extra_unaffordable = extra > extra_limit;
                    let bounded_extra : modexp_pre_osaka_extra = if extra_unaffordable then {
                            extra_limit
                        } else {
                            extra
                        };
                    if extra_unaffordable then {
                        return None()
                    };
                    8 * bounded_extra + high_bits
                };
            if count == 0 then { 1 } else { count }
        };

    /* EIP-7883: gas = max(500, mult*it) (no /3); EIP-2565: max(200, mult*it/3) */
    if iterations == 0 then {
        None()
    } else {
        let product : modexp_product = mult * iterations;
        let product_unaffordable = wide_product_limit < product;
        let affordable_product : modexp_factor = if product_unaffordable then { product_limit } else { product };
        if product_unaffordable then {
            None()
        } else {
            let calculated = if osaka then {
                    affordable_product
                } else {
                    affordable_product / 3
                };
            let cost = if calculated < minimum then { minimum } else { calculated };
            if cost <= available then {
                let affordable : gas_cost = cost;
                Some(affordable)
            } else {
                None()
            }
        }
    }
}

/*! The EIP-2537 BLS12-381 G1 MSM discount table: entry `k−1` is the
    per-pair discount (in thousandths) applied to a `k`-pair MSM,
    constant for `k ≥ 128`. */
let BLS_G1_DISCOUNT : vector(128, inc, bls_discount) = [
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
    ]
let BLS_G2_DISCOUNT : vector(128, inc, bls_discount) = [
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
    ]

/*! EIP-2537 MSM gas: `(k · base · discount(k)) / 1000`, with the
    discount clamped to the `k = 128` entry beyond the table. */
function bls_msm_gas(
    table : vector(128, inc, bls_discount),
    base : gas_constant,
    maxd : bls_discount,
    k : source_length,
    available : gas,
) -> (
    option(gas_cost)
) = {
    if k == 0 then {
        Some(GAS_COST_ZERO)
    } else {
        var discount : bls_discount = maxd;
        if k < 128 then {
            /* Sail vector literals place the first table entry at index 127;
             * the clamped k = 128 entry is supplied by maxd (index 0). */
            let index : range(1, 127) = 128 - k;
            discount = table[index]
        };
        let coefficient : bls_msm_coefficient = base * discount;
        let whole = k / 1000;
        let remainder = tmod_nat(k, 1000);
        let whole_product : bls_msm_product = coefficient * whole;
        if whole_product > available then {
            None()
        } else {
            let major : gas_cost = whole_product;
            let tail_product : bls_msm_tail_product = coefficient * remainder;
            let tail : gas_cost = tail_product / 1000;
            let total : bls_msm_product = major + tail;
            if total <= available then {
                let affordable : gas_cost = total;
                Some(affordable)
            } else {
                None()
            }
        }
    }
}

/*! Returns a linear precompile cost only after the caller can afford its
    transaction-controlled multiplier. */
function linear_gas(
    base : gas_constant,
    per_unit : gas_constant,
    units : source_length,
    available : gas,
) -> (
    option(gas_cost)
) = {
    let variable_cost : linear_gas_variable_product = per_unit * units;
    let exact_cost : linear_gas_product = variable_cost + base;
    if exact_cost > available then {
        None()
    } else {
        let affordable : gas_cost = exact_cost;
        Some(affordable)
    }
}

/*! Narrows a structurally bounded precompile charge to the caller's live gas
    domain. */
val fixed_precompile_gas : forall 'cost, 0 <= 'cost. (int('cost), gas) -> option(gas_cost)
function fixed_precompile_gas(cost, available) =
    if cost <= available then {
        let affordable : gas_cost = cost;
        Some(affordable)
    } else {
        None()
    }

/*! The gas of the precompile at address `num` for a given input. Gas is
    protocol policy defined entirely here; implementations return only
    output. Length-only costs
    derive from the word count; the two input-dependent curves (`MODEXP`,
    `BLAKE2F` rounds) read the input in place. The match arms are the
    precompile catalog with their addresses and pricing EIPs. */
function precompile_gas(num : precompile_id, input : CalldataSlice, available : gas) -> option(gas_cost) = {
    let input_len = calldata_slice_length(input);
    let input_length = input_len;
    let words = memory_word_count(input_len);
    match num {
        1 => fixed_precompile_gas(3000, available), /* ECRECOVER (0x01) */
        2 => linear_gas(60, 12, words, available),
        3 => linear_gas(600, 120, words, available),
        4 => linear_gas(15, 3, words, available),
        5 => modexp_gas(input, available), /* MODEXP (0x05), EIP-2565/7883 */
        6 => fixed_precompile_gas(150, available), /* BN_ADD (0x06), EIP-1108 */
        7 => fixed_precompile_gas(6000, available), /* BN_MUL (0x07), EIP-1108 */
        8 => linear_gas(45000, 34000, input_length / 192, available), /* BN_PAIRING (0x08), EIP-1108 */
        9 => fixed_precompile_gas(pc_blake2_rounds(input), available), /* BLAKE2F */
        10 => fixed_precompile_gas(50000, available), /* POINT_EVALUATION */
        /* EIP-2537 BLS12-381 */
        11 => fixed_precompile_gas(375, available), /* G1ADD */
        12 => bls_msm_gas(BLS_G1_DISCOUNT, 12000, 519, input_length / 160, available),
        13 => fixed_precompile_gas(600, available), /* G2ADD */
        14 => bls_msm_gas(BLS_G2_DISCOUNT, 22500, 524, input_length / 288, available),
        15 => linear_gas(37700, 32600, input_length / 384, available), /* PAIRING_CHECK */
        16 => fixed_precompile_gas(5500, available), /* MAP_FP_TO_G1 */
        17 => fixed_precompile_gas(23800, available), /* MAP_FP2_TO_G2 */
        256 => fixed_precompile_gas(6900, available), /* P256VERIFY */
        _ => Some(GAS_COST_ZERO),
    }
}

/*md
## The SSTORE cost and refund
*/
/*! The independent effects of one `SSTORE`: execution gas, the signed
    transaction refund, state gas charged, and state gas returned. */
struct SstoreCosts = {
    execution : gas_cost,
    refund : gas_refund,
    state_charge : gas_cost,
    state_credit : state_gas_spill,
}

/*! Returns the Amsterdam execution-gas cost of accessing a storage slot. */
function amsterdam_storage_access_cost(cold : bool) -> gas_constant =
    if cold then { G_amsterdam_cold_storage_access } else { G_warm_access }

/*! Legacy `SSTORE` pricing: a three-way comparison (EIP-2200) of
    *original* (the slot's transaction-start value), *current*, and *new*.
    Writing the same value, or dirtying an already-dirty slot, costs warm
    access only; a clean slot going zero↔nonzero pays `G_sset` /
    `G_sreset`. Refunds track clearing/un-clearing and restoring the original
    value. The EIP-2929 cold surcharge is added when the slot was not warm. */
function legacy_sstore_costs(original : word, current : word, new : word, cold : bool) -> SstoreCosts = {
    let cold_cost : gas_constant = if cold then { G_cold_sload } else { GAS_CONSTANT_ZERO };
    let clear_refund : gas_constant = sstore_clear_refund();
    let base : gas_constant = if current == new then {
            G_warm_access
        } else {
            if original == current then {
                if word_is_zero(original) then { G_sset } else { G_sreset }
            } else {
                G_warm_access
            }
        };
    let refund : gas_refund = if current == new then {
            0
        } else if original == current then {
            /* A clean non-zero slot earns the clear refund exactly once. */
            if not_bool(word_is_zero(original)) & word_is_zero(new) then {
                clear_refund
            } else {
                0
            }
        } else {
            /* A dirty slot first reverses or reapplies an earlier clear, then
             * adds the reset-to-original refund when applicable. Computing
             * the one transition delta directly avoids treating these
             * mutually related components as independent accumulators. */
            let clear_delta =
                if word_is_zero(original) then {
                    0
                } else {
                      (if word_is_zero(current) then { 0 - clear_refund } else { 0 })
                    + (if word_is_zero(new) then { clear_refund } else { 0 })
                };
            let reset_delta =
                if original == new then {
                    if word_is_zero(original) then {
                        G_sset - G_warm_access
                    } else {
                        G_sreset - G_warm_access
                    }
                } else {
                    0
                };
            clear_delta + reset_delta
        };
    struct { execution = base + cold_cost, refund = refund, state_charge = 0, state_credit = 0 }
}

/*! Amsterdam `SSTORE` pricing (EIP-8037). Access and first-write work debit
    execution gas. Introducing a new non-zero slot debits state gas; restoring
    that slot to its transaction-start zero value returns the state charge. */
function amsterdam_sstore_costs(original : word, current : word, new : word, cold : bool) -> SstoreCosts = {
    let changed = current != new;
    let clean_change = (original == current) & changed;
    let access = amsterdam_storage_access_cost(cold);
    let execution : gas_cost = if clean_change then {
            access + G_amsterdam_storage_write
        } else {
            access
        };
    let clear_delta = if
        changed & not_bool(word_is_zero(original)) & not_bool(word_is_zero(current)) & word_is_zero(new)
    then {
        R_amsterdam_storage_clear
    } else if changed & not_bool(word_is_zero(original)) & word_is_zero(current) then {
        0 - R_amsterdam_storage_clear
    } else {
        0
    };
    let restore_delta = if changed & (original == new) then {
            G_amsterdam_storage_write
        } else {
            0
        };
    let refund : gas_refund = clear_delta + restore_delta;
    let state_charge : gas_cost = if clean_change & word_is_zero(original) then {
            G_amsterdam_state_storage_set
        } else {
            GAS_COST_ZERO
        };
    var state_credit : state_gas_spill = 0;

    if changed & (original == new) then {
        if word_is_zero(original) then {
            state_credit = G_amsterdam_state_storage_set
        }
    };

    struct { execution = execution, refund = refund, state_charge = state_charge, state_credit = state_credit }
}

/*! Computes the fork-specific effects of one `SSTORE`. The refund delta is
    accumulated and capped at transaction settlement, not here. */
function sstore_costs(original : word, current : word, new : word, cold : bool) -> SstoreCosts = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if profile.fork >= Amsterdam then {
        amsterdam_sstore_costs(original, current, new, cold)
    } else {
        legacy_sstore_costs(original, current, new, cold)
    }
}

/* Per-word memory cost for an operand. Empty operands retain the opcode base. */
/*md
## The per-opcode costs

Operand-sized costs that ride on top of an opcode's static cost,
word-counted via `⌈byte_len / 32⌉` where the spec charges per 32-byte
word (YP Appendix G).
*/
/*! Charges a word-sized unit count only after proving the product affordable,
    so optimized builds never materialize an overflowing native gas cost. */
function charge_word_scaled_gas(per_unit : gas_constant, units : word) -> unit = {
    if not_bool(is_running()) | per_unit == 0 | units == 0 then {
        return ()
    };
    let remaining = gas_remaining;
    if units <= remaining then {
        let affordable_units : gas_cost = units;
        let exact_cost : linear_gas_variable_product = per_unit * affordable_units;
        if exact_cost <= remaining then {
            let cost : gas_cost = exact_cost;
            let _ = charge(cost);
            ()
        } else {
            exc_halt(OutOfGas)
        }
    } else {
        exc_halt(OutOfGas)
    }
}

/*! Charges an opcode base cost and its per-memory-word component. */
function charge_memory_word_gas(base : gas_constant, per_word : gas_constant, size : word) -> unit = {
    if not_bool(charge(base)) then { return () };
    charge_word_scaled_gas(per_word, memory_word_count_word(size))
}

/* KECCAK256: base + per-word hashing cost. */
function charge_keccak_gas(size : word) -> unit =
    charge_memory_word_gas(G_keccak, G_keccak_word, size)

/* *COPY (CALLDATA/CODE/RETURNDATA/EXTCODECOPY, MCOPY): per-word copy cost. */
function charge_copy_gas(size : word) -> unit =
    charge_memory_word_gas(GAS_CONSTANT_ZERO, G_copy_word, size)

/* LOG0..LOG4: base + per-topic + per-data-byte. */
/*! Charges the base, topic, and data-byte components of a log operation. */
function charge_log_gas(num_topics : log_topic_count, size : word) -> unit = {
    if not_bool(charge(G_log)) then { return () };
    let topic_cost : gas_cost = G_logtopic * num_topics;
    if not_bool(charge(topic_cost)) then { return () };
    charge_word_scaled_gas(G_logdata, size)
}

/*! `EXP`: base plus `G_expbyte` per significant exponent byte
    (EIP-160). */
function exp_gas(exponent : word) -> gas_cost = {
    G_expbyte * word_byte_length(exponent) + G_exp
}

/*! The EIP-3860 charge for transaction-backed initcode, retaining the SSZ
    transaction-envelope ceiling for native cost aggregation. */
function transaction_initcode_gas(byte_len : transaction_byte_length) -> transaction_initcode_cost = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if profile.fork >= Shanghai then {
        let words = memory_word_count(byte_len);
        words + words
    } else {
        0
    }
}

/*md
## The call-gas cap
*/
/*! Applies the EIP-150 forwarding cap to a word-sized gas request. */
function call_gas_cap_word(available : gas, requested : word) -> gas = {
    let retained : gas = available / 64;
    let all_but_64th = gas_sub_or_oog(available, retained);
    if requested < all_but_64th then { requested } else { all_but_64th }
}
