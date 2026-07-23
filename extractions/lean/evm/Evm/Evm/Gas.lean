import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Host.EvmByteSlice
import Evm.Host.Kernel.Environment
import Evm.Evm.Machine

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open Sail.ConcurrencyInterfaceV1

namespace Evm

open ConcurrencyInterfaceV1

open Defs
namespace Functions

open option
open exception
open ast
open TxType
open TrieUpdateSource
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open Register
open NodeRef
open MerkleSlot
open HaltKind
open FrameStatus
open FrameContinuation
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open ByteRegionResult
open BlockError

/-! # The gas schedule

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

`blob_base_fee = fake_exponential(MIN_BLOB_BASE_FEE, excess_blob_gas,
BLOB_BASE_FEE_UPDATE_FRACTION)`. The active target, maximum, and update
fraction come from the validated SSZ chain config, including BPO1/BPO2. -/

def MIN_BLOB_BASE_FEE : word := ⟨(WORD_ONE).value⟩

def GAS_PER_BLOB : Nat := (2 ^i 17)

def BLOB_RESERVE_PRICE_SHIFT_LIMIT : word := ⟨((U256 (2 ^i 252))).value⟩

def undefined_ScaledBlobValue (_ : Unit) : SailM ScaledBlobValue := do
  (pure { whole := ← do
              let publicField ← (undefined_range 0 ((2 ^i 256) - 1))
              pure (⟨publicField⟩),
          remainder := ← do
              let publicField ← (undefined_range 0 (11684671 - 1))
              pure (⟨publicField⟩) })

def undefined_BlobProductDivMod (_ : Unit) : SailM BlobProductDivMod := do
  (pure { quotient := ← do
              let publicField ← (undefined_range 0 ((2 ^i 256) - 1))
              pure (⟨publicField⟩),
          remainder := ← do
              let publicField ← (undefined_range 0 ((2 ^i 256) - 1))
              pure (⟨publicField⟩) })

/-- Advances the fake-exponential series index without exceeding `uint64`. -/
/- Type quantifiers: value : Nat, 1 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def fake_exponential_index_increment (value : fake_exponential_index) : SailM fake_exponential_index := do
  let value := (value).value
  let publicResult ← do
    if ((value <b ((2 ^i 64) - 1)) : Bool)
    then (pure (value + 1))
    else sailThrow ((InvalidBlock ExecutionInvalid))
  pure (⟨publicResult⟩)

/- Type quantifiers: left : Nat, right : Nat, 0 ≤ left ∧
  left < (2 ^ 256) ∧ 0 ≤ right ∧ right < (2 ^ 256) -/
def blob_word_add (left : Nat) (right : Nat) : SailM word := do
  let publicResult ← do
    if ((word_ule right ((word_sub_word (WORD_ALL_ONES).value left)).value) : Bool)
    then (pure ((word_add_word left right)).value)
    else sailThrow ((InvalidBlock ExecutionInvalid))
  pure (⟨publicResult⟩)

/- Type quantifiers: left : Nat, right : Nat, 0 ≤ left ∧
  left < (2 ^ 256) ∧ 0 ≤ right ∧ right < (2 ^ 256) -/
def blob_word_mul (left : Nat) (right : Nat) : SailM word := do
  let publicResult ← do
    if ((word_is_zero left) : Bool)
    then (pure (WORD_ZERO).value)
    else
      (do
        if ((word_ule right ((word_div_word (WORD_ALL_ONES).value left)).value) : Bool)
        then (pure (word_mul_word left right))
        else sailThrow ((InvalidBlock ExecutionInvalid)))
  pure (⟨publicResult⟩)

/-- Adds two exact denominator-scaled blob-fee values. -/
/- Type quantifiers: k_ex410624_ : Nat, 1 ≤ k_ex410624_ ∧ k_ex410624_ ≤ 11684671 -/
def scaled_blob_add (left : ScaledBlobValue) (right : ScaledBlobValue) (denominator : blob_fee_update_fraction) : SailM ScaledBlobValue := do
  let denominator := (denominator).value
  let combined := ((left.remainder).value + (right.remainder).value)
  let carry := (denominator ≤b combined)
  let remainder_value :=
    if (carry : Bool)
    then (combined - denominator)
    else combined
  if ((remainder_value <b denominator) : Bool)
  then
    (do
      let remainder : Nat := remainder_value
      let whole ← do
        (do
            let publicResult ← (blob_word_add (left.whole).value (right.whole).value)
            pure ((publicResult).value))
      let carried_whole ← do
        if (carry : Bool)
        then
          (do
              let publicResult ← (blob_word_add whole (WORD_ONE).value)
              pure ((publicResult).value))
        else (pure whole)
      (pure { whole := ⟨carried_whole⟩,
              remainder := ⟨remainder⟩ }))
  else sailThrow ((InvalidBlock ExecutionInvalid))

/-- Divides an exact blob-fee product while detecting word overflow. -/
/- Type quantifiers: k_ex410629_ : Nat, k_ex410628_ : Nat, k_ex410627_ : Nat, k_ex410626_ : Nat, k_ex410625_
  : Nat, 0 ≤ k_ex410625_ ∧ k_ex410625_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex410626_ ∧
  k_ex410626_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex410627_ ∧ k_ex410627_ ≤ (2 ^ 256 - 1), 1 ≤
  k_ex410628_ ∧ k_ex410628_ ≤ 11684671, 1 ≤ k_ex410629_ ∧ k_ex410629_ ≤ (2 ^ 64 - 1) -/
