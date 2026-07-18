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

def undefined_ExceptionKind (_ : Unit) : SailM ExceptionKind := do
  (internal_pick
    [StackUnderflow, StackOverflow, OutOfGas, InvalidOpcode, InvalidJump, StaticViolation, CallDepthExceeded, InsufficientBalance, WriteProtection, InitCodeTooLarge, NonceOverflow, AddressCollision])

/-- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 11 -/
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

def num_of_ExceptionKind (arg_ : ExceptionKind) : Int :=
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

def undefined_BlockError (_ : Unit) : SailM BlockError := do
  (internal_pick
    [InvalidConfig, HeaderChainBroken, RlpDecode, InvalidSignature, InvalidGasLimit, GasUsedExceedsLimit, BlobGasLimitExceeded, ExecutionInvalid, InvalidGasUsed, InvalidBlobGasUsed, InvalidExcessBlobGas, InvalidStateRoot, InvalidReceiptsRoot, InvalidLogsBloom, InvalidBlockHash, InvalidParentHash, BlockAccessListTooLarge, InvalidBlockAccessList, InvalidExecutionRequests, WitnessDeficient])

/-- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 19 -/
def BlockError_of_num (arg_ : Nat) : BlockError :=
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
  | _ => WitnessDeficient

def num_of_BlockError (arg_ : BlockError) : Int :=
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

