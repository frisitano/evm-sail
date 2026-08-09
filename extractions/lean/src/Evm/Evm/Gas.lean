import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Host.RegionAccess
import Evm.Primitives.Fork
import Evm.Kernel.Environment
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
open ast
open TxType
open TxSignatureScheme
open TrieUpdateSource
open TrieUpdateRelation
open TrieLeafValue
open TrieItemValue
open TrieChange
open StorageTxPopResult
open StorageTxLookup
open StorageBlockIterResult
open StateJournalEntry
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
open NodeRef
open LogTopics
open LogData
open InputTrieNode
open IndexedTrieSource
open HtrRequestKind
open HaltKind
open FrameStatus
open FrameContinuation
open FatalError
open ExceptionKind
open EnvField
open DeepStackOperation
open CreateKind
open CalldataSlice
open CallKind
open BalIterEntry
open AcctTxPopResult
open AcctBlockIterResult

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

`blob_base_fee = fake_exponential(excess_blob_gas,
BLOB_BASE_FEE_UPDATE_FRACTION)`. The active target, maximum, and update
fraction come from the validated SSZ chain config, including BPO1/BPO2. -/

def GAS_PER_BLOB : Nat := (2 ^i 17)

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value < (2 ^ 256) -/
def protocol_word (value : Nat) : Nat :=
  (u256 value)

/- Type quantifiers: k_target : Nat, k_maximum : Nat, k_denominator : Nat, numerator : Nat, (blob_schedule_parameters k_target k_maximum k_denominator)
  ∧
  0 ≤ numerator ∧
  numerator ≤
  (blob_fee_word_exponent_limit * k_denominator + (k_maximum - k_target) * gas_per_blob_value) -/
def fake_exponential_word (schedule : (BlobScheduleFields k_target k_maximum k_denominator)) (numerator : Nat) : SailM Nat := do
  let denominator : Nat := k_denominator
  let scaled_limit : Nat := (denominator *i (2 ^i 256))
  let term_index : Nat := 1
  let output : Nat := 0
  let numerator_accum : Nat := denominator
  let (numerator_accum, output, term_index) ← (( do
    let loop_vars ← whileFuelM (fuel :=(scaled_limit - output)) (fun (numerator_accum, output, term_index) => (pure (numerator_accum >b 0))) (numerator_accum, output, term_index)
      fun (numerator_accum, output, term_index) => do
        assert true "loop dummy assert"
        let current_accum := numerator_accum
        let (numerator_accum, output, term_index) ← (( do
          if ((scaled_limit ≤b current_accum) : Bool)
          then
            (do
              (fatal_error NumericOverflow)
              (pure (numerator_accum, output, term_index)))
          else
            (do
              let bounded_accum : Nat := current_accum
              let next_output := (output + bounded_accum)
              let (numerator_accum, output, term_index) ← (( do
                if ((scaled_limit ≤b next_output) : Bool)
                then
                  (do
                    (fatal_error NumericOverflow)
                    (pure (numerator_accum, output, term_index)))
                else
                  (do
                    let output : Nat := next_output
                    let numerator_accum : Nat :=
                      ((bounded_accum *i numerator) / (denominator *i term_index))
                    let current_index := term_index
                    let term_index ← (( do
                      if ((current_index <b scaled_limit) : Bool)
                      then
                        (let term_index : Nat := (current_index + 1)
                        (pure term_index))
                      else
                        (do
                          (fatal_error NumericOverflow)
                          (pure term_index)) ) : SailM Nat )
                    (pure (numerator_accum, output, term_index))) ) : SailM (Nat × Nat × Nat) )
              (pure (numerator_accum, output, term_index))) ) : SailM (Nat × Nat × Nat) )
        (pure (numerator_accum, output, term_index))
    (pure loop_vars) ) : SailM (Nat × Nat × Nat) )
  let price := (output / denominator)
  (pure (protocol_word price))

/-- Evaluates the word result for one complete correlated profile. The profile
keeps the selected schedule and its reachable excess-gas bound related, so
the generic recurrence needs no fork-specific dispatch. -/
/- Type quantifiers: k_ex552159_ : Nat, profile_dependentWitness9 : Nat, profile_dependentWitness8 :
  Nat, profile_dependentWitness7 : Nat, profile_dependentWitness6 : Nat, profile_dependentWitness5 :
  Nat, profile_dependentWitness4 : Nat, profile_dependentWitness3 : Nat, profile_dependentWitness2 :
  Nat, profile_dependentWitness1 : Nat, profile_dependentWitness0 : Nat, profile_dependentWitness0 =
  5 ∧
  profile_dependentWitness1 = 0 ∧
  profile_dependentWitness2 = 0 ∧
  profile_dependentWitness3 = 1 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 0 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 0 ∧ profile_dependentWitness9 = 2 ∨
  6 ≤ profile_dependentWitness0 ∧ profile_dependentWitness0 ≤ 9 ∧
  profile_dependentWitness1 = 0 ∧
  profile_dependentWitness2 = 0 ∧
  profile_dependentWitness3 = 1 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 0 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 0 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 10 ∧
  profile_dependentWitness1 = 0 ∧
  profile_dependentWitness2 = 0 ∧
  profile_dependentWitness3 = 1 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 0 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 11 ∧
  profile_dependentWitness1 = 3 ∧
  profile_dependentWitness2 = 6 ∧
  profile_dependentWitness3 = 3338477 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 12 ∧
  profile_dependentWitness1 = 6 ∧
  profile_dependentWitness2 = 9 ∧
  profile_dependentWitness3 = 5007716 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 9 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 13 ∧
  profile_dependentWitness1 = 6 ∧
  profile_dependentWitness2 = 9 ∧
  profile_dependentWitness3 = 5007716 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 24) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 14 ∧
  profile_dependentWitness1 = 10 ∧
  profile_dependentWitness2 = 15 ∧
  profile_dependentWitness3 = 8346193 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 24) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 15 ∧
  profile_dependentWitness1 = 14 ∧
  profile_dependentWitness2 = 21 ∧
  profile_dependentWitness3 = 11684671 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 24) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 16 ∧
  profile_dependentWitness1 = 14 ∧
  profile_dependentWitness2 = 21 ∧
  profile_dependentWitness3 = 11684671 ∧
  profile_dependentWitness4 = 65536 ∧
  profile_dependentWitness5 = 131072 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5, 0 ≤ k_ex552159_ ∧
  k_ex552159_ ≤ (256 * 11684671 + 21 * 2 ^ 17) -/
