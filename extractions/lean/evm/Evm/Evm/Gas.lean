import Evm.Flow
import Evm.Arith
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

open word
open option
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open b256
open ast
open address
open TxType
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open StateCheckpoint
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

def MIN_BLOB_BASE_FEE : word := WORD_ONE

def GAS_PER_BLOB : blob_gas := ⟨131072⟩

/-- Adds two scaled blob-fee values without losing their fractional parts. -/
/- Type quantifiers: k_ex161298_ : Nat, 0 ≤ k_ex161298_ ∧ k_ex161298_ ≤ (2 ^ 64 - 1) -/
def scaled_blob_add (left : ScaledBlobValue) (right : ScaledBlobValue) (denominator : blob_fee_update_fraction) : (Option ScaledBlobValue) := ExceptM.run do
  let denominator := (denominator).value
  if ((denominator == 0) : Bool)
  then (pure none)
  else
    (do
      let combined := ((left.remainder).value + (right.remainder).value)
      let carry := (denominator ≤b combined)
      let remainder :=
        if (carry : Bool)
        then (combined -i denominator)
        else combined
      if (((BYTE_QUANTITY_MAX).value <b remainder) : Bool)
      then (pure none)
      else
        (do
          let whole ← (( do
            match (word_checked_add left.whole right.whole) with
            | .some value => (pure value)
            | none => throw (none : (Option ScaledBlobValue)) ) : ExceptM (Option ScaledBlobValue)
            word )
          if (carry : Bool)
          then
            (match (word_checked_add whole WORD_ONE) with
            | .some value =>
              (pure (some
                  { whole := value,
                    remainder := ⟨remainder⟩ }))
            | none => (pure none))
          else
            (pure (some
                { whole := whole,
                  remainder := ⟨remainder⟩ }))))

/-- Divides an exact blob-fee product while detecting word overflow. -/
/- Type quantifiers: k_ex161302_ : Nat, k_ex161301_ : Nat, k_ex161300_ : Nat, k_ex161299_ : Nat, 0
  ≤ k_ex161299_ ∧ k_ex161299_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161300_ ∧
  k_ex161300_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161301_ ∧ k_ex161301_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex161302_ ∧ k_ex161302_ ≤ (2 ^ 64 - 1) -/
def blob_product_divmod (value : word) (factor : blob_gas) (addend : protocol_quantity) (denominator : blob_fee_update_fraction) (iteration : item_index) : SailM (Option WordDivMod) := SailME.run do
  let factor := (factor).value
  let addend := (addend).value
  let denominator := (denominator).value
  let iteration := (iteration).value
  let divisor ← do
    (pure (word_mul (← (word_of_protocol_quantity ⟨denominator⟩))
        (← (word_of_protocol_quantity ⟨iteration⟩))))
  if ((word_is_zero divisor) : Bool)
  then (pure none)
  else
    (do
      let value_parts := (word_divmod value divisor)
      let whole ← (( do
        match (← (word_checked_mul_protocol_quantity value_parts.quotient ⟨factor⟩)) with
        | .some product => (pure product)
        | none => SailME.throw (none : (Option WordDivMod)) ) : SailME (Option WordDivMod) word )
      let residual_product ← do
        (pure (word_mul value_parts.remainder (← (word_of_protocol_quantity ⟨factor⟩))))
      let residual_sum ← (( do
        match (word_checked_add residual_product (← (word_of_protocol_quantity ⟨addend⟩))) with
        | .some sum => (pure sum)
        | none => SailME.throw (none : (Option WordDivMod)) ) : SailME (Option WordDivMod) word )
      let residual_parts := (word_divmod residual_sum divisor)
      let quotient ← (( do
        match (word_checked_add whole residual_parts.quotient) with
        | .some sum => (pure sum)
        | none => SailME.throw (none : (Option WordDivMod)) ) : SailME (Option WordDivMod) word )
      (pure (some
          { quotient := quotient,
            remainder := residual_parts.remainder })))

/-- Computes the next scaled term of the blob-fee Taylor series. -/
/- Type quantifiers: k_ex161305_ : Nat, k_ex161304_ : Nat, k_ex161303_ : Nat, 0 ≤ k_ex161303_ ∧
  k_ex161303_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161304_ ∧ k_ex161304_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex161305_ ∧ k_ex161305_ ≤ (2 ^ 64 - 1) -/
def scaled_blob_next (term : ScaledBlobValue) (numerator : blob_gas) (denominator : blob_fee_update_fraction) (iteration : item_index) : SailM (Option ScaledBlobValue) := SailME.run do
  let numerator := (numerator).value
  let denominator := (denominator).value
  let iteration := (iteration).value
  let remainder_product ← do
    (pure (word_mul (← (word_of_protocol_quantity ⟨(term.remainder).value⟩))
        (← (word_of_protocol_quantity ⟨numerator⟩))))
  let addend_word ← do
    (pure (word_divmod remainder_product (← (word_of_protocol_quantity ⟨denominator⟩))).quotient)
  let addend ← (( do
    match (word_to_limb addend_word) with
    | .some value => (pure (BitVec.toNatInt value))
    | none => SailME.throw (none : (Option ScaledBlobValue)) ) : SailME (Option ScaledBlobValue) Nat
    )
  let divided ← (( do
    match (← (blob_product_divmod term.whole ⟨numerator⟩ ⟨addend⟩ ⟨denominator⟩
        ⟨iteration⟩)) with
    | .some result => (pure result)
    | none => SailME.throw (none : (Option ScaledBlobValue)) ) : SailME (Option ScaledBlobValue)
    WordDivMod )
  let remainder_word ← do
    (pure (word_divmod divided.remainder (← (word_of_protocol_quantity ⟨iteration⟩))).quotient)
  let remainder ← (( do
    match (word_to_limb remainder_word) with
    | .some value => (pure (BitVec.toNatInt value))
    | none => SailME.throw (none : (Option ScaledBlobValue)) ) : SailME (Option ScaledBlobValue) Nat
    )
  (pure (some
      { whole := divided.quotient,
        remainder := ⟨remainder⟩ }))

