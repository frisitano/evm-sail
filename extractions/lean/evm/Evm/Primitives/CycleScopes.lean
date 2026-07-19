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

/-! # Instrumentation scopes

Named stages of the stateless validation pipeline, used to attribute
md
# Instrumentation scopes

Named stages of the stateless validation pipeline, used to attribute
cycle counts when profiling the model.

!!! note "Implementation"
    Instrumentation only: the scope markers carry no protocol meaning and
    do not affect execution.

| Scope | Stage |
| ----- | ----- |
| `SCOPE_DECODE_INPUT` | Input decoding |
| `SCOPE_INDEX_WITNESS` | Witness indexing |
| `SCOPE_VALIDATE_PAYLOAD` | Payload commitments |
| `SCOPE_EXECUTE_BLOCK` | Block execution |
| `SCOPE_VALIDATE_RESULT` | Result validation |
| `SCOPE_COMPUTE_OUTPUT_ROOT` | Output root |
| `SCOPE_SERIALIZE_OUTPUT` | Output serialization |


! The pipeline stages, as scope identifiers.
let SCOPE_STATELESS_VALIDATION : bits(8) = 0x00
let SCOPE_DECODE_INPUT : bits(8) = 0x01
let SCOPE_INDEX_WITNESS : bits(8) = 0x02
let SCOPE_VALIDATE_PAYLOAD : bits(8) = 0x03
let SCOPE_EXECUTE_BLOCK : bits(8) = 0x04
let SCOPE_VALIDATE_RESULT : bits(8) = 0x05
let SCOPE_COMPUTE_OUTPUT_ROOT : bits(8) = 0x06
let SCOPE_SERIALIZE_OUTPUT : bits(8) = 0x07


!!! note "Implementation"
    Instrumentation only: the scope markers carry no protocol meaning and
    do not affect execution.

| Scope | Stage |
| ----- | ----- |
| `SCOPE_DECODE_INPUT` | Input decoding |
| `SCOPE_INDEX_WITNESS` | Witness indexing |
| `SCOPE_VALIDATE_PAYLOAD` | Payload commitments |
| `SCOPE_EXECUTE_BLOCK` | Block execution |
| `SCOPE_VALIDATE_RESULT` | Result validation |
| `SCOPE_COMPUTE_OUTPUT_ROOT` | Output root |
| `SCOPE_SERIALIZE_OUTPUT` | Output serialization | -/

/-- The pipeline stages, as scope identifiers. -/
def SCOPE_STATELESS_VALIDATION : (BitVec 8) := 0x00#8

def SCOPE_DECODE_INPUT : (BitVec 8) := 0x01#8

def SCOPE_INDEX_WITNESS : (BitVec 8) := 0x02#8

def SCOPE_VALIDATE_PAYLOAD : (BitVec 8) := 0x03#8

def SCOPE_EXECUTE_BLOCK : (BitVec 8) := 0x04#8

def SCOPE_VALIDATE_RESULT : (BitVec 8) := 0x05#8

def SCOPE_COMPUTE_OUTPUT_ROOT : (BitVec 8) := 0x06#8

def SCOPE_SERIALIZE_OUTPUT : (BitVec 8) := 0x07#8

