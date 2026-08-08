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
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
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

/-! # Code

Executable code and its PUSH-aware `JUMPDEST` analysis (YP §9.4.3). -/

def EMPTY_JUMP_TABLE : jump_table_index := 0

/- Type quantifiers: k_off : Nat, k_len : Nat, (code_region_valid_range k_off k_len) ∧
  (code_valid_length k_len) -/
def code_slice (bytes : (CodeRegionSliceFields k_off k_len)) : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_syn_len : Nat) => (CodeRegionSliceFields k_syn_off k_syn_len))) :=
  ((⟨_, ⟨_, bytes⟩⟩ : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_syn_len : Nat) => (CodeRegionSliceFields k_syn_off k_syn_len)))) : (Sigma fun
  (k_syn_off : Nat) => (Sigma fun (k_syn_len : Nat) => (CodeRegionSliceFields k_syn_off k_syn_len))))

/-- Converts a source span whose producer guarantees executable cursor
headroom. The explicit check re-establishes the proof after the length has
crossed a non-dependent host boundary. -/
/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, 0 ≤
  bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def validated_code_slice (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeRegionSliceFields k_off k_len)))) : SailM (Sigma fun
  (bytes_dependentWitness0 : Nat) =>
  (Sigma fun (bytes_dependentWitness1 : Nat) =>
  (CodeRegionSliceFields bytes_dependentWitness0 bytes_dependentWitness1))) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  if _sailIf0 : ((bytes.len ≤b (((2 ^i 32) - 1) - 32)) : Bool) = true
  then
    (pure ((code_slice bytes) : (Sigma fun (bytes_dependentWitness0 : Nat) =>
      (Sigma fun (bytes_dependentWitness1 : Nat) =>
      (CodeRegionSliceFields bytes_dependentWitness0 bytes_dependentWitness1)))))
  else
    (do
      assert false "executable code cursor headroom"
      throw Error.Exit)

/-- Canonical empty executable code. -/
def EMPTY_CODE_SLICE : CodeSlice := (code_slice EMPTY_CODE_REGION_SLICE)

def undefined_DeepStackOperation (_ : Unit) : SailM DeepStackOperation := do
  (internal_pick [DeepStackDuplicate, DeepStackSwap, DeepStackExchange, NotDeepStackOperation])

/- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 3 -/
def DeepStackOperation_of_num (arg_ : Nat) : DeepStackOperation :=
  match arg_ with
  | 0 => DeepStackDuplicate
  | 1 => DeepStackSwap
  | 2 => DeepStackExchange
  | _ => NotDeepStackOperation

def num_of_DeepStackOperation (arg_ : DeepStackOperation) : Nat :=
  match arg_ with
  | .DeepStackDuplicate => 0
  | .DeepStackSwap => 1
  | .DeepStackExchange => 2
  | .NotDeepStackOperation => 3

/-- Classifies an opcode against Amsterdam's immediate deep-stack
operations; every other opcode maps to the non-member. -/
/- Type quantifiers: opcode : Nat, 0 ≤ opcode ∧ opcode ≤ 255 -/
def deep_stack_operation (opcode : Nat) : DeepStackOperation :=
  match opcode with
  | 230 => DeepStackDuplicate
  | 231 => DeepStackSwap
  | 232 => DeepStackExchange
  | _ => NotDeepStackOperation

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

/-- Applies the immediate-validity rule selected by a decoded deep-stack
operation. `DUPN` and `SWAPN` share the single-index encoding, while
`EXCHANGE` uses the pair encoding. -/
def deep_stack_operation_immediate_valid (operation : DeepStackOperation) (immediate : (BitVec 8)) : Bool :=
  match operation with
  | .DeepStackDuplicate => (deep_stack_immediate_valid immediate)
  | .DeepStackSwap => (deep_stack_immediate_valid immediate)
  | .DeepStackExchange => (exchange_immediate_valid immediate)
  | .NotDeepStackOperation => false

/- Type quantifiers: jumpdests : Nat, k_off : Nat, k_len : Nat, (code_region_valid_range k_off k_len)
  ∧ (code_valid_length k_len), 0 ≤ jumpdests ∧ jumpdests ≤ (2 ^ 64 - 1) -/
def analyzed_code (bytes : (CodeRegionSliceFields k_off k_len)) (jumpdests : Nat) : (CodeFields k_off k_len) :=
  { jumpdests := jumpdests }

/- Type quantifiers: k_off : Nat, k_len : Nat, (code_region_valid_range k_off k_len) ∧
  (code_valid_length k_len) -/
def code_bytes (code : (CodeFields k_off k_len)) : (CodeRegionSliceFields k_off k_len) :=
  {  }

def EMPTY_CODE : Code := ⟨_, ⟨_, (analyzed_code ((EMPTY_CODE_SLICE).2).2 EMPTY_JUMP_TABLE)⟩⟩

