import Evm.Flow
import Evm.Arith
import Evm.Primitives.Quantities
import Evm.Primitives.Code
import Evm.Primitives.Crypto
import Evm.Host.EvmByteSlice

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

def store_jumpdest_chunk (table : JumpdestRef) (code_len : byte_length) (analysis : CodeAnalysis) : SailM Unit := do
  if ((analysis.chunk != EMPTY_JUMPDEST_CHUNK) : Bool)
  then
    (do
      let stored ← do
        (jumpdest_table_store_chunk table code_len analysis.chunk_index analysis.chunk)
      assert stored "JUMPDEST chunk store")
  else (pure ())

/-- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ 255 -/
def jumpdest_bit (index : Nat) : JumpdestChunk :=
  let chunk : (BitVec 256) := EMPTY_JUMPDEST_CHUNK
  (BitVec.update chunk index 1#1)

/-- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_analyze_code_from (code : EvmByteSlice) (table : JumpdestRef) (pc : code_pointer) (analysis : CodeAnalysis) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((byte_quantity_lt pc code.len) : Bool)
      then
        (do
          let chunk := analysis.chunk
          let chunk_index : byte_quantity := analysis.chunk_index
          let chunk_offset : Nat := analysis.chunk_offset
          let opcode ← do (slice_byte code pc)
          let chunk : (BitVec 256) :=
            if ((opcode == 0x5B#8) : Bool)
            then (chunk ||| (jumpdest_bit chunk_offset))
            else chunk
          let opcode_value := (BitVec.toNatInt opcode)
          let step : Nat :=
            if (((96 ≤b opcode_value) && (opcode_value ≤b 127)) : Bool)
            then (opcode_value -i 94)
            else 1
          let step_quantity := (ByteQuantity step)
          if ((byte_quantity_lt step_quantity (← (byte_quantity_sub code.len pc))) : Bool)
          then
            (do
              let added ← (( do (byte_quantity_add pc step_quantity) ) : SailM byte_quantity )
              let progressed : Nat := (chunk_offset + step)
              if ((progressed <b 256) : Bool)
              then
                (_rec_analyze_code_from code table added
                  { chunk := chunk,
                    chunk_index := chunk_index,
                    chunk_offset := progressed } _reclimit_pred)
              else
                (do
                  (store_jumpdest_chunk table code.len
                    { chunk := chunk,
                      chunk_index := chunk_index,
                      chunk_offset := chunk_offset })
                  (_rec_analyze_code_from code table added
                    { chunk := EMPTY_JUMPDEST_CHUNK,
                      chunk_index := ← (byte_quantity_add chunk_index BYTE_ONE),
                      chunk_offset := (progressed -i 256) } _reclimit_pred)))
          else
            (store_jumpdest_chunk table code.len
              { chunk := chunk,
                chunk_index := chunk_index,
                chunk_offset := chunk_offset }))
      else (store_jumpdest_chunk table code.len analysis))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

def analyze_code_from (code : EvmByteSlice) (table : JumpdestRef) (pc : code_pointer) (analysis : CodeAnalysis) : SailM Unit := do
  let _measure :=
    (let .ByteQuantity code_len := code.len
    let .ByteQuantity position := pc
    (code_len -i position) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_analyze_code_from code table pc analysis (_measure + 1))

def analyze_code (code : EvmByteSlice) : SailM JumpdestRef := do
  if ((byte_quantity_equal code.len BYTE_ZERO) : Bool)
  then (pure EMPTY_JUMPDEST_REF)
  else
    (do
      let table ← do (jumpdest_table_alloc code.len)
      assert (table != EMPTY_JUMPDEST_REF) "JUMPDEST table allocation"
      (analyze_code_from code table BYTE_ZERO
        { chunk := EMPTY_JUMPDEST_CHUNK,
          chunk_index := BYTE_ZERO,
          chunk_offset := 0 })
      (pure table))

def code_db_insert (code : EvmByteSlice) : SailM hash := do
  (code_db_store code (← (analyze_code code)))

def code_db_resolve (code_hash : hash) : SailM Code := do
  if ((code_hash == KECCAK_EMPTY) : Bool)
  then (pure EMPTY_CODE)
  else
    (do
      match (← (code_db_lookup code_hash)) with
      | .some code => (pure code)
      | none => sailThrow ((InvalidBlock WitnessDeficient)))

