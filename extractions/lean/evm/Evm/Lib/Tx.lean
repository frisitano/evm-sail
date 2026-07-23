import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Bytes
import Evm.Primitives.Crypto
import Evm.Primitives.Tx
import Evm.Host.Kernel.Scratch
import Evm.Lib.Rlp.Rlp

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
open exception
open ast
open TxType
open TrieUpdateSource
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open Register
open NodeRef
open MerkleSlot
open HaltKind
open FrameStatus
open FrameContinuation
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open ByteRegionResult
open BlockError

/-! # Transaction signature rules

The signature semantics of transactions, between the crypto core
(secp recovery and the curve constants) and the envelope decoder:
the signing preimage hashes, the per-envelope `v`-range rule, and sender
authentication — binding the witnessed public key to a transaction's
signature, called from transaction validity. -/

/-- The chain id encoded in a legacy signature with `v >= 35`
(EIP-155). -/
/- Type quantifiers: v : Nat, 0 ≤ v ∧ v ≤ (2 ^ 256 - 1) -/
def legacy_sig_chain_id (v : word) : word :=
  let v := (v).value
  ⟨((word_div_word ((word_sub_word v 35)).value 2)).value⟩

def LEGACY_SIGNATURE_SUFFIX_LENGTH : Nat := 2

def PUBLIC_KEY_BODY_LENGTH : Nat := DOUBLE_WORD_BYTE_LENGTH

/-- The transaction signing-preimage hash. `content_src` spans the RLP of
the pre-signature fields in the witness and is hashed as a segment
(never materialized); legacy EIP-155 transactions append
`(chain_id, 0, 0)`, typed transactions prepend the type byte as a
domain separator (EIP-2718). -/
/- Type quantifiers: k_ex407782_ : Nat, k_ex407781_ : Nat, k_ex407780_ : Nat, 0 ≤ k_ex407780_ ∧
  0 ≤ k_ex407781_, 0 ≤ k_ex407782_ ∧ k_ex407782_ ≤ (2 ^ 256 - 1) -/
