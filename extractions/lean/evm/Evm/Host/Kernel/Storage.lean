import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Quantities
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

def storage_key (a : address) (s : word) : StorageKey :=
  { addr := a,
    slot := s }

def k_access_account (a : address) : SailM Bool := do
  let prior ← do (warm_addr_touch a)
  if ((! prior) : Bool)
  then (journal_push (JWarmA a))
  else (pure ())
  (pure prior)

def k_slot_is_warm (a : address) (s : word) : SailM Bool := do
  let prior ← do (warm_slot_touch a s)
  if ((! prior) : Bool)
  then (journal_push (JWarmS (a, s)))
  else (pure ())
  (pure prior)

def decode_state_account (value : EvmByteSlice) : SailM AccountInfo := do
  let (nonce, fields) ← do (rlp_cursor_pop (← (rlp_node_cursor value)))
  let (balance, fields) ← do (rlp_cursor_pop fields)
  let (storage, fields) ← do (rlp_cursor_pop fields)
  let (code, fields) ← do (rlp_cursor_pop fields)
  (rlp_cursor_expect_end fields)
  (pure { nonce := ← do
              let semanticField ← (do
                  let semanticResult ← (rlp_ref_uint nonce)
                  pure ((semanticResult).value))
              pure (⟨semanticField⟩),
          balance := ← (rlp_ref_uint_word balance),
          storage_root := ← if ((byte_quantity_equal storage.content_len BYTE_ZERO) : Bool)
            then (pure EMPTY_TRIE_ROOT)
            else (rlp_ref_word storage),
          code_hash := ← if ((byte_quantity_equal code.content_len BYTE_ZERO) : Bool)
            then (pure KECCAK_EMPTY)
            else (rlp_ref_word code) })

/-- Type quantifiers: k_ex161143_ : Nat, 0 ≤ k_ex161143_ ∧ k_ex161143_ ≤ 64 -/
def path_new (data : (BitVec 256)) (len : trie_path_len) : TriePath :=
  let len := (len).value
  { data := data,
    len := ⟨len⟩ }

def path_from_hash (h : (BitVec 256)) : TriePath :=
  (path_new h ⟨64⟩)

def node_db_lookup (h : (BitVec 256)) : SailM EvmByteSlice := do
  let len ← do (nodedb_len h)
  if ((byte_quantity_equal len BYTE_ZERO) : Bool)
  then (pure EMPTY_SLICE)
  else (pure (stateless_input_byte_slice (← (nodedb_off h)) len))

def branch_children_get (children : BranchChildren) (index : nibble) : RlpFieldRef :=
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

def path_len (path : TriePath) : trie_path_len :=
  ⟨(path.len).value⟩

def path_concat (a : TriePath) (b : TriePath) : SailM TriePath := do
  let alen := ((path_len a)).value
  let blen := ((path_len b)).value
  let combined := (alen + blen)
  if ((combined ≤b 64) : Bool)
  then (pure (path_new (a.data ||| (b.data >>> (alen *i 4))) ⟨combined⟩))
  else sailThrow ((InvalidBlock WitnessDeficient))

def path_empty (_ : Unit) : TriePath :=
  (path_new ZERO_WORD ⟨0⟩)

def path_single (n : nibble) : TriePath :=
  (path_new ((Sail.BitVec.zeroExtend n 256) <<< 252) ⟨1⟩)

def HEX_PREFIX_MAX_LENGTH : byte_length := (ByteQuantity 33)

