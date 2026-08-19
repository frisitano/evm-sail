import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Lib.Ssz.Ssz
import Evm.Lib.Rlp.Encoding
import Evm.Lib.Ssz.StatelessInput

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

/-! # Withdrawal RLP codec

The header-level commitments recomputed from the payload: the
transactions and withdrawals tries, the EIP-7685 requests hash, and the
block header hash itself. -/

/-- The RLP of one withdrawal (EIP-4895), assembled in the scratch
arena. -/
/- Type quantifiers: withdrawal_dependentWitness1 : Nat, withdrawal_dependentWitness0 : Nat, 0 ≤
  withdrawal_dependentWitness0 ∧
  0 ≤ withdrawal_dependentWitness1 ∧
  (withdrawal_dependentWitness0 + withdrawal_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  withdrawal_dependentWitness1 = 44 -/
def withdrawal_rlp (withdrawal : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Sigma fun
  (withdrawal_dependentWitness0 : Nat) =>
  (Sigma fun (withdrawal_dependentWitness1 : Nat) =>
  (ScratchSliceFields withdrawal_dependentWitness0 withdrawal_dependentWitness1))) := do
  let withdrawal_dependentWitness0 := (withdrawal).1
  let withdrawal_dependentWitness1 := ((withdrawal).2).1
  let withdrawal := ((withdrawal).2).2
  let index ← do (decode_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_INDEX)
  let validator_index ← do (decode_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_VALIDATOR_INDEX)
  let address := (stateless_input_sub_slice withdrawal WD_ADDRESS ADDRESS_BYTE_LENGTH)
  let amount ← do (decode_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_AMOUNT)
  let index_length := (rlp_uint_word_size index)
  let validator_index_length := (rlp_uint_word_size validator_index)
  let address_length ← do (rlp_input_slice_size address)
  let amount_length := (rlp_uint_word_size amount)
  let content_length := (((index_length + validator_index_length) + address_length) + amount_length)
  if ((48 <b content_length) : Bool)
  then (fatal_error RlpDecode)
  else (pure ())
  let bounded_content_length : Nat := (Nat.mod content_length 49)
  let content_len := bounded_content_length
  let encoded_length ← do (rlp_list_size content_len)
  let encoder ← do (rlp_encoder_begin encoded_length)
  (rlp_write_list_prefix content_len)
  (rlp_write_uint_word index)
  (rlp_write_uint_word validator_index)
  (rlp_write_input_slice ⟨_, ⟨_, address⟩⟩)
  (rlp_write_uint_word amount)
  (rlp_encoder_finish encoder)

