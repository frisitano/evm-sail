import Evm.Flow
import Evm.Primitives.Crypto
import Evm.Host.Kernel.Storage
import Evm.Lib.Mpt.Primitives
import Evm.Lib.Mpt.Updates

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

/-! # The witness-native trie

The witness-native Ethereum Merkle-Patricia trie. [trie_root][] merges
ordered updates into an authenticated base trie and fails closed when a
touched hash is absent from the witness. With an empty base the same
builder computes the Yellow Paper `TRIE(I)` directly.

## The witness walker

`witness_emit(node, evm_prefix, updates, sink, cursor)` emits the sorted
post-state items of the subtree rooted at `node` (whose position is
`evm_prefix`) and returns the updated sink together with the first update
after that subtree. `updates` is a cursor into the one globally sorted
update stream; each call consumes its contiguous evm_prefix range and returns
the cursor to its parent.

The walker descends only along touched paths; untouched children pass
through as single reference items with zero node-db work. Deletes are
consumed here and only here: a delete suppresses its base leaf, and a
delete with no base leaf (the walk proves absence) emits nothing. Whatever
survives is streamed through [trie_sink_emit][] and finalized by
[trie_sink_finish][], so a branch losing children to deletes collapses in
the builder with no walker involvement. RLP fields retain their source and
spans, so embedded nodes and leaf values remain witness slices. -/

