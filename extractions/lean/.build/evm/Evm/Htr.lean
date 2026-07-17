import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Quantities
import Evm.Bytes
import Evm.Crypto
import Evm.ByteSlice
import Evm.Ssz
import Evm.StatelessInput0
import Evm.Receipts

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open ConcurrencyInterfaceV1

namespace Evm.Functions

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
  { frontier := []
    count := 0 }

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

/-- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ (2 ^ 64 - 1) -/
def merkle_push (frontier : (List MerkleSlot)) (count : Nat) (leaf : (BitVec 256)) : SailM (List MerkleSlot) := do
  if (((Int.tmod count 2) == 0) : Bool)
  then
    (match frontier with
    | (_ :: rest) => ((OccupiedMerkleSlot leaf) :: rest)
    | [] => [(pure (OccupiedMerkleSlot leaf))])
  else
    (do
      match frontier with
      | (.OccupiedMerkleSlot left :: rest) =>
        ((EmptyMerkleSlot ()) :: (← (merkle_push rest (← (protocol_quantity_quotient count 2))
            (← (sha256_pair left leaf)))))
      | (.EmptyMerkleSlot () :: _) => sailThrow ((InvalidBlock WitnessDeficient))
      | [] => sailThrow ((InvalidBlock WitnessDeficient)))

def merkle_accumulator_push (accumulator : MerkleAccumulator) (leaf : (BitVec 256)) : SailM MerkleAccumulator := do
  if ((accumulator.count <b ((2 ^i 64) -i 1)) : Bool)
  then
    (pure { frontier := ← (merkle_push accumulator.frontier accumulator.count leaf)
            count := ← (item_count_increment accumulator.count) })
  else sailThrow ((InvalidBlock WitnessDeficient))

/-- Type quantifiers: remaining : Nat, n : Nat, 0 ≤ n ∧ n ≤ (2 ^ 64 - 1), 0 ≤ remaining ∧
  remaining ≤ 64 -/
def merkle_root_levels (slots : (List MerkleSlot)) (n : Nat) (zero : (BitVec 256)) (acc : (BitVec 256)) (remaining : Nat) : SailM (BitVec 256) := do
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
      (merkle_root_levels rest (← (protocol_quantity_quotient n 2)) (← (sha256_pair zero zero))
        next_acc (← (merkle_depth_decrement remaining))))
termination_by let (slots, n, zero, acc, remaining) := (slots, n, zero, acc, remaining); (remaining).toNat

/-- Type quantifiers: depth : Nat, 0 ≤ depth ∧ depth ≤ 64 -/
def merkle_accumulator_root (accumulator : MerkleAccumulator) (depth : Nat) : SailM (BitVec 256) := do
  (merkle_root_levels accumulator.frontier accumulator.count (BitVec.zero 256) (BitVec.zero 256)
    depth)

def merkle_accumulate (leaves : (List (BitVec 256))) (accumulator : MerkleAccumulator) : SailM MerkleAccumulator := do
  match leaves with
  | [] => (pure accumulator)
  | (leaf :: rest) => (merkle_accumulate rest (← (merkle_accumulator_push accumulator leaf)))

/-- Type quantifiers: depth : Nat, 0 ≤ depth ∧ depth ≤ 64 -/
def merkleize (leaves : (List (BitVec 256))) (depth : Nat) : SailM (BitVec 256) := do
  (merkle_accumulator_root (← (merkle_accumulate leaves EMPTY_MERKLE_ACCUMULATOR)) depth)

/-- Type quantifiers: v : Nat, 0 ≤ v ∧ v ≤ (2 ^ 64 - 1) -/
def htr_uint (v : Nat) : SailM (BitVec 256) := do
  let value ← do (word_of_nat v)
  let acc : hash := (BitVec.zero 256)
  let loop_k_lower := 0
  let loop_k_upper := 7
  let mut loop_vars := acc
  for k in [loop_k_lower:loop_k_upper:1]i do
    let acc := loop_vars
    loop_vars :=
      let byte := (Sail.BitVec.extractLsb (value >>> (8 *i k)) 7 0)
      (acc ||| ((Sail.BitVec.zeroExtend byte 256) <<< (8 *i (31 -i k))))
  (pure loop_vars)