def blob_product_divmod (value : word) (factor : excess_blob_gas) (addend : word) (denominator : blob_fee_update_fraction) (iteration : fake_exponential_index) : SailM BlobProductDivMod := do
  let value := (value).value
  let factor := (factor).value
  let addend := (addend).value
  let denominator := (denominator).value
  let iteration := (iteration).value
  let divisor := (word_mul_word denominator iteration)
  if ((word_is_zero divisor) : Bool)
  then sailThrow ((InvalidBlock ExecutionInvalid))
  else (pure ())
  let value_quotient := ((word_div_word value divisor)).value
  let value_remainder := ((word_mod_word value divisor)).value
  let whole ← do
    (do
        let publicResult ← (blob_word_mul value_quotient factor)
        pure ((publicResult).value))
  let residual_product := (word_mul_word value_remainder factor)
  let residual_sum ← do
    (do
        let publicResult ← (blob_word_add residual_product addend)
        pure ((publicResult).value))
  let residual_quotient := ((word_div_word residual_sum divisor)).value
  (pure { quotient := ← do
              let publicField ← (do
                  let publicResult ← (blob_word_add whole residual_quotient)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          remainder := ⟨((word_mod_word residual_sum divisor)).value⟩ })

/-- Computes the next exact denominator-scaled Taylor term. -/
/- Type quantifiers: k_ex410632_ : Nat, k_ex410631_ : Nat, k_ex410630_ : Nat, 0 ≤ k_ex410630_ ∧
  k_ex410630_ ≤ (2 ^ 64 - 1), 1 ≤ k_ex410631_ ∧ k_ex410631_ ≤ 11684671, 1 ≤ k_ex410632_
  ∧ k_ex410632_ ≤ (2 ^ 64 - 1) -/
def scaled_blob_next (term : ScaledBlobValue) (numerator : excess_blob_gas) (denominator : blob_fee_update_fraction) (iteration : fake_exponential_index) : SailM ScaledBlobValue := do
  let numerator := (numerator).value
  let denominator := (denominator).value
  let iteration := (iteration).value
  let remainder_product := (word_mul_word (term.remainder).value numerator)
  let addend := ((word_div_word remainder_product denominator)).value
  let divided ← do
    (blob_product_divmod ⟨(term.whole).value⟩ ⟨numerator⟩ ⟨addend⟩ ⟨denominator⟩
      ⟨iteration⟩)
  let remainder_word := ((word_div_word (divided.remainder).value iteration)).value
  if ((remainder_word <b denominator) : Bool)
  then
    (let remainder : Nat := remainder_word
    (pure { whole := ⟨(divided.quotient).value⟩,
            remainder := ⟨remainder⟩ }))
  else sailThrow ((InvalidBlock ExecutionInvalid))

/-- The EIP-4844 `fake_exponential`: an integer approximation of
`factor · e^(numerator/denominator)` by Taylor expansion. -/
/- Type quantifiers: k_ex410635_ : Nat, k_ex410634_ : Nat, k_ex410633_ : Nat, 0 ≤ k_ex410633_ ∧
  k_ex410633_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex410634_ ∧ k_ex410634_ ≤ (2 ^ 64 - 1), 1 ≤
  k_ex410635_ ∧ k_ex410635_ ≤ 11684671 -/
def fake_exponential (factor : word) (numerator : excess_blob_gas) (denominator : blob_fee_update_fraction) : SailM word := do
  let factor := (factor).value
  let numerator := (numerator).value
  let denominator := (denominator).value
  let publicResult ← do
    let i : Nat := 1
    let output : ScaledBlobValue :=
      { whole := ⟨(WORD_ZERO).value⟩,
        remainder := ⟨0⟩ }
    let term : ScaledBlobValue :=
      { whole := ⟨factor⟩,
        remainder := ⟨0⟩ }
    let (i, output, term) ← (( do
      let loop_vars ← whileFuelM (fuel :=((2 ^i 64) - i)) (fun (i, output, term) => (pure ((word_nonzero
            (term.whole).value) || (((term.remainder).value != 0) : Bool)))) (i, output, term)
        fun (i, output, term) => do
          assert true "loop dummy assert"
          let output ← (scaled_blob_add output term ⟨denominator⟩)
          let term ← (scaled_blob_next term ⟨numerator⟩ ⟨denominator⟩ ⟨i⟩)
          let i ←
            (do
                let publicResult ← (fake_exponential_index_increment ⟨i⟩)
                pure ((publicResult).value))
          (pure (i, output, term))
      (pure loop_vars) ) : SailM (Nat × ScaledBlobValue × ScaledBlobValue) )
    (pure (output.whole).value)
  pure (⟨publicResult⟩)

/-- The blob base fee for a given excess blob gas. -/
/- Type quantifiers: excess_blob_gas : Nat, 0 ≤ excess_blob_gas ∧
  excess_blob_gas ≤ (2 ^ 64 - 1) -/
def blob_base_fee (excess_blob_gas : excess_blob_gas) : SailM word := do
  let excess_blob_gas := (excess_blob_gas).value
  let publicResult ← do
    (do
        let publicResult ← (fake_exponential ⟨(MIN_BLOB_BASE_FEE).value⟩ ⟨excess_blob_gas⟩
        ⟨((← readReg k_blob_schedule).base_fee_update_fraction).value⟩)
        pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Tests the EIP-7918 reserve-price condition without overflowing a word. -/
/- Type quantifiers: k_ex410638_ : Nat, k_ex410637_ : Nat, 0 ≤ k_ex410637_ ∧
  k_ex410637_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex410638_ ∧ k_ex410638_ ≤ (2 ^ 256 - 1) -/
def blob_reserve_price_active (base_fee : word) (price : word) : Bool :=
  let base_fee := (base_fee).value
  let price := (price).value
  if ((word_ult price (BLOB_RESERVE_PRICE_SHIFT_LIMIT).value) : Bool)
  then (word_ult ((word_shift_left ⟨price⟩ ⟨4⟩)).value base_fee)
  else false

def blob_schedule_target (_ : Unit) : SailM blob_target_count := do
  let publicResult ← do (pure ((← readReg k_blob_schedule).target).value)
  pure (⟨publicResult⟩)

def blob_schedule_max (_ : Unit) : SailM blob_count := do
  let publicResult ← do (pure ((← readReg k_blob_schedule).max).value)
  pure (⟨publicResult⟩)

/-- Converts a blob count to blob gas and rejects an invalid schedule. -/
/- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ 21 -/
def blob_gas_for_count (count : blob_count) : blob_gas_used :=
  let count := (count).value
  ⟨(GAS_PER_BLOB *i count)⟩

def blob_target_gas_per_block (_ : Unit) : SailM blob_gas_used := do
  let publicResult ← do
    (pure ((blob_gas_for_count ⟨((← (blob_schedule_target ()))).value⟩)).value)
  pure (⟨publicResult⟩)

def blob_max_gas_per_block (_ : Unit) : SailM blob_gas_used := do
  let publicResult ← do
    (pure ((blob_gas_for_count ⟨((← (blob_schedule_max ()))).value⟩)).value)
  pure (⟨publicResult⟩)

/-- Adds transaction blob gas to the block total, rejecting the active
schedule limit. -/
/- Type quantifiers: k_ex410641_ : Nat, k_ex410640_ : Nat, 0 ≤ k_ex410640_ ∧
  k_ex410640_ ≤ (21 * 2 ^ 17), 0 ≤ k_ex410641_ ∧ k_ex410641_ ≤ (9 * 2 ^ 17) -/
def checked_block_blob_gas_add (accumulated : blob_gas_used) (transaction : transaction_blob_gas) : SailM blob_gas_used := do
  let accumulated := (accumulated).value
  let transaction := (transaction).value
  let publicResult ← do
    let maximum ← do
      (do
          let publicResult ← (blob_max_gas_per_block ())
          pure ((publicResult).value))
    if ((accumulated ≤b maximum) : Bool)
    then
      (do
        if ((transaction ≤b (maximum - accumulated)) : Bool)
        then (pure (accumulated + transaction))
        else sailThrow ((InvalidBlock BlobGasLimitExceeded)))
    else sailThrow ((InvalidBlock BlobGasLimitExceeded))
  pure (⟨publicResult⟩)

/-- The header `excess_blob_gas` rule: decreases toward zero when the
parent underused blobs, otherwise accumulates; from Osaka, EIP-7918
substitutes the reserve-price form when the execution base fee
dominates. -/
/- Type quantifiers: k_ex410644_ : Nat, k_ex410643_ : Nat, k_ex410642_ : Nat, 0 ≤ k_ex410642_ ∧
  k_ex410642_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex410643_ ∧ k_ex410643_ ≤ (21 * 2 ^ 17), 0 ≤
  k_ex410644_ ∧ k_ex410644_ ≤ (2 ^ 256 - 1) -/
def next_excess_blob_gas (parent_excess_blob_gas : excess_blob_gas) (parent_blob_gas_used : blob_gas_used) (parent_base_fee_per_gas : word) : SailM excess_blob_gas := do
  let parent_excess_blob_gas := (parent_excess_blob_gas).value
  let parent_blob_gas_used := (parent_blob_gas_used).value
  let parent_base_fee_per_gas := (parent_base_fee_per_gas).value
  let publicResult ← do
    let parent_blob_gas := (parent_excess_blob_gas + parent_blob_gas_used)
    let target_blob_gas ← do
      (do
          let publicResult ← (blob_target_gas_per_block ())
          pure ((publicResult).value))
    if ((parent_blob_gas <b target_blob_gas) : Bool)
    then (pure 0)
    else
      (do
        let price ← do
          (do
              let publicResult ← (blob_base_fee ⟨parent_excess_blob_gas⟩)
              pure ((publicResult).value))
        if (((fork_gteq (← readReg k_fork) Osaka) && (blob_reserve_price_active
               ⟨parent_base_fee_per_gas⟩ ⟨price⟩)) : Bool)
        then
          (do
            let target ← do
              (do
                  let publicResult ← (blob_schedule_target ())
                  pure ((publicResult).value))
            let maximum ← do
              (do
                  let publicResult ← (blob_schedule_max ())
                  pure ((publicResult).value))
            if (((maximum == 0) || (maximum <b target)) : Bool)
            then sailThrow ((InvalidBlock InvalidConfig))
            else (pure ())
            let schedule_delta := (maximum - target)
            let scaled := (parent_blob_gas_used *i schedule_delta)
            let share := (Int.ediv scaled maximum)
            let next := (parent_excess_blob_gas + share)
            if ((next ≤b ((2 ^i 64) - 1)) : Bool)
            then (pure next)
            else sailThrow ((InvalidBlock InvalidConfig)))
        else
          (do
            let next := (parent_blob_gas - target_blob_gas)
            if ((next ≤b ((2 ^i 64) - 1)) : Bool)
            then (pure next)
            else sailThrow ((InvalidBlock InvalidConfig))))
  pure (⟨publicResult⟩)

def G_zero : gas_constant := ⟨0⟩

def G_jumpdest : gas_constant := ⟨1⟩

def G_base : gas_constant := ⟨2⟩

def G_verylow : gas_constant := ⟨3⟩

def G_low : gas_constant := ⟨5⟩

def G_mid : gas_constant := ⟨8⟩

def G_high : gas_constant := ⟨10⟩

def G_warm_access : gas_constant := ⟨100⟩

def G_cold_sload : gas_constant := ⟨2100⟩

def G_cold_account : gas_constant := ⟨2600⟩

def G_keccak : gas_constant := ⟨30⟩

def G_keccak_word : Nat := 6

def G_copy_word : gas_constant := ⟨3⟩

def G_memory : Nat := 3

def G_log : gas_constant := ⟨375⟩

def G_logtopic : gas_constant := ⟨375⟩

def G_logdata : gas_constant := ⟨8⟩

def G_exp : gas_constant := ⟨10⟩

def G_expbyte : gas_constant := ⟨50⟩

def G_sset : gas_constant := ⟨20000⟩

def G_sreset : gas_constant := ⟨2900⟩

def R_sclear_pre_london : gas_constant := ⟨15000⟩

def R_sclear_london : gas_constant := ⟨4800⟩

def R_selfdestruct_pre_london : gas_constant := ⟨24000⟩

def G_create : gas_constant := ⟨32000⟩

def G_codedeposit : Nat := 200

def G_callvalue : gas_constant := ⟨9000⟩

def G_callstipend : gas := 2300

def G_newaccount : gas_constant := ⟨25000⟩

def G_selfdestruct : gas_constant := ⟨5000⟩

def G_initcode_word : Nat := 2

def G_amsterdam_cold_account_access : gas_constant := ⟨3000⟩

def G_amsterdam_cold_storage_access : gas_constant := ⟨3000⟩

def G_amsterdam_storage_write : gas_constant := ⟨10000⟩

def G_amsterdam_account_write : gas_constant := ⟨8000⟩

def G_amsterdam_call_value : gas_constant := ⟨10300⟩

def G_amsterdam_create_access : gas_constant := ⟨11000⟩

def G_amsterdam_state_byte : gas_constant := ⟨1530⟩

def G_amsterdam_state_storage_set : state_gas_spill := ⟨97920⟩

def G_amsterdam_state_new_account : state_gas_spill := ⟨183600⟩

def G_amsterdam_state_auth_base : state_gas_spill := ⟨35190⟩

def R_amsterdam_storage_clear : gas_constant := ⟨12480⟩

def G_sstore_sentry : gas_cost := 2301

/-- EIP-170's deployed-code limit, active from Spurious Dragon. Byzantium is
the first post-Spurious-Dragon fork represented by this model. -/
def EIP170_DEPLOYED_CODE_SIZE_LIMIT : Nat := 24576

/-- EIP-3860's initcode limit, active from Shanghai. -/
def EIP3860_INITCODE_SIZE_LIMIT : Nat := 49152

/-- EIP-7954's Amsterdam deployed-code limit. -/
def EIP7954_DEPLOYED_CODE_SIZE_LIMIT : Nat := 65536

/-- EIP-7954's Amsterdam initcode limit. -/
def EIP7954_INITCODE_SIZE_LIMIT : Nat := 131072

/- Type quantifiers: size : Nat, 0 ≤ size -/
def deployed_code_size_allowed (size : Nat) : SailM Bool := do
  if ((fork_lt (← readReg k_fork) Byzantium) : Bool)
  then (pure true)
  else
    (do
      if ((fork_lt (← readReg k_fork) Amsterdam) : Bool)
      then (pure (size ≤b EIP170_DEPLOYED_CODE_SIZE_LIMIT))
      else (pure (size ≤b EIP7954_DEPLOYED_CODE_SIZE_LIMIT)))

/- Type quantifiers: size : Nat, 0 ≤ size -/
def initcode_size_allowed (size : Nat) : SailM Bool := do
  if ((fork_lt (← readReg k_fork) Shanghai) : Bool)
  then (pure true)
  else
    (do
      if ((fork_lt (← readReg k_fork) Amsterdam) : Bool)
      then (pure (size ≤b EIP3860_INITCODE_SIZE_LIMIT))
      else (pure (size ≤b EIP7954_INITCODE_SIZE_LIMIT)))

/-- The `SSTORE`-clears refund: 4800 from London (EIP-3529), 15000
before. -/
def sstore_clear_refund (_ : Unit) : SailM gas_constant := do
  let publicResult ← do
    if ((fork_gteq (← readReg k_fork) London) : Bool)
    then (pure (R_sclear_london).value)
    else (pure (R_sclear_pre_london).value)
  pure (⟨publicResult⟩)

/- Type quantifiers: amount : Nat, 0 ≤ amount -/
def charge (amount : Nat) : SailM Unit := do
  if (((! (← (is_running ()))) || (amount == 0)) : Bool)
  then (pure ())
  else
    (do
      let remaining ← do pure ((← readReg gas_remaining))
      if ((amount ≤b remaining) : Bool)
      then writeReg gas_remaining (remaining - amount)
      else (exc_halt OutOfGas))

/- Type quantifiers: amount : Nat, 0 ≤ amount -/
def check_execution_gas (amount : Nat) : SailM Unit := do
  let remaining ← do pure ((← readReg gas_remaining))
  if (((← (is_running ())) && (remaining <b amount)) : Bool)
  then (exc_halt OutOfGas)
  else (pure ())

/- Type quantifiers: left : Nat, 0 ≤ left ∧ left ≤ transaction_execution_gas_limit_value -/
def state_gas_spill_room (left : Nat) : Nat :=
  ((2 ^i 24) - left)

/-- Adds a nonnegative spill amount or rejects a value beyond the transaction cap. -/
/- Type quantifiers: left : Nat, right : Nat, 0 ≤ right, 0 ≤ left ∧ left ≤ (2 ^ 24) -/
def state_gas_spill_add (left : state_gas_spill) (right : Nat) : SailM state_gas_spill := do
  let left := (left).value
  let publicResult ← do
    if ((right ≤b (state_gas_spill_room left)) : Bool)
    then (pure (left + right))
    else sailThrow ((InvalidBlock ExecutionInvalid))
  pure (⟨publicResult⟩)

/- Type quantifiers: amount : Nat, 0 ≤ amount -/
def debit_state_gas (amount : Nat) : SailM Bool := do
  if ((amount == 0) : Bool)
  then (pure true)
  else
    (do
      let state_left ← do pure ((← readReg state_gas_remaining))
      let execution_left ← do pure ((← readReg gas_remaining))
      if ((amount ≤b state_left) : Bool)
      then
        (do
          writeReg state_gas_remaining (state_left - amount)
          (pure true))
      else
        (do
          let remainder := (amount - state_left)
          if ((remainder ≤b execution_left) : Bool)
          then
            (do
              let spilled ← do pure (((← readReg state_gas_spilled)).value)
              writeReg state_gas_remaining GAS_ZERO
              writeReg gas_remaining (execution_left - remainder)
              writeReg state_gas_spilled ⟨((← (state_gas_spill_add ⟨spilled⟩ remainder))).value⟩
              (pure true))
          else (pure false)))

/- Type quantifiers: amount : Nat, 0 ≤ amount -/
def charge_state_gas (amount : Nat) : SailM Unit := do
  if (((← (is_running ())) && (! (← (debit_state_gas amount)))) : Bool)
  then (exc_halt OutOfGas)
  else (pure ())

/- Type quantifiers: amount : Nat, 0 ≤ amount -/
def charge_deployment_state_gas (amount : Nat) : SailM Unit := do
  if ((! (← (debit_state_gas amount))) : Bool)
  then (exc_halt OutOfGas)
  else (pure ())

/-- Credits a state-gas charge in exact reverse order: execution spill first,
then the state reservoir. -/
/- Type quantifiers: amount : Nat, 0 ≤ amount ∧ amount ≤ (2 ^ 24) -/
def credit_state_gas_refund (amount : state_gas_spill) : SailM Unit := do
  let amount := (amount).value
  let spilled ← do pure (((← readReg state_gas_spilled)).value)
  if ((amount ≤b spilled) : Bool)
  then
    (do
      if ((amount != 0) : Bool)
      then
        (do
          writeReg gas_remaining (conserved_gas_add (← readReg gas_remaining) amount)
          writeReg state_gas_spilled ⟨(spilled - amount)⟩)
      else (pure ()))
  else
    (do
      if ((spilled != 0) : Bool)
      then
        (do
          writeReg gas_remaining (conserved_gas_add (← readReg gas_remaining) spilled)
          writeReg state_gas_spilled ⟨(STATE_GAS_SPILL_ZERO).value⟩)
      else (pure ())
      let to_state : Nat := (amount - spilled)
      writeReg state_gas_remaining (conserved_gas_add (← readReg state_gas_remaining) to_state))

/-- Returns a completed child's remaining state reservoir to its parent and
carries forward any state gas that the child drew from execution gas. -/
/- Type quantifiers: k_ex410726_ : Nat, k_ex410725_ : Nat, 0 ≤ k_ex410725_, 0 ≤ k_ex410726_ ∧
  k_ex410726_ ≤ (2 ^ 24) -/
def return_child_state_gas (child_remaining : gas) (child_spilled : state_gas_spill) : SailM Unit := do
  let child_spilled := (child_spilled).value
  writeReg state_gas_remaining (conserved_gas_add (← readReg state_gas_remaining) child_remaining)
  writeReg state_gas_spilled ⟨((← (state_gas_spill_add
    ⟨((← readReg state_gas_spilled)).value⟩ child_spilled))).value⟩

/-- Subtracts an exact cost from gas or records an out-of-gas halt. -/
/- Type quantifiers: k_ex410728_ : Nat, k_ex410727_ : Nat, 0 ≤ k_ex410727_, 0 ≤ k_ex410728_ -/
def gas_sub_or_oog (left : gas) (right : gas_cost) : SailM gas := do
  if ((right ≤b left) : Bool)
  then (pure (left - right))
  else
    (do
      (exc_halt OutOfGas)
      (pure GAS_ZERO))

/-- Returns unused child gas to the parent frame. -/
/- Type quantifiers: amount : Nat, 0 ≤ amount -/
def refund_gas (amount : gas) : SailM Unit := do
  writeReg gas_remaining (conserved_gas_add (← readReg gas_remaining) amount)

/- Type quantifiers: byte_len : Nat, 0 ≤ byte_len -/
def memory_word_count (byte_len : Nat) : Nat :=
  (Nat.div (byte_len + 31) 32)

/-- Returns the number of words covering a word-sized byte length without
forming `byte_len + 31`, whose mathematical intermediate can require 257
bits even though the final quotient remains an EVM word. -/
/- Type quantifiers: byte_len : Nat, 0 ≤ byte_len ∧ byte_len ≤ (2 ^ 256 - 1) -/
def memory_word_count_word (byte_len : word) : word :=
  let byte_len := (byte_len).value
  ⟨let quotient := ((word_div_word byte_len ((U256 32)).value)).value
  if ((((word_mod_word byte_len ((U256 32)).value)).value == (WORD_ZERO).value) : Bool)
  then quotient
  else ((word_add_word quotient (WORD_ONE).value)).value⟩

/- Type quantifiers: words : Nat, 0 ≤ words -/
def mem_cost (words : Nat) : Nat :=
  ((G_memory *i words) + (Int.ediv (words *i words) 512))

/- Type quantifiers: words : Nat, 0 ≤ words ∧ words ≤ block_gas_limit_bound -/
def bounded_mem_cost (words : Nat) : Nat :=
  ((G_memory *i words) + (Int.ediv (words *i words) 512))

/- Type quantifiers: start : Nat, size : Nat, 0 ≤ size ∧ size < (2 ^ 256), 0 ≤ start ∧
  start ≤ (2 ^ 256 - 1) -/
def memory_required_size (start : word) (size : Nat) : Nat :=
  let start := (start).value
  if ((size == 0) : Bool)
  then 0
  else (start + size)

/- Type quantifiers: required_size : Nat, available : Nat, 0 ≤ required_size ∧
  live_gas_valid(available) -/
def memory_expansion_cost (required_size : Nat) (available : Nat) : SailM Nat := do
  let new_words := (memory_word_count required_size)
  let old_words ← do (pure (memory_word_count (← (evm_memory_high_water ()))))
  if ((new_words ≤b old_words) : Bool)
  then (pure 0)
  else
    (do
      let old_cost := (mem_cost old_words)
      let new_cost := (mem_cost new_words)
      if (((old_cost ≤b new_cost) && (new_cost ≤b (old_cost + available))) : Bool)
      then
        (let cost : Nat := (new_cost - old_cost)
        (pure cost))
      else
        (do
          (exc_halt OutOfGas)
          (pure 0)))

/- Type quantifiers: start : Nat, size : Nat, available : Nat, 0 ≤ size ∧
  size < (2 ^ 256) ∧ live_gas_valid(available), 0 ≤ start ∧ start ≤ (2 ^ 256 - 1) -/
def memory_expansion (start : word) (size : Nat) (available : Nat) : SailM (MemoryExpansion available) := do
  let start := (start).value
  let required_size := (memory_required_size ⟨start⟩ size)
  let ⟨_, ⟨_, range⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len))) :=
    if ((size == 0) : Bool)
    then
      ((⟨_, ⟨_, EMPTY_MEMORY_RANGE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len))))
    else
      ((⟨_, ⟨_, (memory_range start size)⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len))))
  (pure { range := ⟨_, ⟨_, range⟩⟩,
          required_size := required_size,
          cost := ← (memory_expansion_cost required_size available) })

