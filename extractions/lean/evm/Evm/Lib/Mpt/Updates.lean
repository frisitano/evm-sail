import Evm.Flow
import Evm.Arith
import Evm.Primitives.Crypto
import Evm.Host.Kernel.Storage
import Evm.Lib.Mpt.Primitives
import Evm.Lib.Mpt.Nodes

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

def updates_empty (updates : (List TrieUpdate)) : Bool :=
  match updates with
  | [] => true
  | _ => false

def next_update_under (updates : (List TrieUpdate)) (evm_prefix' : TriePath) : Bool :=
  match updates with
  | (update :: _) => (path_prefix_of evm_prefix' update.key)
  | [] => false

def item_leaf (path : TriePath) (value : EvmByteSlice) : TrieItem :=
  { path := path,
    value := (LeafItem value) }

def item_branch (path : TriePath) (childref : NodeRef) : TrieItem :=
  { path := path,
    value := (BranchItem childref) }

def item_subtree (path : TriePath) (childref : NodeRef) : TrieItem :=
  { path := path,
    value := (SubtreeItem childref) }

/-- Type quantifiers: k_ex161576_ : Nat, 0 ≤ k_ex161576_ ∧ k_ex161576_ ≤ 64 -/
def item_ref (it : TrieItem) (depth : trie_path_len) : SailM NodeRef := do
  let depth := (depth).value
  let suffix := (path_drop it.path ⟨depth⟩)
  match it.value with
  | .LeafItem value => (leaf_child_ref suffix value)
  | .BranchItem subref =>
    (do
      if ((((path_len suffix)).value == 0) : Bool)
      then (pure subref)
      else (merge_ext_ref suffix subref))
  | .SubtreeItem subref =>
    (do
      if ((((path_len suffix)).value == 0) : Bool)
      then (pure subref)
      else
        (do
          match subref with
          | .HashRef h =>
            (do
              let node ← do (node_db_lookup h)
              if ((byte_quantity_equal node.len BYTE_ZERO) : Bool)
              then sailThrow ((InvalidBlock WitnessDeficient))
              else (merge_ext_node suffix node))
          | _ => (merge_ext_ref suffix subref)))

/-- Type quantifiers: depth : Nat, 0 ≤ depth ∧ depth ≤ 63 -/
def empty_trie_branch_frame (depth : trie_depth) : TrieBranchFrame :=
  let depth := (depth).value
  { depth := ⟨depth⟩,
    mask := 0x0000#16,
    children := (vectorInit (EmptyRef ())) }

def trie_builder_empty (_ : Unit) : TrieBuilder :=
  { frames := [],
    root := (EmptyRef ()),
    complete := false }

/-- Type quantifiers: k_ex161579_ : Nat, 0 ≤ k_ex161579_ ∧ k_ex161579_ ≤ 63 -/
def trie_builder_push (builder : TrieBuilder) (depth : trie_depth) : TrieBuilder :=
  let depth := (depth).value
  { frames := ((empty_trie_branch_frame ⟨depth⟩) :: builder.frames),
    root := builder.root,
    complete := builder.complete }

def trie_builder_attach (builder : TrieBuilder) (path : TriePath) (child : NodeRef) : SailM TrieBuilder := do
  match builder.frames with
  | [] => sailThrow ((InvalidBlock WitnessDeficient))
  | (current :: rest) =>
    (do
      let frame := current
      let depth := (frame.depth).value
      if ((((path_len path)).value ≤b depth) : Bool)
      then sailThrow ((InvalidBlock WitnessDeficient))
      else (pure ())
      let child_index := (path_nibble path ⟨depth⟩)
      if ((branch_mask_has frame.mask child_index) : Bool)
      then sailThrow ((InvalidBlock WitnessDeficient))
      else (pure ())
      let frame : TrieBranchFrame := { frame with mask := (branch_mask_set frame.mask child_index) }
      let frame : TrieBranchFrame :=
        { frame with children := (branch_refs_set frame.children child_index child) }
      (pure { frames := (frame :: rest),
              root := builder.root,
              complete := builder.complete }))

def trie_builder_pop (builder : TrieBuilder) : SailM (TrieBranchFrame × TrieBuilder) := do
  match builder.frames with
  | [] => sailThrow ((InvalidBlock WitnessDeficient))
  | (frame :: rest) =>
    (pure (frame, { frames := rest,
                    root := builder.root,
                    complete := builder.complete }))

/-- Type quantifiers: k_ex161581_ : Nat, k_ex161580_ : Nat, 0 ≤ k_ex161580_ ∧ k_ex161580_ ≤ 63, 0
  ≤ k_ex161581_ ∧ k_ex161581_ ≤ 63 -/
def trie_builder_wrap_branch (anchor : TriePath) (parent_depth : trie_depth) (child_depth : trie_depth) (child : NodeRef) : SailM NodeRef := do
  let parent_depth := (parent_depth).value
  let child_depth := (child_depth).value
  let child_start : Nat := (parent_depth + 1)
  if ((child_depth ≤b child_start) : Bool)
  then (pure child)
  else
    (do
      let gap : Nat := (child_depth -i child_start)
      (extension_child_ref (path_take (path_drop anchor ⟨child_start⟩) ⟨gap⟩) child))

/-- Type quantifiers: _reclimit : Nat, k_ex161582_ : Nat, 0 ≤ k_ex161582_ ∧ k_ex161582_ ≤ 64, 0
  ≤ _reclimit -/
def _rec_trie_builder_close (builder : TrieBuilder) (anchor : TriePath) (next_common : (Option trie_depth)) (fuel : trie_path_len) (_reclimit : Nat) : SailM TrieBuilder := do
  let next_common := (Option.map (fun semanticValue => (semanticValue).value) (next_common))
  let fuel := (fuel).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((fuel == 0) : Bool)
      then sailThrow ((InvalidBlock WitnessDeficient))
      else
        (do
          match builder.frames with
          | [] => (pure builder)
          | (top :: _) =>
            (do
              let should_close : Bool :=
                match next_common with
                | none => true
                | .some depth => (depth <b (top.depth).value)
              if ((! should_close) : Bool)
              then (pure builder)
              else
                (do
                  let (frame, popped) ← do (trie_builder_pop builder)
                  let child ← do (branch_child_ref frame.mask frame.children)
                  let with_parent ← (( do
                    match popped.frames with
                    | (parent :: _) =>
                      (do
                        match next_common with
                        | .some common =>
                          (do
                            if (((parent.depth).value <b common) : Bool)
                            then
                              (do
                                let intermediate := (trie_builder_push popped ⟨common⟩)
                                (trie_builder_attach intermediate anchor
                                  (← (trie_builder_wrap_branch anchor ⟨common⟩
                                      ⟨(frame.depth).value⟩ child))))
                            else
                              (trie_builder_attach popped anchor
                                (← (trie_builder_wrap_branch anchor ⟨(parent.depth).value⟩
                                    ⟨(frame.depth).value⟩ child))))
                        | none =>
                          (trie_builder_attach popped anchor
                            (← (trie_builder_wrap_branch anchor ⟨(parent.depth).value⟩
                                ⟨(frame.depth).value⟩ child))))
                    | [] =>
                      (do
                        match next_common with
                        | .some common =>
                          (do
                            let parent := (trie_builder_push popped ⟨common⟩)
                            (trie_builder_attach parent anchor
                              (← (trie_builder_wrap_branch anchor ⟨common⟩
                                  ⟨(frame.depth).value⟩ child))))
                        | none =>
                          (do
                            let depth := (frame.depth).value
                            let root ← do
                              if ((depth == 0) : Bool)
                              then (pure child)
                              else (extension_child_ref (path_take anchor ⟨depth⟩) child)
                            (pure { frames := popped.frames,
                                    root := root,
                                    complete := true }))) ) : SailM TrieBuilder )
                  (_rec_trie_builder_close with_parent anchor
                    (Option.map (fun semanticValue => ⟨semanticValue⟩) (next_common))
                    ⟨(fuel -i 1)⟩ _reclimit_pred)))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Type quantifiers: fuel : Nat, 0 ≤ fuel ∧ fuel ≤ 64 -/
def trie_builder_close (builder : TrieBuilder) (anchor : TriePath) (next_common : (Option trie_depth)) (fuel : trie_path_len) : SailM TrieBuilder := do
  let next_common := (Option.map (fun semanticValue => (semanticValue).value) (next_common))
  let fuel := (fuel).value
  let _measure := (fuel : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else
    (_rec_trie_builder_close builder anchor
      (Option.map (fun semanticValue => ⟨semanticValue⟩) (next_common)) ⟨fuel⟩
      (_measure + 1))

def trie_item_next_common (item : TrieItem) (next_key : (Option TriePath)) : SailM (Option trie_depth) := do
  let semanticResult ← do
    match next_key with
    | none => (pure none)
    | .some next =>
      (do
        let common ← do
          (do
              let semanticResult ← (common_prefix_from item.path next ⟨0⟩)
              pure ((semanticResult).value))
        if (((! (path_lt item.path next)) || (((((path_len item.path)).value ≤b common) || ((((path_len
                   next)).value ≤b common) : Bool)) : Bool)) : Bool)
        then sailThrow ((InvalidBlock WitnessDeficient))
        else (pure ())
        (pure (some ((← (to_trie_depth ⟨common⟩))).value)))
  pure ((Option.map (fun semanticValue => ⟨semanticValue⟩) (semanticResult)))

def trie_insert_item (builder : TrieBuilder) (item : TrieItem) (next_key : (Option TriePath)) : SailM TrieBuilder := do
  if (builder.complete : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else (pure ())
  let next_common ← do
    (do
        let semanticResult ← (trie_item_next_common item next_key)
        pure ((Option.map (fun semanticValue => (semanticValue).value) (semanticResult))))
  let open_child : Bool :=
    match next_common with
    | none => false
    | .some common =>
      (match builder.frames with
      | [] => true
      | (top :: _) => ((top.depth).value <b common))
  if (open_child : Bool)
  then
    (do
      match next_common with
      | .some common =>
        (do
          let opened := (trie_builder_push builder ⟨common⟩)
          (trie_builder_attach opened item.path (← (item_ref item ⟨(common + 1)⟩))))
      | none => sailThrow ((InvalidBlock WitnessDeficient)))
  else
    (do
      match builder.frames with
      | [] =>
        (pure { frames := builder.frames,
                root := ← (item_ref item ⟨0⟩),
                complete := true })
      | (top :: _) =>
        (do
          let attached ← do
            (trie_builder_attach builder item.path
              (← (item_ref item ⟨((top.depth).value + 1)⟩)))
          (trie_builder_close attached item.path
            (Option.map (fun semanticValue => ⟨semanticValue⟩) (next_common)) ⟨64⟩)))

def trie_sink_empty (_ : Unit) : TrieItemSink :=
  { builder := (trie_builder_empty ()),
    pending := none }

def trie_sink_emit (sink : TrieItemSink) (item : TrieItem) : SailM TrieItemSink := do
  match sink.pending with
  | none =>
    (pure { builder := sink.builder,
            pending := (some item) })
  | .some previous =>
    (pure { builder := ← (trie_insert_item sink.builder previous (some item.path)),
            pending := (some item) })

def trie_sink_finish (sink : TrieItemSink) : SailM TrieItemSink := do
  match sink.pending with
  | none => (pure sink)
  | .some item =>
    (pure { builder := ← (trie_insert_item sink.builder item none),
            pending := none })

def trie_builder_root (builder : TrieBuilder) : SailM hash := do
  match builder.frames with
  | (_ :: _) => sailThrow ((InvalidBlock WitnessDeficient))
  | [] =>
    (do
      if (builder.complete : Bool)
      then (trie_ref_to_root builder.root)
      else (pure EMPTY_TRIE_ROOT))

def trie_sink_root (sink : TrieItemSink) : SailM hash := do
  match sink.pending with
  | .some _ => sailThrow ((InvalidBlock WitnessDeficient))
  | none => (trie_builder_root sink.builder)

