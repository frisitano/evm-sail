import Evm.Primitives.Crypto
import Evm.Host.Kernel.Environment
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

/-! # The state trie

Ethereum account and storage tries over the generic MPT core: secure-trie
reads for stateless execution, and the post-state root computation
(YP §4.1). -/

/-- Derives and caches one account's post-state storage root. Keeping this
preparation separate from the account-update pull source makes the nested
storage walk statically acyclic while retaining lazy ordered update streams. -/
def prepare_account_post_storage_root (entry : AcctEntry) : SailM Unit := do
  let current := entry.value.curr
  (storage_block_iter_begin entry.addr)
  let storage_updates ← do (trie_updates_begin (StorageTrieUpdates entry.addr))
  let base_storage_root :=
    if (current.storage_cleared : Bool)
    then EMPTY_TRIE_ROOT
    else current.info.storage_root
  let storage_root ← do
    if (((! current.present) || (account_info_empty current.info)) : Bool)
    then (pure base_storage_root)
    else (pure (← (trie_root_cursor base_storage_root storage_updates)).root)
  (acct_post_storage_root_store entry.addr storage_root)

/-- Prepares post-state storage roots for every account update candidate. -/
def prepare_changed_account_post_storage_roots (_ : Unit) : SailM Unit := do
  let preparing : Bool := true
  let preparing ← (( do
    let loop_vars ← whileFuelM (fuel :=(2 ^i 64)) (fun preparing => (pure preparing)) preparing
      fun preparing => do
        assert true "loop dummy assert"
        match (← (acct_block_iter_next ())) with
        | .some entry =>
          (do
            (prepare_account_post_storage_root entry)
            (pure preparing))
        | none =>
          (let preparing : Bool := false
          (pure preparing))
    (pure loop_vars) ) : SailM Bool )
  (pure ())

/-- The post-state root: traverses every changed account in the kernel's
block-level overlay, recomputes each touched account's storage root
from its changed slots (zero-valued slots delete), re-encodes the
account leaf (empty accounts delete, per EIP-161), and streams the
ordered updates into the parent state root via [trie_root][]. -/
def compute_state_root (_ : Unit) : SailM hash := do
  (acct_block_iter_begin ())
  (prepare_changed_account_post_storage_roots ())
  (acct_block_iter_begin ())
  (pure (← (trie_root (← readReg k_parent_state_root) (ChangedAccountTrieUpdates ()))).root)