/- Type quantifiers: k_ex410760_ : Nat, k_ex410759_ : Nat, k_ex410758_ : Nat, k_ex410757_ : Nat, available
  : Nat, live_gas_valid(available), 0 ≤ k_ex410757_ ∧ k_ex410757_ ≤ (2 ^ 256 - 1), 0 ≤
  k_ex410758_ ∧ k_ex410758_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex410759_ ∧ k_ex410759_ ≤ (2 ^ 256 - 1), 0
  ≤ k_ex410760_ ∧ k_ex410760_ ≤ (2 ^ 256 - 1) -/
def memory_pair_expansion (left_start : word) (left_size : word) (right_start : word) (right_size : word) (available : Nat) : SailM (MemoryPairExpansion available) := do
  let left_start := (left_start).value
  let left_size := (left_size).value
  let right_start := (right_start).value
  let right_size := (right_size).value
  let left_required := (memory_required_size ⟨left_start⟩ left_size)
  let right_required := (memory_required_size ⟨right_start⟩ right_size)
  let required_size :=
    if ((left_required <b right_required) : Bool)
    then right_required
    else left_required
  let ⟨_, ⟨_, left⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len))) :=
    if ((left_size == 0) : Bool)
    then
      ((⟨_, ⟨_, EMPTY_MEMORY_RANGE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len))))
    else
      ((⟨_, ⟨_, (memory_range left_start left_size)⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len))))
  let ⟨_, ⟨_, right⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len))) :=
    if ((right_size == 0) : Bool)
    then
      ((⟨_, ⟨_, EMPTY_MEMORY_RANGE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len))))
    else
      ((⟨_, ⟨_, (memory_range right_start right_size)⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len))))
  (pure { left := ⟨_, ⟨_, left⟩⟩,
          right := ⟨_, ⟨_, right⟩⟩,
          required_size := required_size,
          cost := ← (memory_expansion_cost required_size available) })

