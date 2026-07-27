import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Primitives.Crypto
import Evm.Host.Kernel.Scratch
import Evm.Lib.Rlp.Rlp
import Evm.Host.Kernel.Storage
import Evm.Lib.Mpt.Primitives

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
open BalIterEntry

/-! # Trie nodes

Merkle-Patricia trie node forms, references, and decoding
(YP Appendix D). -/

def undefined_InlineNode (_ : Unit) : SailM InlineNode := do
  (pure { data := ← (undefined_vector 32 (← (undefined_bitvector 8))),
          len := ← (undefined_range 0 31) })

/-- Hashes an inline node directly from its fixed byte vector. -/
def inline_node_hash (node : InlineNode) : SailM (Vector (BitVec 8) 32) := do
  (keccak256_segments [(bytes_fixed32 node.data node.len)])

/-- Advances the branch payload length while preserving its structural bound. -/
/- Type quantifiers: k_ex415881_ : Nat, k_ex415880_ : Nat, 0 ≤ k_ex415880_ ∧ k_ex415880_ ≤ 529, 0
  ≤ k_ex415881_ ∧ k_ex415881_ ≤ 33 -/
def branch_content_length_add (current : Nat) (addition : Nat) : SailM Nat := do
  if ((addition ≤b (529 - current)) : Bool)
  then (pure (current + addition))
  else sailThrow ((InvalidBlock RlpDecode))

/-- Returns the RLP width of a child reference in its parent node. -/
def node_ref_size (r : NodeRef) : Nat :=
  match r with
  | .EmptyRef () => 1
  | .InlineRef node => node.len
  | .HashRef _ => (rlp_word_size ())

