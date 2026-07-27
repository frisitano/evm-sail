import Evm.Flow
import Evm.Vector
import Evm.Prelude
import Evm.Primitives.CycleScopes
import Evm.Host.CycleScopesDisabled
import Evm.Primitives.Quantities
import Evm.Primitives.Bytes
import Evm.Primitives.Crypto
import Evm.Host.EvmByteSlice
import Evm.Lib.Ssz.Ssz
import Evm.Lib.Ssz.StatelessInput
import Evm.Executor.Receipts

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

/-! # SSZ hash_tree_root

Consensus-layer merkleization (SHA-256). Variable SSZ lists are consumed
in source order: each real leaf is folded into a Merkle frontier exactly
once, and final zero padding is then folded one level at a time. No leaf
list or zero-hash lookup table is materialized. -/

def HTR_BYTE_LIST_LIMIT : Nat := 1073741824

def HTR_DEPOSIT_LENGTH : Nat := 192

def HTR_DEPOSIT_PUBKEY : Nat := 0

def HTR_DEPOSIT_PUBKEY_LENGTH : Nat := 48

def HTR_DEPOSIT_WITHDRAWAL_CREDENTIALS : Nat := 48

def HTR_DEPOSIT_AMOUNT : Nat := 80

def HTR_DEPOSIT_SIGNATURE : Nat := 88

def HTR_DEPOSIT_SIGNATURE_LENGTH : Nat := 96

def HTR_DEPOSIT_INDEX : Nat := 184

def HTR_WITHDRAWAL_REQUEST_LENGTH : Nat := 76

def HTR_REQUEST_SOURCE_ADDRESS : Nat := 0

def HTR_REQUEST_SOURCE_PUBKEY : Nat := 20

def HTR_REQUEST_PUBKEY_LENGTH : Nat := 48

def HTR_WITHDRAWAL_REQUEST_AMOUNT : Nat := 68

def HTR_CONSOLIDATION_REQUEST_LENGTH : Nat := 116

def HTR_CONSOLIDATION_REQUEST_TARGET_PUBKEY : Nat := 68

def HTR_BUILDER_DEPOSIT_LENGTH : Nat := 184

def HTR_BUILDER_DEPOSIT_PUBKEY : Nat := 0

def HTR_BUILDER_DEPOSIT_WITHDRAWAL_CREDENTIALS : Nat := 48

def HTR_BUILDER_DEPOSIT_AMOUNT : Nat := 80

def HTR_BUILDER_DEPOSIT_SIGNATURE : Nat := 88

def HTR_BUILDER_EXIT_LENGTH : Nat := 68

def HTR_BUILDER_EXIT_SOURCE_ADDRESS : Nat := 0

def HTR_BUILDER_EXIT_PUBKEY : Nat := 20

