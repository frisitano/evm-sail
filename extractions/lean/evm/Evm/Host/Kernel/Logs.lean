import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Lib.Bytes
import Evm.Primitives.System
import Evm.Host.Kernel.Environment

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
open StateCheckpoint
open Register
open NodeRef
open MerkleSlot
open HaltKind
open FrameStatus
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

def k_log (a : address) (topics : (List word)) (data : Bytes) : SailM Unit := do
  (log_append a topics data)

def k_emit_transfer_log (src : address) (dst : address) (v : word) : SailM Unit := do
  if (((fork_lt (← readReg k_fork) Amsterdam) || ((word_is_zero v) || (src == dst))) : Bool)
  then (pure ())
  else
    (log_append EIP7708_SYSTEM_ADDRESS
      [EIP7708_TRANSFER_TOPIC, (address_to_word src), (address_to_word dst)]
      (bytes_list (word_to_bytes32 v) WORD_BYTE_LENGTH))

def k_emit_burn_log (a : address) (v : word) : SailM Unit := do
  if (((fork_lt (← readReg k_fork) Amsterdam) || (word_is_zero v)) : Bool)
  then (pure ())
  else
    (log_append EIP7708_SYSTEM_ADDRESS [EIP7708_BURN_TOPIC, (address_to_word a)]
      (bytes_list (word_to_bytes32 v) WORD_BYTE_LENGTH))