def hex_prefix_decode_ref (f : RlpFieldRef) : SailM (Bool × TriePath) := do
  if (f.is_list : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (pure ())
  let n := f.content_len
  let off := f.content_off
  if ((byte_quantity_equal n BYTE_ZERO) : Bool)
  then (pure (false, (path_empty ())))
  else
    (do
      if ((byte_quantity_lt HEX_PREFIX_MAX_LENGTH n) : Bool)
      then sailThrow ((InvalidBlock RlpDecode))
      else (pure ())
      let fb ← do (slice_byte f.source off)
      let flag : (BitVec 4) := (Sail.BitVec.extractLsb fb 7 4)
      let is_leaf : Bool := ((BitVec.access flag 1) == 1#1)
      let odd : Bool := ((BitVec.access flag 0) == 1#1)
      let path := (path_empty ())
      let path ← (( do
        if (odd : Bool)
        then
          (do
            (path_concat path (path_single (Sail.BitVec.extractLsb fb 3 0))))
        else (pure path) ) : SailM TriePath )
      let cursor : byte_quantity := BYTE_ONE
      let (cursor, path) ← (( do
        let loop__step_lower := 0
        let loop__step_upper := 31
        let mut loop_vars := (cursor, path)
        for _step in [loop__step_lower:loop__step_upper:1]i do
          let (cursor, path) := loop_vars
          loop_vars ← do
            let (cursor, path) ← (( do
              if ((byte_quantity_lt cursor n) : Bool)
              then
                (do
                  let b ← do (slice_byte f.source (← (byte_quantity_add off cursor)))
                  let path ← (path_concat path (path_single (Sail.BitVec.extractLsb b 7 4)))
                  let path ← (path_concat path (path_single (Sail.BitVec.extractLsb b 3 0)))
                  let cursor ← (byte_quantity_add cursor BYTE_ONE)
                  (pure (cursor, path)))
              else (pure (cursor, path)) ) : SailM (byte_quantity × TriePath) )
            (pure (cursor, path))
        (pure loop_vars) ) : SailM (byte_quantity × TriePath) )
      (pure (is_leaf, path)))

def decode_trie_node (node : EvmByteSlice) : SailM TrieNode := do
  let fields ← do (rlp_node_cursor node)
  if (((! fields.valid) || (rlp_cursor_empty fields)) : Bool)
  then (pure (InvalidNode ()))
  else
    (do
      let (first, fields) ← do (rlp_cursor_pop fields)
      if ((rlp_cursor_empty fields) : Bool)
      then (pure (InvalidNode ()))
      else
        (do
          let (second, fields) ← do (rlp_cursor_pop fields)
          if ((rlp_cursor_empty fields) : Bool)
          then
            (do
              let path_field := first
              let value := second
              let (is_leaf, path) ← do (hex_prefix_decode_ref path_field)
              if (is_leaf : Bool)
              then
                (pure (LeafNode
                    { path := path,
                      value := value }))
              else
                (if ((((path_len path)).value == 0) : Bool)
                then (pure (InvalidNode ()))
                else
                  (pure (ExtensionNode
                      { path := path,
                        child := value }))))
          else
            (do
              let children : (Vector RlpFieldRef 16) := (vectorInit first)
              let children : (Vector RlpFieldRef 16) := (vectorUpdate children 0 first)
              let children : (Vector RlpFieldRef 16) := (vectorUpdate children 1 second)
              let cursor := fields
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
                        (pure (children, complete, cursor)))
                      else
                        (do
                          let (child, next) ← do (rlp_cursor_pop cursor)
                          let children : (Vector RlpFieldRef 16) :=
                            (vectorUpdate children index child)
                          let cursor : RlpCursor := next
                          (pure (children, complete, cursor))) ) : SailM
                      ((Vector RlpFieldRef 16) × Bool × RlpCursor) )
                    (pure (children, complete, cursor))
                (pure loop_vars) ) : SailM ((Vector RlpFieldRef 16) × Bool × RlpCursor) )
              if (((! complete) || (rlp_cursor_empty cursor)) : Bool)
              then (pure (InvalidNode ()))
              else
                (do
                  let (value, cursor) ← do (rlp_cursor_pop cursor)
                  if ((rlp_cursor_empty cursor) : Bool)
                  then
                    (pure (BranchNode
                        { children := children,
                          value := value }))
                  else (pure (InvalidNode ()))))))

def MPT_HASH_LENGTH : byte_length := WORD_BYTE_LENGTH

