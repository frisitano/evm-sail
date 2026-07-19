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

open word
open option
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open b256
open ast
open address
open TxType
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open StateCheckpoint
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
open BlockError

/-! # State: storage

Warm/cold accounting (EIP-2929), persistent storage (`SLOAD`/`SSTORE`),
and transient storage (EIP-1153), over the host state stores. -/

def storage_key (a : address) (s : word) : StorageKey :=
  { addr := a,
    slot := s }

/-- Marks an address warm and returns its prior warm bit (EIP-2929: the
EVM charges the cold cost on `false`, the warm cost on `true`). The
host includes a new warm entry in its semantic checkpoint. -/
def k_access_account (a : address) : SailM Bool := do
  (warm_addr_touch a)

/-- Marks a storage slot warm and returns its prior warm bit. New entries
belong to the host's semantic checkpoint. -/
def k_slot_is_warm (a : address) (s : word) : SailM Bool := do
  (warm_slot_touch a s)

/-- Decodes an account trie leaf — `rlp([nonce, balance, storage_root,
code_hash])` — into an [AccountInfo][type-AccountInfo]; empty
root/hash fields decode to their empty-sentinel digests. -/
def decode_state_account (value : EvmByteSlice) : SailM AccountInfo := do
  let (nonce, fields) ← do (rlp_cursor_pop (← (rlp_node_cursor value)))
  let (balance, fields) ← do (rlp_cursor_pop fields)
  let (storage, fields) ← do (rlp_cursor_pop fields)
  let (code, fields) ← do (rlp_cursor_pop fields)
  (rlp_cursor_expect_end fields)
  let storage_root ← do
    if ((storage.content_len == BYTE_ZERO) : Bool)
    then (pure EMPTY_TRIE_ROOT)
    else (pure (word_to_hash (← (rlp_ref_word storage))))
  let code_hash ← do
    if ((code.content_len == BYTE_ZERO) : Bool)
    then (pure KECCAK_EMPTY)
    else (pure (word_to_hash (← (rlp_ref_word code))))
  (pure { nonce := ← do
              let semanticField ← (do
                  let semanticResult ← (rlp_ref_uint nonce)
                  pure ((semanticResult).value))
              pure (⟨semanticField⟩),
          balance := ← (rlp_ref_uint_word balance),
          storage_root := storage_root,
          code_hash := code_hash })

/-- Constructs a path from high-aligned data and a nibble length. -/
/- Type quantifiers: k_ex161279_ : Nat, 0 ≤ k_ex161279_ ∧ k_ex161279_ ≤ 64 -/
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
  let len ← do (nodedb_len h)
  if ((len == BYTE_ZERO) : Bool)
  then (pure EMPTY_SLICE)
  else (pure (stateless_input_byte_slice (← (nodedb_off h)) len))

/-- Selects a decoded branch child field by nibble value. -/
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

/-- Maps a nibble cursor to the corresponding byte in decreasing vector order. -/
/- Type quantifiers: i : Nat, 0 ≤ i ∧ i ≤ 64 -/
def path_byte_index (i : trie_path_cursor) : SailM b256_index := do
  let i := (i).value
  let semanticResult ← do
    let quotient := (Int.tdiv i 2)
    let natural_index ← (( do
      if (((0 ≤b quotient) && (quotient ≤b 31)) : Bool)
      then (pure quotient)
      else
        (do
          assert false "sail/lib/mpt/primitives.sail:64.24-64.25"
          throw Error.Exit) ) : SailM Nat )
    (pure (31 -i natural_index))
  pure (⟨semanticResult⟩)

/-- The path length in nibbles. -/
def path_len (path : TriePath) : trie_path_len :=
  ⟨(path.len).value⟩

/-- Increments a path length, rejecting a value already at the key bound. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def trie_path_len_increment (value : trie_path_len) : SailM trie_path_len := do
  let value := (value).value
  let semanticResult ← do
    if ((value <b 64) : Bool)
    then (pure (value + 1))
    else sailThrow ((InvalidBlock WitnessDeficient))
  pure (⟨semanticResult⟩)