def blob_base_fee (profile : (Sigma fun (k_fork : Nat) =>
  (Sigma fun (k_target : Nat) =>
  (Sigma fun (k_maximum : Nat) =>
  (Sigma fun (k_denominator : Nat) =>
  (Sigma fun (k_code_limit : Nat) =>
  (Sigma fun (k_initcode_limit : Nat) =>
  (Sigma fun (k_transaction_total_gas_limit : Nat) =>
  (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
  (Sigma fun (k_transaction_blob_limit : Nat) =>
  (Sigma fun (k_refund_divisor : Nat) =>
  (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))) (excess_blob_gas : Nat) : SailM Nat := do
  let profile_dependentWitness0 := (profile).1
  let profile_dependentWitness1 := ((profile).2).1
  let profile_dependentWitness2 := (((profile).2).2).1
  let profile_dependentWitness3 := ((((profile).2).2).2).1
  let profile_dependentWitness4 := (((((profile).2).2).2).2).1
  let profile_dependentWitness5 := ((((((profile).2).2).2).2).2).1
  let profile_dependentWitness6 := (((((((profile).2).2).2).2).2).2).1
  let profile_dependentWitness7 := ((((((((profile).2).2).2).2).2).2).2).1
  let profile_dependentWitness8 := (((((((((profile).2).2).2).2).2).2).2).2).1
  let profile_dependentWitness9 := ((((((((((profile).2).2).2).2).2).2).2).2).2).1
  let profile := ((((((((((profile).2).2).2).2).2).2).2).2).2).2
  let limit := profile.excess_blob_gas_limit
  if (((profile.fork ≥b Cancun) && (excess_blob_gas ≤b limit)) : Bool)
  then (fake_exponential_word profile.blob_schedule excess_blob_gas)
  else (fatal_error InvalidConfig)

/- Type quantifiers: maximum_count : Nat, accumulated : Nat, transaction : Nat, 0 ≤ accumulated
  ∧
  accumulated ≤ (bpo2_blob_max_count * gas_per_blob_value) ∧
  0 ≤ transaction ∧ transaction ≤ (prague_blob_max_count * gas_per_blob_value), maximum_count
  = 0 ∨ maximum_count = 6 ∨ maximum_count = 9 ∨ maximum_count = 15 ∨ maximum_count = 21 -/
def block_blob_gas_add (maximum_count : Nat) (accumulated : Nat) (transaction : Nat) : SailM Nat := do
  let maximum := ((2 ^i 17) *i maximum_count)
  if (((accumulated ≤b maximum) && (transaction ≤b (maximum -i accumulated))) : Bool)
  then (pure (accumulated + transaction))
  else (fatal_error BlobGasLimitExceeded)

/-- The header `excess_blob_gas` rule: decreases toward zero when the
parent underused blobs, otherwise accumulates; from Osaka, EIP-7918
substitutes the reserve-price form when the execution base fee
dominates. -/
/- Type quantifiers: k_ex552193_ : Nat, k_ex552192_ : Nat, k_ex552191_ : Nat, profile_dependentWitness9
  : Nat, profile_dependentWitness8 : Nat, profile_dependentWitness7 : Nat, profile_dependentWitness6
  : Nat, profile_dependentWitness5 : Nat, profile_dependentWitness4 : Nat, profile_dependentWitness3
  : Nat, profile_dependentWitness2 : Nat, profile_dependentWitness1 : Nat, profile_dependentWitness0
  : Nat, profile_dependentWitness0 = 5 ∧
  profile_dependentWitness1 = 0 ∧
  profile_dependentWitness2 = 0 ∧
  profile_dependentWitness3 = 1 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 0 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 0 ∧ profile_dependentWitness9 = 2 ∨
  6 ≤ profile_dependentWitness0 ∧ profile_dependentWitness0 ≤ 9 ∧
  profile_dependentWitness1 = 0 ∧
  profile_dependentWitness2 = 0 ∧
  profile_dependentWitness3 = 1 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 0 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 0 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 10 ∧
  profile_dependentWitness1 = 0 ∧
  profile_dependentWitness2 = 0 ∧
  profile_dependentWitness3 = 1 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 0 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 11 ∧
  profile_dependentWitness1 = 3 ∧
  profile_dependentWitness2 = 6 ∧
  profile_dependentWitness3 = 3338477 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 12 ∧
  profile_dependentWitness1 = 6 ∧
  profile_dependentWitness2 = 9 ∧
  profile_dependentWitness3 = 5007716 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 9 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 13 ∧
  profile_dependentWitness1 = 6 ∧
  profile_dependentWitness2 = 9 ∧
  profile_dependentWitness3 = 5007716 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 24) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 14 ∧
  profile_dependentWitness1 = 10 ∧
  profile_dependentWitness2 = 15 ∧
  profile_dependentWitness3 = 8346193 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 24) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 15 ∧
  profile_dependentWitness1 = 14 ∧
  profile_dependentWitness2 = 21 ∧
  profile_dependentWitness3 = 11684671 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 24) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 16 ∧
  profile_dependentWitness1 = 14 ∧
  profile_dependentWitness2 = 21 ∧
  profile_dependentWitness3 = 11684671 ∧
  profile_dependentWitness4 = 65536 ∧
  profile_dependentWitness5 = 131072 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5, 0 ≤ k_ex552191_ ∧
  k_ex552191_ ≤ (256 * 11684671 + 21 * 2 ^ 17), 0 ≤ k_ex552192_ ∧ k_ex552192_ ≤ 21, 0 ≤
  k_ex552193_ ∧ k_ex552193_ ≤ (2 ^ 256 - 1) -/
def next_excess_blob_gas (profile : (Sigma fun (k_fork : Nat) =>
  (Sigma fun (k_target : Nat) =>
  (Sigma fun (k_maximum : Nat) =>
  (Sigma fun (k_denominator : Nat) =>
  (Sigma fun (k_code_limit : Nat) =>
  (Sigma fun (k_initcode_limit : Nat) =>
  (Sigma fun (k_transaction_total_gas_limit : Nat) =>
  (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
  (Sigma fun (k_transaction_blob_limit : Nat) =>
  (Sigma fun (k_refund_divisor : Nat) =>
  (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))) (parent_excess_blob_gas : Nat) (parent_blob_gas_used : Nat) (parent_base_fee_per_gas : Nat) : SailM Nat := do
  let profile_dependentWitness0 := (profile).1
  let profile_dependentWitness1 := ((profile).2).1
  let profile_dependentWitness2 := (((profile).2).2).1
  let profile_dependentWitness3 := ((((profile).2).2).2).1
  let profile_dependentWitness4 := (((((profile).2).2).2).2).1
  let profile_dependentWitness5 := ((((((profile).2).2).2).2).2).1
  let profile_dependentWitness6 := (((((((profile).2).2).2).2).2).2).1
  let profile_dependentWitness7 := ((((((((profile).2).2).2).2).2).2).2).1
  let profile_dependentWitness8 := (((((((((profile).2).2).2).2).2).2).2).2).1
  let profile_dependentWitness9 := ((((((((((profile).2).2).2).2).2).2).2).2).2).1
  let profile := ((((((((((profile).2).2).2).2).2).2).2).2).2).2
  let parent_blob_gas := (parent_excess_blob_gas + parent_blob_gas_used)
  let target_blob_gas := ((2 ^i 17) *i profile.blob_schedule.target)
  if ((parent_blob_gas <b target_blob_gas) : Bool)
  then (pure 0)
  else
    (do
      let parent_blob_base_fee ← do
        (blob_base_fee
          ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩
          parent_excess_blob_gas)
      if (((profile.fork ≥b Osaka) && ((16 *i parent_blob_base_fee) <b parent_base_fee_per_gas)) : Bool)
      then
        (do
          let maximum := profile.blob_schedule.max
          let share :=
            ((parent_blob_gas_used *i (maximum - profile.blob_schedule.target)) / maximum)
          let next := (parent_excess_blob_gas + share)
          let limit := profile.excess_blob_gas_limit
          if ((next ≤b limit) : Bool)
          then (pure next)
          else (fatal_error InvalidConfig))
      else
        (do
          let next := (parent_blob_gas - target_blob_gas)
          let limit := profile.excess_blob_gas_limit
          if ((next ≤b limit) : Bool)
          then (pure next)
          else (fatal_error InvalidConfig)))

def G_zero : gas_constant := 0

def G_jumpdest : gas_constant := 1

def G_base : gas_constant := 2

def G_verylow : gas_constant := 3

def G_low : gas_constant := 5

def G_mid : gas_constant := 8

def G_high : gas_constant := 10

def G_warm_access : gas_constant := 100

def G_cold_sload : gas_constant := 2100

def G_cold_account : gas_constant := 2600

def G_keccak : gas_constant := 30

abbrev G_keccak_word : Nat := 6

def G_copy_word : gas_constant := 3

abbrev G_memory : Nat := 3

def G_log : gas_constant := 375

def G_logtopic : gas_constant := 375

def G_logdata : gas_constant := 8

def G_exp : gas_constant := 10

def G_expbyte : gas_constant := 50

def G_sset : gas_constant := 20000

def G_sreset : gas_constant := 2900

def R_sclear_pre_london : gas_constant := 15000

def R_sclear_london : gas_constant := 4800

def R_selfdestruct_pre_london : gas_constant := 24000

def G_create : gas_constant := 32000

abbrev G_codedeposit : Nat := 200

def G_callvalue : gas_constant := 9000

def G_callstipend : gas := 2300

def G_newaccount : gas_constant := 25000

def G_selfdestruct : gas_constant := 5000

abbrev G_initcode_word : Nat := 2

def G_amsterdam_cold_account_access : gas_constant := 3000

def G_amsterdam_cold_storage_access : gas_constant := 3000

def G_amsterdam_storage_write : gas_constant := 10000

def G_amsterdam_account_write : gas_constant := 8000

def G_amsterdam_call_value : gas_constant := 10300

def G_amsterdam_create_access : gas_constant := 11000

def G_amsterdam_state_byte : gas_constant := 1530

def G_amsterdam_state_storage_set : state_gas_spill := 97920

def G_amsterdam_state_new_account : state_gas_spill := 183600

def G_amsterdam_state_auth_base : state_gas_spill := 35190

def R_amsterdam_storage_clear : gas_constant := 12480

def G_sstore_sentry : gas_cost := 2301

/- Type quantifiers: size : Nat, 0 ≤ size -/
def deployed_code_size_allowed (size : Nat) : SailM Bool := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  (pure (size ≤b profile.deployed_code_size_limit))

/- Type quantifiers: size : Nat, 0 ≤ size -/
def initcode_size_allowed (size : Nat) : SailM Bool := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let limit := profile.initcode_size_limit
  (pure ((limit == 0) || (size ≤b limit)))

/-- The `SSTORE`-clears refund: 4800 from London (EIP-3529), 15000
before. -/
def sstore_clear_refund (_ : Unit) : SailM Nat := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if ((profile.fork ≥b London) : Bool)
  then (pure R_sclear_london)
  else (pure R_sclear_pre_london)

/- Type quantifiers: g : Nat, amount : Nat, 0 ≤ amount, 0 ≤ g -/
def charge (g : Nat) (amount : Nat) : SailM (Bool × Nat) := do
  if ((amount ≤b g) : Bool)
  then (pure (true, (g - amount)))
  else (pure (false, (← (exc_halt g OutOfGas))))

/- Type quantifiers: g : Nat, amount : Nat, 0 ≤ amount, 0 ≤ g -/
def check_execution_gas (g : Nat) (amount : Nat) : SailM (Bool × Nat) := do
  if ((g <b amount) : Bool)
  then (pure (false, (← (exc_halt g OutOfGas))))
  else (pure (true, g))

/- Type quantifiers: left : Nat, 0 ≤ left ∧ left ≤ transaction_execution_gas_limit_value -/
def state_gas_spill_room (left : Nat) : Nat :=
  ((2 ^i 24) - left)

/-- Adds a nonnegative spill amount or rejects a value beyond the transaction cap. -/
/- Type quantifiers: left : Nat, right : Nat, 0 ≤ right, 0 ≤ left ∧ left ≤ (2 ^ 24) -/
def state_gas_spill_add (left : Nat) (right : Nat) : SailM Nat := do
  let room := (state_gas_spill_room left)
  if ((right ≤b room) : Bool)
  then (pure (left + right))
  else (fatal_error ExecutionInvalid)

/- Type quantifiers: g : Nat, amount : Nat, 0 ≤ amount, 0 ≤ g -/
def debit_state_gas (g : Nat) (amount : Nat) : SailM (Bool × Nat) := do
  if ((amount == 0) : Bool)
  then (pure (true, g))
  else
    (do
      let state_left ← do readReg state_gas_remaining
      if ((amount ≤b state_left) : Bool)
      then
        (do
          writeReg state_gas_remaining (state_left - amount)
          (pure (true, g)))
      else
        (do
          let remainder := (amount - state_left)
          if ((remainder ≤b g) : Bool)
          then
            (do
              let spilled ← do readReg state_gas_spilled
              writeReg state_gas_remaining GAS_ZERO
              writeReg state_gas_spilled (← (state_gas_spill_add spilled remainder))
              (pure (true, (g - remainder))))
          else (pure (false, g))))

/- Type quantifiers: g : Nat, amount : Nat, 0 ≤ amount, 0 ≤ g -/
def charge_state_gas (g : Nat) (amount : Nat) : SailM (Bool × Nat) := do
  let (debited, debited_gas) ← do (debit_state_gas g amount)
  if (debited : Bool)
  then (pure (true, debited_gas))
  else (pure (false, (← (exc_halt debited_gas OutOfGas))))

/- Type quantifiers: g : Nat, amount : Nat, 0 ≤ amount, 0 ≤ g -/
def charge_deployment_state_gas (g : Nat) (amount : Nat) : SailM (Bool × Nat) := do
  let (debited, debited_gas) ← do (debit_state_gas g amount)
  if (debited : Bool)
  then (pure (true, debited_gas))
  else (pure (false, (← (exc_halt debited_gas OutOfGas))))

/-- Credits a state-gas charge in exact reverse order: execution spill first,
then the state reservoir. Returns the carried gas with any spill
portion restored. -/
/- Type quantifiers: k_ex552279_ : Nat, k_ex552278_ : Nat, 0 ≤ k_ex552278_, 0 ≤ k_ex552279_ ∧
  k_ex552279_ ≤ (2 ^ 24) -/
def credit_state_gas_refund (g : Nat) (amount : Nat) : SailM Nat := do
  let spilled ← do readReg state_gas_spilled
  if ((amount ≤b spilled) : Bool)
  then
    (do
      if ((amount != 0) : Bool)
      then
        (do
          writeReg state_gas_spilled (spilled - amount)
          (pure (conserved_gas_add g amount)))
      else (pure g))
  else
    (do
      let credited ← (( do
        if ((spilled != 0) : Bool)
        then
          (do
            writeReg state_gas_spilled STATE_GAS_SPILL_ZERO
            (pure (conserved_gas_add g spilled)))
        else (pure g) ) : SailM Nat )
      let to_state : Nat := (amount - spilled)
      writeReg state_gas_remaining (conserved_gas_add (← readReg state_gas_remaining) to_state)
      (pure credited))

/-- Returns a completed child's remaining state reservoir to its parent and
carries forward any state gas that the child drew from execution gas. -/
/- Type quantifiers: k_ex552281_ : Nat, k_ex552280_ : Nat, 0 ≤ k_ex552280_, 0 ≤ k_ex552281_ ∧
  k_ex552281_ ≤ (2 ^ 24) -/
def return_child_state_gas (child_remaining : Nat) (child_spilled : Nat) : SailM Unit := do
  writeReg state_gas_remaining (conserved_gas_add (← readReg state_gas_remaining) child_remaining)
  writeReg state_gas_spilled (← (state_gas_spill_add (← readReg state_gas_spilled) child_spilled))

/-- Returns unused child gas to the parent frame's carried gas. -/
/- Type quantifiers: k_ex552283_ : Nat, k_ex552282_ : Nat, 0 ≤ k_ex552282_, 0 ≤ k_ex552283_ -/
def refund_gas (g : Nat) (amount : Nat) : Nat :=
  (conserved_gas_add g amount)

/-- Total gas subtraction. Protocol callers establish affordability first;
the saturated arm keeps the primitive representation-safe by construction. -/
/- Type quantifiers: k_ex552285_ : Nat, k_ex552284_ : Nat, 0 ≤ k_ex552284_, 0 ≤ k_ex552285_ -/
def gas_sub (left : Nat) (right : Nat) : Nat :=
  if ((right ≤b left) : Bool)
  then (left - right)
  else GAS_ZERO

/- Type quantifiers: byte_len : Nat, 0 ≤ byte_len -/
def memory_word_count (byte_len : Nat) : Nat :=
  let quotient := (Nat.div byte_len 32)
  let remainder := (Nat.mod byte_len 32)
  if ((remainder == 0) : Bool)
  then quotient
  else (quotient + 1)

/-- Returns the number of words covering a word-sized byte length without
forming `byte_len + 31`, whose mathematical intermediate can require 257
bits even though the final quotient remains an EVM word. -/
/- Type quantifiers: byte_len : Nat, 0 ≤ byte_len ∧ byte_len ≤ (2 ^ 256 - 1) -/
def memory_word_count_word (byte_len : Nat) : Nat :=
  let word_bytes := (u256 32)
  let quotient := (word_div_word byte_len word_bytes)
  let remainder := (word_mod_word byte_len word_bytes)
  if ((remainder == WORD_ZERO) : Bool)
  then quotient
  else (word_add_word quotient WORD_ONE)

/- Type quantifiers: words : Nat, (memory_cost_input words) -/
def mem_cost (words : Nat) : Nat :=
  let linear : Nat := (G_memory *i words)
  let square : Nat := (words *i words)
  let quadratic : Nat := (square / 512)
  (linear + quadratic)

/- Type quantifiers: start : Nat, size : Nat, 0 ≤ start ∧
  start < (2 ^ 256) ∧ 0 ≤ size ∧ size < (2 ^ 256) -/
def memory_required_size (start : Nat) (size : Nat) : Nat :=
  if ((size == 0) : Bool)
  then 0
  else (start + size)

/- Type quantifiers: start : Nat, size : Nat, 0 ≤ start ∧
  start < (2 ^ 256) ∧ 0 ≤ size ∧ size < (2 ^ 256) -/
def memory_access (start : Nat) (size : Nat) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) =>
  (Sigma fun (k_required : Nat) => (MemoryAccessFields k_off k_len k_required)))) := do
  if _sailIf0 : ((size == 0) : Bool) = true
  then
    (pure ((⟨_, ⟨_, ⟨_, EMPTY_MEMORY_ACCESS⟩⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) =>
      (Sigma fun (k_required : Nat) => (MemoryAccessFields k_off k_len k_required))))) : (Sigma fun
      (k_off : Nat) =>
      (Sigma fun (k_len : Nat) =>
      (Sigma fun (k_required : Nat) => (MemoryAccessFields k_off k_len k_required))))))
  else
    (do
      if _sailIf1 : ((start ≤b ((2 ^i 32) - 1)) : Bool) = true
      then
        (do
          let bounded_start : Nat := start
          if _sailIf2 : ((size ≤b (((2 ^i 32) - 1) - bounded_start)) : Bool) = true
          then
            (let bounded_size : Nat := size
            let access :=
              ({ range := (memory_range bounded_start bounded_size) } : (MemoryAccessFields bounded_start bounded_size (bounded_start + bounded_size)))
            (pure ((⟨_, ⟨_, ⟨_, access⟩⟩⟩ : (Sigma fun (k_off : Nat) =>
              (Sigma fun (k_len : Nat) =>
              (Sigma fun (k_required : Nat) => (MemoryAccessFields k_off k_len k_required))))) : (Sigma
              fun (k_off : Nat) =>
              (Sigma fun (k_len : Nat) =>
              (Sigma fun (k_required : Nat) => (MemoryAccessFields k_off k_len k_required)))))))
          else
            (do
              (fatal_error ExecutionInvalid)))
      else
        (do
          (fatal_error ExecutionInvalid)))

/- Type quantifiers: mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, required_size : Nat, (memory_expansion_endpoint required_size), 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def memory_expansion_cost (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (required_size : Nat) : Nat :=
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let new_words := (memory_word_count required_size)
  let old_size := (memory_high_water ⟨_, ⟨_, mem⟩⟩)
  let old_words := (memory_word_count old_size)
  if ((new_words ≤b old_words) : Bool)
  then 0
  else
    (let old_cost := (mem_cost old_words)
    let new_cost := (mem_cost new_words)
    (new_cost - old_cost))

/-- Materializes the exact byte high-water mark after gas has been charged,
returning the updated memory cursor. -/
/- Type quantifiers: k_ex552314_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552314_ ∧ k_ex552314_ ≤ (2 ^ 32 - 1) -/
def expand_memory (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (required_size : Nat) : SailM (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let (_, expanded) ← do (memory_expand_to ⟨_, ⟨_, mem⟩⟩ required_size)
  (pure (expanded : (Sigma fun (mem_dependentWitness0 : Nat) =>
    (Sigma fun (mem_dependentWitness1 : Nat) =>
    (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))))

/-- The account-access cost for a prior warm bit. -/
/- Type quantifiers: k_ex552315_ : Bool -/
def account_cost (warm : Bool) : SailM Nat := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if (warm : Bool)
  then (pure G_warm_access)
  else
    (if ((profile.fork ≥b Amsterdam) : Bool)
    then (pure G_amsterdam_cold_account_access)
    else (pure G_cold_account))

/-- The second database read performed by `EXTCODESIZE` and `EXTCODECOPY`.
EIP-8038 prices the code-store read as one warm access at Amsterdam. -/
def external_code_read_cost (_ : Unit) : SailM Nat := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if ((profile.fork ≥b Amsterdam) : Bool)
  then (pure G_warm_access)
  else (pure G_zero)

/-- The `SLOAD` cost for a prior warm bit (cold = 2100, EIP-2929). -/
/- Type quantifiers: k_ex552316_ : Bool -/
def sload_cost (warm : Bool) : SailM Nat := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if (warm : Bool)
  then (pure G_warm_access)
  else
    (if ((profile.fork ≥b Amsterdam) : Bool)
    then (pure G_amsterdam_cold_storage_access)
    else (pure G_cold_sload))

/-- The execution-gas component of a value-bearing CALL/CALLCODE. Amsterdam
reprices the account write while retaining the 2300 child stipend. -/
def call_value_cost (_ : Unit) : SailM Nat := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if ((profile.fork ≥b Amsterdam) : Bool)
  then (pure G_amsterdam_call_value)
  else (pure G_callvalue)

/-- The CREATE/CREATE2 execution-access charge. Persistent account growth is
charged separately as state gas at Amsterdam. -/
def create_access_cost (_ : Unit) : SailM Nat := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if ((profile.fork ≥b Amsterdam) : Bool)
  then (pure G_amsterdam_create_access)
  else (pure G_create)

/-- Returns the affordable execution-gas charge after successful initcode.
Legacy forks charge per byte; Amsterdam charges the keccak word cost. The
affordability guard bounds the native product without imposing a protocol
code-size limit on Frontier or Homestead. -/
/- Type quantifiers: k_ex552318_ : Nat, k_ex552317_ : Nat, 0 ≤ k_ex552317_ ∧
  k_ex552317_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex552318_ -/
def code_deployment_execution_cost (byte_len : Nat) (available : Nat) : SailM GasCharge := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if ((profile.fork ≥b Amsterdam) : Bool)
  then
    (if ((byte_len ≤b profile.deployed_code_size_limit) : Bool)
    then
      (let words := (memory_word_count byte_len)
      if ((words ≤b (available / G_keccak_word)) : Bool)
      then
        (let cost : Nat := (G_keccak_word *i words)
        (pure (gas_charge cost)))
      else (pure GAS_CHARGE_UNAFFORDABLE))
    else (pure GAS_CHARGE_UNAFFORDABLE))
  else
    (if ((byte_len ≤b (available / G_codedeposit)) : Bool)
    then
      (let cost : Nat := (G_codedeposit *i byte_len)
      (pure (gas_charge cost)))
    else (pure GAS_CHARGE_UNAFFORDABLE))

/-- Amsterdam state gas charged for each byte of newly deployed code. -/
/- Type quantifiers: byte_len : Nat, 0 ≤ byte_len ∧ byte_len ≤ (2 ^ 32 - 1) -/
def code_deployment_state_cost (byte_len : Nat) : SailM Nat := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if ((profile.fork ≥b Amsterdam) : Bool)
  then
    (do
      if ((byte_len ≤b profile.deployed_code_size_limit) : Bool)
      then (pure (G_amsterdam_state_byte *i byte_len))
      else (fatal_error ExecutionInvalid))
  else (pure GAS_COST_ZERO)

/- Type quantifiers: start : Nat, byte_count : Nat, (source_valid_length start) ∧
  0 ≤ byte_count ∧ byte_count ≤ 32 -/
def pc_word (input : CalldataSlice) (start : Nat) (byte_count : Nat) : SailM Nat := do
  let value : Nat := ZERO_WORD
  let start_offset := start
  let count := byte_count
  let input_length := (calldata_slice_length input)
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
                    (calldata_slice_byte input cursor))
                else (pure 0x00#8))
            else (pure 0x00#8)
          let shifted := (word_shift_left value 8)
          let byte_value := (BitVec.toNatInt next_byte)
          (pure (word_add_word shifted byte_value)))
      else (pure value)
  (pure loop_vars)

/- Type quantifiers: declared_length : Nat, evm_prefix' : Nat, byte_count : Nat, (source_valid_length evm_prefix')
  ∧ 0 ≤ byte_count ∧ byte_count ≤ 32, 0 ≤ declared_length ∧
  declared_length ≤ (2 ^ 256 - 1) -/
def pc_word_after_declared_field (input : CalldataSlice) (evm_prefix' : Nat) (declared_length : Nat) (byte_count : Nat) : SailM Nat := do
  let input_length := (calldata_slice_length input)
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
          (pc_word input start byte_count))
      else (pure ZERO_WORD))
  else (pure ZERO_WORD)

/-- Reads the `BLAKE2F` rounds count: the big-endian 32-bit value in the
input's first four bytes (EIP-152). -/
def pc_blake2_rounds (input : CalldataSlice) : SailM Nat := do
  let rounds_word ← do (pc_word input 0 4)
  (pure (Nat.mod rounds_word (2 ^i 32)))

/-- `MODEXP` (precompile `0x05`) gas: EIP-2565 as amended by EIP-7883
(Osaka). Input layout: `bsize[32] esize[32] msize[32] base exp mod`.
Gas grows with the multiplication complexity of the larger of base
and modulus, and with an iteration count derived from the exponent's
bit length; EIP-7823 (Osaka) caps each field at 1024 bytes, modeled
as an unforwardable cost. Before Osaka the exact expression is below
`2^768`, but an EVM caller can observe only whether that expression fits
its available gas. The staged affordability checks below therefore keep
every materialized intermediate at most 256 bits in optimized builds. -/
/- Type quantifiers: k_ex552331_ : Nat, 0 ≤ k_ex552331_ -/
def modexp_gas (input : CalldataSlice) (available : Nat) : SailM GasCharge := SailME.run do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let bl_word ← do (pc_word input 0 32)
  let el_word ← do (pc_word input 32 32)
  let ml_word ← do (pc_word input 64 32)
  let bl : Nat := bl_word
  let el : Nat := el_word
  let ml : Nat := ml_word
  let osaka := (profile.fork ≥b Osaka)
  if ((osaka && ((bl >b 1024) || ((el >b 1024) || (ml >b 1024)))) : Bool)
  then (pure GAS_CHARGE_UNAFFORDABLE)
  else
    (do
      let minimum : Nat :=
        if (osaka : Bool)
        then 500
        else 200
      if ((available <b minimum) : Bool)
      then (pure GAS_CHARGE_UNAFFORDABLE)
      else
        (do
          let pre_osaka := (! osaka)
          let base_is_zero := (word_is_zero bl_word)
          let modulus_is_zero := (word_is_zero ml_word)
          if ((pre_osaka && (base_is_zero && modulus_is_zero)) : Bool)
          then (pure (gas_charge 200))
          else
            (do
              let maxlen :=
                if ((ml <b bl) : Bool)
                then bl
                else ml
              let whole_words := (maxlen / 8)
              let partial_word_bytes := (Nat.mod maxlen 8)
              let words :=
                (whole_words + (if ((partial_word_bytes == 0) : Bool)
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
              then (pure GAS_CHARGE_UNAFFORDABLE)
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
                  then (pure GAS_CHARGE_UNAFFORDABLE)
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
                                let doubled_limit_value : Nat := (available / 2)
                                let doubled_limit := doubled_limit_value
                                let wide_doubled_limit : Nat := doubled_limit
                                let doubled_unaffordable := (words_squared >b wide_doubled_limit)
                                let bounded_square : Nat :=
                                  if (doubled_unaffordable : Bool)
                                  then 0
                                  else words_squared
                                if (doubled_unaffordable : Bool)
                                then SailME.throw (GAS_CHARGE_UNAFFORDABLE : GasCharge)
                                else (pure (2 *i bounded_square)))
                            else (pure affordable_square)) ) : SailME GasCharge Nat )
                      let mult := mult_value
                      if ((product_limit <b mult) : Bool)
                      then (pure GAS_CHARGE_UNAFFORDABLE)
                      else
                        (do
                          let iterations ← (( do
                            if ((el ≤b 32) : Bool)
                            then
                              (do
                                let exponent_head ← do
                                  (pc_word_after_declared_field input 96 bl el)
                                let exponent_bits := (word_bit_length exponent_head)
                                if ((exponent_bits == 0) : Bool)
                                then (pure 1)
                                else
                                  (let count := (exponent_bits - 1)
                                  if ((count == 0) : Bool)
                                  then (pure 1)
                                  else (pure count)))
                            else
                              (do
                                let exponent_head ← do
                                  (pc_word_after_declared_field input 96 bl 32)
                                let head_bits := (word_bit_length exponent_head)
                                let high_bits :=
                                  if ((head_bits != 0) : Bool)
                                  then (head_bits - 1)
                                  else 0
                                let extra := (el - 32)
                                let count ← (( do
                                  if (osaka : Bool)
                                  then
                                    (do
                                      let extra_limit : Nat := (available / 16)
                                      let maximum_extra := (u256 992)
                                      let exceeds_maximum :=
                                        (word_greater_than_word extra maximum_extra)
                                      let osaka_extra : Nat :=
                                        if (exceeds_maximum : Bool)
                                        then 992
                                        else extra
                                      let extra_unaffordable := (osaka_extra >b extra_limit)
                                      let bounded_extra : Nat :=
                                        if (extra_unaffordable : Bool)
                                        then extra_limit
                                        else osaka_extra
                                      if (extra_unaffordable : Bool)
                                      then SailME.throw (GAS_CHARGE_UNAFFORDABLE : GasCharge)
                                      else (pure ((16 *i bounded_extra) + high_bits)))
                                  else
                                    (do
                                      let pre_osaka_limit := ((3 *i available) + 2)
                                      let extra_limit : Nat := (pre_osaka_limit / 8)
                                      let extra_unaffordable := (extra >b extra_limit)
                                      let bounded_extra : Nat :=
                                        if (extra_unaffordable : Bool)
                                        then extra_limit
                                        else extra
                                      if (extra_unaffordable : Bool)
                                      then SailME.throw (GAS_CHARGE_UNAFFORDABLE : GasCharge)
                                      else (pure ((8 *i bounded_extra) + high_bits))) ) : SailME
                                  GasCharge Nat )
                                if ((count == 0) : Bool)
                                then (pure 1)
                                else (pure count)) ) : SailME GasCharge Nat )
                          if ((iterations == 0) : Bool)
                          then (pure GAS_CHARGE_UNAFFORDABLE)
                          else
                            (let product : Nat := (mult *i iterations)
                            let product_unaffordable := (wide_product_limit <b product)
                            let affordable_product : Nat :=
                              if (product_unaffordable : Bool)
                              then product_limit
                              else product
                            if (product_unaffordable : Bool)
                            then (pure GAS_CHARGE_UNAFFORDABLE)
                            else
                              (let calculated :=
                                if (osaka : Bool)
                                then affordable_product
                                else (affordable_product / 3)
                              let cost :=
                                if ((calculated <b minimum) : Bool)
                                then minimum
                                else calculated
                              if ((cost ≤b available) : Bool)
                              then
                                (let affordable : Nat := cost
                                (pure (gas_charge affordable)))
                              else (pure GAS_CHARGE_UNAFFORDABLE)))))))))

/-- The EIP-2537 BLS12-381 G1 MSM discount table: entry `k−1` is the
per-pair discount (in thousandths) applied to a `k`-pair MSM,
constant for `k ≥ 128`. -/
def BLS_G1_DISCOUNT : (Vector bls_discount 128) :=
  #v[519, 520, 520, 521, 522, 522, 523, 524, 525, 525, 526, 527, 528, 528, 529, 530, 531, 532, 532, 533, 534, 535, 536, 536, 537, 538, 539, 540, 540, 541, 542, 543, 544, 545, 546, 547, 547, 548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563, 564, 565, 566, 567, 568, 569, 570, 572, 573, 574, 575, 576, 577, 579, 580, 581, 582, 584, 585, 586, 588, 589, 591, 592, 593, 595, 596, 598, 599, 601, 603, 604, 606, 608, 609, 611, 613, 615, 617, 619, 621, 623, 625, 627, 630, 632, 635, 637, 640, 642, 645, 648, 651, 654, 658, 661, 665, 669, 673, 677, 682, 687, 692, 698, 705, 712, 719, 728, 738, 750, 764, 797, 848, 949, 1000]

def BLS_G2_DISCOUNT : (Vector bls_discount 128) :=
  #v[524, 524, 525, 526, 526, 527, 528, 528, 529, 530, 530, 531, 532, 532, 533, 534, 535, 535, 536, 537, 537, 538, 539, 540, 541, 541, 542, 543, 544, 545, 545, 546, 547, 548, 549, 550, 551, 552, 552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563, 565, 566, 567, 568, 569, 570, 571, 573, 574, 575, 576, 578, 579, 580, 582, 583, 584, 586, 587, 589, 590, 592, 593, 595, 597, 598, 600, 602, 604, 606, 607, 609, 611, 613, 615, 618, 620, 622, 624, 627, 629, 632, 634, 637, 640, 643, 646, 649, 652, 655, 659, 663, 666, 670, 674, 679, 683, 688, 693, 699, 704, 711, 717, 724, 732, 740, 749, 759, 770, 782, 796, 812, 832, 855, 884, 923, 1000, 1000]

/-- EIP-2537 MSM gas: `(k · base · discount(k)) / 1000`, with the
discount clamped to the `k = 128` entry beyond the table. -/
/- Type quantifiers: k_ex552591_ : Nat, k_ex552590_ : Nat, k_ex552589_ : Nat, k_ex552588_ : Nat, 0
  ≤ k_ex552588_ ∧ k_ex552588_ ≤ 45000, 0 ≤ k_ex552589_ ∧ k_ex552589_ ≤ (2 ^ 16 - 1), 0
  ≤ k_ex552590_ ∧ k_ex552590_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex552591_ -/
def bls_msm_gas (table : (Vector Nat 128)) (base : Nat) (maxd : Nat) (k : Nat) (available : Nat) : GasCharge :=
  if ((k == 0) : Bool)
  then (gas_charge GAS_COST_ZERO)
  else
    (let discount : Nat := maxd
    let discount : Nat :=
      if ((k <b 128) : Bool)
      then
        (let index : Nat := (128 - k)
        (GetElem?.getElem! table index))
      else discount
    let coefficient : Nat := (base *i discount)
    let whole := (k / 1000)
    let remainder := (Nat.mod k 1000)
    let whole_product : Nat := (coefficient *i whole)
    if ((whole_product >b available) : Bool)
    then GAS_CHARGE_UNAFFORDABLE
    else
      (let major : Nat := whole_product
      let tail_product : Nat := (coefficient *i remainder)
      let tail : Nat := (tail_product / 1000)
      let total : Nat := (major + tail)
      if ((total ≤b available) : Bool)
      then
        (let affordable : Nat := total
        (gas_charge affordable))
      else GAS_CHARGE_UNAFFORDABLE))

/-- Returns a linear precompile cost only after the caller can afford its
transaction-controlled multiplier. -/
/- Type quantifiers: k_ex552595_ : Nat, k_ex552594_ : Nat, k_ex552593_ : Nat, k_ex552592_ : Nat, 0
  ≤ k_ex552592_ ∧ k_ex552592_ ≤ 45000, 0 ≤ k_ex552593_ ∧ k_ex552593_ ≤ 45000, 0 ≤
  k_ex552594_ ∧ k_ex552594_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex552595_ -/
def linear_gas (base : Nat) (per_unit : Nat) (units : Nat) (available : Nat) : GasCharge :=
  let variable_cost : Nat := (per_unit *i units)
  let exact_cost : Nat := (variable_cost + base)
  if ((exact_cost >b available) : Bool)
  then GAS_CHARGE_UNAFFORDABLE
  else
    (let affordable : Nat := exact_cost
    (gas_charge affordable))

/- Type quantifiers: available : Nat, cost : Nat, 0 ≤ cost, 0 ≤ available -/
def fixed_precompile_gas (cost : Nat) (available : Nat) : GasCharge :=
  if ((cost ≤b available) : Bool)
  then
    (let affordable : Nat := cost
    (gas_charge affordable))
  else GAS_CHARGE_UNAFFORDABLE

/-- The gas of the precompile at address `num` for a given input. Gas is
protocol policy defined entirely here; implementations return only
output. Length-only costs
derive from the word count; the two input-dependent curves (`MODEXP`,
`BLAKE2F` rounds) read the input in place. The match arms are the
precompile catalog with their addresses and pricing EIPs. -/
/- Type quantifiers: k_ex552597_ : Nat, 0 ≤ k_ex552597_ -/
def precompile_gas (num : PrecompileId) (input : CalldataSlice) (available : Nat) : SailM GasCharge := do
  let input_len := (calldata_slice_length input)
  let input_length := input_len
  let words := (memory_word_count input_len)
  match num with
  | .NotPrecompile => (pure GAS_CHARGE_UNAFFORDABLE)
  | .Ecrecover => (pure (fixed_precompile_gas 3000 available))
  | .Sha256 => (pure (linear_gas 60 12 words available))
  | .Ripemd160 => (pure (linear_gas 600 120 words available))
  | .Identity => (pure (linear_gas 15 3 words available))
  | .Modexp => (modexp_gas input available)
  | .Bn254Add => (pure (fixed_precompile_gas 150 available))
  | .Bn254Mul => (pure (fixed_precompile_gas 6000 available))
  | .Bn254Pairing => (pure (linear_gas 45000 34000 (input_length / 192) available))
  | .Blake2f =>
    (do
      let rounds ← do (pc_blake2_rounds input)
      (pure (fixed_precompile_gas rounds available)))
  | .KzgPointEvaluation => (pure (fixed_precompile_gas 50000 available))
  | .BlsG1Add => (pure (fixed_precompile_gas 375 available))
  | .BlsG1Msm => (pure (bls_msm_gas BLS_G1_DISCOUNT 12000 519 (input_length / 160) available))
  | .BlsG2Add => (pure (fixed_precompile_gas 600 available))
  | .BlsG2Msm => (pure (bls_msm_gas BLS_G2_DISCOUNT 22500 524 (input_length / 288) available))
  | .BlsPairing => (pure (linear_gas 37700 32600 (input_length / 384) available))
  | .BlsMapFpToG1 => (pure (fixed_precompile_gas 5500 available))
  | .BlsMapFp2ToG2 => (pure (fixed_precompile_gas 23800 available))
  | .P256Verify => (pure (fixed_precompile_gas 6900 available))

def undefined_SstoreCosts (_ : Unit) : SailM SstoreCosts := do
  (pure { execution := ← (undefined_nat ()),
          refund := ← (undefined_range (Neg.neg (199 *i ((2 ^i 64) - 1))) (199 *i ((2 ^i 64) - 1))),
          state_charge := ← (undefined_nat ()),
          state_credit := ← (undefined_range 0 (2 ^i 24)) })

/-- Returns the Amsterdam execution-gas cost of accessing a storage slot. -/
/- Type quantifiers: k_ex552598_ : Bool -/
def amsterdam_storage_access_cost (cold : Bool) : Nat :=
  if (cold : Bool)
  then G_amsterdam_cold_storage_access
  else G_warm_access

/-- Minimum execution gas required before an Amsterdam `SSTORE` may inspect
or mutate authenticated state. -/
/- Type quantifiers: k_ex552599_ : Bool -/
def sstore_sentry_cost (cold : Bool) : Nat :=
  let access_cost := (amsterdam_storage_access_cost cold)
  if ((access_cost <b G_sstore_sentry) : Bool)
  then G_sstore_sentry
  else access_cost

/-- Legacy `SSTORE` pricing: a three-way comparison (EIP-2200) of
*original* (the slot's transaction-start value), *current*, and *new*.
Writing the same value, or dirtying an already-dirty slot, costs warm
access only; a clean slot going zero↔nonzero pays `G_sset` /
`G_sreset`. Refunds track clearing/un-clearing and restoring the original
value. The EIP-2929 cold surcharge is added when the slot was not warm. -/
/- Type quantifiers: k_ex552603_ : Bool, k_ex552602_ : Nat, k_ex552601_ : Nat, k_ex552600_ : Nat, 0
  ≤ k_ex552600_ ∧ k_ex552600_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex552601_ ∧
  k_ex552601_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex552602_ ∧ k_ex552602_ ≤ (2 ^ 256 - 1) -/
def legacy_sstore_costs (original : Nat) (current : Nat) (new_value : Nat) (cold : Bool) : SailM SstoreCosts := do
  let cold_cost : Nat :=
    if (cold : Bool)
    then G_cold_sload
    else GAS_CONSTANT_ZERO
  let clear_refund ← (( do (sstore_clear_refund ()) ) : SailM Nat )
  let clean_change := ((current != new_value) && (original == current))
  let original_is_zero := (word_is_zero original)
  let current_is_zero := (word_is_zero current)
  let new_value_is_zero := (word_is_zero new_value)
  let original_is_nonzero := (! original_is_zero)
  let base : Nat :=
    if (clean_change : Bool)
    then
      (if (original_is_zero : Bool)
      then G_sset
      else G_sreset)
    else G_warm_access
  let refund : Int :=
    if ((current == new_value) : Bool)
    then 0
    else
      (if ((original == current) : Bool)
      then
        (if ((original_is_nonzero && new_value_is_zero) : Bool)
        then clear_refund
        else 0)
      else
        (let clear_delta :=
          if (original_is_zero : Bool)
          then 0
          else
            ((if (current_is_zero : Bool)
              then (0 -i clear_refund)
              else 0) +i (if (new_value_is_zero : Bool)
              then clear_refund
              else 0))
        let reset_delta :=
          if ((original == new_value) : Bool)
          then
            (if (original_is_zero : Bool)
            then (G_sset -i G_warm_access)
            else (G_sreset -i G_warm_access))
          else 0
        (clear_delta +i reset_delta)))
  (pure { execution := (base + cold_cost),
          refund := refund,
          state_charge := 0,
          state_credit := 0 })

/-- Amsterdam `SSTORE` pricing (EIP-8037). Access and first-write work debit
execution gas. Introducing a new non-zero slot debits state gas; restoring
that slot to its transaction-start zero value returns the state charge. -/
/- Type quantifiers: k_ex552607_ : Bool, k_ex552606_ : Nat, k_ex552605_ : Nat, k_ex552604_ : Nat, 0
  ≤ k_ex552604_ ∧ k_ex552604_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex552605_ ∧
  k_ex552605_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex552606_ ∧ k_ex552606_ ≤ (2 ^ 256 - 1) -/
def amsterdam_sstore_costs (original : Nat) (current : Nat) (new_value : Nat) (cold : Bool) : SstoreCosts :=
  let changed := (current != new_value)
  let clean_change := ((original == current) && changed)
  let access := (amsterdam_storage_access_cost cold)
  let original_is_zero := (word_is_zero original)
  let current_is_zero := (word_is_zero current)
  let new_value_is_zero := (word_is_zero new_value)
  let original_is_nonzero := (! original_is_zero)
  let current_is_nonzero := (! current_is_zero)
  let execution : Nat :=
    if (clean_change : Bool)
    then (access + G_amsterdam_storage_write)
    else access
  let clear_delta :=
    if ((changed && (original_is_nonzero && (current_is_nonzero && new_value_is_zero))) : Bool)
    then R_amsterdam_storage_clear
    else
      (if ((changed && (original_is_nonzero && current_is_zero)) : Bool)
      then (0 -i R_amsterdam_storage_clear)
      else 0)
  let restore_delta :=
    if ((changed && (original == new_value)) : Bool)
    then G_amsterdam_storage_write
    else 0
  let refund : Int := (clear_delta +i restore_delta)
  let state_charge : Nat :=
    if ((clean_change && original_is_zero) : Bool)
    then G_amsterdam_state_storage_set
    else GAS_COST_ZERO
  let state_credit : Nat := 0
  let state_credit : Nat :=
    if ((changed && ((original == new_value) && original_is_zero)) : Bool)
    then
      (let state_credit : Nat := G_amsterdam_state_storage_set
      state_credit)
    else state_credit
  { execution := execution,
    refund := refund,
    state_charge := state_charge,
    state_credit := state_credit }

/-- Computes the fork-specific effects of one `SSTORE`. The refund delta is
accumulated and capped at transaction settlement, not here. -/
/- Type quantifiers: k_ex552611_ : Bool, k_ex552610_ : Nat, k_ex552609_ : Nat, k_ex552608_ : Nat, 0
  ≤ k_ex552608_ ∧ k_ex552608_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex552609_ ∧
  k_ex552609_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex552610_ ∧ k_ex552610_ ≤ (2 ^ 256 - 1) -/
def sstore_costs (original : Nat) (current : Nat) (new_value : Nat) (cold : Bool) : SailM SstoreCosts := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if ((profile.fork ≥b Amsterdam) : Bool)
  then (pure (amsterdam_sstore_costs original current new_value cold))
  else (legacy_sstore_costs original current new_value cold)

/-- Charges a word-sized unit count only after proving the product affordable,
so optimized builds never materialize an overflowing native gas cost. -/
/- Type quantifiers: k_ex552614_ : Nat, k_ex552613_ : Nat, k_ex552612_ : Nat, 0 ≤ k_ex552612_, 0
  ≤ k_ex552613_ ∧ k_ex552613_ ≤ 45000, 0 ≤ k_ex552614_ ∧ k_ex552614_ ≤ (2 ^ 256 - 1) -/
def charge_word_scaled_gas (g : Nat) (per_unit : Nat) (units : Nat) : SailM (Bool × Nat) := do
  if (((per_unit == 0) || (units == 0)) : Bool)
  then (pure (true, g))
  else
    (do
      if ((units ≤b g) : Bool)
      then
        (do
          let affordable_units : Nat := units
          let exact_cost : Nat := (per_unit *i affordable_units)
          if ((exact_cost ≤b g) : Bool)
          then
            (do
              let cost : Nat := exact_cost
              (charge g cost))
          else (pure (false, (← (exc_halt g OutOfGas)))))
      else (pure (false, (← (exc_halt g OutOfGas)))))

/-- Charges an opcode base cost and its per-memory-word component. -/
/- Type quantifiers: k_ex552618_ : Nat, k_ex552617_ : Nat, k_ex552616_ : Nat, k_ex552615_ : Nat, 0
  ≤ k_ex552615_, 0 ≤ k_ex552616_ ∧ k_ex552616_ ≤ 45000, 0 ≤ k_ex552617_ ∧
  k_ex552617_ ≤ 45000, 0 ≤ k_ex552618_ ∧ k_ex552618_ ≤ (2 ^ 256 - 1) -/
def charge_memory_word_gas (g : Nat) (base : Nat) (per_word : Nat) (size : Nat) : SailM (Bool × Nat) := do
  let (base_charged, base_gas) ← do (charge g base)
  if (base_charged : Bool)
  then
    (do
      let words := (memory_word_count_word size)
      (charge_word_scaled_gas base_gas per_word words))
  else (pure (false, base_gas))

/- Type quantifiers: k_ex552620_ : Nat, k_ex552619_ : Nat, 0 ≤ k_ex552619_, 0 ≤ k_ex552620_ ∧
  k_ex552620_ ≤ (2 ^ 256 - 1) -/
def charge_keccak_gas (g : Nat) (size : Nat) : SailM (Bool × Nat) := do
  (charge_memory_word_gas g G_keccak G_keccak_word size)

/- Type quantifiers: k_ex552622_ : Nat, k_ex552621_ : Nat, 0 ≤ k_ex552621_, 0 ≤ k_ex552622_ ∧
  k_ex552622_ ≤ (2 ^ 256 - 1) -/
def charge_copy_gas (g : Nat) (size : Nat) : SailM (Bool × Nat) := do
  (charge_memory_word_gas g GAS_CONSTANT_ZERO G_copy_word size)

/-- Charges the base, topic, and data-byte components of a log operation. -/
/- Type quantifiers: k_ex552625_ : Nat, k_ex552624_ : Nat, k_ex552623_ : Nat, 0 ≤ k_ex552623_, 0
  ≤ k_ex552624_ ∧ k_ex552624_ ≤ 4, 0 ≤ k_ex552625_ ∧ k_ex552625_ ≤ (2 ^ 256 - 1) -/
def charge_log_gas (g : Nat) (num_topics : Nat) (size : Nat) : SailM (Bool × Nat) := do
  let (base_charged, base_gas) ← do (charge g G_log)
  if (base_charged : Bool)
  then
    (do
      let topic_cost : Nat := (G_logtopic *i num_topics)
      let (topics_charged, topics_gas) ← do (charge base_gas topic_cost)
      if (topics_charged : Bool)
      then (charge_word_scaled_gas topics_gas G_logdata size)
      else (pure (false, topics_gas)))
  else (pure (false, base_gas))

/-- `EXP`: base plus `G_expbyte` per significant exponent byte
(EIP-160). -/
/- Type quantifiers: exponent : Nat, 0 ≤ exponent ∧ exponent ≤ (2 ^ 256 - 1) -/
def exp_gas (exponent : Nat) : Nat :=
  ((G_expbyte *i (word_byte_length exponent)) + G_exp)

/-- The EIP-3860 charge for transaction-backed initcode, retaining the SSZ
transaction-envelope ceiling for native cost aggregation. -/
/- Type quantifiers: byte_len : Nat, 0 ≤ byte_len ∧ byte_len ≤ (2 ^ 30) -/
def transaction_initcode_gas (byte_len : Nat) : SailM Nat := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if ((profile.fork ≥b Shanghai) : Bool)
  then
    (let words := (memory_word_count byte_len)
    (pure (words + words)))
  else (pure 0)

/-- Applies the EIP-150 forwarding cap to a word-sized gas request. -/
/- Type quantifiers: k_ex552629_ : Nat, k_ex552628_ : Nat, 0 ≤ k_ex552628_, 0 ≤ k_ex552629_ ∧
  k_ex552629_ ≤ (2 ^ 256 - 1) -/
def call_gas_cap_word (available : Nat) (requested : Nat) : Nat :=
  let retained : Nat := (available / 64)
  let all_but_64th : Nat := (gas_sub available retained)
  if ((requested <b all_but_64th) : Bool)
  then requested
  else all_but_64th