/-- The EIP-4844 `fake_exponential`: an integer approximation of
`factor · e^(numerator/denominator)` by Taylor expansion. -/
/- Type quantifiers: k_ex161307_ : Nat, k_ex161306_ : Nat, 0 ≤ k_ex161306_ ∧
  k_ex161306_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161307_ ∧ k_ex161307_ ≤ (2 ^ 64 - 1) -/
def fake_exponential (factor : word) (numerator : blob_gas) (denominator : blob_fee_update_fraction) : SailM word := do
  let numerator := (numerator).value
  let denominator := (denominator).value
  if ((denominator == 0) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let i : Nat := 1
  let output : ScaledBlobValue :=
    { whole := ZERO_WORD,
      remainder := ⟨0⟩ }
  let term : ScaledBlobValue :=
    { whole := factor,
      remainder := ⟨0⟩ }
  let (i, output, term) ← (( do
    let loop_vars ← whileFuelM (fuel :=((2 ^i 64) -i i)) (fun (i, output, term) => (pure ((word_nonzero
          term.whole) || (((term.remainder).value != 0) : Bool)))) (i, output, term)
      fun (i, output, term) => do
        assert true "loop dummy assert"
        let output ←
          match (scaled_blob_add output term ⟨denominator⟩) with
          | .some sum => (pure sum)
          | none => sailThrow ((InvalidBlock ExecutionInvalid))
        let term ←
          match (← (scaled_blob_next term ⟨numerator⟩ ⟨denominator⟩ ⟨i⟩)) with
          | .some next => (pure next)
          | none => sailThrow ((InvalidBlock ExecutionInvalid))
        if ((i == (BYTE_QUANTITY_MAX).value) : Bool)
        then sailThrow ((InvalidBlock ExecutionInvalid))
        else (pure ())
        let i ←
          (do
              let semanticResult ← (item_count_increment ⟨i⟩)
              pure ((semanticResult).value))
        (pure (i, output, term))
    (pure loop_vars) ) : SailM (Nat × ScaledBlobValue × ScaledBlobValue) )
  (pure output.whole)

def blob_base_fee_update_fraction (_ : Unit) : SailM blob_fee_update_fraction := do
  let semanticResult ← do (pure ((← readReg k_blob_schedule).base_fee_update_fraction).value)
  pure (⟨semanticResult⟩)

/-- The blob base fee for a given excess blob gas. -/
/- Type quantifiers: excess_blob_gas : Nat, 0 ≤ excess_blob_gas ∧
  excess_blob_gas ≤ (2 ^ 64 - 1) -/
def blob_base_fee (excess_blob_gas : blob_gas) : SailM word := do
  let excess_blob_gas := (excess_blob_gas).value
  (fake_exponential MIN_BLOB_BASE_FEE ⟨excess_blob_gas⟩
    ⟨((← (blob_base_fee_update_fraction ()))).value⟩)

/-- Tests the EIP-7918 reserve-price condition without overflowing a word. -/
def blob_reserve_price_active (base_fee : word) (price : word) : Bool :=
  if ((((word_limb_3 price) &&& 0xF000000000000000#64) == LIMB_ZERO) : Bool)
  then (word_ult (word_shift_left_limb price 0x0000000000000004#64) base_fee)
  else false

def blob_schedule_target (_ : Unit) : SailM blob_count := do
  let semanticResult ← do (pure ((← readReg k_blob_schedule).target).value)
  pure (⟨semanticResult⟩)

def blob_schedule_max (_ : Unit) : SailM blob_count := do
  let semanticResult ← do (pure ((← readReg k_blob_schedule).max).value)
  pure (⟨semanticResult⟩)

/-- Converts a blob count to blob gas and rejects an invalid schedule. -/
/- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ (2 ^ 64 - 1) -/
def blob_gas_for_count (count : blob_count) : SailM blob_gas := do
  let count := (count).value
  let semanticResult ← do
    let product := ((GAS_PER_BLOB).value *i count)
    if ((product ≤b (BYTE_QUANTITY_MAX).value) : Bool)
    then (pure product)
    else sailThrow ((InvalidBlock InvalidConfig))
  pure (⟨semanticResult⟩)

def blob_target_gas_per_block (_ : Unit) : SailM blob_gas := do
  let semanticResult ← do
    (do
        let semanticResult ← (blob_gas_for_count ⟨((← (blob_schedule_target ()))).value⟩)
        pure ((semanticResult).value))
  pure (⟨semanticResult⟩)

def blob_max_gas_per_block (_ : Unit) : SailM blob_gas := do
  let semanticResult ← do
    (do
        let semanticResult ← (blob_gas_for_count ⟨((← (blob_schedule_max ()))).value⟩)
        pure ((semanticResult).value))
  pure (⟨semanticResult⟩)

/-- Adds transaction blob gas to the block total, rejecting the active
schedule limit. -/
/- Type quantifiers: k_ex161311_ : Nat, k_ex161310_ : Nat, 0 ≤ k_ex161310_ ∧
  k_ex161310_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161311_ ∧ k_ex161311_ ≤ (2 ^ 64 - 1) -/
def checked_block_blob_gas_add (accumulated : blob_gas) (transaction : blob_gas) : SailM blob_gas := do
  let accumulated := (accumulated).value
  let transaction := (transaction).value
  let semanticResult ← do
    let maximum ← do
      (do
          let semanticResult ← (blob_max_gas_per_block ())
          pure ((semanticResult).value))
    if ((accumulated ≤b maximum) : Bool)
    then
      (do
        if ((transaction ≤b (maximum -i accumulated)) : Bool)
        then (pure (accumulated + transaction))
        else sailThrow ((InvalidBlock BlobGasLimitExceeded)))
    else sailThrow ((InvalidBlock BlobGasLimitExceeded))
  pure (⟨semanticResult⟩)

/-- The header `excess_blob_gas` rule: decreases toward zero when the
parent underused blobs, otherwise accumulates; from Osaka, EIP-7918
substitutes the reserve-price form when the execution base fee
dominates. -/
/- Type quantifiers: k_ex161313_ : Nat, k_ex161312_ : Nat, 0 ≤ k_ex161312_ ∧
  k_ex161312_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161313_ ∧ k_ex161313_ ≤ (2 ^ 64 - 1) -/
def next_excess_blob_gas (parent_excess_blob_gas : blob_gas) (parent_blob_gas_used : blob_gas) (parent_base_fee_per_gas : word) : SailM blob_gas := do
  let parent_excess_blob_gas := (parent_excess_blob_gas).value
  let parent_blob_gas_used := (parent_blob_gas_used).value
  let semanticResult ← do
    let parent_blob_gas := (parent_excess_blob_gas + parent_blob_gas_used)
    let target_blob_gas ← do
      (do
          let semanticResult ← (blob_target_gas_per_block ())
          pure ((semanticResult).value))
    if ((parent_blob_gas <b target_blob_gas) : Bool)
    then (pure 0)
    else
      (do
        let price ← do (blob_base_fee ⟨parent_excess_blob_gas⟩)
        if (((fork_gteq (← readReg k_fork) Osaka) && (blob_reserve_price_active
               parent_base_fee_per_gas price)) : Bool)
        then
          (do
            let target ← do
              (do
                  let semanticResult ← (blob_schedule_target ())
                  pure ((semanticResult).value))
            let maximum ← do
              (do
                  let semanticResult ← (blob_schedule_max ())
                  pure ((semanticResult).value))
            if (((maximum == 0) || (maximum <b target)) : Bool)
            then sailThrow ((InvalidBlock InvalidConfig))
            else (pure ())
            let schedule_delta := (maximum -i target)
            let scaled := (parent_blob_gas_used *i schedule_delta)
            let share ← do (exact_quotient scaled maximum)
            let next := (parent_excess_blob_gas + share)
            if ((next ≤b (BYTE_QUANTITY_MAX).value) : Bool)
            then (pure next)
            else sailThrow ((InvalidBlock InvalidConfig)))
        else
          (do
            let next := (parent_blob_gas -i target_blob_gas)
            if ((next ≤b (BYTE_QUANTITY_MAX).value) : Bool)
            then (pure next)
            else sailThrow ((InvalidBlock InvalidConfig))))
  pure (⟨semanticResult⟩)

def G_zero : gas_constant := (GasConstant 0)

def G_jumpdest : gas_constant := (GasConstant 1)

def G_base : gas_constant := (GasConstant 2)

def G_verylow : gas_constant := (GasConstant 3)

def G_low : gas_constant := (GasConstant 5)

def G_mid : gas_constant := (GasConstant 8)

def G_high : gas_constant := (GasConstant 10)

def G_warm_access : gas_constant := (GasConstant 100)

def G_cold_sload : gas_constant := (GasConstant 2100)

def G_cold_account : gas_constant := (GasConstant 2600)

def G_keccak : gas_constant := (GasConstant 30)

def G_keccak_word : gas_constant := (GasConstant 6)

def G_copy_word : gas_constant := (GasConstant 3)

def G_memory : gas_constant := (GasConstant 3)

def G_log : gas_constant := (GasConstant 375)

def G_logtopic : gas_constant := (GasConstant 375)

def G_logdata : gas_constant := (GasConstant 8)

def G_exp : gas_constant := (GasConstant 10)

def G_expbyte : gas_constant := (GasConstant 50)

def G_sset : gas_constant := (GasConstant 20000)

def G_sreset : gas_constant := (GasConstant 2900)

def R_sclear_pre_london : gas_constant := (GasConstant 15000)

def R_sclear_london : gas_constant := (GasConstant 4800)

def R_selfdestruct_pre_london : gas_constant := (GasConstant 24000)

def G_create : gas_constant := (GasConstant 32000)

def G_codedeposit : gas_constant := (GasConstant 200)

def G_callvalue : gas_constant := (GasConstant 9000)

def G_callstipend : gas := (Gas 2300)

def G_newaccount : gas_constant := (GasConstant 25000)

def G_selfdestruct : gas_constant := (GasConstant 5000)

def G_initcode_word : gas_constant := (GasConstant 2)

/-- The deployed-code size limit: 24576 (EIP-170), raised to 32768 by
EIP-7954 at Amsterdam; the shared interpreter serves both the
pre-Amsterdam state tests and the guest. -/
def max_code_size (_ : Unit) : SailM code_length := do
  if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
  then (pure (ByteQuantity 32768))
  else (pure (ByteQuantity 24576))

/-- The `SSTORE`-clears refund: 4800 from London (EIP-3529), 15000
before. -/
def initcode_size_allowed (size : byte_quantity) : SailM Bool := do
  (pure ((fork_lt (← readReg k_fork) Shanghai) || (byte_quantity_le size
        (← (byte_quantity_mul (ByteQuantity 2) (← (max_code_size ())))))))

def sstore_clear_refund (_ : Unit) : SailM gas_constant := do
  if ((fork_gteq (← readReg k_fork) London) : Bool)
  then (pure R_sclear_london)
  else (pure R_sclear_pre_london)

/- Type quantifiers: k_ex161316_ : Nat, k_ex161315_ : Nat, k_ex161314_ : Nat, 0 ≤ k_ex161314_, 0
  ≤ k_ex161315_, 1 ≤ k_ex161316_ ∧ k_ex161316_ ≤ 2000 -/
def gas_product_quotient (left : Nat) (right : Nat) (divisor : gas_divisor) : SailM gas_cost := do
  let divisor := (divisor).value
  (pure (GasCost (← (exact_quotient (left *i right) divisor))))

/-- Computes an exact product-quotient cost from byte quantities. -/
/- Type quantifiers: k_ex161317_ : Nat, 1 ≤ k_ex161317_ ∧ k_ex161317_ ≤ 2000 -/
def gas_product_quotient_bytes (typ_0 : byte_quantity) (typ_1 : byte_quantity) (divisor : gas_divisor) : SailM gas_cost := do
  let divisor := (divisor).value
  let .ByteQuantity left : byte_quantity := typ_0
  let .ByteQuantity right : byte_quantity := typ_1
  (pure (GasCost (← (exact_quotient (left *i right) divisor))))

/-- Debits an exact cost from the running frame or halts out of gas. -/
def charge_cost (app_0 : gas_cost) : SailM Unit := do
  let .GasCost amount := app_0
  if (((! (← (is_running ()))) || (amount == 0)) : Bool)
  then (pure ())
  else
    (do
      let .Gas remaining ← do readReg gas_remaining
      if ((amount ≤b remaining) : Bool)
      then writeReg gas_remaining (Gas (remaining -i amount))
      else (exc_halt OutOfGas))

/-- Debits a fixed schedule cost from the running frame. -/
def charge_constant (app_0 : gas_constant) : SailM Unit := do
  let .GasConstant amount := app_0
  (charge_cost (GasCost amount))

/-- Subtracts an exact cost from gas or records an out-of-gas halt. -/
def gas_sub_cost_or_oog (typ_0 : gas) (typ_1 : gas_cost) : SailM gas := do
  let .Gas left : gas := typ_0
  let .GasCost right : gas_cost := typ_1
  if ((right ≤b left) : Bool)
  then (pure (Gas (left -i right)))
  else
    (do
      (exc_halt OutOfGas)
      (pure GAS_ZERO))

/-- Subtracts bounded gas or records an out-of-gas halt. -/
def gas_sub_gas_or_oog (typ_0 : gas) (typ_1 : gas) : SailM gas := do
  let .Gas left : gas := typ_0
  let .Gas right : gas := typ_1
  if ((right ≤b left) : Bool)
  then (pure (Gas (left -i right)))
  else
    (do
      (exc_halt OutOfGas)
      (pure GAS_ZERO))

/-- Returns unused child gas to the parent frame. -/
def refund_gas (app_0 : gas) : SailM Unit := do
  let .Gas amount := app_0
  if ((gas_sum_supported (← readReg gas_remaining) (Gas amount)) : Bool)
  then writeReg gas_remaining (← (gas_add (← readReg gas_remaining) (Gas amount)))
  else sailThrow ((InvalidBlock ExecutionInvalid))

/- Type quantifiers: byte_len : Nat, 0 ≤ byte_len -/
def memory_word_count_nat (byte_len : Nat) : SailM Nat := do
  (exact_quotient (byte_len + 31) 32)

/-- Returns the number of 32-byte words covering a byte length. -/
def memory_word_count_bytes (app_0 : byte_quantity) : SailM byte_quantity := do
  let .ByteQuantity byte_len := app_0
  (pure (ByteQuantity (← (exact_quotient (byte_len + 31) 32))))

/-- `C_mem` (YP §9.4.1): the cumulative memory cost of `words` words. -/
def mem_cost (words : byte_quantity) : SailM gas_cost := do
  (pure (gas_cost_add (gas_constant_scale_byte_quantity G_memory words)
      (← (gas_product_quotient_bytes words words ⟨512⟩))))

/-- Computes the exclusive end of an addressed memory range. -/
def memory_required_size (start : word) (size : word) : byte_quantity :=
  if ((word_is_zero size) : Bool)
  then BYTE_ZERO
  else
    (ByteQuantity ((BitVec.toNatInt (word_to_bits start)) + (BitVec.toNatInt (word_to_bits size))))

/-- Computes the larger exclusive end of two addressed memory ranges. -/
def maximum_memory_required_size (left_start : word) (left_size : word) (right_start : word) (right_size : word) : byte_quantity :=
  let left := (memory_required_size left_start left_size)
  let right := (memory_required_size right_start right_size)
  if ((byte_quantity_lt left right) : Bool)
  then right
  else left

/-- Computes the incremental gas needed to reach a memory size. -/
def memory_expansion_gas (required_size : byte_quantity) : SailM gas_cost := do
  let new_words ← do (memory_word_count_bytes required_size)
  let old_words ← do (memory_word_count_bytes (← (evm_memory_size ())))
  if ((byte_quantity_le new_words old_words) : Bool)
  then (pure GAS_COST_ZERO)
  else (gas_cost_sub (← (mem_cost new_words)) (← (mem_cost old_words)))

/-- Materializes the word-aligned memory size after gas has been charged. -/
def expand_memory (required_size : byte_quantity) : SailM Unit := do
  let expanded_size ← do
    (byte_quantity_mul (← (memory_word_count_bytes required_size)) (ByteQuantity 32))
  assert (byte_quantity_fits_limb expanded_size) "sail/evm/gas.sail:500.49-500.50"
  (memory_expand_to expanded_size)

/-- Charges and applies the expansion implied by one memory range. -/
def charge_memory_expansion (start : word) (size : word) : SailM Unit := do
  if ((← (is_running ())) : Bool)
  then
    (do
      let required_size := (memory_required_size start size)
      (charge_cost (← (memory_expansion_gas required_size)))
      if ((← (is_running ())) : Bool)
      then (expand_memory required_size)
      else (pure ()))
  else (pure ())

/-- Applies a memory expansion whose gas was included in an earlier charge. -/
def expand_charged_memory (required_size : byte_quantity) : SailM Unit := do
  if ((← (is_running ())) : Bool)
  then (expand_memory required_size)
  else (pure ())

/-- The account-access cost for a prior warm bit. -/
/- Type quantifiers: k_ex161319_ : Bool -/
def account_cost (warm : Bool) : gas_constant :=
  if (warm : Bool)
  then G_warm_access
  else G_cold_account

/-- The `SLOAD` cost for a prior warm bit (cold = 2100, EIP-2929). -/
/- Type quantifiers: k_ex161320_ : Bool -/
def sload_cost (warm : Bool) : gas_constant :=
  if (warm : Bool)
  then G_warm_access
  else G_cold_sload

/-- Reads up to 32 big-endian precompile input bytes with zero padding. -/
def pc_word (input : EvmByteSlice) (start : source_pointer) (byte_count : byte_length) : SailM word := do
  let value : word := ZERO_WORD
  let loop_byte_index_lower := 0
  let loop_byte_index_upper := 31
  let mut loop_vars := value
  for byte_index in [loop_byte_index_lower:loop_byte_index_upper:1]i do
    let value := loop_vars
    loop_vars ← do
      let offset : byte_quantity := (ByteQuantity byte_index)
      if ((byte_quantity_lt offset byte_count) : Bool)
      then
        (do
          let cursor ← do (byte_quantity_add start offset)
          let next_byte ← do
            if ((byte_quantity_lt cursor input.len) : Bool)
            then (slice_byte input cursor)
            else (pure 0x00#8)
          (pure (word_or (word_shift_left_limb value 0x0000000000000008#64)
              (U256 (Sail.BitVec.zeroExtend next_byte 256)))))
      else (pure value)
  (pure loop_vars)

def pc_blake2_rounds (input : EvmByteSlice) : SailM blake2_rounds := do
  let semanticResult ← do
    (pure (BitVec.toNatInt
        (Sail.BitVec.extractLsb
          (word_limb_0 (← (pc_word input (ByteQuantity 0) (ByteQuantity 4)))) 31 0)))
  pure (⟨semanticResult⟩)

/-- `MODEXP` (precompile `0x05`) gas: EIP-2565 as amended by EIP-7883
(Osaka). Input layout: `bsize[32] esize[32] msize[32] base exp mod`.
Gas grows with the multiplication complexity of the larger of base
and modulus, and with an iteration count derived from the exponent's
bit length; EIP-7823 (Osaka) caps each field at 1024 bytes, modeled
as an unforwardable cost. -/
def modexp_gas (input : EvmByteSlice) : SailM (Option gas_cost) := do
  let bl_word ← do (pc_word input (ByteQuantity 0) (ByteQuantity 32))
  let el_word ← do (pc_word input (ByteQuantity 32) (ByteQuantity 32))
  let ml_word ← do (pc_word input (ByteQuantity 64) (ByteQuantity 32))
  let bl : Nat := (BitVec.toNatInt (word_to_bits bl_word))
  let el : Nat := (BitVec.toNatInt (word_to_bits el_word))
  let ml : Nat := (BitVec.toNatInt (word_to_bits ml_word))
  let osaka ← do (pure (fork_gteq (← readReg k_fork) Osaka))
  if ((osaka && ((bl >b 1024) || ((el >b 1024) || (ml >b 1024)))) : Bool)
  then (pure none)
  else
    (do
      if (((! osaka) && ((word_is_zero bl_word) && (word_is_zero ml_word))) : Bool)
      then (pure (some (GasCost 200)))
      else
        (do
          let maxlen :=
            if ((ml <b bl) : Bool)
            then bl
            else ml
          let words ← do (exact_quotient (maxlen + 7) 8)
          let words_squared := (words *i words)
          let mult : Nat :=
            if ((osaka && (maxlen ≤b 32)) : Bool)
            then 16
            else
              (if (osaka : Bool)
              then (2 *i words_squared)
              else words_squared)
          let big_mul : Nat :=
            if (osaka : Bool)
            then 16
            else 8
          let exp_off := (96 + bl)
          let iterations ← (( do
            if ((el ≤b 32) : Bool)
            then
              (do
                let exponent_bits ← do
                  (pure ((word_bit_length
                      (← (pc_word input (ByteQuantity exp_off) (ByteQuantity el))))).value)
                if ((exponent_bits == 0) : Bool)
                then (pure 1)
                else
                  (let count := (exponent_bits -i 1)
                  if ((count == 0) : Bool)
                  then (pure 1)
                  else (pure count)))
            else
              (do
                let head_bits ← do
                  (pure ((word_bit_length
                      (← (pc_word input (ByteQuantity exp_off) (ByteQuantity 32))))).value)
                let high_bits :=
                  if ((head_bits != 0) : Bool)
                  then (head_bits -i 1)
                  else 0
                let count := ((big_mul *i (el -i 32)) + high_bits)
                if ((count == 0) : Bool)
                then (pure 1)
                else (pure count)) ) : SailM Nat )
          let calculated ← (( do
            if (osaka : Bool)
            then (pure (mult *i iterations))
            else (exact_quotient (mult *i iterations) 3) ) : SailM Nat )
          let minimum : Nat :=
            if (osaka : Bool)
            then 500
            else 200
          (pure (some
              (GasCost
                (if ((calculated <b minimum) : Bool)
                then minimum
                else calculated))))))

/-- The EIP-2537 BLS12-381 G1 MSM discount table: entry `k−1` is the
per-pair discount (in thousandths) applied to a `k`-pair MSM,
constant for `k ≥ 128`. -/
def BLS_G1_DISCOUNT : (Vector (BitVec 16) 128) :=
  #v[0x0207#16, 0x0208#16, 0x0208#16, 0x0209#16, 0x020A#16, 0x020A#16, 0x020B#16, 0x020C#16, 0x020D#16, 0x020D#16, 0x020E#16, 0x020F#16, 0x0210#16, 0x0210#16, 0x0211#16, 0x0212#16, 0x0213#16, 0x0214#16, 0x0214#16, 0x0215#16, 0x0216#16, 0x0217#16, 0x0218#16, 0x0218#16, 0x0219#16, 0x021A#16, 0x021B#16, 0x021C#16, 0x021C#16, 0x021D#16, 0x021E#16, 0x021F#16, 0x0220#16, 0x0221#16, 0x0222#16, 0x0223#16, 0x0223#16, 0x0224#16, 0x0225#16, 0x0226#16, 0x0227#16, 0x0228#16, 0x0229#16, 0x022A#16, 0x022B#16, 0x022C#16, 0x022D#16, 0x022E#16, 0x022F#16, 0x0230#16, 0x0231#16, 0x0232#16, 0x0233#16, 0x0234#16, 0x0235#16, 0x0236#16, 0x0237#16, 0x0238#16, 0x0239#16, 0x023A#16, 0x023C#16, 0x023D#16, 0x023E#16, 0x023F#16, 0x0240#16, 0x0241#16, 0x0243#16, 0x0244#16, 0x0245#16, 0x0246#16, 0x0248#16, 0x0249#16, 0x024A#16, 0x024C#16, 0x024D#16, 0x024F#16, 0x0250#16, 0x0251#16, 0x0253#16, 0x0254#16, 0x0256#16, 0x0257#16, 0x0259#16, 0x025B#16, 0x025C#16, 0x025E#16, 0x0260#16, 0x0261#16, 0x0263#16, 0x0265#16, 0x0267#16, 0x0269#16, 0x026B#16, 0x026D#16, 0x026F#16, 0x0271#16, 0x0273#16, 0x0276#16, 0x0278#16, 0x027B#16, 0x027D#16, 0x0280#16, 0x0282#16, 0x0285#16, 0x0288#16, 0x028B#16, 0x028E#16, 0x0292#16, 0x0295#16, 0x0299#16, 0x029D#16, 0x02A1#16, 0x02A5#16, 0x02AA#16, 0x02AF#16, 0x02B4#16, 0x02BA#16, 0x02C1#16, 0x02C8#16, 0x02CF#16, 0x02D8#16, 0x02E2#16, 0x02EE#16, 0x02FC#16, 0x031D#16, 0x0350#16, 0x03B5#16, 0x03E8#16]

def BLS_G2_DISCOUNT : (Vector (BitVec 16) 128) :=
  #v[0x020C#16, 0x020C#16, 0x020D#16, 0x020E#16, 0x020E#16, 0x020F#16, 0x0210#16, 0x0210#16, 0x0211#16, 0x0212#16, 0x0212#16, 0x0213#16, 0x0214#16, 0x0214#16, 0x0215#16, 0x0216#16, 0x0217#16, 0x0217#16, 0x0218#16, 0x0219#16, 0x0219#16, 0x021A#16, 0x021B#16, 0x021C#16, 0x021D#16, 0x021D#16, 0x021E#16, 0x021F#16, 0x0220#16, 0x0221#16, 0x0221#16, 0x0222#16, 0x0223#16, 0x0224#16, 0x0225#16, 0x0226#16, 0x0227#16, 0x0228#16, 0x0228#16, 0x0229#16, 0x022A#16, 0x022B#16, 0x022C#16, 0x022D#16, 0x022E#16, 0x022F#16, 0x0230#16, 0x0231#16, 0x0232#16, 0x0233#16, 0x0235#16, 0x0236#16, 0x0237#16, 0x0238#16, 0x0239#16, 0x023A#16, 0x023B#16, 0x023D#16, 0x023E#16, 0x023F#16, 0x0240#16, 0x0242#16, 0x0243#16, 0x0244#16, 0x0246#16, 0x0247#16, 0x0248#16, 0x024A#16, 0x024B#16, 0x024D#16, 0x024E#16, 0x0250#16, 0x0251#16, 0x0253#16, 0x0255#16, 0x0256#16, 0x0258#16, 0x025A#16, 0x025C#16, 0x025E#16, 0x025F#16, 0x0261#16, 0x0263#16, 0x0265#16, 0x0267#16, 0x026A#16, 0x026C#16, 0x026E#16, 0x0270#16, 0x0273#16, 0x0275#16, 0x0278#16, 0x027A#16, 0x027D#16, 0x0280#16, 0x0283#16, 0x0286#16, 0x0289#16, 0x028C#16, 0x028F#16, 0x0293#16, 0x0297#16, 0x029A#16, 0x029E#16, 0x02A2#16, 0x02A7#16, 0x02AB#16, 0x02B0#16, 0x02B5#16, 0x02BB#16, 0x02C0#16, 0x02C7#16, 0x02CD#16, 0x02D4#16, 0x02DC#16, 0x02E4#16, 0x02ED#16, 0x02F7#16, 0x0302#16, 0x030E#16, 0x031C#16, 0x032C#16, 0x0340#16, 0x0357#16, 0x0374#16, 0x039B#16, 0x03E8#16, 0x03E8#16]

/-- EIP-2537 MSM gas: `(k · base · discount(k)) / 1000`, with the
discount clamped to the `k = 128` entry beyond the table. -/
/- Type quantifiers: k_ex161334_ : Nat, 0 ≤ k_ex161334_ ∧ k_ex161334_ ≤ (2 ^ 16 - 1) -/
def bls_msm_gas (table : (Vector (BitVec 16) 128)) (base : gas_constant) (maxd : bls_discount) (k : byte_quantity) : SailM gas_cost := do
  let maxd := (maxd).value
  if ((k == BYTE_ZERO) : Bool)
  then (pure GAS_COST_ZERO)
  else
    (do
      let discount : Nat := maxd
      let discount ← (( do
        if ((byte_quantity_lt k (ByteQuantity 128)) : Bool)
        then
          (do
            let target ← do (byte_quantity_sub (ByteQuantity 128) k)
            let cursor : byte_quantity := BYTE_ZERO
            let (cursor, discount) ← (( do
              let loop_index_lower := 0
              let loop_index_upper := 127
              let mut loop_vars := (cursor, discount)
              for index in [loop_index_lower:loop_index_upper:1]i do
                let (cursor, discount) := loop_vars
                loop_vars ← do
                  let discount : Nat :=
                    if ((cursor == target) : Bool)
                    then (BitVec.toNatInt (GetElem?.getElem! table index))
                    else discount
                  let cursor ← (byte_quantity_add cursor BYTE_ONE)
                  (pure (cursor, discount))
              (pure loop_vars) ) : SailM (byte_quantity × Nat) )
            (pure discount))
        else (pure discount) ) : SailM Nat )
      (gas_cost_quotient (gas_cost_scale_byte_quantity (gas_constant_scale base discount) k)
        ⟨1000⟩))

def linear_gas (base : gas_constant) (per_unit : gas_constant) (units : byte_quantity) : gas_cost :=
  (gas_cost_add_constant (gas_constant_scale_byte_quantity per_unit units) base)

/-- The gas of the precompile at address `num` for a given input. Gas is
protocol policy defined entirely here; implementations return only
output. Length-only costs
derive from the word count; the two input-dependent curves (`MODEXP`,
`BLAKE2F` rounds) read the input in place. The match arms are the
precompile catalog with their addresses and pricing EIPs. -/
/- Type quantifiers: k_ex161335_ : Nat, 1 ≤ k_ex161335_ ∧ k_ex161335_ ≤ 256 -/
def precompile_gas (num : precompile_id) (input : EvmByteSlice) : SailM (Option gas_cost) := do
  let num := (num).value
  let input_len := input.len
  let words ← do (memory_word_count_bytes input_len)
  match num with
  | 1 => (pure (some (GasCost 3000)))
  | 2 => (pure (some (linear_gas (GasConstant 60) (GasConstant 12) words)))
  | 3 => (pure (some (linear_gas (GasConstant 600) (GasConstant 120) words)))
  | 4 => (pure (some (linear_gas (GasConstant 15) (GasConstant 3) words)))
  | 5 => (modexp_gas input)
  | 6 => (pure (some (GasCost 150)))
  | 7 => (pure (some (GasCost 6000)))
  | 8 =>
    (pure (some
        (linear_gas (GasConstant 45000) (GasConstant 34000)
          (← (byte_quantity_quotient input_len (ByteQuantity 192))))))
  | 9 => (pure (some (GasCost ((← (pc_blake2_rounds input))).value)))
  | 10 => (pure (some (GasCost 50000)))
  | 11 => (pure (some (GasCost 375)))
  | 12 =>
    (pure (some
        (← (bls_msm_gas BLS_G1_DISCOUNT (GasConstant 12000) ⟨519⟩
            (← (byte_quantity_quotient input_len (ByteQuantity 160)))))))
  | 13 => (pure (some (GasCost 600)))
  | 14 =>
    (pure (some
        (← (bls_msm_gas BLS_G2_DISCOUNT (GasConstant 22500) ⟨524⟩
            (← (byte_quantity_quotient input_len (ByteQuantity 288)))))))
  | 15 =>
    (pure (some
        (linear_gas (GasConstant 37700) (GasConstant 32600)
          (← (byte_quantity_quotient input_len (ByteQuantity 384))))))
  | 16 => (pure (some (GasCost 5500)))
  | 17 => (pure (some (GasCost 23800)))
  | 256 => (pure (some (GasCost 6900)))
  | _ => (pure (some GAS_COST_ZERO))

/-- `SSTORE` pricing: a three-way comparison (EIP-2200) of *original*
(the slot's transaction-start value), *current*, and *new*. Writing
the same value, or dirtying an already-dirty slot, costs warm access
only; a clean slot going zero↔nonzero pays `G_sset` / `G_sreset`.
Returns `(gas_cost, refund_delta)`; refunds track
clearing/un-clearing and resetting a slot to its original value
(EIP-3529 rates). The EIP-2929 cold surcharge is added when the
caller reports the slot was not yet warm. The refund delta is summed
and capped at transaction scope (EIP-3529, `gas_used/5`), not
here. -/
/- Type quantifiers: k_ex161336_ : Bool -/
def sstore_gas (original : word) (current : word) (new : word) (cold : Bool) : SailM (gas_cost × gas_refund) := do
  let cold_cost : gas_constant :=
    if (cold : Bool)
    then G_cold_sload
    else GAS_CONSTANT_ZERO
  let clear_refund ← (( do (pure (gas_constant_to_refund (← (sstore_clear_refund ())))) ) :
    SailM gas_refund )
  let base : gas_constant :=
    if ((current == new) : Bool)
    then G_warm_access
    else
      (if ((original == current) : Bool)
      then
        (if ((word_is_zero original) : Bool)
        then G_sset
        else G_sreset)
      else G_warm_access)
  let refund : gas_refund := GAS_REFUND_ZERO
  let refund : gas_refund :=
    if ((bne current new) : Bool)
    then
      (if ((original == current) : Bool)
      then
        (if (((! (word_is_zero original)) && (word_is_zero new)) : Bool)
        then (gas_refund_add refund clear_refund)
        else refund)
      else
        (let refund : gas_refund :=
          if ((! (word_is_zero original)) : Bool)
          then
            (let refund : gas_refund :=
              if ((word_is_zero current) : Bool)
              then (gas_refund_sub refund clear_refund)
              else refund
            if ((word_is_zero new) : Bool)
            then (gas_refund_add refund clear_refund)
            else refund)
          else refund
        if ((original == new) : Bool)
        then
          (if ((word_is_zero original) : Bool)
          then
            (gas_refund_sub (gas_refund_add refund (gas_constant_to_refund G_sset))
              (gas_constant_to_refund G_warm_access))
          else
            (gas_refund_sub (gas_refund_add refund (gas_constant_to_refund G_sreset))
              (gas_constant_to_refund G_warm_access)))
        else refund))
    else refund
  let cost := (gas_constant_add base cold_cost)
  (pure (cost, refund))

def charge_memory_word_gas (base : gas_constant) (per_word : gas_constant) (size : byte_quantity) : SailM Unit := do
  (charge_cost
    (gas_cost_add_constant
      (gas_constant_scale_byte_quantity per_word (← (memory_word_count_bytes size))) base))

def charge_keccak_gas (size : byte_quantity) : SailM Unit := do
  (charge_memory_word_gas G_keccak G_keccak_word size)

def charge_copy_gas (size : byte_quantity) : SailM Unit := do
  (charge_memory_word_gas GAS_CONSTANT_ZERO G_copy_word size)

/-- Charges the base, topic, and data-byte components of a log operation. -/
/- Type quantifiers: k_ex161337_ : Nat, 0 ≤ k_ex161337_ ∧ k_ex161337_ ≤ 4 -/
def charge_log_gas (num_topics : log_topic_count) (size : byte_quantity) : SailM Unit := do
  let num_topics := (num_topics).value
  (charge_cost
    (gas_cost_add_constant
      (gas_cost_add (gas_constant_scale_protocol_quantity G_logtopic ⟨num_topics⟩)
        (gas_constant_scale_byte_quantity G_logdata size)) G_log))

/-- `EXP`: base plus `G_expbyte` per significant exponent byte
(EIP-160). -/
def exp_gas (exponent : word) : SailM gas_cost := do
  let significant_bits := ((word_bit_length exponent)).value
  let rounded_bits : Nat := (significant_bits + 7)
  let byte_count ← do
    (do
        let semanticResult ← (protocol_quantity_quotient ⟨rounded_bits⟩ ⟨8⟩)
        pure ((semanticResult).value))
  let significant_bytes ← (( do
    if ((byte_count ≤b 32) : Bool)
    then (pure byte_count)
    else
      (do
        assert false "sail/evm/gas.sail:1073.24-1073.25"
        throw Error.Exit) ) : SailM Nat )
  (pure (gas_cost_add_constant
      (gas_constant_scale_protocol_quantity G_expbyte ⟨significant_bytes⟩) G_exp))

/-- The EIP-3860 per-word initcode cost, charged by `CREATE`/`CREATE2`
and create transactions from Shanghai. -/
def initcode_gas (byte_len : byte_quantity) : SailM gas_cost := do
  if ((fork_gteq (← readReg k_fork) Shanghai) : Bool)
  then
    (pure (gas_constant_scale_byte_quantity G_initcode_word (← (memory_word_count_bytes byte_len))))
  else (pure GAS_COST_ZERO)

/-- Applies the EIP-150 forwarding cap to a word-sized gas request. -/
def call_gas_cap_word (available : gas) (requested : word) : SailM gas := do
  let retained ← do (gas_quotient available ⟨64⟩)
  let all_but_64th ← do (gas_sub_gas_or_oog available retained)
  match (word_to_gas requested) with
  | .some requested_gas =>
    (if ((gas_lt requested_gas all_but_64th) : Bool)
    then (pure requested_gas)
    else (pure all_but_64th))
  | _ => (pure all_but_64th)

