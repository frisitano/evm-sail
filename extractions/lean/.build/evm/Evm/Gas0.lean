import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Quantities
import Evm.Gas
import Evm.ByteSlice
import Evm.Environment
import Evm.Machine

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open ConcurrencyInterfaceV1

namespace Evm.Functions

open option
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open ast
open TxType
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open Register
open NodeRef
open MerkleSlot
open JEntry
open HaltKind
open FrameStatus
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

def MIN_BLOB_BASE_FEE : word := WORD_ONE

def GAS_PER_BLOB : blob_gas := 131072

def undefined_ScaledBlobValue (_ : Unit) : SailM ScaledBlobValue := do
  (pure { whole := ← (undefined_bitvector 256)
          remainder := ← (undefined_range 0 ((2 ^i 64) -i 1)) })

/-- Type quantifiers: denominator : Nat, 0 ≤ denominator ∧ denominator ≤ (2 ^ 64 - 1) -/
def scaled_blob_add (left : ScaledBlobValue) (right : ScaledBlobValue) (denominator : Nat) : (Option ScaledBlobValue) := ExceptM.run do
  if ((denominator == 0) : Bool)
  then (pure none)
  else
    (do
      let combined := (left.remainder +i right.remainder)
      let carry := (denominator ≤b combined)
      let remainder :=
        if (carry : Bool)
        then (combined -i denominator)
        else combined
      if ((((2 ^i 64) -i 1) <b remainder) : Bool)
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
                  { whole := value
                    remainder := remainder }))
            | none => (pure none))
          else
            (pure (some
                { whole := whole
                  remainder := remainder }))))

/-- Type quantifiers: iteration : Nat, denominator : Nat, addend : Nat, factor : Nat, 0 ≤ factor
  ∧ factor ≤ (2 ^ 64 - 1), 0 ≤ addend ∧ addend ≤ (2 ^ 64 - 1), 0 ≤ denominator ∧
  denominator ≤ (2 ^ 64 - 1), 0 ≤ iteration ∧ iteration ≤ (2 ^ 64 - 1) -/
def blob_product_divmod (value : (BitVec 256)) (factor : Nat) (addend : Nat) (denominator : Nat) (iteration : Nat) : SailM (Option WordDivMod) := SailME.run do
  let divisor ← do (pure (word_mul (← (word_of_nat denominator)) (← (word_of_nat iteration))))
  if ((word_is_zero divisor) : Bool)
  then (pure none)
  else
    (do
      let value_parts := (word_divmod value divisor)
      let whole ← (( do
        match (← (word_checked_mul_protocol_quantity value_parts.quotient factor)) with
        | .some product => (pure product)
        | none => SailME.throw (none : (Option WordDivMod)) ) : SailME (Option WordDivMod) word )
      let residual_product ← do (pure (word_mul value_parts.remainder (← (word_of_nat factor))))
      let residual_sum ← (( do
        match (word_checked_add residual_product (← (word_of_nat addend))) with
        | .some sum => (pure sum)
        | none => SailME.throw (none : (Option WordDivMod)) ) : SailME (Option WordDivMod) word )
      let residual_parts := (word_divmod residual_sum divisor)
      let quotient ← (( do
        match (word_checked_add whole residual_parts.quotient) with
        | .some sum => (pure sum)
        | none => SailME.throw (none : (Option WordDivMod)) ) : SailME (Option WordDivMod) word )
      (pure (some
          { quotient := quotient
            remainder := residual_parts.remainder })))

/-- Type quantifiers: iteration : Nat, denominator : Nat, numerator : Nat, 0 ≤ numerator ∧
  numerator ≤ (2 ^ 64 - 1), 0 ≤ denominator ∧ denominator ≤ (2 ^ 64 - 1), 0 ≤ iteration
  ∧ iteration ≤ (2 ^ 64 - 1) -/
