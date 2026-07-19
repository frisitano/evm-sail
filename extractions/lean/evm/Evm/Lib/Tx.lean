import Evm.Flow
import Evm.Arith
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

open word
open option
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open b256
open ast
open address
open TxType
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open StateCheckpoint
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
open BlockError

/-! # Transaction signature rules

The signature semantics of transactions, between the crypto core
(secp recovery and the curve constants) and the envelope decoder:
the signing preimage hashes, the per-envelope `v`-range rule, and sender
authentication — binding the witnessed public key to a transaction's
signature, called from transaction validity. -/

/-- The chain id encoded in a legacy signature with `v >= 35`
(EIP-155). -/
def legacy_sig_chain_id (v : word) : word :=
  (word_divmod (word_sub v (limb_to_word 0x0000000000000023#64))
    (limb_to_word 0x0000000000000002#64)).quotient

def LEGACY_SIGNATURE_SUFFIX_LENGTH : byte_length := (ByteQuantity 2)

def PUBLIC_KEY_BODY_LENGTH : byte_length := DOUBLE_WORD_BYTE_LENGTH

/-- The transaction signing-preimage hash. `content_src` spans the RLP of
the pre-signature fields in the witness and is hashed as a segment
(never materialized); legacy EIP-155 transactions append
`(chain_id, 0, 0)`, typed transactions prepend the type byte as a
domain separator (EIP-2718). -/
def tx_signing_hash (t : TxType) (content_src : EvmByteSlice) (v : word) : SailM hash := do
  let tb := (tx_type_byte t)
  let eip155 := ((tb == 0x00#8) && (word_ule (limb_to_word 0x0000000000000023#64) v))
  let chain_id :=
    if (eip155 : Bool)
    then (legacy_sig_chain_id v)
    else ZERO_WORD
  let suffix_len ← do
    if (eip155 : Bool)
    then (byte_quantity_add (← (rlp_uint_word_size chain_id)) LEGACY_SIGNATURE_SUFFIX_LENGTH)
    else (pure BYTE_ZERO)
  let content_len ← do (byte_quantity_add content_src.len suffix_len)
  let (evm_prefix', prefix_len) ← do (rlp_list_prefix content_len)
  if (eip155 : Bool)
  then
    (do
      let mark ← do (scratch_begin ())
      (rlp_write_uint_word chain_id)
      (scratch_push_bytes [0x80#8, 0x80#8] LEGACY_SIGNATURE_SUFFIX_LENGTH)
      let suffix ← do (rlp_finish mark suffix_len)
      let signing_hash ← do
        (keccak256_segments
          [(bytes_list evm_prefix' prefix_len), (BytesSlice content_src), (BytesSlice suffix)])
      (scratch_rewind mark)
      (pure signing_hash))
  else
    (do
      if ((tb != 0x00#8) : Bool)
      then
        (keccak256_segments
          [(bytes_list [tb] BYTE_ONE), (bytes_list evm_prefix' prefix_len), (BytesSlice content_src)])
      else (keccak256_segments [(bytes_list evm_prefix' prefix_len), (BytesSlice content_src)]))

/-- The EIP-7702 authorization signing hash:
`keccak256(0x05 || rlp([chain_id, address, nonce]))`. -/
/- Type quantifiers: k_ex161225_ : Nat, 0 ≤ k_ex161225_ ∧ k_ex161225_ ≤ (2 ^ 64 - 1) -/
def auth_signing_hash (chain_id : word) (addr : address) (nonce : account_nonce) : SailM hash := do
  let nonce := (nonce).value
  let content_len ← do (rlp_uint_word_size chain_id)
  let content_len ← (byte_quantity_add content_len (rlp_addr_size ()))
  let content_len ← (byte_quantity_add content_len (← (rlp_protocol_quantity_size ⟨nonce⟩)))
  let encoded_len ← do (byte_quantity_add BYTE_ONE (← (rlp_list_size content_len)))
  let mark ← do (scratch_begin ())
  (scratch_push_bytes [0x05#8] BYTE_ONE)
  (rlp_write_list_prefix content_len)
  (rlp_write_uint_word chain_id)
  (rlp_write_addr addr)
  (rlp_write_protocol_quantity ⟨nonce⟩)
  let encoded ← do (rlp_finish mark encoded_len)
  let signing_hash ← do (keccak256_slice encoded)
  (scratch_rewind mark)
  (pure signing_hash)

/-- The per-envelope `v`-range rule: legacy accepts `27`/`28` or the
EIP-155 form binding the chain id; typed envelopes accept
`y_parity ∈ {0, 1}`. -/
/- Type quantifiers: k_ex161226_ : Nat, 0 ≤ k_ex161226_ ∧ k_ex161226_ ≤ (2 ^ 64 - 1) -/
def tx_sig_v_ok (chain_id : chain_identifier) (t : TxType) (v : word) : SailM Bool := do
  let chain_id := (chain_id).value
  match t with
  | .LegacyTx =>
    (pure ((v == (limb_to_word 0x000000000000001B#64)) || ((v == (limb_to_word 0x000000000000001C#64)) || ((word_ule
              (limb_to_word 0x0000000000000023#64) v) && ((legacy_sig_chain_id v) == (← (word_of_protocol_quantity
                  ⟨chain_id⟩)))))))
  | _ => (pure ((v == WORD_ZERO) || (v == WORD_ONE)))

/-- Extracts the recovery parity after [tx_sig_v_ok][] has validated the
envelope-specific `v` domain. Legacy `27` and every EIP-155 parity-zero
value are odd; typed envelopes carry the parity directly. -/
def tx_y_parity (t : TxType) (v : word) : y_parity :=
  ⟨match t with
  | .LegacyTx =>
    (if (((word_and v WORD_ONE) == WORD_ONE) : Bool)
    then 0
    else 1)
  | _ =>
    (if ((v == WORD_ZERO) : Bool)
    then 0
    else 1)⟩

/-- Authenticates a transaction: enforce the EIP-2 low-`s` bound, recover the
signer selected by `y_parity`, and bind it to the address derived from the
witnessed 65-byte public key. -/
/- Type quantifiers: k_ex161227_ : Nat, 0 ≤ k_ex161227_ ∧ k_ex161227_ ≤ 1 -/
def tx_auth_ok (sender : address) (h : hash) (parity : y_parity) (r : word) (s : word) : SailM Bool := do
  let parity := (parity).value
  if ((word_ult SECP_N_HALF s) : Bool)
  then (pure false)
  else
    (do
      let (recovered, recovered_sender) ← do (ecrecover_addr h ⟨parity⟩ r s)
      (pure (recovered && (recovered_sender == sender))))