def htr_u256 (w : (BitVec 256)) : (BitVec 256) := Id.run do
  let acc : hash := (BitVec.zero 256)
  let loop_k_lower := 0
  let loop_k_upper := 31
  let mut loop_vars := acc
  for k in [loop_k_lower:loop_k_upper:1]i do
    let acc := loop_vars
    loop_vars :=
      let byte := (Sail.BitVec.extractLsb (w >>> (8 *i k)) 7 0)
      (acc ||| ((Sail.BitVec.zeroExtend byte 256) <<< (8 *i (31 -i k))))
  (pure loop_vars)

def htr_bytes32 (b : (BitVec 256)) : (BitVec 256) :=
  b

def htr_addr (a : (BitVec 160)) : (BitVec 256) :=
  ((Sail.BitVec.zeroExtend a 256) <<< 96)

/-- Type quantifiers: len : Nat, 0 ≤ len ∧ len ≤ (2 ^ 64 - 1) -/
def mix_in_length (root : (BitVec 256)) (len : Nat) : SailM (BitVec 256) := do
  (sha256_pair root (← (htr_uint len)))

/-- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ (2 ^ 64 - 1) -/
def clog2 (n : Nat) : SailM Nat := do
  let remaining : item_count :=
    if ((n == 0) : Bool)
    then 0
    else (n -i 1)
  let depth : merkle_depth := 0
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
              let depth ← (merkle_depth_increment depth)
              let remaining ← (protocol_quantity_quotient remaining 2)
              (pure (depth, remaining)))
          else (pure (depth, remaining)) ) : SailM (Nat × Nat) )
        (pure (depth, remaining))
    (pure loop_vars) ) : SailM (Nat × Nat) )
  (pure depth)

/-- Type quantifiers: chunk_index : Nat, 0 ≤ chunk_index ∧ chunk_index ≤ (2 ^ 64 - 1) -/
def htr_chunk (bytes : ByteSlice) (chunk_index : Nat) : SailM (BitVec 256) := do
  let acc : hash := (BitVec.zero 256)
  let start ← (( do (byte_quantity_mul (ByteQuantity chunk_index) WORD_BYTE_LENGTH) ) : SailM
    source_pointer )
  let cursor : source_pointer := start
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
          else (pure 0x00#8) ) : SailM byte )
        let acc : hash := ((acc <<< 8) ||| (Sail.BitVec.zeroExtend b 256))
        let cursor ← (( do
          if ((byte_index <b 31) : Bool)
          then
            (do
              (byte_quantity_add cursor BYTE_ONE))
          else (pure cursor) ) : SailM byte_quantity )
        (pure (acc, cursor))
    (pure loop_vars) ) : SailM ((BitVec 256) × byte_quantity) )
  (pure acc)

def htr_chunk_count (app_0 : byte_quantity) : SailM Nat := do
  let .ByteQuantity byte_len := app_0
  let count ← do (exact_quotient (byte_len +i 31) 32)
  if ((count ≤b ((2 ^i 64) -i 1)) : Bool)
  then (pure count)
  else sailThrow ((InvalidBlock InvalidConfig))

def htr_fixed_count (typ_0 : byte_quantity) (typ_1 : byte_quantity) : SailM Nat := do
  let .ByteQuantity byte_len : byte_length := typ_0
  let .ByteQuantity item_len : byte_length := typ_1
  let count ← do (exact_quotient byte_len item_len)
  if ((count ≤b ((2 ^i 64) -i 1)) : Bool)
  then (pure count)
  else sailThrow ((InvalidBlock InvalidConfig))