/-- Materializes the exact byte high-water mark after gas has been charged. -/
/- Type quantifiers: required_size : Nat, 0 ≤ required_size -/
def expand_memory (required_size : memory_length) : SailM Unit := do
  let ⟨_, ⟨_, _⟩⟩ ← do (memory_expand_to required_size)
  (pure ())

/- Type quantifiers: k_available : Nat, live_gas_valid(k_available) -/
def apply_memory_expansion (plan : (MemoryExpansion k_available)) : SailM MemoryRange := do
  (expand_memory plan.required_size)
  (pure (plan.range : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len)))))

/- Type quantifiers: start : Nat, size : Nat, 0 ≤ size ∧ size < (2 ^ 256), 0 ≤ start ∧
  start ≤ (2 ^ 256 - 1) -/
def charge_memory_range (start : word) (size : Nat) : SailM MemoryRange := do
  let start := (start).value
  if ((← (is_running ())) : Bool)
  then
    (do
      let available ← do pure ((← readReg gas_remaining))
      let plan ← do (memory_expansion ⟨start⟩ size available)
      (charge plan.cost)
      if ((← (is_running ())) : Bool)
      then
        (do
          (apply_memory_expansion plan))
      else
        (pure ((⟨_, ⟨_, EMPTY_MEMORY_RANGE⟩⟩ : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len)))) : (Sigma fun (k_off : Nat)
          => (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len))))))
  else
    (pure ((⟨_, ⟨_, EMPTY_MEMORY_RANGE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len)))))

/- Type quantifiers: k_available : Nat, live_gas_valid(k_available) -/
def apply_memory_pair_expansion (plan : (MemoryPairExpansion k_available)) : SailM (MemoryRange × MemoryRange) := do
  (expand_memory plan.required_size)
  (pure ((plan.left, plan.right) : ((Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len))) × (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len))))))

/-- The account-access cost for a prior warm bit. -/
/- Type quantifiers: k_ex410777_ : Bool -/
def account_cost (warm : Bool) : SailM gas_constant := do
  let publicResult ← do
    if (warm : Bool)
    then (pure (G_warm_access).value)
    else
      (do
        if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
        then (pure (G_amsterdam_cold_account_access).value)
        else (pure (G_cold_account).value))
  pure (⟨publicResult⟩)

/-- The second database read performed by `EXTCODESIZE` and `EXTCODECOPY`.
EIP-8038 prices the code-store read as one warm access at Amsterdam. -/
def external_code_read_cost (_ : Unit) : SailM gas_constant := do
  let publicResult ← do
    if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
    then (pure (G_warm_access).value)
    else (pure (G_zero).value)
  pure (⟨publicResult⟩)

/-- The `SLOAD` cost for a prior warm bit (cold = 2100, EIP-2929). -/
/- Type quantifiers: k_ex410778_ : Bool -/
def sload_cost (warm : Bool) : SailM gas_constant := do
  let publicResult ← do
    if (warm : Bool)
    then (pure (G_warm_access).value)
    else
      (do
        if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
        then (pure (G_amsterdam_cold_storage_access).value)
        else (pure (G_cold_sload).value))
  pure (⟨publicResult⟩)

/-- The execution-gas component of a value-bearing CALL/CALLCODE. Amsterdam
reprices the account write while retaining the 2300 child stipend. -/
def call_value_cost (_ : Unit) : SailM gas_constant := do
  let publicResult ← do
    if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
    then (pure (G_amsterdam_call_value).value)
    else (pure (G_callvalue).value)
  pure (⟨publicResult⟩)

/-- The CREATE/CREATE2 execution-access charge. Persistent account growth is
charged separately as state gas at Amsterdam. -/
def create_access_cost (_ : Unit) : SailM gas_constant := do
  let publicResult ← do
    if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
    then (pure (G_amsterdam_create_access).value)
    else (pure (G_create).value)
  pure (⟨publicResult⟩)

/-- Returns the affordable execution-gas charge after successful initcode.
Legacy forks charge per byte; Amsterdam charges the keccak word cost. The
affordability guard bounds the native product without imposing a protocol
code-size limit on Frontier or Homestead. -/
/- Type quantifiers: k_ex410780_ : Nat, k_ex410779_ : Nat, 0 ≤ k_ex410779_, 0 ≤ k_ex410780_ -/
def code_deployment_execution_cost (byte_len : code_length) (available : gas) : SailM (Option gas_cost) := do
  if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
  then
    (if ((byte_len ≤b EIP7954_DEPLOYED_CODE_SIZE_LIMIT) : Bool)
    then
      (let words := (memory_word_count byte_len)
      if ((words ≤b (Int.ediv available G_keccak_word)) : Bool)
      then
        (let cost : Nat := (G_keccak_word *i words)
        (pure (some cost)))
      else (pure none))
    else (pure none))
  else
    (if ((byte_len ≤b (Int.ediv available G_codedeposit)) : Bool)
    then
      (let cost : Nat := (G_codedeposit *i byte_len)
      (pure (some cost)))
    else (pure none))

