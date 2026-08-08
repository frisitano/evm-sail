import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Exceptions
import Evm.Primitives.Crypto
import Evm.Lib.Rlp.Codecs.TransactionSigning

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

/-! # Transaction signature rules

Signature validity, recovery parity, and sender authentication after the
transaction codec has established the envelope's canonical wire shape. -/

/-- Validates the selected signature scheme's `v` domain and returns its
recovery parity. Legacy accepts `27`/`28` or an EIP-155 value binding the
chain id; typed envelopes accept only an explicit zero-or-one parity.
An invalid `v` rejects the block here, so an unvalidated parity is
impossible to pass to sender recovery. -/
/- Type quantifiers: k_ex551072_ : Nat, k_ex551071_ : Nat, 0 ≤ k_ex551071_ ∧
  k_ex551071_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551072_ ∧ k_ex551072_ ≤ (2 ^ 256 - 1) -/
def tx_signature_parity (chain_id : Nat) (scheme : TxSignatureScheme) (v : Nat) : SailM Nat := do
  match scheme with
  | .LegacySignature =>
    (do
      let eip155_v := (word_ule 35 v)
      let signature_chain_id := (legacy_sig_chain_id v)
      let expected_chain_id := (word_of_chain_identifier chain_id)
      if (((v == 27) || ((v == 28) || (eip155_v && (signature_chain_id == expected_chain_id)))) : Bool)
      then
        (let parity_bit := (word_and v WORD_ONE)
        if ((parity_bit == WORD_ONE) : Bool)
        then (pure 0)
        else (pure 1))
      else (fatal_error InvalidSignature))
  | .TypedSignature =>
    (do
      if ((v == WORD_ZERO) : Bool)
      then (pure 0)
      else
        (do
          if ((v == WORD_ONE) : Bool)
          then (pure 1)
          else (fatal_error InvalidSignature)))

/-- Authenticates a transaction: enforce the EIP-2 low-`s` bound, recover the
signer selected by `y_parity`, and bind it to the address derived from the
witnessed 65-byte public key. -/
/- Type quantifiers: k_ex551075_ : Nat, k_ex551074_ : Nat, k_ex551073_ : Nat, 0 ≤ k_ex551073_ ∧
  k_ex551073_ ≤ 1, 0 ≤ k_ex551074_ ∧ k_ex551074_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex551075_ ∧
  k_ex551075_ ≤ (2 ^ 256 - 1) -/
def tx_auth_valid (sender : (Vector (BitVec 8) 20)) (h : (Vector (BitVec 8) 32)) (parity : Nat) (r : Nat) (s : Nat) : SailM Bool := do
  let high_s := (word_ult SECP_N_HALF s)
  if (high_s : Bool)
  then (pure false)
  else
    (do
      let (recovered, recovered_sender) ← do (ecrecover_addr h parity r s)
      (pure (recovered && (recovered_sender == sender))))

