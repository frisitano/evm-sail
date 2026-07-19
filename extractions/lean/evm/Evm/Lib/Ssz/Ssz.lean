import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Bytes
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

/-! # SSZ source decoding

Scalar readers and variable-list navigation over the private-input byte
source. Concrete container layouts belong in their decoder modules. -/

/-- The width of one entry in an SSZ variable-field offset table
(`uint32`, little-endian). -/
def SSZ_OFF_BYTES : byte_length := (ByteQuantity 4)

def SSZ_UINT_BYTES : byte_length := EIGHT_BYTE_LENGTH

def ssz_field_offset (base : source_pointer) (delta : byte_length) : SailM source_pointer := do
  (byte_quantity_add base delta)

/-- Reads a little-endian SSZ `uint32` used by a variable-field offset table. -/
def ssz_u32_at (input : EvmByteSlice) (offset : source_pointer) : SailM protocol_quantity := do
  let semanticResult ← do
    let b0 ← do (pure (Sail.BitVec.zeroExtend (← (slice_byte input offset)) 64))
    let b1 ← do
      (pure (Sail.BitVec.zeroExtend
          (← (slice_byte input (← (ssz_field_offset offset BYTE_ONE)))) 64))
    let b2 ← do
      (pure (Sail.BitVec.zeroExtend
          (← (slice_byte input (← (ssz_field_offset offset (ByteQuantity 2))))) 64))
    let b3 ← do
      (pure (Sail.BitVec.zeroExtend
          (← (slice_byte input (← (ssz_field_offset offset (ByteQuantity 3))))) 64))
    (pure (BitVec.toNatInt (b0 ||| ((b1 <<< 8) ||| ((b2 <<< 16) ||| (b3 <<< 24))))))
  pure (⟨semanticResult⟩)

/-- A little-endian `uint32` at absolute input offset `b`. -/
def ssz_u32 (input : EvmByteSlice) (offset : source_pointer) : SailM protocol_quantity := do
  let semanticResult ← do
    (do
        let semanticResult ← (ssz_u32_at input offset)
        pure ((semanticResult).value))
  pure (⟨semanticResult⟩)

/-- Reads an eight-byte little-endian limb from a validated SSZ field. -/
def ssz_limb (input : EvmByteSlice) (offset : source_pointer) : SailM limb := do
  let b0 ← do (pure (Sail.BitVec.zeroExtend (← (slice_byte input offset)) 64))
  let b1 ← do
    (pure (Sail.BitVec.zeroExtend (← (slice_byte input (← (ssz_field_offset offset BYTE_ONE))))
        64))
  let b2 ← do
    (pure (Sail.BitVec.zeroExtend
        (← (slice_byte input (← (ssz_field_offset offset (ByteQuantity 2))))) 64))
  let b3 ← do
    (pure (Sail.BitVec.zeroExtend
        (← (slice_byte input (← (ssz_field_offset offset (ByteQuantity 3))))) 64))
  let b4 ← do
    (pure (Sail.BitVec.zeroExtend
        (← (slice_byte input (← (ssz_field_offset offset SSZ_OFF_BYTES)))) 64))
  let b5 ← do
    (pure (Sail.BitVec.zeroExtend
        (← (slice_byte input (← (ssz_field_offset offset (ByteQuantity 5))))) 64))
  let b6 ← do
    (pure (Sail.BitVec.zeroExtend
        (← (slice_byte input (← (ssz_field_offset offset (ByteQuantity 6))))) 64))
  let b7 ← do
    (pure (Sail.BitVec.zeroExtend
        (← (slice_byte input (← (ssz_field_offset offset (ByteQuantity 7))))) 64))
  (pure (b0 ||| ((b1 <<< 8) ||| ((b2 <<< 16) ||| ((b3 <<< 24) ||| ((b4 <<< 32) ||| ((b5 <<< 40) ||| ((b6 <<< 48) ||| (b7 <<< 56)))))))))

def ssz_uint (input : EvmByteSlice) (offset : source_pointer) : SailM protocol_quantity := do
  let semanticResult ← do (pure (BitVec.toNatInt (← (ssz_limb input offset))))
  pure (⟨semanticResult⟩)

/-- A big-endian 20-byte address at offset `b`. -/
def ssz_addr (input : EvmByteSlice) (offset : source_pointer) : SailM address := do
  (pure (word_to_address (← (slice_load_n input offset ADDRESS_BYTE_LENGTH))))

/-- A big-endian 32-byte field (`Bytes32`, e.g. `prev_randao`). -/
def ssz_bytes32 (input : EvmByteSlice) (offset : source_pointer) : SailM hash := do
  (pure (word_to_hash (← (slice_load input offset))))

/-- The 256-byte logs bloom at offset `b`. -/
def ssz_logs_bloom (input : EvmByteSlice) (offset : source_pointer) : SailM LogsBloom := do
  let out : (Vector (BitVec 64) 32) := (vectorInit LIMB_ZERO)
  let cursor : byte_quantity := offset
  let (cursor, out) ← (( do
    let loop_k_lower := 0
    let loop_k_upper := 31
    let mut loop_vars := (cursor, out)
    for k in [loop_k_lower:loop_k_upper:1]i do
      let (cursor, out) := loop_vars
      loop_vars ← do
        let out ←
          (pure (vectorUpdate out k
              (word_limb_0 (← (slice_load_n input cursor EIGHT_BYTE_LENGTH)))))
        let cursor ← (ssz_field_offset cursor SSZ_UINT_BYTES)
        (pure (cursor, out))
    (pure loop_vars) ) : SailM (byte_quantity × (Vector (BitVec 64) 32)) )
  (pure out)

/-- A little-endian 32-byte `uint256` (SSZ). -/
def ssz_u256 (input : EvmByteSlice) (offset : source_pointer) : SailM word := do
  let c0 ← do (ssz_limb input offset)
  let c1 ← do (ssz_limb input (← (ssz_field_offset offset SSZ_UINT_BYTES)))
  let c2 ← do (ssz_limb input (← (ssz_field_offset offset (ByteQuantity 16))))
  let c3 ← do (ssz_limb input (← (ssz_field_offset offset (ByteQuantity 24))))
  (pure (word_from_limbs c0 c1 c2 c3))