def SSZ_ZERO_HASH_01 : hash :=
  (B256
    (to_bytes_le (n := 32) 0xF5A5FD42D16A20302798EF6ED309979B43003D2320D9F0E8EA9831A92759FB4B#256))

def SSZ_ZERO_HASH_02 : hash :=
  (B256
    (to_bytes_le (n := 32) 0xDB56114E00FDD4C1F85C892BF35AC9A89289AAECB1EBD0A96CDE606A748B5D71#256))

def SSZ_ZERO_HASH_03 : hash :=
  (B256
    (to_bytes_le (n := 32) 0xC78009FDF07FC56A11F122370658A353AAA542ED63E44C4BC15FF4CD105AB33C#256))

def SSZ_ZERO_HASH_04 : hash :=
  (B256
    (to_bytes_le (n := 32) 0x536D98837F2DD165A55D5EEAE91485954472D56F246DF256BF3CAE19352A123C#256))

def SSZ_ZERO_HASH_05 : hash :=
  (B256
    (to_bytes_le (n := 32) 0x9EFDE052AA15429FAE05BAD4D0B1D7C64DA64D03D7A1854A588C2CB8430C0D30#256))

def SSZ_ZERO_HASH_06 : hash :=
  (B256
    (to_bytes_le (n := 32) 0xD88DDFEED400A8755596B21942C1497E114C302E6118290F91E6772976041FA1#256))

def SSZ_ZERO_HASH_07 : hash :=
  (B256
    (to_bytes_le (n := 32) 0x87EB0DDBA57E35F6D286673802A4AF5975E22506C7CF4C64BB6BE5EE11527F2C#256))

def SSZ_ZERO_HASH_08 : hash :=
  (B256
    (to_bytes_le (n := 32) 0x26846476FD5FC54A5D43385167C95144F2643F533CC85BB9D16B782F8D7DB193#256))

def SSZ_ZERO_HASH_09 : hash :=
  (B256
    (to_bytes_le (n := 32) 0x506D86582D252405B840018792CAD2BF1259F1EF5AA5F887E13CB2F0094F51E1#256))

def SSZ_ZERO_HASH_10 : hash :=
  (B256
    (to_bytes_le (n := 32) 0xFFFF0AD7E659772F9534C195C815EFC4014EF1E1DAED4404C06385D11192E92B#256))

def SSZ_ZERO_HASH_11 : hash :=
  (B256
    (to_bytes_le (n := 32) 0x6CF04127DB05441CD833107A52BE852868890E4317E6A02AB47683AA75964220#256))

def SSZ_ZERO_HASH_12 : hash :=
  (B256
    (to_bytes_le (n := 32) 0xB7D05F875F140027EF5118A2247BBB84CE8F2F0F1123623085DAF7960C329F5F#256))

def SSZ_ZERO_HASH_13 : hash :=
  (B256
    (to_bytes_le (n := 32) 0xDF6AF5F5BBDB6BE9EF8AA618E4BF8073960867171E29676F8B284DEA6A08A85E#256))

def SSZ_ZERO_HASH_14 : hash :=
  (B256
    (to_bytes_le (n := 32) 0xB58D900F5E182E3C50EF74969EA16C7726C549757CC23523C369587DA7293784#256))

def SSZ_ZERO_HASH_15 : hash :=
  (B256
    (to_bytes_le (n := 32) 0xD49A7502FFCFB0340B1D7885688500CA308161A7F96B62DF9D083B71FCC8F2BB#256))

def SSZ_ZERO_HASH_16 : hash :=
  (B256
    (to_bytes_le (n := 32) 0x8FE6B1689256C0D385F42F5BBE2027A22C1996E110BA97C171D3E5948DE92BEB#256))

def SSZ_ZERO_HASH_17 : hash :=
  (B256
    (to_bytes_le (n := 32) 0x8D0D63C39EBADE8509E0AE3C9C3876FB5FA112BE18F905ECACFECB92057603AB#256))

def SSZ_ZERO_HASH_18 : hash :=
  (B256
    (to_bytes_le (n := 32) 0x95EEC8B2E541CAD4E91DE38385F2E046619F54496C2382CB6CACD5B98C26F5A4#256))

def SSZ_ZERO_HASH_19 : hash :=
  (B256
    (to_bytes_le (n := 32) 0xF893E908917775B62BFF23294DBBE3A1CD8E6CC1C35B4801887B646A6F81F17F#256))

def SSZ_ZERO_HASH_20 : hash :=
  (B256
    (to_bytes_le (n := 32) 0xCDDBA7B592E3133393C16194FAC7431ABF2F5485ED711DB282183C819E08EBAA#256))

def SSZ_ZERO_HASH_21 : hash :=
  (B256
    (to_bytes_le (n := 32) 0x8A8D7FE3AF8CAA085A7639A832001457DFB9128A8061142AD0335629FF23FF9C#256))

def SSZ_ZERO_HASH_22 : hash :=
  (B256
    (to_bytes_le (n := 32) 0xFEB3C337D7A51A6FBF00B9E34C52E1C9195C969BD4E7A0BFD51D5C5BED9C1167#256))

def SSZ_ZERO_HASH_23 : hash :=
  (B256
    (to_bytes_le (n := 32) 0xE71F0AA83CC32EDFBEFA9F4D3E0174CA85182EEC9F3A09F6A6C0DF6377A510D7#256))

def SSZ_ZERO_HASH_24 : hash :=
  (B256
    (to_bytes_le (n := 32) 0x31206FA80A50BB6ABE29085058F16212212A60EEC8F049FECB92D8C8E0A84BC0#256))

def SSZ_ZERO_HASH_25 : hash :=
  (B256
    (to_bytes_le (n := 32) 0x21352BFECBEDDDE993839F614C3DAC0A3EE37543F9B412B16199DC158E23B544#256))

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

/-- Advances within the supported execution-layer SSZ depth range. -/
/- Type quantifiers: depth : Nat, 0 ≤ depth ∧ depth ≤ 25 -/
def htr_depth_increment (depth : Nat) : SailM Nat := do
  if ((depth <b 25) : Bool)
  then (pure (depth + 1))
  else sailThrow ((InvalidBlock WitnessDeficient))

/-- Creates an empty accumulator whose capacity is determined by its SSZ tree
depth. -/
/- Type quantifiers: depth : Nat, 0 ≤ depth ∧ depth ≤ 25 -/
def merkle_accumulator_empty (depth : Nat) : MerkleAccumulator :=
  { frontier := [],
    count := 0,
    depth := depth }

/-- Adds leaf number `count` to a low-to-high frontier. A set bit in
`count` has one occupied slot at that level; carries consume those
slots head-first. -/
/- Type quantifiers: k_ex416669_ : Nat, 0 ≤ k_ex416669_ ∧ k_ex416669_ ≤ 33554432 -/
def merkle_push (frontier : (List MerkleSlot)) (count : Nat) (leaf : (Vector (BitVec 8) 32)) : SailM (List MerkleSlot) := do
  if (((Nat.mod count 2) == 0) : Bool)
  then
    (match frontier with
    | (_ :: rest) => (pure ((OccupiedMerkleSlot leaf) :: rest))
    | [] => (pure [(OccupiedMerkleSlot leaf)]))
  else
    (do
      match frontier with
      | (.OccupiedMerkleSlot left :: rest) =>
        (pure ((EmptyMerkleSlot ()) :: (← (merkle_push rest (Nat.div count 2)
              (← (sha256_pair left leaf))))))
      | (.EmptyMerkleSlot () :: _) => sailThrow ((InvalidBlock WitnessDeficient))
      | [] => sailThrow ((InvalidBlock WitnessDeficient)))

/-- Adds one leaf to a streaming Merkle accumulator. -/
def merkle_accumulator_push (accumulator : MerkleAccumulator) (leaf : (Vector (BitVec 8) 32)) : SailM MerkleAccumulator := do
  let count := accumulator.count
  if ((count <b (htr_leaf_capacity accumulator.depth)) : Bool)
  then
    (pure { frontier := ← (merkle_push accumulator.frontier count leaf),
            count := (count + 1),
            depth := accumulator.depth })
  else sailThrow ((InvalidBlock WitnessDeficient))

/-- Completes a frontier with zero subtrees through `remaining` levels and
returns its Merkle root. -/
/- Type quantifiers: _reclimit : Nat, k_ex416672_ : Nat, k_ex416671_ : Nat, k_ex416670_ : Nat, 0 ≤
  k_ex416670_ ∧ k_ex416670_ ≤ 33554432, 0 ≤ k_ex416671_ ∧ k_ex416671_ ≤ 25, 0 ≤
  k_ex416672_ ∧ k_ex416672_ ≤ 25, 0 ≤ _reclimit -/
def _rec_merkle_root_levels (slots : (List MerkleSlot)) (n : Nat) (acc : (Vector (BitVec 8) 32)) (level : Nat) (remaining : Nat) (_reclimit : Nat) : SailM (Vector (BitVec 8) 32) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((remaining == 0) : Bool)
      then
        (do
          if ((n == 0) : Bool)
          then (pure acc)
          else
            (do
              if ((n == 1) : Bool)
              then
                (do
                  match slots with
                  | (.OccupiedMerkleSlot root :: _) => (pure root)
                  | _ => sailThrow ((InvalidBlock WitnessDeficient)))
              else sailThrow ((InvalidBlock WitnessDeficient))))
      else
        (do
          let zero := (ssz_zero_hash level)
          let slot : MerkleSlot := (EmptyMerkleSlot ())
          let rest : (List MerkleSlot) := []
          let (rest, slot) : ((List MerkleSlot) × MerkleSlot) :=
            match slots with
            | (head :: tail) =>
              (let slot : MerkleSlot := head
              let rest : (List MerkleSlot) := tail
              (rest, slot))
            | [] => (rest, slot)
          let next_acc ← do
            if (((Nat.mod n 2) == 1) : Bool)
            then
              (do
                match slot with
                | .OccupiedMerkleSlot left => (sha256_pair left acc)
                | .EmptyMerkleSlot () => sailThrow ((InvalidBlock WitnessDeficient)))
            else (sha256_pair acc zero)
          let next_level ← do (htr_depth_increment level)
          (_rec_merkle_root_levels rest (Nat.div n 2) next_acc next_level (remaining - 1)
            _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Completes a frontier with zero subtrees through `remaining` levels and
returns its Merkle root. -/
/- Type quantifiers: remaining : Nat, level : Nat, n : Nat, 0 ≤ n ∧ n ≤ 33554432, 0 ≤ level
  ∧ level ≤ 25, 0 ≤ remaining ∧ remaining ≤ 25 -/
def merkle_root_levels (slots : (List MerkleSlot)) (n : Nat) (acc : (Vector (BitVec 8) 32)) (level : Nat) (remaining : Nat) : SailM (Vector (BitVec 8) 32) := do
  let _measure := (remaining : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_merkle_root_levels slots n acc level remaining (_measure + 1))

/-- Pads an incremental Merkle accumulator to its declared depth and returns
its root. -/
def merkle_accumulator_root (accumulator : MerkleAccumulator) : SailM (Vector (BitVec 8) 32) := do
  let _ : Unit := (cycle_scope_start SCOPE_HTR_MERKLE_PADDING)
  let root ← do
    (merkle_root_levels accumulator.frontier accumulator.count ZERO_HASH 0 accumulator.depth)
  let _ : Unit := (cycle_scope_end SCOPE_HTR_MERKLE_PADDING)
  (pure root)

/-- Folds a leaf list into an existing Merkle accumulator in source order. -/
def merkle_accumulate (leaves : (List (Vector (BitVec 8) 32))) (accumulator : MerkleAccumulator) : SailM MerkleAccumulator := do
  match leaves with
  | [] => (pure accumulator)
  | (leaf :: rest) => (merkle_accumulate rest (← (merkle_accumulator_push accumulator leaf)))

/-- `merkleize` of a fixed leaf list at `depth` (SSZ spec). -/
/- Type quantifiers: k_ex416678_ : Nat, 0 ≤ k_ex416678_ ∧ k_ex416678_ ≤ 25 -/
def merkleize (leaves : (List (Vector (BitVec 8) 32))) (depth : Nat) : SailM (Vector (BitVec 8) 32) := do
  (merkle_accumulator_root (← (merkle_accumulate leaves (merkle_accumulator_empty depth))))

/-- Encodes a protocol quantity as an SSZ little-endian basic-value chunk. -/
/- Type quantifiers: v : Nat, 0 ≤ v ∧ v ≤ (2 ^ 64 - 1) -/
def htr_uint (v : Nat) : (Vector (BitVec 8) 32) :=
  let bytes := ZERO_HASH
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 31 (get_slice_int 8 v 0))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 30 (get_slice_int 8 v 8))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 29 (get_slice_int 8 v 16))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 28 (get_slice_int 8 v 24))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 27 (get_slice_int 8 v 32))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 26 (get_slice_int 8 v 40))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 25 (get_slice_int 8 v 48))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 24 (get_slice_int 8 v 56))
  (B256 bytes)

/-- `uint256` as a 32-byte little-endian chunk. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def htr_u256 (value : Nat) : (Vector (BitVec 8) 32) :=
  let bytes := ZERO_HASH
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 31 (get_slice_int 8 value 0))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 30 (get_slice_int 8 value 8))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 29 (get_slice_int 8 value 16))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 28 (get_slice_int 8 value 24))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 27 (get_slice_int 8 value 32))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 26 (get_slice_int 8 value 40))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 25 (get_slice_int 8 value 48))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 24 (get_slice_int 8 value 56))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 23 (get_slice_int 8 value 64))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 22 (get_slice_int 8 value 72))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 21 (get_slice_int 8 value 80))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 20 (get_slice_int 8 value 88))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 19 (get_slice_int 8 value 96))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 18 (get_slice_int 8 value 104))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 17 (get_slice_int 8 value 112))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 16 (get_slice_int 8 value 120))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 15 (get_slice_int 8 value 128))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 14 (get_slice_int 8 value 136))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 13 (get_slice_int 8 value 144))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 12 (get_slice_int 8 value 152))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 11 (get_slice_int 8 value 160))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 10 (get_slice_int 8 value 168))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 9 (get_slice_int 8 value 176))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 8 (get_slice_int 8 value 184))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 7 (get_slice_int 8 value 192))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 6 (get_slice_int 8 value 200))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 5 (get_slice_int 8 value 208))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 4 (get_slice_int 8 value 216))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 3 (get_slice_int 8 value 224))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 2 (get_slice_int 8 value 232))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 1 (get_slice_int 8 value 240))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 0 (get_slice_int 8 value 248))
  (B256 bytes)

/-- `Bytes32` chunks are themselves. -/
def htr_bytes32 (b : (Vector (BitVec 8) 32)) : (Vector (BitVec 8) 32) :=
  b

/-- A 20-byte address, left-aligned in its chunk. -/
def htr_addr (address_bytes : (Vector (BitVec 8) 20)) : (Vector (BitVec 8) 32) :=
  let bytes := ZERO_HASH
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 12 (GetElem?.getElem! address_bytes 0))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 13 (GetElem?.getElem! address_bytes 1))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 14 (GetElem?.getElem! address_bytes 2))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 15 (GetElem?.getElem! address_bytes 3))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 16 (GetElem?.getElem! address_bytes 4))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 17 (GetElem?.getElem! address_bytes 5))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 18 (GetElem?.getElem! address_bytes 6))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 19 (GetElem?.getElem! address_bytes 7))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 20 (GetElem?.getElem! address_bytes 8))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 21 (GetElem?.getElem! address_bytes 9))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 22 (GetElem?.getElem! address_bytes 10))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 23 (GetElem?.getElem! address_bytes 11))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 24 (GetElem?.getElem! address_bytes 12))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 25 (GetElem?.getElem! address_bytes 13))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 26 (GetElem?.getElem! address_bytes 14))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 27 (GetElem?.getElem! address_bytes 15))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 28 (GetElem?.getElem! address_bytes 16))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 29 (GetElem?.getElem! address_bytes 17))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 30 (GetElem?.getElem! address_bytes 18))
  let bytes : (Vector (BitVec 8) 32) := (vectorUpdate bytes 31 (GetElem?.getElem! address_bytes 19))
  (B256 bytes)

