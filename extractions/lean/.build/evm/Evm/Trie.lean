import Evm.Flow
import Evm.Arith
import Evm.Crypto
import Evm.Rlp
import Evm.Storage
import Evm.Primitives
import Evm.Updates

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

def emit_live_updates_under (sink : TrieItemSink) (updates : (List TrieUpdate)) (prefix : TriePath) : SailM (TrieItemSink × (List TrieUpdate)) := do
  match updates with
  | [] => (pure (sink, []))
  | (update :: rest) =>
    (do
      if ((path_prefix_of prefix update.key) : Bool)
      then
        (do
          let next_sink ← (( do
            match update.change with
            | .TrieDelete () => (pure sink)
            | .TriePut value => (trie_sink_emit sink (item_leaf update.key value)) ) : SailM
            TrieItemSink )
          (emit_live_updates_under next_sink rest prefix))
      else (pure (sink, updates)))

def emit_updates_before_child (sink : TrieItemSink) (updates : (List TrieUpdate)) (prefix : TriePath) (child : TriePath) : SailM (TrieItemSink × (List TrieUpdate)) := do
  match updates with
  | [] => (pure (sink, []))
  | (update :: rest) =>
    (do
      if (((! (path_prefix_of prefix update.key)) || ((path_prefix_of child update.key) || (! (path_lt
                 update.key child)))) : Bool)
      then (pure (sink, updates))
      else
        (do
          let next_sink ← (( do
            match update.change with
            | .TrieDelete () => (pure sink)
            | .TriePut value => (trie_sink_emit sink (item_leaf update.key value)) ) : SailM
            TrieItemSink )
          (emit_updates_before_child next_sink rest prefix child)))

def emit_leaf_overlay (sink : TrieItemSink) (updates : (List TrieUpdate)) (prefix : TriePath) (key : TriePath) (value : ByteSlice) : SailM (TrieItemSink × (List TrieUpdate)) := do
  match updates with
  | [] => (pure ((← (trie_sink_emit sink (item_leaf key value))), []))
  | (update :: rest) =>
    (do
      if ((! (path_prefix_of prefix update.key)) : Bool)
      then (pure ((← (trie_sink_emit sink (item_leaf key value))), updates))
      else
        (do
          if ((path_eq update.key key) : Bool)
          then
            (do
              let updated_sink ← (( do
                match update.change with
                | .TrieDelete () => (pure sink)
                | .TriePut updated => (trie_sink_emit sink (item_leaf key updated)) ) : SailM
                TrieItemSink )
              (emit_live_updates_under updated_sink rest prefix))
          else
            (do
              if ((path_lt key update.key) : Bool)
              then
                (emit_live_updates_under (← (trie_sink_emit sink (item_leaf key value))) updates
                  prefix)
              else
                (do
                  let updated_sink ← (( do
                    match update.change with
                    | .TrieDelete () => (pure sink)
                    | .TriePut updated => (trie_sink_emit sink (item_leaf update.key updated)) ) :
                    SailM TrieItemSink )
                  (emit_leaf_overlay updated_sink rest prefix key value)))))


mutual
/-- Type quantifiers: fuel : Nat, 0 ≤ fuel ∧ fuel ≤ 256 -/
def witness_child_emit (field : RlpFieldRef) (prefix : TriePath) (updates : (List TrieUpdate)) (sink : TrieItemSink) (fuel : Nat) : SailM (TrieItemSink × (List TrieUpdate)) := do
  match (← (field_to_ref field)) with
  | .InlineRef node => (witness_emit (← (inline_node_slice node)) prefix updates sink fuel)
  | .EmptyRef () => (emit_live_updates_under sink updates prefix)
  | .HashRef hash =>
    (do
      let node ← do (node_db_lookup hash)
      if ((byte_quantity_equal node.len BYTE_ZERO) : Bool)
      then sailThrow ((InvalidBlock WitnessDeficient))
      else (witness_emit node prefix updates sink fuel))
