import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Host.RegionAccess
import Evm.Lib.Ssz.Ssz
import Evm.Kernel.Logs
import Evm.Lib.Ssz.StatelessInput

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

/-! # SSZ hash_tree_root

Consensus-layer merkleization (SHA-256). The standard specification follows
the recursive SSZ definition directly: a subtree is either its leaf, the
precomputed zero root at that depth, or the hash of its two children. Source
references are indexed only when recursion reaches a live leaf, so no leaf
list or explicit Merkle frontier is materialized. -/

abbrev HTR_BYTE_LIST_LIMIT : Nat := 1073741824

abbrev HTR_DEPOSIT_LENGTH : Nat := 192

abbrev HTR_DEPOSIT_PUBKEY : Nat := 0

abbrev HTR_DEPOSIT_PUBKEY_LENGTH : Nat := 48

abbrev HTR_DEPOSIT_WITHDRAWAL_CREDENTIALS : Nat := 48

abbrev HTR_DEPOSIT_AMOUNT : Nat := 80

abbrev HTR_DEPOSIT_SIGNATURE : Nat := 88

abbrev HTR_DEPOSIT_SIGNATURE_LENGTH : Nat := 96

abbrev HTR_DEPOSIT_INDEX : Nat := 184

abbrev HTR_WITHDRAWAL_REQUEST_LENGTH : Nat := 76

abbrev HTR_REQUEST_SOURCE_ADDRESS : Nat := 0

abbrev HTR_REQUEST_SOURCE_PUBKEY : Nat := 20

abbrev HTR_REQUEST_PUBKEY_LENGTH : Nat := 48

abbrev HTR_WITHDRAWAL_REQUEST_AMOUNT : Nat := 68

abbrev HTR_CONSOLIDATION_REQUEST_LENGTH : Nat := 116

abbrev HTR_CONSOLIDATION_REQUEST_TARGET_PUBKEY : Nat := 68

abbrev HTR_BUILDER_DEPOSIT_LENGTH : Nat := 184

abbrev HTR_BUILDER_DEPOSIT_PUBKEY : Nat := 0

abbrev HTR_BUILDER_DEPOSIT_WITHDRAWAL_CREDENTIALS : Nat := 48

abbrev HTR_BUILDER_DEPOSIT_AMOUNT : Nat := 80

abbrev HTR_BUILDER_DEPOSIT_SIGNATURE : Nat := 88

abbrev HTR_BUILDER_EXIT_LENGTH : Nat := 68

abbrev HTR_BUILDER_EXIT_SOURCE_ADDRESS : Nat := 0

abbrev HTR_BUILDER_EXIT_PUBKEY : Nat := 20

