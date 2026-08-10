import Evm.Prelude
import Evm.Primitives.Code
import Evm.Host.RegionAccess
import Evm.Primitives.Crypto
import Evm.Host.Code
import Evm.Kernel.Environment
import Evm.Kernel.Storage
import Evm.Kernel.Accounts

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

/-! # State: account code

Account code operations over the content-addressed code store, including
EIP-7702 delegation designators. -/

/-- The account's code hash — the code-store key. -/
def k_code_key (a : (Vector (BitVec 8) 20)) : SailM (Vector (BitVec 8) 32) := do
  (pure (← (k_aload a)).info.code_hash)

/-- `EXTCODEHASH` (EIP-1052): a truly non-existent account reads as `0`,
not `KECCAK_EMPTY`. -/
def k_get_codehash (a : (Vector (BitVec 8) 20)) : SailM (Vector (BitVec 8) 32) := do
  let acc ← do (k_aload a)
  let missing := (! acc.present)
  if (missing : Bool)
  then (pure ZERO_HASH)
  else (pure acc.info.code_hash)

/-- Deploys code to an account: analyzes, stores, and binds its hash. -/
/- Type quantifiers: code_dependentWitness1 : Nat, code_dependentWitness0 : Nat, 0 ≤
  code_dependentWitness0 ∧
  0 ≤ code_dependentWitness1 ∧
  (code_dependentWitness0 + code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ code_dependentWitness1 ∧ (code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1) -/
def k_deploy_code (a : (Vector (BitVec 8) 20)) (code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeRegionSliceFields k_off k_len)))) : SailM Unit := do
  let code_dependentWitness0 := (code).1
  let code_dependentWitness1 := ((code).2).1
  let code := ((code).2).2
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let cur ← do (k_aload a)
  let h ← (( do (code_db_insert ⟨_, ⟨_, code⟩⟩ profile.fork) ) : SailM
    (Vector (BitVec 8) 32) )
  (store_account_info a cur { cur.info with code_hash := h })

/-- Installs an EIP-7702 delegation designator
(`0xef0100 ‖ target`) as the account's code. -/
def k_set_delegation (a : (Vector (BitVec 8) 20)) (target : (Vector (BitVec 8) 20)) : SailM Unit := do
  let cur ← do (k_aload a)
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let ⟨_, ⟨_, code_region⟩⟩ ← do (code_region_from_delegation target)
  let ⟨_, ⟨_, code⟩⟩ ← do (validated_code_slice ⟨_, ⟨_, code_region⟩⟩)
  let h ← (( do (code_db_insert ⟨_, ⟨_, code⟩⟩ execution_profile.protocol.fork) ) : SailM
    (Vector (BitVec 8) 32) )
  (store_account_info a cur { cur.info with code_hash := h })

/-- Resets an account's code to empty (EIP-7702 clearing). -/
def k_clear_code (a : (Vector (BitVec 8) 20)) : SailM Unit := do
  let cur ← do (k_aload a)
  (store_account_info a cur { cur.info with code_hash := KECCAK_EMPTY })

/-- The delegation target of an account's code, with a validity flag —
false when the code is not a designator. -/
def k_deleg_target (a : (Vector (BitVec 8) 20)) : SailM (Bool × (Vector (BitVec 8) 20)) := do
  let h ← (( do (k_code_key a) ) : SailM (Vector (BitVec 8) 32) )
  let r ← (( do (code_db_read_delegation h) ) : SailM AddressResult )
  (pure (r.success, r.address))

/-- `EXTCODESIZE`: the account's code length in bytes. -/
def k_get_code_size (a : (Vector (BitVec 8) 20)) : SailM Nat := do
  let code_key ← do (k_code_key a)
  let ⟨_, ⟨_, code⟩⟩ ← do (code_db_resolve code_key)
  (pure code.len)

/-- `EXTCODECOPY`: copies account code into frame memory, zero-padded
past the end. -/
/- Type quantifiers: k_ex551982_ : Nat, k_ex551981_ : Nat, k_ex551980_ : Nat, 0 ≤ k_ex551980_ ∧
  k_ex551980_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex551981_ ∧ k_ex551981_ ≤ (2 ^ 256 - 1), 0 ≤
  k_ex551982_ ∧ k_ex551982_ ≤ (2 ^ 32 - 1) -/
def k_code_copy (a : (Vector (BitVec 8) 20)) (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  let code_key ← do (k_code_key a)
  let ⟨_, ⟨_, code⟩⟩ ← do (code_db_resolve code_key)
  let bytes := (code_bytes code)
  (code_slice_copy_word_offset ⟨_, ⟨_, bytes⟩⟩ dst off len)

