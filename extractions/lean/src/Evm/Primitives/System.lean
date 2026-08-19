import Evm.Prelude

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

/-! # System addresses and topics

The protocol-defined system actors: the pseudo-caller of block-level
system calls, the system predeploys they target, and the EIP-7708
transfer-log identity. Pure data — no registers, no externs.

| Name | Address / Topic | Source |
| ---- | --------------- | ------ |
| `SYSTEM_ADDRESS` | `0xfffe…fffe` | EIP-4788/2935/7002/7251 caller |
| `BEACON_ROOTS_ADDR` | `0x000f…ac02` | EIP-4788 |
| `HISTORY_STORAGE_ADDR` | `0x0000…2935` | EIP-2935 |
| `WITHDRAWAL_REQUEST_ADDR` | `0x0000…7002` | EIP-7002 |
| `CONSOLIDATION_REQUEST_ADDR` | `0x0000…7251` | EIP-7251 |
| `BUILDER_DEPOSIT_REQUEST_ADDR` | `0x0000…8282` | EIP-8282 |
| `BUILDER_EXIT_REQUEST_ADDR` | `0x0000…8282` | EIP-8282 |
| `DEPOSIT_CONTRACT_ADDR` | `0x0000…05fa` | EIP-6110 |
| `EIP7708_TRANSFER_TOPIC` | `keccak256("Transfer(…)")` | EIP-7708 | -/

/-- The pseudo-caller of protocol system calls (EIP-4788, EIP-2935,
EIP-7002, EIP-7251, EIP-8282). -/
def SYSTEM_ADDRESS : address :=
  (address_from_bits 0x000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE#256)

def BEACON_ROOTS_ADDR : address :=
  (address_from_bits 0x000000000000000000000000000F3DF6D732807EF1319FB7B8BB8522D0BEAC02#256)

def HISTORY_STORAGE_ADDR : address :=
  (address_from_bits 0x0000000000000000000000000000F90827F1C53A10CB7A02335B175320002935#256)

def WITHDRAWAL_REQUEST_ADDR : address :=
  (address_from_bits 0x00000000000000000000000000000961EF480EB55E80D19AD83579A64C007002#256)

def CONSOLIDATION_REQUEST_ADDR : address :=
  (address_from_bits 0x0000000000000000000000000000BBDDC7CE488642FB579F8B00F3A590007251#256)

def BUILDER_DEPOSIT_REQUEST_ADDR : address :=
  (address_from_bits 0x0000000000000000000000000000BFF46984E3725691FA540A8C7589300D8282#256)

def BUILDER_EXIT_REQUEST_ADDR : address :=
  (address_from_bits 0x000000000000000000000000000064D678505AD48F8CCB093BC65613800E8282#256)

def DEPOSIT_CONTRACT_ADDR : address :=
  (address_from_bits 0x00000000000000000000000000000000219AB540356CBB839CBE05303D7705FA#256)

def DEPOSIT_EVENT_TOPIC : word :=
  (word_from_bits 0x649BBC62D0E31342AFEA4E5CD82D4049E7E1EE912FC0889AA790803BE39038C5#256)

def EIP7708_SYSTEM_ADDRESS : address :=
  (address_from_bits 0x000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE#256)

def EIP7708_TRANSFER_TOPIC : word :=
  (word_from_bits 0xDDF252AD1BE2C89B69C2B068FC378DAA952BA7F163C4A11628F55A4DF523B3EF#256)

def EIP7708_BURN_TOPIC : word :=
  (word_from_bits 0xCC16F5DBB4873280815C1EE09DBD06736CFFCC184412CF7A71A0FDB75D397CA5#256)

