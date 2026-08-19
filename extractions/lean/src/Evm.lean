import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Kernel.Scratch
import Evm.Primitives.ChainConfig
import Evm.Primitives.Tx
import Evm.Kernel.Environment
import Evm.Main

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
open StackValidation
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
open OpcodeOutcome
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

def initialize_registers (_ : Unit) : Unit :=
  ()

def sail_model_init (x_0 : Unit) : SailM Unit := do
  writeReg scratch_arena ⟨_, ⟨_, EMPTY_SCRATCH_SLICE⟩⟩
  writeReg k_parent_state_root ZERO_HASH
  writeReg k_n_headers 0
  writeReg k_chain_id 1
  writeReg k_execution_profile ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, (((((((((((((DEFAULT_EXECUTION_PROFILE).2).2).2).2).2).2).2).2).2).2).2).2).2⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩
  writeReg k_header { number := 0,
                      timestamp := 0,
                      extra_data := ⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩,
                      gas_limit := 0,
                      gas_used := 0,
                      prev_randao := ZERO_WORD,
                      base_fee := ZERO_WORD,
                      blob_gas_used := 0,
                      excess_blob_gas := 0,
                      state_root := ZERO_HASH,
                      receipts_root := ZERO_HASH,
                      logs_bloom := ⟨_, ⟨_, (stateless_input_slice 0 256)⟩⟩,
                      fee_recipient := ZERO_ADDRESS,
                      parent_hash := ZERO_HASH,
                      parent_beacon_block_root := ZERO_HASH,
                      slot_number := 0 }
  writeReg k_tx ⟨_, ({ origin := ZERO_ADDRESS,
                         gas_price := ZERO_WORD,
                         blob_hashes := EMPTY_BLOB_HASHES } : (TxEnvFields 0))⟩
  writeReg k_current_transaction_epoch 0
  (pure (initialize_registers ()))

end Evm.Functions

open Evm
open Evm.Functions
open Defs

def main (_ : List String) : IO UInt32 := do
  main_of_sail_main ⟨default, (), default, default, default, default⟩ (sail_model_init >=> sail_main)
