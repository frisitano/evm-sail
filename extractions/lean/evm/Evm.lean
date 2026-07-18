import Evm.Arith
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

def initialize_registers (_ : Unit) : Unit :=
  ()

def sail_model_init (x_0 : Unit) : SailM Unit := do
  writeReg scratch_cursor BYTE_ZERO
  writeReg k_parent_state_root (BitVec.zero 256)
  writeReg k_n_headers 0
  writeReg k_chain_id 1
  writeReg k_fork Amsterdam
  writeReg k_blob_schedule { target := ⟨14⟩,
                             max := ⟨21⟩,
                             base_fee_update_fraction := ⟨11684671⟩ }
  writeReg k_header { number := ⟨0⟩,
                      timestamp := ⟨0⟩,
                      extra_data := EMPTY_SLICE,
                      gas_limit := GAS_ZERO,
                      gas_used := GAS_ZERO,
                      prev_randao := ZERO_WORD,
                      base_fee := ZERO_WORD,
                      blob_gas_used := ⟨0⟩,
                      excess_blob_gas := ⟨0⟩,
                      state_root := ZERO_WORD,
                      receipts_root := ZERO_WORD,
                      logs_bloom := EMPTY_LOGS_BLOOM,
                      fee_recipient := ZERO_ADDR,
                      parent_hash := ZERO_WORD,
                      parent_beacon_block_root := ZERO_WORD,
                      slot_number := ⟨0⟩ }
  writeReg k_tx { origin := ZERO_ADDR,
                  gas_price := ZERO_WORD,
                  blob_hashes := EMPTY_BLOB_HASHES }
  writeReg pc BYTE_ZERO
  writeReg gas_remaining GAS_ZERO
  writeReg frame_refund GAS_REFUND_ZERO
  writeReg frame_status (Running ())
  writeReg message DEFAULT_MESSAGE
  writeReg call_depth 0
  writeReg frame_code EMPTY_CODE
  writeReg calldata EMPTY_SLICE
  writeReg returndata EMPTY_SLICE
  writeReg evm_memory (byte_slice EvmMemorySource BYTE_ZERO BYTE_ZERO)
  (pure (initialize_registers ()))

end Evm.Functions

open Evm
open Evm.Functions
open Defs

def main (_ : List String) : IO UInt32 := do
  main_of_sail_main ⟨default, (), default, default, default, default⟩ (sail_model_init >=> sail_main)
