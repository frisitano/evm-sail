import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Host.RegionAccess
import Evm.Primitives.Crypto
import Evm.Primitives.Fork
import Evm.Primitives.Account
import Evm.Lib.Rlp.Decoding
import Evm.Kernel.Environment

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

/-! # State: storage

Warm/cold accounting (EIP-2929), persistent storage (`SLOAD`/`SSTORE`),
and transient storage (EIP-1153), over the host state stores. -/

/- Type quantifiers: k_ex608955_ : Nat, 0 ≤ k_ex608955_ ∧ k_ex608955_ ≤ (2 ^ 256 - 1) -/
def storage_key (a : (Vector (BitVec 8) 20)) (s : Nat) : StorageKey :=
  { addr := a,
    slot := s }

/-- Whether address `n` is an active precompile at the current fork:
1–4 always; 5–8 from Byzantium; 9 from Istanbul; 10 from Cancun
(EIP-4844); 11–17 from Prague (EIP-2537); 0x100 from Osaka
(EIP-7951). -/
def precompile_active_at_fork (n : PrecompileId) : SailM Bool := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  match n with
  | .NotPrecompile => (pure false)
  | .Ecrecover => (pure true)
  | .Sha256 => (pure true)
  | .Ripemd160 => (pure true)
  | .Identity => (pure true)
  | .Modexp => (pure (profile.fork ≥b Byzantium))
  | .Bn254Add => (pure (profile.fork ≥b Byzantium))
  | .Bn254Mul => (pure (profile.fork ≥b Byzantium))
  | .Bn254Pairing => (pure (profile.fork ≥b Byzantium))
  | .Blake2f => (pure (profile.fork ≥b Istanbul))
  | .KzgPointEvaluation => (pure (profile.fork ≥b Cancun))
  | .BlsG1Add => (pure (profile.fork ≥b Prague))
  | .BlsG1Msm => (pure (profile.fork ≥b Prague))
  | .BlsG2Add => (pure (profile.fork ≥b Prague))
  | .BlsG2Msm => (pure (profile.fork ≥b Prague))
  | .BlsPairing => (pure (profile.fork ≥b Prague))
  | .BlsMapFpToG1 => (pure (profile.fork ≥b Prague))
  | .BlsMapFp2ToG2 => (pure (profile.fork ≥b Prague))
  | .P256Verify => (pure (profile.fork ≥b Osaka))

/-- Returns the active precompile represented by `target`, or
`NotPrecompile` when it is an ordinary address at the current fork. -/
def precompile_id_if_active (candidate : PrecompileId) : SailM PrecompileId := do
  let active ← do (precompile_active_at_fork candidate)
  if (active : Bool)
  then (pure candidate)
  else (pure NotPrecompile)

def PRECOMPILE_ADDRESS_1 : address := (address_from_nat 1)

def PRECOMPILE_ADDRESS_10 : address := (address_from_nat 10)

def PRECOMPILE_ADDRESS_11 : address := (address_from_nat 11)

def PRECOMPILE_ADDRESS_12 : address := (address_from_nat 12)

def PRECOMPILE_ADDRESS_13 : address := (address_from_nat 13)

def PRECOMPILE_ADDRESS_14 : address := (address_from_nat 14)

def PRECOMPILE_ADDRESS_15 : address := (address_from_nat 15)

def PRECOMPILE_ADDRESS_16 : address := (address_from_nat 16)

def PRECOMPILE_ADDRESS_17 : address := (address_from_nat 17)

def PRECOMPILE_ADDRESS_2 : address := (address_from_nat 2)

def PRECOMPILE_ADDRESS_256 : address := (address_from_nat 256)

def PRECOMPILE_ADDRESS_3 : address := (address_from_nat 3)

def PRECOMPILE_ADDRESS_4 : address := (address_from_nat 4)

def PRECOMPILE_ADDRESS_5 : address := (address_from_nat 5)

def PRECOMPILE_ADDRESS_6 : address := (address_from_nat 6)

def PRECOMPILE_ADDRESS_7 : address := (address_from_nat 7)

def PRECOMPILE_ADDRESS_8 : address := (address_from_nat 8)

def PRECOMPILE_ADDRESS_9 : address := (address_from_nat 9)

