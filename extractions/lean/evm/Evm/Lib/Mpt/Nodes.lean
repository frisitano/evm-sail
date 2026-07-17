import Evm.Flow
import Evm.Arith
import Evm.Primitives.Quantities
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

def inline_node_segment (node : InlineNode) : Bytes :=
  (bytes_list (inline_node_to_list node) node.len)

def branch_refs_get (children : (Vector NodeRef 16)) (index : (BitVec 4)) : NodeRef :=
  match index with
  | 0x0 => (GetElem?.getElem! children 0)
  | 0x1 => (GetElem?.getElem! children 1)
  | 0x2 => (GetElem?.getElem! children 2)
  | 0x3 => (GetElem?.getElem! children 3)
  | 0x4 => (GetElem?.getElem! children 4)
  | 0x5 => (GetElem?.getElem! children 5)
  | 0x6 => (GetElem?.getElem! children 6)
  | 0x7 => (GetElem?.getElem! children 7)
  | 0x8 => (GetElem?.getElem! children 8)
  | 0x9 => (GetElem?.getElem! children 9)
  | 0xA => (GetElem?.getElem! children 10)
  | 0xB => (GetElem?.getElem! children 11)
  | 0xC => (GetElem?.getElem! children 12)
  | 0xD => (GetElem?.getElem! children 13)
  | 0xE => (GetElem?.getElem! children 14)
  | _ => (GetElem?.getElem! children 15)

def branch_refs_set (children : (Vector NodeRef 16)) (index : (BitVec 4)) (value : NodeRef) : (Vector NodeRef 16) :=
  let result := children
  match index with
  | 0x0 => (vectorUpdate result 0 value)
  | 0x1 => (vectorUpdate result 1 value)
  | 0x2 => (vectorUpdate result 2 value)
  | 0x3 => (vectorUpdate result 3 value)
  | 0x4 => (vectorUpdate result 4 value)
  | 0x5 => (vectorUpdate result 5 value)
  | 0x6 => (vectorUpdate result 6 value)
  | 0x7 => (vectorUpdate result 7 value)
  | 0x8 => (vectorUpdate result 8 value)
  | 0x9 => (vectorUpdate result 9 value)
  | 0xA => (vectorUpdate result 10 value)
  | 0xB => (vectorUpdate result 11 value)
  | 0xC => (vectorUpdate result 12 value)
  | 0xD => (vectorUpdate result 13 value)
  | 0xE => (vectorUpdate result 14 value)
  | _ => (vectorUpdate result 15 value)

def node_ref_size (r : NodeRef) : byte_quantity :=
  match r with
  | .EmptyRef () => BYTE_ONE
  | .InlineRef node => node.len
  | .HashRef _ => (rlp_word_size ())

