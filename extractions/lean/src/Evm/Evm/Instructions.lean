import Evm.Primitives.Code

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

/-! # The instruction set

The abstract syntax of EVM bytecode: the full defined opcode set through
the Osaka fork (Yellow Paper Appendix H, plus the EIPs that extend it —
`PUSH0` EIP-3855, `SHL`/`SHR`/`SAR` EIP-145, `CLZ` EIP-7939,
`TLOAD`/`TSTORE` EIP-1153, `MCOPY` EIP-5656, `BLOBHASH`/`BLOBBASEFEE`
EIP-4844/EIP-7516, and `DUPN`/`SWAPN`/`EXCHANGE` EIP-8024). -/

/-- Decodes the immediate shared by EIP-8024 `DUPN` and `SWAPN` into
their one-based deep-stack index (17–235). -/
def decode_single_stack_index (immediate : (BitVec 8)) : SailM Nat := do
  let valid := (deep_stack_immediate_valid immediate)
  assert valid "sail/evm/instructions.sail:73.16-73.17"
  let value : Nat := (BitVec.toNatInt immediate)
  if ((value ≤b 90) : Bool)
  then (pure (value + 145))
  else
    (do
      assert (128 ≤b value) "sail/evm/instructions.sail:78.27-78.28"
      (pure (value - 111)))

/-- Decodes the EIP-8024 `EXCHANGE` immediate into the two zero-based
stack depths that it exchanges. -/
def decode_exchange_stack_indices (immediate : (BitVec 8)) : SailM (Nat × Nat) := do
  let valid := (exchange_immediate_valid immediate)
  assert valid "sail/evm/instructions.sail:87.16-87.17"
  let shifted : (BitVec 8) := (immediate ^^^ 0x8F#8)
  let quotient : Nat := (BitVec.toNatInt (Sail.BitVec.extractLsb shifted 7 4))
  let remainder : Nat := (BitVec.toNatInt (Sail.BitVec.extractLsb shifted 3 0))
  if ((quotient <b remainder) : Bool)
  then (pure ((quotient + 1), (remainder + 1)))
  else (pure ((remainder + 1), (29 - quotient)))