/-- Maps an address to its active precompile identifier; any other address,
including one whose precompile is not yet active at the current fork,
is `NotPrecompile`. -/
def precompile_id_for_address (bytes : (Vector (BitVec 8) 20)) : SailM PrecompileId := do
  let g__0 := bytes
  if ((bytes == PRECOMPILE_ADDRESS_1) : Bool)
  then (precompile_id_if_active Ecrecover)
  else
    (do
      if ((bytes == PRECOMPILE_ADDRESS_2) : Bool)
      then (precompile_id_if_active Sha256)
      else
        (do
          if ((bytes == PRECOMPILE_ADDRESS_3) : Bool)
          then (precompile_id_if_active Ripemd160)
          else
            (do
              if ((bytes == PRECOMPILE_ADDRESS_4) : Bool)
              then (precompile_id_if_active Identity)
              else
                (do
                  if ((bytes == PRECOMPILE_ADDRESS_5) : Bool)
                  then (precompile_id_if_active Modexp)
                  else
                    (do
                      if ((bytes == PRECOMPILE_ADDRESS_6) : Bool)
                      then (precompile_id_if_active Bn254Add)
                      else
                        (do
                          if ((bytes == PRECOMPILE_ADDRESS_7) : Bool)
                          then (precompile_id_if_active Bn254Mul)
                          else
                            (do
                              if ((bytes == PRECOMPILE_ADDRESS_8) : Bool)
                              then (precompile_id_if_active Bn254Pairing)
                              else
                                (do
                                  if ((bytes == PRECOMPILE_ADDRESS_9) : Bool)
                                  then (precompile_id_if_active Blake2f)
                                  else
                                    (do
                                      if ((bytes == PRECOMPILE_ADDRESS_10) : Bool)
                                      then (precompile_id_if_active KzgPointEvaluation)
                                      else
                                        (do
                                          if ((bytes == PRECOMPILE_ADDRESS_11) : Bool)
                                          then (precompile_id_if_active BlsG1Add)
                                          else
                                            (do
                                              if ((bytes == PRECOMPILE_ADDRESS_12) : Bool)
                                              then (precompile_id_if_active BlsG1Msm)
                                              else
                                                (do
                                                  if ((bytes == PRECOMPILE_ADDRESS_13) : Bool)
                                                  then (precompile_id_if_active BlsG2Add)
                                                  else
                                                    (do
                                                      if ((bytes == PRECOMPILE_ADDRESS_14) : Bool)
                                                      then (precompile_id_if_active BlsG2Msm)
                                                      else
                                                        (do
                                                          if ((bytes == PRECOMPILE_ADDRESS_15) : Bool)
                                                          then (precompile_id_if_active BlsPairing)
                                                          else
                                                            (do
                                                              if ((bytes == PRECOMPILE_ADDRESS_16) : Bool)
                                                              then
                                                                (precompile_id_if_active
                                                                  BlsMapFpToG1)
                                                              else
                                                                (do
                                                                  if ((bytes == PRECOMPILE_ADDRESS_17) : Bool)
                                                                  then
                                                                    (precompile_id_if_active
                                                                      BlsMapFp2ToG2)
                                                                  else
                                                                    (do
                                                                      if ((bytes == PRECOMPILE_ADDRESS_256) : Bool)
                                                                      then
                                                                        (precompile_id_if_active
                                                                          P256Verify)
                                                                      else (pure NotPrecompile))))))))))))))))))

/-- Returns the address's EIP-2929 warm bit without changing state. Active
precompiles are warm independently of the BAL-derived account table. -/
def k_account_is_warm (a : (Vector (BitVec 8) 20)) : SailM Bool := do
  let precompile_id ← do (precompile_id_for_address a)
  if ((bne precompile_id NotPrecompile) : Bool)
  then (pure true)
  else (account_is_warm a)

/-- Marks an address warm after the caller has established that its access gas
is affordable. Active precompiles need no host-table entry. -/
def k_account_mark_warm (a : (Vector (BitVec 8) 20)) : SailM Unit := do
  let precompile_id ← do (precompile_id_for_address a)
  if ((bne precompile_id NotPrecompile) : Bool)
  then (pure ())
  else (account_mark_warm a)

/-- Returns a storage slot's EIP-2929 warm bit without changing state. -/
/- Type quantifiers: k_ex608956_ : Nat, 0 ≤ k_ex608956_ ∧ k_ex608956_ ≤ (2 ^ 256 - 1) -/
def k_slot_is_warm (a : (Vector (BitVec 8) 20)) (s : Nat) : SailM Bool := do
  (storage_is_warm a s)

