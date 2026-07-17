import Evm.Primitives.Bytes
import Evm.Lib.Bytes

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

def KECCAK_EMPTY : hash := 0xC5D2460186F7233C927E7DB2DCC703C0E500B653CA82273B7BFAD8045D85A470#256

def EMPTY_TRIE_ROOT : hash := 0x56E81F171BCC55A6FF8345E692C0F86E5B48E01B996CADC001622FB5E363B421#256

def keccak256_slice (s : EvmByteSlice) : SailM (BitVec 256) := do
  (keccak256_segments [(BytesSlice s)])

def keccak256_word (w : word) : SailM (BitVec 256) := do
  (keccak256_segments [(bytes_list (word_to_bytes32 w) WORD_BYTE_LENGTH)])

def keccak256_address (a : address) : SailM (BitVec 256) := do
  (keccak256_segments [(bytes_list (address_to_bytes a) ADDRESS_BYTE_LENGTH)])

def sha256_pair (a : (BitVec 256)) (b : (BitVec 256)) : SailM (BitVec 256) := do
  (sha256_segments
    [(bytes_list (word_to_bytes32 a) WORD_BYTE_LENGTH), (bytes_list (word_to_bytes32 b)
      WORD_BYTE_LENGTH)])

def SECP_N_FULL : (BitVec 256) :=
  0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141#256

def SECP_N_HALF : (BitVec 256) :=
  0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF5D576E7357A4501DDFE92F46681B20A0#256

/-- Type quantifiers: k_ex161056_ : Nat, 0 ≤ k_ex161056_ ∧ k_ex161056_ ≤ 1 -/
def ecrecover_addr (h : (BitVec 256)) (yparity : y_parity) (r : word) (s : word) : SailM (Bool × address) := do
  let yparity := (yparity).value
  let recovered ← do (host_ecrecover h ⟨yparity⟩ r s)
  (pure (((BitVec.access recovered 160) == 1#1), (Sail.BitVec.extractLsb recovered 159 0)))

