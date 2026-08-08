import Evm.Prelude
import Evm.Lib.Rlp.Encoding

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

/-! # CREATE address RLP codec

The `CREATE` and `CREATE2` address rules (YP §7, EIP-1014). -/

/-- The `CREATE` address (YP §7): the low 20 bytes of
`keccak256(rlp([sender, nonce]))`. -/
/- Type quantifiers: k_ex551059_ : Nat, 0 ≤ k_ex551059_ ∧ k_ex551059_ ≤ (2 ^ 64 - 1) -/
def create_address (sender : (Vector (BitVec 8) 20)) (nonce : Nat) : SailM (Vector (BitVec 8) 20) := do
  let address_length := (rlp_addr_size ())
  let nonce_length := (rlp_uint_word_size nonce)
  let content_len := (address_length + nonce_length)
  let encoded_len ← do (rlp_list_size content_len)
  let encoder ← do (rlp_encoder_begin encoded_len)
  (rlp_write_list_prefix content_len)
  (rlp_write_addr sender)
  (rlp_write_uint_word nonce)
  let ⟨_, ⟨_, encoded⟩⟩ ← do (rlp_encoder_finish encoder)
  let digest ← do (scratch_keccak256 ⟨_, ⟨_, encoded⟩⟩)
  let digest_word := (hash_to_word digest)
  let address := (word_to_address digest_word)
  (rlp_encoder_rewind encoder)
  (pure address)