/-- Type quantifiers: depth : Nat, 0 ≤ depth ∧ depth ≤ 64 -/
def htr_bytes_root (bytes : ByteSlice) (depth : Nat) : SailM (BitVec 256) := do
  let count ← do (htr_chunk_count bytes.len)
  let accumulator := EMPTY_MERKLE_ACCUMULATOR
  let index : item_index := 0
  let (accumulator, index) ← (( do
    let mut loop_vars := (accumulator, index)
    while (λ (accumulator, index) => (index <b count)) loop_vars do
      let (accumulator, index) := loop_vars
      loop_vars ← do
        let accumulator ← (merkle_accumulator_push accumulator (← (htr_chunk bytes index)))
        let index ← (item_index_increment index)
        (pure (accumulator, index))
    (pure loop_vars) ) : SailM (MerkleAccumulator × Nat) )
  (merkle_accumulator_root accumulator depth)

def htr_bytevector (bytes : ByteSlice) : SailM (BitVec 256) := do
  let chunks ← do (htr_chunk_count bytes.len)
  (htr_bytes_root bytes (← (clog2 chunks)))

def htr_bytelist (bytes : ByteSlice) (limit_bytes : byte_quantity) : SailM (BitVec 256) := do
  let capacity ← do (htr_chunk_count limit_bytes)
  let .ByteQuantity byte_len := bytes.len
  if ((byte_len ≤b ((2 ^i 64) -i 1)) : Bool)
  then (mix_in_length (← (htr_bytes_root bytes (← (clog2 capacity)))) byte_len)
  else sailThrow ((InvalidBlock InvalidConfig))

def htr_withdrawal (withdrawal : ByteSlice) : SailM (BitVec 256) := do
  (merkleize
    [(← (htr_uint (← (ssz_uint withdrawal WD_INDEX)))), (← (htr_uint
        (← (ssz_uint withdrawal WD_VALIDATOR_INDEX)))), (htr_addr
      (← (ssz_addr withdrawal WD_ADDRESS))), (← (htr_uint (← (ssz_uint withdrawal WD_AMOUNT))))]
    2)

def htr_transactions (transactions : SszListRef) : SailM (BitVec 256) := do
  let cursor ← do (ssz_list_cursor transactions)
  let accumulator := EMPTY_MERKLE_ACCUMULATOR
  let (accumulator, cursor) ← (( do
    let mut loop_vars := (accumulator, cursor)
    while (λ (accumulator, cursor) => (! (ssz_list_cursor_empty cursor))) loop_vars do
      let (accumulator, cursor) := loop_vars
      loop_vars ← do
        let (transaction, next) ← do (ssz_list_pop cursor)
        let cursor : SszListCursor := next
        let accumulator ←
          (merkle_accumulator_push accumulator (← (htr_bytelist transaction HTR_BYTE_LIST_LIMIT)))
        (pure (accumulator, cursor))
    (pure loop_vars) ) : SailM (MerkleAccumulator × SszListCursor) )
  (mix_in_length (← (merkle_accumulator_root accumulator 20)) accumulator.count)

def htr_withdrawals (withdrawals : SszListRef) : SailM (BitVec 256) := do
  let rest : SszListRef := withdrawals
  let accumulator := EMPTY_MERKLE_ACCUMULATOR
  let (accumulator, rest) ← (( do
    let mut loop_vars := (accumulator, rest)
    while (λ (accumulator, rest) => (rest.count != 0)) loop_vars do
      let (accumulator, rest) := loop_vars
      loop_vars ← do
        let (withdrawal, tail) ← do (ssz_fixed_list_pop rest WD_SIZE)
        let rest : SszListRef := tail
        let accumulator ← (merkle_accumulator_push accumulator (← (htr_withdrawal withdrawal)))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × SszListRef) )
  (mix_in_length (← (merkle_accumulator_root accumulator 4)) accumulator.count)

