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

/-! # Code

Executable code and its PUSH-aware `JUMPDEST` analysis (YP §9.4.3). -/

def EMPTY_JUMPDEST_CHUNK : JumpdestChunk :=
  0x0000000000000000000000000000000000000000000000000000000000000000#256

def EMPTY_JUMPDEST_REF : JumpdestRef := 0x0000000000000000#64

def EMPTY_CODE : Code :=
  { bytes := EMPTY_SLICE,
    jumpdests := EMPTY_JUMPDEST_REF }