def scaled_blob_next (term : ScaledBlobValue) (numerator : Nat) (denominator : Nat) (iteration : Nat) : SailM (Option ScaledBlobValue) := SailME.run do
  let remainder_product ← do
    (pure (word_mul (← (word_of_nat term.remainder)) (← (word_of_nat numerator))))
  let addend_word ← do
    (pure (word_divmod remainder_product (← (word_of_nat denominator))).quotient)
  if (((Sail.BitVec.extractLsb addend_word 255 64) != (BitVec.zero 192)) : Bool)
  then (pure none)
  else
    (do
      let addend := (BitVec.toNatInt (Sail.BitVec.extractLsb addend_word 63 0))
      let divided ← (( do
        match (← (blob_product_divmod term.whole numerator addend denominator iteration)) with
        | .some result => (pure result)
        | none => SailME.throw (none : (Option ScaledBlobValue)) ) : SailME (Option ScaledBlobValue)
        WordDivMod )
      let remainder_word ← do
        (pure (word_divmod divided.remainder (← (word_of_nat iteration))).quotient)
      if (((Sail.BitVec.extractLsb remainder_word 255 64) != (BitVec.zero 192)) : Bool)
      then (pure none)
      else
        (let remainder := (BitVec.toNatInt (Sail.BitVec.extractLsb remainder_word 63 0))
        (pure (some
            { whole := divided.quotient
              remainder := remainder }))))

/-- Type quantifiers: denominator : Nat, numerator : Nat, 0 ≤ numerator ∧
  numerator ≤ (2 ^ 64 - 1), 0 ≤ denominator ∧ denominator ≤ (2 ^ 64 - 1) -/
