import Evm.Primitives.Crypto

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

def validation_debug_reset (_ : Unit) : Unit :=
  ()

def validation_debug_record (_scope : (BitVec 8)) (_reason : BlockError) : Unit :=
  ()

def debug_account_storage_root (_a : (BitVec 160)) : (BitVec 256) :=
  EMPTY_TRIE_ROOT

def debug_rebuild_state_root (_ : Unit) : (BitVec 256) :=
  EMPTY_TRIE_ROOT

