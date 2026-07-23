import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Primitives.Crypto
import Evm.Primitives.Account
import Evm.Host.EvmByteSlice
import Evm.Host.Kernel.Scratch
import Evm.Lib.Rlp.Rlp
import Evm.Host.Kernel.Environment

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

/-! # State: storage

Warm/cold accounting (EIP-2929), persistent storage (`SLOAD`/`SSTORE`),
and transient storage (EIP-1153), over the host state stores. -/

/- Type quantifiers: k_ex408155_ : Nat, 0 ≤ k_ex408155_ ∧ k_ex408155_ ≤ (2 ^ 256 - 1) -/
def storage_key (a : address) (s : word) : StorageKey :=
  let s := (s).value
  { addr := a,
    slot := ⟨s⟩ }

/-- Marks an address warm and returns its prior warm bit (EIP-2929: the
EVM charges the cold cost on `false`, the warm cost on `true`). The
host includes a new warm entry in its semantic checkpoint. -/
def k_access_account (a : address) : SailM Bool := do
  (warm_addr_touch a)

/-- Marks a storage slot warm and returns its prior warm bit. New entries
belong to the host's semantic checkpoint. -/
/- Type quantifiers: k_ex408156_ : Nat, 0 ≤ k_ex408156_ ∧ k_ex408156_ ≤ (2 ^ 256 - 1) -/
def k_slot_is_warm (a : address) (s : word) : SailM Bool := do
  let s := (s).value
  (warm_slot_touch a ⟨s⟩)

