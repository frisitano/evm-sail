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

noncomputable section
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
  { path := path
    value := (LeafItem value) }

def item_branch (path : TriePath) (childref : NodeRef) : TrieItem :=
  { path := path
    value := (BranchItem childref) }

def item_subtree (path : TriePath) (childref : NodeRef) : TrieItem :=
  { path := path
    value := (SubtreeItem childref) }

/-- Type quantifiers: depth : Nat, 0 ≤ depth ∧ depth ≤ 64 -/
def item_ref (it : TrieItem) (depth : Nat) : SailM NodeRef := do
  let suffix := (path_drop it.path depth)
  match it.value with
  | .LeafItem value => (leaf_child_ref suffix value)
  | .BranchItem subref =>
    (do
      if (((path_len suffix) == 0) : Bool)
      then (pure subref)
      else (merge_ext_ref suffix subref))
  | .SubtreeItem subref =>
    (do
      if (((path_len suffix) == 0) : Bool)
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
def empty_trie_branch_frame (depth : Nat) : TrieBranchFrame :=
  { depth := depth
    mask := 0x0000#16
    children := (vectorInit (EmptyRef ())) }

def trie_builder_empty (_ : Unit) : TrieBuilder :=
  { frames := []
    root := (EmptyRef ())
    complete := false }

/-- Type quantifiers: depth : Nat, 0 ≤ depth ∧ depth ≤ 63 -/
def trie_builder_push (builder : TrieBuilder) (depth : Nat) : TrieBuilder :=
  { frames := ((empty_trie_branch_frame depth) :: builder.frames)
    root := builder.root
    complete := builder.complete }

def trie_builder_attach (builder : TrieBuilder) (path : TriePath) (child : NodeRef) : SailM TrieBuilder := do
  match builder.frames with
  | [] => sailThrow ((InvalidBlock WitnessDeficient))
  | (current :: rest) =>
    (do
      let frame := current
      let depth := frame.depth
      if (((path_len path) ≤b depth) : Bool)
      then sailThrow ((InvalidBlock WitnessDeficient))
      else (pure ())
      let child_index := (path_nibble path depth)
      if ((branch_mask_has frame.mask child_index) : Bool)
      then sailThrow ((InvalidBlock WitnessDeficient))
      else (pure ())
      let frame : TrieBranchFrame := { frame with mask := (branch_mask_set frame.mask child_index) }
      let frame : TrieBranchFrame :=
        { frame with children := (branch_refs_set frame.children child_index child) }
      (pure { frames := (frame :: rest)
              root := builder.root
              complete := builder.complete }))

def trie_builder_pop (builder : TrieBuilder) : SailM (TrieBranchFrame × TrieBuilder) := do
  match builder.frames with
  | [] => sailThrow ((InvalidBlock WitnessDeficient))
  | (frame :: rest) =>
    (pure (frame, { frames := rest
                    root := builder.root
                    complete := builder.complete }))

/-- Type quantifiers: child_depth : Nat, parent_depth : Nat, 0 ≤ parent_depth ∧
  parent_depth ≤ 63, 0 ≤ child_depth ∧ child_depth ≤ 63 -/
def trie_builder_wrap_branch (anchor : TriePath) (parent_depth : Nat) (child_depth : Nat) (child : NodeRef) : SailM NodeRef := do
  let child_start : trie_path_len := (parent_depth +i 1)
  if ((child_depth ≤b child_start) : Bool)
  then (pure child)
  else
    (do
      let gap : trie_path_len := (child_depth -i child_start)
      (extension_child_ref (path_take (path_drop anchor child_start) gap) child))

/-- Type quantifiers: fuel : Nat, 0 ≤ fuel ∧ fuel ≤ 64 -/
def trie_builder_close (builder : TrieBuilder) (anchor : TriePath) (next_common : (Option Nat)) (fuel : Nat) : SailM TrieBuilder := do
  if ((fuel == 0) : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else (pure ())
  match builder.frames with
  | [] => (pure builder)
  | (top :: _) =>
    (do
      let should_close : Bool :=
        match next_common with
        | none => true
        | .some depth => (depth <b top.depth)
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
                    if ((parent.depth <b common) : Bool)
                    then
                      (do
                        let intermediate := (trie_builder_push popped common)
                        (trie_builder_attach intermediate anchor
                          (← (trie_builder_wrap_branch anchor common frame.depth child))))
                    else
                      (trie_builder_attach popped anchor
                        (← (trie_builder_wrap_branch anchor parent.depth frame.depth child))))
                | none =>
                  (trie_builder_attach popped anchor
                    (← (trie_builder_wrap_branch anchor parent.depth frame.depth child))))
            | [] =>
              (do
                match next_common with
                | .some common =>
                  (do
                    let parent := (trie_builder_push popped common)
                    (trie_builder_attach parent anchor
                      (← (trie_builder_wrap_branch anchor common frame.depth child))))
                | none =>
                  (do
                    let depth := frame.depth
                    let root ← do
                      if ((depth == 0) : Bool)
                      then (pure child)
                      else (extension_child_ref (path_take anchor depth) child)
                    (pure { frames := popped.frames
                            root := root
                            complete := true }))) ) : SailM TrieBuilder )
          (trie_builder_close with_parent anchor next_common (fuel -i 1))))