def inline_node_from_slice (bytes : EvmByteSlice) : SailM InlineNode := do
  let length := bytes.len
  if ((byte_quantity_le MPT_HASH_LENGTH length) : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else (pure ())
  let data : (BitVec 256) := ZERO_WORD
  let data ← (( do
    let loop_offset_lower := 0
    let loop_offset_upper := 30
    let mut loop_vars := data
    for offset in [loop_offset_lower:loop_offset_upper:1]i do
      let data := loop_vars
      loop_vars ← do
        let byte_offset := (ByteQuantity offset)
        if ((byte_quantity_lt byte_offset length) : Bool)
        then
          (do
            (pure ((data <<< 8) ||| (Sail.BitVec.zeroExtend (← (slice_byte bytes byte_offset)) 256))))
        else (pure data)
    (pure loop_vars) ) : SailM (BitVec 256) )
  (pure { data := data,
          len := length })

def field_to_ref (f : RlpFieldRef) : SailM NodeRef := do
  if (f.is_list : Bool)
  then (pure (InlineRef (← (inline_node_from_slice (← (rlp_ref_full f))))))
  else
    (do
      if ((byte_quantity_equal f.content_len MPT_HASH_LENGTH) : Bool)
      then (pure (HashRef (← (rlp_ref_word f))))
      else (pure (EmptyRef ())))

/-- Type quantifiers: k_ex161148_ : Nat, 0 ≤ k_ex161148_ ∧ k_ex161148_ ≤ 64 -/
def path_nibble (path : TriePath) (i : trie_path_cursor) : nibble :=
  let i := (i).value
  if ((((path_len path)).value ≤b i) : Bool)
  then 0x0#4
  else
    (let distance := (63 -i i)
    (Sail.BitVec.extractLsb (path.data >>> (distance *i 4)) 3 0))

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def trie_path_len_increment (value : trie_path_len) : SailM trie_path_len := do
  let value := (value).value
  let semanticResult ← do
    if ((value <b 64) : Bool)
    then (pure (value + 1))
    else sailThrow ((InvalidBlock WitnessDeficient))
  pure (⟨semanticResult⟩)

/-- Type quantifiers: k_ex161150_ : Nat, 0 ≤ k_ex161150_ ∧ k_ex161150_ ≤ 64 -/
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
                  let ok : Bool :=
                    if ((key_index ≤b 64) : Bool)
                    then
                      (if (((path_nibble key ⟨key_index⟩) != (path_nibble seg ⟨offset⟩)) : Bool)
                      then false
                      else ok)
                    else false
                  let offset ←
                    (do
                        let semanticResult ← (trie_path_len_increment ⟨offset⟩)
                        pure ((semanticResult).value))
                  (pure (offset, ok)))
              else (pure (offset, ok)) ) : SailM (Nat × Bool) )
            (pure (offset, ok))
        (pure loop_vars) ) : SailM (Nat × Bool) )
      (pure ok))

def inline_node_to_list (node : InlineNode) : (List byte) := Id.run do
  let bytes : (List (BitVec 8)) := []
  let data : (BitVec 256) := node.data
  let (bytes, data) ← (( do
    let loop_index_lower := 0
    let loop_index_upper := 30
    let mut loop_vars := (bytes, data)
    for index in [loop_index_lower:loop_index_upper:1]i do
      let (bytes, data) := loop_vars
      loop_vars :=
        let (bytes, data) : ((List (BitVec 8)) × (BitVec 256)) :=
          if ((byte_quantity_lt (ByteQuantity index) node.len) : Bool)
          then
            (let bytes : (List (BitVec 8)) := ((Sail.BitVec.extractLsb data 7 0) :: bytes)
            let data : (BitVec 256) := (data >>> 8)
            (bytes, data))
          else (bytes, data)
        (bytes, data)
    (pure loop_vars) ) : Id ((List (BitVec 8)) × (BitVec 256)) )
  (pure bytes)

def inline_node_slice (node : InlineNode) : SailM EvmByteSlice := do
  let start ← do (scratch_begin ())
  (scratch_push_bytes (inline_node_to_list node) node.len)
  (scratch_finish start)

def resolve_ref (r : NodeRef) : SailM EvmByteSlice := do
  match r with
  | .EmptyRef () => (pure EMPTY_SLICE)
  | .InlineRef node => (inline_node_slice node)
  | .HashRef h =>
    (do
      let node ← do (node_db_lookup h)
      if ((byte_quantity_equal node.len BYTE_ZERO) : Bool)
      then sailThrow ((InvalidBlock WitnessDeficient))
      else (pure node))

