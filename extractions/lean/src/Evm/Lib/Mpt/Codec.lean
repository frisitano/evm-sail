import Evm.Flow
import Evm.Prelude
import Evm.Exceptions
import Evm.Kernel.Scratch
import Evm.Primitives.Crypto
import Evm.Lib.Rlp.Encoding
import Evm.Lib.Rlp.Decoding
import Evm.Kernel.Storage
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

/-! # Merkle-Patricia trie node codec

Canonical hex-evm_prefix and RLP encoding, decoding, and references for trie nodes
(YP Appendix C/D). -/

/-- Returns the RLP width of a child reference in its parent node. -/
def node_ref_size (r : NodeRef) : Nat :=
  match r with
  | .EmptyRef () => 1
  | .InputInlineRef ⟨_, ⟨_, node⟩⟩ => node.len
  | .ScratchInlineRef node => node.len
  | .HashRef _ => (rlp_word_size ())

/-- Appends a child reference in its canonical RLP representation. -/
def rlp_write_node_ref (r : NodeRef) : SailM Unit := do
  match r with
  | .EmptyRef () => (scratch_push_byte 0x80#8)
  | .InputInlineRef ⟨_, ⟨_, node⟩⟩ =>
    (stateless_input_scratch_push_slice ⟨_, ⟨_, node⟩⟩)
  | .ScratchInlineRef node => (scratch_push_b256 node.data node.len)
  | .HashRef h =>
    (do
      let hash_word := (hash_to_word h)
      (rlp_write_word hash_word))

/-- Returns the canonical RLP width of a compact trie path. -/
/- Type quantifiers: k_ex551588_ : Bool -/
def rlp_hex_prefix_size (path : TriePath) (is_leaf : Bool) : SailM Nat := do
  let encoded_length := (hex_prefix_encoded_length path)
  let first ← do (hex_prefix_first_byte path is_leaf)
  if (((encoded_length == 1) && ((BitVec.access first 7) == 0#1)) : Bool)
  then (pure 1)
  else (pure (encoded_length + 1))

/-- Writes the hex-evm_prefix path directly into scratch in wire order. -/
/- Type quantifiers: k_ex551589_ : Bool -/
def rlp_write_hex_prefix (path : TriePath) (is_leaf : Bool) : SailM Unit := do
  let length := (path_len path)
  let encoded_length := (hex_prefix_encoded_length path)
  let first ← do (hex_prefix_first_byte path is_leaf)
  (rlp_write_string_prefix encoded_length first)
  (scratch_push_byte first)
  let odd_length := ((Nat.mod length 2) != 0)
  let index : Nat :=
    if (odd_length : Bool)
    then 1
    else 0
  let index ← (( do
    let loop_vars ← whileFuelM (fuel :=(length -i index)) (fun index => (pure (index <b length))) index
      fun index => do
        assert true "loop dummy assert"
        let current : Nat := (Nat.mod index 65)
        let next : Nat := (Nat.mod (current + 1) 65)
        let high ← do (path_nibble path current)
        let low ← do (path_nibble path next)
        let path_byte := (high +++ low)
        (scratch_push_byte path_byte)
        let index : Nat := (next + 1)
        (pure index)
    (pure loop_vars) ) : SailM Nat )
  (pure ())

/-- The canonical child reference for an encoded node: inline under 32
bytes, otherwise its hash (YP Appendix D, Eq. 207). -/
/- Type quantifiers: encoded_dependentWitness1 : Nat, encoded_dependentWitness0 : Nat, 0 ≤
  encoded_dependentWitness0 ∧
  0 ≤ encoded_dependentWitness1 ∧
  (encoded_dependentWitness0 + encoded_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def child_ref (encoded : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : SailM NodeRef := do
  let encoded_dependentWitness0 := (encoded).1
  let encoded_dependentWitness1 := ((encoded).2).1
  let encoded := ((encoded).2).2
  if ((encoded.len <b MPT_HASH_LENGTH) : Bool)
  then
    (do
      let inline_node ← do (inline_node_from_scratch_slice ⟨_, ⟨_, encoded⟩⟩)
      (pure (ScratchInlineRef inline_node)))
  else
    (do
      let node_hash ← do (scratch_keccak256 ⟨_, ⟨_, encoded⟩⟩)
      (pure (HashRef node_hash)))

/-- Returns the one-hot presence mask for a branch-child nibble. -/
def branch_mask_for (index : (BitVec 4)) : (BitVec 16) :=
  let shift := (BitVec.toNatInt index)
  (0x0001#16 <<< shift)

/-- Whether the mask records a child at the given nibble. -/
def branch_mask_has (mask : (BitVec 16)) (index : (BitVec 4)) : Bool :=
  let index_mask := (branch_mask_for index)
  ((mask &&& index_mask) != 0x0000#16)

/-- Returns the mask with the child at the given nibble marked present. -/
def branch_mask_set (mask : (BitVec 16)) (index : (BitVec 4)) : (BitVec 16) :=
  let index_mask := (branch_mask_for index)
  (mask ||| index_mask)

/-- The child reference of a leaf, keeping the value in its native
representation: long nodes hash the RLP framing and value as segments;
only an inline node materializes a slice. -/
/- Type quantifiers: value_dependentWitness1 : Nat, value_dependentWitness0 : Nat, 0 ≤
  value_dependentWitness0 ∧
  0 ≤ value_dependentWitness1 ∧
  (value_dependentWitness0 + value_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def input_leaf_child_ref (key : TriePath) (value : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM NodeRef := do
  let value_dependentWitness0 := (value).1
  let value_dependentWitness1 := ((value).2).1
  let value := ((value).2).2
  let path_size ← do (rlp_hex_prefix_size key true)
  let value_size ← do (rlp_input_scratch_slice_size ⟨_, ⟨_, value⟩⟩)
  let content_len ← do (rlp_scratch_length_add path_size value_size)
  let encoded_size ← do (rlp_scratch_list_size content_len)
  let encoder ← do (rlp_encoder_begin encoded_size)
  (rlp_write_list_prefix content_len)
  (rlp_write_hex_prefix key true)
  (rlp_write_input_slice ⟨_, ⟨_, value⟩⟩)
  let ⟨_, ⟨_, encoded⟩⟩ ← do (rlp_encoder_finish encoder)
  let result ← do (child_ref ⟨_, ⟨_, encoded⟩⟩)
  (rlp_encoder_rewind encoder)
  (pure result)

/-- [input_leaf_child_ref][] for a leaf value encoded in the scratch
arena (YP Appendix D, Eq. 208). -/
/- Type quantifiers: value_dependentWitness1 : Nat, value_dependentWitness0 : Nat, 0 ≤
  value_dependentWitness0 ∧
  0 ≤ value_dependentWitness1 ∧
  (value_dependentWitness0 + value_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def scratch_leaf_child_ref (key : TriePath) (value : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : SailM NodeRef := do
  let value_dependentWitness0 := (value).1
  let value_dependentWitness1 := ((value).2).1
  let value := ((value).2).2
  let path_size ← do (rlp_hex_prefix_size key true)
  let value_size ← do (rlp_scratch_scratch_slice_size ⟨_, ⟨_, value⟩⟩)
  let content_len ← do (rlp_scratch_length_add path_size value_size)
  let encoded_size ← do (rlp_scratch_list_size content_len)
  let encoder ← do (rlp_encoder_begin encoded_size)
  (rlp_write_list_prefix content_len)
  (rlp_write_hex_prefix key true)
  (rlp_write_scratch_slice ⟨_, ⟨_, value⟩⟩)
  let ⟨_, ⟨_, encoded⟩⟩ ← do (rlp_encoder_finish encoder)
  let result ← do (child_ref ⟨_, ⟨_, encoded⟩⟩)
  (rlp_encoder_rewind encoder)
  (pure result)

/-- The child reference of a leaf, selecting the encoder for the value's
backing region. -/
def leaf_child_ref (key : TriePath) (value : TrieLeafValue) : SailM NodeRef := do
  match value with
  | .InputTrieLeaf ⟨_, ⟨_, bytes⟩⟩ => (input_leaf_child_ref key ⟨_, ⟨_, bytes⟩⟩)
  | .ScratchTrieLeaf ⟨_, ⟨_, bytes⟩⟩ => (scratch_leaf_child_ref key ⟨_, ⟨_, bytes⟩⟩)

/-- The child reference of an extension node. -/
def extension_child_ref (key : TriePath) (childref : NodeRef) : SailM NodeRef := do
  let path_length ← do (rlp_hex_prefix_size key false)
  let child_length := (node_ref_size childref)
  let content_len := (path_length + child_length)
  let encoded_size ← do (rlp_list_size content_len)
  let encoder ← do (rlp_encoder_begin encoded_size)
  (rlp_write_list_prefix content_len)
  (rlp_write_hex_prefix key false)
  (rlp_write_node_ref childref)
  let ⟨_, ⟨_, encoded⟩⟩ ← do (rlp_encoder_finish encoder)
  let result ← do (child_ref ⟨_, ⟨_, encoded⟩⟩)
  (rlp_encoder_rewind encoder)
  (pure result)

/-- The child reference of a branch node. -/
def branch_child_ref (mask : (BitVec 16)) (children : (Vector NodeRef 16)) : SailM NodeRef := do
  let content_length : Nat := 1
  let child_bit : (BitVec 16) := 0x0001#16
  let (child_bit, content_length) ← (( do
    let loop_i_lower := 0
    let loop_i_upper := 15
    let mut loop_vars := (child_bit, content_length)
    for i in [loop_i_lower:loop_i_upper:1]i do
      let (child_bit, content_length) := loop_vars
      loop_vars ← do
        let child_present := ((mask &&& child_bit) != 0x0000#16)
        let content_length ← (( do
          if (child_present : Bool)
          then
            (do
              let childref := (GetElem?.getElem! children i)
              let child_length := (node_ref_size childref)
              (branch_content_length_add content_length child_length))
          else
            (do
              (branch_content_length_add content_length 1)) ) : SailM Nat )
        let child_bit : (BitVec 16) := (child_bit <<< 1)
        (pure (child_bit, content_length))
    (pure loop_vars) ) : SailM ((BitVec 16) × Nat) )
  let scratch_content_length ← do (rlp_scratch_length_add content_length 0)
  let encoded_size ← do (rlp_scratch_list_size scratch_content_length)
  let encoder ← do (rlp_encoder_begin encoded_size)
  (rlp_write_list_prefix content_length)
  let child_bit : (BitVec 16) := 0x0001#16
  let child_bit ← (( do
    let loop_i_lower := 0
    let loop_i_upper := 15
    let mut loop_vars := child_bit
    for i in [loop_i_lower:loop_i_upper:1]i do
      let child_bit := loop_vars
      loop_vars ← do
        let child_present := ((mask &&& child_bit) != 0x0000#16)
        if (child_present : Bool)
        then
          (do
            let childref := (GetElem?.getElem! children i)
            (rlp_write_node_ref childref))
        else (scratch_push_byte 0x80#8)
        (pure (child_bit <<< 1))
    (pure loop_vars) ) : SailM (BitVec 16) )
  (scratch_push_byte 0x80#8)
  let ⟨_, ⟨_, encoded⟩⟩ ← do (rlp_encoder_finish encoder)
  let result ← do (child_ref ⟨_, ⟨_, encoded⟩⟩)
  (rlp_encoder_rewind encoder)
  (pure result)

/-- The root hash a node reference commits to; the empty reference is the
empty-trie root. -/
def trie_ref_to_root (r : NodeRef) : SailM (Vector (BitVec 8) 32) := do
  match r with
  | .EmptyRef () => (pure EMPTY_TRIE_ROOT)
  | .InputInlineRef ⟨_, ⟨_, node⟩⟩ => (stateless_input_keccak256 ⟨_, ⟨_, node⟩⟩)
  | .ScratchInlineRef node => (inline_node_hash node)
  | .HashRef h => (pure h)

/-- The reference form of authenticated input node bytes. -/
/- Type quantifiers: node_dependentWitness1 : Nat, node_dependentWitness0 : Nat, 0 ≤
  node_dependentWitness0 ∧
  0 ≤ node_dependentWitness1 ∧
  (node_dependentWitness0 + node_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def input_node_to_ref (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM NodeRef := do
  let node_dependentWitness0 := (node).1
  let node_dependentWitness1 := ((node).2).1
  let node := ((node).2).2
  if ((node.len == 0) : Bool)
  then (pure (EmptyRef ()))
  else
    (do
      if ((node.len <b MPT_HASH_LENGTH) : Bool)
      then (pure (InputInlineRef ⟨_, ⟨_, node⟩⟩))
      else
        (do
          let node_hash ← do (stateless_input_keccak256 ⟨_, ⟨_, node⟩⟩)
          (pure (HashRef node_hash))))

/-- The reference form of freshly encoded scratch node bytes. -/
/- Type quantifiers: node_dependentWitness1 : Nat, node_dependentWitness0 : Nat, 0 ≤
  node_dependentWitness0 ∧
  0 ≤ node_dependentWitness1 ∧
  (node_dependentWitness0 + node_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def scratch_node_to_ref (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : SailM NodeRef := do
  let node_dependentWitness0 := (node).1
  let node_dependentWitness1 := ((node).2).1
  let node := ((node).2).2
  if ((node.len == 0) : Bool)
  then (pure (EmptyRef ()))
  else
    (do
      if ((node.len <b MPT_HASH_LENGTH) : Bool)
      then
        (do
          let inline_node ← do (inline_node_from_scratch_slice ⟨_, ⟨_, node⟩⟩)
          (pure (ScratchInlineRef inline_node)))
      else
        (do
          let node_hash ← do (scratch_keccak256 ⟨_, ⟨_, node⟩⟩)
          (pure (HashRef node_hash))))

/-- [input_field_to_ref][] for a scratch-backed child field; an inline
list is copied into a self-contained inline node. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def scratch_field_to_ref (f : (ScratchRlpFieldRef k_source_off k_source_len k_content_len)) : SailM NodeRef := do
  if (f.is_list : Bool)
  then
    (do
      let inline_node ← do (inline_node_from_scratch_slice ⟨_, ⟨_, f.source⟩⟩)
      (pure (ScratchInlineRef inline_node)))
  else
    (do
      if ((k_content_len == MPT_HASH_LENGTH) : Bool)
      then
        (do
          let word ← do (scratch_rlp_decode_word f)
          let hash := (word_to_hash word)
          (pure (HashRef hash)))
      else (pure (EmptyRef ())))

/-- [decode_input_branch_node][] over the scratch cursor family. -/
/- Type quantifiers: _reclimit : Nat, k_ex551647_ : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 2
  ≤ k_ex551647_ ∧ k_ex551647_ ≤ 16, 0 ≤ _reclimit -/
def _rec_decode_scratch_branch_node (cursor : (ScratchSliceFields k_source_off k_source_len)) (index : Nat) (children : (Vector NodeRef 16)) (_reclimit : Nat) : SailM ScratchTrieNode := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((index <b 16) : Bool)
      then
        (do
          let ⟨_, ⟨_, child⟩⟩ ← do (scratch_rlp_decode_item cursor)
          let next := (scratch_rlp_cursor_advance cursor child.source.len)
          let decoded_child ← do (scratch_field_to_ref child)
          let updated := children
          let updated : (Vector NodeRef 16) := (vectorUpdate updated index decoded_child)
          (_rec_decode_scratch_branch_node next (index + 1) updated _reclimit_pred))
      else
        (do
          let ⟨_, ⟨_, value⟩⟩ ← do (scratch_rlp_decode_item cursor)
          let next := (scratch_rlp_cursor_advance cursor value.source.len)
          (scratch_rlp_cursor_expect_end next)
          let ⟨_, ⟨_, content⟩⟩ := (scratch_rlp_item_content value)
          (pure (ScratchBranchNode
              ((fun (dependentValue0, dependentValue1) => (dependentValue0, ⟨_, ⟨_, dependentValue1⟩⟩)) ((children, content)))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- [decode_input_branch_node][] over the scratch cursor family. -/
/- Type quantifiers: index : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 2
  ≤ index ∧ index ≤ 16 -/
def decode_scratch_branch_node (cursor : (ScratchSliceFields k_source_off k_source_len)) (index : Nat) (children : (Vector NodeRef 16)) : SailM ScratchTrieNode := do
  let _measure := ((16 - index) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_decode_scratch_branch_node cursor index children (_measure + 1))

/-- [decode_input_trie_node][] over freshly encoded scratch node bytes. -/
/- Type quantifiers: node_dependentWitness1 : Nat, node_dependentWitness0 : Nat, 0 ≤
  node_dependentWitness0 ∧
  0 ≤ node_dependentWitness1 ∧
  (node_dependentWitness0 + node_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def decode_scratch_trie_node (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : SailM ScratchTrieNode := do
  let node_dependentWitness0 := (node).1
  let node_dependentWitness1 := ((node).2).1
  let node := ((node).2).2
  let ⟨_, ⟨_, fields⟩⟩ ← do (scratch_rlp_node_cursor ⟨_, ⟨_, node⟩⟩)
  let ⟨_, ⟨_, first⟩⟩ ← do (scratch_rlp_decode_item fields)
  let fields := (scratch_rlp_cursor_advance fields first.source.len)
  let ⟨_, ⟨_, second⟩⟩ ← do (scratch_rlp_decode_item fields)
  let fields := (scratch_rlp_cursor_advance fields second.source.len)
  if ((fields.len == 0) : Bool)
  then
    (do
      let (is_leaf, path) ← do (scratch_hex_prefix_decode_ref first)
      if (is_leaf : Bool)
      then
        (let ⟨_, ⟨_, value⟩⟩ := (scratch_rlp_item_content second)
        (pure (ScratchLeafNode
            ((fun (dependentValue0, dependentValue1) => (dependentValue0, ⟨_, ⟨_, dependentValue1⟩⟩)) ((path, value))))))
      else
        (do
          let path_length := (path_len path)
          if ((path_length == 0) : Bool)
          then (fatal_error RlpDecode)
          else
            (do
              let child ← do (scratch_field_to_ref second)
              (pure (ScratchExtensionNode (path, child))))))
  else
    (do
      let empty_child := (EmptyRef ())
      let first_child ← do (scratch_field_to_ref first)
      let second_child ← do (scratch_field_to_ref second)
      let children : (Vector NodeRef 16) := (vectorInit empty_child)
      let children : (Vector NodeRef 16) := (vectorUpdate children 0 first_child)
      let children : (Vector NodeRef 16) := (vectorUpdate children 1 second_child)
      (decode_scratch_branch_node fields 2 children))

/-- Re-keys a decoded child node under `evm_prefix` without copying a leaf
value. -/
/- Type quantifiers: childnode_dependentWitness1 : Nat, childnode_dependentWitness0 : Nat, 0 ≤
  childnode_dependentWitness0 ∧
  0 ≤ childnode_dependentWitness1 ∧
  (childnode_dependentWitness0 + childnode_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def merge_ext_node (evm_prefix' : TriePath) (childnode : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM NodeRef := do
  let childnode_dependentWitness0 := (childnode).1
  let childnode_dependentWitness1 := ((childnode).2).1
  let childnode := ((childnode).2).2
  let prefix_length := (path_len evm_prefix')
  if ((prefix_length == 0) : Bool)
  then (input_node_to_ref ⟨_, ⟨_, childnode⟩⟩)
  else
    (do
      if ((childnode.len == 0) : Bool)
      then (pure (EmptyRef ()))
      else
        (do
          let decoded ← do (decode_input_trie_node ⟨_, ⟨_, childnode⟩⟩)
          match decoded with
          | .InputLeafNode (path, value) =>
            (do
              let merged_path ← do (path_concat evm_prefix' path)
              (input_leaf_child_ref merged_path value))
          | .InputExtensionNode (path, child) =>
            (do
              let merged_path ← do (path_concat evm_prefix' path)
              (extension_child_ref merged_path child))
          | _ =>
            (do
              let childref ← do (input_node_to_ref ⟨_, ⟨_, childnode⟩⟩)
              (extension_child_ref evm_prefix' childref))))

/-- [merge_ext_node][] over a child reference: an inline reference
carries its node bytes and re-keys canonically; a 32-byte hash
reference is wrapped in an extension, which is canonical only when the
referenced node is a branch. -/
def merge_ext_ref (evm_prefix' : TriePath) (childref : NodeRef) : SailM NodeRef := do
  let prefix_length := (path_len evm_prefix')
  if ((prefix_length == 0) : Bool)
  then (pure childref)
  else
    (do
      match childref with
      | .EmptyRef () => (pure (EmptyRef ()))
      | .HashRef _ => (extension_child_ref evm_prefix' childref)
      | .InputInlineRef ⟨_, ⟨_, node⟩⟩ =>
        (do
          let decoded ← do (decode_input_trie_node ⟨_, ⟨_, node⟩⟩)
          match decoded with
          | .InputLeafNode (path, value) =>
            (do
              let merged_path ← do (path_concat evm_prefix' path)
              (input_leaf_child_ref merged_path value))
          | .InputExtensionNode (path, child) =>
            (do
              let merged_path ← do (path_concat evm_prefix' path)
              (extension_child_ref merged_path child))
          | _ => (extension_child_ref evm_prefix' childref))
      | .ScratchInlineRef node =>
        (do
          let ⟨_, ⟨_, node_slice⟩⟩ ← do (inline_node_slice node)
          let decoded ← do (decode_scratch_trie_node ⟨_, ⟨_, node_slice⟩⟩)
          match decoded with
          | .ScratchLeafNode (path, value) =>
            (do
              let merged_path ← do (path_concat evm_prefix' path)
              (scratch_leaf_child_ref merged_path value))
          | .ScratchExtensionNode (path, child) =>
            (do
              let merged_path ← do (path_concat evm_prefix' path)
              (extension_child_ref merged_path child))
          | _ => (extension_child_ref evm_prefix' childref)))
