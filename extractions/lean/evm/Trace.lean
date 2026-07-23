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

open Sail
open Evm
open Evm.Defs
open Evm.Defs.Register
open Evm.Functions

private def hexValue (character : Char) : Nat :=
  if '0' ≤ character ∧ character ≤ '9' then
    character.toNat - '0'.toNat
  else if 'a' ≤ character ∧ character ≤ 'f' then
    character.toNat - 'a'.toNat + 10
  else if 'A' ≤ character ∧ character ≤ 'F' then
    character.toNat - 'A'.toNat + 10
  else
    0

private def parseHex : List Char → Array byte
  | high :: low :: rest =>
      #[BitVec.ofNat 8 (16 * hexValue high + hexValue low)] ++ parseHex rest
  | _ => #[]

private def traceModelInit (_ : Unit) : Evm.SailM Unit := do
  writeReg scratch_arena ⟨_, ⟨_, byte_slice .ScratchSource 0 0⟩⟩
  writeReg k_parent_state_root ZERO_HASH
  writeReg k_n_headers ⟨0⟩
  writeReg k_chain_id 1
  writeReg k_fork .Amsterdam
  writeReg k_blob_schedule
    { target := ⟨14⟩
      max := ⟨21⟩
      base_fee_update_fraction := ⟨11684671⟩ }
  writeReg k_header
    { number := 0
      timestamp := 0
      extra_data := ⟨_, ⟨_, EMPTY_SLICE⟩⟩
      gas_limit := ⟨0⟩
      gas_used := 0
      prev_randao := ⟨ZERO_WORD.value⟩
      base_fee := ⟨ZERO_WORD.value⟩
      blob_gas_used := ⟨0⟩
      excess_blob_gas := ⟨0⟩
      state_root := ZERO_HASH
      receipts_root := ZERO_HASH
      logs_bloom := EMPTY_LOGS_BLOOM
      fee_recipient := ZERO_ADDRESS
      parent_hash := ZERO_HASH
      parent_beacon_block_root := ZERO_HASH
      slot_number := ⟨0⟩ }
  writeReg k_tx
    { origin := ZERO_ADDRESS
      gas_price := ⟨ZERO_WORD.value⟩
      blob_hashes := EMPTY_BLOB_HASHES }
  writeReg pc 0
  writeReg gas_remaining GAS_ZERO
  writeReg state_gas_remaining GAS_ZERO
  writeReg state_gas_spilled ⟨STATE_GAS_SPILL_ZERO.value⟩
  writeReg frame_refund ⟨GAS_REFUND_ZERO.value⟩
  writeReg frame_status (.Running ())
  writeReg message DEFAULT_MESSAGE
  writeReg call_depth ⟨0⟩
  writeReg frame_stack (vectorInit DEFAULT_FRAME_CONTINUATION)
  writeReg frame_stack_top ⟨0⟩
  writeReg frame_code EMPTY_CODE
  writeReg calldata ⟨_, ⟨_, EMPTY_SLICE⟩⟩
  writeReg returndata ⟨_, ⟨_, EMPTY_SLICE⟩⟩
  writeReg evm_memory ⟨_, ⟨_, byte_slice .EvmMemorySource 0 0⟩⟩

def main : IO UInt32 := do
  let inputText ← (← IO.getStdin).readToEnd
  let hostState :=
    { initialHostState with
      inputBytes := parseHex inputText.trimAscii.toString.toList }
  let action : Evm.SailM StatelessValidationResult := do
    traceModelInit ()
    let inputRef ← decode_stateless_input_ref (← stateless_input ())
    verify_stateless_payload inputRef
  match (action.run hostState).run default with
  | .ok (.StatelessPayloadValid (), _) _ => do
      IO.println "valid"
      return 0
  | .ok (.StatelessPayloadInvalid failure, _) _ => do
      IO.println s!"invalid scope={failure.scope.toNat} reason={repr failure.reason}"
      return 0
  | .error (.User (.InvalidBlock reason)) _ => do
      IO.eprintln s!"uncaught invalid block: {repr reason}"
      return 1
  | .error error _ => do
      IO.eprintln s!"error: {error.print}"
      return 1
