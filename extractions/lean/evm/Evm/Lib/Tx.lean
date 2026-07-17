import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Bytes
import Evm.Primitives.Crypto
import Evm.Primitives.Tx
import Evm.Host.EvmByteSlice
import Evm.Host.Kernel.Scratch
import Evm.Lib.Rlp.Rlp

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open Sail.ConcurrencyInterfaceV1

noncomputable section
namespace Evm

open ConcurrencyInterfaceV1

open Defs
namespace Functions

open option
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open ast
open TxType
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open Register
open NodeRef
open MerkleSlot
open JEntry
open HaltKind
open FrameStatus
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

def legacy_sig_chain_id (v : (BitVec 256)) : (BitVec 256) :=
  (word_divmod (v - 0x0000000000000000000000000000000000000000000000000000000000000023#256)
    0x0000000000000000000000000000000000000000000000000000000000000002#256).quotient

def LEGACY_SIGNATURE_SUFFIX_LENGTH : byte_length := (ByteQuantity 2)

def PUBLIC_KEY_Y_OFFSET : source_pointer := (ByteQuantity 33)

def PUBLIC_KEY_BODY_LENGTH : byte_length := DOUBLE_WORD_BYTE_LENGTH

def tx_signing_hash (t : TxType) (content_src : EvmByteSlice) (v : (BitVec 256)) : SailM (BitVec 256) := do
  let tb := (tx_type_byte t)
  let eip155 :=
    ((tb == 0x00#8) && (word_ule
        0x0000000000000000000000000000000000000000000000000000000000000023#256 v))
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

/-- Type quantifiers: nonce : Nat, 0 ≤ nonce ∧ nonce ≤ (2 ^ 64 - 1) -/
def auth_signing_hash (chain_id : (BitVec 256)) (addr : (BitVec 160)) (nonce : Nat) : SailM (BitVec 256) := do
  let content_len ← do (rlp_uint_word_size chain_id)
  let content_len ← (byte_quantity_add content_len (rlp_addr_size ()))
  let content_len ← (byte_quantity_add content_len (← (rlp_protocol_quantity_size nonce)))
  let encoded_len ← do (byte_quantity_add BYTE_ONE (← (rlp_list_size content_len)))
  let mark ← do (scratch_begin ())
  (scratch_push_bytes [0x05#8] BYTE_ONE)
  (rlp_write_list_prefix content_len)
  (rlp_write_uint_word chain_id)
  (rlp_write_addr addr)
  (rlp_write_protocol_quantity nonce)
  let encoded ← do (rlp_finish mark encoded_len)
  let signing_hash ← do (keccak256_slice encoded)
  (scratch_rewind mark)
  (pure signing_hash)

/-- Type quantifiers: chain_id : Nat, 0 ≤ chain_id ∧ chain_id ≤ (2 ^ 64 - 1) -/
def tx_sig_v_ok (chain_id : Nat) (t : TxType) (v : (BitVec 256)) : SailM Bool := do
  match t with
  | .LegacyTx =>
    (pure ((v == 0x000000000000000000000000000000000000000000000000000000000000001B#256) || ((v == 0x000000000000000000000000000000000000000000000000000000000000001C#256) || ((word_ule
              0x0000000000000000000000000000000000000000000000000000000000000023#256 v) && ((legacy_sig_chain_id
                v) == (← (word_of_nat chain_id)))))))
  | _ => (pure ((v == WORD_ZERO) || (v == WORD_ONE)))

def tx_auth_ok (pubkey : EvmByteSlice) (h : (BitVec 256)) (r : (BitVec 256)) (s : (BitVec 256)) : SailM Bool := do
  if ((word_ult SECP_N_HALF s) : Bool)
  then (pure false)
  else
    (do
      let x ← do (slice_load pubkey BYTE_ONE)
      let y ← do (slice_load pubkey PUBLIC_KEY_Y_OFFSET)
      (secp256k1_verify h r s x y))

