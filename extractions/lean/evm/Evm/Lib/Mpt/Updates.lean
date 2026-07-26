import Evm.Prelude
import Evm.Primitives.Crypto
import Evm.Host.Kernel.Scratch
import Evm.Lib.Rlp.Rlp
import Evm.Host.Kernel.Storage
import Evm.Host.Kernel.Accounts
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

/-! # Trie updates and the canonical builder

Ordered leaf updates, their merge into item streams, and the trie builder
that recomposes canonical nodes (YP Appendix D). -/

def cached_account_trie_update_next (_ : Unit) : (Option TrieUpdate) :=
  none

/-- Encodes an account trie leaf with its recomputed storage root. -/
def encode_state_account (info : AccountInfo) (storage_root : (Vector (BitVec 8) 32)) : SailM (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  let nonce_length := (rlp_uint_word_size info.nonce)
  let balance_length := (rlp_uint_word_size info.balance)
  let storage_root_length := (rlp_word_size ())
  let code_hash_length := (rlp_word_size ())
  let content_len := (((nonce_length + balance_length) + storage_root_length) + code_hash_length)
  let start ← do (scratch_begin ())
  (rlp_write_list_prefix content_len)
  (rlp_write_uint_word info.nonce)
  (rlp_write_uint_word info.balance)
  (rlp_write_word (hash_to_word storage_root))
  (rlp_write_word (hash_to_word info.code_hash))
  (rlp_finish start)

/-- Converts one account row and its recomputed storage root into a
state-trie insertion or deletion. -/
def account_update (entry : AcctEntry) (storage_root : (Vector (BitVec 8) 32)) : SailM TrieUpdate := do
  let current := entry.value.curr
  let key ← do (pure (path_from_hash (← (keccak256_address entry.addr))))
  if (((! current.present) || (account_info_empty current.info)) : Bool)
  then
    (pure { key := key,
            change := (TrieDelete ()) })
  else
    (pure { key := key,
            change := ← (do
                  let dependentArg0 := (← (encode_state_account current.info storage_root))
                  pure ((TriePut dependentArg0))) })

/-- Whether any persisted account field changed across the block. -/
def account_value_changed (value : AcctValue) : Bool :=
  ((! (value.curr.info.nonce == value.orig.info.nonce)) || (((! (value.curr.info.balance == value.orig.info.balance)) || ((! (value.curr.info.storage_root == value.orig.info.storage_root)) || ((! (value.curr.info.code_hash == value.orig.info.code_hash)) || ((! (value.curr.present == value.orig.present)) || (! (value.curr.storage_cleared == value.orig.storage_cleared)))))) : Bool))

def storage_value_changed (value : StorageValue) : Bool :=
  (! (value.curr == value.orig))

/-- Pulls the next changed storage row, skipping materialized read-only rows. -/
def next_changed_storage_entry (addr : (Vector (BitVec 8) 20)) : SailM (Option StorageEntry) := do
  let searching : Bool := true
  let result : (Option StorageEntry) := none
  let (result, searching) ← (( do
    let loop_vars ← whileFuelM (fuel :=(2 ^i 64)) (fun (result, searching) => (pure searching)) (result, searching)
      fun (result, searching) => do
        assert true "loop dummy assert"
        let (result, searching) ← (( do
          match (← (storage_block_iter_next addr)) with
          | .some entry =>
            (let (result, searching) : ((Option StorageEntry) × Bool) :=
              if ((storage_value_changed entry.value) : Bool)
              then
                (let result : (Option StorageEntry) := (some entry)
                let searching : Bool := false
                (result, searching))
              else (result, searching)
            (pure (result, searching)))
          | none =>
            (let searching : Bool := false
            (pure (result, searching))) ) : SailM ((Option StorageEntry) × Bool) )
        (pure (result, searching))
    (pure loop_vars) ) : SailM ((Option StorageEntry) × Bool) )
  (pure result)

/-- Computes one already-prepared account update and whether the account or
any storage row has a net change. The equality decision remains in Sail. -/
def account_trie_update (entry : AcctEntry) : SailM (TrieUpdate × Bool) := do
  (storage_block_iter_begin entry.addr)
  let storage_changed ← (( do
    match (← (next_changed_storage_entry entry.addr)) with
    | .some _storage_entry => (pure true)
    | none => (pure false) ) : SailM Bool )
  let storage_root ← do (acct_post_storage_root_read entry.addr)
  (pure ((← (account_update entry storage_root)), ((account_value_changed entry.value) || storage_changed)))

/-- Pulls the next net-changed account update for the protocol state-root path.
The host excludes read-only candidates; Sail skips reverted/no-op writes. -/
def next_changed_account_trie_update (_ : Unit) : SailM (Option TrieUpdate) := do
  let searching : Bool := true
  let result : (Option TrieUpdate) := none
  let (result, searching) ← (( do
    let loop_vars ← whileFuelM (fuel :=(2 ^i 64)) (fun (result, searching) => (pure searching)) (result, searching)
      fun (result, searching) => do
        assert true "loop dummy assert"
        let (result, searching) ← (( do
          match (← (acct_block_iter_next ())) with
          | .some entry =>
            (do
              let (update, changed) ← do (account_trie_update entry)
              let (result, searching) : ((Option TrieUpdate) × Bool) :=
                if (changed : Bool)
                then
                  (let result : (Option TrieUpdate) := (some update)
                  let searching : Bool := false
                  (result, searching))
                else (result, searching)
              (pure (result, searching)))
          | none =>
            (let searching : Bool := false
            (pure (result, searching))) ) : SailM ((Option TrieUpdate) × Bool) )
        (pure (result, searching))
    (pure loop_vars) ) : SailM ((Option TrieUpdate) × Bool) )
  (pure result)

/-- Encodes a nonzero storage value as its minimal RLP integer leaf payload. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def encode_storage_value (value : Nat) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  let encoded_len := (rlp_uint_word_size value)
  let start ← do (scratch_begin ())
  (rlp_write_uint_word value)
  (rlp_finish start)

/-- Converts one changed storage row into a secure-trie update. -/
def storage_update (entry : StorageEntry) : SailM TrieUpdate := do
  let key ← do (pure (path_from_hash (← (keccak256_word entry.key.slot))))
  let change ← do
    if ((word_is_zero entry.value.curr) : Bool)
    then (pure (TrieDelete ()))
    else
      (do
          let dependentArg0 := (← (encode_storage_value entry.value.curr))
          pure ((TriePut dependentArg0)))
  (pure { key := key,
          change := change })

/-- Converts the next changed storage row to a trie update. -/
def next_storage_trie_update (addr : (Vector (BitVec 8) 20)) : SailM (Option TrieUpdate) := do
  match (← (next_changed_storage_entry addr)) with
  | .some entry => (pure (some (← (storage_update entry))))
  | none => (pure none)

/-- State-backed implementation of the generic trie's pull-source contract. -/
def trie_update_source_next (source : TrieUpdateSource) : SailM (Option TrieUpdate) := do
  match source with
  | .StorageTrieUpdates addr => (next_storage_trie_update addr)
  | .ChangedAccountTrieUpdates () => (next_changed_account_trie_update ())
  | .CachedAccountTrieUpdates () => (pure (cached_account_trie_update_next ()))

/-- Opens a pull cursor by fetching only its first update. -/
def trie_updates_begin (source : TrieUpdateSource) : SailM TrieUpdateCursor := do
  (pure { source := source,
          pending := ← (trie_update_source_next source) })

/-- Whether the pull cursor has reached the end of its source. -/
def updates_empty (updates : TrieUpdateCursor) : Bool :=
  match updates.pending with
  | none => true
  | .some _ => false

/-- Consumes the pending update and pulls one replacement. -/
def trie_updates_advance (updates : TrieUpdateCursor) : SailM TrieUpdateCursor := do
  (pure { source := updates.source,
          pending := ← (trie_update_source_next updates.source) })

/-- Whether the cursor's next update falls under `evm_prefix`. -/
def next_update_under (updates : TrieUpdateCursor) (evm_prefix' : TriePath) : SailM Bool := do
  match updates.pending with
  | .some update => (path_prefix_of evm_prefix' update.key)
  | none => (pure false)

/- Type quantifiers: value_dependentWitness1 : Nat, value_dependentWitness0 : Nat, 0 ≤
  value_dependentWitness0 ∧ 0 ≤ value_dependentWitness1 -/
def item_leaf (path : TriePath) (value : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : TrieItem :=
  let value_dependentWitness0 := (value).1
  let value_dependentWitness1 := ((value).2).1
  let value := ((value).2).2
  { path := path,
    value := (LeafItem ⟨_, ⟨_, value⟩⟩) }

def item_branch (path : TriePath) (childref : NodeRef) : TrieItem :=
  { path := path,
    value := (BranchItem childref) }

def item_subtree (path : TriePath) (childref : NodeRef) : TrieItem :=
  { path := path,
    value := (SubtreeItem childref) }

/-- The child reference of a single-item subtree at `depth`: the item's
remaining path is absorbed into it. This is the one place a delete
collapse can demand node material: an unknown-type hash reference
absorbing a nonempty suffix resolves its node from the witness db
(fail-closed). -/
/- Type quantifiers: k_ex417060_ : Nat, 0 ≤ k_ex417060_ ∧ k_ex417060_ ≤ 64 -/
def item_ref (it : TrieItem) (depth : Nat) : SailM NodeRef := do
  let suffix ← do (path_drop it.path depth)
  match it.value with
  | .LeafItem ⟨_, ⟨_, value⟩⟩ => (leaf_child_ref suffix ⟨_, ⟨_, value⟩⟩)
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
              let ⟨_, ⟨_, node⟩⟩ ← do (node_db_lookup h)
              if ((node.len == 0) : Bool)
              then sailThrow ((InvalidBlock WitnessDeficient))
              else (merge_ext_node suffix ⟨_, ⟨_, node⟩⟩))
          | _ => (merge_ext_ref suffix subref)))

/- Type quantifiers: depth : Nat, 0 ≤ depth ∧ depth ≤ 63 -/
def empty_trie_branch_frame (depth : Nat) : TrieBranchFrame :=
  { depth := depth,
    mask := 0x0000#16,
    children := (vectorInit (EmptyRef ())) }

def trie_builder_empty (_ : Unit) : TrieBuilder :=
  { frames := [],
    root := (EmptyRef ()),
    complete := false }

/-- Opens an empty branch at `depth` on the builder stack. -/
/- Type quantifiers: k_ex417063_ : Nat, 0 ≤ k_ex417063_ ∧ k_ex417063_ ≤ 63 -/
def trie_builder_push (builder : TrieBuilder) (depth : Nat) : TrieBuilder :=
  { frames := ((empty_trie_branch_frame depth) :: builder.frames),
    root := builder.root,
    complete := builder.complete }

/-- Attaches one child to the open branch position selected by `path`. -/
def trie_builder_attach (builder : TrieBuilder) (path : TriePath) (child : NodeRef) : SailM TrieBuilder := do
  match builder.frames with
  | [] => sailThrow ((InvalidBlock WitnessDeficient))
  | (current :: rest) =>
    (do
      let frame := current
      let depth := frame.depth
      if (((path_len path) ≤b depth) : Bool)
      then sailThrow ((InvalidBlock WitnessDeficient))
      else
        (do
          let child_index ← do (path_nibble path depth)
          if ((branch_mask_has frame.mask child_index) : Bool)
          then sailThrow ((InvalidBlock WitnessDeficient))
          else
            (let frame : TrieBranchFrame :=
              { frame with mask := (branch_mask_set frame.mask child_index) }
            let frame : TrieBranchFrame :=
              { frame with children := (vectorUpdate frame.children (BitVec.toNatInt child_index)
                  child) }
            (pure { frames := (frame :: rest),
                    root := builder.root,
                    complete := builder.complete }))))

/-- Removes and returns the innermost open branch. -/
def trie_builder_pop (builder : TrieBuilder) : SailM (TrieBranchFrame × TrieBuilder) := do
  match builder.frames with
  | [] => sailThrow ((InvalidBlock WitnessDeficient))
  | (frame :: rest) =>
    (pure (frame, { frames := rest,
                    root := builder.root,
                    complete := builder.complete }))

/-- Inserts an extension between parent and child depths when their paths have
an unbranched gap. -/
/- Type quantifiers: k_ex417065_ : Nat, k_ex417064_ : Nat, 0 ≤ k_ex417064_ ∧ k_ex417064_ ≤ 63, 0
  ≤ k_ex417065_ ∧ k_ex417065_ ≤ 63 -/
def trie_builder_wrap_branch (anchor : TriePath) (parent_depth : Nat) (child_depth : Nat) (child : NodeRef) : SailM NodeRef := do
  let child_start : Nat := (parent_depth + 1)
  if ((child_depth ≤b child_start) : Bool)
  then (pure child)
  else
    (do
      let gap : Nat := (child_depth - child_start)
      (extension_child_ref (← (path_take (← (path_drop anchor child_start)) gap)) child))

/-- Closes every branch deeper than the next key's common evm_prefix. -/
/- Type quantifiers: _reclimit : Nat, k_ex417066_ : Nat, 0 ≤ k_ex417066_ ∧ k_ex417066_ ≤ 64, 0
  ≤ _reclimit -/
def _rec_trie_builder_close (builder : TrieBuilder) (anchor : TriePath) (next_common : (Option Nat)) (fuel : Nat) (_reclimit : Nat) : SailM TrieBuilder := do
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
                              else (extension_child_ref (← (path_take anchor depth)) child)
                            (pure { frames := popped.frames,
                                    root := root,
                                    complete := true }))) ) : SailM TrieBuilder )
                  (_rec_trie_builder_close with_parent anchor next_common (fuel - 1) _reclimit_pred)))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Closes every branch deeper than the next key's common evm_prefix. -/
