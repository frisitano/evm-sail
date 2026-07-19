import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Crypto
import Evm.Host.Kernel.Scratch
import Evm.Lib.Rlp.Rlp
import Evm.Host.Kernel.Environment
import Evm.Host.Kernel.Storage
import Evm.Host.Kernel.Accounts
import Evm.Lib.Mpt.Updates
import Evm.Lib.Mpt.Trie

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

open word
open option
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open b256
open ast
open address
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
open FrameContinuation
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

/-! # The state trie

Ethereum account and storage tries over the generic MPT core: secure-trie
reads for stateless execution, and the post-state root computation
(YP §4.1). -/

def storage_value_changed (value : StorageValue) : Bool :=
  (! (value.curr == value.orig))

/-- Whether any persisted account field or lifecycle marker changed. -/
def account_value_changed (value : AcctValue) : Bool :=
  ((! ((value.curr.info.nonce).value == (value.orig.info.nonce).value)) || ((! (value.curr.info.balance == value.orig.info.balance)) || ((! (value.curr.info.storage_root == value.orig.info.storage_root)) || ((! (value.curr.info.code_hash == value.orig.info.code_hash)) || ((! (value.curr.present == value.orig.present)) || (! (value.curr.storage_cleared == value.orig.storage_cleared)))))))

/-- Encodes a nonzero storage value as its minimal RLP integer leaf payload. -/
def encode_storage_value (value : word) : SailM EvmByteSlice := do
  let encoded_len ← do (rlp_uint_word_size value)
  let start ← do (scratch_begin ())
  (rlp_write_uint_word value)
  (rlp_finish start encoded_len)

/-- Encodes an account trie leaf with its recomputed storage root. -/
def encode_state_account (info : AccountInfo) (storage_root : hash) : SailM EvmByteSlice := do
  let content_len ← do (rlp_protocol_quantity_size ⟨(info.nonce).value⟩)
  let content_len ← (byte_quantity_add content_len (← (rlp_uint_word_size info.balance)))
  let content_len ← (byte_quantity_add content_len (rlp_word_size ()))
  let content_len ← (byte_quantity_add content_len (rlp_word_size ()))
  let encoded_len ← do (rlp_list_size content_len)
  let start ← do (scratch_begin ())
  (rlp_write_list_prefix content_len)
  (rlp_write_protocol_quantity ⟨(info.nonce).value⟩)
  (rlp_write_uint_word info.balance)
  (rlp_write_word (hash_to_word storage_root))
  (rlp_write_word (hash_to_word info.code_hash))
  (rlp_finish start encoded_len)

/-- Collects changed storage slots of an account as ascending secure-trie
updates. -/
def storage_updates (addr : address) : SailM (List TrieUpdate) := do
  let remaining ← (( do
    (do
        let semanticResult ← (storage_block_count addr)
        pure ((semanticResult).value)) ) : SailM Nat )
  let updates : (List TrieUpdate) := []
  let (remaining, updates) ← (( do
    let loop_vars ← whileFuelM (fuel :=remaining) (fun (remaining, updates) => (pure (remaining != 0))) (remaining, updates)
      fun (remaining, updates) => do
        assert true "loop dummy assert"
        let remaining ←
          (do
              let semanticResult ← (protocol_quantity_decrement ⟨remaining⟩)
              pure ((semanticResult).value))
        let index := remaining
        let updates ← (( do
          match (← (storage_block_row addr ⟨index⟩)) with
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
                  (pure ({ key := key,
                           change := change } :: updates)))
              else (pure updates))
          | none => sailThrow ((InvalidBlock WitnessDeficient)) ) : SailM (List TrieUpdate) )
        (pure (remaining, updates))
    (pure loop_vars) ) : SailM (Nat × (List TrieUpdate)) )
  (pure updates)

/-- Converts one changed account and its storage updates into a state-trie
insertion or deletion. -/
def account_update (entry : AcctEntry) (storage : (List TrieUpdate)) : SailM TrieUpdate := do
  let current := entry.value.curr
  let key ← do (pure (path_from_hash (← (keccak256_address entry.addr))))
  if (((! current.present) || (account_info_empty current.info)) : Bool)
  then
    (pure { key := key,
            change := (TrieDelete ()) })
  else
    (do
      let storage_root ← do
        (trie_root
          (if (current.storage_cleared : Bool)
          then EMPTY_TRIE_ROOT
          else current.info.storage_root) storage)
      (pure { key := key,
              change := ← (pure (TriePut (← (encode_state_account current.info storage_root)))) }))

/-- The post-state root: drains every changed account from the kernel's
block-level overlay, recomputes each touched account's storage root
from its changed slots (zero-valued slots delete), re-encodes the
account leaf (empty accounts delete, per EIP-161), and folds the
sorted update list into the parent state root via [trie_root][]. -/
def compute_state_root (_ : Unit) : SailM hash := do
  let remaining ← (( do
    (do
        let semanticResult ← (acct_block_count ())
        pure ((semanticResult).value)) ) : SailM Nat )
  let accounts : (List TrieUpdate) := []
  let (accounts, remaining) ← (( do
    let loop_vars ← whileFuelM (fuel :=remaining) (fun (accounts, remaining) => (pure (remaining != 0))) (accounts, remaining)
      fun (accounts, remaining) => do
        assert true "loop dummy assert"
        let remaining ←
          (do
              let semanticResult ← (protocol_quantity_decrement ⟨remaining⟩)
              pure ((semanticResult).value))
        let index := remaining
        let accounts ← (( do
          match (← (acct_block_row ⟨index⟩)) with
          | .some entry =>
            (do
              let storage ← do (storage_updates entry.addr)
              if (((account_value_changed entry.value) || (! (updates_empty storage))) : Bool)
              then
                (do
                  (pure ((← (account_update entry storage)) :: accounts)))
              else (pure accounts))
          | none => sailThrow ((InvalidBlock WitnessDeficient)) ) : SailM (List TrieUpdate) )
        (pure (accounts, remaining))
    (pure loop_vars) ) : SailM ((List TrieUpdate) × Nat) )
  (trie_root (← readReg k_parent_state_root) accounts)

