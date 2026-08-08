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
open BalIterEntry

/-! # State: storage

Warm/cold accounting (EIP-2929), persistent storage (`SLOAD`/`SSTORE`),
and transient storage (EIP-1153), over the host state stores. -/

/- Type quantifiers: k_ex415073_ : Nat, 0 ≤ k_ex415073_ ∧ k_ex415073_ ≤ (2 ^ 256 - 1) -/
def storage_key (a : (Vector (BitVec 8) 20)) (s : Nat) : StorageKey :=
  { addr := a,
    slot := s }

/-- Marks an address warm and returns its prior warm bit (EIP-2929: the
EVM charges the cold cost on `false`, the warm cost on `true`). The
host includes a new warm entry in its semantic checkpoint. -/
def k_access_account (a : (Vector (BitVec 8) 20)) : SailM Bool := do
  (warm_addr_touch a)

/-- Marks a storage slot warm and returns its prior warm bit. New entries
belong to the host's semantic checkpoint. -/
/- Type quantifiers: k_ex415074_ : Nat, 0 ≤ k_ex415074_ ∧ k_ex415074_ ≤ (2 ^ 256 - 1) -/
def k_slot_is_warm (a : (Vector (BitVec 8) 20)) (s : Nat) : SailM Bool := do
  (warm_slot_touch a s)

/-- Decodes an account trie leaf — `rlp([nonce, balance, storage_root,
code_hash])` — into an [AccountInfo][type-AccountInfo]; empty
root/hash fields decode to their empty-sentinel digests. -/
/- Type quantifiers: value_dependentWitness1 : Nat, value_dependentWitness0 : Nat, 0 ≤
  value_dependentWitness0 ∧ 0 ≤ value_dependentWitness1 -/