def SSZ_ZERO_HASH_01 : hash :=
  (hash_from_bits 0xF5A5FD42D16A20302798EF6ED309979B43003D2320D9F0E8EA9831A92759FB4B#256)

def SSZ_ZERO_HASH_02 : hash :=
  (hash_from_bits 0xDB56114E00FDD4C1F85C892BF35AC9A89289AAECB1EBD0A96CDE606A748B5D71#256)

def SSZ_ZERO_HASH_03 : hash :=
  (hash_from_bits 0xC78009FDF07FC56A11F122370658A353AAA542ED63E44C4BC15FF4CD105AB33C#256)

def SSZ_ZERO_HASH_04 : hash :=
  (hash_from_bits 0x536D98837F2DD165A55D5EEAE91485954472D56F246DF256BF3CAE19352A123C#256)

def SSZ_ZERO_HASH_05 : hash :=
  (hash_from_bits 0x9EFDE052AA15429FAE05BAD4D0B1D7C64DA64D03D7A1854A588C2CB8430C0D30#256)

def SSZ_ZERO_HASH_06 : hash :=
  (hash_from_bits 0xD88DDFEED400A8755596B21942C1497E114C302E6118290F91E6772976041FA1#256)

def SSZ_ZERO_HASH_07 : hash :=
  (hash_from_bits 0x87EB0DDBA57E35F6D286673802A4AF5975E22506C7CF4C64BB6BE5EE11527F2C#256)

def SSZ_ZERO_HASH_08 : hash :=
  (hash_from_bits 0x26846476FD5FC54A5D43385167C95144F2643F533CC85BB9D16B782F8D7DB193#256)

def SSZ_ZERO_HASH_09 : hash :=
  (hash_from_bits 0x506D86582D252405B840018792CAD2BF1259F1EF5AA5F887E13CB2F0094F51E1#256)

def SSZ_ZERO_HASH_10 : hash :=
  (hash_from_bits 0xFFFF0AD7E659772F9534C195C815EFC4014EF1E1DAED4404C06385D11192E92B#256)

def SSZ_ZERO_HASH_11 : hash :=
  (hash_from_bits 0x6CF04127DB05441CD833107A52BE852868890E4317E6A02AB47683AA75964220#256)

def SSZ_ZERO_HASH_12 : hash :=
  (hash_from_bits 0xB7D05F875F140027EF5118A2247BBB84CE8F2F0F1123623085DAF7960C329F5F#256)

def SSZ_ZERO_HASH_13 : hash :=
  (hash_from_bits 0xDF6AF5F5BBDB6BE9EF8AA618E4BF8073960867171E29676F8B284DEA6A08A85E#256)

def SSZ_ZERO_HASH_14 : hash :=
  (hash_from_bits 0xB58D900F5E182E3C50EF74969EA16C7726C549757CC23523C369587DA7293784#256)

def SSZ_ZERO_HASH_15 : hash :=
  (hash_from_bits 0xD49A7502FFCFB0340B1D7885688500CA308161A7F96B62DF9D083B71FCC8F2BB#256)

def SSZ_ZERO_HASH_16 : hash :=
  (hash_from_bits 0x8FE6B1689256C0D385F42F5BBE2027A22C1996E110BA97C171D3E5948DE92BEB#256)

def SSZ_ZERO_HASH_17 : hash :=
  (hash_from_bits 0x8D0D63C39EBADE8509E0AE3C9C3876FB5FA112BE18F905ECACFECB92057603AB#256)

def SSZ_ZERO_HASH_18 : hash :=
  (hash_from_bits 0x95EEC8B2E541CAD4E91DE38385F2E046619F54496C2382CB6CACD5B98C26F5A4#256)

def SSZ_ZERO_HASH_19 : hash :=
  (hash_from_bits 0xF893E908917775B62BFF23294DBBE3A1CD8E6CC1C35B4801887B646A6F81F17F#256)

def SSZ_ZERO_HASH_20 : hash :=
  (hash_from_bits 0xCDDBA7B592E3133393C16194FAC7431ABF2F5485ED711DB282183C819E08EBAA#256)

def SSZ_ZERO_HASH_21 : hash :=
  (hash_from_bits 0x8A8D7FE3AF8CAA085A7639A832001457DFB9128A8061142AD0335629FF23FF9C#256)

def SSZ_ZERO_HASH_22 : hash :=
  (hash_from_bits 0xFEB3C337D7A51A6FBF00B9E34C52E1C9195C969BD4E7A0BFD51D5C5BED9C1167#256)

def SSZ_ZERO_HASH_23 : hash :=
  (hash_from_bits 0xE71F0AA83CC32EDFBEFA9F4D3E0174CA85182EEC9F3A09F6A6C0DF6377A510D7#256)

def SSZ_ZERO_HASH_24 : hash :=
  (hash_from_bits 0x31206FA80A50BB6ABE29085058F16212212A60EEC8F049FECB92D8C8E0A84BC0#256)

def SSZ_ZERO_HASH_25 : hash :=
  (hash_from_bits 0x21352BFECBEDDDE993839F614C3DAC0A3EE37543F9B412B16199DC158E23B544#256)

/-- The root of an all-zero subtree at `level`. -/
/- Type quantifiers: level : Nat, 0 ≤ level ∧ level ≤ 25 -/
def ssz_zero_hash (level : Nat) : (Vector (BitVec 8) 32) :=
  match level with
  | 0 => ZERO_HASH
  | 1 => SSZ_ZERO_HASH_01
  | 2 => SSZ_ZERO_HASH_02
  | 3 => SSZ_ZERO_HASH_03
  | 4 => SSZ_ZERO_HASH_04
  | 5 => SSZ_ZERO_HASH_05
  | 6 => SSZ_ZERO_HASH_06
  | 7 => SSZ_ZERO_HASH_07
  | 8 => SSZ_ZERO_HASH_08
  | 9 => SSZ_ZERO_HASH_09
  | 10 => SSZ_ZERO_HASH_10
  | 11 => SSZ_ZERO_HASH_11
  | 12 => SSZ_ZERO_HASH_12
  | 13 => SSZ_ZERO_HASH_13
  | 14 => SSZ_ZERO_HASH_14
  | 15 => SSZ_ZERO_HASH_15
  | 16 => SSZ_ZERO_HASH_16
  | 17 => SSZ_ZERO_HASH_17
  | 18 => SSZ_ZERO_HASH_18
  | 19 => SSZ_ZERO_HASH_19
  | 20 => SSZ_ZERO_HASH_20
  | 21 => SSZ_ZERO_HASH_21
  | 22 => SSZ_ZERO_HASH_22
  | 23 => SSZ_ZERO_HASH_23
  | 24 => SSZ_ZERO_HASH_24
  | _ => SSZ_ZERO_HASH_25

/-- Number of leaves in a complete supported SSZ tree at `depth`. -/
/- Type quantifiers: depth : Nat, 0 ≤ depth ∧ depth ≤ 25 -/
def htr_leaf_capacity (depth : Nat) : Nat :=
  match depth with
  | 0 => 1
  | 1 => 2
  | 2 => 4
  | 3 => 8
  | 4 => 16
  | 5 => 32
  | 6 => 64
  | 7 => 128
  | 8 => 256
  | 9 => 512
  | 10 => 1024
  | 11 => 2048
  | 12 => 4096
  | 13 => 8192
  | 14 => 16384
  | 15 => 32768
  | 16 => 65536
  | 17 => 131072
  | 18 => 262144
  | 19 => 524288
  | 20 => 1048576
  | 21 => 2097152
  | 22 => 4194304
  | 23 => 8388608
  | 24 => 16777216
  | _ => 33554432

/- Type quantifiers: _reclimit : Nat, depth : Nat, count : Nat, k_n : Nat, start : Nat, 1 ≤ k_n
  ∧ k_n ≤ 32 ∧ 0 ≤ start, 0 ≤ count ∧ count ≤ 33554432, 0 ≤ depth ∧ depth ≤ 25, 0
  ≤ _reclimit -/
def _rec_merkleize_vector_subtree (leaves : (Vector (Vector (BitVec 8) 32) k_n)) (start : Nat) (count : Nat) (depth : Nat) (_reclimit : Nat) : SailM (Vector (BitVec 8) 32) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((count == 0) : Bool)
      then (pure (ssz_zero_hash depth))
      else
        (do
          if ((depth == 0) : Bool)
          then
            (do
              let leaves_length := (Vector.length leaves)
              if (((count == 1) && (start <b leaves_length)) : Bool)
              then (pure (GetElem?.getElem! leaves ((leaves_length - 1) - start)))
              else (fatal_error WitnessDeficient))
          else
            (do
              let child_depth : Nat := (depth - 1)
              let half := (htr_leaf_capacity child_depth)
              let left_count : Nat :=
                if ((count <b half) : Bool)
                then count
                else half
              let right_count : Nat :=
                if ((count <b half) : Bool)
                then 0
                else (count - half)
              let left_root ← do
                (_rec_merkleize_vector_subtree leaves start left_count child_depth _reclimit_pred)
              let right_root ← do
                (_rec_merkleize_vector_subtree leaves (start + half) right_count child_depth
                  _reclimit_pred)
              (sha256_pair left_root right_root))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: depth : Nat, count : Nat, k_n : Nat, start : Nat, 1 ≤ k_n ∧
  k_n ≤ 32 ∧ 0 ≤ start, 0 ≤ count ∧ count ≤ 33554432, 0 ≤ depth ∧ depth ≤ 25 -/
def merkleize_vector_subtree (leaves : (Vector (Vector (BitVec 8) 32) k_n)) (start : Nat) (count : Nat) (depth : Nat) : SailM (Vector (BitVec 8) 32) := do
  let _measure := (depth : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_merkleize_vector_subtree leaves start count depth (_measure + 1))

/- Type quantifiers: depth : Nat, k_n : Nat, 1 ≤ k_n ∧ k_n ≤ 32, 0 ≤ depth ∧ depth ≤ 25 -/
def merkleize (leaves : (Vector (Vector (BitVec 8) 32) k_n)) (depth : Nat) : SailM (Vector (BitVec 8) 32) := do
  let count : Nat := (Vector.length leaves)
  let capacity := (htr_leaf_capacity depth)
  if ((count ≤b capacity) : Bool)
  then (merkleize_vector_subtree leaves 0 count depth)
  else (fatal_error WitnessDeficient)

/-- Encodes a protocol quantity as an SSZ little-endian basic-value chunk. -/
/- Type quantifiers: v : Nat, 0 ≤ v ∧ v ≤ (2 ^ 64 - 1) -/
def htr_uint (v : Nat) : (Vector (BitVec 8) 32) :=
  let bytes := ZERO_HASH
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 0 (get_slice_int 8 v 0))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 1 (get_slice_int 8 v 8))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 2 (get_slice_int 8 v 16))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 3 (get_slice_int 8 v 24))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 4 (get_slice_int 8 v 32))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 5 (get_slice_int 8 v 40))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 6 (get_slice_int 8 v 48))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 7 (get_slice_int 8 v 56))
  (B256 bytes)

/-- `uint256` as a 32-byte little-endian chunk. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def htr_u256 (value : Nat) : (Vector (BitVec 8) 32) :=
  let bytes := ZERO_HASH
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 0 (get_slice_int 8 value 0))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 1 (get_slice_int 8 value 8))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 2 (get_slice_int 8 value 16))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 3 (get_slice_int 8 value 24))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 4 (get_slice_int 8 value 32))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 5 (get_slice_int 8 value 40))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 6 (get_slice_int 8 value 48))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 7 (get_slice_int 8 value 56))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 8 (get_slice_int 8 value 64))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 9 (get_slice_int 8 value 72))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 10 (get_slice_int 8 value 80))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 11 (get_slice_int 8 value 88))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 12 (get_slice_int 8 value 96))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 13 (get_slice_int 8 value 104))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 14 (get_slice_int 8 value 112))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 15 (get_slice_int 8 value 120))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 16 (get_slice_int 8 value 128))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 17 (get_slice_int 8 value 136))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 18 (get_slice_int 8 value 144))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 19 (get_slice_int 8 value 152))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 20 (get_slice_int 8 value 160))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 21 (get_slice_int 8 value 168))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 22 (get_slice_int 8 value 176))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 23 (get_slice_int 8 value 184))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 24 (get_slice_int 8 value 192))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 25 (get_slice_int 8 value 200))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 26 (get_slice_int 8 value 208))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 27 (get_slice_int 8 value 216))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 28 (get_slice_int 8 value 224))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 29 (get_slice_int 8 value 232))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 30 (get_slice_int 8 value 240))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 31 (get_slice_int 8 value 248))
  (B256 bytes)

