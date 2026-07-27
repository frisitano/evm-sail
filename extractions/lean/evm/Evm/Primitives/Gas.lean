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
open BalIterEntry

/-! # Gas quantities

Semantic domains for available gas, transaction and block gas, schedule
constants, computed costs, and the signed refund counter. The canonical model
keeps EVM gas arithmetic mathematical; optimized builds may choose bounded
representations after preserving the validation and out-of-gas behavior.

## Types

`gas` is the live EVM quantity, `transaction_gas` is supplied by a transaction,
`block_gas_limit` bounds block execution, and `block_gas` records gas consumed
by the block. `gas_constant` contains fixed schedule values and `gas_cost`
marks transient, non-negative charges after their affordability or structural
bound has been established. It remains an exact natural in the canonical
model. `gas_refund` records signed changes before the transaction refund cap.

## Constants

The zero values initialize each gas domain. Protocol limits are checked at the
transaction or block boundary rather than encoded as a shared implementation
ceiling. -/

def GAS_COST_ZERO : gas_cost := 0

def TRANSACTION_EXECUTION_GAS_LIMIT : transaction_gas := (2 ^i 24)

def STATE_GAS_SPILL_ZERO : state_gas_spill := 0

def STATE_GAS_SPILL_LIMIT : state_gas_spill := (2 ^i 24)

def GAS_ZERO : gas := 0

/-- The execution-gas allowance of each protocol system call (EIP-4788,
EIP-2935, EIP-7002, EIP-7251, and EIP-8282). -/
def SYSTEM_CALL_GAS_LIMIT : transaction_gas := 30000000

def GAS_CONSTANT_ZERO : gas_constant := 0

def GAS_REFUND_ZERO : gas_refund := 0

def FRAME_STATE_GAS_DELTA_ZERO : frame_state_gas_delta := 0

def STATE_GAS_DELTA_ZERO : state_gas_delta := 0