/- Type quantifiers: len : Nat, (source_valid_length len) -/
def mix_in_length (root : (Vector (BitVec 8) 32)) (len : Nat) : SailM (Vector (BitVec 8) 32) := do
  (sha256_pair root (htr_u256 (← (word_of_source_byte_count len))))

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
              let depth ← (htr_depth_increment depth)
              let remaining : Nat := (Nat.div remaining 2)
              (pure (depth, remaining)))
          else (pure (depth, remaining)) ) : SailM (Nat × Nat) )
        (pure (depth, remaining))
    (pure loop_vars) ) : SailM (Nat × Nat) )
  (pure depth)

/-- The `chunk_index`-th 32-byte chunk of a byte span, zero-padded. -/
/- Type quantifiers: k_ex416693_ : Nat, bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, 0
  ≤ bytes_dependentWitness0 ∧ 0 ≤ bytes_dependentWitness1, 0 ≤ k_ex416693_ ∧
  k_ex416693_ ≤ 33554432 -/
def htr_chunk (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (chunk_index : Nat) : SailM (Vector (BitVec 8) 32) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  (pure (word_to_hash (← (slice_load ⟨_, ⟨_, bytes⟩⟩ (chunk_index *i WORD_BYTE_LENGTH)))))

/- Type quantifiers: byte_len : Nat, (source_valid_length byte_len) -/
def htr_chunk_count (byte_len : Nat) : SailM Nat := do
  if ((byte_len ≤b HTR_BYTE_LIST_LIMIT) : Bool)
  then (pure ((byte_len + 31) / 32))
  else sailThrow ((InvalidBlock WitnessDeficient))

/-- The Merkle root of a byte span's chunks at `depth`. -/
/- Type quantifiers: k_ex416705_ : Nat, bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, 0
  ≤ bytes_dependentWitness0 ∧ 0 ≤ bytes_dependentWitness1, 0 ≤ k_ex416705_ ∧
  k_ex416705_ ≤ 25 -/
def htr_bytes_root (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (depth : Nat) : SailM (Vector (BitVec 8) 32) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let _ : Unit := (cycle_scope_start SCOPE_HTR_BYTES_ROOT)
  let count ← do (htr_chunk_count bytes.len)
  let accumulator := (merkle_accumulator_empty depth)
  let index : Nat := 0
  let (accumulator, index) ← (( do
    let loop_vars ← whileFuelM (fuel :=(count -i index)) (fun (accumulator, index) => (pure (index <b count))) (accumulator, index)
      fun (accumulator, index) => do
        assert true "loop dummy assert"
        let accumulator ←
          (merkle_accumulator_push accumulator (← (htr_chunk ⟨_, ⟨_, bytes⟩⟩ index)))
        let index : Nat := accumulator.count
        (pure (accumulator, index))
    (pure loop_vars) ) : SailM (MerkleAccumulator × Nat) )
  let root ← do (merkle_accumulator_root accumulator)
  let _ : Unit := (cycle_scope_end SCOPE_HTR_BYTES_ROOT)
  (pure root)

/-- `hash_tree_root` of a fixed-size `ByteVector`. -/
/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, 0 ≤
  bytes_dependentWitness0 ∧ 0 ≤ bytes_dependentWitness1 -/
def htr_bytevector (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let chunks ← do (htr_chunk_count bytes.len)
  (htr_bytes_root ⟨_, ⟨_, bytes⟩⟩ (← (clog2 chunks)))

/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, limit_bytes : Nat, (source_valid_length limit_bytes), 0
  ≤ bytes_dependentWitness0 ∧ 0 ≤ bytes_dependentWitness1 -/
def htr_bytelist (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (limit_bytes : Nat) : SailM (Vector (BitVec 8) 32) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let capacity ← do (htr_chunk_count limit_bytes)
  (mix_in_length (← (htr_bytes_root ⟨_, ⟨_, bytes⟩⟩ (← (clog2 capacity)))) bytes.len)

/-- `hash_tree_root` of one SSZ withdrawal (4 fields, depth 2). -/
/- Type quantifiers: withdrawal_dependentWitness1 : Nat, withdrawal_dependentWitness0 : Nat, 0 ≤
  withdrawal_dependentWitness0 ∧ 0 ≤ withdrawal_dependentWitness1 ∧
  withdrawal_dependentWitness1 = 44 -/
def htr_withdrawal (withdrawal : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let withdrawal_dependentWitness0 := (withdrawal).1
  let withdrawal_dependentWitness1 := ((withdrawal).2).1
  let withdrawal := ((withdrawal).2).2
  (merkleize
    [(htr_uint (← (decode_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_INDEX))), (htr_uint
      (← (decode_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_VALIDATOR_INDEX))), (htr_addr
      (← (ssz_addr ⟨_, ⟨_, withdrawal⟩⟩ WD_ADDRESS))), (htr_uint
      (← (decode_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_AMOUNT)))] 2)

/-- `hash_tree_root` of the transactions list
(`List[ByteList[2^30], 2^20]`). -/
def htr_transactions (transactions : (BoundedSszListRef (2 ^ 20))) : SailM (Vector (BitVec 8) 32) := do
  let _ : Unit := (cycle_scope_start SCOPE_HTR_TRANSACTIONS)
  let cursor ← do (ssz_list_cursor transactions)
  let accumulator := (merkle_accumulator_empty 20)
  let (accumulator, cursor) ← (( do
    let loop_vars ← whileFuelM (fuel :=(cursor.items.count -i cursor.index)) (fun (accumulator, cursor) => (pure (! (ssz_list_cursor_empty
          cursor)))) (accumulator, cursor)
      fun (accumulator, cursor) => do
        assert true "loop dummy assert"
        let (transaction, next) ← do (ssz_list_pop cursor)
        let cursor : (BoundedSszListCursor (2 ^ 20)) := next
        let accumulator ←
          (merkle_accumulator_push accumulator (← (htr_bytelist transaction HTR_BYTE_LIST_LIMIT)))
        (pure (accumulator, cursor))
    (pure loop_vars) ) : SailM (MerkleAccumulator × (BoundedSszListCursor (2 ^ 20))) )
  let root ← do (mix_in_length (← (merkle_accumulator_root accumulator)) accumulator.count)
  let _ : Unit := (cycle_scope_end SCOPE_HTR_TRANSACTIONS)
  (pure root)

/-- `hash_tree_root` of the withdrawals list (`List[SszWithdrawal, 16]`). -/
def htr_withdrawals (withdrawals : (BoundedSszListRef (2 ^ 4))) : SailM (Vector (BitVec 8) 32) := do
  let _ : Unit := (cycle_scope_start SCOPE_HTR_WITHDRAWALS)
  let rest : (BoundedSszListRef (2 ^ 4)) := withdrawals
  let accumulator := (merkle_accumulator_empty 4)
  let (accumulator, rest) ← (( do
    let loop_vars ← whileFuelM (fuel :=rest.count) (fun (accumulator, rest) => (pure (rest.count != 0))) (accumulator, rest)
      fun (accumulator, rest) => do
        assert true "loop dummy assert"
        let (withdrawal, tail) ← do (ssz_fixed_list_pop rest WD_SIZE)
        let rest : (BoundedSszListRef (2 ^ 4)) := tail
        let accumulator ← (merkle_accumulator_push accumulator (← (htr_withdrawal withdrawal)))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × (BoundedSszListRef (2 ^ 4))) )
  let root ← do (mix_in_length (← (merkle_accumulator_root accumulator)) accumulator.count)
  let _ : Unit := (cycle_scope_end SCOPE_HTR_WITHDRAWALS)
  (pure root)

/-- `hash_tree_root` of the `SszExecutionPayload` (19 fields, depth 5). -/
def htr_execution_payload (input_ref : StatelessInputRef) : SailM (Vector (BitVec 8) 32) := do
  let _ : Unit := (cycle_scope_start SCOPE_HTR_EXECUTION_PAYLOAD)
  let ⟨_, ⟨_, payload⟩⟩ := input_ref.execution_payload
  let root ← do
    (merkleize
      [(htr_bytes32 (← (ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ 0))), (htr_addr
        (← (ssz_addr ⟨_, ⟨_, payload⟩⟩ PL_FEE_RECIPIENT))), (htr_bytes32
        (← (ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ PL_STATE_ROOT))), (htr_bytes32
        (← (ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ PL_RECEIPTS_ROOT))), (← (htr_bytevector
          ⟨_, ⟨_, (sub_slice payload PL_LOGS_BLOOM LOGS_BLOOM_BYTE_LENGTH)⟩⟩)), (htr_bytes32
        (← (ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ PL_PREV_RANDAO))), (htr_uint
        (← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_BLOCK_NUMBER))), (htr_uint
        (← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_GAS_LIMIT))), (htr_uint
        (← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_GAS_USED))), (htr_uint
        (← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_TIMESTAMP))), (← (htr_bytelist
          input_ref.extra_data WORD_BYTE_LENGTH)), (htr_u256
        (← (ssz_u256 ⟨_, ⟨_, payload⟩⟩ PL_BASE_FEE))), (htr_bytes32
        (← (ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ PL_BLOCK_HASH))), (← (htr_transactions
          input_ref.transactions)), (← (htr_withdrawals input_ref.withdrawals)), (htr_uint
        (← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_BLOB_GAS_USED))), (htr_uint
        (← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_EXCESS_BLOB_GAS))), (← (htr_bytelist
          input_ref.block_access_list HTR_BYTE_LIST_LIMIT)), (htr_uint
        (← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_SLOT_NUMBER)))] 5)
  let _ : Unit := (cycle_scope_end SCOPE_HTR_EXECUTION_PAYLOAD)
  (pure root)