/-- Amsterdam state gas charged for each byte of newly deployed code. -/
/- Type quantifiers: byte_len : Nat, 0 ≤ byte_len -/
def code_deployment_state_cost (byte_len : code_length) : SailM gas_cost := do
  if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
  then
    (do
      if ((byte_len ≤b EIP7954_DEPLOYED_CODE_SIZE_LIMIT) : Bool)
      then (pure ((G_amsterdam_state_byte).value *i byte_len))
      else sailThrow ((InvalidBlock ExecutionInvalid)))
  else (pure GAS_COST_ZERO)

/- Type quantifiers: k_ex410788_ : Nat, k_ex410787_ : Nat, start : Nat, byte_count : Nat, source_valid_length(start)
  ∧ 0 ≤ byte_count ∧ byte_count ≤ 32, 0 ≤ k_ex410787_ ∧ 0 ≤ k_ex410788_ -/
def pc_word (input : EvmByteSlice) (start : Nat) (byte_count : Nat) : SailM word := do
  let input := ((input).2).2
  let publicResult ← do
    let value : Nat := (ZERO_WORD).value
    let start_offset := start
    let count := byte_count
    let input_length := input.len
    let loop_byte_index_lower := 0
    let loop_byte_index_upper := 31
    let mut loop_vars := value
    for byte_index in [loop_byte_index_lower:loop_byte_index_upper:1]i do
      let value := loop_vars
      loop_vars ← do
        let offset : Nat := byte_index
        if ((offset <b count) : Bool)
        then
          (do
            let next_byte ← do
              if ((start_offset <b input_length) : Bool)
              then
                (do
                  let remaining := (input_length - start_offset)
                  if ((offset <b remaining) : Bool)
                  then
                    (do
                      let cursor := (start_offset + offset)
                      (slice_byte ⟨_, ⟨_, input⟩⟩ cursor))
                  else (pure 0x00#8))
              else (pure 0x00#8)
            (pure ((word_add_word ((word_shift_left ⟨value⟩ ⟨8⟩)).value
                (BitVec.toNatInt next_byte))).value))
        else (pure value)
    (pure loop_vars)
  pure (⟨publicResult⟩)

/- Type quantifiers: declared_length : Nat, k_ex410797_ : Nat, k_ex410796_ : Nat, evm_prefix' : Nat, byte_count
  : Nat, source_valid_length(evm_prefix') ∧ 0 ≤ byte_count ∧ byte_count ≤ 32, 0 ≤ k_ex410796_
  ∧ 0 ≤ k_ex410797_, 0 ≤ declared_length ∧ declared_length ≤ (2 ^ 256 - 1) -/
def pc_word_after_declared_field (input : EvmByteSlice) (evm_prefix' : Nat) (declared_length : word) (byte_count : Nat) : SailM word := do
  let input := ((input).2).2
  let declared_length := (declared_length).value
  let publicResult ← do
    let input_length := input.len
    let prefix_offset := evm_prefix'
    if ((prefix_offset <b input_length) : Bool)
    then
      (do
        let suffix_length := (input_length - prefix_offset)
        if ((declared_length <b suffix_length) : Bool)
        then
          (do
            let field_length : Nat := declared_length
            let start : Nat := (prefix_offset + field_length)
            (do
                let publicResult ← (pc_word ⟨_, ⟨_, input⟩⟩ start byte_count)
                pure ((publicResult).value)))
        else (pure (ZERO_WORD).value))
    else (pure (ZERO_WORD).value)
  pure (⟨publicResult⟩)

/- Type quantifiers: k_ex410804_ : Nat, k_ex410803_ : Nat, 0 ≤ k_ex410803_ ∧ 0 ≤ k_ex410804_ -/
def pc_blake2_rounds (input : EvmByteSlice) : SailM blake2_rounds := do
  let input := ((input).2).2
  let publicResult ← do
    (pure (Nat.mod ((← (pc_word ⟨_, ⟨_, input⟩⟩ 0 4))).value (2 ^i 32)))
  pure (⟨publicResult⟩)

/-- `MODEXP` (precompile `0x05`) gas: EIP-2565 as amended by EIP-7883
(Osaka). Input layout: `bsize[32] esize[32] msize[32] base exp mod`.
Gas grows with the multiplication complexity of the larger of base
and modulus, and with an iteration count derived from the exponent's
bit length; EIP-7823 (Osaka) caps each field at 1024 bytes, modeled
as an unforwardable cost. Before Osaka the exact expression is below
`2^768`, but an EVM caller can observe only whether that expression fits
its available gas. The staged affordability checks below therefore keep
every materialized intermediate at most 256 bits in optimized builds. -/
/- Type quantifiers: k_ex410809_ : Nat, k_ex410808_ : Nat, k_ex410807_ : Nat, 0 ≤ k_ex410807_ ∧
  0 ≤ k_ex410808_, 0 ≤ k_ex410809_ -/
def modexp_gas (input : EvmByteSlice) (available : gas) : SailM (Option gas_cost) := SailME.run do
  let input := ((input).2).2
  let bl_word ← do
    (do
        let publicResult ← (pc_word ⟨_, ⟨_, input⟩⟩ 0 32)
        pure ((publicResult).value))
  let el_word ← do
    (do
        let publicResult ← (pc_word ⟨_, ⟨_, input⟩⟩ 32 32)
        pure ((publicResult).value))
  let ml_word ← do
    (do
        let publicResult ← (pc_word ⟨_, ⟨_, input⟩⟩ 64 32)
        pure ((publicResult).value))
  let bl : Nat := bl_word
  let el : Nat := el_word
  let ml : Nat := ml_word
  let osaka ← do (pure (fork_gteq (← readReg k_fork) Osaka))
  if ((osaka && ((bl >b 1024) || ((el >b 1024) || (ml >b 1024)))) : Bool)
  then (pure none)
  else
    (do
      let minimum : Nat :=
        if (osaka : Bool)
        then 500
        else 200
      if ((available <b minimum) : Bool)
      then (pure none)
      else
        (do
          if (((! osaka) && ((word_is_zero bl_word) && (word_is_zero ml_word))) : Bool)
          then (pure (some 200))
          else
            (do
              let maxlen :=
                if ((ml <b bl) : Bool)
                then bl
                else ml
              let whole_words := (Int.ediv maxlen 8)
              let words :=
                (whole_words + (if (((Nat.mod maxlen 8) == 0) : Bool)
                  then 0
                  else 1))
              let product_limit_value : Nat :=
                if (osaka : Bool)
                then available
                else ((3 *i available) + 2)
              let product_limit := product_limit_value
              let words_unaffordable := (product_limit <b words)
              let bounded_words : Nat :=
                if (words_unaffordable : Bool)
                then product_limit
                else words
              if (words_unaffordable : Bool)
              then (pure none)
              else
                (do
                  let words_squared : Nat := (bounded_words *i bounded_words)
                  let wide_product_limit : Nat := product_limit
                  let square_unaffordable := (wide_product_limit <b words_squared)
                  let affordable_square : Nat :=
                    if (square_unaffordable : Bool)
                    then product_limit
                    else words_squared
                  if (square_unaffordable : Bool)
                  then (pure none)
                  else
                    (do
                      let mult_value ← (( do
                        if ((osaka && (maxlen ≤b 32)) : Bool)
                        then (pure 16)
                        else
                          (do
                            if (osaka : Bool)
                            then
                              (do
                                let doubled_limit_value : Nat := (Int.ediv available 2)
                                let doubled_limit := doubled_limit_value
                                let wide_doubled_limit : Nat := doubled_limit
                                let doubled_unaffordable := (words_squared >b wide_doubled_limit)
                                let bounded_square : Nat :=
                                  if (doubled_unaffordable : Bool)
                                  then 0
                                  else words_squared
                                if (doubled_unaffordable : Bool)
                                then SailME.throw (none : (Option Nat))
                                else (pure (2 *i bounded_square)))
                            else (pure affordable_square)) ) : SailME (Option gas_cost) Nat )
                      let mult := mult_value
                      if ((product_limit <b mult) : Bool)
                      then (pure none)
                      else
                        (do
                          let iterations ← (( do
                            if ((el ≤b 32) : Bool)
                            then
                              (do
                                let exponent_bits ← do
                                  (pure ((word_bit_length
                                      ((← (pc_word_after_declared_field ⟨_, ⟨_, input⟩⟩ 96
                                        ⟨bl⟩ el))).value)).value)
                                if ((exponent_bits == 0) : Bool)
                                then (pure 1)
                                else
                                  (let count := (exponent_bits - 1)
                                  if ((count == 0) : Bool)
                                  then (pure 1)
                                  else (pure count)))
                            else
                              (do
                                let head_bits ← do
                                  (pure ((word_bit_length
                                      ((← (pc_word_after_declared_field ⟨_, ⟨_, input⟩⟩ 96
                                        ⟨bl⟩ 32))).value)).value)
                                let high_bits :=
                                  if ((head_bits != 0) : Bool)
                                  then (head_bits - 1)
                                  else 0
                                let extra := (el - 32)
                                let count ← (( do
                                  if (osaka : Bool)
                                  then
                                    (do
                                      let extra_limit : Nat := (Int.ediv available 16)
                                      let osaka_extra : Nat :=
                                        if ((word_greater_than_word extra ((U256 992)).value) : Bool)
                                        then 992
                                        else extra
                                      let extra_unaffordable := (osaka_extra >b extra_limit)
                                      let bounded_extra : Nat :=
                                        if (extra_unaffordable : Bool)
                                        then extra_limit
                                        else osaka_extra
                                      if (extra_unaffordable : Bool)
                                      then SailME.throw (none : (Option Nat))
                                      else (pure ((16 *i bounded_extra) + high_bits)))
                                  else
                                    (do
                                      let pre_osaka_limit := ((3 *i available) + 2)
                                      let extra_limit : Nat := (Int.ediv pre_osaka_limit 8)
                                      let extra_unaffordable := (extra >b extra_limit)
                                      let bounded_extra : Nat :=
                                        if (extra_unaffordable : Bool)
                                        then extra_limit
                                        else extra
                                      if (extra_unaffordable : Bool)
                                      then SailME.throw (none : (Option Nat))
                                      else (pure ((8 *i bounded_extra) + high_bits))) ) : SailME
                                  (Option gas_cost) Nat )
                                if ((count == 0) : Bool)
                                then (pure 1)
                                else (pure count)) ) : SailME (Option gas_cost) Nat )
                          if ((iterations == 0) : Bool)
                          then (pure none)
                          else
                            (let product : Nat := (mult *i iterations)
                            let product_unaffordable := (wide_product_limit <b product)
                            let affordable_product : Nat :=
                              if (product_unaffordable : Bool)
                              then product_limit
                              else product
                            if (product_unaffordable : Bool)
                            then (pure none)
                            else
                              (let calculated :=
                                if (osaka : Bool)
                                then affordable_product
                                else (Int.ediv affordable_product 3)
                              let cost :=
                                if ((calculated <b minimum) : Bool)
                                then minimum
                                else calculated
                              if ((cost ≤b available) : Bool)
                              then
                                (let affordable : Nat := cost
                                (pure (some affordable)))
                              else (pure none)))))))))

/-- The EIP-2537 BLS12-381 G1 MSM discount table: entry `k−1` is the
per-pair discount (in thousandths) applied to a `k`-pair MSM,
constant for `k ≥ 128`. -/
def BLS_G1_DISCOUNT : (Vector (BitVec 16) 128) :=
  #v[0x0207#16, 0x0208#16, 0x0208#16, 0x0209#16, 0x020A#16, 0x020A#16, 0x020B#16, 0x020C#16, 0x020D#16, 0x020D#16, 0x020E#16, 0x020F#16, 0x0210#16, 0x0210#16, 0x0211#16, 0x0212#16, 0x0213#16, 0x0214#16, 0x0214#16, 0x0215#16, 0x0216#16, 0x0217#16, 0x0218#16, 0x0218#16, 0x0219#16, 0x021A#16, 0x021B#16, 0x021C#16, 0x021C#16, 0x021D#16, 0x021E#16, 0x021F#16, 0x0220#16, 0x0221#16, 0x0222#16, 0x0223#16, 0x0223#16, 0x0224#16, 0x0225#16, 0x0226#16, 0x0227#16, 0x0228#16, 0x0229#16, 0x022A#16, 0x022B#16, 0x022C#16, 0x022D#16, 0x022E#16, 0x022F#16, 0x0230#16, 0x0231#16, 0x0232#16, 0x0233#16, 0x0234#16, 0x0235#16, 0x0236#16, 0x0237#16, 0x0238#16, 0x0239#16, 0x023A#16, 0x023C#16, 0x023D#16, 0x023E#16, 0x023F#16, 0x0240#16, 0x0241#16, 0x0243#16, 0x0244#16, 0x0245#16, 0x0246#16, 0x0248#16, 0x0249#16, 0x024A#16, 0x024C#16, 0x024D#16, 0x024F#16, 0x0250#16, 0x0251#16, 0x0253#16, 0x0254#16, 0x0256#16, 0x0257#16, 0x0259#16, 0x025B#16, 0x025C#16, 0x025E#16, 0x0260#16, 0x0261#16, 0x0263#16, 0x0265#16, 0x0267#16, 0x0269#16, 0x026B#16, 0x026D#16, 0x026F#16, 0x0271#16, 0x0273#16, 0x0276#16, 0x0278#16, 0x027B#16, 0x027D#16, 0x0280#16, 0x0282#16, 0x0285#16, 0x0288#16, 0x028B#16, 0x028E#16, 0x0292#16, 0x0295#16, 0x0299#16, 0x029D#16, 0x02A1#16, 0x02A5#16, 0x02AA#16, 0x02AF#16, 0x02B4#16, 0x02BA#16, 0x02C1#16, 0x02C8#16, 0x02CF#16, 0x02D8#16, 0x02E2#16, 0x02EE#16, 0x02FC#16, 0x031D#16, 0x0350#16, 0x03B5#16, 0x03E8#16]

def BLS_G2_DISCOUNT : (Vector (BitVec 16) 128) :=
  #v[0x020C#16, 0x020C#16, 0x020D#16, 0x020E#16, 0x020E#16, 0x020F#16, 0x0210#16, 0x0210#16, 0x0211#16, 0x0212#16, 0x0212#16, 0x0213#16, 0x0214#16, 0x0214#16, 0x0215#16, 0x0216#16, 0x0217#16, 0x0217#16, 0x0218#16, 0x0219#16, 0x0219#16, 0x021A#16, 0x021B#16, 0x021C#16, 0x021D#16, 0x021D#16, 0x021E#16, 0x021F#16, 0x0220#16, 0x0221#16, 0x0221#16, 0x0222#16, 0x0223#16, 0x0224#16, 0x0225#16, 0x0226#16, 0x0227#16, 0x0228#16, 0x0228#16, 0x0229#16, 0x022A#16, 0x022B#16, 0x022C#16, 0x022D#16, 0x022E#16, 0x022F#16, 0x0230#16, 0x0231#16, 0x0232#16, 0x0233#16, 0x0235#16, 0x0236#16, 0x0237#16, 0x0238#16, 0x0239#16, 0x023A#16, 0x023B#16, 0x023D#16, 0x023E#16, 0x023F#16, 0x0240#16, 0x0242#16, 0x0243#16, 0x0244#16, 0x0246#16, 0x0247#16, 0x0248#16, 0x024A#16, 0x024B#16, 0x024D#16, 0x024E#16, 0x0250#16, 0x0251#16, 0x0253#16, 0x0255#16, 0x0256#16, 0x0258#16, 0x025A#16, 0x025C#16, 0x025E#16, 0x025F#16, 0x0261#16, 0x0263#16, 0x0265#16, 0x0267#16, 0x026A#16, 0x026C#16, 0x026E#16, 0x0270#16, 0x0273#16, 0x0275#16, 0x0278#16, 0x027A#16, 0x027D#16, 0x0280#16, 0x0283#16, 0x0286#16, 0x0289#16, 0x028C#16, 0x028F#16, 0x0293#16, 0x0297#16, 0x029A#16, 0x029E#16, 0x02A2#16, 0x02A7#16, 0x02AB#16, 0x02B0#16, 0x02B5#16, 0x02BB#16, 0x02C0#16, 0x02C7#16, 0x02CD#16, 0x02D4#16, 0x02DC#16, 0x02E4#16, 0x02ED#16, 0x02F7#16, 0x0302#16, 0x030E#16, 0x031C#16, 0x032C#16, 0x0340#16, 0x0357#16, 0x0374#16, 0x039B#16, 0x03E8#16, 0x03E8#16]

/-- EIP-2537 MSM gas: `(k · base · discount(k)) / 1000`, with the
discount clamped to the `k = 128` entry beyond the table. -/
/- Type quantifiers: k_ex410826_ : Nat, k_ex410825_ : Nat, k_ex410824_ : Nat, k_ex410823_ : Nat, 0
  ≤ k_ex410823_ ∧ k_ex410823_ ≤ 45000, 0 ≤ k_ex410824_ ∧ k_ex410824_ ≤ (2 ^ 16 - 1), 0
  ≤ k_ex410825_, 0 ≤ k_ex410826_ -/
def bls_msm_gas (table : (Vector (BitVec 16) 128)) (base : gas_constant) (maxd : bls_discount) (k : source_length) (available : gas) : (Option gas_cost) :=
  let base := (base).value
  let maxd := (maxd).value
  if ((k == 0) : Bool)
  then (some GAS_COST_ZERO)
  else
    (let discount : Nat := maxd
    let discount : Nat :=
      if ((k <b 128) : Bool)
      then
        (let index : Nat := (128 - k)
        let discount : Nat := (BitVec.toNatInt (GetElem?.getElem! table index))
        discount)
      else discount
    let coefficient : Nat := (base *i discount)
    let whole := (Int.ediv k 1000)
    let remainder := (Nat.mod k 1000)
    let whole_product : Nat := (coefficient *i whole)
    if ((whole_product >b available) : Bool)
    then none
    else
      (let major : Nat := whole_product
      let tail_product : Nat := (coefficient *i remainder)
      let tail : Nat := (Int.ediv tail_product 1000)
      let total : Nat := (major + tail)
      if ((total ≤b available) : Bool)
      then
        (let affordable : Nat := total
        (some affordable))
      else none))

/-- Returns a linear precompile cost only after the caller can afford its
transaction-controlled multiplier. -/
/- Type quantifiers: k_ex410830_ : Nat, k_ex410829_ : Nat, k_ex410828_ : Nat, k_ex410827_ : Nat, 0
  ≤ k_ex410827_ ∧ k_ex410827_ ≤ 45000, 0 ≤ k_ex410828_ ∧ k_ex410828_ ≤ 45000, 0 ≤
  k_ex410829_, 0 ≤ k_ex410830_ -/
def linear_gas (base : gas_constant) (per_unit : gas_constant) (units : source_length) (available : gas) : (Option gas_cost) :=
  let base := (base).value
  let per_unit := (per_unit).value
  let variable_cost : Nat := (per_unit *i units)
  let exact_cost : Nat := (variable_cost + base)
  if ((exact_cost >b available) : Bool)
  then none
  else
    (let affordable : Nat := exact_cost
    (some affordable))

/- Type quantifiers: available : Nat, cost : Nat, 0 ≤ cost, 0 ≤ available -/
def fixed_precompile_gas (cost : Nat) (available : gas) : (Option gas_cost) :=
  if ((cost ≤b available) : Bool)
  then
    (let affordable : Nat := cost
    (some affordable))
  else none

/-- The gas of the precompile at address `num` for a given input. Gas is
protocol policy defined entirely here; implementations return only
output. Length-only costs
derive from the word count; the two input-dependent curves (`MODEXP`,
`BLAKE2F` rounds) read the input in place. The match arms are the
precompile catalog with their addresses and pricing EIPs. -/
/- Type quantifiers: k_ex410837_ : Nat, k_ex410836_ : Nat, k_ex410835_ : Nat, k_ex410832_ : Nat, 1
  ≤ k_ex410832_ ∧ k_ex410832_ ≤ 256, 0 ≤ k_ex410835_ ∧ 0 ≤ k_ex410836_, 0 ≤
  k_ex410837_ -/
def precompile_gas (num : precompile_id) (input : EvmByteSlice) (available : gas) : SailM (Option gas_cost) := do
  let num := (num).value
  let input := ((input).2).2
  let input_len := input.len
  let input_length := input_len
  let words := (memory_word_count input_len)
  match num with
  | 1 => (pure (fixed_precompile_gas 3000 available))
  | 2 => (pure (linear_gas ⟨60⟩ ⟨12⟩ words available))
  | 3 => (pure (linear_gas ⟨600⟩ ⟨120⟩ words available))
  | 4 => (pure (linear_gas ⟨15⟩ ⟨3⟩ words available))
  | 5 => (modexp_gas ⟨_, ⟨_, input⟩⟩ available)
  | 6 => (pure (fixed_precompile_gas 150 available))
  | 7 => (pure (fixed_precompile_gas 6000 available))
  | 8 => (pure (linear_gas ⟨45000⟩ ⟨34000⟩ (Int.ediv input_length 192) available))
  | 9 =>
    (pure (fixed_precompile_gas ((← (pc_blake2_rounds ⟨_, ⟨_, input⟩⟩))).value available))
  | 10 => (pure (fixed_precompile_gas 50000 available))
  | 11 => (pure (fixed_precompile_gas 375 available))
  | 12 =>
    (pure (bls_msm_gas BLS_G1_DISCOUNT ⟨12000⟩ ⟨519⟩ (Int.ediv input_length 160) available))
  | 13 => (pure (fixed_precompile_gas 600 available))
  | 14 =>
    (pure (bls_msm_gas BLS_G2_DISCOUNT ⟨22500⟩ ⟨524⟩ (Int.ediv input_length 288) available))
  | 15 => (pure (linear_gas ⟨37700⟩ ⟨32600⟩ (Int.ediv input_length 384) available))
  | 16 => (pure (fixed_precompile_gas 5500 available))
  | 17 => (pure (fixed_precompile_gas 23800 available))
  | 256 => (pure (fixed_precompile_gas 6900 available))
  | _ => (pure (some GAS_COST_ZERO))

def undefined_SstoreCosts (_ : Unit) : SailM SstoreCosts := do
  (pure { execution := ← (undefined_nat ()),
          refund := ← do
              let publicField ← (undefined_range (Neg.neg (199 *i ((2 ^i 64) - 1)))
                (199 *i ((2 ^i 64) - 1)))
              pure (⟨publicField⟩),
          state_charge := ← (undefined_nat ()),
          state_credit := ← do
              let publicField ← (undefined_range 0 (2 ^i 24))
              pure (⟨publicField⟩) })

/-- Returns the Amsterdam execution-gas cost of accessing a storage slot. -/
/- Type quantifiers: k_ex410838_ : Bool -/
def amsterdam_storage_access_cost (cold : Bool) : gas_constant :=
  ⟨if (cold : Bool)
  then (G_amsterdam_cold_storage_access).value
  else (G_warm_access).value⟩

/-- Legacy `SSTORE` pricing: a three-way comparison (EIP-2200) of
*original* (the slot's transaction-start value), *current*, and *new*.
Writing the same value, or dirtying an already-dirty slot, costs warm
access only; a clean slot going zero↔nonzero pays `G_sset` /
`G_sreset`. Refunds track clearing/un-clearing and restoring the original
value. The EIP-2929 cold surcharge is added when the slot was not warm. -/
/- Type quantifiers: k_ex410842_ : Bool, k_ex410841_ : Nat, k_ex410840_ : Nat, k_ex410839_ : Nat, 0
  ≤ k_ex410839_ ∧ k_ex410839_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex410840_ ∧
  k_ex410840_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex410841_ ∧ k_ex410841_ ≤ (2 ^ 256 - 1) -/
def legacy_sstore_costs (original : word) (current : word) (new : word) (cold : Bool) : SailM SstoreCosts := do
  let original := (original).value
  let current := (current).value
  let new := (new).value
  let cold_cost : Nat :=
    if (cold : Bool)
    then (G_cold_sload).value
    else (GAS_CONSTANT_ZERO).value
  let clear_refund ← (( do
    (do
        let publicResult ← (sstore_clear_refund ())
        pure ((publicResult).value)) ) : SailM Nat )
  let base : Nat :=
    if ((current == new) : Bool)
    then (G_warm_access).value
    else
      (if ((original == current) : Bool)
      then
        (if ((word_is_zero original) : Bool)
        then (G_sset).value
        else (G_sreset).value)
      else (G_warm_access).value)
  let refund : Int := (GAS_REFUND_ZERO).value
  let refund ← (( do
    if ((current != new) : Bool)
    then
      (do
        if ((original == current) : Bool)
        then
          (do
            if (((! (word_is_zero original)) && (word_is_zero new)) : Bool)
            then
              (do
                (do
                    let publicResult ← (validated_refund_add refund clear_refund)
                    pure ((publicResult).value)))
            else (pure refund))
        else
          (do
            let refund ← (( do
              if ((! (word_is_zero original)) : Bool)
              then
                (do
                  let refund ← (( do
                    if ((word_is_zero current) : Bool)
                    then
                      (do
                        (do
                            let publicResult ← (validated_refund_add refund
                            ((GAS_CONSTANT_ZERO).value -i clear_refund))
                            pure ((publicResult).value)))
                    else (pure refund) ) : SailM Int )
                  if ((word_is_zero new) : Bool)
                  then
                    (do
                      (do
                          let publicResult ← (validated_refund_add refund clear_refund)
                          pure ((publicResult).value)))
                  else (pure refund))
              else (pure refund) ) : SailM Int )
            if ((original == new) : Bool)
            then
              (do
                if ((word_is_zero original) : Bool)
                then
                  (do
                    (do
                        let publicResult ← (validated_refund_add refund
                        ((G_sset).value -i (G_warm_access).value))
                        pure ((publicResult).value)))
                else
                  (do
                    (do
                        let publicResult ← (validated_refund_add refund
                        ((G_sreset).value -i (G_warm_access).value))
                        pure ((publicResult).value))))
            else (pure refund)))
    else (pure refund) ) : SailM Int )
  (pure { execution := (base + cold_cost),
          refund := ⟨refund⟩,
          state_charge := 0,
          state_credit := ⟨0⟩ })

/-- Amsterdam `SSTORE` pricing (EIP-8037). Access and first-write work debit
execution gas. Introducing a new non-zero slot debits state gas; restoring
that slot to its transaction-start zero value returns the state charge. -/
/- Type quantifiers: k_ex410846_ : Bool, k_ex410845_ : Nat, k_ex410844_ : Nat, k_ex410843_ : Nat, 0
  ≤ k_ex410843_ ∧ k_ex410843_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex410844_ ∧
  k_ex410844_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex410845_ ∧ k_ex410845_ ≤ (2 ^ 256 - 1) -/
def amsterdam_sstore_costs (original : word) (current : word) (new : word) (cold : Bool) : SailM SstoreCosts := do
  let original := (original).value
  let current := (current).value
  let new := (new).value
  let changed := (current != new)
  let clean_change := ((original == current) && changed)
  let access := ((amsterdam_storage_access_cost cold)).value
  let execution : Nat :=
    if (clean_change : Bool)
    then (access + (G_amsterdam_storage_write).value)
    else access
  let refund : Int := (GAS_REFUND_ZERO).value
  let state_charge : Nat :=
    if ((clean_change && (word_is_zero original)) : Bool)
    then (G_amsterdam_state_storage_set).value
    else GAS_COST_ZERO
  let state_credit : Nat := 0
  let (refund, state_credit) ← (( do
    if (changed : Bool)
    then
      (do
        let refund ← (( do
          if (((! (word_is_zero original)) && ((! (word_is_zero current)) && (word_is_zero new))) : Bool)
          then
            (do
              (do
                  let publicResult ← (validated_refund_add refund
                  (R_amsterdam_storage_clear).value)
                  pure ((publicResult).value)))
          else (pure refund) ) : SailM Int )
        let refund ← (( do
          if (((! (word_is_zero original)) && (word_is_zero current)) : Bool)
          then
            (do
              (do
                  let publicResult ← (validated_refund_add refund
                  ((GAS_CONSTANT_ZERO).value -i (R_amsterdam_storage_clear).value))
                  pure ((publicResult).value)))
          else (pure refund) ) : SailM Int )
        let (refund, state_credit) ← (( do
          if ((original == new) : Bool)
          then
            (do
              let refund ←
                (do
                    let publicResult ← (validated_refund_add refund
                    (G_amsterdam_storage_write).value)
                    pure ((publicResult).value))
              let state_credit : Nat :=
                if ((word_is_zero original) : Bool)
                then
                  (let state_credit : Nat := (G_amsterdam_state_storage_set).value
                  state_credit)
                else state_credit
              (pure (refund, state_credit)))
          else (pure (refund, state_credit)) ) : SailM (Int × Nat) )
        (pure (refund, state_credit)))
    else (pure (refund, state_credit)) ) : SailM (Int × Nat) )
  (pure { execution := execution,
          refund := ⟨refund⟩,
          state_charge := state_charge,
          state_credit := ⟨state_credit⟩ })

/-- Computes the fork-specific effects of one `SSTORE`. The refund delta is
accumulated and capped at transaction settlement, not here. -/
/- Type quantifiers: k_ex410850_ : Bool, k_ex410849_ : Nat, k_ex410848_ : Nat, k_ex410847_ : Nat, 0
  ≤ k_ex410847_ ∧ k_ex410847_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex410848_ ∧
  k_ex410848_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex410849_ ∧ k_ex410849_ ≤ (2 ^ 256 - 1) -/
def sstore_costs (original : word) (current : word) (new : word) (cold : Bool) : SailM SstoreCosts := do
  let original := (original).value
  let current := (current).value
  let new := (new).value
  if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
  then (amsterdam_sstore_costs ⟨original⟩ ⟨current⟩ ⟨new⟩ cold)
  else (legacy_sstore_costs ⟨original⟩ ⟨current⟩ ⟨new⟩ cold)

/-- Charges a word-sized unit count only after proving the product affordable,
so optimized builds never materialize an overflowing native gas cost. -/
/- Type quantifiers: k_ex410852_ : Nat, k_ex410851_ : Nat, 0 ≤ k_ex410851_ ∧
  k_ex410851_ ≤ 45000, 0 ≤ k_ex410852_ ∧ k_ex410852_ ≤ (2 ^ 256 - 1) -/
def charge_word_scaled_gas (per_unit : gas_constant) (units : word) : SailM Unit := do
  let per_unit := (per_unit).value
  let units := (units).value
  if (((! (← (is_running ()))) || ((per_unit == 0) || (units == 0))) : Bool)
  then (pure ())
  else
    (do
      let remaining ← do pure ((← readReg gas_remaining))
      if ((units ≤b remaining) : Bool)
      then
        (do
          let affordable_units : Nat := units
          let exact_cost : Nat := (per_unit *i affordable_units)
          if ((exact_cost ≤b remaining) : Bool)
          then
            (do
              let cost : Nat := exact_cost
              (charge cost))
          else (exc_halt OutOfGas))
      else (exc_halt OutOfGas))

/-- Charges an opcode base cost and its per-memory-word component. -/
/- Type quantifiers: k_ex410855_ : Nat, k_ex410854_ : Nat, k_ex410853_ : Nat, 0 ≤ k_ex410853_ ∧
  k_ex410853_ ≤ 45000, 0 ≤ k_ex410854_ ∧ k_ex410854_ ≤ 45000, 0 ≤ k_ex410855_ ∧
  k_ex410855_ ≤ (2 ^ 256 - 1) -/
def charge_memory_word_gas (base : gas_constant) (per_word : gas_constant) (size : word) : SailM Unit := do
  let base := (base).value
  let per_word := (per_word).value
  let size := (size).value
  (charge base)
  (charge_word_scaled_gas ⟨per_word⟩ ⟨((memory_word_count_word ⟨size⟩)).value⟩)

/- Type quantifiers: size : Nat, 0 ≤ size ∧ size ≤ (2 ^ 256 - 1) -/
def charge_keccak_gas (size : word) : SailM Unit := do
  let size := (size).value
  (charge_memory_word_gas ⟨(G_keccak).value⟩ ⟨G_keccak_word⟩ ⟨size⟩)

/- Type quantifiers: size : Nat, 0 ≤ size ∧ size ≤ (2 ^ 256 - 1) -/
def charge_copy_gas (size : word) : SailM Unit := do
  let size := (size).value
  (charge_memory_word_gas ⟨(GAS_CONSTANT_ZERO).value⟩ ⟨(G_copy_word).value⟩ ⟨size⟩)

/-- Charges the base, topic, and data-byte components of a log operation. -/
/- Type quantifiers: k_ex410859_ : Nat, k_ex410858_ : Nat, 0 ≤ k_ex410858_ ∧ k_ex410858_ ≤ 4, 0
  ≤ k_ex410859_ ∧ k_ex410859_ ≤ (2 ^ 256 - 1) -/
def charge_log_gas (num_topics : log_topic_count) (size : word) : SailM Unit := do
  let num_topics := (num_topics).value
  let size := (size).value
  (charge (G_log).value)
  let topic_cost : Nat := ((G_logtopic).value *i num_topics)
  (charge topic_cost)
  (charge_word_scaled_gas ⟨(G_logdata).value⟩ ⟨size⟩)

/-- `EXP`: base plus `G_expbyte` per significant exponent byte
(EIP-160). -/
/- Type quantifiers: exponent : Nat, 0 ≤ exponent ∧ exponent ≤ (2 ^ 256 - 1) -/
def exp_gas (exponent : word) : gas_cost :=
  let exponent := (exponent).value
  (((G_expbyte).value *i (word_byte_length exponent)) + (G_exp).value)

/-- The EIP-3860 per-word initcode cost, charged by `CREATE`/`CREATE2`
and create transactions from Shanghai. -/
/- Type quantifiers: byte_len : Nat, 0 ≤ byte_len -/
def initcode_gas (byte_len : Nat) : SailM Nat := do
  if ((fork_gteq (← readReg k_fork) Shanghai) : Bool)
  then
    (let words := (memory_word_count byte_len)
    (pure (words + words)))
  else (pure 0)

/-- The EIP-3860 charge for transaction-backed initcode, retaining the SSZ
transaction-envelope ceiling for native cost aggregation. -/
/- Type quantifiers: byte_len : Nat, 0 ≤ byte_len ∧ byte_len ≤ (2 ^ 30) -/
def transaction_initcode_gas (byte_len : transaction_byte_length) : SailM transaction_initcode_cost := do
  let byte_len := (byte_len).value
  let publicResult ← do
    if ((fork_gteq (← readReg k_fork) Shanghai) : Bool)
    then
      (let words := (memory_word_count byte_len)
      (pure (words + words)))
    else (pure 0)
  pure (⟨publicResult⟩)

/-- Applies the EIP-150 forwarding cap to a word-sized gas request. -/
/- Type quantifiers: k_ex410864_ : Nat, k_ex410863_ : Nat, 0 ≤ k_ex410863_, 0 ≤ k_ex410864_ ∧
  k_ex410864_ ≤ (2 ^ 256 - 1) -/
def call_gas_cap_word (available : gas) (requested : word) : SailM gas := do
  let requested := (requested).value
  let retained : Nat := (Int.ediv available 64)
  let all_but_64th ← do (gas_sub_or_oog available retained)
  if ((requested <b all_but_64th) : Bool)
  then (pure requested)
  else (pure all_but_64th)