/- Type quantifiers: fuel : Nat, 0 ≤ fuel ∧ fuel ≤ 64 -/
def trie_builder_close (builder : TrieBuilder) (anchor : TriePath) (next_common : (Option Nat)) (fuel : Nat) : SailM TrieBuilder := do
  let _measure := (fuel : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_trie_builder_close builder anchor next_common fuel (_measure + 1))

/-- Computes the branch depth shared by an item and its ordered successor. -/
def trie_item_next_common (item : TrieItem) (next_key : (Option TriePath)) : SailM (Option Nat) := do
  match next_key with
  | none => (pure none)
  | .some next =>
    (do
      let common ← do (common_prefix_from item.path next 0)
      if (((! (path_lt item.path next)) || ((((path_len item.path) ≤b common) || (((path_len next) ≤b common) : Bool)) : Bool)) : Bool)
      then sailThrow ((InvalidBlock WitnessDeficient))
      else (pure (some (← (to_trie_depth common)))))

/-- Inserts one ordered, evm_prefix-free item using its successor for branch
lookahead. -/
def trie_insert_item (builder : TrieBuilder) (item : TrieItem) (next_key : (Option TriePath)) : SailM TrieBuilder := do
  if (builder.complete : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else
    (do
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
              (trie_builder_attach opened item.path (← (item_ref item (common + 1)))))
          | none => sailThrow ((InvalidBlock WitnessDeficient)))
      else
        (do
          match builder.frames with
          | [] =>
            (pure { frames := builder.frames,
                    root := ← (item_ref item 0),
                    complete := true })
          | (top :: _) =>
            (do
              let attached ← do
                (trie_builder_attach builder item.path (← (item_ref item (top.depth + 1))))
              (trie_builder_close attached item.path next_common 64))))