/-- `Bytes32` chunks are themselves. -/
def htr_bytes32 (b : (Vector (BitVec 8) 32)) : (Vector (BitVec 8) 32) :=
  b

/-- A 20-byte address, left-aligned in its chunk. -/
def htr_addr (address_bytes : (Vector (BitVec 8) 20)) : (Vector (BitVec 8) 32) :=
  let bytes := ZERO_HASH
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 0 (GetElem?.getElem! address_bytes 0))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 1 (GetElem?.getElem! address_bytes 1))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 2 (GetElem?.getElem! address_bytes 2))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 3 (GetElem?.getElem! address_bytes 3))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 4 (GetElem?.getElem! address_bytes 4))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 5 (GetElem?.getElem! address_bytes 5))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 6 (GetElem?.getElem! address_bytes 6))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 7 (GetElem?.getElem! address_bytes 7))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 8 (GetElem?.getElem! address_bytes 8))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 9 (GetElem?.getElem! address_bytes 9))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 10 (GetElem?.getElem! address_bytes 10))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 11 (GetElem?.getElem! address_bytes 11))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 12 (GetElem?.getElem! address_bytes 12))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 13 (GetElem?.getElem! address_bytes 13))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 14 (GetElem?.getElem! address_bytes 14))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 15 (GetElem?.getElem! address_bytes 15))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 16 (GetElem?.getElem! address_bytes 16))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 17 (GetElem?.getElem! address_bytes 17))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 18 (GetElem?.getElem! address_bytes 18))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 19 (GetElem?.getElem! address_bytes 19))
  (B256 bytes)

/- Type quantifiers: len : Nat, (source_valid_length len) -/
def htr_length_chunk (len : Nat) : SailM (Vector (BitVec 8) 32) := do
  let length_word ← do (word_of_source_byte_count len)
  (pure (htr_u256 length_word))

/- Type quantifiers: len : Nat, (source_valid_length len) -/
def mix_in_length (root : (Vector (BitVec 8) 32)) (len : Nat) : SailM (Vector (BitVec 8) 32) := do
  let length_chunk ← do (htr_length_chunk len)
  (sha256_pair root length_chunk)

/-- The ceiling of log2 — the Merkle depth of an `n`-chunk capacity. -/
/- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ 33554432 -/
def clog2 (n : Nat) : SailM Nat := do
  let remaining : Nat :=
    if ((n == 0) : Bool)
    then 0
    else (n - 1)
  let depth : Nat := 0
  let (depth, remaining) ← (( do
    let loop__step_lower := 0
    let loop__step_upper := 24
    let mut loop_vars := (depth, remaining)
    for _step in [loop__step_lower:loop__step_upper:1]i do
      let (depth, remaining) := loop_vars
      loop_vars ← do
        let (depth, remaining) ← (( do
          if ((remaining != 0) : Bool)
          then
            (do
              let current_depth := depth
              let depth ←
                if ((current_depth <b 25) : Bool)
                then (pure (current_depth + 1))
                else (fatal_error WitnessDeficient)
              let remaining : Nat := (Nat.div remaining 2)
              (pure (depth, remaining)))
          else (pure (depth, remaining)) ) : SailM (Nat × Nat) )
        (pure (depth, remaining))
    (pure loop_vars) ) : SailM (Nat × Nat) )
  (pure depth)

/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, chunk_index : Nat, 0
  ≤ chunk_index, 0 ≤ bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def htr_chunk (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (chunk_index : Nat) : SailM (Vector (BitVec 8) 32) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  if ((chunk_index ≤b 33554432) : Bool)
  then
    (do
      let offset : Nat := (chunk_index *i WORD_BYTE_LENGTH)
      let chunk_word ← do (stateless_input_slice_load ⟨_, ⟨_, bytes⟩⟩ offset)
      (pure (word_to_hash chunk_word)))
  else (fatal_error WitnessDeficient)

/- Type quantifiers: byte_len : Nat, (source_valid_length byte_len) -/
def htr_chunk_count (byte_len : Nat) : SailM Nat := do
  if ((byte_len ≤b HTR_BYTE_LIST_LIMIT) : Bool)
  then (pure ((byte_len + 31) / 32))
  else (fatal_error WitnessDeficient)

/- Type quantifiers: _reclimit : Nat, depth : Nat, count : Nat, bytes_dependentWitness1 : Nat, bytes_dependentWitness0
  : Nat, start : Nat, 0 ≤ start, 0 ≤ bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ count ∧
  count ≤ 33554432, 0 ≤ depth ∧ depth ≤ 25, 0 ≤ _reclimit -/
def _rec_htr_bytes_subtree (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (start : Nat) (count : Nat) (depth : Nat) (_reclimit : Nat) : SailM (Vector (BitVec 8) 32) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((count == 0) : Bool)
      then (pure (ssz_zero_hash depth))
      else
        (do
          if ((depth == 0) : Bool)
          then
            (do
              if ((count == 1) : Bool)
              then (htr_chunk ⟨_, ⟨_, bytes⟩⟩ start)
              else (fatal_error WitnessDeficient))
          else
            (do
              let child_depth : Nat := (depth - 1)
              let half := (htr_leaf_capacity child_depth)
              let left_count : Nat :=
                if ((count <b half) : Bool)
                then count
                else half
              let right_count : Nat :=
                if ((count <b half) : Bool)
                then 0
                else (count - half)
              let left_root ← do
                (_rec_htr_bytes_subtree ⟨_, ⟨_, bytes⟩⟩ start left_count child_depth
                  _reclimit_pred)
              let right_root ← do
                (_rec_htr_bytes_subtree ⟨_, ⟨_, bytes⟩⟩ (start + half) right_count
                  child_depth _reclimit_pred)
              (sha256_pair left_root right_root))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: depth : Nat, count : Nat, bytes_dependentWitness1 : Nat, bytes_dependentWitness0
  : Nat, start : Nat, 0 ≤ start, 0 ≤ bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ count ∧
  count ≤ 33554432, 0 ≤ depth ∧ depth ≤ 25 -/
def htr_bytes_subtree (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (start : Nat) (count : Nat) (depth : Nat) : SailM (Vector (BitVec 8) 32) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let _measure := (depth : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_htr_bytes_subtree ⟨_, ⟨_, bytes⟩⟩ start count depth (_measure + 1))

/-- The Merkle root of a byte span's chunks at `depth`. -/
/- Type quantifiers: k_ex554935_ : Nat, bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, 0
  ≤ bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ k_ex554935_ ∧
  k_ex554935_ ≤ 25 -/
def htr_bytes_root (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (depth : Nat) : SailM (Vector (BitVec 8) 32) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let count ← do (htr_chunk_count bytes.len)
  let capacity := (htr_leaf_capacity depth)
  if ((count ≤b capacity) : Bool)
  then (htr_bytes_subtree ⟨_, ⟨_, bytes⟩⟩ 0 count depth)
  else (fatal_error WitnessDeficient)

/-- `hash_tree_root` of a fixed-size `ByteVector`. -/
/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, 0 ≤
  bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def htr_bytevector (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let chunks ← do (htr_chunk_count bytes.len)
  let depth ← do (clog2 chunks)
  (htr_bytes_root ⟨_, ⟨_, bytes⟩⟩ depth)

/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, offset : Nat, (source_valid_range offset 8), 0
  ≤ bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def htr_ssz_uint (bytes : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_syn_off k_len)))) (offset : Nat) : SailM (Vector (BitVec 8) 32) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let value ← do (decode_ssz_uint ⟨_, ⟨_, bytes⟩⟩ offset)
  (pure (htr_uint value))

/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, offset : Nat, (source_valid_range offset 32), 0
  ≤ bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def htr_ssz_u256 (bytes : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_syn_off k_len)))) (offset : Nat) : SailM (Vector (BitVec 8) 32) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let value ← do (ssz_u256 ⟨_, ⟨_, bytes⟩⟩ offset)
  (pure (htr_u256 value))

