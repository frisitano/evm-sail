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

def undefined_StateCheckpoint (_ : Unit) : SailM StateCheckpoint := do
  (pure { journal := ← (undefined_range 0 ((2 ^i 64) -i 1))
          accounts := ← (undefined_range 0 ((2 ^i 64) -i 1))
          storage := ← (undefined_range 0 ((2 ^i 64) -i 1))
          logs := ← (undefined_range 0 ((2 ^i 64) -i 1)) })

