import Evm.Primitives.Bytes

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

/-! # Code

Executable code and its PUSH-aware `JUMPDEST` analysis (YP §9.4.3). -/

def EMPTY_JUMPDEST_CHUNK : JumpdestChunk :=
  0x0000000000000000000000000000000000000000000000000000000000000000#256

def EMPTY_JUMPDEST_REF : JumpdestRef := 0x0000000000000000#64

/- Type quantifiers: k_off : Nat, k_len : Nat, (source_valid_range k_off k_len) ∧
  (code_valid_length k_len) -/
def code_slice (bytes : (EvmByteSliceFields k_off k_len)) : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len))) :=
  ((⟨_, ⟨_, bytes⟩⟩ : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len)))) : (Sigma fun
  (k_syn_off : Nat) => (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len))))

/-- Converts a source span whose producer guarantees executable cursor
headroom. This is vacuous in the canonical model; the optimized splice
checks the representation invariant if a proof was erased by storage in a
non-dependent aggregate. -/
/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, 0 ≤
  bytes_dependentWitness0 ∧ 0 ≤ bytes_dependentWitness1 -/
def validated_code_slice (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
  (bytes_dependentWitness0 : Nat) =>
  (Sigma fun (bytes_dependentWitness1 : Nat) =>
  (EvmByteSliceFields bytes_dependentWitness0 bytes_dependentWitness1))) :=
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  ((code_slice bytes) : (Sigma fun (bytes_dependentWitness0 : Nat) =>
  (Sigma fun (bytes_dependentWitness1 : Nat) =>
  (EvmByteSliceFields bytes_dependentWitness0 bytes_dependentWitness1))))

/-- Canonical empty executable code. -/
def EMPTY_CODE_SLICE : CodeSlice := (code_slice (byte_slice StatelessInputSource 0 0))

/-- Whether an EIP-8024 `DUPN`/`SWAPN` immediate is valid. Invalid
immediates remain opcode-aligned during JUMPDEST analysis. -/
def deep_stack_immediate_valid (immediate : (BitVec 8)) : Bool :=
  let value : Nat := (BitVec.toNatInt immediate)
  ((value ≤b 90) || (128 ≤b value))

/-- Whether an EIP-8024 `EXCHANGE` immediate is valid. Invalid immediates
remain opcode-aligned during JUMPDEST analysis. -/
def exchange_immediate_valid (immediate : (BitVec 8)) : Bool :=
  let value : Nat := (BitVec.toNatInt immediate)
  ((value ≤b 81) || (128 ≤b value))

def EMPTY_CODE : Code :=
  { bytes := EMPTY_CODE_SLICE,
    jumpdests := EMPTY_JUMPDEST_REF }

