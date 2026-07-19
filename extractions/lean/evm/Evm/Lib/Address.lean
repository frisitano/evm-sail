import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Bytes
import Evm.Lib.Bytes
import Evm.Primitives.Crypto
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

/-! # Contract address derivation

The `CREATE` and `CREATE2` address rules (YP §7, EIP-1014). -/

/-- The `CREATE` address (YP §7): the low 20 bytes of
`keccak256(rlp([sender, nonce]))`. -/
/- Type quantifiers: k_ex161224_ : Nat, 0 ≤ k_ex161224_ ∧ k_ex161224_ ≤ (2 ^ 64 - 1) -/
def create_address (sender : address) (nonce : account_nonce) : SailM address := do
  let nonce := (nonce).value
  let content_len ← do
    (byte_quantity_add (rlp_addr_size ()) (← (rlp_protocol_quantity_size ⟨nonce⟩)))
  let encoded_len ← do (rlp_list_size content_len)
  let mark ← do (scratch_begin ())
  (rlp_write_list_prefix content_len)
  (rlp_write_addr sender)
  (rlp_write_protocol_quantity ⟨nonce⟩)
  let encoded ← do (rlp_finish mark encoded_len)
  let address ← do (pure (word_to_address (hash_to_word (← (keccak256_slice encoded)))))
  (scratch_rewind mark)
  (pure address)

/-- The `CREATE2` address (EIP-1014): the low 20 bytes of
`keccak256(0xff ++ sender ++ salt ++ keccak256(initcode))`. -/
def create2_address (sender : address) (salt : word) (init_hash : hash) : SailM address := do
  (pure (word_to_address
      (hash_to_word
        (← (keccak256_segments
            [(bytes_list [0xFF#8] BYTE_ONE), (bytes_list (address_to_bytes sender)
              ADDRESS_BYTE_LENGTH), (bytes_list (word_to_bytes32 salt) WORD_BYTE_LENGTH), (bytes_list
              (hash_to_bytes32 init_hash) WORD_BYTE_LENGTH)])))))