def tx_signing_hash (t : TxType) (content_src : EvmByteSlice) (v : word) : SailM hash := do
  let content_src := ((content_src).2).2
  let v := (v).value
  let tb := (tx_type_byte t)
  let eip155 := ((tb == 0x00#8) && (word_ule 35 v))
  let chain_id :=
    if (eip155 : Bool)
    then ((legacy_sig_chain_id ⟨v⟩)).value
    else (ZERO_WORD).value
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
    else sailThrow ((InvalidBlock RlpDecode)) ) : SailM Nat )
  let suffix_length := suffix_len
  let content_len : Nat := (content_length + suffix_length)
  let (evm_prefix', prefix_len) ← do (rlp_list_prefix content_len)
  if (eip155 : Bool)
  then
    (do
      let mark ← do (scratch_begin ())
      (rlp_write_uint_word chain_id)
      (scratch_push_bytes [0x80#8, 0x80#8] LEGACY_SIGNATURE_SUFFIX_LENGTH)
      let ⟨_, ⟨_, suffix⟩⟩ ← do (rlp_finish mark)
      let signing_hash ← do
        (keccak256_segments
          [(bytes_list evm_prefix' prefix_len), (BytesSlice ⟨_, ⟨_, content_src⟩⟩), (BytesSlice
            ⟨_, ⟨_, suffix⟩⟩)])
      (scratch_rewind mark)
      (pure signing_hash))
  else
    (do
      if ((tb != 0x00#8) : Bool)
      then
        (keccak256_segments
          [(bytes_list [tb] 1), (bytes_list evm_prefix' prefix_len), (BytesSlice
            ⟨_, ⟨_, content_src⟩⟩)])
      else
        (keccak256_segments
          [(bytes_list evm_prefix' prefix_len), (BytesSlice ⟨_, ⟨_, content_src⟩⟩)]))

/-- The EIP-7702 authorization signing hash:
`keccak256(0x05 || rlp([chain_id, address, nonce]))`. -/
/- Type quantifiers: k_ex407784_ : Nat, k_ex407783_ : Nat, 0 ≤ k_ex407783_ ∧
  k_ex407783_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex407784_ ∧ k_ex407784_ ≤ (2 ^ 64 - 1) -/
def auth_signing_hash (chain_id : word) (addr : address) (nonce : account_nonce) : SailM hash := do
  let chain_id := (chain_id).value
  let nonce := (nonce).value
  let chain_id_length := (rlp_uint_word_size chain_id)
  let address_length := (rlp_addr_size ())
  let nonce_length := (rlp_uint_word_size nonce)
  let content_len : Nat := ((chain_id_length + address_length) + nonce_length)
  let mark ← do (scratch_begin ())
  (scratch_push_bytes [0x05#8] 1)
  (rlp_write_list_prefix content_len)
  (rlp_write_uint_word chain_id)
  (rlp_write_addr addr)
  (rlp_write_uint_word nonce)
  let ⟨_, ⟨_, encoded⟩⟩ ← do (rlp_finish mark)
  let signing_hash ← do (keccak256_slice ⟨_, ⟨_, encoded⟩⟩)
  (scratch_rewind mark)
  (pure signing_hash)

/-- The per-envelope `v`-range rule: legacy accepts `27`/`28` or the
EIP-155 form binding the chain id; typed envelopes accept
`y_parity ∈ {0, 1}`. -/
/- Type quantifiers: k_ex407786_ : Nat, k_ex407785_ : Nat, 0 ≤ k_ex407785_, 0 ≤ k_ex407786_ ∧
  k_ex407786_ ≤ (2 ^ 256 - 1) -/
def tx_sig_v_valid (chain_id : chain_identifier) (t : TxType) (v : word) : SailM Bool := do
  let v := (v).value
  match t with
  | .LegacyTx =>
    (pure ((v == 27) || ((v == 28) || ((word_ule 35 v) && (← do
              (pure (((legacy_sig_chain_id ⟨v⟩)).value == (← (word_of_chain_identifier
                      chain_id)))))))))
  | _ => (pure ((v == (WORD_ZERO).value) || (v == (WORD_ONE).value)))

/-- Extracts the recovery parity after [tx_sig_v_valid][] has validated the
envelope-specific `v` domain. Legacy `27` and every EIP-155 parity-zero
value are odd; typed envelopes carry the parity directly. -/
/- Type quantifiers: k_ex407787_ : Nat, 0 ≤ k_ex407787_ ∧ k_ex407787_ ≤ (2 ^ 256 - 1) -/
def tx_y_parity (t : TxType) (v : word) : y_parity :=
  let v := (v).value
  ⟨match t with
  | .LegacyTx =>
    (if ((((word_and ⟨v⟩ ⟨(WORD_ONE).value⟩)).value == (WORD_ONE).value) : Bool)
    then 0
    else 1)
  | _ =>
    (if ((v == (WORD_ZERO).value) : Bool)
    then 0
    else 1)⟩

/-- Authenticates a transaction: enforce the EIP-2 low-`s` bound, recover the
signer selected by `y_parity`, and bind it to the address derived from the
witnessed 65-byte public key. -/
/- Type quantifiers: k_ex407790_ : Nat, k_ex407789_ : Nat, k_ex407788_ : Nat, 0 ≤ k_ex407788_ ∧
  k_ex407788_ ≤ 1, 0 ≤ k_ex407789_ ∧ k_ex407789_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex407790_ ∧
  k_ex407790_ ≤ (2 ^ 256 - 1) -/
def tx_auth_valid (sender : address) (h : hash) (parity : y_parity) (r : word) (s : word) : SailM Bool := do
  let parity := (parity).value
  let r := (r).value
  let s := (s).value
  if ((word_ult (SECP_N_HALF).value s) : Bool)
  then (pure false)
  else
    (do
      let (recovered, recovered_sender) ← do (ecrecover_addr h ⟨parity⟩ ⟨r⟩ ⟨s⟩)
      (pure (recovered && (recovered_sender == sender))))

