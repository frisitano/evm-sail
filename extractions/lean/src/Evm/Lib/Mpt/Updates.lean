import Evm.Flow
import Evm.Prelude
import Evm.Exceptions
import Evm.Primitives.Crypto
import Evm.Lib.Rlp.Encoding
import Evm.Kernel.Storage
import Evm.Kernel.Accounts
import Evm.Lib.Mpt.Primitives
import Evm.Lib.Mpt.Codec

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
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
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

/-! # Trie updates and subtree assembly

Ordered leaf updates, their structural items, and the recursive assembly that
recomposes canonical nodes (YP Appendix D).

An account update carries its account's post-state storage root, so pulling
one account update runs this same builder one trie level down and the cursor,
the source pull, and the builder form a single recursive group. Every member
of that group declares the same constant termination measure, 1024, which
bounds the nested call depth of a complete state-root computation: at most two
trie levels, each at most 65 nibble depths deep, each depth reached through at
most four nested builder calls, plus the constant pull chain that opens the
nested level. The measure is a proof-obligation budget only; it is erased from
the executable backends. -/

/-- The payload sentinel used by exhausted source pulls. -/
def EMPTY_TRIE_UPDATE : TrieUpdate :=
  { key := (path_empty ()),
    change := (TrieDelete ()) }

/-- Encodes an account trie leaf with its recomputed storage root. -/
def encode_state_account (info : AccountInfo) (storage_root : (Vector (BitVec 8) 32)) : SailM (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))) := do
  let nonce_length := (rlp_uint_word_size info.nonce)
  let balance_length := (rlp_uint_word_size info.balance)
  let storage_root_length := (rlp_word_size ())
  let code_hash_length := (rlp_word_size ())
  let content_len := (((nonce_length + balance_length) + storage_root_length) + code_hash_length)
  let encoded_length ← do (rlp_list_size content_len)
  let encoder ← do (rlp_encoder_begin encoded_length)
  (rlp_write_list_prefix content_len)
  (rlp_write_uint_word info.nonce)
  (rlp_write_uint_word info.balance)
  let storage_root_word := (hash_to_word storage_root)
  (rlp_write_word storage_root_word)
  let code_hash_word := (hash_to_word info.code_hash)
  (rlp_write_word code_hash_word)
  (rlp_encoder_finish encoder)

/-- Converts one account row and its recomputed storage root into a
state-trie insertion or deletion. -/
def account_update (trie_entry : AcctTrieEntry) (storage_root : (Vector (BitVec 8) 32)) : SailM TrieUpdate := do
  let entry := trie_entry.entry
  let current := entry.value.curr
  let key := (path_new trie_entry.address_hash 64)
  let account_absent := (! current.present)
  let account_empty := (account_info_empty current.info)
  if ((account_absent || account_empty) : Bool)
  then
    (pure { key := key,
            change := (TrieDelete ()) })
  else
    (do
      let ⟨_, ⟨_, encoded_account⟩⟩ ← do (encode_state_account current.info storage_root)
      (pure { key := key,
              change := (TriePut ⟨_, ⟨_, encoded_account⟩⟩) }))

/-- Whether any persisted account field changed across the block. -/
def account_value_changed (value : AcctValue) : Bool :=
  ((! (value.curr.info.nonce == value.orig.info.nonce)) || (((! (value.curr.info.balance == value.orig.info.balance)) || ((! (value.curr.info.storage_root == value.orig.info.storage_root)) || ((! (value.curr.info.code_hash == value.orig.info.code_hash)) || ((! (value.curr.present == value.orig.present)) || (! (value.curr.storage_cleared == value.orig.storage_cleared)))))) : Bool))

/-- The child reference of a single-item subtree at `depth`: the item's
remaining path is absorbed into it. This is the one place a delete
collapse can demand node material: an unknown-type hash reference
absorbing a nonempty suffix resolves its node from the witness db
(fail-closed). -/
/- Type quantifiers: k_ex553583_ : Nat, 0 ≤ k_ex553583_ ∧ k_ex553583_ ≤ 64 -/
def trie_child_ref (it : TrieItem) (depth : Nat) : SailM NodeRef := do
  let suffix ← do (path_drop it.path depth)
  let suffix_len := (path_len suffix)
  match it.value with
  | .EmptySubtree () => (pure (EmptyRef ()))
  | .LeafItem value => (leaf_child_ref suffix value)
  | .BranchItem subref =>
    (do
      if ((suffix_len == 0) : Bool)
      then (pure subref)
      else (merge_ext_ref suffix subref))
  | .SubtreeItem subref =>
    (do
      if ((suffix_len == 0) : Bool)
      then (pure subref)
      else
        (do
          match subref with
          | .HashRef h =>
            (do
              let ⟨_, ⟨_, node⟩⟩ ← do (node_db_lookup h)
              if ((node.len == 0) : Bool)
              then (fatal_error WitnessDeficient)
              else (merge_ext_node suffix ⟨_, ⟨_, node⟩⟩))
          | _ => (merge_ext_ref suffix subref)))

/-- Converts a recursively assembled top-level item to the committed root. -/
def trie_subtree_root (subtree : TrieItem) : SailM (Vector (BitVec 8) 32) := do
  match subtree.value with
  | .EmptySubtree () => (pure EMPTY_TRIE_ROOT)
  | _ =>
    (do
      let root_ref ← do (trie_child_ref subtree 0)
      (trie_ref_to_root root_ref))

/-- Whether the pull cursor has reached the end of its source. -/
def updates_empty (updates : TrieUpdateCursor) : Bool :=
  match updates.relation with
  | .UpdateSourceExhausted _ => true
  | _ => false