/-- Decodes an account trie leaf — `rlp([nonce, balance, storage_root,
code_hash])` — into an [AccountInfo][type-AccountInfo]; empty
root/hash fields decode to their empty-sentinel digests. -/
/- Type quantifiers: k_ex408160_ : Nat, k_ex408159_ : Nat, 0 ≤ k_ex408159_ ∧ 0 ≤ k_ex408160_ -/
def decode_state_account (value : EvmByteSlice) : SailM AccountInfo := do
  let value := ((value).2).2
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, ⟨k_ex416961_, ⟨k_ex416960_, ⟨k_ex416959_, ⟨k_ex416958_, (nonce, fields)⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    (do
        let ⟨_, ⟨_, ⟨_, ⟨_, dependentArg0⟩⟩⟩⟩ ← (rlp_node_cursor
          ⟨_, ⟨_, value⟩⟩)
        let publicResult ← (rlp_cursor_pop dependentArg0)
        pure ((⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ((((publicResult).2).2).2).2⟩⟩⟩⟩⟩⟩⟩⟩ : (Sigma
        fun (k_ex416958_ : Nat) =>
        (Sigma fun (k_ex416959_ : Nat) =>
        (Sigma fun (k_ex416960_ : Nat) =>
        (Sigma fun (k_ex416961_ : Nat) =>
        (Sigma fun (k_content : Nat) =>
        (Sigma fun (k_content_len : Nat) =>
        (Sigma fun (k_next : Nat) =>
        (Sigma fun (k_full_len : Nat) =>
        ((RlpFieldRefFields k_ex416958_ k_ex416959_ k_ex416960_ k_full_len k_content k_content_len) × (RlpCursorFields k_ex416958_ k_ex416959_ k_next k_ex416961_)))))))))))))
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (balance, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (storage, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (code, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  (rlp_cursor_expect_end ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩)
  let storage_root ← do
    if ((storage.content_len == 0) : Bool)
    then (pure EMPTY_TRIE_ROOT)
    else
      (pure (word_to_hash
          ⟨((← (rlp_ref_word ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, storage⟩⟩⟩⟩⟩⟩))).value⟩))
  let code_hash ← do
    if ((code.content_len == 0) : Bool)
    then (pure KECCAK_EMPTY)
    else
      (pure (word_to_hash
          ⟨((← (rlp_ref_word ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, code⟩⟩⟩⟩⟩⟩))).value⟩))
  (pure { nonce := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_account_nonce
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, nonce⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          balance := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, balance⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          storage_root := storage_root,
          code_hash := code_hash })

/-- Constructs a path from high-aligned data and a nibble length. -/
/- Type quantifiers: k_ex408161_ : Nat, 0 ≤ k_ex408161_ ∧ k_ex408161_ ≤ 64 -/
def path_new (data : b256) (len : trie_path_len) : TriePath :=
  let len := (len).value
  { data := data,
    len := ⟨len⟩ }

/-- The 64-nibble path of a 32-byte hash — the secure-trie key form. -/
def path_from_hash (h : hash) : TriePath :=
  (path_new h ⟨64⟩)

/-- The witness node bytes whose KECCAK-256 digest is `h`, retained as a
slice into the stateless input; empty if unwitnessed. -/
def node_db_lookup (h : hash) : SailM EvmByteSlice := do
  (nodedb_lookup h)

/-- Selects a decoded branch child field by nibble value. -/
def branch_children_get (children : BranchChildren) (index : nibble) : RlpFieldRef :=
  match index with
  | 0x0 =>
    ((GetElem?.getElem! children 0) : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) =>
    (Sigma fun (k_full_off : Nat) =>
    (Sigma fun (k_full_len : Nat) =>
    (Sigma fun (k_content_off : Nat) =>
    (Sigma fun (k_content_len : Nat) =>
    (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))
  | 0x1 =>
    ((GetElem?.getElem! children 1) : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) =>
    (Sigma fun (k_full_off : Nat) =>
    (Sigma fun (k_full_len : Nat) =>
    (Sigma fun (k_content_off : Nat) =>
    (Sigma fun (k_content_len : Nat) =>
    (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))
  | 0x2 =>
    ((GetElem?.getElem! children 2) : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) =>
    (Sigma fun (k_full_off : Nat) =>
    (Sigma fun (k_full_len : Nat) =>
    (Sigma fun (k_content_off : Nat) =>
    (Sigma fun (k_content_len : Nat) =>
    (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))
  | 0x3 =>
    ((GetElem?.getElem! children 3) : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) =>
    (Sigma fun (k_full_off : Nat) =>
    (Sigma fun (k_full_len : Nat) =>
    (Sigma fun (k_content_off : Nat) =>
    (Sigma fun (k_content_len : Nat) =>
    (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))
  | 0x4 =>
    ((GetElem?.getElem! children 4) : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) =>
    (Sigma fun (k_full_off : Nat) =>
    (Sigma fun (k_full_len : Nat) =>
    (Sigma fun (k_content_off : Nat) =>
    (Sigma fun (k_content_len : Nat) =>
    (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))
  | 0x5 =>
    ((GetElem?.getElem! children 5) : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) =>
    (Sigma fun (k_full_off : Nat) =>
    (Sigma fun (k_full_len : Nat) =>
    (Sigma fun (k_content_off : Nat) =>
    (Sigma fun (k_content_len : Nat) =>
    (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))
  | 0x6 =>
    ((GetElem?.getElem! children 6) : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) =>
    (Sigma fun (k_full_off : Nat) =>
    (Sigma fun (k_full_len : Nat) =>
    (Sigma fun (k_content_off : Nat) =>
    (Sigma fun (k_content_len : Nat) =>
    (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))
  | 0x7 =>
    ((GetElem?.getElem! children 7) : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) =>
    (Sigma fun (k_full_off : Nat) =>
    (Sigma fun (k_full_len : Nat) =>
    (Sigma fun (k_content_off : Nat) =>
    (Sigma fun (k_content_len : Nat) =>
    (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))
  | 0x8 =>
    ((GetElem?.getElem! children 8) : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) =>
    (Sigma fun (k_full_off : Nat) =>
    (Sigma fun (k_full_len : Nat) =>
    (Sigma fun (k_content_off : Nat) =>
    (Sigma fun (k_content_len : Nat) =>
    (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))
  | 0x9 =>
    ((GetElem?.getElem! children 9) : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) =>
    (Sigma fun (k_full_off : Nat) =>
    (Sigma fun (k_full_len : Nat) =>
    (Sigma fun (k_content_off : Nat) =>
    (Sigma fun (k_content_len : Nat) =>
    (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))
  | 0xA =>
    ((GetElem?.getElem! children 10) : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) =>
    (Sigma fun (k_full_off : Nat) =>
    (Sigma fun (k_full_len : Nat) =>
    (Sigma fun (k_content_off : Nat) =>
    (Sigma fun (k_content_len : Nat) =>
    (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))
  | 0xB =>
    ((GetElem?.getElem! children 11) : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) =>
    (Sigma fun (k_full_off : Nat) =>
    (Sigma fun (k_full_len : Nat) =>
    (Sigma fun (k_content_off : Nat) =>
    (Sigma fun (k_content_len : Nat) =>
    (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))
  | 0xC =>
    ((GetElem?.getElem! children 12) : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) =>
    (Sigma fun (k_full_off : Nat) =>
    (Sigma fun (k_full_len : Nat) =>
    (Sigma fun (k_content_off : Nat) =>
    (Sigma fun (k_content_len : Nat) =>
    (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))
  | 0xD =>
    ((GetElem?.getElem! children 13) : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) =>
    (Sigma fun (k_full_off : Nat) =>
    (Sigma fun (k_full_len : Nat) =>
    (Sigma fun (k_content_off : Nat) =>
    (Sigma fun (k_content_len : Nat) =>
    (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))
  | 0xE =>
    ((GetElem?.getElem! children 14) : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) =>
    (Sigma fun (k_full_off : Nat) =>
    (Sigma fun (k_full_len : Nat) =>
    (Sigma fun (k_content_off : Nat) =>
    (Sigma fun (k_content_len : Nat) =>
    (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))
  | _ =>
    ((GetElem?.getElem! children 15) : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) =>
    (Sigma fun (k_full_off : Nat) =>
    (Sigma fun (k_full_len : Nat) =>
    (Sigma fun (k_content_off : Nat) =>
    (Sigma fun (k_content_len : Nat) =>
    (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))

/-- The empty path. -/
def path_empty (_ : Unit) : TriePath :=
  (path_new ZERO_HASH ⟨0⟩)

def HEX_PREFIX_MAX_LENGTH : Nat := 33

/-- Decodes a compact path directly from its RLP source span, returning
the leaf flag and the path. -/
/- Type quantifiers: k_ex408174_ : Nat, k_ex408173_ : Nat, k_ex408172_ : Nat, k_ex408171_ : Nat, k_ex408170_
  : Nat, k_ex408169_ : Nat, 0 ≤ k_ex408169_ ∧ 0 ≤ k_ex408170_ ∧
  0 ≤ k_ex408171_ ∧
  0 ≤ k_ex408172_ ∧
  (k_ex408171_ + k_ex408172_) ≤ k_ex408170_ ∧
  0 ≤ k_ex408173_ ∧ 0 ≤ k_ex408174_ ∧ (k_ex408173_ + k_ex408174_) ≤ k_ex408170_ -/
def hex_prefix_decode_ref (f : RlpFieldRef) : SailM (Bool × TriePath) := do
  let f := ((((((f).2).2).2).2).2).2
  if (f.is_list : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (pure ())
  let n := f.content_len
  if ((n == 0) : Bool)
  then (pure (false, (path_empty ())))
  else
    (do
      let maximum_length := HEX_PREFIX_MAX_LENGTH
      if ((maximum_length <b n) : Bool)
      then sailThrow ((InvalidBlock RlpDecode))
      else
        (do
          let content := (rlp_ref_content f)
          let fb ← do (slice_byte ⟨_, ⟨_, content⟩⟩ 0)
          let flag : (BitVec 4) := (Sail.BitVec.extractLsb fb 7 4)
          let is_leaf : Bool := ((BitVec.access flag 1) == 1#1)
          let odd : Bool := ((BitVec.access flag 0) == 1#1)
          let tail_length : Nat := (n - 1)
          let tail := (slice_suffix content 1)
          let packed ← do
            (do
                let publicResult ← (slice_load ⟨_, ⟨_, tail⟩⟩ 0)
                pure ((publicResult).value))
          let paired_nibbles : Nat := (tail_length *i 2)
          if (odd : Bool)
          then
            (do
              if ((paired_nibbles <b 64) : Bool)
              then
                (let bytes := (word_to_hash ⟨((word_shift_right ⟨packed⟩ ⟨4⟩)).value⟩)
                let bytes : (Vector (BitVec 8) 32) :=
                  (vectorUpdate bytes 31
                    ((Sail.BitVec.extractLsb fb 3 0) +++ (Sail.BitVec.extractLsb
                        (GetElem?.getElem! bytes 31) 3 0)))
                (pure (is_leaf, (path_new (B256 bytes) ⟨(paired_nibbles + 1)⟩))))
              else sailThrow ((InvalidBlock WitnessDeficient)))
          else (pure (is_leaf, (path_new (word_to_hash ⟨packed⟩) ⟨paired_nibbles⟩)))))

/-- The path length in nibbles. -/
def path_len (path : TriePath) : trie_path_len :=
  ⟨(path.len).value⟩

/-- Decodes node bytes into leaf/extension/branch form by field count
(2 = leaf or extension by the HP flag; 17 = branch). -/
/- Type quantifiers: k_ex408178_ : Nat, k_ex408177_ : Nat, 0 ≤ k_ex408177_ ∧ 0 ≤ k_ex408178_ -/
def decode_trie_node (node : EvmByteSlice) : SailM TrieNode := do
  let node := ((node).2).2
  let ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩ ← do (rlp_node_cursor ⟨_, ⟨_, node⟩⟩)
  if (((! fields.valid) || (rlp_cursor_empty ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩)) : Bool)
  then (pure (InvalidNode ()))
  else
    (do
      let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (first, fields)⟩⟩⟩⟩ ← do
        (rlp_cursor_pop fields)
      if ((rlp_cursor_empty ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩) : Bool)
      then (pure (InvalidNode ()))
      else
        (do
          let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (second, fields)⟩⟩⟩⟩ ← do
            (rlp_cursor_pop fields)
          if ((rlp_cursor_empty ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩) : Bool)
          then
            (do
              let path_field := first
              let value := second
              let (is_leaf, path) ← do
                (hex_prefix_decode_ref
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, path_field⟩⟩⟩⟩⟩⟩)
              if (is_leaf : Bool)
              then
                (pure (LeafNode
                    { path := path,
                      value := ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, value⟩⟩⟩⟩⟩⟩ }))
              else
                (if ((((path_len path)).value == 0) : Bool)
                then (pure (InvalidNode ()))
                else
                  (pure (ExtensionNode
                      { path := path,
                        child := ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, value⟩⟩⟩⟩⟩⟩ }))))
          else
            (do
              let children : (Vector (Sigma fun (k_source_off : Nat) =>
                  (Sigma fun (k_source_len : Nat) =>
                  (Sigma fun (k_full_off : Nat) =>
                  (Sigma fun (k_full_len : Nat) =>
                  (Sigma fun (k_content_off : Nat) =>
                  (Sigma fun (k_content_len : Nat) =>
                  (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))) 16) :=
                (vectorInit ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, first⟩⟩⟩⟩⟩⟩)
              let children : (Vector (Sigma fun (k_source_off : Nat) =>
                  (Sigma fun (k_source_len : Nat) =>
                  (Sigma fun (k_full_off : Nat) =>
                  (Sigma fun (k_full_len : Nat) =>
                  (Sigma fun (k_content_off : Nat) =>
                  (Sigma fun (k_content_len : Nat) =>
                  (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))) 16) :=
                (vectorUpdate children 0 ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, first⟩⟩⟩⟩⟩⟩)
              let children : (Vector (Sigma fun (k_source_off : Nat) =>
                  (Sigma fun (k_source_len : Nat) =>
                  (Sigma fun (k_full_off : Nat) =>
                  (Sigma fun (k_full_len : Nat) =>
                  (Sigma fun (k_content_off : Nat) =>
                  (Sigma fun (k_content_len : Nat) =>
                  (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))) 16) :=
                (vectorUpdate children 1
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, second⟩⟩⟩⟩⟩⟩)
              let cursor : (Sigma fun (k_source_off : Nat) =>
                (Sigma fun (k_source_len : Nat) =>
                (Sigma fun (k_current : Nat) =>
                (Sigma fun (k_stop : Nat) =>
                (RlpCursorFields k_source_off k_source_len k_current k_stop))))) :=
                ((⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩ : (Sigma fun (k_source_off : Nat) =>
                (Sigma fun (k_source_len : Nat) =>
                (Sigma fun (k_current : Nat) =>
                (Sigma fun (k_stop : Nat) =>
                (RlpCursorFields k_source_off k_source_len k_current k_stop)))))) : (Sigma fun
                (k_source_off : Nat) =>
                (Sigma fun (k_source_len : Nat) =>
                (Sigma fun (k_current : Nat) =>
                (Sigma fun (k_stop : Nat) =>
                (RlpCursorFields k_source_off k_source_len k_current k_stop))))))
              let complete : Bool := true
              let (children, complete, cursor) ← (( do
                let loop_index_lower := 2
                let loop_index_upper := 15
                let mut loop_vars := (children, complete, cursor)
                for index in [loop_index_lower:loop_index_upper:1]i do
                  let (children, complete, cursor) := loop_vars
                  loop_vars ← do
                    let (children, complete, cursor) ← (( do
                      if ((rlp_cursor_empty cursor) : Bool)
                      then
                        (let complete : Bool := false
                        (pure ((children, complete, cursor) : ((Vector (Sigma fun
                            (k_source_off : Nat) =>
                            (Sigma fun (k_source_len : Nat) =>
                            (Sigma fun (k_full_off : Nat) =>
                            (Sigma fun (k_full_len : Nat) =>
                            (Sigma fun (k_content_off : Nat) =>
                            (Sigma fun (k_content_len : Nat) =>
                            (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))) 16) × Bool × (Sigma
                          fun (k_source_off : Nat) =>
                          (Sigma fun (k_source_len : Nat) =>
                          (Sigma fun (k_current : Nat) =>
                          (Sigma fun (k_stop : Nat) =>
                          (RlpCursorFields k_source_off k_source_len k_current k_stop)))))))))
                      else
                        (do
                          let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (child, next)⟩⟩⟩⟩ ← do
                            (rlp_cursor_pop ((((cursor).2).2).2).2)
                          let children : (Vector (Sigma fun (k_source_off : Nat) =>
                              (Sigma fun (k_source_len : Nat) =>
                              (Sigma fun (k_full_off : Nat) =>
                              (Sigma fun (k_full_len : Nat) =>
                              (Sigma fun (k_content_off : Nat) =>
                              (Sigma fun (k_content_len : Nat) =>
                              (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))) 16) :=
                            (vectorUpdate children index
                              ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, child⟩⟩⟩⟩⟩⟩)
                          let cursor : (Sigma fun (k_source_off : Nat) =>
                            (Sigma fun (k_source_len : Nat) =>
                            (Sigma fun (k_current : Nat) =>
                            (Sigma fun (k_stop : Nat) =>
                            (RlpCursorFields k_source_off k_source_len k_current k_stop))))) :=
                            ((⟨_, ⟨_, ⟨_, ⟨_, next⟩⟩⟩⟩ : (Sigma fun
                            (k_source_off : Nat) =>
                            (Sigma fun (k_source_len : Nat) =>
                            (Sigma fun (k_current : Nat) =>
                            (Sigma fun (k_stop : Nat) =>
                            (RlpCursorFields k_source_off k_source_len k_current k_stop)))))) : (Sigma
                            fun (k_source_off : Nat) =>
                            (Sigma fun (k_source_len : Nat) =>
                            (Sigma fun (k_current : Nat) =>
                            (Sigma fun (k_stop : Nat) =>
                            (RlpCursorFields k_source_off k_source_len k_current k_stop))))))
                          (pure ((children, complete, cursor) : ((Vector (Sigma fun
                              (k_source_off : Nat) =>
                              (Sigma fun (k_source_len : Nat) =>
                              (Sigma fun (k_full_off : Nat) =>
                              (Sigma fun (k_full_len : Nat) =>
                              (Sigma fun (k_content_off : Nat) =>
                              (Sigma fun (k_content_len : Nat) =>
                              (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))) 16) × Bool × (Sigma
                            fun (k_source_off : Nat) =>
                            (Sigma fun (k_source_len : Nat) =>
                            (Sigma fun (k_current : Nat) =>
                            (Sigma fun (k_stop : Nat) =>
                            (RlpCursorFields k_source_off k_source_len k_current k_stop))))))))) ) :
                      SailM
                      ((Vector (Sigma fun (k_source_off : Nat) =>
                        (Sigma fun (k_source_len : Nat) =>
                        (Sigma fun (k_full_off : Nat) =>
                        (Sigma fun (k_full_len : Nat) =>
                        (Sigma fun (k_content_off : Nat) =>
                        (Sigma fun (k_content_len : Nat) =>
                        (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))) 16) × Bool × (Sigma
                      fun (k_source_off : Nat) =>
                      (Sigma fun (k_source_len : Nat) =>
                      (Sigma fun (k_current : Nat) =>
                      (Sigma fun (k_stop : Nat) =>
                      (RlpCursorFields k_source_off k_source_len k_current k_stop)))))) )
                    (pure (children, complete, cursor))
                (pure loop_vars) ) : SailM
                ((Vector (Sigma fun (k_source_off : Nat) =>
                  (Sigma fun (k_source_len : Nat) =>
                  (Sigma fun (k_full_off : Nat) =>
                  (Sigma fun (k_full_len : Nat) =>
                  (Sigma fun (k_content_off : Nat) =>
                  (Sigma fun (k_content_len : Nat) =>
                  (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))) 16) × Bool × (Sigma
                fun (k_source_off : Nat) =>
                (Sigma fun (k_source_len : Nat) =>
                (Sigma fun (k_current : Nat) =>
                (Sigma fun (k_stop : Nat) =>
                (RlpCursorFields k_source_off k_source_len k_current k_stop)))))) )
              if (((! complete) || (rlp_cursor_empty cursor)) : Bool)
              then (pure (InvalidNode ()))
              else
                (do
                  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (value, cursor)⟩⟩⟩⟩ ← do
                    (rlp_cursor_pop ((((cursor).2).2).2).2)
                  if ((rlp_cursor_empty ⟨_, ⟨_, ⟨_, ⟨_, cursor⟩⟩⟩⟩) : Bool)
                  then
                    (pure (BranchNode
                        { children := children,
                          value := ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, value⟩⟩⟩⟩⟩⟩ }))
                  else (pure (InvalidNode ()))))))

def MPT_HASH_LENGTH : Nat := WORD_BYTE_LENGTH

/-- Copies a sub-32-byte node encoding into an inline node value. -/
/- Type quantifiers: k_ex408182_ : Nat, k_ex408181_ : Nat, 0 ≤ k_ex408181_ ∧ 0 ≤ k_ex408182_ -/
def inline_node_from_slice (bytes : EvmByteSlice) : SailM InlineNode := do
  let bytes := ((bytes).2).2
  let length := bytes.len
  if ((MPT_HASH_LENGTH ≤b length) : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else (pure ())
  (pure { data := ← (pure (word_to_hash ⟨((← (slice_load ⟨_, ⟨_, bytes⟩⟩ 0))).value⟩)),
          len := length })

/-- The reference denoted by a child field: an inline list, a 32-byte
hash, or empty. -/
/- Type quantifiers: k_ex408194_ : Nat, k_ex408193_ : Nat, k_ex408192_ : Nat, k_ex408191_ : Nat, k_ex408190_
  : Nat, k_ex408189_ : Nat, 0 ≤ k_ex408189_ ∧ 0 ≤ k_ex408190_ ∧
  0 ≤ k_ex408191_ ∧
  0 ≤ k_ex408192_ ∧
  (k_ex408191_ + k_ex408192_) ≤ k_ex408190_ ∧
  0 ≤ k_ex408193_ ∧ 0 ≤ k_ex408194_ ∧ (k_ex408193_ + k_ex408194_) ≤ k_ex408190_ -/
def field_to_ref (f : RlpFieldRef) : SailM NodeRef := do
  let f := ((((((f).2).2).2).2).2).2
  if (f.is_list : Bool)
  then
    (pure (InlineRef
        (← (inline_node_from_slice
            (rlp_ref_full ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, f⟩⟩⟩⟩⟩⟩)))))
  else
    (do
      if ((f.content_len == MPT_HASH_LENGTH) : Bool)
      then
        (pure (HashRef
            (word_to_hash
              ⟨((← (rlp_ref_word ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, f⟩⟩⟩⟩⟩⟩))).value⟩)))
      else (pure (EmptyRef ())))

/-- Maps a nibble cursor to the corresponding byte in decreasing vector order. -/
/- Type quantifiers: i : Nat, 0 ≤ i ∧ i ≤ 64 -/
def path_byte_index (i : trie_path_cursor) : SailM b256_index := do
  let i := (i).value
  let publicResult ← do
    let quotient := (Nat.div i 2)
    let natural_index ← (( do
      if (((0 ≤b quotient) && (quotient ≤b 31)) : Bool)
      then (pure quotient)
      else
        (do
          assert false "sail/lib/mpt/primitives.sail:64.24-64.25"
          throw Error.Exit) ) : SailM Nat )
    (pure (31 - natural_index))
  pure (⟨publicResult⟩)

/-- The `i`-th nibble, most significant first; out of range yields `0`. -/
/- Type quantifiers: k_ex408196_ : Nat, 0 ≤ k_ex408196_ ∧ k_ex408196_ ≤ 64 -/
def path_nibble (path : TriePath) (i : trie_path_cursor) : SailM nibble := do
  let i := (i).value
  if ((((path_len path)).value ≤b i) : Bool)
  then (pure 0x0#4)
  else
    (do
      let bytes := path.data
      let byte_index ← do
        (do
            let publicResult ← (path_byte_index ⟨i⟩)
            pure ((publicResult).value))
      if (((Nat.mod i 2) == 0) : Bool)
      then (pure (Sail.BitVec.extractLsb (GetElem?.getElem! bytes byte_index) 7 4))
      else (pure (Sail.BitVec.extractLsb (GetElem?.getElem! bytes byte_index) 3 0)))

/-- Increments a path length, rejecting a value already at the key bound. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def trie_path_len_increment (value : trie_path_len) : SailM trie_path_len := do
  let value := (value).value
  let publicResult ← do
    if ((value <b 64) : Bool)
    then (pure (value + 1))
    else sailThrow ((InvalidBlock WitnessDeficient))
  pure (⟨publicResult⟩)

/-- Whether `seg` occurs in `key` at nibble position `pos`. -/
/- Type quantifiers: k_ex408198_ : Nat, 0 ≤ k_ex408198_ ∧ k_ex408198_ ≤ 64 -/
def path_matches (key : TriePath) (pos : trie_path_cursor) (seg : TriePath) : SailM Bool := do
  let pos := (pos).value
  let stop := (pos + ((path_len seg)).value)
  if ((((path_len key)).value <b stop) : Bool)
  then (pure false)
  else
    (do
      let ok : Bool := true
      let offset : Nat := 0
      let (offset, ok) ← (( do
        let loop__step_lower := 0
        let loop__step_upper := 63
        let mut loop_vars := (offset, ok)
        for _step in [loop__step_lower:loop__step_upper:1]i do
          let (offset, ok) := loop_vars
          loop_vars ← do
            let (offset, ok) ← (( do
              if ((offset <b ((path_len seg)).value) : Bool)
              then
                (do
                  let key_index := (pos + offset)
                  let ok ← (( do
                    if ((key_index ≤b 64) : Bool)
                    then
                      (do
                        if (((← (path_nibble key ⟨key_index⟩)) != (← (path_nibble seg
                                 ⟨offset⟩))) : Bool)
                        then
                          (let ok : Bool := false
                          (pure ok))
                        else (pure ok))
                    else
                      (let ok : Bool := false
                      (pure ok)) ) : SailM Bool )
                  let offset ←
                    (do
                        let publicResult ← (trie_path_len_increment ⟨offset⟩)
                        pure ((publicResult).value))
                  (pure (offset, ok)))
              else (pure (offset, ok)) ) : SailM (Nat × Bool) )
            (pure (offset, ok))
        (pure loop_vars) ) : SailM (Nat × Bool) )
      (pure ok))

/-- Materializes an inline node in scratch memory as a byte slice. -/
def inline_node_slice (node : InlineNode) : SailM EvmByteSlice := do
  let start ← do (scratch_begin ())
  (scratch_push_b256 node.data node.len)
  (scratch_finish start)

/-- Resolves a reference to node bytes. Resolving a missing hash is a
deficient witness (`InvalidBlock(WitnessDeficient)`), never an empty
subtree. -/
def resolve_ref (r : NodeRef) : SailM EvmByteSlice := do
  match r with
  | .EmptyRef () =>
    (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  | .InlineRef node =>
    (do
      (inline_node_slice node))
  | .HashRef h =>
    (do
      let ⟨_, ⟨_, node⟩⟩ ← do (node_db_lookup h)
      if ((node.len == 0) : Bool)
      then
        (do
          sailThrow ((InvalidBlock WitnessDeficient)))
      else
        (pure ((⟨_, ⟨_, node⟩⟩ : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))))

/-- Walks the trie toward `key` from `pos`, returning the leaf value
without copying it; absent paths yield empty bytes. -/
/- Type quantifiers: _reclimit : Nat, k_ex408203_ : Nat, k_ex408202_ : Nat, k_ex408201_ : Nat, 0 ≤
  k_ex408201_ ∧ 0 ≤ k_ex408202_, 0 ≤ k_ex408203_ ∧ k_ex408203_ ≤ 64, 0 ≤ _reclimit -/
def _rec_trie_walk (node : EvmByteSlice) (key : TriePath) (pos : trie_path_cursor) (_reclimit : Nat) : SailM EvmByteSlice := do
  let node := ((node).2).2
  let pos := (pos).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((node.len == 0) : Bool)
      then
        (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
      else
        (do
          match (← (decode_trie_node ⟨_, ⟨_, node⟩⟩)) with
          | .LeafNode leaf =>
            (do
              if ((! (← (path_matches key ⟨pos⟩ leaf.path))) : Bool)
              then
                (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
                  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
                  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
              else
                (if (((pos + ((path_len leaf.path)).value) == ((path_len key)).value) : Bool)
                then
                  (pure ((⟨_, ⟨_, ((((⟨_, ⟨_, ⟨_, (rlp_ref_content
                      ((((((leaf.value).2).2).2).2).2).2)⟩⟩⟩ : (Sigma fun (k_ex418238_ : Nat)
                      =>
                      (Sigma fun (k_ex418242_ : Nat) =>
                      (Sigma fun (k_ex418243_ : Nat) =>
                      (EvmByteSliceFields (k_ex418238_ + k_ex418242_) k_ex418243_)))))).2).2).2⟩⟩ : (Sigma
                    fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma
                    fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
                else
                  (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
                    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
                    (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))))
          | .ExtensionNode ext =>
            (do
              let extension_len : Nat := ((path_len ext.path)).value
              if ((extension_len == 0) : Bool)
              then
                (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
                  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
                  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
              else
                (do
                  if ((! (← (path_matches key ⟨pos⟩ ext.path))) : Bool)
                  then
                    (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
                      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
                      (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
                  else
                    (do
                      let next_pos := (pos + extension_len)
                      if ((next_pos ≤b 64) : Bool)
                      then
                        (do
                          (_rec_trie_walk (← (resolve_ref (← (field_to_ref ext.child)))) key
                            ⟨next_pos⟩ _reclimit_pred))
                      else
                        (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
                          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
                          (k_off : Nat) =>
                          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))))))
          | .BranchNode branch =>
            (do
              if ((pos == ((path_len key)).value) : Bool)
              then
                (pure ((⟨_, ⟨_, ((((⟨_, ⟨_, ⟨_, (rlp_ref_content
                    ((((((branch.value).2).2).2).2).2).2)⟩⟩⟩ : (Sigma fun (k_ex418372_ : Nat)
                    =>
                    (Sigma fun (k_ex418376_ : Nat) =>
                    (Sigma fun (k_ex418377_ : Nat) =>
                    (EvmByteSliceFields (k_ex418372_ + k_ex418376_) k_ex418377_)))))).2).2).2⟩⟩ : (Sigma
                  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma
                  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
              else
                (do
                  if ((pos <b ((path_len key)).value) : Bool)
                  then
                    (do
                      let ⟨_, ⟨_, child⟩⟩ ← do
                        (resolve_ref
                          (← (field_to_ref
                              (branch_children_get branch.children (← (path_nibble key ⟨pos⟩))))))
                      (_rec_trie_walk ⟨_, ⟨_, child⟩⟩ key ⟨(pos + 1)⟩ _reclimit_pred))
                  else
                    (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
                      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
                      (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))))
          | .InvalidNode () =>
            (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
              (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
              (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Walks the trie toward `key` from `pos`, returning the leaf value
without copying it; absent paths yield empty bytes. -/
/- Type quantifiers: pos : Nat, k_ex408208_ : Nat, k_ex408207_ : Nat, 0 ≤ k_ex408207_ ∧
  0 ≤ k_ex408208_, 0 ≤ pos ∧ pos ≤ 64 -/
def trie_walk (node : EvmByteSlice) (key : TriePath) (pos : trie_path_cursor) : SailM EvmByteSlice := do
  let node := ((node).2).2
  let pos := (pos).value
  let _measure := ((64 - pos) : Int)
  if ((_measure <b 0) : Bool)
  then
    (do
      throw Error.Exit)
  else
    (do
      (_rec_trie_walk ⟨_, ⟨_, node⟩⟩ key ⟨pos⟩ (_measure + 1)))

/-- Looks up `key` from a root hash; the root node itself must be
witnessed. -/
def trie_lookup (root : hash) (key : TriePath) : SailM EvmByteSlice := do
  if ((root == EMPTY_TRIE_ROOT) : Bool)
  then
    (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  else
    (do
      let ⟨_, ⟨_, node⟩⟩ ← do (node_db_lookup root)
      if ((node.len == 0) : Bool)
      then
        (do
          sailThrow ((InvalidBlock WitnessDeficient)))
      else
        (do
          (trie_walk ⟨_, ⟨_, node⟩⟩ key ⟨0⟩)))

/-- The witnessed account at address `a` under state root `root`,
reading the secure trie at `keccak256(a)`; `None` when the walk
proves absence. -/
def stateless_account (root : hash) (a : address) : SailM (Option AccountInfo) := do
  let ⟨_, ⟨_, value⟩⟩ ← do (trie_lookup root (path_from_hash (← (keccak256_address a))))
  if ((value.len == 0) : Bool)
  then (pure none)
  else (pure (some (← (decode_state_account ⟨_, ⟨_, value⟩⟩))))

/-- The account at `a`: transaction overlay, then block overlay, then the
authenticated witness (cached block-level on first read). Every load
is recorded for the EIP-7928 block access list. -/
def k_aload (a : address) : SailM Account := SailME.run do
  (bal_account_touch a)
  match (← (acct_tx_get a)) with
  | .some acc => SailME.throw (acc : Account)
  | none => (pure ())
  match (← (acct_block_get a)) with
  | .some acc => SailME.throw (acc : Account)
  | none => (pure ())
  let acc ← (( do
    match (← (stateless_account (← readReg k_parent_state_root) a)) with
    | .some info => (pure (account_from_info info))
    | none => (pure EMPTY_ACCOUNT) ) : SailME Account Account )
  (acct_block_cache a acc)
  (pure acc)

/-- The witnessed storage value of `slot` under a storage root, reading
the secure trie at `keccak256(slot)`; absent slots are zero. -/
/- Type quantifiers: k_ex408211_ : Nat, 0 ≤ k_ex408211_ ∧ k_ex408211_ ≤ (2 ^ 256 - 1) -/
def stateless_storage (root : hash) (slot : word) : SailM word := do
  let slot := (slot).value
  let publicResult ← do
    let ⟨_, ⟨_, value⟩⟩ ← do
      (trie_lookup root (path_from_hash (← (keccak256_word ⟨slot⟩))))
    if ((value.len == 0) : Bool)
    then (pure (ZERO_WORD).value)
    else
      (do
          let publicResult ← (rlp_ref_uint_word (← (rlp_single_ref ⟨_, ⟨_, value⟩⟩)))
          pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Resolves a slot to its live [StorageValue][type-StorageValue]: `curr`
is the value `SLOAD` pushes; `orig` is the EIP-2200 transaction-start
value the `SSTORE` gas policy compares against. The guarded
`SLOAD`/`SSTORE` opcode paths are the only callers, so reaching this
function is the semantic EIP-7928 storage-access boundary: reads are
recorded independently of the cache layer that supplies the value and
survive frame rollback (the BAL encoder removes slots that also have a
storage change). [stateless_storage][] is the base primitive — an
authenticated MPT point-get, one walk for both the witness and the
harness-built alloc trie; everything above it (the overlay, the
read-through, the journal) is common. -/
/- Type quantifiers: k_ex408212_ : Nat, 0 ≤ k_ex408212_ ∧ k_ex408212_ ≤ (2 ^ 256 - 1) -/
def k_sload (a : address) (s : word) : SailM StorageValue := SailME.run do
  let s := (s).value
  (bal_storage_read a ⟨s⟩)
  let key := (storage_key a ⟨s⟩)
  match (← (storage_tx_get key)) with
  | .some e => SailME.throw (e : StorageValue)
  | none => (pure ())
  let acc ← do (k_aload a)
  match (← (storage_block_get key)) with
  | .some e =>
    SailME.throw ({ curr := ⟨(e.curr).value⟩,
                    orig := ⟨(e.curr).value⟩ } : StorageValue)
  | none => (pure ())
  let v ← do
    if (acc.storage_cleared : Bool)
    then (pure (ZERO_WORD).value)
    else
      (do
          let publicResult ← (stateless_storage acc.info.storage_root ⟨s⟩)
          pure ((publicResult).value))
  (storage_block_cache key ⟨v⟩)
  (pure { curr := ⟨v⟩,
          orig := ⟨v⟩ })

/-- `SSTORE`: creates or updates the live transaction row. The preceding
[k_sload][] supplies the transaction-original value; the host keeps
clear generations and frame undo history private. -/
/- Type quantifiers: k_ex408213_ : Nat, 0 ≤ k_ex408213_ ∧ k_ex408213_ ≤ (2 ^ 256 - 1) -/
def k_sstore (a : address) (s : word) (v : StorageValue) : SailM Unit := do
  let s := (s).value
  (storage_tx_update
    { key := (storage_key a ⟨s⟩),
      value := v })

/-- `TLOAD` (EIP-1153): reads per-transaction transient storage, which is
discarded at transaction end and is not part of the state trie. -/
/- Type quantifiers: k_ex408214_ : Nat, 0 ≤ k_ex408214_ ∧ k_ex408214_ ≤ (2 ^ 256 - 1) -/
def k_tload (a : address) (s : word) : SailM word := do
  let s := (s).value
  let publicResult ← do
    (do
        let publicResult ← (transient_load a ⟨s⟩)
        pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- `TSTORE` (EIP-1153): writes transient storage. Frame rollback is part
of the host's semantic checkpoint contract. -/
/- Type quantifiers: k_ex408216_ : Nat, k_ex408215_ : Nat, 0 ≤ k_ex408215_ ∧
  k_ex408215_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex408216_ ∧ k_ex408216_ ≤ (2 ^ 256 - 1) -/
def k_tstore (a : address) (s : word) (v : word) : SailM Unit := do
  let s := (s).value
  let v := (v).value
  (transient_store a ⟨s⟩ ⟨v⟩)

