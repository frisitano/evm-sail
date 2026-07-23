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

/-! # Protocol forks

The protocol forks the model executes, oldest to newest. Declaration order
is the activation order, and every fork-gated rule in the specification
compares on it via [fork_gteq][] / [fork_lt][]. Pure data — no registers,
no externs. -/

def undefined_Fork (_ : Unit) : SailM Fork := do
  (internal_pick
    [Frontier, Homestead, Byzantium, Constantinople, Istanbul, Berlin, London, Paris, Shanghai, Cancun, Prague, Osaka, Amsterdam])

/- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 12 -/
def Fork_of_num (arg_ : Nat) : Fork :=
  match arg_ with
  | 0 => Frontier
  | 1 => Homestead
  | 2 => Byzantium
  | 3 => Constantinople
  | 4 => Istanbul
  | 5 => Berlin
  | 6 => London
  | 7 => Paris
  | 8 => Shanghai
  | 9 => Cancun
  | 10 => Prague
  | 11 => Osaka
  | _ => Amsterdam

def num_of_Fork (arg_ : Fork) : Nat :=
  match arg_ with
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
  | .Amsterdam => 12