/-- Emits all live put updates beneath a evm_prefix and consumes that contiguous
range from the ordered update stream. -/
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_emit_live_updates_under (sink : TrieItemSink) (updates : TrieUpdateCursor) (evm_prefix' : TriePath) (_reclimit : Nat) : SailM (TrieItemSink × TrieUpdateCursor) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      match updates.pending with
      | none => (pure (sink, updates))
      | .some update =>
        (do
          if ((← (path_prefix_of evm_prefix' update.key)) : Bool)
          then
            (do
              let next_sink ← (( do
                match update.change with
                | .TrieDelete () => (pure sink)
                | .TriePut ⟨_, ⟨_, value⟩⟩ =>
                  (trie_sink_emit sink (item_leaf update.key ⟨_, ⟨_, value⟩⟩)) ) : SailM
                TrieItemSink )
              (_rec_emit_live_updates_under next_sink (← (trie_updates_advance updates)) evm_prefix'
                _reclimit_pred))
          else (pure (sink, updates))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Emits all live put updates beneath a evm_prefix and consumes that contiguous
range from the ordered update stream. -/
def emit_live_updates_under (sink : TrieItemSink) (updates : TrieUpdateCursor) (evm_prefix' : TriePath) : SailM (TrieItemSink × TrieUpdateCursor) := do
  let _measure := ((2 ^i 64) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_emit_live_updates_under sink updates evm_prefix' (_measure + 1))

/-- Emits live updates preceding a witness child while retaining the first
update at or after that child. -/
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_emit_updates_before_child (sink : TrieItemSink) (updates : TrieUpdateCursor) (evm_prefix' : TriePath) (child : TriePath) (_reclimit : Nat) : SailM (TrieItemSink × TrieUpdateCursor) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      match updates.pending with
      | none => (pure (sink, updates))
      | .some update =>
        (do
          if ((← if ((! (← (path_prefix_of evm_prefix' update.key))) : Bool)
               then (pure true)
               else
                 (do
                   (pure ((← (path_prefix_of child update.key)) || (! (path_lt update.key child)))))) : Bool)
          then (pure (sink, updates))
          else
            (do
              let next_sink ← (( do
                match update.change with
                | .TrieDelete () => (pure sink)
                | .TriePut ⟨_, ⟨_, value⟩⟩ =>
                  (trie_sink_emit sink (item_leaf update.key ⟨_, ⟨_, value⟩⟩)) ) : SailM
                TrieItemSink )
              (_rec_emit_updates_before_child next_sink (← (trie_updates_advance updates)) evm_prefix'
                child _reclimit_pred))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Emits live updates preceding a witness child while retaining the first
update at or after that child. -/
def emit_updates_before_child (sink : TrieItemSink) (updates : TrieUpdateCursor) (evm_prefix' : TriePath) (child : TriePath) : SailM (TrieItemSink × TrieUpdateCursor) := do
  let _measure := ((2 ^i 64) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_emit_updates_before_child sink updates evm_prefix' child (_measure + 1))

/-- Merges one witness leaf with all ordered updates in the same subtree. -/
/- Type quantifiers: _reclimit : Nat, value_dependentWitness1 : Nat, value_dependentWitness0 : Nat, 0
  ≤ value_dependentWitness0 ∧ 0 ≤ value_dependentWitness1, 0 ≤ _reclimit -/
def _rec_emit_leaf_overlay (sink : TrieItemSink) (updates : TrieUpdateCursor) (evm_prefix' : TriePath) (key : TriePath) (value : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (_reclimit : Nat) : SailM (TrieItemSink × TrieUpdateCursor) := do
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
      match updates.pending with
      | none =>
        (pure ((← (trie_sink_emit sink (item_leaf key ⟨_, ⟨_, value⟩⟩))), updates))
      | .some update =>
        (do
          if ((! (← (path_prefix_of evm_prefix' update.key))) : Bool)
          then (pure ((← (trie_sink_emit sink (item_leaf key ⟨_, ⟨_, value⟩⟩))), updates))
          else
            (do
              if ((path_eq update.key key) : Bool)
              then
                (do
                  let updated_sink ← (( do
                    match update.change with
                    | .TrieDelete () => (pure sink)
                    | .TriePut ⟨_, ⟨_, updated⟩⟩ =>
                      (trie_sink_emit sink (item_leaf key ⟨_, ⟨_, updated⟩⟩)) ) : SailM
                    TrieItemSink )
                  (emit_live_updates_under updated_sink (← (trie_updates_advance updates)) evm_prefix'))
              else
                (do
                  if ((path_lt key update.key) : Bool)
                  then
                    (emit_live_updates_under
                      (← (trie_sink_emit sink (item_leaf key ⟨_, ⟨_, value⟩⟩))) updates
                      evm_prefix')
                  else
                    (do
                      let updated_sink ← (( do
                        match update.change with
                        | .TrieDelete () => (pure sink)
                        | .TriePut ⟨_, ⟨_, updated⟩⟩ =>
                          (trie_sink_emit sink (item_leaf update.key ⟨_, ⟨_, updated⟩⟩)) ) :
                        SailM TrieItemSink )
                      (_rec_emit_leaf_overlay updated_sink (← (trie_updates_advance updates))
                        evm_prefix' key ⟨_, ⟨_, value⟩⟩ _reclimit_pred))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Merges one witness leaf with all ordered updates in the same subtree. -/
/- Type quantifiers: value_dependentWitness1 : Nat, value_dependentWitness0 : Nat, 0 ≤
  value_dependentWitness0 ∧ 0 ≤ value_dependentWitness1 -/
def emit_leaf_overlay (sink : TrieItemSink) (updates : TrieUpdateCursor) (evm_prefix' : TriePath) (key : TriePath) (value : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (TrieItemSink × TrieUpdateCursor) := do
  let value_dependentWitness0 := (value).1
  let value_dependentWitness1 := ((value).2).1
  let value := ((value).2).2
  let _measure := ((2 ^i 64) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_emit_leaf_overlay sink updates evm_prefix' key ⟨_, ⟨_, value⟩⟩ (_measure + 1))

/-- Walks a touched witness subtree and streams its post-update items into the
canonical trie builder. -/
/- Type quantifiers: _reclimit : Nat, k_ex417458_ : Nat, node_dependentWitness1 : Nat, node_dependentWitness0
  : Nat, 0 ≤ node_dependentWitness0 ∧ 0 ≤ node_dependentWitness1, 0 ≤ k_ex417458_ ∧
  k_ex417458_ ≤ 64, 0 ≤ _reclimit -/
def _rec_witness_emit (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (evm_prefix' : TriePath) (updates : TrieUpdateCursor) (sink : TrieItemSink) (cursor : Nat) (_reclimit : Nat) : SailM (TrieItemSink × TrieUpdateCursor) := do
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
      if ((node.len == 0) : Bool)
      then (emit_live_updates_under sink updates evm_prefix')
      else
        (do
          match (← (decode_trie_node ⟨_, ⟨_, node⟩⟩)) with
          | .LeafNode (path, value) =>
            (do
              let key ← do (path_concat evm_prefix' path)
              (emit_leaf_overlay sink updates evm_prefix' key value))
          | .ExtensionNode (path, childref) =>
            (do
              let extension_len := (path_len path)
              let next_cursor := (cursor + extension_len)
              if (((extension_len == 0) || (64 <b next_cursor)) : Bool)
              then sailThrow ((InvalidBlock WitnessDeficient))
              else
                (do
                  let child_prefix ← do (path_concat evm_prefix' path)
                  let (before_sink, child_updates) ← do
                    (emit_updates_before_child sink updates evm_prefix' child_prefix)
                  let (child_sink, later_updates) ← do
                    if ((← (next_update_under child_updates child_prefix)) : Bool)
                    then
                      (do
                        let ⟨_, ⟨_, child⟩⟩ ← do (resolve_ref childref)
                        if ((child.len == 0) : Bool)
                        then (emit_live_updates_under before_sink child_updates child_prefix)
                        else
                          (_rec_witness_emit ⟨_, ⟨_, child⟩⟩ child_prefix child_updates
                            before_sink next_cursor _reclimit_pred))
                    else
                      (pure ((← (trie_sink_emit before_sink (item_branch child_prefix childref))), child_updates))
                  (emit_live_updates_under child_sink later_updates evm_prefix')))
          | .BranchNode (children, value) =>
            (do
              if (((((value).2).2.len != 0) || (64 ≤b cursor)) : Bool)
              then sailThrow ((InvalidBlock WitnessDeficient))
              else
                (do
                  let next_cursor := (cursor + 1)
                  let current_sink := sink
                  let remaining := updates
                  let nib : (BitVec 4) := 0x0#4
                  let (current_sink, nib, remaining) ← (( do
                    let loop_i_lower := 0
                    let loop_i_upper := 15
                    let mut loop_vars := (current_sink, nib, remaining)
                    for i in [loop_i_lower:loop_i_upper:1]i do
                      let (current_sink, nib, remaining) := loop_vars
                      loop_vars ← do
                        let child_prefix ← do (path_concat evm_prefix' (← (path_single nib)))
                        let childref := (GetElem?.getElem! children i)
                        let present : Bool :=
                          match childref with
                          | .EmptyRef () => false
                          | _ => true
                        let (current_sink, remaining) ← (( do
                          if ((← (next_update_under remaining child_prefix)) : Bool)
                          then
                            (do
                              let (next_sink, next_updates) ← do
                                if (present : Bool)
                                then
                                  (do
                                      let dependentArg0 := (← (resolve_ref childref))
                                      (_rec_witness_emit dependentArg0 child_prefix remaining
                                      current_sink next_cursor _reclimit_pred))
                                else (emit_live_updates_under current_sink remaining child_prefix)
                              let current_sink : TrieItemSink := next_sink
                              let remaining : TrieUpdateCursor := next_updates
                              (pure (current_sink, remaining)))
                          else
                            (do
                              let current_sink ← (( do
                                if (present : Bool)
                                then
                                  (do
                                    (trie_sink_emit current_sink
                                      (item_subtree child_prefix childref)))
                                else (pure current_sink) ) : SailM TrieItemSink )
                              (pure (current_sink, remaining))) ) : SailM
                          (TrieItemSink × TrieUpdateCursor) )
                        let nib : (BitVec 4) := (nib + 0x1#4)
                        (pure (current_sink, nib, remaining))
                    (pure loop_vars) ) : SailM (TrieItemSink × (BitVec 4) × TrieUpdateCursor) )
                  (pure (current_sink, remaining))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Walks a touched witness subtree and streams its post-update items into the
canonical trie builder. -/
/- Type quantifiers: cursor : Nat, node_dependentWitness1 : Nat, node_dependentWitness0 : Nat, 0 ≤
  node_dependentWitness0 ∧ 0 ≤ node_dependentWitness1, 0 ≤ cursor ∧ cursor ≤ 64 -/
def witness_emit (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (evm_prefix' : TriePath) (updates : TrieUpdateCursor) (sink : TrieItemSink) (cursor : Nat) : SailM (TrieItemSink × TrieUpdateCursor) := do
  let node_dependentWitness0 := (node).1
  let node_dependentWitness1 := ((node).2).1
  let node := ((node).2).2
  let _measure := ((64 - cursor) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_witness_emit ⟨_, ⟨_, node⟩⟩ evm_prefix' updates sink cursor (_measure + 1))

def undefined_TrieRootResult (_ : Unit) : SailM TrieRootResult := do
  (pure { root := ← (undefined_vector 32 (← (undefined_bitvector 8))),
          changed := ← (undefined_bool ()) })

/-- Applies an already-open update cursor. The `changed` result records
whether the source contained at least one update. -/
def trie_root_cursor (base_root : (Vector (BitVec 8) 32)) (updates : TrieUpdateCursor) : SailM TrieRootResult := do
  if ((updates_empty updates) : Bool)
  then
    (pure { root := base_root,
            changed := false })
  else
    (do
      let sink := (trie_sink_empty ())
      let (updated_sink, remaining) ← do
        if ((base_root == EMPTY_TRIE_ROOT) : Bool)
        then (emit_live_updates_under sink updates (path_empty ()))
        else
          (do
            let ⟨_, ⟨_, node⟩⟩ ← do (node_db_lookup base_root)
            if ((node.len == 0) : Bool)
            then sailThrow ((InvalidBlock WitnessDeficient))
            else (witness_emit ⟨_, ⟨_, node⟩⟩ (path_empty ()) updates sink 0))
      if ((updates_empty remaining) : Bool)
      then
        (pure { root := ← (trie_sink_root (← (trie_sink_finish updated_sink))),
                changed := true })
      else sailThrow ((InvalidBlock WitnessDeficient)))

/-- The root of the trie after pulling and applying the source's ordered
updates. The source's host iterator must be opened by its owner first. -/
def trie_root (base_root : (Vector (BitVec 8) 32)) (source : TrieUpdateSource) : SailM TrieRootResult := do
  (trie_root_cursor base_root (← (trie_updates_begin source)))

