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

/-! # Protocol forks

The protocol forks the model executes, oldest to newest. Their singleton
integer values are the activation order, so ordinary integer comparisons
express every fork gate without an overloaded comparison or conversion.
Pure data — no registers, no externs. -/

/-- Launch protocol. -/
abbrev Frontier : Nat := 0

/-- EIP-2 create rules, EIP-7 DELEGATECALL. -/
abbrev Homestead : Nat := 1

/-- EIP-140 REVERT, EIP-211 returndata; precompiles 0x05-0x08. -/
abbrev Byzantium : Nat := 2

/-- EIP-145 shifts, EIP-1014 CREATE2, EIP-1052 EXTCODEHASH. -/
abbrev Constantinople : Nat := 3

/-- EIP-1344 CHAINID, EIP-2200 SSTORE metering. -/
abbrev Istanbul : Nat := 4

/-- EIP-2929/2930 access lists and warm/cold access costs. -/
abbrev Berlin : Nat := 5

/-- EIP-1559 fee market and EIP-3529 refund reduction. -/
abbrev London : Nat := 6

/-- Difficulty-bomb-only schema fork; execution rules remain London. -/
abbrev ArrowGlacier : Nat := 7

/-- Difficulty-bomb-only schema fork; execution rules remain London. -/
abbrev GrayGlacier : Nat := 8

/-- EIP-4399 PREVRANDAO replaces DIFFICULTY. -/
abbrev Paris : Nat := 9

/-- EIP-3651 warm coinbase, EIP-3855 PUSH0, EIP-3860 initcode. -/
abbrev Shanghai : Nat := 10

/-- EIP-1153/4844; precompiles 0x01-0x0a. -/
abbrev Cancun : Nat := 11

/-- EIP-7623 calldata floor; BLS precompiles 0x0b-0x11. -/
abbrev Prague : Nat := 12

/-- EIP-7883 modexp gas, EIP-7825 cap; precompile 0x100. -/
abbrev Osaka : Nat := 13

/-- First blob-parameter-only fork; execution rules remain Osaka. -/
abbrev BPO1 : Nat := 14

/-- Second blob-parameter-only fork; execution rules remain Osaka. -/
abbrev BPO2 : Nat := 15

/-- EIP-7954 code/initcode size bump (65536/131072). -/
abbrev Amsterdam : Nat := 16
