import Evm.Kernel.Environment
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
open ast
open TxType
open TxSignatureScheme
open TrieUpdateSource
open TrieUpdateRelation
open TrieLeafValue
open TrieItemValue
open TrieChange
open StorageTxPopResult
open StorageTxLookup
open StorageBlockIterResult
open StateJournalEntry
open StackValidation
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
open OpcodeOutcome
open NodeRef
open LogTopics
open LogData
open InputTrieNode
open IndexedTrieSource
open HtrRequestKind
open HaltKind
open FrameStatus
open FrameContinuation
open FatalError
open ExceptionKind
open EnvField
open DeepStackOperation
open CreateKind
open CalldataSlice
open CallKind
open BalIterEntry
open AcctTxPopResult
open AcctBlockIterResult

/-! # State trie

Authenticated state/storage traversal and post-state update assembly over the
shared MPT and state-leaf codec. -/

/-- The post-state root: traverses every changed account in the kernel's
block-level overlay, recomputes each touched account's storage root
from its changed slots (zero-valued slots delete), re-encodes the
account leaf (empty accounts delete, per EIP-161), and streams the
ordered updates into the parent state root via [trie_root][]. -/
def compute_state_root (_ : Unit) : SailM (Vector (BitVec 8) 32) := do
  (acct_block_iter_begin ())
  let updates := (ChangedAccountTrieUpdates ())
  (pure (← (trie_root (← readReg k_parent_state_root) updates)).root)