/-- `hash_tree_root` of the versioned hashes (`List[Bytes32, 4096]`). -/
/- Type quantifiers: versioned_hashes_dependentWitness1 : Nat, versioned_hashes_dependentWitness0 :
  Nat, 0 ≤ versioned_hashes_dependentWitness0 ∧ 0 ≤ versioned_hashes_dependentWitness1 -/
def htr_versioned_hashes (versioned_hashes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let versioned_hashes_dependentWitness0 := (versioned_hashes).1
  let versioned_hashes_dependentWitness1 := ((versioned_hashes).2).1
  let versioned_hashes := ((versioned_hashes).2).2
  let _ : Unit := (cycle_scope_start SCOPE_HTR_VERSIONED_HASHES)
  let rest ← do
    (ssz_bounded_fixed_list_ref ⟨_, ⟨_, versioned_hashes⟩⟩ WORD_BYTE_LENGTH (2 ^i 12))
  let accumulator := (merkle_accumulator_empty 12)
  let (accumulator, rest) ← (( do
    let loop_vars ← whileFuelM (fuel :=rest.count) (fun (accumulator, rest) => (pure (rest.count != 0))) (accumulator, rest)
      fun (accumulator, rest) => do
        assert true "loop dummy assert"
        let (versioned_hash, tail) ← do (ssz_fixed_list_pop rest WORD_BYTE_LENGTH)
        let rest : (BoundedSszListRef (2 ^ 12)) := tail
        let accumulator ←
          (merkle_accumulator_push accumulator (word_to_hash (← (slice_load versioned_hash 0))))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × (BoundedSszListRef (2 ^ 12))) )
  let root ← do (mix_in_length (← (merkle_accumulator_root accumulator)) accumulator.count)
  let _ : Unit := (cycle_scope_end SCOPE_HTR_VERSIONED_HASHES)
  (pure root)

/-- `hash_tree_root` of one deposit request (5 fields, depth 3). -/
/- Type quantifiers: deposit_dependentWitness1 : Nat, deposit_dependentWitness0 : Nat, 0 ≤
  deposit_dependentWitness0 ∧ 0 ≤ deposit_dependentWitness1 ∧ deposit_dependentWitness1 = 192 -/
def htr_deposit (deposit : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let deposit_dependentWitness0 := (deposit).1
  let deposit_dependentWitness1 := ((deposit).2).1
  let deposit := ((deposit).2).2
  (merkleize
    [(← (htr_bytevector
        ⟨_, ⟨_, (sub_slice deposit HTR_DEPOSIT_PUBKEY HTR_DEPOSIT_PUBKEY_LENGTH)⟩⟩)), (htr_bytes32
      (← (ssz_bytes32 ⟨_, ⟨_, deposit⟩⟩ HTR_DEPOSIT_WITHDRAWAL_CREDENTIALS))), (htr_uint
      (← (decode_ssz_uint ⟨_, ⟨_, deposit⟩⟩ HTR_DEPOSIT_AMOUNT))), (← (htr_bytevector
        ⟨_, ⟨_, (sub_slice deposit HTR_DEPOSIT_SIGNATURE HTR_DEPOSIT_SIGNATURE_LENGTH)⟩⟩)), (htr_uint
      (← (decode_ssz_uint ⟨_, ⟨_, deposit⟩⟩ HTR_DEPOSIT_INDEX)))] 3)

/-- `hash_tree_root` of one withdrawal request (3 fields, depth 2). -/
/- Type quantifiers: request_dependentWitness1 : Nat, request_dependentWitness0 : Nat, 0 ≤
  request_dependentWitness0 ∧ 0 ≤ request_dependentWitness1 ∧ request_dependentWitness1 = 76 -/
def htr_withdrawal_request (request : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let request_dependentWitness0 := (request).1
  let request_dependentWitness1 := ((request).2).1
  let request := ((request).2).2
  (merkleize
    [(← (htr_bytevector
        ⟨_, ⟨_, (sub_slice request HTR_REQUEST_SOURCE_ADDRESS ADDRESS_BYTE_LENGTH)⟩⟩)), (← (htr_bytevector
        ⟨_, ⟨_, (sub_slice request HTR_REQUEST_SOURCE_PUBKEY HTR_REQUEST_PUBKEY_LENGTH)⟩⟩)), (htr_uint
      (← (decode_ssz_uint ⟨_, ⟨_, request⟩⟩ HTR_WITHDRAWAL_REQUEST_AMOUNT)))] 2)

/-- `hash_tree_root` of one consolidation request (3 fields, depth 2). -/
/- Type quantifiers: request_dependentWitness1 : Nat, request_dependentWitness0 : Nat, 0 ≤
  request_dependentWitness0 ∧ 0 ≤ request_dependentWitness1 ∧ request_dependentWitness1 = 116 -/
def htr_consolidation_request (request : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let request_dependentWitness0 := (request).1
  let request_dependentWitness1 := ((request).2).1
  let request := ((request).2).2
  (merkleize
    [(← (htr_bytevector
        ⟨_, ⟨_, (sub_slice request HTR_REQUEST_SOURCE_ADDRESS ADDRESS_BYTE_LENGTH)⟩⟩)), (← (htr_bytevector
        ⟨_, ⟨_, (sub_slice request HTR_REQUEST_SOURCE_PUBKEY HTR_REQUEST_PUBKEY_LENGTH)⟩⟩)), (← (htr_bytevector
        ⟨_, ⟨_, (sub_slice request HTR_CONSOLIDATION_REQUEST_TARGET_PUBKEY
          HTR_REQUEST_PUBKEY_LENGTH)⟩⟩))] 2)

/-- `hash_tree_root` of one builder deposit request (4 fields, depth 2). -/
/- Type quantifiers: request_dependentWitness1 : Nat, request_dependentWitness0 : Nat, 0 ≤
  request_dependentWitness0 ∧ 0 ≤ request_dependentWitness1 ∧ request_dependentWitness1 = 184 -/
def htr_builder_deposit_request (request : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let request_dependentWitness0 := (request).1
  let request_dependentWitness1 := ((request).2).1
  let request := ((request).2).2
  (merkleize
    [(← (htr_bytevector
        ⟨_, ⟨_, (sub_slice request HTR_BUILDER_DEPOSIT_PUBKEY HTR_REQUEST_PUBKEY_LENGTH)⟩⟩)), (htr_bytes32
      (← (ssz_bytes32 ⟨_, ⟨_, request⟩⟩ HTR_BUILDER_DEPOSIT_WITHDRAWAL_CREDENTIALS))), (htr_uint
      (← (decode_ssz_uint ⟨_, ⟨_, request⟩⟩ HTR_BUILDER_DEPOSIT_AMOUNT))), (← (htr_bytevector
        ⟨_, ⟨_, (sub_slice request HTR_BUILDER_DEPOSIT_SIGNATURE HTR_DEPOSIT_SIGNATURE_LENGTH)⟩⟩))]
    2)

/-- `hash_tree_root` of one builder exit request (2 fields, depth 1). -/
/- Type quantifiers: request_dependentWitness1 : Nat, request_dependentWitness0 : Nat, 0 ≤
  request_dependentWitness0 ∧ 0 ≤ request_dependentWitness1 ∧ request_dependentWitness1 = 68 -/
def htr_builder_exit_request (request : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let request_dependentWitness0 := (request).1
  let request_dependentWitness1 := ((request).2).1
  let request := ((request).2).2
  (merkleize
    [(← (htr_bytevector
        ⟨_, ⟨_, (sub_slice request HTR_BUILDER_EXIT_SOURCE_ADDRESS ADDRESS_BYTE_LENGTH)⟩⟩)), (← (htr_bytevector
        ⟨_, ⟨_, (sub_slice request HTR_BUILDER_EXIT_PUBKEY HTR_REQUEST_PUBKEY_LENGTH)⟩⟩))] 1)

/-- `hash_tree_root` of the deposit-request list (depth 13). -/
/- Type quantifiers: deposits_dependentWitness1 : Nat, deposits_dependentWitness0 : Nat, 0 ≤
  deposits_dependentWitness0 ∧ 0 ≤ deposits_dependentWitness1 -/
def htr_deposits (deposits : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let deposits_dependentWitness0 := (deposits).1
  let deposits_dependentWitness1 := ((deposits).2).1
  let deposits := ((deposits).2).2
  let rest ← do
    (ssz_bounded_fixed_list_ref ⟨_, ⟨_, deposits⟩⟩ HTR_DEPOSIT_LENGTH (2 ^i 13))
  let accumulator := (merkle_accumulator_empty 13)
  let (accumulator, rest) ← (( do
    let loop_vars ← whileFuelM (fuel :=rest.count) (fun (accumulator, rest) => (pure (rest.count != 0))) (accumulator, rest)
      fun (accumulator, rest) => do
        assert true "loop dummy assert"
        let (deposit, tail) ← do (ssz_fixed_list_pop rest HTR_DEPOSIT_LENGTH)
        let rest : (BoundedSszListRef (2 ^ 13)) := tail
        let accumulator ← (merkle_accumulator_push accumulator (← (htr_deposit deposit)))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × (BoundedSszListRef (2 ^ 13))) )
  (mix_in_length (← (merkle_accumulator_root accumulator)) accumulator.count)

/-- `hash_tree_root` of the withdrawal-request list (depth 4). -/
/- Type quantifiers: requests_dependentWitness1 : Nat, requests_dependentWitness0 : Nat, 0 ≤
  requests_dependentWitness0 ∧ 0 ≤ requests_dependentWitness1 -/
def htr_withdrawal_requests (requests : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let requests_dependentWitness0 := (requests).1
  let requests_dependentWitness1 := ((requests).2).1
  let requests := ((requests).2).2
  let rest ← do
    (ssz_bounded_fixed_list_ref ⟨_, ⟨_, requests⟩⟩ HTR_WITHDRAWAL_REQUEST_LENGTH (2 ^i 4))
  let accumulator := (merkle_accumulator_empty 4)
  let (accumulator, rest) ← (( do
    let loop_vars ← whileFuelM (fuel :=rest.count) (fun (accumulator, rest) => (pure (rest.count != 0))) (accumulator, rest)
      fun (accumulator, rest) => do
        assert true "loop dummy assert"
        let (request, tail) ← do (ssz_fixed_list_pop rest HTR_WITHDRAWAL_REQUEST_LENGTH)
        let rest : (BoundedSszListRef (2 ^ 4)) := tail
        let accumulator ←
          (merkle_accumulator_push accumulator (← (htr_withdrawal_request request)))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × (BoundedSszListRef (2 ^ 4))) )
  (mix_in_length (← (merkle_accumulator_root accumulator)) accumulator.count)

/-- `hash_tree_root` of the consolidation-request list (depth 1). -/
/- Type quantifiers: requests_dependentWitness1 : Nat, requests_dependentWitness0 : Nat, 0 ≤
  requests_dependentWitness0 ∧ 0 ≤ requests_dependentWitness1 -/
def htr_consolidation_requests (requests : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let requests_dependentWitness0 := (requests).1
  let requests_dependentWitness1 := ((requests).2).1
  let requests := ((requests).2).2
  let rest ← do
    (ssz_bounded_fixed_list_ref ⟨_, ⟨_, requests⟩⟩ HTR_CONSOLIDATION_REQUEST_LENGTH (2 ^i 1))
  let accumulator := (merkle_accumulator_empty 1)
  let (accumulator, rest) ← (( do
    let loop_vars ← whileFuelM (fuel :=rest.count) (fun (accumulator, rest) => (pure (rest.count != 0))) (accumulator, rest)
      fun (accumulator, rest) => do
        assert true "loop dummy assert"
        let (request, tail) ← do (ssz_fixed_list_pop rest HTR_CONSOLIDATION_REQUEST_LENGTH)
        let rest : (BoundedSszListRef (2 ^ 1)) := tail
        let accumulator ←
          (merkle_accumulator_push accumulator (← (htr_consolidation_request request)))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × (BoundedSszListRef (2 ^ 1))) )
  (mix_in_length (← (merkle_accumulator_root accumulator)) accumulator.count)

/-- `hash_tree_root` of the builder-deposit-request list (depth 6). -/
/- Type quantifiers: requests_dependentWitness1 : Nat, requests_dependentWitness0 : Nat, 0 ≤
  requests_dependentWitness0 ∧ 0 ≤ requests_dependentWitness1 -/
def htr_builder_deposit_requests (requests : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let requests_dependentWitness0 := (requests).1
  let requests_dependentWitness1 := ((requests).2).1
  let requests := ((requests).2).2
  let rest ← do
    (ssz_bounded_fixed_list_ref ⟨_, ⟨_, requests⟩⟩ HTR_BUILDER_DEPOSIT_LENGTH (2 ^i 6))
  let accumulator := (merkle_accumulator_empty 6)
  let (accumulator, rest) ← (( do
    let loop_vars ← whileFuelM (fuel :=rest.count) (fun (accumulator, rest) => (pure (rest.count != 0))) (accumulator, rest)
      fun (accumulator, rest) => do
        assert true "loop dummy assert"
        let (request, tail) ← do (ssz_fixed_list_pop rest HTR_BUILDER_DEPOSIT_LENGTH)
        let rest : (BoundedSszListRef (2 ^ 6)) := tail
        let accumulator ←
          (merkle_accumulator_push accumulator (← (htr_builder_deposit_request request)))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × (BoundedSszListRef (2 ^ 6))) )
  (mix_in_length (← (merkle_accumulator_root accumulator)) accumulator.count)

/-- `hash_tree_root` of the builder-exit-request list (depth 4). -/
/- Type quantifiers: requests_dependentWitness1 : Nat, requests_dependentWitness0 : Nat, 0 ≤
  requests_dependentWitness0 ∧ 0 ≤ requests_dependentWitness1 -/
def htr_builder_exit_requests (requests : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let requests_dependentWitness0 := (requests).1
  let requests_dependentWitness1 := ((requests).2).1
  let requests := ((requests).2).2
  let rest ← do
    (ssz_bounded_fixed_list_ref ⟨_, ⟨_, requests⟩⟩ HTR_BUILDER_EXIT_LENGTH (2 ^i 4))
  let accumulator := (merkle_accumulator_empty 4)
  let (accumulator, rest) ← (( do
    let loop_vars ← whileFuelM (fuel :=rest.count) (fun (accumulator, rest) => (pure (rest.count != 0))) (accumulator, rest)
      fun (accumulator, rest) => do
        assert true "loop dummy assert"
        let (request, tail) ← do (ssz_fixed_list_pop rest HTR_BUILDER_EXIT_LENGTH)
        let rest : (BoundedSszListRef (2 ^ 4)) := tail
        let accumulator ←
          (merkle_accumulator_push accumulator (← (htr_builder_exit_request request)))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × (BoundedSszListRef (2 ^ 4))) )
  (mix_in_length (← (merkle_accumulator_root accumulator)) accumulator.count)

/-- `hash_tree_root` of the `SszExecutionRequests` container. -/
def htr_execution_requests (input_ref : StatelessInputRef) : SailM (Vector (BitVec 8) 32) := do
  let _ : Unit := (cycle_scope_start SCOPE_HTR_EXECUTION_REQUESTS)
  let root ← do
    (merkleize
      [(← (htr_deposits input_ref.deposits)), (← (htr_withdrawal_requests
          input_ref.withdrawal_requests)), (← (htr_consolidation_requests
          input_ref.consolidation_requests)), (← (htr_builder_deposit_requests
          input_ref.builder_deposit_requests)), (← (htr_builder_exit_requests
          input_ref.builder_exit_requests))] 3)
  let _ : Unit := (cycle_scope_end SCOPE_HTR_EXECUTION_REQUESTS)
  (pure root)

/-- `hash_tree_root` of the `SszNewPayloadRequest` (4 fields, depth 2) —
the commitment the guest proves its input against. -/
def htr_new_payload_request (input_ref : StatelessInputRef) : SailM (Vector (BitVec 8) 32) := do
  (merkleize
    [(← (htr_execution_payload input_ref)), (← (htr_versioned_hashes input_ref.versioned_hashes)), (htr_bytes32
      (← (ssz_bytes32 input_ref.new_payload_request NPR_BEACON_ROOT))), (← (htr_execution_requests
        input_ref))] 2)