/-- Marks a storage slot warm after the caller has paid its access charge. -/
/- Type quantifiers: k_ex608957_ : Nat, 0 ≤ k_ex608957_ ∧ k_ex608957_ ≤ (2 ^ 256 - 1) -/
def k_slot_mark_warm (a : (Vector (BitVec 8) 20)) (s : Nat) : SailM Unit := do
  (storage_mark_warm a s)

/-- Warms an explicitly addressed storage slot while preparing a transaction's
access list, before any EVM frame owns the execution context. -/
/- Type quantifiers: k_ex608958_ : Nat, 0 ≤ k_ex608958_ ∧ k_ex608958_ ≤ (2 ^ 256 - 1) -/
def k_prewarm_slot (a : (Vector (BitVec 8) 20)) (s : Nat) : SailM Unit := do
  (storage_mark_warm a s)

/-- Decodes an account trie leaf — `rlp([nonce, balance, storage_root,
code_hash])` — into an [AccountInfo][type-AccountInfo]; empty
root/hash fields decode to their empty-sentinel digests. -/
/- Type quantifiers: value_dependentWitness1 : Nat, value_dependentWitness0 : Nat, 0 ≤
  value_dependentWitness0 ∧
  0 ≤ value_dependentWitness1 ∧
  (value_dependentWitness0 + value_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def decode_state_account (value : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM AccountInfo := do
  let value_dependentWitness0 := (value).1
  let value_dependentWitness1 := ((value).2).1
  let value := ((value).2).2
  let ⟨_, ⟨_, fields⟩⟩ ← do (rlp_node_cursor ⟨_, ⟨_, value⟩⟩)
  let ⟨_, ⟨_, nonce⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields nonce.source.len)
  let ⟨_, ⟨_, balance⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields balance.source.len)
  let ⟨_, ⟨_, storage⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields storage.source.len)
  let ⟨_, ⟨_, code⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields code.source.len)
  (rlp_cursor_expect_end fields)
  let storage_root ← do
    if ((storage.content_len == 0) : Bool)
    then (pure EMPTY_TRIE_ROOT)
    else
      (do
        let storage_word ← do (rlp_decode_word storage)
        (pure (word_to_hash storage_word)))
  let code_hash ← do
    if ((code.content_len == 0) : Bool)
    then (pure KECCAK_EMPTY)
    else
      (do
        let code_word ← do (rlp_decode_word code)
        (pure (word_to_hash code_word)))
  (pure { nonce := ← (rlp_decode_uint64 nonce),
          balance := ← (rlp_decode_u256 balance),
          storage_root := storage_root,
          code_hash := code_hash })

/-- Constructs a path from high-aligned data and a nibble length. -/
/- Type quantifiers: k_ex608965_ : Nat, 0 ≤ k_ex608965_ ∧ k_ex608965_ ≤ 64 -/
def path_new (data : (Vector (BitVec 8) 32)) (len : Nat) : TriePath :=
  { data := data,
    len := len }

/-- The witness node bytes whose KECCAK-256 digest is `h`, retained as a
slice into the stateless input; empty if unwitnessed. -/
def node_db_lookup (h : (Vector (BitVec 8) 32)) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) := do
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

abbrev MPT_HASH_LENGTH : Nat := 32