def htr_execution_payload (input_ref : StatelessInputRef) : SailM (BitVec 256) := do
  let payload := input_ref.execution_payload
  (merkleize
    [(htr_bytes32 (← (ssz_bytes32 payload BYTE_ZERO))), (htr_addr
      (← (ssz_addr payload PL_FEE_RECIPIENT))), (htr_bytes32
      (← (ssz_bytes32 payload PL_STATE_ROOT))), (htr_bytes32
      (← (ssz_bytes32 payload PL_RECEIPTS_ROOT))), (← (htr_bytevector
        (← (sub_slice payload PL_LOGS_BLOOM LOGS_BLOOM_BYTE_LENGTH)))), (htr_bytes32
      (← (ssz_bytes32 payload PL_PREV_RANDAO))), (← (htr_uint
        (← (ssz_uint payload PL_BLOCK_NUMBER)))), (← (htr_uint
        (← (ssz_uint payload PL_GAS_LIMIT)))), (← (htr_uint (← (ssz_uint payload PL_GAS_USED)))), (← (htr_uint
        (← (ssz_uint payload PL_TIMESTAMP)))), (← (htr_bytelist input_ref.extra_data
        WORD_BYTE_LENGTH)), (htr_u256 (← (ssz_u256 payload PL_BASE_FEE))), (htr_bytes32
      (← (ssz_bytes32 payload PL_BLOCK_HASH))), (← (htr_transactions input_ref.transactions)), (← (htr_withdrawals
        input_ref.withdrawals)), (← (htr_uint (← (ssz_uint payload PL_BLOB_GAS_USED)))), (← (htr_uint
        (← (ssz_uint payload PL_EXCESS_BLOB_GAS)))), (← (htr_bytelist
        input_ref.block_access_list HTR_BYTE_LIST_LIMIT)), (← (htr_uint
        (← (ssz_uint payload PL_SLOT_NUMBER))))] 5)

def htr_versioned_hashes (versioned_hashes : ByteSlice) : SailM (BitVec 256) := do
  let count ← do (htr_fixed_count versioned_hashes.len WORD_BYTE_LENGTH)
  let rest : SszListRef :=
    { bytes := versioned_hashes
      count := count }
  let accumulator := EMPTY_MERKLE_ACCUMULATOR
  let (accumulator, rest) ← (( do
    let mut loop_vars := (accumulator, rest)
    while (λ (accumulator, rest) => (rest.count != 0)) loop_vars do
      let (accumulator, rest) := loop_vars
      loop_vars ← do
        let (versioned_hash, tail) ← do (ssz_fixed_list_pop rest WORD_BYTE_LENGTH)
        let rest : SszListRef := tail
        let accumulator ←
          (merkle_accumulator_push accumulator (← (slice_load versioned_hash BYTE_ZERO)))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × SszListRef) )
  (mix_in_length (← (merkle_accumulator_root accumulator 12)) accumulator.count)

def htr_deposit (deposit : ByteSlice) : SailM (BitVec 256) := do
  (merkleize
    [(← (htr_bytevector (← (sub_slice deposit HTR_DEPOSIT_PUBKEY HTR_DEPOSIT_PUBKEY_LENGTH)))), (htr_bytes32
      (← (ssz_bytes32 deposit HTR_DEPOSIT_WITHDRAWAL_CREDENTIALS))), (← (htr_uint
        (← (ssz_uint deposit HTR_DEPOSIT_AMOUNT)))), (← (htr_bytevector
        (← (sub_slice deposit HTR_DEPOSIT_SIGNATURE HTR_DEPOSIT_SIGNATURE_LENGTH)))), (← (htr_uint
        (← (ssz_uint deposit HTR_DEPOSIT_INDEX))))] 3)

def htr_withdrawal_request (request : ByteSlice) : SailM (BitVec 256) := do
  (merkleize
    [(← (htr_bytevector (← (sub_slice request HTR_REQUEST_SOURCE_ADDRESS ADDRESS_BYTE_LENGTH)))), (← (htr_bytevector
        (← (sub_slice request HTR_REQUEST_SOURCE_PUBKEY HTR_REQUEST_PUBKEY_LENGTH)))), (← (htr_uint
        (← (ssz_uint request HTR_WITHDRAWAL_REQUEST_AMOUNT))))] 2)

def htr_consolidation_request (request : ByteSlice) : SailM (BitVec 256) := do
  (merkleize
    [(← (htr_bytevector (← (sub_slice request HTR_REQUEST_SOURCE_ADDRESS ADDRESS_BYTE_LENGTH)))), (← (htr_bytevector
        (← (sub_slice request HTR_REQUEST_SOURCE_PUBKEY HTR_REQUEST_PUBKEY_LENGTH)))), (← (htr_bytevector
        (← (sub_slice request HTR_CONSOLIDATION_REQUEST_TARGET_PUBKEY HTR_REQUEST_PUBKEY_LENGTH))))]
    2)

