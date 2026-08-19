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

The decoded `SszChainConfig`. Its activation point is validated while decoding,
so the resulting value only carries the chain id. The schema evm_prefix selects the
protocol profile, so neither the fork identifier nor its blob schedule is
repeated inside the SSZ body. Pure data — no registers, no externs. -/

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

/-- The schema's stable fork byte selects one complete protocol profile. The
branches are the sole table of admitted schema/profile combinations. -/
def schema_protocol_profile (schema_fork : (BitVec 8)) : (Sigma fun (k_fork : Nat) =>
  (Sigma fun (k_target : Nat) =>
  (Sigma fun (k_maximum : Nat) =>
  (Sigma fun (k_denominator : Nat) =>
  (Sigma fun (k_code_limit : Nat) =>
  (Sigma fun (k_initcode_limit : Nat) =>
  (Sigma fun (k_transaction_total_gas_limit : Nat) =>
  (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
  (Sigma fun (k_transaction_blob_limit : Nat) =>
  (Sigma fun (k_refund_divisor : Nat) =>
  (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor))))))))))) :=
  match schema_fork with
  | 0x0A =>
    (let profile :=
      ({ blob_schedule := (blob_schedule 0 0 1) } : (ProtocolProfileFields 5 0 0 1 24576 0 (2 ^ 64 - 1) (2 ^ 64 - 1) 0 2))
    ((pack_protocol_profile profile) : (Sigma fun (k_fork : Nat) =>
    (Sigma fun (k_target : Nat) =>
    (Sigma fun (k_maximum : Nat) =>
    (Sigma fun (k_denominator : Nat) =>
    (Sigma fun (k_code_limit : Nat) =>
    (Sigma fun (k_initcode_limit : Nat) =>
    (Sigma fun (k_transaction_total_gas_limit : Nat) =>
    (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
    (Sigma fun (k_transaction_blob_limit : Nat) =>
    (Sigma fun (k_refund_divisor : Nat) =>
    (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))))
  | 0x0B =>
    (let profile :=
      ({ blob_schedule := (blob_schedule 0 0 1) } : (ProtocolProfileFields 6 0 0 1 24576 0 (2 ^ 64 - 1) (2 ^ 64 - 1) 0 5))
    ((pack_protocol_profile profile) : (Sigma fun (k_fork : Nat) =>
    (Sigma fun (k_target : Nat) =>
    (Sigma fun (k_maximum : Nat) =>
    (Sigma fun (k_denominator : Nat) =>
    (Sigma fun (k_code_limit : Nat) =>
    (Sigma fun (k_initcode_limit : Nat) =>
    (Sigma fun (k_transaction_total_gas_limit : Nat) =>
    (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
    (Sigma fun (k_transaction_blob_limit : Nat) =>
    (Sigma fun (k_refund_divisor : Nat) =>
    (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))))
  | 0x0C =>
    (let profile :=
      ({ blob_schedule := (blob_schedule 0 0 1) } : (ProtocolProfileFields 7 0 0 1 24576 0 (2 ^ 64 - 1) (2 ^ 64 - 1) 0 5))
    ((pack_protocol_profile profile) : (Sigma fun (k_fork : Nat) =>
    (Sigma fun (k_target : Nat) =>
    (Sigma fun (k_maximum : Nat) =>
    (Sigma fun (k_denominator : Nat) =>
    (Sigma fun (k_code_limit : Nat) =>
    (Sigma fun (k_initcode_limit : Nat) =>
    (Sigma fun (k_transaction_total_gas_limit : Nat) =>
    (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
    (Sigma fun (k_transaction_blob_limit : Nat) =>
    (Sigma fun (k_refund_divisor : Nat) =>
    (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))))
  | 0x0D =>
    (let profile :=
      ({ blob_schedule := (blob_schedule 0 0 1) } : (ProtocolProfileFields 8 0 0 1 24576 0 (2 ^ 64 - 1) (2 ^ 64 - 1) 0 5))
    ((pack_protocol_profile profile) : (Sigma fun (k_fork : Nat) =>
    (Sigma fun (k_target : Nat) =>
    (Sigma fun (k_maximum : Nat) =>
    (Sigma fun (k_denominator : Nat) =>
    (Sigma fun (k_code_limit : Nat) =>
    (Sigma fun (k_initcode_limit : Nat) =>
    (Sigma fun (k_transaction_total_gas_limit : Nat) =>
    (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
    (Sigma fun (k_transaction_blob_limit : Nat) =>
    (Sigma fun (k_refund_divisor : Nat) =>
    (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))))
  | 0x0E =>
    (let profile :=
      ({ blob_schedule := (blob_schedule 0 0 1) } : (ProtocolProfileFields 9 0 0 1 24576 0 (2 ^ 64 - 1) (2 ^ 64 - 1) 0 5))
    ((pack_protocol_profile profile) : (Sigma fun (k_fork : Nat) =>
    (Sigma fun (k_target : Nat) =>
    (Sigma fun (k_maximum : Nat) =>
    (Sigma fun (k_denominator : Nat) =>
    (Sigma fun (k_code_limit : Nat) =>
    (Sigma fun (k_initcode_limit : Nat) =>
    (Sigma fun (k_transaction_total_gas_limit : Nat) =>
    (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
    (Sigma fun (k_transaction_blob_limit : Nat) =>
    (Sigma fun (k_refund_divisor : Nat) =>
    (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))))
  | 0x0F =>
    (let profile :=
      ({ blob_schedule := (blob_schedule 0 0 1) } : (ProtocolProfileFields 10 0 0 1 24576 49152 (2 ^ 64 - 1) (2 ^ 64 - 1) 0 5))
    ((pack_protocol_profile profile) : (Sigma fun (k_fork : Nat) =>
    (Sigma fun (k_target : Nat) =>
    (Sigma fun (k_maximum : Nat) =>
    (Sigma fun (k_denominator : Nat) =>
    (Sigma fun (k_code_limit : Nat) =>
    (Sigma fun (k_initcode_limit : Nat) =>
    (Sigma fun (k_transaction_total_gas_limit : Nat) =>
    (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
    (Sigma fun (k_transaction_blob_limit : Nat) =>
    (Sigma fun (k_refund_divisor : Nat) =>
    (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))))
  | 0x10 =>
    (let profile :=
      ({ blob_schedule := (blob_schedule 3 6 3338477) } : (ProtocolProfileFields 11 3 6 3338477 24576 49152 (2 ^ 64 - 1) (2 ^ 64 - 1) 6 5))
    ((pack_protocol_profile profile) : (Sigma fun (k_fork : Nat) =>
    (Sigma fun (k_target : Nat) =>
    (Sigma fun (k_maximum : Nat) =>
    (Sigma fun (k_denominator : Nat) =>
    (Sigma fun (k_code_limit : Nat) =>
    (Sigma fun (k_initcode_limit : Nat) =>
    (Sigma fun (k_transaction_total_gas_limit : Nat) =>
    (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
    (Sigma fun (k_transaction_blob_limit : Nat) =>
    (Sigma fun (k_refund_divisor : Nat) =>
    (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))))
  | 0x11 =>
    (let profile :=
      ({ blob_schedule := (blob_schedule 6 9 5007716) } : (ProtocolProfileFields 12 6 9 5007716 24576 49152 (2 ^ 64 - 1) (2 ^ 64 - 1) 9 5))
    ((pack_protocol_profile profile) : (Sigma fun (k_fork : Nat) =>
    (Sigma fun (k_target : Nat) =>
    (Sigma fun (k_maximum : Nat) =>
    (Sigma fun (k_denominator : Nat) =>
    (Sigma fun (k_code_limit : Nat) =>
    (Sigma fun (k_initcode_limit : Nat) =>
    (Sigma fun (k_transaction_total_gas_limit : Nat) =>
    (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
    (Sigma fun (k_transaction_blob_limit : Nat) =>
    (Sigma fun (k_refund_divisor : Nat) =>
    (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))))
  | 0x12 =>
    (let profile :=
      ({ blob_schedule := (blob_schedule 6 9 5007716) } : (ProtocolProfileFields 13 6 9 5007716 24576 49152 (2 ^ 24) (2 ^ 24) 6 5))
    ((pack_protocol_profile profile) : (Sigma fun (k_fork : Nat) =>
    (Sigma fun (k_target : Nat) =>
    (Sigma fun (k_maximum : Nat) =>
    (Sigma fun (k_denominator : Nat) =>
    (Sigma fun (k_code_limit : Nat) =>
    (Sigma fun (k_initcode_limit : Nat) =>
    (Sigma fun (k_transaction_total_gas_limit : Nat) =>
    (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
    (Sigma fun (k_transaction_blob_limit : Nat) =>
    (Sigma fun (k_refund_divisor : Nat) =>
    (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))))
  | 0x13 =>
    (let profile :=
      ({ blob_schedule := (blob_schedule 10 15 8346193) } : (ProtocolProfileFields 14 10 15 8346193 24576 49152 (2 ^ 24) (2 ^ 24) 6 5))
    ((pack_protocol_profile profile) : (Sigma fun (k_fork : Nat) =>
    (Sigma fun (k_target : Nat) =>
    (Sigma fun (k_maximum : Nat) =>
    (Sigma fun (k_denominator : Nat) =>
    (Sigma fun (k_code_limit : Nat) =>
    (Sigma fun (k_initcode_limit : Nat) =>
    (Sigma fun (k_transaction_total_gas_limit : Nat) =>
    (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
    (Sigma fun (k_transaction_blob_limit : Nat) =>
    (Sigma fun (k_refund_divisor : Nat) =>
    (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))))
  | 0x14 =>
    (let profile :=
      ({ blob_schedule := (blob_schedule 14 21 11684671) } : (ProtocolProfileFields 15 14 21 11684671 24576 49152 (2 ^ 24) (2 ^ 24) 6 5))
    ((pack_protocol_profile profile) : (Sigma fun (k_fork : Nat) =>
    (Sigma fun (k_target : Nat) =>
    (Sigma fun (k_maximum : Nat) =>
    (Sigma fun (k_denominator : Nat) =>
    (Sigma fun (k_code_limit : Nat) =>
    (Sigma fun (k_initcode_limit : Nat) =>
    (Sigma fun (k_transaction_total_gas_limit : Nat) =>
    (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
    (Sigma fun (k_transaction_blob_limit : Nat) =>
    (Sigma fun (k_refund_divisor : Nat) =>
    (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))))
  | _ =>
    (let profile :=
      ({ blob_schedule := (blob_schedule 14 21 11684671) } : (ProtocolProfileFields 16 14 21 11684671 65536 131072 (2 ^ 64 - 1) (2 ^ 24) 6 5))
    ((pack_protocol_profile profile) : (Sigma fun (k_fork : Nat) =>
    (Sigma fun (k_target : Nat) =>
    (Sigma fun (k_maximum : Nat) =>
    (Sigma fun (k_denominator : Nat) =>
    (Sigma fun (k_code_limit : Nat) =>
    (Sigma fun (k_initcode_limit : Nat) =>
    (Sigma fun (k_transaction_total_gas_limit : Nat) =>
    (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
    (Sigma fun (k_transaction_blob_limit : Nat) =>
    (Sigma fun (k_refund_divisor : Nat) =>
    (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))))

/-- Whether a schema fork byte has a protocol-profile branch. -/
def schema_protocol_profile_forwards_matches (schema_fork : (BitVec 8)) : Bool :=
  ((schema_fork == 0x0A#8) || ((schema_fork == 0x0B#8) || ((schema_fork == 0x0C#8) || ((schema_fork == 0x0D#8) || ((schema_fork == 0x0E#8) || ((schema_fork == 0x0F#8) || ((schema_fork == 0x10#8) || ((schema_fork == 0x11#8) || ((schema_fork == 0x12#8) || ((schema_fork == 0x13#8) || ((schema_fork == 0x14#8) || (schema_fork == 0x15#8))))))))))))

/-- The Amsterdam mapping entry initializes the kernel before input decoding. -/
def DEFAULT_PROTOCOL_PROFILE : ProtocolProfile := (schema_protocol_profile 0x15#8)

def DEFAULT_EXECUTION_PROFILE : ExecutionProfile :=
  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, (execution_profile_for
    ((((((((((DEFAULT_PROTOCOL_PROFILE).2).2).2).2).2).2).2).2).2).2 0)⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩

def undefined_ChainConfig (_ : Unit) : SailM ChainConfig := do
  (pure { chain_id := ← (undefined_range 0 ((2 ^i 64) - 1)) })