def fake_exponential (factor : (BitVec 256)) (numerator : Nat) (denominator : Nat) : SailM (BitVec 256) := do
  if ((denominator == 0) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let i : item_index := 1
  let output : ScaledBlobValue :=
    { whole := ZERO_WORD
      remainder := 0 }
  let term : ScaledBlobValue :=
    { whole := factor
      remainder := 0 }
  let (i, output, term) ← (( do
    let mut loop_vars := (i, output, term)
    while (λ (i, output, term) => ((word_nonzero term.whole) || ((term.remainder != 0) : Bool)))
      loop_vars
      do
      let (i, output, term) := loop_vars
      loop_vars ← do
        let output ←
          match (scaled_blob_add output term denominator) with
          | .some sum => (pure sum)
          | none => sailThrow ((InvalidBlock ExecutionInvalid))
        let term ←
          match (← (scaled_blob_next term numerator denominator i)) with
          | .some next => (pure next)
          | none => sailThrow ((InvalidBlock ExecutionInvalid))
        if ((i == ((2 ^i 64) -i 1)) : Bool)
        then sailThrow ((InvalidBlock ExecutionInvalid))
        else (pure ())
        let i ← (item_count_increment i)
        (pure (i, output, term))
    (pure loop_vars) ) : SailM (Nat × ScaledBlobValue × ScaledBlobValue) )
  (pure output.whole)

def blob_base_fee_update_fraction (_ : Unit) : SailM Nat := do
  (pure (← readReg k_blob_schedule).base_fee_update_fraction)

/-- Type quantifiers: excess_blob_gas : Nat, 0 ≤ excess_blob_gas ∧
  excess_blob_gas ≤ (2 ^ 64 - 1) -/
def blob_base_fee (excess_blob_gas : Nat) : SailM (BitVec 256) := do
  (fake_exponential MIN_BLOB_BASE_FEE excess_blob_gas (← (blob_base_fee_update_fraction ())))

def blob_reserve_price_active (base_fee : (BitVec 256)) (price : (BitVec 256)) : Bool :=
  if (((Sail.BitVec.extractLsb price 255 252) == (BitVec.zero 4)) : Bool)
  then (word_ult (price <<< 4) base_fee)
  else false

def blob_schedule_target (_ : Unit) : SailM Nat := do
  (pure (← readReg k_blob_schedule).target)

def blob_schedule_max (_ : Unit) : SailM Nat := do
  (pure (← readReg k_blob_schedule).max)

/-- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ (2 ^ 64 - 1) -/
def blob_gas_for_count (count : Nat) : SailM Nat := do
  let product := (GAS_PER_BLOB *i count)
  if ((product ≤b ((2 ^i 64) -i 1)) : Bool)
  then (pure product)
  else sailThrow ((InvalidBlock InvalidConfig))

def blob_target_gas_per_block (_ : Unit) : SailM Nat := do
  (blob_gas_for_count (← (blob_schedule_target ())))

def blob_max_gas_per_block (_ : Unit) : SailM Nat := do
  (blob_gas_for_count (← (blob_schedule_max ())))

/-- Type quantifiers: parent_blob_gas_used : Nat, parent_excess_blob_gas : Nat, 0 ≤
  parent_excess_blob_gas ∧ parent_excess_blob_gas ≤ (2 ^ 64 - 1), 0 ≤ parent_blob_gas_used ∧
  parent_blob_gas_used ≤ (2 ^ 64 - 1) -/
def next_excess_blob_gas (parent_excess_blob_gas : Nat) (parent_blob_gas_used : Nat) (parent_base_fee_per_gas : (BitVec 256)) : SailM Nat := do
  let parent_blob_gas := (parent_excess_blob_gas +i parent_blob_gas_used)
  let target_blob_gas ← do (blob_target_gas_per_block ())
  if ((parent_blob_gas <b target_blob_gas) : Bool)
  then (pure 0)
  else
    (do
      let price ← do (blob_base_fee parent_excess_blob_gas)
      if (((fork_gteq (← readReg k_fork) Osaka) && (blob_reserve_price_active
             parent_base_fee_per_gas price)) : Bool)
      then
        (do
          let target ← do (blob_schedule_target ())
          let maximum ← do (blob_schedule_max ())
          if (((maximum == 0) || (maximum <b target)) : Bool)
          then sailThrow ((InvalidBlock InvalidConfig))
          else (pure ())
          let schedule_delta := (maximum -i target)
          let scaled := (parent_blob_gas_used *i schedule_delta)
          let share ← do (exact_quotient scaled maximum)
          let next := (parent_excess_blob_gas +i share)
          if ((next ≤b ((2 ^i 64) -i 1)) : Bool)
          then (pure next)
          else sailThrow ((InvalidBlock InvalidConfig)))
      else
        (do
          let next := (parent_blob_gas -i target_blob_gas)
          if ((next ≤b ((2 ^i 64) -i 1)) : Bool)
          then (pure next)
          else sailThrow ((InvalidBlock InvalidConfig))))

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

def max_code_size (_ : Unit) : SailM byte_quantity := do
  if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
  then (pure (ByteQuantity 32768))
  else (pure (ByteQuantity 24576))

/-- Type quantifiers: size : Nat, 0 ≤ size -/
def initcode_size_allowed (size : Nat) : SailM Bool := do
  let .ByteQuantity code_limit ← do (max_code_size ())
  (pure ((fork_lt (← readReg k_fork) Shanghai) || (size ≤b (2 *i code_limit))))

def sstore_clear_refund (_ : Unit) : SailM gas_constant := do
  if ((fork_gteq (← readReg k_fork) London) : Bool)
  then (pure R_sclear_london)
  else (pure R_sclear_pre_london)

/-- Type quantifiers: divisor : Nat, right : Nat, left : Nat, 0 ≤ left, 0 ≤ right, 1 ≤ divisor
  ∧ divisor ≤ 1000 -/
def gas_product_quotient (left : Nat) (right : Nat) (divisor : Nat) : SailM gas_cost := do
  (pure (GasCost (← (exact_quotient (left *i right) divisor))))

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

def charge_constant (amount : gas_constant) : SailM Unit := do
  (charge_cost (gas_constant_to_cost amount))

def gas_sub_cost_or_oog (typ_0 : gas) (typ_1 : gas_cost) : SailM gas := do
  let .Gas left : gas := typ_0
  let .GasCost right : gas_cost := typ_1
  if ((right ≤b left) : Bool)
  then (pure (Gas (left -i right)))
  else
    (do
      (exc_halt OutOfGas)
      (pure GAS_ZERO))

def gas_sub_gas_or_oog (typ_0 : gas) (typ_1 : gas) : SailM gas := do
  let .Gas left : gas := typ_0
  let .Gas right : gas := typ_1
  if ((right ≤b left) : Bool)
  then (pure (Gas (left -i right)))
  else
    (do
      (exc_halt OutOfGas)
      (pure GAS_ZERO))

def refund_gas (app_0 : gas) : SailM Unit := do
  let .Gas amount := app_0
  let .Gas remaining ← do readReg gas_remaining
  if ((amount ≤b (((2 ^i 63) -i 1) -i remaining)) : Bool)
  then writeReg gas_remaining (Gas (remaining +i amount))
  else sailThrow ((InvalidBlock ExecutionInvalid))

/-- Type quantifiers: byte_len : Nat, 0 ≤ byte_len -/
def memory_word_count (byte_len : Nat) : SailM Nat := do
  (exact_quotient (byte_len +i 31) 32)

/-- Type quantifiers: words : Nat, 0 ≤ words -/
def mem_cost (words : Nat) : SailM gas_cost := do
  (pure (gas_cost_add (gas_constant_scale G_memory words)
      (← (gas_product_quotient words words 512))))

def memory_required_size (start : (BitVec 256)) (size : (BitVec 256)) : Nat :=
  if ((word_is_zero size) : Bool)
  then 0
  else ((BitVec.toNatInt start) +i (BitVec.toNatInt size))

def maximum_memory_required_size (left_start : (BitVec 256)) (left_size : (BitVec 256)) (right_start : (BitVec 256)) (right_size : (BitVec 256)) : Nat :=
  let left := (memory_required_size left_start left_size)
  let right := (memory_required_size right_start right_size)
  if ((left <b right) : Bool)
  then right
  else left

/-- Type quantifiers: required_size : Nat, 0 ≤ required_size -/
def memory_expansion_gas (required_size : Nat) : SailM gas_cost := do
  let new_words ← do (memory_word_count required_size)
  let .ByteQuantity old_size ← do (evm_memory_size ())
  let old_words ← do (memory_word_count old_size)
  if ((new_words ≤b old_words) : Bool)
  then (pure GAS_COST_ZERO)
  else (gas_cost_sub (← (mem_cost new_words)) (← (mem_cost old_words)))

/-- Type quantifiers: required_size : Nat, 0 ≤ required_size -/
def expand_memory (required_size : Nat) : SailM Unit := do
  let expanded_size ← do (pure ((← (memory_word_count required_size)) *i 32))
  assert (nat_fits_limb expanded_size) "sail/evm/gas.sail:405.39-405.40"
  (memory_expand_to (ByteQuantity expanded_size))

def charge_memory_expansion (start : (BitVec 256)) (size : (BitVec 256)) : SailM Unit := do
  if ((← (is_running ())) : Bool)
  then
    (do
      let required_size := (memory_required_size start size)
      (charge_cost (← (memory_expansion_gas required_size)))
      if ((← (is_running ())) : Bool)
      then (expand_memory required_size)
      else (pure ()))
  else (pure ())

/-- Type quantifiers: required_size : Nat, 0 ≤ required_size -/
def expand_charged_memory (required_size : Nat) : SailM Unit := do
  if ((← (is_running ())) : Bool)
  then (expand_memory required_size)
  else (pure ())

/-- Type quantifiers: k_ex161865_ : Bool -/
def account_cost (warm : Bool) : gas_constant :=
  if (warm : Bool)
  then G_warm_access
  else G_cold_account

/-- Type quantifiers: k_ex161867_ : Bool -/
def sload_cost (warm : Bool) : gas_constant :=
  if (warm : Bool)
  then G_warm_access
  else G_cold_sload

/-- Type quantifiers: byte_count : Nat, start : Nat, 0 ≤ start, 0 ≤ byte_count -/
def pc_word (input : ByteSlice) (start : Nat) (byte_count : Nat) : SailM (BitVec 256) := do
  let value : word := ZERO_WORD
  let .ByteQuantity input_len := input.len
  let loop_byte_index_lower := 0
  let loop_byte_index_upper := 31
  let mut loop_vars := value
  for byte_index in [loop_byte_index_lower:loop_byte_index_upper:1]i do
    let value := loop_vars
    loop_vars ← do
      if ((byte_index <b byte_count) : Bool)
      then
        (do
          let cursor := (start +i byte_index)
          let next_byte ← do
            if ((cursor <b input_len) : Bool)
            then (slice_byte input (ByteQuantity cursor))
            else (pure 0x00#8)
          (pure ((value <<< 8) ||| (Sail.BitVec.zeroExtend next_byte 256))))
      else (pure value)
  (pure loop_vars)

def pc_blake2_rounds (input : ByteSlice) : SailM Nat := do
  (pure (BitVec.toNatInt (Sail.BitVec.extractLsb (← (pc_word input 0 4)) 31 0)))

def modexp_gas (input : ByteSlice) : SailM (Option gas_cost) := do
  let bl_word ← do (pc_word input 0 32)
  let el_word ← do (pc_word input 32 32)
  let ml_word ← do (pc_word input 64 32)
  let bl : Nat := (BitVec.toNatInt bl_word)
  let el : Nat := (BitVec.toNatInt el_word)
  let ml : Nat := (BitVec.toNatInt ml_word)
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
          let words ← do (exact_quotient (maxlen +i 7) 8)
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
          let exp_off := (96 +i bl)
          let iterations ← (( do
            if ((el ≤b 32) : Bool)
            then
              (do
                let exponent_bits ← do (pure (word_bit_length (← (pc_word input exp_off el))))
                if ((exponent_bits == 0) : Bool)
                then (pure 1)
                else
                  (let count := (exponent_bits -i 1)
                  if ((count == 0) : Bool)
                  then (pure 1)
                  else (pure count)))
            else
              (do
                let head_bits ← do (pure (word_bit_length (← (pc_word input exp_off 32))))
                let high_bits :=
                  if ((head_bits != 0) : Bool)
                  then (head_bits -i 1)
                  else 0
                let count := ((big_mul *i (el -i 32)) +i high_bits)
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

def BLS_G1_DISCOUNT : (Vector (BitVec 16) 128) :=
  #v[0x0207#16, 0x0208#16, 0x0208#16, 0x0209#16, 0x020A#16, 0x020A#16, 0x020B#16, 0x020C#16, 0x020D#16, 0x020D#16, 0x020E#16, 0x020F#16, 0x0210#16, 0x0210#16, 0x0211#16, 0x0212#16, 0x0213#16, 0x0214#16, 0x0214#16, 0x0215#16, 0x0216#16, 0x0217#16, 0x0218#16, 0x0218#16, 0x0219#16, 0x021A#16, 0x021B#16, 0x021C#16, 0x021C#16, 0x021D#16, 0x021E#16, 0x021F#16, 0x0220#16, 0x0221#16, 0x0222#16, 0x0223#16, 0x0223#16, 0x0224#16, 0x0225#16, 0x0226#16, 0x0227#16, 0x0228#16, 0x0229#16, 0x022A#16, 0x022B#16, 0x022C#16, 0x022D#16, 0x022E#16, 0x022F#16, 0x0230#16, 0x0231#16, 0x0232#16, 0x0233#16, 0x0234#16, 0x0235#16, 0x0236#16, 0x0237#16, 0x0238#16, 0x0239#16, 0x023A#16, 0x023C#16, 0x023D#16, 0x023E#16, 0x023F#16, 0x0240#16, 0x0241#16, 0x0243#16, 0x0244#16, 0x0245#16, 0x0246#16, 0x0248#16, 0x0249#16, 0x024A#16, 0x024C#16, 0x024D#16, 0x024F#16, 0x0250#16, 0x0251#16, 0x0253#16, 0x0254#16, 0x0256#16, 0x0257#16, 0x0259#16, 0x025B#16, 0x025C#16, 0x025E#16, 0x0260#16, 0x0261#16, 0x0263#16, 0x0265#16, 0x0267#16, 0x0269#16, 0x026B#16, 0x026D#16, 0x026F#16, 0x0271#16, 0x0273#16, 0x0276#16, 0x0278#16, 0x027B#16, 0x027D#16, 0x0280#16, 0x0282#16, 0x0285#16, 0x0288#16, 0x028B#16, 0x028E#16, 0x0292#16, 0x0295#16, 0x0299#16, 0x029D#16, 0x02A1#16, 0x02A5#16, 0x02AA#16, 0x02AF#16, 0x02B4#16, 0x02BA#16, 0x02C1#16, 0x02C8#16, 0x02CF#16, 0x02D8#16, 0x02E2#16, 0x02EE#16, 0x02FC#16, 0x031D#16, 0x0350#16, 0x03B5#16, 0x03E8#16]

def BLS_G2_DISCOUNT : (Vector (BitVec 16) 128) :=
  #v[0x020C#16, 0x020C#16, 0x020D#16, 0x020E#16, 0x020E#16, 0x020F#16, 0x0210#16, 0x0210#16, 0x0211#16, 0x0212#16, 0x0212#16, 0x0213#16, 0x0214#16, 0x0214#16, 0x0215#16, 0x0216#16, 0x0217#16, 0x0217#16, 0x0218#16, 0x0219#16, 0x0219#16, 0x021A#16, 0x021B#16, 0x021C#16, 0x021D#16, 0x021D#16, 0x021E#16, 0x021F#16, 0x0220#16, 0x0221#16, 0x0221#16, 0x0222#16, 0x0223#16, 0x0224#16, 0x0225#16, 0x0226#16, 0x0227#16, 0x0228#16, 0x0228#16, 0x0229#16, 0x022A#16, 0x022B#16, 0x022C#16, 0x022D#16, 0x022E#16, 0x022F#16, 0x0230#16, 0x0231#16, 0x0232#16, 0x0233#16, 0x0235#16, 0x0236#16, 0x0237#16, 0x0238#16, 0x0239#16, 0x023A#16, 0x023B#16, 0x023D#16, 0x023E#16, 0x023F#16, 0x0240#16, 0x0242#16, 0x0243#16, 0x0244#16, 0x0246#16, 0x0247#16, 0x0248#16, 0x024A#16, 0x024B#16, 0x024D#16, 0x024E#16, 0x0250#16, 0x0251#16, 0x0253#16, 0x0255#16, 0x0256#16, 0x0258#16, 0x025A#16, 0x025C#16, 0x025E#16, 0x025F#16, 0x0261#16, 0x0263#16, 0x0265#16, 0x0267#16, 0x026A#16, 0x026C#16, 0x026E#16, 0x0270#16, 0x0273#16, 0x0275#16, 0x0278#16, 0x027A#16, 0x027D#16, 0x0280#16, 0x0283#16, 0x0286#16, 0x0289#16, 0x028C#16, 0x028F#16, 0x0293#16, 0x0297#16, 0x029A#16, 0x029E#16, 0x02A2#16, 0x02A7#16, 0x02AB#16, 0x02B0#16, 0x02B5#16, 0x02BB#16, 0x02C0#16, 0x02C7#16, 0x02CD#16, 0x02D4#16, 0x02DC#16, 0x02E4#16, 0x02ED#16, 0x02F7#16, 0x0302#16, 0x030E#16, 0x031C#16, 0x032C#16, 0x0340#16, 0x0357#16, 0x0374#16, 0x039B#16, 0x03E8#16, 0x03E8#16]

/-- Type quantifiers: k : Nat, maxd : Nat, 0 ≤ maxd ∧ maxd ≤ (2 ^ 16 - 1), 0 ≤ k -/
def bls_msm_gas (table : (Vector (BitVec 16) 128)) (base : gas_constant) (maxd : Nat) (k : Nat) : SailM gas_cost := do
  if ((k == 0) : Bool)
  then (pure GAS_COST_ZERO)
  else
    (do
      let discount : bls_discount := maxd
      let discount ← (( do
        if ((k <b 128) : Bool)
        then
          (do
            let target := (128 -i k)
            let cursor : Nat := 0
            let (cursor, discount) ← (( do
              let loop_index_lower := 0
              let loop_index_upper := 127
              let mut loop_vars := (cursor, discount)
              for index in [loop_index_lower:loop_index_upper:1]i do
                let (cursor, discount) := loop_vars
                loop_vars :=
                  let discount : Nat :=
                    if ((cursor == target) : Bool)
                    then (BitVec.toNatInt (GetElem?.getElem! table index))
                    else discount
                  let cursor : Nat := (cursor +i 1)
                  (cursor, discount)
              (pure loop_vars) ) : SailM (Nat × Nat) )
            (pure discount))
        else (pure discount) ) : SailM Nat )
      (gas_cost_quotient (gas_cost_scale (gas_constant_scale base discount) k) 1000))

/-- Type quantifiers: units : Nat, 0 ≤ units -/
def linear_gas (base : gas_constant) (per_unit : gas_constant) (units : Nat) : gas_cost :=
  (gas_constant_add_cost base (gas_constant_scale per_unit units))

/-- Type quantifiers: num : Nat, 1 ≤ num ∧ num ≤ 256 -/
def precompile_gas (num : Nat) (input : ByteSlice) : SailM (Option gas_cost) := do
  let .ByteQuantity input_len := input.len
  let words ← do (memory_word_count input_len)
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
        (linear_gas (GasConstant 45000) (GasConstant 34000) (← (exact_quotient input_len 192)))))
  | 9 => (pure (some (GasCost (← (pc_blake2_rounds input)))))
  | 10 => (pure (some (GasCost 50000)))
  | 11 => (pure (some (GasCost 375)))
  | 12 =>
    (pure (some
        (← (bls_msm_gas BLS_G1_DISCOUNT (GasConstant 12000) 519
            (← (exact_quotient input_len 160))))))
  | 13 => (pure (some (GasCost 600)))
  | 14 =>
    (pure (some
        (← (bls_msm_gas BLS_G2_DISCOUNT (GasConstant 22500) 524
            (← (exact_quotient input_len 288))))))
  | 15 =>
    (pure (some
        (linear_gas (GasConstant 37700) (GasConstant 32600) (← (exact_quotient input_len 384)))))
  | 16 => (pure (some (GasCost 5500)))
  | 17 => (pure (some (GasCost 23800)))
  | 256 => (pure (some (GasCost 6900)))
  | _ => (pure (some GAS_COST_ZERO))

/-- Type quantifiers: k_ex161970_ : Bool -/
def sstore_gas (original : (BitVec 256)) (current : (BitVec 256)) (new : (BitVec 256)) (cold : Bool) : SailM (gas_cost × gas_refund) := do
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
    if ((current != new) : Bool)
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

/-- Type quantifiers: size : Nat, 0 ≤ size -/
def charge_memory_word_gas (base : gas_constant) (per_word : gas_constant) (size : Nat) : SailM Unit := do
  (charge_cost
    (gas_constant_add_cost base (gas_constant_scale per_word (← (memory_word_count size)))))

/-- Type quantifiers: size : Nat, 0 ≤ size -/
def charge_keccak_gas (size : Nat) : SailM Unit := do
  (charge_memory_word_gas G_keccak G_keccak_word size)

/-- Type quantifiers: size : Nat, 0 ≤ size -/
def charge_copy_gas (size : Nat) : SailM Unit := do
  (charge_memory_word_gas GAS_CONSTANT_ZERO G_copy_word size)

/-- Type quantifiers: size : Nat, num_topics : Nat, 0 ≤ num_topics ∧ num_topics ≤ 4, 0 ≤
  size -/
def charge_log_gas (num_topics : Nat) (size : Nat) : SailM Unit := do
  (charge_cost
    (gas_cost_add (gas_constant_add_cost G_log (gas_constant_scale G_logtopic num_topics))
      (gas_constant_scale G_logdata size)))

def exp_gas (exponent : (BitVec 256)) : SailM gas_cost := do
  let significant_bits := (word_bit_length exponent)
  let byte_count ← do (exact_quotient (significant_bits +i 7) 8)
  let significant_bytes ← (( do
    if ((byte_count ≤b 32) : Bool)
    then (pure byte_count)
    else
      (do
        assert false "sail/evm/gas.sail:946.24-946.25"
        throw Error.Exit) ) : SailM Nat )
  (pure (gas_constant_add_cost G_exp (gas_constant_scale G_expbyte significant_bytes)))

/-- Type quantifiers: byte_len : Nat, 0 ≤ byte_len -/
def initcode_gas (byte_len : Nat) : SailM gas_cost := do
  if ((fork_gteq (← readReg k_fork) Shanghai) : Bool)
  then (pure (gas_constant_scale G_initcode_word (← (memory_word_count byte_len))))
  else (pure GAS_COST_ZERO)

def call_gas_cap_word (available : gas) (requested : (BitVec 256)) : SailM gas := do
  let retained ← do (gas_quotient available 64)
  let all_but_64th ← do (gas_sub_gas_or_oog available retained)
  match (word_to_gas requested) with
  | .some requested_gas =>
    (if ((gas_lt requested_gas all_but_64th) : Bool)
    then (pure requested_gas)
    else (pure all_but_64th))
  | _ => (pure all_but_64th)

