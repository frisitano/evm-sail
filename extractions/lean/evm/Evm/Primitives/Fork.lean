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

def undefined_Fork (_ : Unit) : SailM Fork := do
  (internal_pick
    [Frontier, Homestead, Byzantium, Constantinople, Istanbul, Berlin, London, Paris, Shanghai, Cancun, Prague, Osaka, Amsterdam])

/-- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 12 -/
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

def num_of_Fork (arg_ : Fork) : Int :=
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

/-- Type quantifiers: idx : Nat, 0 ≤ idx ∧ idx ≤ (2 ^ 64 - 1) -/
def fork_of_protocol_index (idx : protocol_fork_index) : Fork :=
  let idx := (idx).value
  if ((20 ≤b idx) : Bool)
  then Amsterdam
  else
    (if ((17 ≤b idx) : Bool)
    then Osaka
    else
      (if ((idx == 16) : Bool)
      then Prague
      else
        (if ((idx == 15) : Bool)
        then Cancun
        else
          (if ((idx == 14) : Bool)
          then Shanghai
          else
            (if ((idx == 13) : Bool)
            then Paris
            else
              (if (((10 ≤b idx) && (idx ≤b 12)) : Bool)
              then London
              else
                (if ((idx == 9) : Bool)
                then Berlin
                else
                  (if ((idx == 7) : Bool)
                  then Istanbul
                  else
                    (if ((idx == 5) : Bool)
                    then Byzantium
                    else
                      (if ((idx == 1) : Bool)
                      then Homestead
                      else Frontier))))))))))