/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, offset : Nat, (source_valid_range offset 32), 0
  ≤ bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def htr_ssz_bytes32 (bytes : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_syn_off k_len)))) (offset : Nat) : SailM (Vector (BitVec 8) 32) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let value ← do (ssz_bytes32 ⟨_, ⟨_, bytes⟩⟩ offset)
  (pure (htr_bytes32 value))

/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, offset : Nat, (source_valid_range offset 20), 0
  ≤ bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def htr_ssz_addr (bytes : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_syn_off k_len)))) (offset : Nat) : SailM (Vector (BitVec 8) 32) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let value ← do (ssz_addr ⟨_, ⟨_, bytes⟩⟩ offset)
  (pure (htr_addr value))

/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, offset : Nat, len
  : Nat, (source_valid_range offset len), 0 ≤ bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def htr_ssz_bytevector (bytes : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_syn_len : Nat) => (StatelessInputSliceFields k_syn_off k_syn_len)))) (offset : Nat) (len : Nat) : SailM (Vector (BitVec 8) 32) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  if (((offset + len) ≤b bytes.len) : Bool)
  then
    (do
      let field := (stateless_input_sub_slice bytes offset len)
      (htr_bytevector ⟨_, ⟨_, field⟩⟩))
  else (fatal_error InvalidConfig)