def decode_state_account (value : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM AccountInfo := do
  let value_dependentWitness0 := (value).1
  let value_dependentWitness1 := ((value).2).1
  let value := ((value).2).2
  let ⟨nonce_ex424595_, ⟨nonce_ex424596_, ⟨nonce_content_len, ⟨nonce_full_len, (nonce, fields)⟩⟩⟩⟩ ← do
    (do
        let dependentArg0 := (← (rlp_node_cursor ⟨_, ⟨_, value⟩⟩))
        let publicResult ← (rlp_cursor_pop ((dependentArg0).2).2)
        pure ((⟨_, ⟨_, ⟨_, ⟨_, ((publicResult).2).2⟩⟩⟩⟩ : (Sigma fun
        (k_ex424595_ : Nat) =>
        (Sigma fun (k_ex424596_ : Nat) =>
        (Sigma fun (k_content_len : Nat) =>
        (Sigma fun (k_full_len : Nat) =>
        ((RlpFieldRef k_ex424595_ k_full_len k_content_len) × (EvmByteSliceFields (k_ex424595_ + k_full_len) (k_ex424596_ - k_full_len))))))))))
  let ⟨balance_content_len, ⟨balance_full_len, (balance, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨storage_content_len, ⟨storage_full_len, (storage, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨code_content_len, ⟨code_full_len, (code, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  (rlp_cursor_expect_end fields)
  let storage_root ← do
    if ((storage_content_len == 0) : Bool)
    then (pure EMPTY_TRIE_ROOT)
    else (pure (word_to_hash (← (rlp_ref_word storage))))
  let code_hash ← do
    if ((code_content_len == 0) : Bool)
    then (pure KECCAK_EMPTY)
    else (pure (word_to_hash (← (rlp_ref_word code))))
  (pure { nonce := ← (rlp_ref_uint64 nonce),
          balance := ← (rlp_ref_uint_word balance),
          storage_root := storage_root,
          code_hash := code_hash })

/-- Constructs a path from high-aligned data and a nibble length. -/
/- Type quantifiers: k_ex415081_ : Nat, 0 ≤ k_ex415081_ ∧ k_ex415081_ ≤ 64 -/
def path_new (data : (Vector (BitVec 8) 32)) (len : Nat) : TriePath :=
  { data := data,
    len := len }

/-- The 64-nibble path of a 32-byte hash — the secure-trie key form. -/
def path_from_hash (h : (Vector (BitVec 8) 32)) : TriePath :=
  (path_new h 64)

/-- The witness node bytes whose KECCAK-256 digest is `h`, retained as a
slice into the stateless input; empty if unwitnessed. -/
def node_db_lookup (h : (Vector (BitVec 8) 32)) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  (nodedb_lookup h)

/-- Selects a decoded branch child reference by nibble value. -/
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

def MPT_HASH_LENGTH : Nat := WORD_BYTE_LENGTH

/-- Copies a sub-32-byte node encoding into an inline node value. -/
/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, 0 ≤
  bytes_dependentWitness0 ∧ 0 ≤ bytes_dependentWitness1 -/
def inline_node_from_slice (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM InlineNode := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let length := bytes.len
  if ((MPT_HASH_LENGTH ≤b length) : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else
    (pure { data := ← (pure (word_to_hash (← (slice_load ⟨_, ⟨_, bytes⟩⟩ 0)))),
            len := length })

/-- The reference denoted by a child field: an inline list, a 32-byte
hash, or empty. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def field_to_ref (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM NodeRef := do
  if (f.is_list : Bool)
  then (pure (InlineRef (← (inline_node_from_slice ⟨_, ⟨_, f.source⟩⟩))))
  else
    (do
      if ((k_content_len == MPT_HASH_LENGTH) : Bool)
      then (pure (HashRef (word_to_hash (← (rlp_ref_word f)))))
      else (pure (EmptyRef ())))

/-- Decodes branch children 2 through 15, followed by the branch value. -/
/- Type quantifiers: _reclimit : Nat, k_ex415117_ : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 2
  ≤ k_ex415117_ ∧ k_ex415117_ ≤ 16, 0 ≤ _reclimit -/
def _rec_decode_branch_node (cursor : (EvmByteSliceFields k_source_off k_source_len)) (index : Nat) (children : (Vector NodeRef 16)) (_reclimit : Nat) : SailM TrieNode := do
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
          let ⟨child_content_len, ⟨child_full_len, (child, next)⟩⟩ ← do
            (rlp_cursor_pop cursor)
          let updated := children
          let updated ← (pure (vectorUpdate updated index (← (field_to_ref child))))
          (_rec_decode_branch_node next (index + 1) updated _reclimit_pred))
      else
        (do
          let ⟨value_content_len, ⟨value_full_len, (value, cursor)⟩⟩ ← do
            (rlp_cursor_pop cursor)
          (rlp_cursor_expect_end cursor)
          (pure (BranchNode (children, (rlp_ref_content value))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Decodes branch children 2 through 15, followed by the branch value. -/
/- Type quantifiers: index : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 2
  ≤ index ∧ index ≤ 16 -/
def decode_branch_node (cursor : (EvmByteSliceFields k_source_off k_source_len)) (index : Nat) (children : (Vector NodeRef 16)) : SailM TrieNode := do
  let _measure := ((16 - index) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_decode_branch_node cursor index children (_measure + 1))

/-- The empty path. -/
def path_empty (_ : Unit) : TriePath :=
  (path_new ZERO_HASH 0)

def HEX_PREFIX_MAX_LENGTH : Nat := 33

/-- Decodes a compact path directly from its RLP source span, returning
the leaf flag and the path. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def hex_prefix_decode_ref (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM (Bool × TriePath) := do
  if (f.is_list : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else
    (do
      let n := k_content_len
      if ((n == 0) : Bool)
      then (pure (false, (path_empty ())))
      else
        (do
          let maximum_length := HEX_PREFIX_MAX_LENGTH
          if ((maximum_length <b n) : Bool)
          then sailThrow ((InvalidBlock RlpDecode))
          else
            (do
              let content := (sub_slice f.source (k_source_len - n) n)
              let fb ← do (slice_byte ⟨_, ⟨_, content⟩⟩ 0)
              let flag : (BitVec 4) := (Sail.BitVec.extractLsb fb 7 4)
              let is_leaf : Bool := ((BitVec.access flag 1) == 1#1)
              let odd : Bool := ((BitVec.access flag 0) == 1#1)
              let tail_length : Nat := (n - 1)
              let tail := (slice_suffix content 1)
              let packed ← do (slice_load ⟨_, ⟨_, tail⟩⟩ 0)
              let paired_nibbles : Nat := (tail_length *i 2)
              if (odd : Bool)
              then
                (do
                  if ((paired_nibbles <b 64) : Bool)
                  then
                    (let bytes := (word_to_hash (word_shift_right packed 4))
                    let bytes : (Vector (BitVec 8) 32) :=
                      (vectorUpdate bytes 31
                        ((Sail.BitVec.extractLsb fb 3 0) +++ (Sail.BitVec.extractLsb
                            (GetElem?.getElem! bytes 31) 3 0)))
                    (pure (is_leaf, (path_new (B256 bytes) (paired_nibbles + 1)))))
                  else sailThrow ((InvalidBlock WitnessDeficient)))
              else (pure (is_leaf, (path_new (word_to_hash packed) paired_nibbles))))))

/-- The path length in nibbles. -/
def path_len (path : TriePath) : Nat :=
  path.len

/-- Decodes node bytes into leaf/extension/branch form by field count
(2 = leaf or extension by the HP flag; 17 = branch). -/
/- Type quantifiers: node_dependentWitness1 : Nat, node_dependentWitness0 : Nat, 0 ≤
  node_dependentWitness0 ∧ 0 ≤ node_dependentWitness1 -/
def decode_trie_node (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM TrieNode := do
  let node_dependentWitness0 := (node).1
  let node_dependentWitness1 := ((node).2).1
  let node := ((node).2).2
  let ⟨_, ⟨_, fields⟩⟩ ← do (rlp_node_cursor ⟨_, ⟨_, node⟩⟩)
  let ⟨first_content_len, ⟨first_full_len, (first, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨second_content_len, ⟨second_full_len, (second, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  if ((fields.len == 0) : Bool)
  then
    (do
      let (is_leaf, path) ← do (hex_prefix_decode_ref first)
      if (is_leaf : Bool)
      then (pure (LeafNode (path, (rlp_ref_content second))))
      else
        (do
          if (((path_len path) == 0) : Bool)
          then sailThrow ((InvalidBlock RlpDecode))
          else (pure (ExtensionNode (path, (← (field_to_ref second)))))))
  else
    (do
      let children : (Vector NodeRef 16) := (vectorInit (EmptyRef ()))
      let children ← (pure (vectorUpdate children 0 (← (field_to_ref first))))
      let children ← (pure (vectorUpdate children 1 (← (field_to_ref second))))
      (decode_branch_node fields 2 children))

/-- Maps a nibble cursor to the corresponding byte in decreasing vector order. -/
/- Type quantifiers: i : Nat, 0 ≤ i ∧ i ≤ 64 -/
def path_byte_index (i : Nat) : SailM Nat := do
  let quotient := (Nat.div i 2)
  let natural_index ← (( do
    if (((0 ≤b quotient) && (quotient ≤b 31)) : Bool)
    then (pure quotient)
    else
      (do
        assert false "sail/lib/mpt/primitives.sail:64.24-64.25"
        throw Error.Exit) ) : SailM Nat )
  (pure (31 - natural_index))

/-- The `i`-th nibble, most significant first; out of range yields `0`. -/
/- Type quantifiers: k_ex415162_ : Nat, 0 ≤ k_ex415162_ ∧ k_ex415162_ ≤ 64 -/
def path_nibble (path : TriePath) (i : Nat) : SailM (BitVec 4) := do
  if (((path_len path) ≤b i) : Bool)
  then (pure 0x0#4)
  else
    (do
      let bytes := path.data
      let byte_index ← do (path_byte_index i)
      if (((Nat.mod i 2) == 0) : Bool)
      then (pure (Sail.BitVec.extractLsb (GetElem?.getElem! bytes byte_index) 7 4))
      else (pure (Sail.BitVec.extractLsb (GetElem?.getElem! bytes byte_index) 3 0)))

/-- Increments a path length, rejecting a value already at the key bound. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def trie_path_len_increment (value : Nat) : SailM Nat := do
  if ((value <b 64) : Bool)
  then (pure (value + 1))
  else sailThrow ((InvalidBlock WitnessDeficient))

/-- Whether `seg` occurs in `key` at nibble position `pos`. -/
/- Type quantifiers: k_ex415164_ : Nat, 0 ≤ k_ex415164_ ∧ k_ex415164_ ≤ 64 -/
def path_matches (key : TriePath) (pos : Nat) (seg : TriePath) : SailM Bool := do
  let stop := (pos + (path_len seg))
  if (((path_len key) <b stop) : Bool)
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
              if ((offset <b (path_len seg)) : Bool)
              then
                (do
                  let key_index := (pos + offset)
                  let ok ← (( do
                    if ((key_index ≤b 64) : Bool)
                    then
                      (do
                        if (((← (path_nibble key key_index)) != (← (path_nibble seg offset))) : Bool)
                        then
                          (let ok : Bool := false
                          (pure ok))
                        else (pure ok))
                    else
                      (let ok : Bool := false
                      (pure ok)) ) : SailM Bool )
                  let offset ← (trie_path_len_increment offset)
                  (pure (offset, ok)))
              else (pure (offset, ok)) ) : SailM (Nat × Bool) )
            (pure (offset, ok))
        (pure loop_vars) ) : SailM (Nat × Bool) )
      (pure ok))

/-- Materializes an inline node in scratch memory as a byte slice. -/
def inline_node_slice (node : InlineNode) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  let start ← do (scratch_begin ())
  (scratch_push_b256 node.data node.len)
  (scratch_finish start)

/-- Resolves a reference to node bytes. Resolving a missing hash is a
deficient witness (`InvalidBlock(WitnessDeficient)`), never an empty
subtree. -/
def resolve_ref (r : NodeRef) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
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
      if _sailIf0 : ((node.len == 0) : Bool) = true
      then
        (do
          sailThrow ((InvalidBlock WitnessDeficient)))
      else
        (pure ((⟨_, ⟨_, node⟩⟩ : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))))

/-- Walks the trie toward `key` from `pos`, returning the leaf value
without copying it; absent paths yield empty bytes. -/
/- Type quantifiers: _reclimit : Nat, k_ex415177_ : Nat, node_dependentWitness1 : Nat, node_dependentWitness0
  : Nat, 0 ≤ node_dependentWitness0 ∧ 0 ≤ node_dependentWitness1, 0 ≤ k_ex415177_ ∧
  k_ex415177_ ≤ 64, 0 ≤ _reclimit -/
def _rec_trie_walk (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (key : TriePath) (pos : Nat) (_reclimit : Nat) : SailM (Sigma
  fun (node_dependentWitness0 : Nat) =>
  (Sigma fun (node_dependentWitness1 : Nat) =>
  (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1))) := do
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
      if _sailIf0 : ((node.len == 0) : Bool) = true
      then
        (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (node_dependentWitness0 : Nat) =>
          (Sigma fun (node_dependentWitness1 : Nat) =>
          (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1)))) : (Sigma fun
          (node_dependentWitness0 : Nat) =>
          (Sigma fun (node_dependentWitness1 : Nat) =>
          (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1)))))
      else
        (do
          match (← (decode_trie_node ⟨_, ⟨_, node⟩⟩)) with
          | .LeafNode (path, value) =>
            (do
              if _sailIf1 : ((! (← (path_matches key pos path))) : Bool) = true
              then
                (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (node_dependentWitness0 : Nat) =>
                  (Sigma fun (node_dependentWitness1 : Nat) =>
                  (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1)))) : (Sigma fun
                  (node_dependentWitness0 : Nat) =>
                  (Sigma fun (node_dependentWitness1 : Nat) =>
                  (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1)))))
              else
                (if _sailIf2 : (((pos + (path_len path)) == (path_len key)) : Bool) = true
                then
                  (pure (value : (Sigma fun (node_dependentWitness0 : Nat) =>
                    (Sigma fun (node_dependentWitness1 : Nat) =>
                    (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1)))))
                else
                  (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (node_dependentWitness0 : Nat)
                    =>
                    (Sigma fun (node_dependentWitness1 : Nat) =>
                    (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1)))) : (Sigma fun
                    (node_dependentWitness0 : Nat) =>
                    (Sigma fun (node_dependentWitness1 : Nat) =>
                    (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1)))))))
          | .ExtensionNode (path, childref) =>
            (do
              let extension_len : Nat := (path_len path)
              if _sailIf1 : ((extension_len == 0) : Bool) = true
              then
                (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (node_dependentWitness0 : Nat) =>
                  (Sigma fun (node_dependentWitness1 : Nat) =>
                  (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1)))) : (Sigma fun
                  (node_dependentWitness0 : Nat) =>
                  (Sigma fun (node_dependentWitness1 : Nat) =>
                  (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1)))))
              else
                (do
                  if _sailIf2 : ((! (← (path_matches key pos path))) : Bool) = true
                  then
                    (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun
                      (node_dependentWitness0 : Nat) =>
                      (Sigma fun (node_dependentWitness1 : Nat) =>
                      (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1)))) : (Sigma
                      fun (node_dependentWitness0 : Nat) =>
                      (Sigma fun (node_dependentWitness1 : Nat) =>
                      (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1)))))
                  else
                    (do
                      let next_pos := (pos + extension_len)
                      if _sailIf3 : ((next_pos ≤b 64) : Bool) = true
                      then
                        (do
                          (do
                              let dependentArg0 := (← (resolve_ref childref))
                              let publicResult ← (_rec_trie_walk dependentArg0 key next_pos
                              _reclimit_pred)
                              pure ((⟨_, ⟨_, ((publicResult).2).2⟩⟩ : (Sigma fun
                              (node_dependentWitness0 : Nat) =>
                              (Sigma fun (node_dependentWitness1 : Nat) =>
                              (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1)))))))
                      else
                        (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun
                          (node_dependentWitness0 : Nat) =>
                          (Sigma fun (node_dependentWitness1 : Nat) =>
                          (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1)))) : (Sigma
                          fun (node_dependentWitness0 : Nat) =>
                          (Sigma fun (node_dependentWitness1 : Nat) =>
                          (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1))))))))
          | .BranchNode (children, value) =>
            (do
              if _sailIf1 : ((pos == (path_len key)) : Bool) = true
              then
                (pure (value : (Sigma fun (node_dependentWitness0 : Nat) =>
                  (Sigma fun (node_dependentWitness1 : Nat) =>
                  (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1)))))
              else
                (do
                  if _sailIf2 : ((pos <b (path_len key)) : Bool) = true
                  then
                    (do
                      let ⟨_, ⟨_, child⟩⟩ ← do
                        (resolve_ref (branch_refs_get children (← (path_nibble key pos))))
                      (_rec_trie_walk ⟨_, ⟨_, child⟩⟩ key (pos + 1) _reclimit_pred))
                  else
                    (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun
                      (node_dependentWitness0 : Nat) =>
                      (Sigma fun (node_dependentWitness1 : Nat) =>
                      (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1)))) : (Sigma
                      fun (node_dependentWitness0 : Nat) =>
                      (Sigma fun (node_dependentWitness1 : Nat) =>
                      (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1)))))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Walks the trie toward `key` from `pos`, returning the leaf value