termination_by (let (builder, anchor, next_common, fuel) := (builder, anchor, next_common, fuel)
fuel).toNat

def trie_item_next_common (item : TrieItem) (next_key : (Option TriePath)) : SailM (Option Nat) := do
  match next_key with
  | none => (pure none)
  | .some next =>
    (do
      let common ← do (common_prefix_from item.path next 0)
      if (((! (path_lt item.path next)) || ((((path_len item.path) ≤b common) || (((path_len next) ≤b common) : Bool)) : Bool)) : Bool)
      then sailThrow ((InvalidBlock WitnessDeficient))
      else (pure ())
      (pure (some (← (to_trie_depth common)))))

def trie_insert_item (builder : TrieBuilder) (item : TrieItem) (next_key : (Option TriePath)) : SailM TrieBuilder := do
  if (builder.complete : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else (pure ())
  let next_common ← do (trie_item_next_common item next_key)
  let open_child : Bool :=
    match next_common with
    | none => false
    | .some common =>
      (match builder.frames with
      | [] => true
      | (top :: _) => (top.depth <b common))
  if (open_child : Bool)
  then
    (do
      match next_common with
      | .some common =>
        (do
          let opened := (trie_builder_push builder common)
          (trie_builder_attach opened item.path (← (item_ref item (common +i 1)))))
      | none => sailThrow ((InvalidBlock WitnessDeficient)))
  else
    (do
      match builder.frames with
      | [] =>
        (pure { frames := builder.frames
                root := ← (item_ref item 0)
                complete := true })
      | (top :: _) =>
        (do
          let attached ← do
            (trie_builder_attach builder item.path (← (item_ref item (top.depth +i 1))))
          (trie_builder_close attached item.path next_common 64)))

def trie_sink_empty (_ : Unit) : TrieItemSink :=
  { builder := (trie_builder_empty ())
    pending := none }

def trie_sink_emit (sink : TrieItemSink) (item : TrieItem) : SailM TrieItemSink := do
  match sink.pending with
  | none =>
    (pure { builder := sink.builder
            pending := (some item) })
  | .some previous =>
    (pure { builder := ← (trie_insert_item sink.builder previous (some item.path))
            pending := (some item) })

def trie_sink_finish (sink : TrieItemSink) : SailM TrieItemSink := do
  match sink.pending with
  | none => (pure sink)
  | .some item =>
    (pure { builder := ← (trie_insert_item sink.builder item none)
            pending := none })

def trie_builder_root (builder : TrieBuilder) : SailM (BitVec 256) := do
  match builder.frames with
  | (_ :: _) => sailThrow ((InvalidBlock WitnessDeficient))
  | [] =>
    (do
      if (builder.complete : Bool)
      then (trie_ref_to_root builder.root)
      else (pure EMPTY_TRIE_ROOT))

def trie_sink_root (sink : TrieItemSink) : SailM (BitVec 256) := do
  match sink.pending with
  | .some _ => sailThrow ((InvalidBlock WitnessDeficient))
  | none => (trie_builder_root sink.builder)

