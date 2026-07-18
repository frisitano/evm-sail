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

/-- Type quantifiers: k_ex160612_ : Bool, k_ex160611_ : Bool -/
def neq_bool (x : Bool) (y : Bool) : Bool :=
  (! (x == y))

def byte_quantity_equal (typ_0 : byte_quantity) (typ_1 : byte_quantity) : Bool :=
  let .ByteQuantity left : byte_quantity := typ_0
  let .ByteQuantity right : byte_quantity := typ_1
  (left == right)

def gas_constant_equal (typ_0 : gas_constant) (typ_1 : gas_constant) : Bool :=
  let .GasConstant left : gas_constant := typ_0
  let .GasConstant right : gas_constant := typ_1
  (left == right)

def gas_cost_equal (typ_0 : gas_cost) (typ_1 : gas_cost) : Bool :=
  let .GasCost left : gas_cost := typ_0
  let .GasCost right : gas_cost := typ_1
  (left == right)

def gas_equal (typ_0 : gas) (typ_1 : gas) : Bool :=
  let .Gas left : gas := typ_0
  let .Gas right : gas := typ_1
  (left == right)

def gas_refund_equal (typ_0 : gas_refund) (typ_1 : gas_refund) : Bool :=
  let .GasRefund left : gas_refund := typ_0
  let .GasRefund right : gas_refund := typ_1
  (left == right)

def byte_quantity_not_equal (typ_0 : byte_quantity) (typ_1 : byte_quantity) : Bool :=
  let .ByteQuantity left : byte_quantity := typ_0
  let .ByteQuantity right : byte_quantity := typ_1
  (left != right)

def byte_quantity_le (typ_0 : byte_quantity) (typ_1 : byte_quantity) : Bool :=
  let .ByteQuantity left : byte_quantity := typ_0
  let .ByteQuantity right : byte_quantity := typ_1
  (left ≤b right)

def gas_cost_le (typ_0 : gas_cost) (typ_1 : gas_cost) : Bool :=
  let .GasCost left : gas_cost := typ_0
  let .GasCost right : gas_cost := typ_1
  (left ≤b right)

def gas_cost_le_gas (typ_0 : gas_cost) (typ_1 : gas) : Bool :=
  let .GasCost cost : gas_cost := typ_0
  let .Gas limit : gas := typ_1
  (cost ≤b limit)

def gas_le (typ_0 : gas) (typ_1 : gas) : Bool :=
  let .Gas left : gas := typ_0
  let .Gas right : gas := typ_1
  (left ≤b right)

def byte_quantity_lt (typ_0 : byte_quantity) (typ_1 : byte_quantity) : Bool :=
  let .ByteQuantity left : byte_quantity := typ_0
  let .ByteQuantity right : byte_quantity := typ_1
  (left <b right)

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

def fork_lt (a : Fork) (b : Fork) : Bool :=
  (((fork_index a)).value <b ((fork_index b)).value)

def gas_cost_lt (typ_0 : gas_cost) (typ_1 : gas_cost) : Bool :=
  let .GasCost left : gas_cost := typ_0
  let .GasCost right : gas_cost := typ_1
  (left <b right)

def gas_lt (typ_0 : gas) (typ_1 : gas) : Bool :=
  let .Gas left : gas := typ_0
  let .Gas right : gas := typ_1
  (left <b right)

def gas_lt_cost (typ_0 : gas) (typ_1 : gas_cost) : Bool :=
  let .Gas limit : gas := typ_0
  let .GasCost cost : gas_cost := typ_1
  (limit <b cost)

def byte_quantity_ge (typ_0 : byte_quantity) (typ_1 : byte_quantity) : Bool :=
  let .ByteQuantity left : byte_quantity := typ_0
  let .ByteQuantity right : byte_quantity := typ_1
  (left ≥b right)

def fork_gteq (a : Fork) (b : Fork) : Bool :=
  (((fork_index b)).value ≤b ((fork_index a)).value)

def byte_quantity_gt (typ_0 : byte_quantity) (typ_1 : byte_quantity) : Bool :=
  let .ByteQuantity left : byte_quantity := typ_0
  let .ByteQuantity right : byte_quantity := typ_1
  (left >b right)

/-- Type quantifiers: x : Int -/
def __id (x : Int) : Int :=
  x