/-- Adds one recursively built child at `index`; an empty subtree
contributes nothing. -/
def trie_children_add (children : TrieChildren) (evm_prefix' : TriePath) (index : (BitVec 4)) (child : TrieItem) : SailM TrieChildren := do
  match child.value with
  | .EmptySubtree () => (pure children)
  | _ =>
    (do
      let depth := (path_len evm_prefix')
      let child_count := children.count
      let child_depth ← (( do
        if ((depth <b 64) : Bool)
        then (pure (depth + 1))
        else (fatal_error WitnessDeficient) ) : SailM Nat )
      let next_child_count ← (( do
        if ((child_count <b 16) : Bool)
        then (pure (child_count + 1))
        else (fatal_error WitnessDeficient) ) : SailM Nat )
      let child_segment ← do (path_single index)
      let child_prefix ← do (path_concat evm_prefix' child_segment)
      let path_below_child ← do (path_prefix_of child_prefix child.path)
      let path_outside_child := (! path_below_child)
      if (path_outside_child : Bool)
      then (fatal_error WitnessDeficient)
      else (pure ())
      let child_already_present := (branch_mask_has children.mask index)
      if (child_already_present : Bool)
      then (fatal_error WitnessDeficient)
      else (pure ())
      let updated := children
      let updated : TrieChildren := { updated with mask := (branch_mask_set updated.mask index) }
      let updated ←
        (pure { updated with children := ← (pure (vectorUpdate updated.children
                (BitVec.toNatInt index) (← (trie_child_ref child child_depth)))) })
      let updated : TrieChildren := { updated with only := child }
      (pure { updated with count := next_child_count }))

/-- The absent subtree (YP `n(I,i) = ()`). Its path carries no meaning. -/
def trie_empty_subtree (_ : Unit) : TrieItem :=
  { path := (path_empty ()),
    value := (EmptySubtree ()) }

/-- Constructs an empty child accumulator. `only` is meaningful iff exactly
one child has been added. -/
def trie_children_empty (_ : Unit) : TrieChildren :=
  let empty_ref := (EmptyRef ())
  let children := (vectorInit empty_ref)
  let empty_subtree := (trie_empty_subtree ())
  { mask := 0x0000#16,
    children := children,
    only := empty_subtree,
    count := 0 }

def trie_branch (path : TriePath) (childref : NodeRef) : TrieItem :=
  { path := path,
    value := (BranchItem childref) }

/-- Finishes a recursive branch: zero children disappear, one child bubbles
upward structurally, and multiple children form a canonical branch item. -/
def trie_children_finish (evm_prefix' : TriePath) (children : TrieChildren) : SailM TrieItem := do
  if ((children.count == 0) : Bool)
  then
    (pure { path := evm_prefix',
            value := (EmptySubtree ()) })
  else
    (do
      if ((children.count == 1) : Bool)
      then (pure children.only)
      else
        (do
          let branch_ref ← do (branch_child_ref children.mask children.children)
          (pure (trie_branch evm_prefix' branch_ref))))

def trie_leaf (path : TriePath) (value : TrieLeafValue) : TrieItem :=
  { path := path,
    value := (LeafItem value) }

/-- Constructs a leaf item over freshly encoded scratch bytes. -/
/- Type quantifiers: value_dependentWitness1 : Nat, value_dependentWitness0 : Nat, 0 ≤
  value_dependentWitness0 ∧
  0 ≤ value_dependentWitness1 ∧
  (value_dependentWitness0 + value_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def trie_scratch_leaf (path : TriePath) (value : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : TrieItem :=
  let value_dependentWitness0 := (value).1
  let value_dependentWitness1 := ((value).2).1
  let value := ((value).2).2
  let leaf_value := (ScratchTrieLeaf ⟨_, ⟨_, value⟩⟩)
  (trie_leaf path leaf_value)

/-- Moves an under-evm_prefix active update through one selected child edge. -/
def trie_updates_descend (updates : TrieUpdateCursor) : SailM TrieUpdateCursor := do
  match updates.relation with
  | .UpdateUnderPrefix path_postfix =>
    (do
      let postfix_len := (path_len path_postfix)
      if ((postfix_len == 0) : Bool)
      then (fatal_error WitnessDeficient)
      else
        (do
          let child_postfix ← do (path_drop path_postfix 1)
          let descended := updates
          (pure { descended with relation := (UpdateUnderPrefix child_postfix) })))
  | .UpdateBeyondPrefix _ => (fatal_error WitnessDeficient)
  | .UpdateSourceExhausted _ => (fatal_error WitnessDeficient)

/-- Reinterprets an already-loaded successor at an ancestor evm_prefix. No source
item is fetched and no evm_prefix scan is repeated: the adjacent-key common
evm_prefix computed by [trie_updates_pop][] decides whether the successor has
reached the subtree where traversal should resume. -/
def trie_updates_rebase (updates : TrieUpdateCursor) (evm_prefix' : TriePath) : SailM TrieUpdateCursor := do
  let prefix_len := (path_len evm_prefix')
  match updates.relation with
  | .UpdateSourceExhausted _ => (pure updates)
  | .UpdateUnderPrefix _ => (pure updates)
  | .UpdateBeyondPrefix common_prefix_len =>
    (do
      if ((prefix_len ≤b common_prefix_len) : Bool)
      then
        (do
          let path_postfix ← do (path_drop updates.current.key prefix_len)
          (pure { source := updates.source,
                  current := updates.current,
                  relation := (UpdateUnderPrefix path_postfix) }))
      else (pure updates))

/-- The active update's next child nibble. -/
def update_child_nibble (updates : TrieUpdateCursor) : SailM (BitVec 4) := do
  match updates.relation with
  | .UpdateUnderPrefix path_postfix =>
    (do
      let postfix_len := (path_len path_postfix)
      if ((postfix_len == 0) : Bool)
      then (fatal_error WitnessDeficient)
      else (path_nibble path_postfix 0))
  | .UpdateBeyondPrefix _ => (fatal_error WitnessDeficient)
  | .UpdateSourceExhausted _ => (fatal_error WitnessDeficient)

/-- Whether the active update belongs to the subtree at the cursor's current
evm_prefix. This inspects only the relation already carried by the cursor; it
never pulls or compares another source key. -/
def update_under_current_prefix (updates : TrieUpdateCursor) : Bool :=
  match updates.relation with
  | .UpdateUnderPrefix _ => true
  | .UpdateBeyondPrefix _ => false
  | .UpdateSourceExhausted _ => false

/-- Remaining sibling-key order after the active update's child nibble.
Recursive consumption returns only a strictly later sibling range. -/
def update_child_ranges_remaining (updates : TrieUpdateCursor) : SailM Nat := do
  let update_pending := (update_under_current_prefix updates)
  if (update_pending : Bool)
  then
    (do
      let child_nibble ← do (update_child_nibble updates)
      let child_index := (BitVec.toNatInt child_nibble)
      (pure (16 - child_index)))
  else (pure 0)

/-- Remaining event order while one existing path is merged with update child
ranges. The next event is either that path's nibble or the active update's
nibble, whichever is earlier. -/
/- Type quantifiers: k_ex553590_ : Bool -/
def overlay_child_ranges_remaining (updates : TrieUpdateCursor) (existing_pending : Bool) (existing_nibble : (BitVec 4)) : SailM Nat := do
  let update_pending := (update_under_current_prefix updates)
  let update_position ← (( do
    if (update_pending : Bool)
    then
      (do
        let child_nibble ← do (update_child_nibble updates)
        (pure (BitVec.toNatInt child_nibble)))
    else (pure 16) ) : SailM Nat )
  let existing_position : Nat :=
    if (existing_pending : Bool)
    then (BitVec.toNatInt existing_nibble)
    else 16
  let next_position :=
    if ((update_position <b existing_position) : Bool)
    then update_position
    else existing_position
  (pure (17 - next_position))

/-- Constructs a leaf item over immutable input bytes. -/
/- Type quantifiers: value_dependentWitness1 : Nat, value_dependentWitness0 : Nat, 0 ≤
  value_dependentWitness0 ∧
  0 ≤ value_dependentWitness1 ∧
  (value_dependentWitness0 + value_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def trie_input_leaf (path : TriePath) (value : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : TrieItem :=
  let value_dependentWitness0 := (value).1
  let value_dependentWitness1 := ((value).2).1
  let value := ((value).2).2
  let leaf_value := (InputTrieLeaf ⟨_, ⟨_, value⟩⟩)
  (trie_leaf path leaf_value)

def trie_subtree (path : TriePath) (childref : NodeRef) : TrieItem :=
  { path := path,
    value := (SubtreeItem childref) }

/-- Encodes a nonzero storage value as its minimal RLP integer leaf payload. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def encode_storage_value (value : Nat) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))) := do
  let encoded_len := (rlp_uint_word_size value)
  let encoder ← do (rlp_encoder_begin encoded_len)
  (rlp_write_uint_word value)
  (rlp_encoder_finish encoder)

/-- Converts one changed storage row into a secure-trie update. -/
def storage_update (trie_entry : StorageTrieEntry) : SailM TrieUpdate := do
  let entry := trie_entry.entry
  let key := (path_new trie_entry.slot_hash 64)
  let value_is_zero := (word_is_zero entry.value.curr)
  let change ← do
    if (value_is_zero : Bool)
    then (pure (TrieDelete ()))
    else
      (do
        let ⟨_, ⟨_, encoded_value⟩⟩ ← do (encode_storage_value entry.value.curr)
        (pure (TriePut ⟨_, ⟨_, encoded_value⟩⟩)))
  (pure { key := key,
          change := change })

def storage_value_changed (value : StorageValue) : Bool :=
  (! (value.curr == value.orig))

/-- Pulls the next changed storage row and converts it to a trie update,
skipping materialized read-only rows. -/
def next_storage_trie_update (addr : (Vector (BitVec 8) 20)) : SailM TrieUpdateFetch := do
  let searching : Bool := true
  let result : TrieUpdateFetch :=
    { available := false,
      update := EMPTY_TRIE_UPDATE }
  let (result, searching) ← (( do
    let loop_vars ← whileFuelM (fuel :=(2 ^i 64)) (fun (result, searching) => (pure searching)) (result, searching)
      fun (result, searching) => do
        assert true "loop dummy assert"
        let iterator_item ← do (storage_block_iter_next addr)
        let (result, searching) ← (( do
          match iterator_item with
          | .StorageBlockIterRow trie_entry =>
            (do
              let changed := (storage_value_changed trie_entry.entry.value)
              let (result, searching) ← (( do
                if (changed : Bool)
                then
                  (do
                    let result ←
                      (pure { available := true,
                              update := ← (storage_update trie_entry) })
                    let searching : Bool := false
                    (pure (result, searching)))
                else (pure (result, searching)) ) : SailM (TrieUpdateFetch × Bool) )
              (pure (result, searching)))
          | .StorageBlockIterExhausted _ =>
            (let searching : Bool := false
            (pure (result, searching))) ) : SailM (TrieUpdateFetch × Bool) )
        (pure (result, searching))
    (pure loop_vars) ) : SailM (TrieUpdateFetch × Bool) )
  (pure result)


mutual
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_account_trie_update (trie_entry : AcctTrieEntry) (_reclimit : Nat) : SailM (TrieUpdate × Bool) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      let entry := trie_entry.entry
      let current := entry.value.curr
      (storage_block_iter_begin entry.addr)
      let storage_source := (StorageTrieUpdates entry.addr)
      let storage_updates ← do (_rec_trie_updates_begin storage_source _reclimit_pred)
      let no_storage_changes := (updates_empty storage_updates)
      let storage_changed := (! no_storage_changes)
      let base_storage_root :=
        if (current.storage_cleared : Bool)
        then EMPTY_TRIE_ROOT
        else current.info.storage_root
      let account_empty := (account_info_empty current.info)
      let account_nonempty := (! account_empty)
      let storage_root ← do
        if ((current.present && (account_nonempty && storage_changed)) : Bool)
        then
          (pure (← (_rec_trie_root_cursor base_storage_root storage_updates _reclimit_pred)).root)
        else (pure base_storage_root)
      let update ← do (account_update trie_entry storage_root)
      let persisted_account_changed := (account_value_changed entry.value)
      (pure (update, (persisted_account_changed || storage_changed))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_next_changed_account_trie_update (_ : Unit) (_reclimit : Nat) : SailM TrieUpdateFetch := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      let searching : Bool := true
      let result : TrieUpdateFetch :=
        { available := false,
          update := EMPTY_TRIE_UPDATE }
      let (result, searching) ← (( do
        let loop_vars ← whileFuelM (fuel :=(2 ^i 64)) (fun (result, searching) => (pure searching)) (result, searching)
          fun (result, searching) => do
            assert true "loop dummy assert"
            let iterator_item ← do (acct_block_iter_next ())
            let (result, searching) ← (( do
              match iterator_item with
              | .AcctBlockIterRow entry =>
                (do
                  let (update, changed) ← do (_rec_account_trie_update entry _reclimit_pred)
                  let (result, searching) : (TrieUpdateFetch × Bool) :=
                    if (changed : Bool)
                    then
                      (let result : TrieUpdateFetch :=
                        { available := true,
                          update := update }
                      let searching : Bool := false
                      (result, searching))
                    else (result, searching)
                  (pure (result, searching)))
              | .AcctBlockIterExhausted _ =>
                (let searching : Bool := false
                (pure (result, searching))) ) : SailM (TrieUpdateFetch × Bool) )
            (pure (result, searching))
        (pure loop_vars) ) : SailM (TrieUpdateFetch × Bool) )
      (pure result))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _
/- Type quantifiers: _reclimit : Nat, k_ex553637_ : Nat, 0 ≤ k_ex553637_ ∧ k_ex553637_ ≤ 64, 0
  ≤ _reclimit -/
def _rec_overlay_extension_subtree (childref : NodeRef) (child_prefix : TriePath) (updates : TrieUpdateCursor) (evm_prefix' : TriePath) (cursor : Nat) (_reclimit : Nat) : SailM (TrieItem × TrieUpdateCursor) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      let prefix_len := (path_len evm_prefix')
      let child_below_prefix ← do (path_prefix_of evm_prefix' child_prefix)
      if (((prefix_len != cursor) || (! child_below_prefix)) : Bool)
      then (fatal_error WitnessDeficient)
      else (pure ())
      let at_child_prefix := (path_eq evm_prefix' child_prefix)
      if (at_child_prefix : Bool)
      then
        (do
          let has_update := (update_under_current_prefix updates)
          if (has_update : Bool)
          then
            (do
              let ⟨_, ⟨_, child⟩⟩ ← do (resolve_witness_ref childref)
              (_rec_witness_subtree ⟨_, ⟨_, child⟩⟩ child_prefix updates cursor
                _reclimit_pred))
          else (pure ((trie_branch child_prefix childref), updates)))
      else
        (do
          if ((cursor == 64) : Bool)
          then (fatal_error WitnessDeficient)
          else
            (do
              let next_cursor : Nat := (cursor + 1)
              let extension_nibble ← do (path_nibble child_prefix cursor)
              let children := (trie_children_empty ())
              let remaining := updates
              let extension_pending : Bool := true
              let update_pending := (update_under_current_prefix remaining)
              let (children, extension_pending, remaining, update_pending) ← (( do
                let loop_vars ← whileFuelM (fuel :=(← (overlay_child_ranges_remaining remaining
                    extension_pending extension_nibble))) (fun (children, extension_pending, remaining, update_pending) => (pure (extension_pending || update_pending))) (children, extension_pending, remaining, update_pending)
                  fun (children, extension_pending, remaining, update_pending) => do
                    assert true "loop dummy assert"
                    let (children, extension_pending, remaining, update_pending) ← (( do
                      if (update_pending : Bool)
                      then
                        (do
                          let update_nibble ← do (update_child_nibble remaining)
                          let extension_index := (BitVec.toNatInt extension_nibble)
                          let update_index := (BitVec.toNatInt update_nibble)
                          let (children, extension_pending, remaining, update_pending) ← (( do
                            if ((extension_pending && (extension_index <b update_index)) : Bool)
                            then
                              (do
                                let extension := (trie_branch child_prefix childref)
                                let children ←
                                  (trie_children_add children evm_prefix' extension_nibble extension)
                                let extension_pending : Bool := false
                                (pure (children, extension_pending, remaining, update_pending)))
                            else
                              (do
                                let next_path ← do (path_single update_nibble)
                                let next_prefix ← do (path_concat evm_prefix' next_path)
                                let descended ← do (trie_updates_descend remaining)
                                let consumes_extension :=
                                  (extension_pending && (update_nibble == extension_nibble))
                                let (child, next) ← do
                                  if (consumes_extension : Bool)
                                  then
                                    (_rec_overlay_extension_subtree childref child_prefix descended
                                      next_prefix next_cursor _reclimit_pred)
                                  else
                                    (_rec_updates_subtree descended next_prefix next_cursor
                                      _reclimit_pred)
                                let children ←
                                  (trie_children_add children evm_prefix' update_nibble child)
                                let extension_pending : Bool :=
                                  if (consumes_extension : Bool)
                                  then
                                    (let extension_pending : Bool := false
                                    extension_pending)
                                  else extension_pending
                                let rebased ← do (trie_updates_rebase next evm_prefix')
                                let rebased_update_pending := (update_under_current_prefix rebased)
                                if (rebased_update_pending : Bool)
                                then
                                  (do
                                    let rebased_nibble ← do (update_child_nibble rebased)
                                    let rebased_index := (BitVec.toNatInt rebased_nibble)
                                    if ((rebased_index ≤b update_index) : Bool)
                                    then (fatal_error WitnessDeficient)
                                    else (pure ()))
                                else (pure ())
                                let remaining : TrieUpdateCursor := rebased
                                let update_pending : Bool := rebased_update_pending
                                (pure (children, extension_pending, remaining, update_pending))) ) :
                            SailM (TrieChildren × Bool × TrieUpdateCursor × Bool) )
                          (pure (children, extension_pending, remaining, update_pending)))
                      else
                        (do
                          let extension := (trie_branch child_prefix childref)
                          let children ←
                            (trie_children_add children evm_prefix' extension_nibble extension)
                          let extension_pending : Bool := false
                          (pure (children, extension_pending, remaining, update_pending))) ) : SailM
                      (TrieChildren × Bool × TrieUpdateCursor × Bool) )
                    (pure (children, extension_pending, remaining, update_pending))
                (pure loop_vars) ) : SailM (TrieChildren × Bool × TrieUpdateCursor × Bool) )
              (pure ((← (trie_children_finish evm_prefix' children)), remaining)))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _
/- Type quantifiers: _reclimit : Nat, k_ex553628_ : Nat, value_dependentWitness1 : Nat, value_dependentWitness0
  : Nat, 0 ≤ value_dependentWitness0 ∧
  0 ≤ value_dependentWitness1 ∧
  (value_dependentWitness0 + value_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ k_ex553628_ ∧
  k_ex553628_ ≤ 64, 0 ≤ _reclimit -/
def _rec_overlay_leaf_subtree (updates : TrieUpdateCursor) (evm_prefix' : TriePath) (key : TriePath) (value : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (cursor : Nat) (_reclimit : Nat) : SailM (TrieItem × TrieUpdateCursor) := do
  let value_dependentWitness0 := (value).1
  let value_dependentWitness1 := ((value).2).1
  let value := ((value).2).2
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      let prefix_len := (path_len evm_prefix')
      let key_below_prefix ← do (path_prefix_of evm_prefix' key)
      if (((prefix_len != cursor) || (! key_below_prefix)) : Bool)
      then (fatal_error WitnessDeficient)
      else (pure ())
      if ((cursor == 64) : Bool)
      then
        (do
          let key_matches_prefix := (path_eq evm_prefix' key)
          if ((! key_matches_prefix) : Bool)
          then (fatal_error WitnessDeficient)
          else (pure ())
          let has_update := (update_under_current_prefix updates)
          if (has_update : Bool)
          then
            (do
              match updates.relation with
              | .UpdateUnderPrefix path_postfix =>
                (do
                  let postfix_len := (path_len path_postfix)
                  if ((postfix_len != 0) : Bool)
                  then (fatal_error WitnessDeficient)
                  else (pure ()))
              | .UpdateBeyondPrefix _ => (fatal_error WitnessDeficient)
              | .UpdateSourceExhausted _ => (fatal_error WitnessDeficient)
              let (update, next) ← do (_rec_trie_updates_pop updates _reclimit_pred)
              let update_matches_key := (path_eq update.key key)
              if ((! update_matches_key) : Bool)
              then (fatal_error WitnessDeficient)
              else (pure ())
              let subtree : TrieItem :=
                match update.change with
                | .TrieDelete () => (trie_empty_subtree ())
                | .TriePut ⟨_, ⟨_, updated⟩⟩ =>
                  (trie_scratch_leaf key ⟨_, ⟨_, updated⟩⟩)
              (pure (subtree, next)))
          else (pure ((trie_input_leaf key ⟨_, ⟨_, value⟩⟩), updates)))
      else
        (do
          let next_cursor : Nat := (cursor + 1)
          let leaf_nibble ← do (path_nibble key cursor)
          let children := (trie_children_empty ())
          let remaining := updates
          let leaf_pending : Bool := true
          let update_pending := (update_under_current_prefix remaining)
          let (children, leaf_pending, remaining, update_pending) ← (( do
            let loop_vars ← whileFuelM (fuel :=(← (overlay_child_ranges_remaining remaining
                leaf_pending leaf_nibble))) (fun (children, leaf_pending, remaining, update_pending) => (pure (leaf_pending || update_pending))) (children, leaf_pending, remaining, update_pending)
              fun (children, leaf_pending, remaining, update_pending) => do
                assert true "loop dummy assert"
                let (children, leaf_pending, remaining, update_pending) ← (( do
                  if (update_pending : Bool)
                  then
                    (do
                      let update_nibble ← do (update_child_nibble remaining)
                      let leaf_index := (BitVec.toNatInt leaf_nibble)
                      let update_index := (BitVec.toNatInt update_nibble)
                      let (children, leaf_pending, remaining, update_pending) ← (( do
                        if ((leaf_pending && (leaf_index <b update_index)) : Bool)
                        then
                          (do
                            let leaf := (trie_input_leaf key ⟨_, ⟨_, value⟩⟩)
                            let children ← (trie_children_add children evm_prefix' leaf_nibble leaf)
                            let leaf_pending : Bool := false
                            (pure (children, leaf_pending, remaining, update_pending)))
                        else
                          (do
                            let child_path ← do (path_single update_nibble)
                            let child_prefix ← do (path_concat evm_prefix' child_path)
                            let descended ← do (trie_updates_descend remaining)
                            let consumes_leaf := (leaf_pending && (update_nibble == leaf_nibble))
                            let (child, next) ← do
                              if (consumes_leaf : Bool)
                              then
                                (_rec_overlay_leaf_subtree descended child_prefix key
                                  ⟨_, ⟨_, value⟩⟩ next_cursor _reclimit_pred)
                              else
                                (_rec_updates_subtree descended child_prefix next_cursor
                                  _reclimit_pred)
                            let children ←
                              (trie_children_add children evm_prefix' update_nibble child)
                            let leaf_pending : Bool :=
                              if (consumes_leaf : Bool)
                              then
                                (let leaf_pending : Bool := false
                                leaf_pending)
                              else leaf_pending
                            let rebased ← do (trie_updates_rebase next evm_prefix')
                            let rebased_update_pending := (update_under_current_prefix rebased)
                            if (rebased_update_pending : Bool)
                            then
                              (do
                                let rebased_nibble ← do (update_child_nibble rebased)
                                let rebased_index := (BitVec.toNatInt rebased_nibble)
                                if ((rebased_index ≤b update_index) : Bool)
                                then (fatal_error WitnessDeficient)
                                else (pure ()))
                            else (pure ())
                            let remaining : TrieUpdateCursor := rebased
                            let update_pending : Bool := rebased_update_pending
                            (pure (children, leaf_pending, remaining, update_pending))) ) : SailM
                        (TrieChildren × Bool × TrieUpdateCursor × Bool) )
                      (pure (children, leaf_pending, remaining, update_pending)))
                  else
                    (do
                      let leaf := (trie_input_leaf key ⟨_, ⟨_, value⟩⟩)
                      let children ← (trie_children_add children evm_prefix' leaf_nibble leaf)
                      let leaf_pending : Bool := false
                      (pure (children, leaf_pending, remaining, update_pending))) ) : SailM
                  (TrieChildren × Bool × TrieUpdateCursor × Bool) )
                (pure (children, leaf_pending, remaining, update_pending))
            (pure loop_vars) ) : SailM (TrieChildren × Bool × TrieUpdateCursor × Bool) )
          (pure ((← (trie_children_finish evm_prefix' children)), remaining))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_trie_root_cursor (base_root : (Vector (BitVec 8) 32)) (updates : TrieUpdateCursor) (_reclimit : Nat) : SailM TrieRootResult := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      let no_updates := (updates_empty updates)
      if (no_updates : Bool)
      then
        (pure { root := base_root,
                changed := false })
      else
        (do
          let empty_prefix := (path_empty ())
          let (subtree, remaining) ← do
            if ((base_root == EMPTY_TRIE_ROOT) : Bool)
            then (_rec_updates_subtree updates empty_prefix 0 _reclimit_pred)
            else
              (do
                let ⟨_, ⟨_, node⟩⟩ ← do (node_db_lookup base_root)
                if ((node.len == 0) : Bool)
                then (fatal_error WitnessDeficient)
                else
                  (_rec_witness_subtree ⟨_, ⟨_, node⟩⟩ empty_prefix updates 0 _reclimit_pred))
          let all_updates_consumed := (updates_empty remaining)
          if (all_updates_consumed : Bool)
          then
            (pure { root := ← (trie_subtree_root subtree),
                    changed := true })
          else (fatal_error WitnessDeficient)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_trie_update_source_next (source : TrieUpdateSource) (_reclimit : Nat) : SailM TrieUpdateFetch := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      match source with
      | .StorageTrieUpdates addr => (next_storage_trie_update addr)
      | .ChangedAccountTrieUpdates () => (_rec_next_changed_account_trie_update () _reclimit_pred))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_trie_updates_begin (source : TrieUpdateSource) (_reclimit : Nat) : SailM TrieUpdateCursor := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      let first ← do (_rec_trie_update_source_next source _reclimit_pred)
      let relation : TrieUpdateRelation :=
        if (first.available : Bool)
        then (UpdateUnderPrefix first.update.key)
        else (UpdateSourceExhausted ())
      (pure { source := source,
              current := first.update,
              relation := relation }))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_trie_updates_pop (updates : TrieUpdateCursor) (_reclimit : Nat) : SailM (TrieUpdate × TrieUpdateCursor) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      match updates.relation with
      | .UpdateSourceExhausted _ => (fatal_error WitnessDeficient)
      | _ =>
        (do
          let current := updates.current
          let successor ← do (_rec_trie_update_source_next updates.source _reclimit_pred)
          let next ← (( do
            if (successor.available : Bool)
            then
              (do
                let common_prefix_len ← do (common_prefix_length current.key successor.update.key)
                (pure { source := updates.source,
                        current := successor.update,
                        relation := (UpdateBeyondPrefix common_prefix_len) }))
            else
              (pure { source := updates.source,
                      current := EMPTY_TRIE_UPDATE,
                      relation := (UpdateSourceExhausted ()) }) ) : SailM TrieUpdateCursor )
          (pure (current, next))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _
/- Type quantifiers: _reclimit : Nat, k_ex553615_ : Nat, 0 ≤ k_ex553615_ ∧ k_ex553615_ ≤ 64, 0
  ≤ _reclimit -/
def _rec_updates_subtree (updates : TrieUpdateCursor) (evm_prefix' : TriePath) (cursor : Nat) (_reclimit : Nat) : SailM (TrieItem × TrieUpdateCursor) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      let prefix_len := (path_len evm_prefix')
      if ((prefix_len != cursor) : Bool)
      then (fatal_error WitnessDeficient)
      else (pure ())
      let has_update := (update_under_current_prefix updates)
      if ((! has_update) : Bool)
      then (pure ((trie_empty_subtree ()), updates))
      else
        (do
          if ((cursor == 64) : Bool)
          then
            (do
              match updates.relation with
              | .UpdateUnderPrefix path_postfix =>
                (do
                  let postfix_len := (path_len path_postfix)
                  if ((postfix_len != 0) : Bool)
                  then (fatal_error WitnessDeficient)
                  else (pure ()))
              | .UpdateBeyondPrefix _ => (fatal_error WitnessDeficient)
              | .UpdateSourceExhausted _ => (fatal_error WitnessDeficient)
              let (update, next) ← do (_rec_trie_updates_pop updates _reclimit_pred)
              let update_matches_prefix := (path_eq update.key evm_prefix')
              if ((! update_matches_prefix) : Bool)
              then (fatal_error WitnessDeficient)
              else (pure ())
              let subtree : TrieItem :=
                match update.change with
                | .TrieDelete () => (trie_empty_subtree ())
                | .TriePut ⟨_, ⟨_, value⟩⟩ =>
                  (trie_scratch_leaf update.key ⟨_, ⟨_, value⟩⟩)
              (pure (subtree, next)))
          else
            (do
              let next_cursor : Nat := (cursor + 1)
              let children := (trie_children_empty ())
              let remaining := updates
              let update_pending := (update_under_current_prefix remaining)
              let (children, remaining, update_pending) ← (( do
                let loop_vars ← whileFuelM (fuel :=(← (update_child_ranges_remaining remaining))) (fun (children, remaining, update_pending) => (pure update_pending)) (children, remaining, update_pending)
                  fun (children, remaining, update_pending) => do
                    assert true "loop dummy assert"
                    let nib ← do (update_child_nibble remaining)
                    let child_path ← do (path_single nib)
                    let child_prefix ← do (path_concat evm_prefix' child_path)
                    let descended ← do (trie_updates_descend remaining)
                    let (child, next) ← do
                      (_rec_updates_subtree descended child_prefix next_cursor _reclimit_pred)
                    let children ← (trie_children_add children evm_prefix' nib child)
                    let rebased ← do (trie_updates_rebase next evm_prefix')
                    let rebased_update_pending := (update_under_current_prefix rebased)
                    if (rebased_update_pending : Bool)
                    then
                      (do
                        let rebased_nibble ← do (update_child_nibble rebased)
                        let rebased_index := (BitVec.toNatInt rebased_nibble)
                        let previous_index := (BitVec.toNatInt nib)
                        if ((rebased_index ≤b previous_index) : Bool)
                        then (fatal_error WitnessDeficient)
                        else (pure ()))
                    else (pure ())
                    let remaining : TrieUpdateCursor := rebased
                    let update_pending : Bool := rebased_update_pending
                    (pure (children, remaining, update_pending))
                (pure loop_vars) ) : SailM (TrieChildren × TrieUpdateCursor × Bool) )
              (pure ((← (trie_children_finish evm_prefix' children)), remaining)))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _
/- Type quantifiers: _reclimit : Nat, cursor : Nat, node_dependentWitness1 : Nat, node_dependentWitness0
  : Nat, 0 ≤ node_dependentWitness0 ∧
  0 ≤ node_dependentWitness1 ∧
  (node_dependentWitness0 + node_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ cursor ∧ cursor ≤ 64, 0
  ≤ _reclimit -/
def _rec_witness_subtree (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (evm_prefix' : TriePath) (updates : TrieUpdateCursor) (cursor : Nat) (_reclimit : Nat) : SailM (TrieItem × TrieUpdateCursor) := do
  let node_dependentWitness0 := (node).1
  let node_dependentWitness1 := ((node).2).1
  let node := ((node).2).2
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      let prefix_len := (path_len evm_prefix')
      if ((prefix_len != cursor) : Bool)
      then (fatal_error WitnessDeficient)
      else (pure ())
      if ((node.len == 0) : Bool)
      then (_rec_updates_subtree updates evm_prefix' cursor _reclimit_pred)
      else
        (do
          let decoded ← do (decode_input_trie_node ⟨_, ⟨_, node⟩⟩)
          match decoded with
          | .InputLeafNode (path, value) =>
            (do
              let key ← do (path_concat evm_prefix' path)
              (_rec_overlay_leaf_subtree updates evm_prefix' key value cursor _reclimit_pred))
          | .InputExtensionNode (path, childref) =>
            (do
              let extension_len := (path_len path)
              let next_cursor := (cursor + extension_len)
              if (((extension_len == 0) || (64 <b next_cursor)) : Bool)
              then (fatal_error WitnessDeficient)
              else
                (do
                  let child_prefix ← do (path_concat evm_prefix' path)
                  (_rec_overlay_extension_subtree childref child_prefix updates evm_prefix' cursor
                    _reclimit_pred)))
          | .InputBranchNode (children, value) =>
            (do
              if (((((value).2).2.len != 0) || (64 ≤b cursor)) : Bool)
              then (fatal_error WitnessDeficient)
              else
                (do
                  let next_cursor := (cursor + 1)
                  let built := (trie_children_empty ())
                  let remaining := updates
                  let nib : (BitVec 4) := 0x0#4
                  let (built, nib, remaining) ← (( do
                    let loop_i_lower := 0
                    let loop_i_upper := 15
                    let mut loop_vars := (built, nib, remaining)
                    for i in [loop_i_lower:loop_i_upper:1]i do
                      let (built, nib, remaining) := loop_vars
                      loop_vars ← do
                        let child_path ← do (path_single nib)
                        let child_prefix ← do (path_concat evm_prefix' child_path)
                        let childref := (GetElem?.getElem! children i)
                        let present : Bool :=
                          match childref with
                          | .EmptyRef () => false
                          | _ => true
                        let update_pending := (update_under_current_prefix remaining)
                        let update_here ← do
                          if (update_pending : Bool)
                          then
                            (do
                              let update_nibble ← do (update_child_nibble remaining)
                              let update_index := (BitVec.toNatInt update_nibble)
                              let child_index := (BitVec.toNatInt nib)
                              if ((update_index <b child_index) : Bool)
                              then (fatal_error WitnessDeficient)
                              else (pure ())
                              (pure (update_nibble == nib)))
                          else (pure false)
                        let (child, next_updates) ← do
                          if (update_here : Bool)
                          then
                            (do
                              let descended ← do (trie_updates_descend remaining)
                              if (present : Bool)
                              then
                                (do
                                  let ⟨_, ⟨_, child⟩⟩ ← do (resolve_witness_ref childref)
                                  (_rec_witness_subtree ⟨_, ⟨_, child⟩⟩ child_prefix
                                    descended next_cursor _reclimit_pred))
                              else
                                (_rec_updates_subtree descended child_prefix next_cursor
                                  _reclimit_pred))
                          else
                            (if (present : Bool)
                            then (pure ((trie_subtree child_prefix childref), remaining))
                            else (pure ((trie_empty_subtree ()), remaining)))
                        let built ← (trie_children_add built evm_prefix' nib child)
                        let remaining ←
                          if (update_here : Bool)
                          then (trie_updates_rebase next_updates evm_prefix')
                          else (pure next_updates)
                        let nib : (BitVec 4) := (nib + 0x1#4)
                        (pure (built, nib, remaining))
                    (pure loop_vars) ) : SailM (TrieChildren × (BitVec 4) × TrieUpdateCursor) )
                  let update_pending := (update_under_current_prefix remaining)
                  if (update_pending : Bool)
                  then (fatal_error WitnessDeficient)
                  else (pure ())
                  (pure ((← (trie_children_finish evm_prefix' built)), remaining))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _
def account_trie_update (trie_entry : AcctTrieEntry) : SailM (TrieUpdate × Bool) := do
  let _measure := (1024 : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_account_trie_update trie_entry (_measure + 1))
def next_changed_account_trie_update (_arg0 : Unit) : SailM TrieUpdateFetch := do
  let _measure := (1024 : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_next_changed_account_trie_update _arg0 (_measure + 1))
/- Type quantifiers: cursor : Nat, 0 ≤ cursor ∧ cursor ≤ 64 -/
def overlay_extension_subtree (childref : NodeRef) (child_prefix : TriePath) (updates : TrieUpdateCursor) (evm_prefix' : TriePath) (cursor : Nat) : SailM (TrieItem × TrieUpdateCursor) := do
  let _measure := ((64 - cursor) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_overlay_extension_subtree childref child_prefix updates evm_prefix' cursor (_measure + 1))
/- Type quantifiers: cursor : Nat, value_dependentWitness1 : Nat, value_dependentWitness0 : Nat, 0
  ≤ value_dependentWitness0 ∧
  0 ≤ value_dependentWitness1 ∧
  (value_dependentWitness0 + value_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ cursor ∧
  cursor ≤ 64 -/
def overlay_leaf_subtree (updates : TrieUpdateCursor) (evm_prefix' : TriePath) (key : TriePath) (value : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (cursor : Nat) : SailM (TrieItem × TrieUpdateCursor) := do
  let value_dependentWitness0 := (value).1
  let value_dependentWitness1 := ((value).2).1
  let value := ((value).2).2
  let _measure := ((64 - cursor) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_overlay_leaf_subtree updates evm_prefix' key ⟨_, ⟨_, value⟩⟩ cursor (_measure + 1))
def trie_root_cursor (base_root : (Vector (BitVec 8) 32)) (updates : TrieUpdateCursor) : SailM TrieRootResult := do
  let _measure := (1024 : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_trie_root_cursor base_root updates (_measure + 1))
def trie_update_source_next (source : TrieUpdateSource) : SailM TrieUpdateFetch := do
  let _measure := (1024 : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_trie_update_source_next source (_measure + 1))
def trie_updates_begin (source : TrieUpdateSource) : SailM TrieUpdateCursor := do
  let _measure := (1024 : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_trie_updates_begin source (_measure + 1))
def trie_updates_pop (updates : TrieUpdateCursor) : SailM (TrieUpdate × TrieUpdateCursor) := do
  let _measure := (1024 : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_trie_updates_pop updates (_measure + 1))
/- Type quantifiers: cursor : Nat, 0 ≤ cursor ∧ cursor ≤ 64 -/
def updates_subtree (updates : TrieUpdateCursor) (evm_prefix' : TriePath) (cursor : Nat) : SailM (TrieItem × TrieUpdateCursor) := do
  let _measure := ((64 - cursor) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_updates_subtree updates evm_prefix' cursor (_measure + 1))
/- Type quantifiers: cursor : Nat, node_dependentWitness1 : Nat, node_dependentWitness0 : Nat, 0 ≤
  node_dependentWitness0 ∧
  0 ≤ node_dependentWitness1 ∧
  (node_dependentWitness0 + node_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ cursor ∧ cursor ≤ 64 -/
def witness_subtree (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (evm_prefix' : TriePath) (updates : TrieUpdateCursor) (cursor : Nat) : SailM (TrieItem × TrieUpdateCursor) := do
  let node_dependentWitness0 := (node).1
  let node_dependentWitness1 := ((node).2).1
  let node := ((node).2).2
  let _measure := ((64 - cursor) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_witness_subtree ⟨_, ⟨_, node⟩⟩ evm_prefix' updates cursor (_measure + 1))
end

