import Evm.Flow
import Evm.Arith
import Evm.Prelude
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

def EMPTY_MERKLE_ACCUMULATOR : MerkleAccumulator :=
  { frontier := [],
    count := ⟨0⟩ }

def HTR_BYTE_LIST_LIMIT : byte_length := (ByteQuantity 1073741824)

def HTR_DEPOSIT_LENGTH : byte_length := (ByteQuantity 192)

def HTR_DEPOSIT_PUBKEY : source_pointer := BYTE_ZERO

def HTR_DEPOSIT_PUBKEY_LENGTH : byte_length := (ByteQuantity 48)

def HTR_DEPOSIT_WITHDRAWAL_CREDENTIALS : source_pointer := (ByteQuantity 48)

def HTR_DEPOSIT_AMOUNT : source_pointer := (ByteQuantity 80)

def HTR_DEPOSIT_SIGNATURE : source_pointer := (ByteQuantity 88)

def HTR_DEPOSIT_SIGNATURE_LENGTH : byte_length := (ByteQuantity 96)

def HTR_DEPOSIT_INDEX : source_pointer := (ByteQuantity 184)

def HTR_WITHDRAWAL_REQUEST_LENGTH : byte_length := (ByteQuantity 76)

def HTR_REQUEST_SOURCE_ADDRESS : source_pointer := BYTE_ZERO

def HTR_REQUEST_SOURCE_PUBKEY : source_pointer := (ByteQuantity 20)

def HTR_REQUEST_PUBKEY_LENGTH : byte_length := (ByteQuantity 48)

def HTR_WITHDRAWAL_REQUEST_AMOUNT : source_pointer := (ByteQuantity 68)

def HTR_CONSOLIDATION_REQUEST_LENGTH : byte_length := (ByteQuantity 116)

def HTR_CONSOLIDATION_REQUEST_TARGET_PUBKEY : source_pointer := (ByteQuantity 68)

/-- Type quantifiers: k_ex161874_ : Nat, 0 ≤ k_ex161874_ ∧ k_ex161874_ ≤ (2 ^ 64 - 1) -/
def merkle_push (frontier : (List MerkleSlot)) (count : item_count) (leaf : hash) : SailM (List MerkleSlot) := do
  let count := (count).value
  if (((Int.tmod count 2) == 0) : Bool)
  then
    (match frontier with
    | (_ :: rest) => (pure ((OccupiedMerkleSlot leaf) :: rest))
    | [] => (pure [(OccupiedMerkleSlot leaf)]))
  else
    (do
      match frontier with
      | (.OccupiedMerkleSlot left :: rest) =>
        (pure ((EmptyMerkleSlot ()) :: (← (merkle_push rest
              ⟨((← (protocol_quantity_quotient ⟨count⟩ ⟨2⟩))).value⟩
              (← (sha256_pair left leaf))))))
      | (.EmptyMerkleSlot () :: _) => sailThrow ((InvalidBlock WitnessDeficient))
      | [] => sailThrow ((InvalidBlock WitnessDeficient)))

def merkle_accumulator_push (accumulator : MerkleAccumulator) (leaf : hash) : SailM MerkleAccumulator := do
  if (((accumulator.count).value <b ((2 ^i 64) -i 1)) : Bool)
  then
    (pure { frontier := ← (merkle_push accumulator.frontier ⟨(accumulator.count).value⟩ leaf),
            count := ← do
                let semanticField ← (do
                    let semanticResult ← (item_count_increment ⟨(accumulator.count).value⟩)
                    pure ((semanticResult).value))
                pure (⟨semanticField⟩) })
  else sailThrow ((InvalidBlock WitnessDeficient))

/-- Type quantifiers: _reclimit : Nat, k_ex161876_ : Nat, k_ex161875_ : Nat, 0 ≤ k_ex161875_ ∧
  k_ex161875_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161876_ ∧ k_ex161876_ ≤ 64, 0 ≤ _reclimit -/