def trie_sink_empty (_ : Unit) : TrieItemSink :=
  { builder := (trie_builder_empty ()),
    pending := none }

/-- Queues an ordered item and commits the previously pending item. -/
def trie_sink_emit (sink : TrieItemSink) (item : TrieItem) : SailM TrieItemSink := do
  match sink.pending with
  | none =>
    (pure { builder := sink.builder,
            pending := (some item) })
  | .some previous =>
    (pure { builder := ← (trie_insert_item sink.builder previous (some item.path)),
            pending := (some item) })

/-- Commits the final pending item with no successor. -/
def trie_sink_finish (sink : TrieItemSink) : SailM TrieItemSink := do
  match sink.pending with
  | none => (pure sink)
  | .some item =>
    (pure { builder := ← (trie_insert_item sink.builder item none),
            pending := none })

/-- Returns the root of a complete builder, or the canonical empty root. -/
def trie_builder_root (builder : TrieBuilder) : SailM (Vector (BitVec 8) 32) := do
  match builder.frames with
  | (_ :: _) => sailThrow ((InvalidBlock WitnessDeficient))
  | [] =>
    (do
      if (builder.complete : Bool)
      then (trie_ref_to_root builder.root)
      else (pure EMPTY_TRIE_ROOT))

/-- Returns the completed root after all sink lookahead has been consumed. -/
def trie_sink_root (sink : TrieItemSink) : SailM (Vector (BitVec 8) 32) := do
  match sink.pending with
  | .some _ => sailThrow ((InvalidBlock WitnessDeficient))
  | none => (trie_builder_root sink.builder)