/-- The reference denoted by a child field: an inline list, a 32-byte
hash, or empty. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def input_field_to_ref (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM NodeRef := do
  if (f.is_list : Bool)
  then
    (do
      if ((k_source_len <b MPT_HASH_LENGTH) : Bool)
      then (pure (InputInlineRef ⟨_, ⟨_, f.source⟩⟩))
      else (fatal_error RlpDecode))
  else
    (do
      if ((k_content_len == MPT_HASH_LENGTH) : Bool)
      then
        (do
          let word ← do (rlp_decode_word f)
          let hash := (word_to_hash word)
          (pure (HashRef hash)))
      else (pure (EmptyRef ())))

/-- Decodes branch children 2 through 15, followed by the branch value. -/
/- Type quantifiers: _reclimit : Nat, k_ex609001_ : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 2
  ≤ k_ex609001_ ∧ k_ex609001_ ≤ 16, 0 ≤ _reclimit -/
def _rec_decode_input_branch_node (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (index : Nat) (children : (Vector NodeRef 16)) (_reclimit : Nat) : SailM InputTrieNode := do
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
          let ⟨_, ⟨_, child⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor child.source.len)
          let updated := children
          let updated ← (pure (vectorUpdate updated index (← (input_field_to_ref child))))
          (_rec_decode_input_branch_node next (index + 1) updated _reclimit_pred))
      else
        (do
          let ⟨_, ⟨_, value⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor value.source.len)
          (rlp_cursor_expect_end next)
          let ⟨_, ⟨_, content⟩⟩ := (rlp_item_content value)
          (pure (InputBranchNode
              ((fun (dependentValue0, dependentValue1) => (dependentValue0, ⟨_, ⟨_, dependentValue1⟩⟩)) ((children, content)))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Decodes branch children 2 through 15, followed by the branch value. -/
/- Type quantifiers: index : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 2
  ≤ index ∧ index ≤ 16 -/
def decode_input_branch_node (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (index : Nat) (children : (Vector NodeRef 16)) : SailM InputTrieNode := do
  let _measure := ((16 - index) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_decode_input_branch_node cursor index children (_measure + 1))

/-- The empty path. -/
def path_empty (_ : Unit) : TriePath :=
  (path_new ZERO_HASH 0)

abbrev HEX_PREFIX_MAX_LENGTH : Nat := 33

/-- Decodes a compact path directly from its RLP source span, returning
the leaf flag and the path. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def hex_prefix_decode_ref (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM (Bool × TriePath) := do
  if (f.is_list : Bool)
  then (fatal_error RlpDecode)
  else (pure ())
  let n := k_content_len
  if ((n == 0) : Bool)
  then (pure (false, (path_empty ())))
  else
    (do
      let maximum_length := HEX_PREFIX_MAX_LENGTH
      if ((maximum_length <b n) : Bool)
      then (fatal_error RlpDecode)
      else
        (do
          let content := (stateless_input_sub_slice f.source (k_source_len - n) n)
          let fb ← do (stateless_input_slice_byte ⟨_, ⟨_, content⟩⟩ 0)
          let flag : (BitVec 4) := (Sail.BitVec.extractLsb fb 7 4)
          let is_leaf : Bool := ((BitVec.access flag 1) == 1#1)
          let odd : Bool := ((BitVec.access flag 0) == 1#1)
          let tail_length : Nat := (n - 1)
          let tail := (stateless_input_slice_suffix content 1)
          let packed ← do (stateless_input_slice_load ⟨_, ⟨_, tail⟩⟩ 0)
          let paired_nibbles : Nat := (tail_length *i 2)
          if (odd : Bool)
          then
            (do
              if ((paired_nibbles <b 64) : Bool)
              then
                (let shifted := (word_shift_right packed 4)
                let bytes := (word_to_hash shifted)
                let bytes : (Vector (BitVec 8) 32) :=
                  (vectorUpdate bytes 0
                    ((Sail.BitVec.extractLsb fb 3 0) +++ (Sail.BitVec.extractLsb
                        (GetElem?.getElem! bytes 0) 3 0)))
                let path_data := (B256 bytes)
                let path := (path_new path_data (paired_nibbles + 1))
                (pure (is_leaf, path)))
              else (fatal_error WitnessDeficient))
          else
            (let path_data := (word_to_hash packed)
            let path := (path_new path_data paired_nibbles)
            (pure (is_leaf, path)))))

/-- The path length in nibbles. -/
def path_len (path : TriePath) : Nat :=
  path.len

/-- Decodes node bytes into leaf/extension/branch form by field count
(2 = leaf or extension by the HP flag; 17 = branch). -/
/- Type quantifiers: node_dependentWitness1 : Nat, node_dependentWitness0 : Nat, 0 ≤
  node_dependentWitness0 ∧
  0 ≤ node_dependentWitness1 ∧
  (node_dependentWitness0 + node_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def decode_input_trie_node (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM InputTrieNode := do
  let node_dependentWitness0 := (node).1
  let node_dependentWitness1 := ((node).2).1
  let node := ((node).2).2
  if ((node.len == 0) : Bool)
  then (fatal_error RlpDecode)
  else (pure ())
  let ⟨_, ⟨_, fields⟩⟩ ← do (rlp_node_cursor ⟨_, ⟨_, node⟩⟩)
  let ⟨_, ⟨_, first⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields first.source.len)
  let ⟨_, ⟨_, second⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields second.source.len)
  if ((fields.len == 0) : Bool)
  then
    (do
      let (is_leaf, path) ← do (hex_prefix_decode_ref first)
      if (is_leaf : Bool)
      then
        (let ⟨_, ⟨_, value⟩⟩ := (rlp_item_content second)
        (pure (InputLeafNode
            ((fun (dependentValue0, dependentValue1) => (dependentValue0, ⟨_, ⟨_, dependentValue1⟩⟩)) ((path, value))))))
      else
        (do
          let path_length := (path_len path)
          if ((path_length == 0) : Bool)
          then (fatal_error RlpDecode)
          else
            (do
              let child ← do (input_field_to_ref second)
              (pure (InputExtensionNode (path, child))))))
  else
    (do
      let empty_child := (EmptyRef ())
      let children : (Vector NodeRef 16) := (vectorInit empty_child)
      let children ← (pure (vectorUpdate children 0 (← (input_field_to_ref first))))
      let children ← (pure (vectorUpdate children 1 (← (input_field_to_ref second))))
      (decode_input_branch_node fields 2 children))

/-- Maps a nibble cursor to the corresponding canonical-order path byte. -/
/- Type quantifiers: i : Nat, 0 ≤ i ∧ i ≤ 64 -/
def path_byte_index (i : Nat) : SailM Nat := do
  let quotient := (Nat.div i 2)
  if (((0 ≤b quotient) && (quotient ≤b 31)) : Bool)
  then (pure quotient)
  else
    (do
      assert false "sail/lib/mpt/primitives.sail:45.24-45.25"
      throw Error.Exit)

/-- The `i`-th nibble, most significant first; out of range yields `0`. -/
/- Type quantifiers: k_ex609046_ : Nat, 0 ≤ k_ex609046_ ∧ k_ex609046_ ≤ 64 -/
def path_nibble (path : TriePath) (i : Nat) : SailM (BitVec 4) := do
  let length := (path_len path)
  if ((length ≤b i) : Bool)
  then (pure 0x0#4)
  else
    (do
      let bytes := path.data
      let byte_index ← do (path_byte_index i)
      let parity := (Nat.mod i 2)
      if ((parity == 0) : Bool)
      then (pure (Sail.BitVec.extractLsb (GetElem?.getElem! bytes byte_index) 7 4))
      else (pure (Sail.BitVec.extractLsb (GetElem?.getElem! bytes byte_index) 3 0)))

/-- Whether `seg` occurs in `key` at nibble position `pos`. -/
/- Type quantifiers: k_ex609047_ : Nat, 0 ≤ k_ex609047_ ∧ k_ex609047_ ≤ 64 -/
def path_matches (key : TriePath) (pos : Nat) (seg : TriePath) : SailM Bool := do
  let segment_len := (path_len seg)
  let key_len := (path_len key)
  let stop := (pos + segment_len)
  if ((key_len <b stop) : Bool)
  then (pure false)
  else
    (do
      let ok : Bool := true
      let offset : Nat := 0
      let (offset, ok) ← (( do
        let loop_vars ← whileFuelM (fuel :=(segment_len -i offset)) (fun (offset, ok) => (pure (ok && ((offset <b segment_len) : Bool)))) (offset, ok)
          fun (offset, ok) => do
            assert true "loop dummy assert"
            let key_index := (pos + offset)
            let ok ← (( do
              if ((key_index ≤b 64) : Bool)
              then
                (do
                  let key_nibble ← do (path_nibble key key_index)
                  let segment_nibble ← do (path_nibble seg offset)
                  if ((key_nibble != segment_nibble) : Bool)
                  then
                    (let ok : Bool := false
                    (pure ok))
                  else (pure ok))
              else
                (let ok : Bool := false
                (pure ok)) ) : SailM Bool )
            let current_offset := offset
            let offset ←
              if ((current_offset <b 64) : Bool)
              then (pure (current_offset + 1))
              else (fatal_error WitnessDeficient)
            (pure (offset, ok))
        (pure loop_vars) ) : SailM (Nat × Bool) )
      (pure ok))

/-- Resolves a reference to node bytes. Resolving a missing hash is a
deficient witness (`fatal_error(WitnessDeficient)`), never an empty
subtree. -/
def resolve_witness_ref (r : NodeRef) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) := do
  match r with
  | .EmptyRef () =>
    (pure ((⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma fun
      (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))))
  | .InputInlineRef ⟨_, ⟨_, node⟩⟩ =>
    (pure ((⟨_, ⟨_, node⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma fun
      (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))))
  | .ScratchInlineRef _ =>
    (do
      (fatal_error WitnessDeficient))
  | .HashRef h =>
    (do
      let ⟨_, ⟨_, node⟩⟩ ← do (node_db_lookup h)
      if _sailIf0 : ((node.len == 0) : Bool) = true
      then
        (do
          (fatal_error WitnessDeficient))
      else
        (pure ((⟨_, ⟨_, node⟩⟩ : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma fun
          (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))))))

/-- Walks the trie toward `key` from `pos`, returning the leaf value
without copying it; absent paths yield empty bytes. -/
/- Type quantifiers: _reclimit : Nat, k_ex609058_ : Nat, node_dependentWitness1 : Nat, node_dependentWitness0
  : Nat, 0 ≤ node_dependentWitness0 ∧
  0 ≤ node_dependentWitness1 ∧
  (node_dependentWitness0 + node_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ k_ex609058_ ∧
  k_ex609058_ ≤ 64, 0 ≤ _reclimit -/
def _rec_trie_walk (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (key : TriePath) (pos : Nat) (_reclimit : Nat) : SailM (Sigma
  fun (node_dependentWitness0 : Nat) =>
  (Sigma fun (node_dependentWitness1 : Nat) =>
  (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1))) := do
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
        (pure ((⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩ : (Sigma fun
          (node_dependentWitness0 : Nat) =>
          (Sigma fun (node_dependentWitness1 : Nat) =>
          (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1)))) : (Sigma fun
          (node_dependentWitness0 : Nat) =>
          (Sigma fun (node_dependentWitness1 : Nat) =>
          (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1)))))
      else
        (do
          let decoded ← do (decode_input_trie_node ⟨_, ⟨_, node⟩⟩)
          match decoded with
          | .InputLeafNode (path, value) =>
            (do
              let matches' ← do (path_matches key pos path)
              if _sailIf1 : ((! matches') : Bool) = true
              then
                (pure ((⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩ : (Sigma fun
                  (node_dependentWitness0 : Nat) =>
                  (Sigma fun (node_dependentWitness1 : Nat) =>
                  (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1)))) : (Sigma
                  fun (node_dependentWitness0 : Nat) =>
                  (Sigma fun (node_dependentWitness1 : Nat) =>
                  (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1)))))
              else
                (let path_length := (path_len path)
                let key_length := (path_len key)
                if _sailIf2 : (((pos + path_length) == key_length) : Bool) = true
                then
                  (pure (value : (Sigma fun (node_dependentWitness0 : Nat) =>
                    (Sigma fun (node_dependentWitness1 : Nat) =>
                    (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1)))))
                else
                  (pure ((⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩ : (Sigma fun
                    (node_dependentWitness0 : Nat) =>
                    (Sigma fun (node_dependentWitness1 : Nat) =>
                    (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1)))) : (Sigma
                    fun (node_dependentWitness0 : Nat) =>
                    (Sigma fun (node_dependentWitness1 : Nat) =>
                    (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1)))))))
          | .InputExtensionNode (path, childref) =>
            (do
              let extension_len : Nat := (path_len path)
              if _sailIf1 : ((extension_len == 0) : Bool) = true
              then
                (pure ((⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩ : (Sigma fun
                  (node_dependentWitness0 : Nat) =>
                  (Sigma fun (node_dependentWitness1 : Nat) =>
                  (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1)))) : (Sigma
                  fun (node_dependentWitness0 : Nat) =>
                  (Sigma fun (node_dependentWitness1 : Nat) =>
                  (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1)))))
              else
                (do
                  let matches' ← do (path_matches key pos path)
                  if _sailIf2 : ((! matches') : Bool) = true
                  then
                    (pure ((⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩ : (Sigma fun
                      (node_dependentWitness0 : Nat) =>
                      (Sigma fun (node_dependentWitness1 : Nat) =>
                      (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1)))) : (Sigma
                      fun (node_dependentWitness0 : Nat) =>
                      (Sigma fun (node_dependentWitness1 : Nat) =>
                      (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1)))))
                  else
                    (do
                      let next_pos := (pos + extension_len)
                      if _sailIf3 : ((next_pos ≤b 64) : Bool) = true
                      then
                        (do
                          let ⟨_, ⟨_, child⟩⟩ ← do (resolve_witness_ref childref)
                          (_rec_trie_walk ⟨_, ⟨_, child⟩⟩ key next_pos _reclimit_pred))
                      else
                        (pure ((⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩ : (Sigma fun
                          (node_dependentWitness0 : Nat) =>
                          (Sigma fun (node_dependentWitness1 : Nat) =>
                          (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1)))) : (Sigma
                          fun (node_dependentWitness0 : Nat) =>
                          (Sigma fun (node_dependentWitness1 : Nat) =>
                          (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1))))))))
          | .InputBranchNode (children, value) =>
            (do
              let key_length := (path_len key)
              if _sailIf1 : ((pos == key_length) : Bool) = true
              then
                (pure (value : (Sigma fun (node_dependentWitness0 : Nat) =>
                  (Sigma fun (node_dependentWitness1 : Nat) =>
                  (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1)))))
              else
                (do
                  if _sailIf2 : ((pos <b key_length) : Bool) = true
                  then
                    (do
                      let child_nibble ← do (path_nibble key pos)
                      let childref := (branch_refs_get children child_nibble)
                      let ⟨_, ⟨_, child⟩⟩ ← do (resolve_witness_ref childref)
                      (_rec_trie_walk ⟨_, ⟨_, child⟩⟩ key (pos + 1) _reclimit_pred))
                  else
                    (pure ((⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩ : (Sigma fun
                      (node_dependentWitness0 : Nat) =>
                      (Sigma fun (node_dependentWitness1 : Nat) =>
                      (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1)))) : (Sigma
                      fun (node_dependentWitness0 : Nat) =>
                      (Sigma fun (node_dependentWitness1 : Nat) =>
                      (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1)))))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Walks the trie toward `key` from `pos`, returning the leaf value
without copying it; absent paths yield empty bytes. -/
/- Type quantifiers: pos : Nat, node_dependentWitness1 : Nat, node_dependentWitness0 : Nat, 0 ≤
  node_dependentWitness0 ∧
  0 ≤ node_dependentWitness1 ∧
  (node_dependentWitness0 + node_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ pos ∧ pos ≤ 64 -/
def trie_walk (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (key : TriePath) (pos : Nat) : SailM (Sigma
  fun (node_dependentWitness0 : Nat) =>
  (Sigma fun (node_dependentWitness1 : Nat) =>
  (StatelessInputSliceFields node_dependentWitness0 node_dependentWitness1))) := do
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
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) := do
  if _sailIf0 : ((root == EMPTY_TRIE_ROOT) : Bool) = true
  then
    (pure ((⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma fun
      (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))))
  else
    (do
      let ⟨_, ⟨_, node⟩⟩ ← do (node_db_lookup root)
      if _sailIf1 : ((node.len == 0) : Bool) = true
      then
        (do
          (fatal_error WitnessDeficient))
      else
        (do
          (trie_walk ⟨_, ⟨_, node⟩⟩ key 0)))

/-- The witnessed account at address `a` under state root `root`,
reading the secure trie at `keccak256(a)`; a walk that proves absence
yields [EMPTY_ACCOUNT], whose `present` field is the absence witness. -/
def stateless_account_by_key (root : (Vector (BitVec 8) 32)) (address_hash : (Vector (BitVec 8) 32)) : SailM Account := do
  let path := (path_new address_hash 64)
  let ⟨_, ⟨_, value⟩⟩ ← do (trie_lookup root path)
  if ((value.len == 0) : Bool)
  then (pure EMPTY_ACCOUNT)
  else
    (do
      let account_info ← do (decode_state_account ⟨_, ⟨_, value⟩⟩)
      (pure (account_from_info account_info)))

/-- Resolves an account through the transaction and block overlays before an
authenticated witness read. A transaction-overlay hit was already touched
when that row was established; misses record the EIP-7928 account touch
before consulting block-scoped state. -/
def k_aload (a : (Vector (BitVec 8) 20)) : SailM Account := do
  let tx_account ← do (acct_tx_get a)
  if (tx_account.found : Bool)
  then (pure tx_account.account)
  else
    (do
      (bal_account_touch a)
      let block_account ← do (acct_block_get a)
      if (block_account.found : Bool)
      then (pure block_account.account)
      else
        (do
          let address_hash ← do (keccak256_address a)
          let account ← do
            (stateless_account_by_key (← readReg k_parent_state_root) address_hash)
          (acct_block_cache a address_hash account)
          (pure account)))

/-- The witnessed storage value of `slot` under a storage root, reading
the secure trie at `keccak256(slot)`; absent slots are zero. -/
def stateless_storage_by_key (root : (Vector (BitVec 8) 32)) (slot_hash : (Vector (BitVec 8) 32)) : SailM Nat := do
  let path := (path_new slot_hash 64)
  let ⟨_, ⟨_, value⟩⟩ ← do (trie_lookup root path)
  if ((value.len == 0) : Bool)
  then (pure ZERO_WORD)
  else
    (do
      let ⟨_, encoded_value⟩ ← do (rlp_single_ref value)
      (rlp_decode_u256 encoded_value))

/-- Resolves a slot to its live [StorageValue][type-StorageValue]: `curr`
is the value `SLOAD` pushes; `orig` is the EIP-2200 transaction-start
value the `SSTORE` gas policy compares against. The guarded
`SLOAD`/`SSTORE` opcode paths are the only callers, so reaching this
function consults the transaction overlay first. A real row hit was already
recorded when that row was established; misses record the EIP-7928 storage
read before consulting either a transaction-local clear generation or
block-scoped state. A clear generation makes an uncached slot known-zero,
but is not itself a slot-cache hit. BAL reads survive frame rollback (the
encoder removes slots that also have a storage change).
[stateless_storage_by_key][] is the base primitive — an authenticated MPT
point-get, one walk for both the witness and the harness-built alloc trie;
everything above it (the overlay, the read-through, the journal) is common. -/
/- Type quantifiers: k_ex609072_ : Nat, 0 ≤ k_ex609072_ ∧ k_ex609072_ ≤ (2 ^ 256 - 1) -/
def k_sload (a : (Vector (BitVec 8) 20)) (s : Nat) : SailM StorageValue := SailME.run do
  let key := (storage_key a s)
  let tx_value ← do (storage_tx_get key)
  match tx_value with
  | .StorageTxHit value => SailME.throw (value : StorageValue)
  | .StorageTxCleared _ =>
    SailME.throw (← do
        (bal_storage_read a s)
        (pure { curr := ZERO_WORD,
                orig := ZERO_WORD }))
  | .StorageTxMiss _ => (bal_storage_read a s)
  let block_value ← do (storage_block_get key)
  if (block_value.found : Bool)
  then
    (pure { curr := block_value.value.curr,
            orig := block_value.value.curr })
  else
    (do
      let acc ← do (k_aload a)
      let slot_hash ← do (keccak256_word s)
      let value ← do
        if (acc.storage_cleared : Bool)
        then (pure ZERO_WORD)
        else (stateless_storage_by_key acc.info.storage_root slot_hash)
      (storage_block_cache key slot_hash value)
      (pure { curr := value,
              orig := value }))

/-- `SSTORE`: creates or updates the live transaction row. The preceding
[k_sload][] supplies the transaction-original value; the host keeps
clear generations and frame undo history private. -/
/- Type quantifiers: k_ex609073_ : Nat, 0 ≤ k_ex609073_ ∧ k_ex609073_ ≤ (2 ^ 256 - 1) -/
def k_sstore (a : (Vector (BitVec 8) 20)) (s : Nat) (v : StorageValue) : SailM Unit := do
  let key := (storage_key a s)
  (storage_tx_update
    { key := key,
      value := v })

/-- `TLOAD` (EIP-1153): reads per-transaction transient storage, which is
discarded at transaction end and is not part of the state trie. -/
/- Type quantifiers: k_ex609074_ : Nat, 0 ≤ k_ex609074_ ∧ k_ex609074_ ≤ (2 ^ 256 - 1) -/
def k_tload (a : (Vector (BitVec 8) 20)) (s : Nat) : SailM Nat := do
  (transient_load a s)

/-- `TSTORE` (EIP-1153): writes transient storage. Frame rollback is part
of the host's semantic checkpoint contract. -/
/- Type quantifiers: k_ex609076_ : Nat, k_ex609075_ : Nat, 0 ≤ k_ex609075_ ∧
  k_ex609075_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex609076_ ∧ k_ex609076_ ≤ (2 ^ 256 - 1) -/
def k_tstore (a : (Vector (BitVec 8) 20)) (s : Nat) (v : Nat) : SailM Unit := do
  (transient_store a s v)

