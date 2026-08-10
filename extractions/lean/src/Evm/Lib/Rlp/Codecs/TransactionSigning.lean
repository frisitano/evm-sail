import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Kernel.Scratch
import Evm.Primitives.Tx
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

/-! # Transaction RLP signing preimages

The signature semantics of transactions, between the crypto core
(secp recovery and the curve constants) and the envelope decoder:
the signing preimage hashes, the per-envelope `v`-range rule, and sender
authentication — binding the witnessed public key to a transaction's
signature, called from transaction validity. -/

/-- The chain id encoded in a legacy signature with `v >= 35`
(EIP-155). -/
/- Type quantifiers: v : Nat, 0 ≤ v ∧ v ≤ (2 ^ 256 - 1) -/
def legacy_sig_chain_id (v : Nat) : Nat :=
  let adjusted_v := (word_sub_word v 35)
  (word_div_word adjusted_v 2)

abbrev LEGACY_SIGNATURE_SUFFIX_LENGTH : Nat := 2

abbrev PUBLIC_KEY_BODY_LENGTH : Nat := 64

/-- The transaction signing-preimage hash. `content_src` spans the RLP of
the pre-signature fields in the witness and is copied once into the final
contiguous preimage; legacy EIP-155 transactions append
`(chain_id, 0, 0)`, typed transactions prepend the type byte as a
domain separator (EIP-2718). -/
/- Type quantifiers: k_ex551164_ : Nat, content_src_dependentWitness1 : Nat, content_src_dependentWitness0
  : Nat, 0 ≤ content_src_dependentWitness0 ∧
  0 ≤ content_src_dependentWitness1 ∧
  (content_src_dependentWitness0 + content_src_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex551164_ ∧ k_ex551164_ ≤ (2 ^ 256 - 1) -/
def tx_signing_hash (t : TxType) (content_src : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (v : Nat) : SailM (Vector (BitVec 8) 32) := do
  let content_src_dependentWitness0 := (content_src).1
  let content_src_dependentWitness1 := ((content_src).2).1
  let content_src := ((content_src).2).2
  let tb : (BitVec 8) := (tx_envelope_type t)
  let eip155 := ((tb == 0x00#8) && (word_ule 35 v))
  let chain_id :=
    if (eip155 : Bool)
    then (legacy_sig_chain_id v)
    else ZERO_WORD
  let suffix_len : Nat :=
    if (eip155 : Bool)
    then
      (let chain_id_length := (rlp_uint_word_size chain_id)
      let suffix_length := LEGACY_SIGNATURE_SUFFIX_LENGTH
      (chain_id_length + suffix_length))
    else 0
  let content_length ← (( do
    if ((content_src.len ≤b (2 ^i 30)) : Bool)
    then (pure content_src.len)
    else (fatal_error RlpDecode) ) : SailM Nat )
  let suffix_length := suffix_len
  let content_len : Nat := (content_length + suffix_length)
  let prefix_len ← do (rlp_length_prefix_len content_len)
  let type_len : Nat :=
    if ((tb == 0x00#8) : Bool)
    then 0
    else 1
  let preimage_len : Nat := ((type_len + prefix_len) + content_len)
  let encoder ← do (rlp_encoder_begin preimage_len)
  if ((tb != 0x00#8) : Bool)
  then (scratch_push_byte tb)
  else (pure ())
  (rlp_write_list_prefix content_len)
  (stateless_input_scratch_push_slice ⟨_, ⟨_, content_src⟩⟩)
  if (eip155 : Bool)
  then
    (do
      (rlp_write_uint_word chain_id)
      (scratch_push_byte 0x80#8)
      (scratch_push_byte 0x80#8))
  else (pure ())
  let ⟨_, ⟨_, preimage⟩⟩ ← do (rlp_encoder_finish encoder)
  let signing_hash ← do (scratch_keccak256 ⟨_, ⟨_, preimage⟩⟩)
  (rlp_encoder_rewind encoder)
  (pure signing_hash)

/-- The EIP-7702 authorization signing hash:
`keccak256(0x05 || rlp([chain_id, address, nonce]))`. -/
/- Type quantifiers: k_ex551166_ : Nat, k_ex551165_ : Nat, 0 ≤ k_ex551165_ ∧
  k_ex551165_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex551166_ ∧ k_ex551166_ ≤ (2 ^ 64 - 1) -/
def auth_signing_hash (chain_id : Nat) (addr : (Vector (BitVec 8) 20)) (nonce : Nat) : SailM (Vector (BitVec 8) 32) := do
  let chain_id_length := (rlp_uint_word_size chain_id)
  let address_length := (rlp_addr_size ())
  let nonce_length := (rlp_uint_word_size nonce)
  let content_len : Nat := ((chain_id_length + address_length) + nonce_length)
  let preimage_len ← (( do (pure (1 + (← (rlp_list_size content_len)))) ) : SailM Nat )
  let encoder ← do (rlp_encoder_begin preimage_len)
  (scratch_push_byte 0x05#8)
  (rlp_write_list_prefix content_len)
  (rlp_write_uint_word chain_id)
  (rlp_write_addr addr)
  (rlp_write_uint_word nonce)
  let ⟨_, ⟨_, encoded⟩⟩ ← do (rlp_encoder_finish encoder)
  let signing_hash ← do (scratch_keccak256 ⟨_, ⟨_, encoded⟩⟩)
  (rlp_encoder_rewind encoder)
  (pure signing_hash)