/-- Appends one nibble to a path, rejecting paths already at the key bound. -/
def path_append_nibble (path : TriePath) (value : nibble) : SailM TriePath := do
  let length := ((path_len path)).value
  if ((64 ≤b length) : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else (pure ())
  let .B256 original := path.data
  let bytes := original
  let byte_index ← do
    (do
        let semanticResult ← (path_byte_index ⟨length⟩)
        pure ((semanticResult).value))
  let bytes : (Vector (BitVec 8) 32) :=
    if (((Int.tmod length 2) == 0) : Bool)
    then (vectorUpdate bytes byte_index (value +++ 0x0#4))
    else
      (vectorUpdate bytes byte_index
        ((Sail.BitVec.extractLsb (GetElem?.getElem! bytes byte_index) 7 4) +++ value))
  (pure (path_new (B256 bytes) ⟨((← (trie_path_len_increment ⟨length⟩))).value⟩))

/-- The `i`-th nibble, most significant first; out of range yields `0`. -/
/- Type quantifiers: k_ex161283_ : Nat, 0 ≤ k_ex161283_ ∧ k_ex161283_ ≤ 64 -/
def path_nibble (path : TriePath) (i : trie_path_cursor) : SailM nibble := do
  let i := (i).value
  if ((((path_len path)).value ≤b i) : Bool)
  then (pure 0x0#4)
  else
    (do
      let .B256 bytes := path.data
      let byte_index ← do
        (do
            let semanticResult ← (path_byte_index ⟨i⟩)
            pure ((semanticResult).value))
      if (((Int.tmod i 2) == 0) : Bool)
      then (pure (Sail.BitVec.extractLsb (GetElem?.getElem! bytes byte_index) 7 4))
      else (pure (Sail.BitVec.extractLsb (GetElem?.getElem! bytes byte_index) 3 0)))

/-- Path concatenation; over 64 nibbles is a witness fault. -/
def path_concat (a : TriePath) (b : TriePath) : SailM TriePath := do
  let alen := ((path_len a)).value
  let blen := ((path_len b)).value
  let combined := (alen + blen)
  if ((combined ≤b 64) : Bool)
  then
    (do
      let result := a
      let index : Nat := 0
      let (index, result) ← (( do
        let loop__step_lower := 0
        let loop__step_upper := 63
        let mut loop_vars := (index, result)
        for _step in [loop__step_lower:loop__step_upper:1]i do
          let (index, result) := loop_vars
          loop_vars ← do
            let (index, result) ← (( do
              if ((index <b blen) : Bool)
              then
                (do
                  let result ← (path_append_nibble result (← (path_nibble b ⟨index⟩)))
                  let index ←
                    (do
                        let semanticResult ← (trie_path_len_increment ⟨index⟩)
                        pure ((semanticResult).value))
                  (pure (index, result)))
              else (pure (index, result)) ) : SailM (Nat × TriePath) )
            (pure (index, result))
        (pure loop_vars) ) : SailM (Nat × TriePath) )
      (pure result))
  else sailThrow ((InvalidBlock WitnessDeficient))

/-- The empty path. -/
def path_empty (_ : Unit) : TriePath :=
  (path_new (b256_zero ()) ⟨0⟩)

/-- A one-nibble path. -/
def path_single (n : nibble) : SailM TriePath := do
  (path_append_nibble (path_empty ()) n)

def HEX_PREFIX_MAX_LENGTH : byte_length := (ByteQuantity 33)

/-- Decodes a compact path directly from its RLP source span, returning
the leaf flag and the path. -/
def hex_prefix_decode_ref (f : RlpFieldRef) : SailM (Bool × TriePath) := do
  if (f.is_list : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (pure ())
  let n := f.content_len
  let off := f.content_off
  if ((n == BYTE_ZERO) : Bool)
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
            (path_concat path (← (path_single (Sail.BitVec.extractLsb fb 3 0)))))
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
                  let path ← (path_concat path (← (path_single (Sail.BitVec.extractLsb b 7 4))))
                  let path ← (path_concat path (← (path_single (Sail.BitVec.extractLsb b 3 0))))
                  let cursor ← (byte_quantity_add cursor BYTE_ONE)
                  (pure (cursor, path)))
              else (pure (cursor, path)) ) : SailM (byte_quantity × TriePath) )
            (pure (cursor, path))
        (pure loop_vars) ) : SailM (byte_quantity × TriePath) )
      (pure (is_leaf, path)))

/-- Decodes node bytes into leaf/extension/branch form by field count
(2 = leaf or extension by the HP flag; 17 = branch). -/
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

/-- Copies a sub-32-byte node encoding into an inline node value. -/
def inline_node_from_slice (bytes : EvmByteSlice) : SailM InlineNode := do
  let length := bytes.len
  if ((byte_quantity_le MPT_HASH_LENGTH length) : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else (pure ())
  let data : (BitVec 256) := (BitVec.zero 256)
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

/-- The reference denoted by a child field: an inline list, a 32-byte
hash, or empty. -/
def field_to_ref (f : RlpFieldRef) : SailM NodeRef := do
  if (f.is_list : Bool)
  then (pure (InlineRef (← (inline_node_from_slice (← (rlp_ref_full f))))))
  else
    (do
      if ((f.content_len == MPT_HASH_LENGTH) : Bool)
      then (pure (HashRef (word_to_hash (← (rlp_ref_word f)))))
      else (pure (EmptyRef ())))

/-- Whether `seg` occurs in `key` at nibble position `pos`. -/
/- Type quantifiers: k_ex161285_ : Nat, 0 ≤ k_ex161285_ ∧ k_ex161285_ ≤ 64 -/
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
                        then (pure false)
                        else (pure ok))
                    else (pure false) ) : SailM Bool )
                  let offset ←
                    (do
                        let semanticResult ← (trie_path_len_increment ⟨offset⟩)
                        pure ((semanticResult).value))
                  (pure (offset, ok)))
              else (pure (offset, ok)) ) : SailM (Nat × Bool) )
            (pure (offset, ok))
        (pure loop_vars) ) : SailM (Nat × Bool) )
      (pure ok))

