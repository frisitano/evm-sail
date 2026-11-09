import Evm.Primitives.Gas
import Evm.Primitives.Fork

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
open StackValidation
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
open OpcodeOutcome
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

/-! # Chain configuration

The protocol profile the guest executes under: Amsterdam with its blob
schedule, fixed by the model. The stateless input's schema id admits only this
profile and its SSZ body carries just the chain id. Pure data — no registers,
no externs. -/

/- Type quantifiers: target : Nat, maximum : Nat, denominator : Nat, (blob_schedule_parameters target maximum denominator) -/
def blob_schedule (target : Nat) (maximum : Nat) (denominator : Nat) : (BlobScheduleFields target maximum denominator) :=
  {  }

/- Type quantifiers: fork : Nat, target : Nat, maximum : Nat, denominator : Nat, 0 ≤ fork ∧
  fork ≤ amsterdam_fork_value ∧ (blob_schedule_parameters target maximum denominator) -/
def compute_profile_excess_blob_gas_limit (fork : Nat) (target : Nat) (maximum : Nat) (denominator : Nat) : Nat :=
  if ((fork <b Cancun) : Bool)
  then 0
  else ((256 *i denominator) + ((maximum - target) *i (2 ^i 17)))

/- Type quantifiers: k_fork : Nat, k_target : Nat, k_maximum : Nat, k_denominator : Nat, k_code_limit
  : Nat, k_initcode_limit : Nat, k_transaction_total_gas_limit : Nat, k_transaction_regular_gas_limit
  : Nat, k_transaction_blob_limit : Nat, k_refund_divisor : Nat, (protocol_profile_parameters k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor) -/
def pack_protocol_profile (profile : (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)) : (Sigma
  fun (k_syn_fork : Nat) =>
  (Sigma fun (k_syn_target : Nat) =>
  (Sigma fun (k_syn_maximum : Nat) =>
  (Sigma fun (k_syn_denominator : Nat) =>
  (Sigma fun (k_syn_code_limit : Nat) =>
  (Sigma fun (k_syn_initcode_limit : Nat) =>
  (Sigma fun (k_syn_transaction_total_gas_limit : Nat) =>
  (Sigma fun (k_syn_transaction_regular_gas_limit : Nat) =>
  (Sigma fun (k_syn_transaction_blob_limit : Nat) =>
  (Sigma fun (k_syn_refund_divisor : Nat) =>
  (ProtocolProfileFields k_syn_fork k_syn_target k_syn_maximum k_syn_denominator k_syn_code_limit k_syn_initcode_limit k_syn_transaction_total_gas_limit k_syn_transaction_regular_gas_limit k_syn_transaction_blob_limit k_syn_refund_divisor))))))))))) :=
  ((⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ : (Sigma
  fun (k_syn_fork : Nat) =>
  (Sigma fun (k_syn_target : Nat) =>
  (Sigma fun (k_syn_maximum : Nat) =>
  (Sigma fun (k_syn_denominator : Nat) =>
  (Sigma fun (k_syn_code_limit : Nat) =>
  (Sigma fun (k_syn_initcode_limit : Nat) =>
  (Sigma fun (k_syn_transaction_total_gas_limit : Nat) =>
  (Sigma fun (k_syn_transaction_regular_gas_limit : Nat) =>
  (Sigma fun (k_syn_transaction_blob_limit : Nat) =>
  (Sigma fun (k_syn_refund_divisor : Nat) =>
  (ProtocolProfileFields k_syn_fork k_syn_target k_syn_maximum k_syn_denominator k_syn_code_limit k_syn_initcode_limit k_syn_transaction_total_gas_limit k_syn_transaction_regular_gas_limit k_syn_transaction_blob_limit k_syn_refund_divisor)))))))))))) : (Sigma
  fun (k_syn_fork : Nat) =>
  (Sigma fun (k_syn_target : Nat) =>
  (Sigma fun (k_syn_maximum : Nat) =>
  (Sigma fun (k_syn_denominator : Nat) =>
  (Sigma fun (k_syn_code_limit : Nat) =>
  (Sigma fun (k_syn_initcode_limit : Nat) =>
  (Sigma fun (k_syn_transaction_total_gas_limit : Nat) =>
  (Sigma fun (k_syn_transaction_regular_gas_limit : Nat) =>
  (Sigma fun (k_syn_transaction_blob_limit : Nat) =>
  (Sigma fun (k_syn_refund_divisor : Nat) =>
  (ProtocolProfileFields k_syn_fork k_syn_target k_syn_maximum k_syn_denominator k_syn_code_limit k_syn_initcode_limit k_syn_transaction_total_gas_limit k_syn_transaction_regular_gas_limit k_syn_transaction_blob_limit k_syn_refund_divisor))))))))))))