/-- Type quantifiers: _reclimit : Nat, k_ex161151_ : Nat, 0 ≤ k_ex161151_ ∧ k_ex161151_ ≤ 64, 0
  ≤ _reclimit -/
def _rec_trie_walk (node : EvmByteSlice) (key : TriePath) (pos : trie_path_cursor) (_reclimit : Nat) : SailM EvmByteSlice := do
  let pos := (pos).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((byte_quantity_equal node.len BYTE_ZERO) : Bool)
      then (pure EMPTY_SLICE)
      else
        (do
          match (← (decode_trie_node node)) with
          | .LeafNode leaf =>
            (do
              if ((! (← (path_matches key ⟨pos⟩ leaf.path))) : Bool)
              then (pure EMPTY_SLICE)
              else
                (do
                  if (((pos + ((path_len leaf.path)).value) == ((path_len key)).value) : Bool)
                  then (rlp_ref_content leaf.value)
                  else (pure EMPTY_SLICE)))
          | .ExtensionNode ext =>
            (do
              let extension_len : Nat := ((path_len ext.path)).value
              if ((extension_len == 0) : Bool)
              then (pure EMPTY_SLICE)
              else
                (do
                  if ((! (← (path_matches key ⟨pos⟩ ext.path))) : Bool)
                  then (pure EMPTY_SLICE)
                  else
                    (do
                      let next_pos := (pos + extension_len)
                      if ((next_pos ≤b 64) : Bool)
                      then
                        (_rec_trie_walk (← (resolve_ref (← (field_to_ref ext.child)))) key
                          ⟨next_pos⟩ _reclimit_pred)
                      else (pure EMPTY_SLICE))))
          | .BranchNode branch =>
            (do
              if ((pos == ((path_len key)).value) : Bool)
              then (rlp_ref_content branch.value)
              else
                (do
                  if ((pos <b ((path_len key)).value) : Bool)
                  then
                    (do
                      let child ← do
                        (resolve_ref
                          (← (field_to_ref
                              (branch_children_get branch.children (path_nibble key ⟨pos⟩)))))
                      (_rec_trie_walk child key ⟨(pos + 1)⟩ _reclimit_pred))
                  else (pure EMPTY_SLICE)))
          | .InvalidNode () => (pure EMPTY_SLICE)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Type quantifiers: pos : Nat, 0 ≤ pos ∧ pos ≤ 64 -/
def trie_walk (node : EvmByteSlice) (key : TriePath) (pos : trie_path_cursor) : SailM EvmByteSlice := do
  let pos := (pos).value
  let _measure := ((64 -i pos) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_trie_walk node key ⟨pos⟩ (_measure + 1))

def trie_lookup (root : hash) (key : TriePath) : SailM EvmByteSlice := do
  if ((root == EMPTY_TRIE_ROOT) : Bool)
  then (pure EMPTY_SLICE)
  else
    (do
      let node ← do (node_db_lookup root)
      if ((byte_quantity_equal node.len BYTE_ZERO) : Bool)
      then sailThrow ((InvalidBlock WitnessDeficient))
      else (trie_walk node key ⟨0⟩))

def stateless_account (root : hash) (a : address) : SailM (Option AccountInfo) := do
  let value ← do (trie_lookup root (path_from_hash (← (keccak256_address a))))
  if ((byte_quantity_equal value.len BYTE_ZERO) : Bool)
  then (pure none)
  else (pure (some (← (decode_state_account value))))

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

def stateless_storage (root : hash) (slot : word) : SailM word := do
  let value ← do (trie_lookup root (path_from_hash (← (keccak256_word slot))))
  if ((byte_quantity_equal value.len BYTE_ZERO) : Bool)
  then (pure ZERO_WORD)
  else (rlp_ref_uint_word (← (rlp_single_ref value)))

def k_sload (a : address) (s : word) : SailM StorageValue := SailME.run do
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

def k_sstore (a : address) (s : word) (v : StorageValue) : SailM Unit := do
  (storage_tx_update
    { key := (storage_key a s),
      value := v })

def k_tload (a : address) (s : word) : SailM word := do
  (transient_load a s)

def k_tstore (a : address) (s : word) (v : word) : SailM Unit := do
  (journal_push (JTran (a, s, (← (transient_load a s)))))
  (transient_store a s v)