/-- Appends a child reference in its canonical RLP representation. -/
def rlp_write_node_ref (r : NodeRef) : SailM Unit := do
  match r with
  | .EmptyRef () => (scratch_push_bytes [0x80#8] 1)
  | .InlineRef node => (scratch_push_b256 node.data node.len)
  | .HashRef h => (rlp_write_word (hash_to_word h))

/-- The canonical child reference for an encoded node: inline under 32
bytes, otherwise its hash (YP Appendix D, Eq. 207). -/
/- Type quantifiers: encoded_dependentWitness1 : Nat, encoded_dependentWitness0 : Nat, 0 ≤
  encoded_dependentWitness0 ∧ 0 ≤ encoded_dependentWitness1 -/
def child_ref (encoded : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM NodeRef := do
  let encoded_dependentWitness0 := (encoded).1
  let encoded_dependentWitness1 := ((encoded).2).1
  let encoded := ((encoded).2).2
  if ((encoded.len <b MPT_HASH_LENGTH) : Bool)
  then (pure (InlineRef (← (inline_node_from_slice ⟨_, ⟨_, encoded⟩⟩))))
  else (pure (HashRef (← (keccak256_slice ⟨_, ⟨_, encoded⟩⟩))))

/-- Returns the one-hot presence mask for a branch-child nibble. -/
def branch_mask_for (index : (BitVec 4)) : (BitVec 16) :=
  (0x0001#16 <<< (BitVec.toNatInt index))

def branch_mask_has (mask : (BitVec 16)) (index : (BitVec 4)) : Bool :=
  ((mask &&& (branch_mask_for index)) != 0x0000#16)

def branch_mask_set (mask : (BitVec 16)) (index : (BitVec 4)) : (BitVec 16) :=
  (mask ||| (branch_mask_for index))

/-- The child reference of a leaf, keeping the value in its native
representation: long nodes hash the RLP framing and value as segments;
only an inline node materializes a slice. -/
/- Type quantifiers: value_dependentWitness1 : Nat, value_dependentWitness0 : Nat, 0 ≤
  value_dependentWitness0 ∧ 0 ≤ value_dependentWitness1 -/
def leaf_child_ref (key : TriePath) (value : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM NodeRef := do
  let value_dependentWitness0 := (value).1
  let value_dependentWitness1 := ((value).2).1
  let value := ((value).2).2
  let (path, encoded_path_len) ← do (hex_prefix_compact key true)
  let content_len ← do
    (pure (rlp_scratch_length_add (← (rlp_scratch_bytes_size path encoded_path_len))
        (← (rlp_scratch_slice_size ⟨_, ⟨_, value⟩⟩))))
  let mark ← do (scratch_begin ())
  (rlp_write_list_prefix content_len)
  (rlp_write_bytes path encoded_path_len)
  (rlp_write_slice ⟨_, ⟨_, value⟩⟩)
  let result ← do
    (do
        let dependentArg0 := (← (rlp_finish mark))
        (child_ref dependentArg0))
  (scratch_rewind mark)
  (pure result)

/-- The child reference of an extension node. -/
def extension_child_ref (key : TriePath) (childref : NodeRef) : SailM NodeRef := do
  let (path, encoded_path_len) ← do (hex_prefix_compact key false)
  let path_length ← do (rlp_bytes_size path encoded_path_len)
  let child_length := (node_ref_size childref)
  let content_len := (path_length + child_length)
  let mark ← do (scratch_begin ())
  (rlp_write_list_prefix content_len)
  (rlp_write_bytes path encoded_path_len)
  (rlp_write_node_ref childref)
  let result ← do
    (do
        let dependentArg0 := (← (rlp_finish mark))
        (child_ref dependentArg0))
  (scratch_rewind mark)
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
        let content_length ← (( do
          if (((mask &&& child_bit) != 0x0000#16) : Bool)
          then
            (do
              let child_length := (node_ref_size (GetElem?.getElem! children i))
              (branch_content_length_add content_length child_length))
          else
            (do
              (branch_content_length_add content_length 1)) ) : SailM Nat )
        let child_bit : (BitVec 16) := (child_bit <<< 1)
        (pure (child_bit, content_length))
    (pure loop_vars) ) : SailM ((BitVec 16) × Nat) )
  let mark ← do (scratch_begin ())
  (rlp_write_list_prefix content_length)
  let child_bit : (BitVec 16) := 0x0001#16
  let child_bit ← (( do
    let loop_i_lower := 0
    let loop_i_upper := 15
    let mut loop_vars := child_bit
    for i in [loop_i_lower:loop_i_upper:1]i do
      let child_bit := loop_vars
      loop_vars ← do
        if (((mask &&& child_bit) != 0x0000#16) : Bool)
        then (rlp_write_node_ref (GetElem?.getElem! children i))
        else (scratch_push_bytes [0x80#8] 1)
        (pure (child_bit <<< 1))
    (pure loop_vars) ) : SailM (BitVec 16) )
  (scratch_push_bytes [0x80#8] 1)
  let result ← do
    (do
        let dependentArg0 := (← (rlp_finish mark))
        (child_ref dependentArg0))
  (scratch_rewind mark)
  (pure result)

/-- The root hash a node reference commits to; the empty reference is the
empty-trie root. -/
def trie_ref_to_root (r : NodeRef) : SailM (Vector (BitVec 8) 32) := do
  match r with
  | .EmptyRef () => (pure EMPTY_TRIE_ROOT)
  | .InlineRef node => (inline_node_hash node)
  | .HashRef h => (pure h)

/-- The reference form of raw node bytes: empty, inline under 32 bytes,
else hashed. -/
/- Type quantifiers: node_dependentWitness1 : Nat, node_dependentWitness0 : Nat, 0 ≤
  node_dependentWitness0 ∧ 0 ≤ node_dependentWitness1 -/
def node_to_ref (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM NodeRef := do
  let node_dependentWitness0 := (node).1
  let node_dependentWitness1 := ((node).2).1
  let node := ((node).2).2
  if ((node.len == 0) : Bool)
  then (pure (EmptyRef ()))
  else
    (do
      if ((node.len <b MPT_HASH_LENGTH) : Bool)
      then (pure (InlineRef (← (inline_node_from_slice ⟨_, ⟨_, node⟩⟩))))
      else (pure (HashRef (← (keccak256_slice ⟨_, ⟨_, node⟩⟩)))))

/-- Re-keys a decoded child node under `evm_prefix` without copying a leaf
value. -/
/- Type quantifiers: childnode_dependentWitness1 : Nat, childnode_dependentWitness0 : Nat, 0 ≤
  childnode_dependentWitness0 ∧ 0 ≤ childnode_dependentWitness1 -/
def merge_ext_node (evm_prefix' : TriePath) (childnode : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM NodeRef := do
  let childnode_dependentWitness0 := (childnode).1
  let childnode_dependentWitness1 := ((childnode).2).1
  let childnode := ((childnode).2).2
  if (((path_len evm_prefix') == 0) : Bool)
  then (node_to_ref ⟨_, ⟨_, childnode⟩⟩)
  else
    (do
      if ((childnode.len == 0) : Bool)
      then (pure (EmptyRef ()))
      else
        (do
          match (← (decode_trie_node ⟨_, ⟨_, childnode⟩⟩)) with
          | .LeafNode (path, value) => (leaf_child_ref (← (path_concat evm_prefix' path)) value)
          | .ExtensionNode (path, child) =>
            (extension_child_ref (← (path_concat evm_prefix' path)) child)
          | _ => (extension_child_ref evm_prefix' (← (node_to_ref ⟨_, ⟨_, childnode⟩⟩)))))

/-- [merge_ext_node][] over a child reference: an inline reference
carries its node bytes and re-keys canonically; a 32-byte hash
reference is wrapped in an extension, which is canonical only when the
referenced node is a branch. -/
def merge_ext_ref (evm_prefix' : TriePath) (childref : NodeRef) : SailM NodeRef := do
  if (((path_len evm_prefix') == 0) : Bool)
  then (pure childref)
  else
    (do
      match childref with
      | .EmptyRef () => (pure (EmptyRef ()))
      | .HashRef _ => (extension_child_ref evm_prefix' childref)
      | .InlineRef node =>
        (do
          match (← do
              let dependentArg0 := (← (inline_node_slice node))
              (decode_trie_node dependentArg0)) with
          | .LeafNode (path, value) => (leaf_child_ref (← (path_concat evm_prefix' path)) value)
          | .ExtensionNode (path, child) =>
            (extension_child_ref (← (path_concat evm_prefix' path)) child)
          | _ => (extension_child_ref evm_prefix' childref)))

