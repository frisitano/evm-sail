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

abbrev BLOCK_ACCESS_LIST_ITEM_GAS : Nat := 2000

def GAS_COST_ZERO : gas_cost := 0

def undefined_GasCharge (_ : Unit) : SailM GasCharge := do
  (pure { affordable := ← (undefined_bool ()),
          cost := ← (undefined_range 0 ((2 ^i 64) - 1)) })

def GAS_CHARGE_UNAFFORDABLE : GasCharge :=
  { affordable := false,
    cost := GAS_COST_ZERO }

/- Type quantifiers: cost : Nat, 0 ≤ cost ∧ cost ≤ (2 ^ 64 - 1) -/
def gas_charge (cost : Nat) : GasCharge :=
  { affordable := true,
    cost := cost }

def TRANSACTION_EXECUTION_GAS_LIMIT : Nat := (2 ^i 24)

abbrev STATE_GAS_SPILL_ZERO : Nat := 0

def STATE_GAS_SPILL_LIMIT : state_gas_spill := (2 ^i 24)

abbrev GAS_ZERO : Nat := 0

abbrev STATE_GAS_ZERO : Nat := 0

/-- The execution-gas allowance of each protocol system call (EIP-4788,
EIP-2935, EIP-7002, EIP-7251, and EIP-8282). -/
abbrev SYSTEM_CALL_GAS_LIMIT : Nat := 30000000

def GAS_CONSTANT_ZERO : gas_constant := 0

def GAS_REFUND_ZERO : gas_refund := 0

def FRAME_STATE_GAS_DELTA_ZERO : frame_state_gas_delta := 0

def STATE_GAS_DELTA_ZERO : state_gas_delta := 0

