import Evm.Flow
import Evm.Arith
import Evm.Prelude
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

def SSZ_OFF_BYTES : byte_length := (ByteQuantity 4)

def SSZ_UINT_BYTES : byte_length := EIGHT_BYTE_LENGTH

/-- Type quantifiers: k_ex161073_ : Nat, 0 ≤ k_ex161073_ -/
def ssz_field_offset (base : source_pointer) (delta : Nat) : SailM source_pointer := do
  (byte_quantity_add base (ByteQuantity delta))

def ssz_u32_at (input : EvmByteSlice) (offset : source_pointer) : SailM protocol_quantity := do
  let semanticResult ← do
    let b0 ← do (pure (Sail.BitVec.zeroExtend (← (slice_byte input offset)) 64))
    let b1 ← do
      (pure (Sail.BitVec.zeroExtend (← (slice_byte input (← (ssz_field_offset offset 1)))) 64))
    let b2 ← do
      (pure (Sail.BitVec.zeroExtend (← (slice_byte input (← (ssz_field_offset offset 2)))) 64))
    let b3 ← do
      (pure (Sail.BitVec.zeroExtend (← (slice_byte input (← (ssz_field_offset offset 3)))) 64))
    (pure (BitVec.toNatInt (b0 ||| ((b1 <<< 8) ||| ((b2 <<< 16) ||| (b3 <<< 24))))))
  pure (⟨semanticResult⟩)

def ssz_u32 (input : EvmByteSlice) (offset : source_pointer) : SailM protocol_quantity := do
  let semanticResult ← do
    (do
        let semanticResult ← (ssz_u32_at input offset)
        pure ((semanticResult).value))
  pure (⟨semanticResult⟩)

def ssz_limb (input : EvmByteSlice) (offset : source_pointer) : SailM limb := do
  let b0 ← do (pure (Sail.BitVec.zeroExtend (← (slice_byte input offset)) 64))
  let b1 ← do
    (pure (Sail.BitVec.zeroExtend (← (slice_byte input (← (ssz_field_offset offset 1)))) 64))
  let b2 ← do
    (pure (Sail.BitVec.zeroExtend (← (slice_byte input (← (ssz_field_offset offset 2)))) 64))
  let b3 ← do
    (pure (Sail.BitVec.zeroExtend (← (slice_byte input (← (ssz_field_offset offset 3)))) 64))
  let b4 ← do
    (pure (Sail.BitVec.zeroExtend (← (slice_byte input (← (ssz_field_offset offset 4)))) 64))
  let b5 ← do
    (pure (Sail.BitVec.zeroExtend (← (slice_byte input (← (ssz_field_offset offset 5)))) 64))
  let b6 ← do
    (pure (Sail.BitVec.zeroExtend (← (slice_byte input (← (ssz_field_offset offset 6)))) 64))
  let b7 ← do
    (pure (Sail.BitVec.zeroExtend (← (slice_byte input (← (ssz_field_offset offset 7)))) 64))
  (pure (b0 ||| ((b1 <<< 8) ||| ((b2 <<< 16) ||| ((b3 <<< 24) ||| ((b4 <<< 32) ||| ((b5 <<< 40) ||| ((b6 <<< 48) ||| (b7 <<< 56)))))))))

def ssz_uint (input : EvmByteSlice) (offset : source_pointer) : SailM protocol_quantity := do
  let semanticResult ← do (pure (BitVec.toNatInt (← (ssz_limb input offset))))
  pure (⟨semanticResult⟩)

def ssz_addr (input : EvmByteSlice) (offset : source_pointer) : SailM address := do
  (pure (Sail.BitVec.extractLsb (← (slice_load_n input offset ADDRESS_BYTE_LENGTH)) 159 0))

def ssz_bytes32 (input : EvmByteSlice) (offset : source_pointer) : SailM (BitVec 256) := do
  (slice_load input offset)

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
              (Sail.BitVec.extractLsb (← (slice_load_n input cursor EIGHT_BYTE_LENGTH)) 63 0)))
        let cursor ← (ssz_field_offset cursor 8)
        (pure (cursor, out))
    (pure loop_vars) ) : SailM (byte_quantity × (Vector (BitVec 64) 32)) )
  (pure out)

def ssz_u256 (input : EvmByteSlice) (offset : source_pointer) : SailM word := do
  let c0 ← do (ssz_limb input offset)
  let c1 ← do (ssz_limb input (← (ssz_field_offset offset 8)))
  let c2 ← do (ssz_limb input (← (ssz_field_offset offset 16)))
  let c3 ← do (ssz_limb input (← (ssz_field_offset offset 24)))
  (pure (c3 +++ (c2 +++ (c1 +++ c0))))