/- Type quantifiers: k_fork : Nat, k_target : Nat, k_maximum : Nat, k_denominator : Nat, k_code_limit
  : Nat, k_initcode_limit : Nat, k_profile_total_limit : Nat, k_profile_regular_limit : Nat, k_transaction_blob_limit
  : Nat, k_refund_divisor : Nat, block_limit : Nat, (protocol_profile_parameters k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_profile_total_limit k_profile_regular_limit k_transaction_blob_limit k_refund_divisor)
  ∧ 0 ≤ block_limit ∧ block_limit ≤ block_gas_limit_bound -/
def gas_limits_for (profile : (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_profile_total_limit k_profile_regular_limit k_transaction_blob_limit k_refund_divisor)) (block_limit : Nat) : (GasLimitsFields block_limit k_profile_total_limit k_profile_regular_limit (if ( block_limit
  < k_profile_total_limit  : Bool) then block_limit else k_profile_total_limit) (if ( (if ( block_limit
  < k_profile_total_limit  : Bool) then block_limit else k_profile_total_limit) <
  k_profile_regular_limit  : Bool) then (if ( block_limit < k_profile_total_limit  : Bool) then block_limit else k_profile_total_limit) else k_profile_regular_limit)) :=
  let transaction_total_limit :=
    if ((block_limit <b k_profile_total_limit) : Bool)
    then block_limit
    else k_profile_total_limit
  let transaction_regular_limit :=
    if ((transaction_total_limit <b k_profile_regular_limit) : Bool)
    then transaction_total_limit
    else k_profile_regular_limit
  {  }

/- Type quantifiers: k_fork : Nat, k_target : Nat, k_maximum : Nat, k_denominator : Nat, k_code_limit
  : Nat, k_initcode_limit : Nat, k_profile_total_limit : Nat, k_profile_regular_limit : Nat, k_transaction_blob_limit
  : Nat, k_refund_divisor : Nat, block_limit : Nat, (protocol_profile_parameters k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_profile_total_limit k_profile_regular_limit k_transaction_blob_limit k_refund_divisor)
  ∧ 0 ≤ block_limit ∧ block_limit ≤ block_gas_limit_bound -/
def execution_profile_for (protocol : (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_profile_total_limit k_profile_regular_limit k_transaction_blob_limit k_refund_divisor)) (block_limit : Nat) : (ExecutionProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_profile_total_limit k_profile_regular_limit k_transaction_blob_limit k_refund_divisor block_limit (if ( block_limit
  < k_profile_total_limit  : Bool) then block_limit else k_profile_total_limit) (if ( (if ( block_limit
  < k_profile_total_limit  : Bool) then block_limit else k_profile_total_limit) <
  k_profile_regular_limit  : Bool) then (if ( block_limit < k_profile_total_limit  : Bool) then block_limit else k_profile_total_limit) else k_profile_regular_limit)) :=
  { protocol := protocol,
    gas := (gas_limits_for protocol block_limit) }

/-- The Amsterdam protocol profile: the sole schema the guest admits. It also
initializes the kernel before input decoding. -/
def DEFAULT_PROTOCOL_PROFILE : ProtocolProfile :=
  let profile :=
    ({ blob_schedule := (blob_schedule 14 21 11684671) } : (ProtocolProfileFields 16 14 21 11684671 65536 131072 (2 ^ 64 - 1) (2 ^ 24) 6 5))
  (pack_protocol_profile profile)

def DEFAULT_EXECUTION_PROFILE : ExecutionProfile :=
  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, (execution_profile_for
    ((((((((((DEFAULT_PROTOCOL_PROFILE).2).2).2).2).2).2).2).2).2).2 0)⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩
