import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Quantities
import Evm.Crypto
import Evm.Scratch
import Evm.Rlp
import Evm.Environment
import Evm.Storage
import Evm.Accounts
import Evm.Updates
import Evm.Trie

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open ConcurrencyInterfaceV1

namespace Evm.Functions

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

def storage_value_changed (value : StorageValue) : Bool :=
  (! (value.curr == value.orig))

def account_value_changed (value : AcctValue) : Bool :=
  ((! (value.curr.info.nonce == value.orig.info.nonce)) || ((! (value.curr.info.balance == value.orig.info.balance)) || ((! (value.curr.info.storage_root == value.orig.info.storage_root)) || ((! (value.curr.info.code_hash == value.orig.info.code_hash)) || ((! (value.curr.exists == value.orig.exists)) || (! (value.curr.storage_cleared == value.orig.storage_cleared)))))))

def encode_storage_value (value : (BitVec 256)) : SailM ByteSlice := do
  let encoded_len ← do (rlp_uint_word_size value)
  let start ← do (scratch_begin ())
  (rlp_write_uint_word value)
  (rlp_finish start encoded_len)

def encode_state_account (info : AccountInfo) (storage_root : (BitVec 256)) : SailM ByteSlice := do
  let content_len ← do (rlp_protocol_quantity_size info.nonce)
  let content_len ← (byte_quantity_add content_len (← (rlp_uint_word_size info.balance)))
  let content_len ← (byte_quantity_add content_len (rlp_word_size ()))
  let content_len ← (byte_quantity_add content_len (rlp_word_size ()))
  let encoded_len ← do (rlp_list_size content_len)
  let start ← do (scratch_begin ())
  (rlp_write_list_prefix content_len)
  (rlp_write_protocol_quantity info.nonce)
  (rlp_write_uint_word info.balance)
  (rlp_write_word storage_root)
  (rlp_write_word info.code_hash)
  (rlp_finish start encoded_len)

def storage_updates (addr : (BitVec 160)) : SailM (List TrieUpdate) := do
  let remaining ← (( do (storage_block_count addr) ) : SailM item_count )
  let updates : (List TrieUpdate) := []
  let (remaining, updates) ← (( do
    let mut loop_vars := (remaining, updates)
    while (λ (remaining, updates) => (remaining != 0)) loop_vars do
      let (remaining, updates) := loop_vars
      loop_vars ← do
        let remaining ← (protocol_quantity_decrement remaining)
        let index := remaining
        let updates ← (( do
          match (← (storage_block_row addr index)) with
          | .some entry =>
            (do
              if ((storage_value_changed entry.value) : Bool)
              then
                (do
                  let key ← do (pure (path_from_hash (← (keccak256_word entry.key.slot))))
                  let change ← do
                    if ((word_is_zero entry.value.curr) : Bool)
                    then (pure (TrieDelete ()))
                    else (pure (TriePut (← (encode_storage_value entry.value.curr))))
                  ({ key := key
                     change := change } :: updates))
              else (pure updates))
          | none => sailThrow ((InvalidBlock WitnessDeficient)) ) : SailM (List TrieUpdate) )
        (pure (remaining, updates))
    (pure loop_vars) ) : SailM (Nat × (List TrieUpdate)) )
  (pure updates)

def account_update (entry : AcctEntry) (storage : (List TrieUpdate)) : SailM TrieUpdate := do
  let current := entry.value.curr
  let key ← do (pure (path_from_hash (← (keccak256_address entry.addr))))
  if (((! current.exists) || (account_info_empty current.info)) : Bool)
  then
    (pure { key := key
            change := (TrieDelete ()) })
  else
    (do
      let storage_root ← do
        (trie_root
          (if (current.storage_cleared : Bool)
          then EMPTY_TRIE_ROOT
          else current.info.storage_root) storage)
      (pure { key := key
              change := ← (pure (TriePut (← (encode_state_account current.info storage_root)))) }))

def compute_state_root (_ : Unit) : SailM (BitVec 256) := do
  let remaining ← (( do (acct_block_count ()) ) : SailM item_count )
  let accounts : (List TrieUpdate) := []
  let (accounts, remaining) ← (( do
    let mut loop_vars := (accounts, remaining)
    while (λ (accounts, remaining) => (remaining != 0)) loop_vars do
      let (accounts, remaining) := loop_vars
      loop_vars ← do
        let remaining ← (protocol_quantity_decrement remaining)
        let index := remaining
        let accounts ← (( do
          match (← (acct_block_row index)) with
          | .some entry =>
            (do
              let storage ← do (storage_updates entry.addr)
              if (((account_value_changed entry.value) || (! (updates_empty storage))) : Bool)
              then
                (do
                  ((← (account_update entry storage)) :: accounts))
              else (pure accounts))
          | none => sailThrow ((InvalidBlock WitnessDeficient)) ) : SailM (List TrieUpdate) )
        (pure (accounts, remaining))
    (pure loop_vars) ) : SailM ((List TrieUpdate) × Nat) )
  (trie_root (← readReg k_parent_state_root) accounts)