without copying it; absent paths yield empty bytes. -/
/- Type quantifiers: pos : Nat, node_dependentWitness1 : Nat, node_dependentWitness0 : Nat, 0 ≤
  node_dependentWitness0 ∧ 0 ≤ node_dependentWitness1, 0 ≤ pos ∧ pos ≤ 64 -/
def trie_walk (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (key : TriePath) (pos : Nat) : SailM (Sigma
  fun (node_dependentWitness0 : Nat) =>
  (Sigma fun (node_dependentWitness1 : Nat) =>
  (EvmByteSliceFields node_dependentWitness0 node_dependentWitness1))) := do
  let node_dependentWitness0 := (node).1
  let node_dependentWitness1 := ((node).2).1
  let node := ((node).2).2
  let _measure := ((64 - pos) : Int)
  if _sailIf0 : ((_measure <b 0) : Bool) = true
  then
    (do
      throw Error.Exit)
  else
    (do
      (_rec_trie_walk ⟨_, ⟨_, node⟩⟩ key pos (_measure + 1)))

/-- Looks up `key` from a root hash; the root node itself must be
witnessed. -/
def trie_lookup (root : (Vector (BitVec 8) 32)) (key : TriePath) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  if _sailIf0 : ((root == EMPTY_TRIE_ROOT) : Bool) = true
  then
    (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  else
    (do
      let ⟨_, ⟨_, node⟩⟩ ← do (node_db_lookup root)
      if _sailIf1 : ((node.len == 0) : Bool) = true
      then
        (do
          sailThrow ((InvalidBlock WitnessDeficient)))
      else
        (do
          (trie_walk ⟨_, ⟨_, node⟩⟩ key 0)))

/-- The witnessed account at address `a` under state root `root`,
reading the secure trie at `keccak256(a)`; `None` when the walk
proves absence. -/
def stateless_account (root : (Vector (BitVec 8) 32)) (a : (Vector (BitVec 8) 20)) : SailM (Option AccountInfo) := do
  let ⟨_, ⟨_, value⟩⟩ ← do (trie_lookup root (path_from_hash (← (keccak256_address a))))
  if ((value.len == 0) : Bool)
  then (pure none)
  else (pure (some (← (decode_state_account ⟨_, ⟨_, value⟩⟩))))

/-- The account at `a`: transaction overlay, then block overlay, then the
authenticated witness (cached block-level on first read). Every load
is recorded for the EIP-7928 block access list. -/
def k_aload (a : (Vector (BitVec 8) 20)) : SailM Account := SailME.run do
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
/- Type quantifiers: k_ex415191_ : Nat, 0 ≤ k_ex415191_ ∧ k_ex415191_ ≤ (2 ^ 256 - 1) -/
def stateless_storage (root : (Vector (BitVec 8) 32)) (slot : Nat) : SailM Nat := do
  let ⟨_, ⟨_, value⟩⟩ ← do (trie_lookup root (path_from_hash (← (keccak256_word slot))))
  if ((value.len == 0) : Bool)
  then (pure ZERO_WORD)
  else
    (do
        let dependentArg0 := (← (rlp_single_ref value))
        (rlp_ref_uint_word (dependentArg0).2))

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
/- Type quantifiers: k_ex415192_ : Nat, 0 ≤ k_ex415192_ ∧ k_ex415192_ ≤ (2 ^ 256 - 1) -/
def k_sload (a : (Vector (BitVec 8) 20)) (s : Nat) : SailM StorageValue := SailME.run do
  (bal_storage_read a s)
  let key := (storage_key a s)
  match (← (storage_tx_get key)) with
  | .some e => SailME.throw (e : StorageValue)
  | none => (pure ())
  let acc ← do (k_aload a)
  match (← (storage_block_get key)) with
  | .some e =>
    SailME.throw ({ curr := e.curr,
                    orig := e.curr } : StorageValue)
  | none => (pure ())
  let v ← do
    if (acc.storage_cleared : Bool)
    then (pure ZERO_WORD)
    else (stateless_storage acc.info.storage_root s)
  (storage_block_cache key v)
  (pure { curr := v,
          orig := v })

/-- `SSTORE`: creates or updates the live transaction row. The preceding
[k_sload][] supplies the transaction-original value; the host keeps
clear generations and frame undo history private. -/
/- Type quantifiers: k_ex415193_ : Nat, 0 ≤ k_ex415193_ ∧ k_ex415193_ ≤ (2 ^ 256 - 1) -/
def k_sstore (a : (Vector (BitVec 8) 20)) (s : Nat) (v : StorageValue) : SailM Unit := do
  (storage_tx_update
    { key := (storage_key a s),
      value := v })

/-- `TLOAD` (EIP-1153): reads per-transaction transient storage, which is
discarded at transaction end and is not part of the state trie. -/
/- Type quantifiers: k_ex415194_ : Nat, 0 ≤ k_ex415194_ ∧ k_ex415194_ ≤ (2 ^ 256 - 1) -/
def k_tload (a : (Vector (BitVec 8) 20)) (s : Nat) : SailM Nat := do
  (transient_load a s)

/-- `TSTORE` (EIP-1153): writes transient storage. Frame rollback is part
of the host's semantic checkpoint contract. -/
/- Type quantifiers: k_ex415196_ : Nat, k_ex415195_ : Nat, 0 ≤ k_ex415195_ ∧
  k_ex415195_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex415196_ ∧ k_ex415196_ ≤ (2 ^ 256 - 1) -/
def k_tstore (a : (Vector (BitVec 8) 20)) (s : Nat) (v : Nat) : SailM Unit := do
  (transient_store a s v)

