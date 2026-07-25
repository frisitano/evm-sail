import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Code
import Evm.Lib.Bytes
import Evm.Primitives.Crypto
import Evm.Host.EvmByteSlice
import Evm.Host.Code
import Evm.Host.Kernel.Environment
import Evm.Host.Kernel.Storage
import Evm.Host.Kernel.Accounts

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

/-! # State: account code

Account code operations over the content-addressed code store, including
EIP-7702 delegation designators. -/

/-- The account's code hash — the code-store key. -/
def k_code_key (a : (Vector (BitVec 8) 20)) : SailM (Vector (BitVec 8) 32) := do
  (pure (← (k_aload a)).info.code_hash)

/-- `EXTCODEHASH` (EIP-1052): a truly non-existent account reads as `0`,
not `KECCAK_EMPTY`. -/
def k_get_codehash (a : (Vector (BitVec 8) 20)) : SailM (Vector (BitVec 8) 32) := do
  let acc ← do (k_aload a)
  if ((! acc.present) : Bool)
  then (pure ZERO_HASH)
  else (pure acc.info.code_hash)

/-- Deploys code to an account: analyzes, stores, and binds its hash. -/
/- Type quantifiers: code_dependentWitness1 : Nat, code_dependentWitness0 : Nat, 0 ≤
  code_dependentWitness0 ∧ 0 ≤ code_dependentWitness1 ∧ 0 ≤ code_dependentWitness1 -/
def k_deploy_code (a : (Vector (BitVec 8) 20)) (code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM Unit := do
  let code_dependentWitness0 := (code).1
  let code_dependentWitness1 := ((code).2).1
  let code := ((code).2).2
  let cur ← do (k_aload a)
  let h ← (( do (code_db_insert ⟨_, ⟨_, code⟩⟩ (← readReg k_fork)) ) : SailM
    (Vector (BitVec 8) 32) )
  (store_account_info a cur { cur.info with code_hash := h })

/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ 19 -/
def delegation_address_index (index : Nat) : Nat :=
  (19 - index)

/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ 19 -/
def delegation_code_index (index : Nat) : Nat :=
  (3 + index)

/-- Marks address bytes that decode as `JUMPDEST` in a delegation designator. -/
def delegation_jumpdest_chunk (target : (Vector (BitVec 8) 20)) : (BitVec 256) := Id.run do
  let bits := EMPTY_JUMPDEST_CHUNK
  let loop_k_lower := 0
  let loop_k_upper := 19
  let mut loop_vars := bits
  for k in [loop_k_lower:loop_k_upper:1]i do
    let bits := loop_vars
    loop_vars :=
      let b := (GetElem?.getElem! target (delegation_address_index k))
      if ((b == 0x5B#8) : Bool)
      then (bits ||| ((Sail.BitVec.zeroExtend 0x01#8 256) <<< (delegation_code_index k)))
      else bits
  (pure loop_vars)

/-- Installs an EIP-7702 delegation designator
(`0xef0100 ‖ target`) as the account's code. -/
def k_set_delegation (a : (Vector (BitVec 8) 20)) (target : (Vector (BitVec 8) 20)) : SailM Unit := do
  let cur ← do (k_aload a)
  let code_len : Nat := 23
  let code_length := code_len
  let model_code_length : Nat := code_length
  let table ← do (jumpdest_table_alloc model_code_length)
  assert (table != EMPTY_JUMPDEST_REF) "delegation JUMPDEST table allocation"
  let chunk := (delegation_jumpdest_chunk target)
  if ((chunk != EMPTY_JUMPDEST_CHUNK) : Bool)
  then
    (do
      let stored ← do (jumpdest_table_store_chunk table model_code_length 0 chunk)
      assert stored "delegation JUMPDEST chunk store")
  else (pure ())
  let h ← (( do (code_intern_delegation target table) ) : SailM (Vector (BitVec 8) 32) )
  (store_account_info a cur { cur.info with code_hash := h })

/-- Resets an account's code to empty (EIP-7702 clearing). -/
def k_clear_code (a : (Vector (BitVec 8) 20)) : SailM Unit := do
  let cur ← do (k_aload a)
  (store_account_info a cur { cur.info with code_hash := KECCAK_EMPTY })

/-- The EIP-7702 delegation designator bytes: `0xef0100 ‖ address`. -/
def delegation_code (a : (Vector (BitVec 8) 20)) : (List (BitVec 8)) :=
  (0xEF#8 :: (0x01#8 :: (0x00#8 :: (address_to_bytes a))))

/-- The delegation target of an account's code, with a validity flag —
false when the code is not a designator. -/
def k_deleg_target (a : (Vector (BitVec 8) 20)) : SailM (Bool × (Vector (BitVec 8) 20)) := do
  let h ← (( do (k_code_key a) ) : SailM (Vector (BitVec 8) 32) )
  let r ← (( do (code_db_read_delegation h) ) : SailM AddressResult )
  (pure (r.success, r.address))

/-- `EXTCODESIZE`: the account's code length in bytes. -/
def k_get_code_size (a : (Vector (BitVec 8) 20)) : SailM Nat := do
  let code ← do (code_db_resolve (← (k_code_key a)))
  let ⟨_, ⟨_, bytes⟩⟩ := code.bytes
  (pure bytes.len)

/-- `EXTCODECOPY`: copies account code into frame memory, zero-padded
past the end. -/
/- Type quantifiers: k_ex416665_ : Nat, k_ex416664_ : Nat, k_ex416663_ : Nat, 0 ≤ k_ex416663_, 0
  ≤ k_ex416664_ ∧ k_ex416664_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex416665_ -/
def k_code_copy (a : (Vector (BitVec 8) 20)) (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  let code ← do (code_db_resolve (← (k_code_key a)))
  (slice_copy_word_offset code.bytes dst off len)

