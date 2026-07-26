import Evm.Prelude
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Primitives.Code
import Evm.Primitives.Tx
import Evm.Primitives.Block
import Evm.Primitives.Evm
import Evm.Host.Kernel.Scratch
import Evm.Host.Kernel.Environment
import Evm.Evm.Machine
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

def initialize_registers (_ : Unit) : Unit :=
  ()

def sail_model_init (x_0 : Unit) : SailM Unit := do
  writeReg scratch_arena ⟨_, ⟨_, (byte_slice ScratchSource 0 0)⟩⟩
  writeReg k_parent_state_root ZERO_HASH
  writeReg k_n_headers 0
  writeReg k_chain_id 1
  writeReg k_fork Amsterdam
  writeReg k_blob_schedule { target := 14,
                             max := 21,
                             base_fee_update_fraction := 11684671 }
  writeReg k_header { number := 0,
                      timestamp := 0,
                      extra_data := ⟨_, ⟨_, EMPTY_SLICE⟩⟩,
                      gas_limit := 0,
                      gas_used := 0,
                      prev_randao := ZERO_WORD,
                      base_fee := ZERO_WORD,
                      blob_gas_used := 0,
                      excess_blob_gas := 0,
                      state_root := ZERO_HASH,
                      receipts_root := ZERO_HASH,
                      logs_bloom := EMPTY_LOGS_BLOOM,
                      fee_recipient := ZERO_ADDRESS,
                      parent_hash := ZERO_HASH,
                      parent_beacon_block_root := ZERO_HASH,
                      slot_number := 0 }
  writeReg k_tx { origin := ZERO_ADDRESS,
                  gas_price := ZERO_WORD,
                  blob_hashes := EMPTY_BLOB_HASHES }
  writeReg k_block_access_index 0
  writeReg pc 0
  writeReg gas_remaining GAS_ZERO
  writeReg state_gas_remaining GAS_ZERO
  writeReg state_gas_spilled STATE_GAS_SPILL_ZERO
  writeReg frame_refund GAS_REFUND_ZERO
  writeReg frame_status (Running ())
  writeReg message DEFAULT_MESSAGE
  writeReg call_depth 0
  writeReg frame_code EMPTY_CODE
  writeReg calldata ⟨_, ⟨_, EMPTY_SLICE⟩⟩
  writeReg returndata ⟨_, ⟨_, EMPTY_SLICE⟩⟩
  writeReg evm_memory ⟨_, ⟨_, (byte_slice EvmMemorySource 0 0)⟩⟩
  (pure (initialize_registers ()))

end Evm.Functions

open Evm
open Evm.Functions
open Defs

def main (_ : List String) : IO UInt32 := do
  main_of_sail_main ⟨default, (), default, default, default, default⟩ (sail_model_init >=> sail_main)
