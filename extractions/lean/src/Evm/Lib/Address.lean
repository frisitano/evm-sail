import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Kernel.Scratch

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

/-! # CREATE2 address derivation

The fixed-width EIP-1014 preimage does not use RLP and remains separate from
the CREATE address codec. -/

/-- The `CREATE2` address (EIP-1014): the low 20 bytes of
`keccak256(0xff ++ sender ++ salt ++ keccak256(initcode))`. -/
/- Type quantifiers: k_ex551060_ : Nat, 0 ≤ k_ex551060_ ∧ k_ex551060_ ≤ (2 ^ 256 - 1) -/
def create2_address (sender : (Vector (BitVec 8) 20)) (salt : Nat) (init_hash : (Vector (BitVec 8) 32)) : SailM (Vector (BitVec 8) 20) := do
  let mark ← do (scratch_reserve 85)
  (scratch_push_byte 0xFF#8)
  (scratch_push_address sender)
  let salt_hash := (word_to_hash salt)
  (scratch_push_b256 salt_hash WORD_BYTE_LENGTH)
  (scratch_push_b256 init_hash WORD_BYTE_LENGTH)
  let ⟨_, ⟨_, preimage⟩⟩ ← do (scratch_finish mark)
  let digest ← do (scratch_keccak256 ⟨_, ⟨_, preimage⟩⟩)
  (scratch_rewind mark)
  let digest_word := (hash_to_word digest)
  (pure (word_to_address digest_word))

