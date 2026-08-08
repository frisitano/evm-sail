import Sail
import Evm.Defs
import Evm.Specialization
import Evm.FakeReal
import Evm.HostAxioms

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

/-! # Exceptions

The failures that interrupt EVM execution or terminate payload validation. An
[ExceptionKind][type-ExceptionKind] exceptionally halts the current frame,
consumes its remaining gas, and reverts its state changes. A
[FatalError][type-FatalError] rejects the entire payload and terminates the
validator immediately. -/

def undefined_ExceptionKind (_ : Unit) : SailM ExceptionKind := do
  (internal_pick
    [StackUnderflow, StackOverflow, OutOfGas, InvalidOpcode, InvalidJump, StaticViolation, CallDepthExceeded, InsufficientBalance, WriteProtection, InitCodeTooLarge, NonceOverflow, AddressCollision])

/- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 11 -/
def ExceptionKind_of_num (arg_ : Nat) : ExceptionKind :=
  match arg_ with
  | 0 => StackUnderflow
  | 1 => StackOverflow
  | 2 => OutOfGas
  | 3 => InvalidOpcode
  | 4 => InvalidJump
  | 5 => StaticViolation
  | 6 => CallDepthExceeded
  | 7 => InsufficientBalance
  | 8 => WriteProtection
  | 9 => InitCodeTooLarge
  | 10 => NonceOverflow
  | _ => AddressCollision

def num_of_ExceptionKind (arg_ : ExceptionKind) : Nat :=
  match arg_ with
  | .StackUnderflow => 0
  | .StackOverflow => 1
  | .OutOfGas => 2
  | .InvalidOpcode => 3
  | .InvalidJump => 4
  | .StaticViolation => 5
  | .CallDepthExceeded => 6
  | .InsufficientBalance => 7
  | .WriteProtection => 8
  | .InitCodeTooLarge => 9
  | .NonceOverflow => 10
  | .AddressCollision => 11

def undefined_FatalError (_ : Unit) : SailM FatalError := do
  (internal_pick
    [InvalidConfig, HeaderChainBroken, RlpDecode, InvalidSignature, InvalidGasLimit, GasUsedExceedsLimit, BlobGasLimitExceeded, ExecutionInvalid, InvalidGasUsed, InvalidBlobGasUsed, InvalidExcessBlobGas, InvalidStateRoot, InvalidReceiptsRoot, InvalidLogsBloom, InvalidBlockHash, InvalidParentHash, BlockAccessListTooLarge, InvalidBlockAccessList, InvalidExecutionRequests, WitnessDeficient, NumericOverflow])

/- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 20 -/
def FatalError_of_num (arg_ : Nat) : FatalError :=
  match arg_ with
  | 0 => InvalidConfig
  | 1 => HeaderChainBroken
  | 2 => RlpDecode
  | 3 => InvalidSignature
  | 4 => InvalidGasLimit
  | 5 => GasUsedExceedsLimit
  | 6 => BlobGasLimitExceeded
  | 7 => ExecutionInvalid
  | 8 => InvalidGasUsed
  | 9 => InvalidBlobGasUsed
  | 10 => InvalidExcessBlobGas
  | 11 => InvalidStateRoot
  | 12 => InvalidReceiptsRoot
  | 13 => InvalidLogsBloom
  | 14 => InvalidBlockHash
  | 15 => InvalidParentHash
  | 16 => BlockAccessListTooLarge
  | 17 => InvalidBlockAccessList
  | 18 => InvalidExecutionRequests
  | 19 => WitnessDeficient
  | _ => NumericOverflow

def num_of_FatalError (arg_ : FatalError) : Nat :=
  match arg_ with
  | .InvalidConfig => 0
  | .HeaderChainBroken => 1
  | .RlpDecode => 2
  | .InvalidSignature => 3
  | .InvalidGasLimit => 4
  | .GasUsedExceedsLimit => 5
  | .BlobGasLimitExceeded => 6
  | .ExecutionInvalid => 7
  | .InvalidGasUsed => 8
  | .InvalidBlobGasUsed => 9
  | .InvalidExcessBlobGas => 10
  | .InvalidStateRoot => 11
  | .InvalidReceiptsRoot => 12
  | .InvalidLogsBloom => 13
  | .InvalidBlockHash => 14
  | .InvalidParentHash => 15
  | .BlockAccessListTooLarge => 16
  | .InvalidBlockAccessList => 17
  | .InvalidExecutionRequests => 18
  | .WitnessDeficient => 19
  | .NumericOverflow => 20

/- Type quantifiers: k_a : Type -/
def fatal_error (_reason : FatalError) : SailM k_a := do
  throw Error.Exit