def rlp_write_node_ref (r : NodeRef) : SailM Unit := do
  match r with
  | .EmptyRef () => (scratch_push_bytes [0x80#8] BYTE_ONE)
  | .InlineRef node => (rlp_write_raw_bytes (inline_node_to_list node) node.len)
  | .HashRef h => (rlp_write_word h)

def child_ref (encoded : EvmByteSlice) : SailM NodeRef := do
  if ((byte_quantity_lt encoded.len MPT_HASH_LENGTH) : Bool)
  then (pure (InlineRef (← (inline_node_from_slice encoded))))
  else (pure (HashRef (← (keccak256_slice encoded))))

def branch_mask_for (index : (BitVec 4)) : (BitVec 16) :=
  match index with
  | 0x0 => 0x0001#16
  | 0x1 => 0x0002#16
  | 0x2 => 0x0004#16
  | 0x3 => 0x0008#16
  | 0x4 => 0x0010#16
  | 0x5 => 0x0020#16
  | 0x6 => 0x0040#16
  | 0x7 => 0x0080#16
  | 0x8 => 0x0100#16
  | 0x9 => 0x0200#16
  | 0xA => 0x0400#16
  | 0xB => 0x0800#16
  | 0xC => 0x1000#16
  | 0xD => 0x2000#16
  | 0xE => 0x4000#16
  | _ => 0x8000#16

def branch_mask_has (mask : (BitVec 16)) (index : (BitVec 4)) : Bool :=
  ((mask &&& (branch_mask_for index)) != 0x0000#16)

def branch_mask_set (mask : (BitVec 16)) (index : (BitVec 4)) : (BitVec 16) :=
  (mask ||| (branch_mask_for index))

def leaf_child_ref (key : TriePath) (value : EvmByteSlice) : SailM NodeRef := do
  let (path, encoded_path_len) ← do (hex_prefix_compact key true)
  let content_len ← do
    (byte_quantity_add (← (rlp_bytes_size path encoded_path_len)) (← (rlp_slice_size value)))
  let encoded_len ← do (rlp_list_size content_len)
  let mark ← do (scratch_begin ())
  (rlp_write_list_prefix content_len)
  (rlp_write_bytes path encoded_path_len)
  (rlp_write_slice value)
  let result ← do (child_ref (← (rlp_finish mark encoded_len)))
  (scratch_rewind mark)
  (pure result)

def extension_child_ref (key : TriePath) (childref : NodeRef) : SailM NodeRef := do
  let (path, encoded_path_len) ← do (hex_prefix_compact key false)
  let content_len ← do
    (byte_quantity_add (← (rlp_bytes_size path encoded_path_len)) (node_ref_size childref))
  let encoded_len ← do (rlp_list_size content_len)
  let mark ← do (scratch_begin ())
  (rlp_write_list_prefix content_len)
  (rlp_write_bytes path encoded_path_len)
  (rlp_write_node_ref childref)
  let result ← do (child_ref (← (rlp_finish mark encoded_len)))
  (scratch_rewind mark)
  (pure result)

def branch_child_ref (mask : (BitVec 16)) (children : (Vector NodeRef 16)) : SailM NodeRef := do
  let content_len : byte_length := BYTE_ONE
  let child_bit : (BitVec 16) := 0x0001#16
  let (child_bit, content_len) ← (( do
    let loop_i_lower := 0
    let loop_i_upper := 15
    let mut loop_vars := (child_bit, content_len)
    for i in [loop_i_lower:loop_i_upper:1]i do
      let (child_bit, content_len) := loop_vars
      loop_vars ← do
        let content_len ← (( do
          if (((mask &&& child_bit) != 0x0000#16) : Bool)
          then
            (do
              (byte_quantity_add content_len (node_ref_size (GetElem?.getElem! children i))))
          else
            (do
              (byte_quantity_add content_len BYTE_ONE)) ) : SailM byte_quantity )
        let child_bit : (BitVec 16) := (child_bit <<< 1)
        (pure (child_bit, content_len))
    (pure loop_vars) ) : SailM ((BitVec 16) × byte_quantity) )
  let encoded_len ← do (rlp_list_size content_len)
  let mark ← do (scratch_begin ())
  (rlp_write_list_prefix content_len)
  let child_bit : (BitVec 16) := 0x0001#16
  let child_bit ← (( do
    let loop_i_lower := 0
    let loop_i_upper := 15
    let mut loop_vars_1 := child_bit
    for i in [loop_i_lower:loop_i_upper:1]i do
      let child_bit := loop_vars_1
      loop_vars_1 ← do
        if (((mask &&& child_bit) != 0x0000#16) : Bool)
        then (rlp_write_node_ref (GetElem?.getElem! children i))
        else (scratch_push_bytes [0x80#8] BYTE_ONE)
        (pure (child_bit <<< 1))
    (pure loop_vars_1) ) : SailM (BitVec 16) )
  (scratch_push_bytes [0x80#8] BYTE_ONE)
  let result ← do (child_ref (← (rlp_finish mark encoded_len)))
  (scratch_rewind mark)
  (pure result)

def trie_ref_to_root (r : NodeRef) : SailM (BitVec 256) := do
  match r with
  | .EmptyRef () => (pure EMPTY_TRIE_ROOT)
  | .InlineRef node => (keccak256_segments [(inline_node_segment node)])
  | .HashRef h => (pure h)

def node_to_ref (node : EvmByteSlice) : SailM NodeRef := do
  if ((byte_quantity_equal node.len BYTE_ZERO) : Bool)
  then (pure (EmptyRef ()))
  else
    (do
      if ((byte_quantity_lt node.len MPT_HASH_LENGTH) : Bool)
      then (pure (InlineRef (← (inline_node_from_slice node))))
      else (pure (HashRef (← (keccak256_slice node)))))

def merge_ext_node (evm_prefix' : TriePath) (childnode : EvmByteSlice) : SailM NodeRef := do
  if (((path_len evm_prefix') == 0) : Bool)
  then (node_to_ref childnode)
  else
    (do
      if ((byte_quantity_equal childnode.len BYTE_ZERO) : Bool)
      then (pure (EmptyRef ()))
      else
        (do
          match (← (decode_trie_node childnode)) with
          | .LeafNode leaf =>
            (leaf_child_ref (← (path_concat evm_prefix' leaf.path)) (← (rlp_ref_content leaf.value)))
          | .ExtensionNode ext =>
            (extension_child_ref (← (path_concat evm_prefix' ext.path)) (← (field_to_ref ext.child)))
          | _ => (extension_child_ref evm_prefix' (← (node_to_ref childnode)))))

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
          match (← (decode_trie_node (← (inline_node_slice node)))) with
          | .LeafNode leaf =>
            (leaf_child_ref (← (path_concat evm_prefix' leaf.path)) (← (rlp_ref_content leaf.value)))
          | .ExtensionNode ext =>
            (extension_child_ref (← (path_concat evm_prefix' ext.path)) (← (field_to_ref ext.child)))
          | _ => (extension_child_ref evm_prefix' childref)))