def _rec_merkle_root_levels (slots : (List MerkleSlot)) (n : item_count) (zero : hash) (acc : hash) (remaining : merkle_depth) (_reclimit : Nat) : SailM hash := do
  let n := (n).value
  let remaining := (remaining).value
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
            if (((Int.tmod n 2) == 1) : Bool)
            then
              (do
                match slot with
                | .OccupiedMerkleSlot left => (sha256_pair left acc)
                | .EmptyMerkleSlot () => sailThrow ((InvalidBlock WitnessDeficient)))
            else (sha256_pair acc zero)
          (_rec_merkle_root_levels rest
            ⟨((← (protocol_quantity_quotient ⟨n⟩ ⟨2⟩))).value⟩
            (← (sha256_pair zero zero)) next_acc ⟨(remaining -i 1)⟩ _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Type quantifiers: remaining : Nat, n : Nat, 0 ≤ n ∧ n ≤ (2 ^ 64 - 1), 0 ≤ remaining ∧
  remaining ≤ 64 -/
def merkle_root_levels (slots : (List MerkleSlot)) (n : item_count) (zero : hash) (acc : hash) (remaining : merkle_depth) : SailM hash := do
  let n := (n).value
  let remaining := (remaining).value
  let _measure := (remaining : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_merkle_root_levels slots ⟨n⟩ zero acc ⟨remaining⟩ (_measure + 1))

/-- Type quantifiers: k_ex161881_ : Nat, 0 ≤ k_ex161881_ ∧ k_ex161881_ ≤ 64 -/
def merkle_accumulator_root (accumulator : MerkleAccumulator) (depth : merkle_depth) : SailM hash := do
  let depth := (depth).value
  (merkle_root_levels accumulator.frontier ⟨(accumulator.count).value⟩ (BitVec.zero 256)
    (BitVec.zero 256) ⟨depth⟩)

def merkle_accumulate (leaves : (List hash)) (accumulator : MerkleAccumulator) : SailM MerkleAccumulator := do
  match leaves with
  | [] => (pure accumulator)
  | (leaf :: rest) => (merkle_accumulate rest (← (merkle_accumulator_push accumulator leaf)))

/-- Type quantifiers: k_ex161882_ : Nat, 0 ≤ k_ex161882_ ∧ k_ex161882_ ≤ 64 -/
def merkleize (leaves : (List hash)) (depth : merkle_depth) : SailM hash := do
  let depth := (depth).value
  (merkle_accumulator_root (← (merkle_accumulate leaves EMPTY_MERKLE_ACCUMULATOR)) ⟨depth⟩)

/-- Type quantifiers: v : Nat, 0 ≤ v ∧ v ≤ (2 ^ 64 - 1) -/
def htr_uint (v : protocol_quantity) : SailM hash := do
  let v := (v).value
  let value ← do (word_of_nat v)
  let acc : (BitVec 256) := (BitVec.zero 256)
  let loop_k_lower := 0
  let loop_k_upper := 7
  let mut loop_vars := acc
  for k in [loop_k_lower:loop_k_upper:1]i do
    let acc := loop_vars
    loop_vars :=
      let byte := (Sail.BitVec.extractLsb (value >>> (8 *i k)) 7 0)
      (acc ||| ((Sail.BitVec.zeroExtend byte 256) <<< (8 *i (31 -i k))))
  (pure loop_vars)

def htr_u256 (w : word) : hash := Id.run do
  let acc : (BitVec 256) := (BitVec.zero 256)
  let loop_k_lower := 0
  let loop_k_upper := 31
  let mut loop_vars := acc
  for k in [loop_k_lower:loop_k_upper:1]i do
    let acc := loop_vars
    loop_vars :=
      let byte := (Sail.BitVec.extractLsb (w >>> (8 *i k)) 7 0)
      (acc ||| ((Sail.BitVec.zeroExtend byte 256) <<< (8 *i (31 -i k))))
  (pure loop_vars)

def htr_bytes32 (b : (BitVec 256)) : hash :=
  b

def htr_addr (a : address) : hash :=
  ((Sail.BitVec.zeroExtend a 256) <<< 96)

/-- Type quantifiers: k_ex161885_ : Nat, 0 ≤ k_ex161885_ ∧ k_ex161885_ ≤ (2 ^ 64 - 1) -/
def mix_in_length (root : hash) (len : item_count) : SailM hash := do
  let len := (len).value
  (sha256_pair root (← (htr_uint ⟨len⟩)))

/-- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ (2 ^ 64 - 1) -/
def clog2 (n : item_count) : SailM merkle_depth := do
  let n := (n).value
  let semanticResult ← do
    let remaining : Nat :=
      if ((n == 0) : Bool)
      then 0
      else (n -i 1)
    let depth : Nat := 0
    let (depth, remaining) ← (( do
      let loop__step_lower := 0
      let loop__step_upper := 63
      let mut loop_vars := (depth, remaining)
      for _step in [loop__step_lower:loop__step_upper:1]i do
        let (depth, remaining) := loop_vars
        loop_vars ← do
          let (depth, remaining) ← (( do
            if ((remaining != 0) : Bool)
            then
              (do
                let depth ←
                  (do
                      let semanticResult ← (merkle_depth_increment ⟨depth⟩)
                      pure ((semanticResult).value))
                let remaining ←
                  (do
                      let semanticResult ← (protocol_quantity_quotient ⟨remaining⟩ ⟨2⟩)
                      pure ((semanticResult).value))
                (pure (depth, remaining)))
            else (pure (depth, remaining)) ) : SailM (Nat × Nat) )
          (pure (depth, remaining))
      (pure loop_vars) ) : SailM (Nat × Nat) )
    (pure depth)
  pure (⟨semanticResult⟩)

/-- Type quantifiers: k_ex161887_ : Nat, 0 ≤ k_ex161887_ ∧ k_ex161887_ ≤ (2 ^ 64 - 1) -/
def htr_chunk (bytes : EvmByteSlice) (chunk_index : item_index) : SailM hash := do
  let chunk_index := (chunk_index).value
  let acc : (BitVec 256) := (BitVec.zero 256)
  let start ← (( do (byte_quantity_mul (ByteQuantity chunk_index) WORD_BYTE_LENGTH) ) : SailM
    byte_quantity )
  let cursor : byte_quantity := start
  let (acc, cursor) ← (( do
    let loop_byte_index_lower := 0
    let loop_byte_index_upper := 31
    let mut loop_vars := (acc, cursor)
    for byte_index in [loop_byte_index_lower:loop_byte_index_upper:1]i do
      let (acc, cursor) := loop_vars
      loop_vars ← do
        let b ← (( do
          if ((byte_quantity_lt cursor bytes.len) : Bool)
          then (slice_byte bytes cursor)
          else (pure 0x00#8) ) : SailM (BitVec 8) )
        let acc : (BitVec 256) := ((acc <<< 8) ||| (Sail.BitVec.zeroExtend b 256))
        let cursor ← (( do
          if ((byte_index <b 31) : Bool)
          then
            (do
              (byte_quantity_add cursor BYTE_ONE))
          else (pure cursor) ) : SailM byte_quantity )
        (pure (acc, cursor))
    (pure loop_vars) ) : SailM ((BitVec 256) × byte_quantity) )
  (pure acc)

def htr_chunk_count (app_0 : byte_length) : SailM item_count := do
  let semanticResult ← do
    let .ByteQuantity byte_len := app_0
    let count ← do (exact_quotient (byte_len + 31) 32)
    if ((count ≤b ((2 ^i 64) -i 1)) : Bool)
    then (pure count)
    else sailThrow ((InvalidBlock InvalidConfig))
  pure (⟨semanticResult⟩)

def htr_fixed_count (typ_0 : byte_length) (typ_1 : byte_length) : SailM item_count := do
  let semanticResult ← do
    let .ByteQuantity byte_len : byte_quantity := typ_0
    let .ByteQuantity item_len : byte_quantity := typ_1
    let count ← do (exact_quotient byte_len item_len)
    if ((count ≤b ((2 ^i 64) -i 1)) : Bool)
    then (pure count)
    else sailThrow ((InvalidBlock InvalidConfig))
  pure (⟨semanticResult⟩)

/-- Type quantifiers: k_ex161888_ : Nat, 0 ≤ k_ex161888_ ∧ k_ex161888_ ≤ 64 -/
def htr_bytes_root (bytes : EvmByteSlice) (depth : merkle_depth) : SailM hash := do
  let depth := (depth).value
  let count ← do
    (do
        let semanticResult ← (htr_chunk_count bytes.len)
        pure ((semanticResult).value))
  let accumulator := EMPTY_MERKLE_ACCUMULATOR
  let index : Nat := 0
  let (accumulator, index) ← (( do
    let loop_vars ← whileFuelM (fuel :=(count -i index)) (fun (accumulator, index) => (pure (index <b count))) (accumulator, index)
      fun (accumulator, index) => do
        assert true "loop dummy assert"
        let accumulator ←
          (merkle_accumulator_push accumulator (← (htr_chunk bytes ⟨index⟩)))
        let index ←
          (do
              let semanticResult ← (item_index_increment ⟨index⟩)
              pure ((semanticResult).value))
        (pure (accumulator, index))
    (pure loop_vars) ) : SailM (MerkleAccumulator × Nat) )
  (merkle_accumulator_root accumulator ⟨depth⟩)

def htr_bytevector (bytes : EvmByteSlice) : SailM hash := do
  let chunks ← do
    (do
        let semanticResult ← (htr_chunk_count bytes.len)
        pure ((semanticResult).value))
  (htr_bytes_root bytes ⟨((← (clog2 ⟨chunks⟩))).value⟩)

def htr_bytelist (bytes : EvmByteSlice) (limit_bytes : byte_length) : SailM hash := do
  let capacity ← do
    (do
        let semanticResult ← (htr_chunk_count limit_bytes)
        pure ((semanticResult).value))
  let .ByteQuantity byte_len := bytes.len
  if ((byte_len ≤b ((2 ^i 64) -i 1)) : Bool)
  then
    (mix_in_length (← (htr_bytes_root bytes ⟨((← (clog2 ⟨capacity⟩))).value⟩))
      ⟨byte_len⟩)
  else sailThrow ((InvalidBlock InvalidConfig))

def htr_withdrawal (withdrawal : EvmByteSlice) : SailM hash := do
  (merkleize
    [(← (htr_uint ⟨((← (ssz_uint withdrawal WD_INDEX))).value⟩)), (← (htr_uint
        ⟨((← (ssz_uint withdrawal WD_VALIDATOR_INDEX))).value⟩)), (htr_addr
      (← (ssz_addr withdrawal WD_ADDRESS))), (← (htr_uint
        ⟨((← (ssz_uint withdrawal WD_AMOUNT))).value⟩))] ⟨2⟩)

def htr_transactions (transactions : SszListRef) : SailM hash := do
  let cursor ← do (ssz_list_cursor transactions)
  let accumulator := EMPTY_MERKLE_ACCUMULATOR
  let (accumulator, cursor) ← (( do
    let loop_vars ← whileFuelM (fuel :=((cursor.items.count).value -i (cursor.index).value)) (fun (accumulator, cursor) => (pure (! (ssz_list_cursor_empty
          cursor)))) (accumulator, cursor)
      fun (accumulator, cursor) => do
        assert true "loop dummy assert"
        let (transaction, next) ← do (ssz_list_pop cursor)
        let cursor : SszListCursor := next
        let accumulator ←
          (merkle_accumulator_push accumulator (← (htr_bytelist transaction HTR_BYTE_LIST_LIMIT)))
        (pure (accumulator, cursor))
    (pure loop_vars) ) : SailM (MerkleAccumulator × SszListCursor) )
  (mix_in_length (← (merkle_accumulator_root accumulator ⟨20⟩))
    ⟨(accumulator.count).value⟩)

def htr_withdrawals (withdrawals : SszListRef) : SailM hash := do
  let rest : SszListRef := withdrawals
  let accumulator := EMPTY_MERKLE_ACCUMULATOR
  let (accumulator, rest) ← (( do
    let loop_vars ← whileFuelM (fuel :=(rest.count).value) (fun (accumulator, rest) => (pure ((rest.count).value != 0))) (accumulator, rest)
      fun (accumulator, rest) => do
        assert true "loop dummy assert"
        let (withdrawal, tail) ← do (ssz_fixed_list_pop rest WD_SIZE)
        let rest : SszListRef := tail
        let accumulator ← (merkle_accumulator_push accumulator (← (htr_withdrawal withdrawal)))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × SszListRef) )
  (mix_in_length (← (merkle_accumulator_root accumulator ⟨4⟩)) ⟨(accumulator.count).value⟩)

def htr_execution_payload (input_ref : StatelessInputRef) : SailM hash := do
  let payload := input_ref.execution_payload
  (merkleize
    [(htr_bytes32 (← (ssz_bytes32 payload BYTE_ZERO))), (htr_addr
      (← (ssz_addr payload PL_FEE_RECIPIENT))), (htr_bytes32
      (← (ssz_bytes32 payload PL_STATE_ROOT))), (htr_bytes32
      (← (ssz_bytes32 payload PL_RECEIPTS_ROOT))), (← (htr_bytevector
        (← (sub_slice payload PL_LOGS_BLOOM LOGS_BLOOM_BYTE_LENGTH)))), (htr_bytes32
      (← (ssz_bytes32 payload PL_PREV_RANDAO))), (← (htr_uint
        ⟨((← (ssz_uint payload PL_BLOCK_NUMBER))).value⟩)), (← (htr_uint
        ⟨((← (ssz_uint payload PL_GAS_LIMIT))).value⟩)), (← (htr_uint
        ⟨((← (ssz_uint payload PL_GAS_USED))).value⟩)), (← (htr_uint
        ⟨((← (ssz_uint payload PL_TIMESTAMP))).value⟩)), (← (htr_bytelist
        input_ref.extra_data WORD_BYTE_LENGTH)), (htr_u256 (← (ssz_u256 payload PL_BASE_FEE))), (htr_bytes32
      (← (ssz_bytes32 payload PL_BLOCK_HASH))), (← (htr_transactions input_ref.transactions)), (← (htr_withdrawals
        input_ref.withdrawals)), (← (htr_uint
        ⟨((← (ssz_uint payload PL_BLOB_GAS_USED))).value⟩)), (← (htr_uint
        ⟨((← (ssz_uint payload PL_EXCESS_BLOB_GAS))).value⟩)), (← (htr_bytelist
        input_ref.block_access_list HTR_BYTE_LIST_LIMIT)), (← (htr_uint
        ⟨((← (ssz_uint payload PL_SLOT_NUMBER))).value⟩))] ⟨5⟩)

def htr_versioned_hashes (versioned_hashes : EvmByteSlice) : SailM hash := do
  let count ← do
    (do
        let semanticResult ← (htr_fixed_count versioned_hashes.len WORD_BYTE_LENGTH)
        pure ((semanticResult).value))
  let rest : SszListRef :=
    { bytes := versioned_hashes,
      count := ⟨count⟩ }
  let accumulator := EMPTY_MERKLE_ACCUMULATOR
  let (accumulator, rest) ← (( do
    let loop_vars ← whileFuelM (fuel :=(rest.count).value) (fun (accumulator, rest) => (pure ((rest.count).value != 0))) (accumulator, rest)
      fun (accumulator, rest) => do
        assert true "loop dummy assert"
        let (versioned_hash, tail) ← do (ssz_fixed_list_pop rest WORD_BYTE_LENGTH)
        let rest : SszListRef := tail
        let accumulator ←
          (merkle_accumulator_push accumulator (← (slice_load versioned_hash BYTE_ZERO)))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × SszListRef) )
  (mix_in_length (← (merkle_accumulator_root accumulator ⟨12⟩))
    ⟨(accumulator.count).value⟩)

def htr_deposit (deposit : EvmByteSlice) : SailM hash := do
  (merkleize
    [(← (htr_bytevector (← (sub_slice deposit HTR_DEPOSIT_PUBKEY HTR_DEPOSIT_PUBKEY_LENGTH)))), (htr_bytes32
      (← (ssz_bytes32 deposit HTR_DEPOSIT_WITHDRAWAL_CREDENTIALS))), (← (htr_uint
        ⟨((← (ssz_uint deposit HTR_DEPOSIT_AMOUNT))).value⟩)), (← (htr_bytevector
        (← (sub_slice deposit HTR_DEPOSIT_SIGNATURE HTR_DEPOSIT_SIGNATURE_LENGTH)))), (← (htr_uint
        ⟨((← (ssz_uint deposit HTR_DEPOSIT_INDEX))).value⟩))] ⟨3⟩)

def htr_withdrawal_request (request : EvmByteSlice) : SailM hash := do
  (merkleize
    [(← (htr_bytevector (← (sub_slice request HTR_REQUEST_SOURCE_ADDRESS ADDRESS_BYTE_LENGTH)))), (← (htr_bytevector
        (← (sub_slice request HTR_REQUEST_SOURCE_PUBKEY HTR_REQUEST_PUBKEY_LENGTH)))), (← (htr_uint
        ⟨((← (ssz_uint request HTR_WITHDRAWAL_REQUEST_AMOUNT))).value⟩))] ⟨2⟩)

def htr_consolidation_request (request : EvmByteSlice) : SailM hash := do
  (merkleize
    [(← (htr_bytevector (← (sub_slice request HTR_REQUEST_SOURCE_ADDRESS ADDRESS_BYTE_LENGTH)))), (← (htr_bytevector
        (← (sub_slice request HTR_REQUEST_SOURCE_PUBKEY HTR_REQUEST_PUBKEY_LENGTH)))), (← (htr_bytevector
        (← (sub_slice request HTR_CONSOLIDATION_REQUEST_TARGET_PUBKEY HTR_REQUEST_PUBKEY_LENGTH))))]
    ⟨2⟩)

def htr_deposits (deposits : EvmByteSlice) : SailM hash := do
  let count ← do
    (do
        let semanticResult ← (htr_fixed_count deposits.len HTR_DEPOSIT_LENGTH)
        pure ((semanticResult).value))
  let rest : SszListRef :=
    { bytes := deposits,
      count := ⟨count⟩ }
  let accumulator := EMPTY_MERKLE_ACCUMULATOR
  let (accumulator, rest) ← (( do
    let loop_vars ← whileFuelM (fuel :=(rest.count).value) (fun (accumulator, rest) => (pure ((rest.count).value != 0))) (accumulator, rest)
      fun (accumulator, rest) => do
        assert true "loop dummy assert"
        let (deposit, tail) ← do (ssz_fixed_list_pop rest HTR_DEPOSIT_LENGTH)
        let rest : SszListRef := tail
        let accumulator ← (merkle_accumulator_push accumulator (← (htr_deposit deposit)))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × SszListRef) )
  (mix_in_length (← (merkle_accumulator_root accumulator ⟨13⟩))
    ⟨(accumulator.count).value⟩)

def htr_withdrawal_requests (requests : EvmByteSlice) : SailM hash := do
  let count ← do
    (do
        let semanticResult ← (htr_fixed_count requests.len HTR_WITHDRAWAL_REQUEST_LENGTH)
        pure ((semanticResult).value))
  let rest : SszListRef :=
    { bytes := requests,
      count := ⟨count⟩ }
  let accumulator := EMPTY_MERKLE_ACCUMULATOR
  let (accumulator, rest) ← (( do
    let loop_vars ← whileFuelM (fuel :=(rest.count).value) (fun (accumulator, rest) => (pure ((rest.count).value != 0))) (accumulator, rest)
      fun (accumulator, rest) => do
        assert true "loop dummy assert"
        let (request, tail) ← do (ssz_fixed_list_pop rest HTR_WITHDRAWAL_REQUEST_LENGTH)
        let rest : SszListRef := tail
        let accumulator ←
          (merkle_accumulator_push accumulator (← (htr_withdrawal_request request)))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × SszListRef) )
  (mix_in_length (← (merkle_accumulator_root accumulator ⟨4⟩)) ⟨(accumulator.count).value⟩)

def htr_consolidation_requests (requests : EvmByteSlice) : SailM hash := do
  let count ← do
    (do
        let semanticResult ← (htr_fixed_count requests.len HTR_CONSOLIDATION_REQUEST_LENGTH)
        pure ((semanticResult).value))
  let rest : SszListRef :=
    { bytes := requests,
      count := ⟨count⟩ }
  let accumulator := EMPTY_MERKLE_ACCUMULATOR
  let (accumulator, rest) ← (( do
    let loop_vars ← whileFuelM (fuel :=(rest.count).value) (fun (accumulator, rest) => (pure ((rest.count).value != 0))) (accumulator, rest)
      fun (accumulator, rest) => do
        assert true "loop dummy assert"
        let (request, tail) ← do (ssz_fixed_list_pop rest HTR_CONSOLIDATION_REQUEST_LENGTH)
        let rest : SszListRef := tail
        let accumulator ←
          (merkle_accumulator_push accumulator (← (htr_consolidation_request request)))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × SszListRef) )
  (mix_in_length (← (merkle_accumulator_root accumulator ⟨1⟩)) ⟨(accumulator.count).value⟩)

def htr_execution_requests (input_ref : StatelessInputRef) : SailM hash := do
  (merkleize
    [(← (htr_deposits input_ref.deposits)), (← (htr_withdrawal_requests
        input_ref.withdrawal_requests)), (← (htr_consolidation_requests
        input_ref.consolidation_requests))] ⟨2⟩)

def htr_new_payload_request (input_ref : StatelessInputRef) : SailM hash := do
  (merkleize
    [(← (htr_execution_payload input_ref)), (← (htr_versioned_hashes input_ref.versioned_hashes)), (htr_bytes32
      (← (ssz_bytes32 input_ref.new_payload_request NPR_BEACON_ROOT))), (← (htr_execution_requests
        input_ref))] ⟨2⟩)

