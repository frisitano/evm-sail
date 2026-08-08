import Evm.Primitives.Code

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

/-! # The instruction set

The abstract syntax of EVM bytecode: the full defined opcode set through
the Osaka fork (Yellow Paper Appendix H, plus the EIPs that extend it —
`PUSH0` EIP-3855, `SHL`/`SHR`/`SAR` EIP-145, `CLZ` EIP-7939,
`TLOAD`/`TSTORE` EIP-1153, `MCOPY` EIP-5656, `BLOBHASH`/`BLOBBASEFEE`
EIP-4844/EIP-7516, and `DUPN`/`SWAPN`/`EXCHANGE` EIP-8024). -/

/-- Decodes the immediate shared by EIP-8024 `DUPN` and `SWAPN` into
their one-based deep-stack index (17–235). -/
def decode_single_stack_index (immediate : (BitVec 8)) : SailM Nat := do
  assert (deep_stack_immediate_valid immediate) "sail/evm/instructions.sail:72.48-72.49"
  let value : Nat := (BitVec.toNatInt immediate)
  if ((value ≤b 90) : Bool)
  then (pure (value + 145))
  else
    (do
      assert (128 ≤b value) "sail/evm/instructions.sail:77.27-77.28"
      (pure (value - 111)))

/-- Decodes the EIP-8024 `EXCHANGE` immediate into the two zero-based
stack depths that it exchanges. -/
def decode_exchange_stack_indices (immediate : (BitVec 8)) : SailM (Nat × Nat) := do
  assert (exchange_immediate_valid immediate) "sail/evm/instructions.sail:85.46-85.47"
  let shifted : (BitVec 8) := (immediate ^^^ 0x8F#8)
  let quotient : Nat := (BitVec.toNatInt (Sail.BitVec.extractLsb shifted 7 4))
  let remainder : Nat := (BitVec.toNatInt (Sail.BitVec.extractLsb shifted 3 0))
  if ((quotient <b remainder) : Bool)
  then (pure ((quotient + 1), (remainder + 1)))
  else (pure ((remainder + 1), (29 - quotient)))