/-- Expands an inline node into its wire-order byte sequence. -/
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

/-- Materializes an inline node in scratch memory as a byte slice. -/
def inline_node_slice (node : InlineNode) : SailM EvmByteSlice := do
  let start ← do (scratch_begin ())
  (scratch_push_bytes (inline_node_to_list node) node.len)
  (scratch_finish start)

/-- Resolves a reference to node bytes. Resolving a missing hash is a
deficient witness (`InvalidBlock(WitnessDeficient)`), never an empty
subtree. -/
def resolve_ref (r : NodeRef) : SailM EvmByteSlice := do
  match r with
  | .EmptyRef () => (pure EMPTY_SLICE)
  | .InlineRef node => (inline_node_slice node)
  | .HashRef h =>
    (do
      let node ← do (node_db_lookup h)
      if ((node.len == BYTE_ZERO) : Bool)
      then sailThrow ((InvalidBlock WitnessDeficient))
      else (pure node))

/-- Walks the trie toward `key` from `pos`, returning the leaf value
without copying it; absent paths yield empty bytes. -/
/- Type quantifiers: _reclimit : Nat, k_ex161286_ : Nat, 0 ≤ k_ex161286_ ∧ k_ex161286_ ≤ 64, 0
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
      if ((node.len == BYTE_ZERO) : Bool)
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
                              (branch_children_get branch.children (← (path_nibble key ⟨pos⟩))))))
                      (_rec_trie_walk child key ⟨(pos + 1)⟩ _reclimit_pred))
                  else (pure EMPTY_SLICE)))
          | .InvalidNode () => (pure EMPTY_SLICE)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Walks the trie toward `key` from `pos`, returning the leaf value
without copying it; absent paths yield empty bytes. -/
/- Type quantifiers: pos : Nat, 0 ≤ pos ∧ pos ≤ 64 -/
def trie_walk (node : EvmByteSlice) (key : TriePath) (pos : trie_path_cursor) : SailM EvmByteSlice := do
  let pos := (pos).value
  let _measure := ((64 -i pos) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_trie_walk node key ⟨pos⟩ (_measure + 1))

/-- Looks up `key` from a root hash; the root node itself must be
witnessed. -/
def trie_lookup (root : hash) (key : TriePath) : SailM EvmByteSlice := do
  if ((root == EMPTY_TRIE_ROOT) : Bool)
  then (pure EMPTY_SLICE)
  else
    (do
      let node ← do (node_db_lookup root)
      if ((node.len == BYTE_ZERO) : Bool)
      then sailThrow ((InvalidBlock WitnessDeficient))
      else (trie_walk node key ⟨0⟩))

/-- The witnessed account at address `a` under state root `root`,
reading the secure trie at `keccak256(a)`; `None` when the walk
proves absence. -/
def stateless_account (root : hash) (a : address) : SailM (Option AccountInfo) := do
  let value ← do (trie_lookup root (path_from_hash (← (keccak256_address a))))
  if ((value.len == BYTE_ZERO) : Bool)
  then (pure none)
  else (pure (some (← (decode_state_account value))))

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
def stateless_storage (root : hash) (slot : word) : SailM word := do
  let value ← do (trie_lookup root (path_from_hash (← (keccak256_word slot))))
  if ((value.len == BYTE_ZERO) : Bool)
  then (pure ZERO_WORD)
  else (rlp_ref_uint_word (← (rlp_single_ref value)))

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

/-- `SSTORE`: creates or updates the live transaction row. The preceding
[k_sload][] supplies the transaction-original value; the host keeps
clear generations and frame undo history private. -/
def k_sstore (a : address) (s : word) (v : StorageValue) : SailM Unit := do
  (storage_tx_update
    { key := (storage_key a s),
      value := v })

/-- `TLOAD` (EIP-1153): reads per-transaction transient storage, which is
discarded at transaction end and is not part of the state trie. -/
def k_tload (a : address) (s : word) : SailM word := do
  (transient_load a s)

/-- `TSTORE` (EIP-1153): writes transient storage. Frame rollback is part
of the host's semantic checkpoint contract. -/
def k_tstore (a : address) (s : word) (v : word) : SailM Unit := do
  (transient_store a s v)

