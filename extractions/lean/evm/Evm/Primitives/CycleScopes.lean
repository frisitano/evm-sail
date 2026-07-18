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

def SCOPE_STATELESS_VALIDATION : (BitVec 8) := 0x00#8

def SCOPE_DECODE_INPUT : (BitVec 8) := 0x01#8

def SCOPE_INDEX_WITNESS : (BitVec 8) := 0x02#8

def SCOPE_VALIDATE_PAYLOAD : (BitVec 8) := 0x03#8

def SCOPE_EXECUTE_BLOCK : (BitVec 8) := 0x04#8

def SCOPE_VALIDATE_RESULT : (BitVec 8) := 0x05#8

def SCOPE_COMPUTE_OUTPUT_ROOT : (BitVec 8) := 0x06#8

def SCOPE_SERIALIZE_OUTPUT : (BitVec 8) := 0x07#8