/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, limit_bytes : Nat, (source_valid_length limit_bytes), 0
  ≤ bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def htr_bytelist (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (limit_bytes : Nat) : SailM (Vector (BitVec 8) 32) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let capacity ← do (htr_chunk_count limit_bytes)
  let depth ← do (clog2 capacity)
  let root ← do (htr_bytes_root ⟨_, ⟨_, bytes⟩⟩ depth)
  (mix_in_length root bytes.len)

/-- `hash_tree_root` of one SSZ withdrawal (4 fields, depth 2). -/
/- Type quantifiers: withdrawal_dependentWitness1 : Nat, withdrawal_dependentWitness0 : Nat, 0 ≤
  withdrawal_dependentWitness0 ∧
  0 ≤ withdrawal_dependentWitness1 ∧
  (withdrawal_dependentWitness0 + withdrawal_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  withdrawal_dependentWitness1 = 44 -/
def htr_withdrawal (withdrawal : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let withdrawal_dependentWitness0 := (withdrawal).1
  let withdrawal_dependentWitness1 := ((withdrawal).2).1
  let withdrawal := ((withdrawal).2).2
  let index_root ← do (htr_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_INDEX)
  let validator_root ← do (htr_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_VALIDATOR_INDEX)
  let address_root ← do (htr_ssz_addr ⟨_, ⟨_, withdrawal⟩⟩ WD_ADDRESS)
  let amount_root ← do (htr_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_AMOUNT)
  (merkleize #v[amount_root, address_root, validator_root, index_root] 2)

/- Type quantifiers: _reclimit : Nat, depth : Nat, count : Nat, start : Nat, 0 ≤ start, 0 ≤
  count ∧ count ≤ 33554432, 0 ≤ depth ∧ depth ≤ 25, 0 ≤ _reclimit -/
def _rec_htr_transactions_subtree (transactions : (BoundedSszListRef (2 ^ 20))) (start : Nat) (count : Nat) (depth : Nat) (_reclimit : Nat) : SailM (Vector (BitVec 8) 32) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((count == 0) : Bool)
      then (pure (ssz_zero_hash depth))
      else
        (do
          if ((depth == 0) : Bool)
          then
            (do
              if ((count == 1) : Bool)
              then
                (do
                  let ⟨_, ⟨_, transaction⟩⟩ ← do (ssz_list_at transactions start)
                  (htr_bytelist ⟨_, ⟨_, transaction⟩⟩ HTR_BYTE_LIST_LIMIT))
              else (fatal_error WitnessDeficient))
          else
            (do
              let child_depth : Nat := (depth - 1)
              let half := (htr_leaf_capacity child_depth)
              let left_count : Nat :=
                if ((count <b half) : Bool)
                then count
                else half
              let right_count : Nat :=
                if ((count <b half) : Bool)
                then 0
                else (count - half)
              let left_root ← do
                (_rec_htr_transactions_subtree transactions start left_count child_depth
                  _reclimit_pred)
              let right_root ← do
                (_rec_htr_transactions_subtree transactions (start + half) right_count child_depth
                  _reclimit_pred)
              (sha256_pair left_root right_root))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: depth : Nat, count : Nat, start : Nat, 0 ≤ start, 0 ≤ count ∧
  count ≤ 33554432, 0 ≤ depth ∧ depth ≤ 25 -/
def htr_transactions_subtree (transactions : (BoundedSszListRef (2 ^ 20))) (start : Nat) (count : Nat) (depth : Nat) : SailM (Vector (BitVec 8) 32) := do
  let _measure := (depth : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_htr_transactions_subtree transactions start count depth (_measure + 1))

/-- `hash_tree_root` of the transactions list
(`List[ByteList[2^30], 2^20]`). -/
def htr_transactions (transactions : (BoundedSszListRef (2 ^ 20))) : SailM (Vector (BitVec 8) 32) := do
  let count : Nat := transactions.count
  let root ← do (htr_transactions_subtree transactions 0 count 20)
  (mix_in_length root transactions.count)

/- Type quantifiers: _reclimit : Nat, depth : Nat, count : Nat, start : Nat, 0 ≤ start, 0 ≤
  count ∧ count ≤ 33554432, 0 ≤ depth ∧ depth ≤ 25, 0 ≤ _reclimit -/
def _rec_htr_withdrawals_subtree (withdrawals : (BoundedSszListRef (2 ^ 4))) (start : Nat) (count : Nat) (depth : Nat) (_reclimit : Nat) : SailM (Vector (BitVec 8) 32) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((count == 0) : Bool)
      then (pure (ssz_zero_hash depth))
      else
        (do
          if ((depth == 0) : Bool)
          then
            (do
              if ((count == 1) : Bool)
              then
                (do
                  let ⟨_, ⟨_, withdrawal⟩⟩ ← do
                    (ssz_fixed_list_at withdrawals start WD_SIZE)
                  (htr_withdrawal ⟨_, ⟨_, withdrawal⟩⟩))
              else (fatal_error WitnessDeficient))
          else
            (do
              let child_depth : Nat := (depth - 1)
              let half := (htr_leaf_capacity child_depth)
              let left_count : Nat :=
                if ((count <b half) : Bool)
                then count
                else half
              let right_count : Nat :=
                if ((count <b half) : Bool)
                then 0
                else (count - half)
              let left_root ← do
                (_rec_htr_withdrawals_subtree withdrawals start left_count child_depth
                  _reclimit_pred)
              let right_root ← do
                (_rec_htr_withdrawals_subtree withdrawals (start + half) right_count child_depth
                  _reclimit_pred)
              (sha256_pair left_root right_root))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: depth : Nat, count : Nat, start : Nat, 0 ≤ start, 0 ≤ count ∧
  count ≤ 33554432, 0 ≤ depth ∧ depth ≤ 25 -/
def htr_withdrawals_subtree (withdrawals : (BoundedSszListRef (2 ^ 4))) (start : Nat) (count : Nat) (depth : Nat) : SailM (Vector (BitVec 8) 32) := do
  let _measure := (depth : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_htr_withdrawals_subtree withdrawals start count depth (_measure + 1))

/-- `hash_tree_root` of the withdrawals list (`List[SszWithdrawal, 16]`). -/
def htr_withdrawals (withdrawals : (BoundedSszListRef (2 ^ 4))) : SailM (Vector (BitVec 8) 32) := do
  let count : Nat := withdrawals.count
  let root ← do (htr_withdrawals_subtree withdrawals 0 count 4)
  (mix_in_length root withdrawals.count)

/-- `hash_tree_root` of the `SszExecutionPayload` (19 fields, depth 5). -/
def htr_execution_payload (input_ref : StatelessInputRef) : SailM (Vector (BitVec 8) 32) := do
  let ⟨_, ⟨_, payload⟩⟩ := input_ref.execution_payload
  let parent_hash_root ← do (htr_ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ 0)
  let fee_recipient_root ← do (htr_ssz_addr ⟨_, ⟨_, payload⟩⟩ PL_FEE_RECIPIENT)
  let state_root ← do (htr_ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ PL_STATE_ROOT)
  let receipts_root ← do (htr_ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ PL_RECEIPTS_ROOT)
  let logs_bloom_root ← do
    (htr_ssz_bytevector ⟨_, ⟨_, payload⟩⟩ PL_LOGS_BLOOM LOGS_BLOOM_BYTE_LENGTH)
  let prev_randao_root ← do (htr_ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ PL_PREV_RANDAO)
  let block_number_root ← do (htr_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_BLOCK_NUMBER)
  let gas_limit_root ← do (htr_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_GAS_LIMIT)
  let gas_used_root ← do (htr_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_GAS_USED)
  let timestamp_root ← do (htr_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_TIMESTAMP)
  let extra_data_root ← do (htr_bytelist input_ref.extra_data WORD_BYTE_LENGTH)
  let base_fee_root ← do (htr_ssz_u256 ⟨_, ⟨_, payload⟩⟩ PL_BASE_FEE)
  let block_hash_root ← do (htr_ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ PL_BLOCK_HASH)
  let transactions_root ← do (htr_transactions input_ref.transactions)
  let withdrawals_root ← do (htr_withdrawals input_ref.withdrawals)
  let blob_gas_used_root ← do (htr_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_BLOB_GAS_USED)
  let excess_blob_gas_root ← do (htr_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_EXCESS_BLOB_GAS)
  let block_access_list_root ← do (htr_bytelist input_ref.block_access_list HTR_BYTE_LIST_LIMIT)
  let slot_number_root ← do (htr_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_SLOT_NUMBER)
  (merkleize
    #v[slot_number_root, block_access_list_root, excess_blob_gas_root, blob_gas_used_root, withdrawals_root, transactions_root, block_hash_root, base_fee_root, extra_data_root, timestamp_root, gas_used_root, gas_limit_root, block_number_root, prev_randao_root, logs_bloom_root, receipts_root, state_root, fee_recipient_root, parent_hash_root]
    5)

/- Type quantifiers: _reclimit : Nat, depth : Nat, count : Nat, versioned_hashes_dependentWitness1 :
  Nat, versioned_hashes_dependentWitness0 : Nat, start : Nat, 0 ≤ start, 0 ≤
  versioned_hashes_dependentWitness0 ∧
  0 ≤ versioned_hashes_dependentWitness1 ∧
  (versioned_hashes_dependentWitness0 + versioned_hashes_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤
  count ∧ count ≤ 33554432, 0 ≤ depth ∧ depth ≤ 25, 0 ≤ _reclimit -/
def _rec_htr_versioned_hashes_subtree (versioned_hashes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (start : Nat) (count : Nat) (depth : Nat) (_reclimit : Nat) : SailM (Vector (BitVec 8) 32) := do
  let versioned_hashes_dependentWitness0 := (versioned_hashes).1
  let versioned_hashes_dependentWitness1 := ((versioned_hashes).2).1
  let versioned_hashes := ((versioned_hashes).2).2
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((count == 0) : Bool)
      then (pure (ssz_zero_hash depth))
      else
        (do
          if ((depth == 0) : Bool)
          then
            (do
              if ((count == 1) : Bool)
              then (htr_chunk ⟨_, ⟨_, versioned_hashes⟩⟩ start)
              else (fatal_error WitnessDeficient))
          else
            (do
              let child_depth : Nat := (depth - 1)
              let half := (htr_leaf_capacity child_depth)
              let left_count : Nat :=
                if ((count <b half) : Bool)
                then count
                else half
              let right_count : Nat :=
                if ((count <b half) : Bool)
                then 0
                else (count - half)
              let left_root ← do
                (_rec_htr_versioned_hashes_subtree ⟨_, ⟨_, versioned_hashes⟩⟩ start
                  left_count child_depth _reclimit_pred)
              let right_root ← do
                (_rec_htr_versioned_hashes_subtree ⟨_, ⟨_, versioned_hashes⟩⟩ (start + half)
                  right_count child_depth _reclimit_pred)
              (sha256_pair left_root right_root))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: depth : Nat, count : Nat, versioned_hashes_dependentWitness1 : Nat, versioned_hashes_dependentWitness0
  : Nat, start : Nat, 0 ≤ start, 0 ≤ versioned_hashes_dependentWitness0 ∧
  0 ≤ versioned_hashes_dependentWitness1 ∧
  (versioned_hashes_dependentWitness0 + versioned_hashes_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤
  count ∧ count ≤ 33554432, 0 ≤ depth ∧ depth ≤ 25 -/
def htr_versioned_hashes_subtree (versioned_hashes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (start : Nat) (count : Nat) (depth : Nat) : SailM (Vector (BitVec 8) 32) := do
  let versioned_hashes_dependentWitness0 := (versioned_hashes).1
  let versioned_hashes_dependentWitness1 := ((versioned_hashes).2).1
  let versioned_hashes := ((versioned_hashes).2).2
  let _measure := (depth : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else
    (_rec_htr_versioned_hashes_subtree ⟨_, ⟨_, versioned_hashes⟩⟩ start count depth
      (_measure + 1))

/-- `hash_tree_root` of the versioned hashes (`List[Bytes32, 4096]`). -/
/- Type quantifiers: versioned_hashes_dependentWitness1 : Nat, versioned_hashes_dependentWitness0 :
  Nat, 0 ≤ versioned_hashes_dependentWitness0 ∧
  0 ≤ versioned_hashes_dependentWitness1 ∧
  (versioned_hashes_dependentWitness0 + versioned_hashes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def htr_versioned_hashes (versioned_hashes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let versioned_hashes_dependentWitness0 := (versioned_hashes).1
  let versioned_hashes_dependentWitness1 := ((versioned_hashes).2).1
  let versioned_hashes := ((versioned_hashes).2).2
  let items ← do
    (ssz_bounded_fixed_list_ref ⟨_, ⟨_, versioned_hashes⟩⟩ WORD_BYTE_LENGTH (2 ^i 12))
  let count : Nat := items.count
  let root ← do (htr_versioned_hashes_subtree ⟨_, ⟨_, versioned_hashes⟩⟩ 0 count 12)
  (mix_in_length root items.count)

/-- `hash_tree_root` of one deposit request (5 fields, depth 3). -/
/- Type quantifiers: deposit_dependentWitness1 : Nat, deposit_dependentWitness0 : Nat, 0 ≤
  deposit_dependentWitness0 ∧
  0 ≤ deposit_dependentWitness1 ∧
  (deposit_dependentWitness0 + deposit_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  deposit_dependentWitness1 = 192 -/
def htr_deposit (deposit : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let deposit_dependentWitness0 := (deposit).1
  let deposit_dependentWitness1 := ((deposit).2).1
  let deposit := ((deposit).2).2
  let pubkey ← do
    (htr_ssz_bytevector ⟨_, ⟨_, deposit⟩⟩ HTR_DEPOSIT_PUBKEY HTR_DEPOSIT_PUBKEY_LENGTH)
  let withdrawal_credentials ← do
    (htr_ssz_bytes32 ⟨_, ⟨_, deposit⟩⟩ HTR_DEPOSIT_WITHDRAWAL_CREDENTIALS)
  let amount ← do (htr_ssz_uint ⟨_, ⟨_, deposit⟩⟩ HTR_DEPOSIT_AMOUNT)
  let signature ← do
    (htr_ssz_bytevector ⟨_, ⟨_, deposit⟩⟩ HTR_DEPOSIT_SIGNATURE HTR_DEPOSIT_SIGNATURE_LENGTH)
  let index ← do (htr_ssz_uint ⟨_, ⟨_, deposit⟩⟩ HTR_DEPOSIT_INDEX)
  (merkleize #v[index, signature, amount, withdrawal_credentials, pubkey] 3)

/-- `hash_tree_root` of one withdrawal request (3 fields, depth 2). -/
/- Type quantifiers: request_dependentWitness1 : Nat, request_dependentWitness0 : Nat, 0 ≤
  request_dependentWitness0 ∧
  0 ≤ request_dependentWitness1 ∧
  (request_dependentWitness0 + request_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  request_dependentWitness1 = 76 -/
def htr_withdrawal_request (request : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let request_dependentWitness0 := (request).1
  let request_dependentWitness1 := ((request).2).1
  let request := ((request).2).2
  let source_address ← do (htr_ssz_addr ⟨_, ⟨_, request⟩⟩ HTR_REQUEST_SOURCE_ADDRESS)
  let source_pubkey ← do
    (htr_ssz_bytevector ⟨_, ⟨_, request⟩⟩ HTR_REQUEST_SOURCE_PUBKEY
      HTR_REQUEST_PUBKEY_LENGTH)
  let amount ← do (htr_ssz_uint ⟨_, ⟨_, request⟩⟩ HTR_WITHDRAWAL_REQUEST_AMOUNT)
  (merkleize #v[amount, source_pubkey, source_address] 2)

/-- `hash_tree_root` of one consolidation request (3 fields, depth 2). -/
/- Type quantifiers: request_dependentWitness1 : Nat, request_dependentWitness0 : Nat, 0 ≤
  request_dependentWitness0 ∧
  0 ≤ request_dependentWitness1 ∧
  (request_dependentWitness0 + request_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  request_dependentWitness1 = 116 -/
def htr_consolidation_request (request : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let request_dependentWitness0 := (request).1
  let request_dependentWitness1 := ((request).2).1
  let request := ((request).2).2
  let source_address ← do (htr_ssz_addr ⟨_, ⟨_, request⟩⟩ HTR_REQUEST_SOURCE_ADDRESS)
  let source_pubkey ← do
    (htr_ssz_bytevector ⟨_, ⟨_, request⟩⟩ HTR_REQUEST_SOURCE_PUBKEY
      HTR_REQUEST_PUBKEY_LENGTH)
  let target_pubkey ← do
    (htr_ssz_bytevector ⟨_, ⟨_, request⟩⟩ HTR_CONSOLIDATION_REQUEST_TARGET_PUBKEY
      HTR_REQUEST_PUBKEY_LENGTH)
  (merkleize #v[target_pubkey, source_pubkey, source_address] 2)

/-- `hash_tree_root` of one builder deposit request (4 fields, depth 2). -/
/- Type quantifiers: request_dependentWitness1 : Nat, request_dependentWitness0 : Nat, 0 ≤
  request_dependentWitness0 ∧
  0 ≤ request_dependentWitness1 ∧
  (request_dependentWitness0 + request_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  request_dependentWitness1 = 184 -/
def htr_builder_deposit_request (request : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let request_dependentWitness0 := (request).1
  let request_dependentWitness1 := ((request).2).1
  let request := ((request).2).2
  let pubkey ← do
    (htr_ssz_bytevector ⟨_, ⟨_, request⟩⟩ HTR_BUILDER_DEPOSIT_PUBKEY
      HTR_REQUEST_PUBKEY_LENGTH)
  let withdrawal_credentials ← do
    (htr_ssz_bytes32 ⟨_, ⟨_, request⟩⟩ HTR_BUILDER_DEPOSIT_WITHDRAWAL_CREDENTIALS)
  let amount ← do (htr_ssz_uint ⟨_, ⟨_, request⟩⟩ HTR_BUILDER_DEPOSIT_AMOUNT)
  let signature ← do
    (htr_ssz_bytevector ⟨_, ⟨_, request⟩⟩ HTR_BUILDER_DEPOSIT_SIGNATURE
      HTR_DEPOSIT_SIGNATURE_LENGTH)
  (merkleize #v[signature, amount, withdrawal_credentials, pubkey] 2)

/-- `hash_tree_root` of one builder exit request (2 fields, depth 1). -/
/- Type quantifiers: request_dependentWitness1 : Nat, request_dependentWitness0 : Nat, 0 ≤
  request_dependentWitness0 ∧
  0 ≤ request_dependentWitness1 ∧
  (request_dependentWitness0 + request_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  request_dependentWitness1 = 68 -/
def htr_builder_exit_request (request : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let request_dependentWitness0 := (request).1
  let request_dependentWitness1 := ((request).2).1
  let request := ((request).2).2
  let source_address ← do (htr_ssz_addr ⟨_, ⟨_, request⟩⟩ HTR_BUILDER_EXIT_SOURCE_ADDRESS)
  let pubkey ← do
    (htr_ssz_bytevector ⟨_, ⟨_, request⟩⟩ HTR_BUILDER_EXIT_PUBKEY HTR_REQUEST_PUBKEY_LENGTH)
  (merkleize #v[pubkey, source_address] 1)

def undefined_HtrRequestKind (_ : Unit) : SailM HtrRequestKind := do
  (internal_pick
    [HtrDeposit, HtrWithdrawalRequest, HtrConsolidationRequest, HtrBuilderDepositRequest, HtrBuilderExitRequest])

/- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 4 -/
def HtrRequestKind_of_num (arg_ : Nat) : HtrRequestKind :=
  match arg_ with
  | 0 => HtrDeposit
  | 1 => HtrWithdrawalRequest
  | 2 => HtrConsolidationRequest
  | 3 => HtrBuilderDepositRequest
  | _ => HtrBuilderExitRequest

def num_of_HtrRequestKind (arg_ : HtrRequestKind) : Nat :=
  match arg_ with
  | .HtrDeposit => 0
  | .HtrWithdrawalRequest => 1
  | .HtrConsolidationRequest => 2
  | .HtrBuilderDepositRequest => 3
  | .HtrBuilderExitRequest => 4

/- Type quantifiers: requests_dependentWitness1 : Nat, requests_dependentWitness0 : Nat, index : Nat, 0
  ≤ index, 0 ≤ requests_dependentWitness0 ∧
  0 ≤ requests_dependentWitness1 ∧
  (requests_dependentWitness0 + requests_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def htr_request_leaf (requests : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (index : Nat) (kind : HtrRequestKind) : SailM (Vector (BitVec 8) 32) := do
  let requests_dependentWitness0 := (requests).1
  let requests_dependentWitness1 := ((requests).2).1
  let requests := ((requests).2).2
  match kind with
  | .HtrDeposit =>
    (do
      let offset := (index *i HTR_DEPOSIT_LENGTH)
      if (((offset + HTR_DEPOSIT_LENGTH) ≤b requests.len) : Bool)
      then
        (do
          let deposit := (stateless_input_sub_slice requests offset HTR_DEPOSIT_LENGTH)
          (htr_deposit ⟨_, ⟨_, deposit⟩⟩))
      else (fatal_error InvalidConfig))
  | .HtrWithdrawalRequest =>
    (do
      let offset := (index *i HTR_WITHDRAWAL_REQUEST_LENGTH)
      if (((offset + HTR_WITHDRAWAL_REQUEST_LENGTH) ≤b requests.len) : Bool)
      then
        (do
          let request := (stateless_input_sub_slice requests offset HTR_WITHDRAWAL_REQUEST_LENGTH)
          (htr_withdrawal_request ⟨_, ⟨_, request⟩⟩))
      else (fatal_error InvalidConfig))
  | .HtrConsolidationRequest =>
    (do
      let offset := (index *i HTR_CONSOLIDATION_REQUEST_LENGTH)
      if (((offset + HTR_CONSOLIDATION_REQUEST_LENGTH) ≤b requests.len) : Bool)
      then
        (do
          let request :=
            (stateless_input_sub_slice requests offset HTR_CONSOLIDATION_REQUEST_LENGTH)
          (htr_consolidation_request ⟨_, ⟨_, request⟩⟩))
      else (fatal_error InvalidConfig))
  | .HtrBuilderDepositRequest =>
    (do
      let offset := (index *i HTR_BUILDER_DEPOSIT_LENGTH)
      if (((offset + HTR_BUILDER_DEPOSIT_LENGTH) ≤b requests.len) : Bool)
      then
        (do
          let request := (stateless_input_sub_slice requests offset HTR_BUILDER_DEPOSIT_LENGTH)
          (htr_builder_deposit_request ⟨_, ⟨_, request⟩⟩))
      else (fatal_error InvalidConfig))
  | .HtrBuilderExitRequest =>
    (do
      let offset := (index *i HTR_BUILDER_EXIT_LENGTH)
      if (((offset + HTR_BUILDER_EXIT_LENGTH) ≤b requests.len) : Bool)
      then
        (do
          let request := (stateless_input_sub_slice requests offset HTR_BUILDER_EXIT_LENGTH)
          (htr_builder_exit_request ⟨_, ⟨_, request⟩⟩))
      else (fatal_error InvalidConfig))

/- Type quantifiers: _reclimit : Nat, depth : Nat, count : Nat, requests_dependentWitness1 : Nat, requests_dependentWitness0
  : Nat, start : Nat, 0 ≤ start, 0 ≤ requests_dependentWitness0 ∧
  0 ≤ requests_dependentWitness1 ∧
  (requests_dependentWitness0 + requests_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ count ∧
  count ≤ 33554432, 0 ≤ depth ∧ depth ≤ 25, 0 ≤ _reclimit -/
def _rec_htr_requests_subtree (requests : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (kind : HtrRequestKind) (start : Nat) (count : Nat) (depth : Nat) (_reclimit : Nat) : SailM (Vector (BitVec 8) 32) := do
  let requests_dependentWitness0 := (requests).1
  let requests_dependentWitness1 := ((requests).2).1
  let requests := ((requests).2).2
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((count == 0) : Bool)
      then (pure (ssz_zero_hash depth))
      else
        (do
          if ((depth == 0) : Bool)
          then
            (do
              if ((count == 1) : Bool)
              then (htr_request_leaf ⟨_, ⟨_, requests⟩⟩ start kind)
              else (fatal_error WitnessDeficient))
          else
            (do
              let child_depth : Nat := (depth - 1)
              let half := (htr_leaf_capacity child_depth)
              let left_count : Nat :=
                if ((count <b half) : Bool)
                then count
                else half
              let right_count : Nat :=
                if ((count <b half) : Bool)
                then 0
                else (count - half)
              let left_root ← do
                (_rec_htr_requests_subtree ⟨_, ⟨_, requests⟩⟩ kind start left_count
                  child_depth _reclimit_pred)
              let right_root ← do
                (_rec_htr_requests_subtree ⟨_, ⟨_, requests⟩⟩ kind (start + half)
                  right_count child_depth _reclimit_pred)
              (sha256_pair left_root right_root))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: depth : Nat, count : Nat, requests_dependentWitness1 : Nat, requests_dependentWitness0
  : Nat, start : Nat, 0 ≤ start, 0 ≤ requests_dependentWitness0 ∧
  0 ≤ requests_dependentWitness1 ∧
  (requests_dependentWitness0 + requests_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ count ∧
  count ≤ 33554432, 0 ≤ depth ∧ depth ≤ 25 -/
def htr_requests_subtree (requests : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (kind : HtrRequestKind) (start : Nat) (count : Nat) (depth : Nat) : SailM (Vector (BitVec 8) 32) := do
  let requests_dependentWitness0 := (requests).1
  let requests_dependentWitness1 := ((requests).2).1
  let requests := ((requests).2).2
  let _measure := (depth : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_htr_requests_subtree ⟨_, ⟨_, requests⟩⟩ kind start count depth (_measure + 1))

/-- `hash_tree_root` of the deposit-request list (depth 13). -/
/- Type quantifiers: deposits_dependentWitness1 : Nat, deposits_dependentWitness0 : Nat, 0 ≤
  deposits_dependentWitness0 ∧
  0 ≤ deposits_dependentWitness1 ∧
  (deposits_dependentWitness0 + deposits_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def htr_deposits (deposits : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let deposits_dependentWitness0 := (deposits).1
  let deposits_dependentWitness1 := ((deposits).2).1
  let deposits := ((deposits).2).2
  let items ← do
    (ssz_bounded_fixed_list_ref ⟨_, ⟨_, deposits⟩⟩ HTR_DEPOSIT_LENGTH (2 ^i 13))
  let count : Nat := items.count
  let root ← do (htr_requests_subtree ⟨_, ⟨_, deposits⟩⟩ HtrDeposit 0 count 13)
  (mix_in_length root items.count)

/-- `hash_tree_root` of the withdrawal-request list (depth 4). -/
/- Type quantifiers: requests_dependentWitness1 : Nat, requests_dependentWitness0 : Nat, 0 ≤
  requests_dependentWitness0 ∧
  0 ≤ requests_dependentWitness1 ∧
  (requests_dependentWitness0 + requests_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def htr_withdrawal_requests (requests : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let requests_dependentWitness0 := (requests).1
  let requests_dependentWitness1 := ((requests).2).1
  let requests := ((requests).2).2
  let items ← do
    (ssz_bounded_fixed_list_ref ⟨_, ⟨_, requests⟩⟩ HTR_WITHDRAWAL_REQUEST_LENGTH (2 ^i 4))
  let count : Nat := items.count
  let root ← do (htr_requests_subtree ⟨_, ⟨_, requests⟩⟩ HtrWithdrawalRequest 0 count 4)
  (mix_in_length root items.count)

/-- `hash_tree_root` of the consolidation-request list (depth 1). -/
/- Type quantifiers: requests_dependentWitness1 : Nat, requests_dependentWitness0 : Nat, 0 ≤
  requests_dependentWitness0 ∧
  0 ≤ requests_dependentWitness1 ∧
  (requests_dependentWitness0 + requests_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def htr_consolidation_requests (requests : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let requests_dependentWitness0 := (requests).1
  let requests_dependentWitness1 := ((requests).2).1
  let requests := ((requests).2).2
  let items ← do
    (ssz_bounded_fixed_list_ref ⟨_, ⟨_, requests⟩⟩ HTR_CONSOLIDATION_REQUEST_LENGTH (2 ^i 1))
  let count : Nat := items.count
  let root ← do
    (htr_requests_subtree ⟨_, ⟨_, requests⟩⟩ HtrConsolidationRequest 0 count 1)
  (mix_in_length root items.count)

/-- `hash_tree_root` of the builder-deposit-request list (depth 6). -/
/- Type quantifiers: requests_dependentWitness1 : Nat, requests_dependentWitness0 : Nat, 0 ≤
  requests_dependentWitness0 ∧
  0 ≤ requests_dependentWitness1 ∧
  (requests_dependentWitness0 + requests_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def htr_builder_deposit_requests (requests : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let requests_dependentWitness0 := (requests).1
  let requests_dependentWitness1 := ((requests).2).1
  let requests := ((requests).2).2
  let items ← do
    (ssz_bounded_fixed_list_ref ⟨_, ⟨_, requests⟩⟩ HTR_BUILDER_DEPOSIT_LENGTH (2 ^i 6))
  let count : Nat := items.count
  let root ← do
    (htr_requests_subtree ⟨_, ⟨_, requests⟩⟩ HtrBuilderDepositRequest 0 count 6)
  (mix_in_length root items.count)

/-- `hash_tree_root` of the builder-exit-request list (depth 4). -/
/- Type quantifiers: requests_dependentWitness1 : Nat, requests_dependentWitness0 : Nat, 0 ≤
  requests_dependentWitness0 ∧
  0 ≤ requests_dependentWitness1 ∧
  (requests_dependentWitness0 + requests_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def htr_builder_exit_requests (requests : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let requests_dependentWitness0 := (requests).1
  let requests_dependentWitness1 := ((requests).2).1
  let requests := ((requests).2).2
  let items ← do
    (ssz_bounded_fixed_list_ref ⟨_, ⟨_, requests⟩⟩ HTR_BUILDER_EXIT_LENGTH (2 ^i 4))
  let count : Nat := items.count
  let root ← do (htr_requests_subtree ⟨_, ⟨_, requests⟩⟩ HtrBuilderExitRequest 0 count 4)
  (mix_in_length root items.count)

/-- `hash_tree_root` of the `SszExecutionRequests` container. -/
def htr_execution_requests (input_ref : StatelessInputRef) : SailM (Vector (BitVec 8) 32) := do
  let deposits ← do (htr_deposits input_ref.deposits)
  let withdrawal_requests ← do (htr_withdrawal_requests input_ref.withdrawal_requests)
  let consolidation_requests ← do (htr_consolidation_requests input_ref.consolidation_requests)
  let builder_deposit_requests ← do
    (htr_builder_deposit_requests input_ref.builder_deposit_requests)
  let builder_exit_requests ← do (htr_builder_exit_requests input_ref.builder_exit_requests)
  (merkleize
    #v[builder_exit_requests, builder_deposit_requests, consolidation_requests, withdrawal_requests, deposits]
    3)

/-- `hash_tree_root` of the `SszNewPayloadRequest` (4 fields, depth 2) —
the commitment the guest proves its input against. -/
def htr_new_payload_request (input_ref : StatelessInputRef) : SailM (Vector (BitVec 8) 32) := do
  let payload_root ← do (htr_execution_payload input_ref)
  let versioned_hashes_root ← do (htr_versioned_hashes input_ref.versioned_hashes)
  let beacon_root ← do (htr_ssz_bytes32 input_ref.new_payload_request NPR_BEACON_ROOT)
  let requests_root ← do (htr_execution_requests input_ref)
  (merkleize #v[requests_root, beacon_root, versioned_hashes_root, payload_root] 2)

