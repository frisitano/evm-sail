import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Code
import Evm.Primitives.Crypto
import Evm.Host.EvmByteSlice
import Evm.Host.Code
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

def k_code_key (a : address) : SailM hash := do
  (pure (← (k_aload a)).info.code_hash)

def k_get_codehash (a : address) : SailM hash := do
  let acc ← do (k_aload a)
  if ((! acc.present) : Bool)
  then (pure WORD_ZERO)
  else (pure acc.info.code_hash)

def k_deploy_code (a : address) (code : EvmByteSlice) : SailM Unit := do
  let cur ← do (k_aload a)
  let h ← (( do (code_db_insert code) ) : SailM (BitVec 256) )
  (store_account_info a cur { cur.info with code_hash := h })

def delegation_jumpdest_chunk (target : address) : JumpdestChunk := Id.run do
  let w := (address_to_word target)
  let bits := EMPTY_JUMPDEST_CHUNK
  let loop_k_lower := 0
  let loop_k_upper := 19
  let mut loop_vars := bits
  for k in [loop_k_lower:loop_k_upper:1]i do
    let bits := loop_vars
    loop_vars :=
      let b := (Sail.BitVec.extractLsb (w >>> (8 *i (19 -i k))) 7 0)
      if ((b == 0x5B#8) : Bool)
      then (bits ||| (WORD_ONE <<< (3 + k)))
      else bits
  (pure loop_vars)

def k_set_delegation (a : address) (target : address) : SailM Unit := do
  let cur ← do (k_aload a)
  let code_len : byte_quantity := (ByteQuantity 23)
  let table ← do (jumpdest_table_alloc code_len)
  assert (table != EMPTY_JUMPDEST_REF) "delegation JUMPDEST table allocation"
  let chunk := (delegation_jumpdest_chunk target)
  if ((chunk != EMPTY_JUMPDEST_CHUNK) : Bool)
  then
    (do
      let stored ← do (jumpdest_table_store_chunk table code_len BYTE_ZERO chunk)
      assert stored "delegation JUMPDEST chunk store")
  else (pure ())
  let h ← (( do (code_intern_delegation target table) ) : SailM (BitVec 256) )
  (store_account_info a cur { cur.info with code_hash := h })

def k_clear_code (a : address) : SailM Unit := do
  let cur ← do (k_aload a)
  (store_account_info a cur { cur.info with code_hash := KECCAK_EMPTY })

def addr_bytes (a : address) : (List byte) := Id.run do
  let w := (address_to_word a)
  let out : (List (BitVec 8)) := []
  let loop_k_lower := 0
  let loop_k_upper := 19
  let mut loop_vars := out
  for k in [loop_k_lower:loop_k_upper:1]i do
    let out := loop_vars
    loop_vars := ((Sail.BitVec.extractLsb (w >>> (8 *i k)) 7 0) :: out)
  (pure loop_vars)

def delegation_code (a : address) : (List byte) :=
  (0xEF#8 :: (0x01#8 :: (0x00#8 :: (addr_bytes a))))

def k_deleg_target (a : address) : SailM (Bool × address) := do
  let h ← (( do (k_code_key a) ) : SailM (BitVec 256) )
  let r ← (( do (code_db_read_delegation h) ) : SailM (BitVec 168) )
  (pure (((BitVec.access r 160) == 1#1), (Sail.BitVec.extractLsb r 159 0)))

def k_get_code_size (a : address) : SailM byte_length := do
  let code ← do (code_db_resolve (← (k_code_key a)))
  (pure code.bytes.len)

def k_code_copy (a : address) (dst : memory_pointer) (off : word) (len : memory_length) : SailM Unit := do
  let code ← do (code_db_resolve (← (k_code_key a)))
  (slice_copy_word_offset code.bytes dst off len)

