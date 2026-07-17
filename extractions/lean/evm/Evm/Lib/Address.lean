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

/-- Type quantifiers: nonce : Nat, 0 ≤ nonce ∧ nonce ≤ (2 ^ 64 - 1) -/
def create_address (sender : (BitVec 160)) (nonce : Nat) : SailM (BitVec 160) := do
  let content_len ← do
    (byte_quantity_add (rlp_addr_size ()) (← (rlp_protocol_quantity_size nonce)))
  let encoded_len ← do (rlp_list_size content_len)
  let mark ← do (scratch_begin ())
  (rlp_write_list_prefix content_len)
  (rlp_write_addr sender)
  (rlp_write_protocol_quantity nonce)
  let encoded ← do (rlp_finish mark encoded_len)
  let address ← do (pure (word_to_address (← (keccak256_slice encoded))))
  (scratch_rewind mark)
  (pure address)

def create2_address (sender : (BitVec 160)) (salt : (BitVec 256)) (init_hash : (BitVec 256)) : SailM (BitVec 160) := do
  (pure (word_to_address
      (← (keccak256_segments
          [(bytes_list [0xFF#8] BYTE_ONE), (bytes_list (address_to_bytes sender) ADDRESS_BYTE_LENGTH), (bytes_list
            (word_to_bytes32 salt) WORD_BYTE_LENGTH), (bytes_list (word_to_bytes32 init_hash)
            WORD_BYTE_LENGTH)]))))