def htr_deposits (deposits : ByteSlice) : SailM (BitVec 256) := do
  let count ← do (htr_fixed_count deposits.len HTR_DEPOSIT_LENGTH)
  let rest : SszListRef :=
    { bytes := deposits
      count := count }
  let accumulator := EMPTY_MERKLE_ACCUMULATOR
  let (accumulator, rest) ← (( do
    let mut loop_vars := (accumulator, rest)
    while (λ (accumulator, rest) => (rest.count != 0)) loop_vars do
      let (accumulator, rest) := loop_vars
      loop_vars ← do
        let (deposit, tail) ← do (ssz_fixed_list_pop rest HTR_DEPOSIT_LENGTH)
        let rest : SszListRef := tail
        let accumulator ← (merkle_accumulator_push accumulator (← (htr_deposit deposit)))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × SszListRef) )
  (mix_in_length (← (merkle_accumulator_root accumulator 13)) accumulator.count)

def htr_withdrawal_requests (requests : ByteSlice) : SailM (BitVec 256) := do
  let count ← do (htr_fixed_count requests.len HTR_WITHDRAWAL_REQUEST_LENGTH)
  let rest : SszListRef :=
    { bytes := requests
      count := count }
  let accumulator := EMPTY_MERKLE_ACCUMULATOR
  let (accumulator, rest) ← (( do
    let mut loop_vars := (accumulator, rest)
    while (λ (accumulator, rest) => (rest.count != 0)) loop_vars do
      let (accumulator, rest) := loop_vars
      loop_vars ← do
        let (request, tail) ← do (ssz_fixed_list_pop rest HTR_WITHDRAWAL_REQUEST_LENGTH)
        let rest : SszListRef := tail
        let accumulator ←
          (merkle_accumulator_push accumulator (← (htr_withdrawal_request request)))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × SszListRef) )
  (mix_in_length (← (merkle_accumulator_root accumulator 4)) accumulator.count)

def htr_consolidation_requests (requests : ByteSlice) : SailM (BitVec 256) := do
  let count ← do (htr_fixed_count requests.len HTR_CONSOLIDATION_REQUEST_LENGTH)
  let rest : SszListRef :=
    { bytes := requests
      count := count }
  let accumulator := EMPTY_MERKLE_ACCUMULATOR
  let (accumulator, rest) ← (( do
    let mut loop_vars := (accumulator, rest)
    while (λ (accumulator, rest) => (rest.count != 0)) loop_vars do
      let (accumulator, rest) := loop_vars
      loop_vars ← do
        let (request, tail) ← do (ssz_fixed_list_pop rest HTR_CONSOLIDATION_REQUEST_LENGTH)
        let rest : SszListRef := tail
        let accumulator ←
          (merkle_accumulator_push accumulator (← (htr_consolidation_request request)))
        (pure (accumulator, rest))
    (pure loop_vars) ) : SailM (MerkleAccumulator × SszListRef) )
  (mix_in_length (← (merkle_accumulator_root accumulator 1)) accumulator.count)

def htr_execution_requests (input_ref : StatelessInputRef) : SailM (BitVec 256) := do
  (merkleize
    [(← (htr_deposits input_ref.deposits)), (← (htr_withdrawal_requests
        input_ref.withdrawal_requests)), (← (htr_consolidation_requests
        input_ref.consolidation_requests))] 2)

def htr_new_payload_request (input_ref : StatelessInputRef) : SailM (BitVec 256) := do
  (merkleize
    [(← (htr_execution_payload input_ref)), (← (htr_versioned_hashes input_ref.versioned_hashes)), (htr_bytes32
      (← (ssz_bytes32 input_ref.new_payload_request NPR_BEACON_ROOT))), (← (htr_execution_requests
        input_ref))] 2)

