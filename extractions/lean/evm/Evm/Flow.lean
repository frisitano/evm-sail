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

/- Type quantifiers: k_ex408359_ : Bool, k_ex408358_ : Bool -/
def neq_bool (x : Bool) (y : Bool) : Bool :=
  (! (x == y))

/-- The activation index of a fork: its declaration position. -/
def fork_index (f : Fork) : protocol_fork_index :=
  ⟨match f with
  | .Frontier => 0
  | .Homestead => 1
  | .Byzantium => 2
  | .Constantinople => 3
  | .Istanbul => 4
  | .Berlin => 5
  | .London => 6
  | .Paris => 7
  | .Shanghai => 8
  | .Cancun => 9
  | .Prague => 10
  | .Osaka => 11
  | .Amsterdam => 12⟩

/-- Fork `a` activates strictly before fork `b`; overloaded onto `<`. -/
def fork_lt (a : Fork) (b : Fork) : Bool :=
  (((fork_index a)).value <b ((fork_index b)).value)

/-- Fork `a` activates no earlier than fork `b`. Overloaded onto `>=` so
fork gates read `k_fork >= Berlin`. -/
def fork_gteq (a : Fork) (b : Fork) : Bool :=
  (((fork_index b)).value ≤b ((fork_index a)).value)

/- Type quantifiers: x : Int -/
def __id (x : Int) : Int :=
  x

