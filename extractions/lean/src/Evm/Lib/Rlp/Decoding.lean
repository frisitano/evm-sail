import Evm.Flow
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Host.RegionAccess
import Evm.Lib.Rlp.Encoding

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
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
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

/-! # RLP decoding

Canonical Recursive Length Prefix framing, cursor traversal, and typed scalar
decoding. Input- and scratch-backed cursors remain nominally distinct so a
decoder never pays for a runtime provenance tag. -/

/- Type quantifiers: _width : Nat, evm_prefix' : Nat, 1 ≤ _width ∧
  _width ≤ 8 ∧ 0 ≤ evm_prefix' ∧ evm_prefix' ≤ (2 ^ (8 * (_width - 1)) - 1) -/
def rlp_uint64_append (_width : Nat) (evm_prefix' : Nat) (next : (BitVec 8)) : Nat :=
  ((evm_prefix' *i 256) + (BitVec.toNatInt next))

/-- Decodes exactly `width` big-endian bytes into a bounded unsigned value. -/
/- Type quantifiers: _reclimit : Nat, content_dependentWitness1 : Nat, content_dependentWitness0 :
  Nat, width : Nat, 0 ≤ width ∧ width ≤ 8, 0 ≤ content_dependentWitness0 ∧
  0 ≤ content_dependentWitness1 ∧
  (content_dependentWitness0 + content_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ _reclimit -/
def _rec_rlp_uint64_width (content : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (width : Nat) (_reclimit : Nat) : SailM Nat := do
  let content_dependentWitness0 := (content).1
  let content_dependentWitness1 := ((content).2).1
  let content := ((content).2).2
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((width == 0) : Bool)
      then (pure 0)
      else
        (do
          let evm_prefix' ← do
            (_rec_rlp_uint64_width ⟨_, ⟨_, content⟩⟩ (width - 1) _reclimit_pred)
          let next ← do (stateless_input_slice_byte ⟨_, ⟨_, content⟩⟩ (width - 1))
          (pure (rlp_uint64_append width evm_prefix' next))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Decodes exactly `width` big-endian bytes into a bounded unsigned value. -/
/- Type quantifiers: content_dependentWitness1 : Nat, content_dependentWitness0 : Nat, width : Nat, 0
  ≤ width ∧ width ≤ 8, 0 ≤ content_dependentWitness0 ∧
  0 ≤ content_dependentWitness1 ∧
  (content_dependentWitness0 + content_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def rlp_uint64_width (content : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (width : Nat) : SailM Nat := do
  let content_dependentWitness0 := (content).1
  let content_dependentWitness1 := ((content).2).1
  let content := ((content).2).2
  let _measure := (width : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_rlp_uint64_width ⟨_, ⟨_, content⟩⟩ width (_measure + 1))

/-- Decodes the first RLP header in an exact remaining source slice:
`(is_list, content_offset, content_length)`. -/
/- Type quantifiers: b_dependentWitness1 : Nat, b_dependentWitness0 : Nat, 0 ≤ b_dependentWitness0
  ∧ 0 ≤ b_dependentWitness1 ∧ (b_dependentWitness0 + b_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def rlp_ref_hdr (b : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Bool × Nat × Nat) := do
  let b_dependentWitness0 := (b).1
  let b_dependentWitness1 := ((b).2).1
  let b := ((b).2).2
  let source_len := b.len
  if ((source_len == 0) : Bool)
  then (fatal_error RlpDecode)
  else (pure ())
  let first_byte ← do (stateless_input_slice_byte ⟨_, ⟨_, b⟩⟩ 0)
  let h := (BitVec.toNatInt first_byte)
  if ((h <b 128) : Bool)
  then (pure (false, 0, 1))
  else
    (do
      if ((h <b 184) : Bool)
      then (pure (false, 1, (h - 128)))
      else
        (do
          if ((h <b 192) : Bool)
          then
            (do
              let length_width : Nat := (h - 183)
              if ((length_width ≤b (source_len -i 1)) : Bool)
              then
                (do
                  let length_bytes := (stateless_input_sub_slice b 1 length_width)
                  (pure (false, (1 + length_width), (← (rlp_uint64_width
                        ⟨_, ⟨_, length_bytes⟩⟩ length_width)))))
              else (fatal_error RlpDecode))
          else
            (do
              if ((h <b 248) : Bool)
              then (pure (true, 1, (h - 192)))
              else
                (do
                  let length_width : Nat := (h - 247)
                  if ((length_width ≤b (source_len -i 1)) : Bool)
                  then
                    (do
                      let length_bytes := (stateless_input_sub_slice b 1 length_width)
                      (pure (true, (1 + length_width), (← (rlp_uint64_width
                            ⟨_, ⟨_, length_bytes⟩⟩ length_width)))))
                  else (fatal_error RlpDecode)))))

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_decode_list (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM (StatelessInputSliceFields (k_source_off + (k_source_len - k_content_len)) k_content_len) := do
  if _sailIf0 : (f.is_list : Bool) = true
  then
    (pure ((cast (by first | rfl | omega | (congr 1 <;> simp_all) | (congr 1 <;> omega) | (simp_all <;> omega) | (simp_all <;> rfl) | simp_all) ((stateless_input_sub_slice
        f.source (k_source_len - k_content_len) k_content_len))) : (StatelessInputSliceFields (k_source_off + (k_source_len - k_content_len)) k_content_len)))
  else (fatal_error RlpDecode)

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def rlp_decode_item (cursor : (StatelessInputSliceFields k_source_off k_source_len)) : SailM (Sigma
  fun (k_content_len : Nat) =>
  (Sigma fun (k_full_len : Nat) => (RlpFieldRef k_source_off k_full_len k_content_len))) := do
  if ((k_source_len == 0) : Bool)
  then (fatal_error RlpDecode)
  else (pure ())
  let (is_list, content_off, content_len_value) ← do (rlp_ref_hdr ⟨_, ⟨_, cursor⟩⟩)
  let content_len := content_len_value
  if ((k_source_len <b content_off) : Bool)
  then (fatal_error RlpDecode)
  else (pure ())
  if (((k_source_len -i content_off) <b content_len) : Bool)
  then (fatal_error RlpDecode)
  else (pure ())
  let full_len := (content_off + content_len)
  if _sailIf0 : (((0 <b full_len) && (full_len ≤b k_source_len)) : Bool) = true
  then
    (let field_source := (stateless_input_sub_slice cursor 0 full_len)
    let field : (RlpFieldRef k_source_off full_len content_len) :=
      { source := field_source,
        is_list := is_list }
    (pure ((⟨_, ⟨_, field⟩⟩ : (Sigma fun (k_syn_content_len : Nat) =>
      (Sigma fun (k_syn_full_len : Nat) =>
      (RlpFieldRef k_source_off k_syn_full_len k_syn_content_len)))) : (Sigma fun
      (k_syn_content_len : Nat) =>
      (Sigma fun (k_syn_full_len : Nat) =>
      (RlpFieldRef k_source_off k_syn_full_len k_syn_content_len))))))
  else
    (do
      (fatal_error RlpDecode))

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, consumed : Nat, (source_valid_range k_source_off k_source_len)
  ∧ (rlp_cursor_advance_valid k_source_len consumed) -/
def rlp_cursor_advance (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (consumed : Nat) : (StatelessInputSliceFields (k_source_off + consumed) (k_source_len - consumed)) :=
  (stateless_input_slice_suffix cursor consumed)

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def rlp_cursor_expect_end (cursor : (StatelessInputSliceFields k_source_off k_source_len)) : SailM Unit := do
  if ((k_source_len == 0) : Bool)
  then (pure ())
  else (fatal_error RlpDecode)

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def rlp_single_ref (item : (StatelessInputSliceFields k_source_off k_source_len)) : SailM (Sigma fun
  (k_content_len : Nat) => (RlpFieldRef k_source_off k_source_len k_content_len)) := do
  let item_length := k_source_len
  if _sailIf0 : ((item_length == 0) : Bool) = true
  then
    (do
      (fatal_error RlpDecode))
  else
    (do
      let (is_list, content_off, content_len_value) ← do (rlp_ref_hdr ⟨_, ⟨_, item⟩⟩)
      let content_len := content_len_value
      if _sailIf1 : (((content_off ≤b item_length) && (content_len == (item_length -i content_off))) : Bool) = true
      then
        (let field : (RlpFieldRef k_source_off k_source_len content_len) :=
          { source := item,
            is_list := is_list }
        (pure ((⟨_, field⟩ : (Sigma fun (k_syn_content_len : Nat) =>
          (RlpFieldRef k_source_off k_source_len k_syn_content_len))) : (Sigma fun
          (k_syn_content_len : Nat) => (RlpFieldRef k_source_off k_source_len k_syn_content_len)))))
      else
        (do
          (fatal_error RlpDecode)))

/-- A cursor over the children of a byte sequence that must be exactly
one RLP list (e.g. a trie node). -/
/- Type quantifiers: node_dependentWitness1 : Nat, node_dependentWitness0 : Nat, 0 ≤
  node_dependentWitness0 ∧
  0 ≤ node_dependentWitness1 ∧
  (node_dependentWitness0 + node_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def rlp_node_cursor (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Sigma fun
  (k_source_off : Nat) =>
  (Sigma fun (k_source_len : Nat) => (StatelessInputSliceFields k_source_off k_source_len))) := do
  let node_dependentWitness0 := (node).1
  let node_dependentWitness1 := ((node).2).1
  let node := ((node).2).2
  if _sailIf0 : ((node.len == 0) : Bool) = true
  then
    (do
      (fatal_error RlpDecode))
  else
    (do
      let ⟨_, item⟩ ← do (rlp_single_ref node)
      (do
        let dependentResult ← (rlp_decode_list item)
        pure ((⟨_, ⟨_, dependentResult⟩⟩ : (Sigma fun (k_source_off : Nat) =>
        (Sigma fun (k_source_len : Nat) => (StatelessInputSliceFields k_source_off k_source_len)))))))

/-- The content span of a field. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_item_content (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) :=
  ((⟨_, ⟨_, (stateless_input_sub_slice f.source (k_source_len - k_content_len) k_content_len)⟩⟩ : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))))

/-- [rlp_uint64_width][] over a scratch-backed content slice. -/
/- Type quantifiers: _reclimit : Nat, content_dependentWitness1 : Nat, content_dependentWitness0 :
  Nat, width : Nat, 0 ≤ width ∧ width ≤ 8, 0 ≤ content_dependentWitness0 ∧
  0 ≤ content_dependentWitness1 ∧
  (content_dependentWitness0 + content_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ _reclimit -/
def _rec_scratch_rlp_uint64_width (content : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) (width : Nat) (_reclimit : Nat) : SailM Nat := do
  let content_dependentWitness0 := (content).1
  let content_dependentWitness1 := ((content).2).1
  let content := ((content).2).2
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((width == 0) : Bool)
      then (pure 0)
      else
        (do
          let evm_prefix' ← do
            (_rec_scratch_rlp_uint64_width ⟨_, ⟨_, content⟩⟩ (width - 1) _reclimit_pred)
          let next ← do (scratch_byte ⟨_, ⟨_, content⟩⟩ (width - 1))
          (pure (rlp_uint64_append width evm_prefix' next))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- [rlp_uint64_width][] over a scratch-backed content slice. -/
/- Type quantifiers: content_dependentWitness1 : Nat, content_dependentWitness0 : Nat, width : Nat, 0
  ≤ width ∧ width ≤ 8, 0 ≤ content_dependentWitness0 ∧
  0 ≤ content_dependentWitness1 ∧
  (content_dependentWitness0 + content_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def scratch_rlp_uint64_width (content : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) (width : Nat) : SailM Nat := do
  let content_dependentWitness0 := (content).1
  let content_dependentWitness1 := ((content).2).1
  let content := ((content).2).2
  let _measure := (width : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_scratch_rlp_uint64_width ⟨_, ⟨_, content⟩⟩ width (_measure + 1))

/-- [rlp_ref_hdr][] over a scratch-backed source slice. -/
/- Type quantifiers: b_dependentWitness1 : Nat, b_dependentWitness0 : Nat, 0 ≤ b_dependentWitness0
  ∧ 0 ≤ b_dependentWitness1 ∧ (b_dependentWitness0 + b_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def scratch_rlp_ref_hdr (b : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : SailM (Bool × Nat × Nat) := do
  let b_dependentWitness0 := (b).1
  let b_dependentWitness1 := ((b).2).1
  let b := ((b).2).2
  let source_len := b.len
  if ((source_len == 0) : Bool)
  then (fatal_error RlpDecode)
  else (pure ())
  let first_byte ← do (scratch_byte ⟨_, ⟨_, b⟩⟩ 0)
  let h := (BitVec.toNatInt first_byte)
  if ((h <b 128) : Bool)
  then (pure (false, 0, 1))
  else
    (do
      if ((h <b 184) : Bool)
      then (pure (false, 1, (h - 128)))
      else
        (do
          if ((h <b 192) : Bool)
          then
            (do
              let length_width : Nat := (h - 183)
              if ((length_width ≤b (source_len -i 1)) : Bool)
              then
                (do
                  let length_bytes := (scratch_sub_slice b 1 length_width)
                  (pure (false, (1 + length_width), (← (scratch_rlp_uint64_width
                        ⟨_, ⟨_, length_bytes⟩⟩ length_width)))))
              else (fatal_error RlpDecode))
          else
            (do
              if ((h <b 248) : Bool)
              then (pure (true, 1, (h - 192)))
              else
                (do
                  let length_width : Nat := (h - 247)
                  if ((length_width ≤b (source_len -i 1)) : Bool)
                  then
                    (do
                      let length_bytes := (scratch_sub_slice b 1 length_width)
                      (pure (true, (1 + length_width), (← (scratch_rlp_uint64_width
                            ⟨_, ⟨_, length_bytes⟩⟩ length_width)))))
                  else (fatal_error RlpDecode)))))

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def scratch_rlp_decode_list (f : (ScratchRlpFieldRef k_source_off k_source_len k_content_len)) : SailM (ScratchSliceFields (k_source_off + (k_source_len - k_content_len)) k_content_len) := do
  if _sailIf0 : (f.is_list : Bool) = true
  then
    (pure ((cast (by first | rfl | omega | (congr 1 <;> simp_all) | (congr 1 <;> omega) | (simp_all <;> omega) | (simp_all <;> rfl) | simp_all) ((scratch_sub_slice
        f.source (k_source_len - k_content_len) k_content_len))) : (ScratchSliceFields (k_source_off + (k_source_len - k_content_len)) k_content_len)))
  else (fatal_error RlpDecode)

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def scratch_rlp_decode_item (cursor : (ScratchSliceFields k_source_off k_source_len)) : SailM (Sigma
  fun (k_content_len : Nat) =>
  (Sigma fun (k_full_len : Nat) => (ScratchRlpFieldRef k_source_off k_full_len k_content_len))) := do
  if ((k_source_len == 0) : Bool)
  then (fatal_error RlpDecode)
  else (pure ())
  let (is_list, content_off, content_len_value) ← do
    (scratch_rlp_ref_hdr ⟨_, ⟨_, cursor⟩⟩)
  let content_len := content_len_value
  if ((k_source_len <b content_off) : Bool)
  then (fatal_error RlpDecode)
  else (pure ())
  if (((k_source_len -i content_off) <b content_len) : Bool)
  then (fatal_error RlpDecode)
  else (pure ())
  let full_len := (content_off + content_len)
  if _sailIf0 : (((0 <b full_len) && (full_len ≤b k_source_len)) : Bool) = true
  then
    (let field_source := (scratch_sub_slice cursor 0 full_len)
    let field : (ScratchRlpFieldRef k_source_off full_len content_len) :=
      { source := field_source,
        is_list := is_list }
    (pure ((⟨_, ⟨_, field⟩⟩ : (Sigma fun (k_syn_content_len : Nat) =>
      (Sigma fun (k_syn_full_len : Nat) =>
      (ScratchRlpFieldRef k_source_off k_syn_full_len k_syn_content_len)))) : (Sigma fun
      (k_syn_content_len : Nat) =>
      (Sigma fun (k_syn_full_len : Nat) =>
      (ScratchRlpFieldRef k_source_off k_syn_full_len k_syn_content_len))))))
  else
    (do
      (fatal_error RlpDecode))

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, consumed : Nat, (source_valid_range k_source_off k_source_len)
  ∧ (rlp_cursor_advance_valid k_source_len consumed) -/
def scratch_rlp_cursor_advance (cursor : (ScratchSliceFields k_source_off k_source_len)) (consumed : Nat) : (ScratchSliceFields (k_source_off + consumed) (k_source_len - consumed)) :=
  (scratch_slice_suffix cursor consumed)

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def scratch_rlp_cursor_expect_end (cursor : (ScratchSliceFields k_source_off k_source_len)) : SailM Unit := do
  if ((k_source_len == 0) : Bool)
  then (pure ())
  else (fatal_error RlpDecode)

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def scratch_rlp_single_ref (item : (ScratchSliceFields k_source_off k_source_len)) : SailM (Sigma
  fun (k_content_len : Nat) => (ScratchRlpFieldRef k_source_off k_source_len k_content_len)) := do
  let item_length := k_source_len
  if _sailIf0 : ((item_length == 0) : Bool) = true
  then
    (do
      (fatal_error RlpDecode))
  else
    (do
      let (is_list, content_off, content_len_value) ← do
        (scratch_rlp_ref_hdr ⟨_, ⟨_, item⟩⟩)
      let content_len := content_len_value
      if _sailIf1 : (((content_off ≤b item_length) && (content_len == (item_length -i content_off))) : Bool) = true
      then
        (let field : (ScratchRlpFieldRef k_source_off k_source_len content_len) :=
          { source := item,
            is_list := is_list }
        (pure ((⟨_, field⟩ : (Sigma fun (k_syn_content_len : Nat) =>
          (ScratchRlpFieldRef k_source_off k_source_len k_syn_content_len))) : (Sigma fun
          (k_syn_content_len : Nat) =>
          (ScratchRlpFieldRef k_source_off k_source_len k_syn_content_len)))))
      else
        (do
          (fatal_error RlpDecode)))

/-- [rlp_node_cursor][] over a freshly encoded scratch node. -/
/- Type quantifiers: node_dependentWitness1 : Nat, node_dependentWitness0 : Nat, 0 ≤
  node_dependentWitness0 ∧
  0 ≤ node_dependentWitness1 ∧
  (node_dependentWitness0 + node_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def scratch_rlp_node_cursor (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : SailM (Sigma fun
  (k_source_off : Nat) =>
  (Sigma fun (k_source_len : Nat) => (ScratchSliceFields k_source_off k_source_len))) := do
  let node_dependentWitness0 := (node).1
  let node_dependentWitness1 := ((node).2).1
  let node := ((node).2).2
  let ⟨_, item⟩ ← do (scratch_rlp_single_ref node)
  (do
    let dependentResult ← (scratch_rlp_decode_list item)
    pure ((⟨_, ⟨_, dependentResult⟩⟩ : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) => (ScratchSliceFields k_source_off k_source_len))))))

/-- [rlp_item_content][] for a scratch-backed field. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def scratch_rlp_item_content (f : (ScratchRlpFieldRef k_source_off k_source_len k_content_len)) : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))) :=
  ((⟨_, ⟨_, (scratch_sub_slice f.source (k_source_len - k_content_len) k_content_len)⟩⟩ : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))))

/-- [rlp_decode_word][] for a scratch-backed field. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def scratch_rlp_decode_word (f : (ScratchRlpFieldRef k_source_off k_source_len k_content_len)) : SailM Nat := do
  let n := k_content_len
  if ((f.is_list || (RLP_WORD_LENGTH_LIMIT <b n)) : Bool)
  then (fatal_error RlpDecode)
  else (scratch_slice_load_n ⟨_, ⟨_, f.source⟩⟩ (k_source_len - n) n)

/-- Whether a field uses the unique canonical RLP framing for its payload. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_ref_framing_canonical (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Bool := do
  let n := k_content_len
  let payload_length := n
  let full_length := k_source_len
  let full_offset := 0
  let content_offset := (full_length - payload_length)
  let source := f.source
  let source_length := source.len
  if ((full_length == 0) : Bool)
  then (pure false)
  else
    (do
      if (f.is_list : Bool)
      then
        (do
          if ((n ≤b RLP_SHORT_LENGTH_LIMIT) : Bool)
          then
            (do
              let length_byte := (rlp_nat_length_byte n)
              if ((full_length == (payload_length + 1)) : Bool)
              then
                (do
                  (pure ((← (stateless_input_slice_byte ⟨_, ⟨_, f.source⟩⟩ 0)) == (0xC0#8 + length_byte))))
              else (pure false))
          else
            (do
              let length_word ← do (rlp_length_word n)
              let length_width := (rlp_minimal_word_len length_word)
              let length_byte := (rlp_nat_length_byte length_width)
              if ((full_length == ((1 + length_width) + payload_length)) : Bool)
              then
                (do
                  if (((← (stateless_input_slice_byte ⟨_, ⟨_, f.source⟩⟩ 0)) == (0xF7#8 + length_byte)) : Bool)
                  then
                    (do
                      (pure ((← (stateless_input_slice_load_n ⟨_, ⟨_, f.source⟩⟩
                              (full_offset + 1) length_width)) == length_word)))
                  else (pure false))
              else (pure false)))
      else
        (do
          if ((payload_length == 0) : Bool)
          then
            if ((full_length == 1) : Bool)
            then
              (do
                (pure ((← (stateless_input_slice_byte ⟨_, ⟨_, f.source⟩⟩ 0)) == 0x80#8)))
            else (pure false)
          else
            (do
              let first ← do
                (stateless_input_slice_byte ⟨_, ⟨_, f.source⟩⟩ content_offset)
              if (((payload_length == 1) && ((BitVec.access first 7) == 0#1)) : Bool)
              then
                if ((full_length == 1) : Bool)
                then
                  (do
                    (pure ((← (stateless_input_slice_byte ⟨_, ⟨_, f.source⟩⟩ 0)) == first)))
                else (pure false)
              else
                (do
                  if ((n ≤b RLP_SHORT_LENGTH_LIMIT) : Bool)
                  then
                    (do
                      let length_byte := (rlp_nat_length_byte n)
                      if ((full_length == (payload_length + 1)) : Bool)
                      then
                        (do
                          (pure ((← (stateless_input_slice_byte ⟨_, ⟨_, f.source⟩⟩ 0)) == (0x80#8 + length_byte))))
                      else (pure false))
                  else
                    (do
                      let length_word ← do (rlp_length_word n)
                      let length_width := (rlp_minimal_word_len length_word)
                      let length_byte := (rlp_nat_length_byte length_width)
                      if ((full_length == ((1 + length_width) + payload_length)) : Bool)
                      then
                        (do
                          if (((← (stateless_input_slice_byte ⟨_, ⟨_, f.source⟩⟩ 0)) == (0xB7#8 + length_byte)) : Bool)
                          then
                            (do
                              (pure ((← (stateless_input_slice_load_n ⟨_, ⟨_, f.source⟩⟩
                                      (full_offset + 1) length_width)) == length_word)))
                          else (pure false))
                      else (pure false))))))

/-- Whether a field is a byte string with its unique canonical RLP framing. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_ref_bytes_canonical (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Bool := do
  if ((! f.is_list) : Bool)
  then
    (do
      (rlp_ref_framing_canonical f))
  else (pure false)

/-- Whether a field is the canonical RLP encoding of a non-negative
integer: minimal big-endian content with no leading zeros and the
exact matching evm_prefix. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_item_uint_canonical (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Bool := do
  let bytes_canonical ← do (rlp_ref_bytes_canonical f)
  let invalid := (! bytes_canonical)
  if (invalid : Bool)
  then (pure false)
  else
    if ((k_content_len == 0) : Bool)
    then (pure true)
    else
      (do
        (pure ((← (stateless_input_slice_byte ⟨_, ⟨_, f.source⟩⟩
                (k_source_len - k_content_len))) != 0x00#8)))

/-- Whether a string field fits one EVM word. This is the non-throwing form
used by structural validation passes that do not need the decoded value. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_word_valid (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : Bool :=
  ((! f.is_list) && (k_content_len ≤b RLP_WORD_LENGTH_LIMIT))

/-- Whether a field is a canonical unsigned EVM-word integer. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_u256_valid (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Bool := do
  let canonical ← do (rlp_item_uint_canonical f)
  (pure (canonical && (k_content_len ≤b RLP_WORD_LENGTH_LIMIT)))

/-- Whether a field is a canonical unsigned 64-bit integer. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_uint64_valid (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Bool := do
  let canonical ← do (rlp_item_uint_canonical f)
  (pure (canonical && (k_content_len ≤b RLP_UINT64_LENGTH_LIMIT)))

/-- Decodes a string field of at most 32 bytes into a word. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_decode_word (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Nat := do
  let n := k_content_len
  if ((f.is_list || (RLP_WORD_LENGTH_LIMIT <b n)) : Bool)
  then (fatal_error RlpDecode)
  else (stateless_input_slice_load_n ⟨_, ⟨_, f.source⟩⟩ (k_source_len - n) n)

/-- Decodes a canonical unsigned integer field into a word; throws
otherwise. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_decode_u256 (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Nat := do
  let canonical ← do (rlp_item_uint_canonical f)
  if (canonical : Bool)
  then (rlp_decode_word f)
  else (fatal_error RlpDecode)

/-- Decodes a canonical unsigned integer into the uint64 wire domain used by
EIP-2681 account nonces and EIP-4844 excess blob gas. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_decode_uint64 (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Nat := do
  let canonical ← do (rlp_item_uint_canonical f)
  let invalid_encoding := (! canonical)
  if ((invalid_encoding || (RLP_UINT64_LENGTH_LIMIT <b k_content_len)) : Bool)
  then (fatal_error RlpDecode)
  else (pure ())
  let content_length := k_content_len
  let ⟨_, ⟨_, content⟩⟩ := (rlp_item_content f)
  let width : Nat := (Nat.mod content_length 9)
  (rlp_uint64_width ⟨_, ⟨_, content⟩⟩ width)

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_decode_bool (field : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM (RlpResult Bool) := do
  let value ← do (rlp_decode_uint64 field)
  match value with
  | 0 => (pure (RlpOk false))
  | 1 => (pure (RlpOk true))
  | _ => (pure (RlpInvalidValue ()))

