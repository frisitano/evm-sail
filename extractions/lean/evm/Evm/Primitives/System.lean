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

noncomputable section
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
open Register
open NodeRef
open MerkleSlot
open JEntry
open HaltKind
open FrameStatus
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

def SYSTEM_ADDRESS : address := 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE#160

def BEACON_ROOTS_ADDR : address := 0x000F3DF6D732807EF1319FB7B8BB8522D0BEAC02#160

def HISTORY_STORAGE_ADDR : address := 0x0000F90827F1C53A10CB7A02335B175320002935#160

def WITHDRAWAL_REQUEST_ADDR : address := 0x00000961EF480EB55E80D19AD83579A64C007002#160

def CONSOLIDATION_REQUEST_ADDR : address := 0x0000BBDDC7CE488642FB579F8B00F3A590007251#160

def DEPOSIT_CONTRACT_ADDR : address := 0x00000000219AB540356CBB839CBE05303D7705FA#160

def DEPOSIT_EVENT_TOPIC : word :=
  0x649BBC62D0E31342AFEA4E5CD82D4049E7E1EE912FC0889AA790803BE39038C5#256

def EIP7708_SYSTEM_ADDRESS : address := 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE#160

def EIP7708_TRANSFER_TOPIC : word :=
  0xDDF252AD1BE2C89B69C2B068FC378DAA952BA7F163C4A11628F55A4DF523B3EF#256

def EIP7708_BURN_TOPIC : word :=
  0xCC16F5DBB4873280815C1EE09DBD06736CFFCC184412CF7A71A0FDB75D397CA5#256