termination_by let (field, prefix, updates, sink, fuel) := (field, prefix, updates, sink, fuel); (fuel).toNat
/-- Type quantifiers: fuel : Nat, 0 ≤ fuel ∧ fuel ≤ 256 -/
def witness_emit (node : ByteSlice) (prefix : TriePath) (updates : (List TrieUpdate)) (sink : TrieItemSink) (fuel : Nat) : SailM (TrieItemSink × (List TrieUpdate)) := do
  if ((fuel == 0) : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else
    (do
      if ((byte_quantity_equal node.len BYTE_ZERO) : Bool)
      then (emit_live_updates_under sink updates prefix)
      else
        (do
          match (← (decode_trie_node node)) with
          | .LeafNode leaf =>
            (do
              let key ← do (path_concat prefix leaf.path)
              (emit_leaf_overlay sink updates prefix key (← (rlp_ref_content leaf.value))))
          | .ExtensionNode extension =>
            (do
              let child_prefix ← do (path_concat prefix extension.path)
              let (before_sink, child_updates) ← do
                (emit_updates_before_child sink updates prefix child_prefix)
              let (child_sink, later_updates) ← do
                if ((next_update_under child_updates child_prefix) : Bool)
                then
                  (witness_child_emit extension.child child_prefix child_updates before_sink
                    (← (trie_fuel_decrement fuel)))
                else
                  (pure ((← (trie_sink_emit before_sink
                        (item_branch child_prefix (← (field_to_ref extension.child))))), child_updates))
              (emit_live_updates_under child_sink later_updates prefix))
          | .BranchNode branch =>
            (do
              if ((byte_quantity_not_equal branch.value.content_len BYTE_ZERO) : Bool)
              then sailThrow ((InvalidBlock WitnessDeficient))
              else (pure ())
              let current_sink := sink
              let remaining := updates
              let nib : nibble := 0x0#4
              let (current_sink, nib, remaining) ← (( do
                let loop_i_lower := 0
                let loop_i_upper := 15
                let mut loop_vars := (current_sink, nib, remaining)
                for i in [loop_i_lower:loop_i_upper:1]i do
                  let (current_sink, nib, remaining) := loop_vars
                  loop_vars ← do
                    let field := (GetElem?.getElem! branch.children i)
                    let child_prefix ← do (path_concat prefix (path_single nib))
                    let childref ← do (field_to_ref field)
                    let present : Bool :=
                      match childref with
                      | .EmptyRef () => false
                      | _ => true
                    let (current_sink, remaining) ← (( do
                      if ((next_update_under remaining child_prefix) : Bool)
                      then
                        (do
                          let (next_sink, next_updates) ← do
                            if (present : Bool)
                            then
                              (witness_child_emit field child_prefix remaining current_sink
                                (← (trie_fuel_decrement fuel)))
                            else (emit_live_updates_under current_sink remaining child_prefix)
                          let current_sink : TrieItemSink := next_sink
                          let remaining : (List TrieUpdate) := next_updates
                          (pure (current_sink, remaining)))
                      else
                        (do
                          let current_sink ← (( do
                            if (present : Bool)
                            then
                              (do
                                (trie_sink_emit current_sink (item_subtree child_prefix childref)))
                            else (pure current_sink) ) : SailM TrieItemSink )
                          (pure (current_sink, remaining))) ) : SailM
                      (TrieItemSink × (List TrieUpdate)) )
                    let nib : nibble := (nib + 0x1#4)
                    (pure (current_sink, nib, remaining))
                (pure loop_vars) ) : SailM (TrieItemSink × (BitVec 4) × (List TrieUpdate)) )
              (pure (current_sink, remaining)))
          | .InvalidNode () => sailThrow ((InvalidBlock WitnessDeficient))))
termination_by let (node, prefix, updates, sink, fuel) := (node, prefix, updates, sink, fuel); (fuel).toNat
end

def trie_root (base_root : (BitVec 256)) (updates : (List TrieUpdate)) : SailM (BitVec 256) := do
  if ((updates_empty updates) : Bool)
  then (pure base_root)
  else
    (do
      let sink := (trie_sink_empty ())
      let (updated_sink, remaining) ← do
        if ((base_root == EMPTY_TRIE_ROOT) : Bool)
        then (emit_live_updates_under sink updates (path_empty ()))
        else
          (do
            let node ← do (node_db_lookup base_root)
            if ((byte_quantity_equal node.len BYTE_ZERO) : Bool)
            then sailThrow ((InvalidBlock WitnessDeficient))
            else (witness_emit node (path_empty ()) updates sink 256))
      if ((updates_empty remaining) : Bool)
      then (trie_sink_root (← (trie_sink_finish updated_sink)))
      else sailThrow ((InvalidBlock WitnessDeficient)))

