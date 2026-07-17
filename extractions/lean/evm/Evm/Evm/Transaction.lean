import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Primitives.Code
import Evm.Primitives.Crypto
import Evm.Primitives.Tx
import Evm.Host.EvmByteSlice
import Evm.Host.Code
import Evm.Lib.Tx
import Evm.Lib.Rlp.Tx
import Evm.Host.Kernel.Environment
import Evm.Host.Kernel.Storage
import Evm.Host.Kernel.Accounts
import Evm.Host.Kernel.Code
import Evm.Host.Kernel.Selfdestruct
import Evm.Host.Kernel.Lifecycle
import Evm.Evm.Machine
import Evm.Evm.Gas
import Evm.Evm.Precompiles
import Evm.Evm.Execute

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

def G_transaction : gas_constant := (GasConstant 21000)

def G_txcreate : gas_constant := (GasConstant 32000)

def G_txdatazero : gas_constant := (GasConstant 4)

def G_txdatanonzero : gas_constant := (GasConstant 16)

def G_access_list_address : gas_constant := (GasConstant 2400)

def G_access_list_storage_key : gas_constant := (GasConstant 1900)

def PER_AUTH_BASE : gas_constant := (GasConstant 12500)

def PER_EMPTY_ACCOUNT : gas_constant := (GasConstant 25000)

def calldata_cost (input : EvmByteSlice) : SailM gas_cost := do
  let .ByteQuantity nonzeroes ← do (slice_count_nonzero input)
  let .ByteQuantity input_len := input.len
  let zeroes ← (( do
    if ((nonzeroes ≤b input_len) : Bool)
    then (pure (input_len -i nonzeroes))
    else sailThrow ((InvalidBlock ExecutionInvalid)) ) : SailM Nat )
  (pure (gas_cost_add (gas_constant_scale G_txdatazero zeroes)
      (gas_constant_scale G_txdatanonzero nonzeroes)))

def calldata_tokens (input : EvmByteSlice) : SailM Nat := do
  let .ByteQuantity nonzeroes ← do (slice_count_nonzero input)
  let .ByteQuantity input_len := input.len
  let zeroes ← (( do
    if ((nonzeroes ≤b input_len) : Bool)
    then (pure (input_len -i nonzeroes))
    else sailThrow ((InvalidBlock ExecutionInvalid)) ) : SailM Nat )
  (pure (zeroes + (4 *i nonzeroes)))

def blob_hashes_versioned (hashes : BlobHashes) : SailM Bool := do
  let valid : Bool := true
  let remaining : Nat := (hashes.count).value
  let offset : byte_quantity := BYTE_ONE
  let (offset, remaining, valid) ← (( do
    let loop_i_lower := 0
    let loop_i_upper := 8
    let mut loop_vars := (offset, remaining, valid)
    for i in [loop_i_lower:loop_i_upper:1]i do
      let (offset, remaining, valid) := loop_vars
      loop_vars ← do
        let (offset, remaining, valid) ← (( do
          if ((valid && ((remaining != 0) : Bool)) : Bool)
          then
            (do
              let valid ← (pure ((← (slice_byte hashes.bytes offset)) == 0x01#8))
              let remaining ←
                (do
                    let semanticResult ← (protocol_quantity_decrement ⟨remaining⟩)
                    pure ((semanticResult).value))
              let offset ← (( do
                if ((i <b 8) : Bool)
                then
                  (do
                    (byte_quantity_add offset BLOB_HASH_RLP_LENGTH))
                else (pure offset) ) : SailM byte_quantity )
              (pure (offset, remaining, valid)))
          else (pure (offset, remaining, valid)) ) : SailM (byte_quantity × Nat × Bool) )
        (pure (offset, remaining, valid))
    (pure loop_vars) ) : SailM (byte_quantity × Nat × Bool) )
  (pure (valid && ((remaining == 0) : Bool)))

def intrinsic_gas (tx : Transaction) : SailM gas_cost := do
  let data_cost ← do (calldata_cost tx.input_src)
  let .ByteQuantity input_len := tx.input_src.len
  let address_cost :=
    (gas_constant_scale G_access_list_address (tx.access_list_address_count).value)
  let slot_cost := (gas_constant_scale G_access_list_storage_key (tx.access_list_slot_count).value)
  let auth_cost := (gas_constant_scale PER_EMPTY_ACCOUNT (tx.authorization_count).value)
  let total : gas_cost := (gas_cost_add_constant data_cost G_transaction)
  let total : gas_cost := (gas_cost_add total address_cost)
  let total : gas_cost := (gas_cost_add total slot_cost)
  let total : gas_cost := (gas_cost_add total auth_cost)
  if (tx.is_create : Bool)
  then
    (do
      let total : gas_cost := (gas_cost_add_constant total G_txcreate)
      (pure (gas_cost_add total (← (initcode_gas input_len)))))
  else (pure total)

def calldata_floor (input : EvmByteSlice) : SailM gas_cost := do
  (pure (gas_cost_add_constant (gas_constant_scale (GasConstant 10) (← (calldata_tokens input)))
      G_transaction))

def transaction_costs (tx : Transaction) (blob_price : word) : SailM (Option TransactionCosts) := SailME.run do
  let intrinsic ← do (intrinsic_gas tx)
  let floor ← do (calldata_floor tx.input_src)
  let blob_gas_value := ((tx.blob_hashes.count).value *i (GAS_PER_BLOB).value)
  let blob_gas ← (( do
    if ((blob_gas_value ≤b ((2 ^i 64) -i 1)) : Bool)
    then (pure blob_gas_value)
    else SailME.throw (none : (Option TransactionCosts)) ) : SailME (Option TransactionCosts) Nat )
  let blob_fee ← (( do
    if ((blob_gas == 0) : Bool)
    then (pure ZERO_WORD)
    else
      (do
        match (← (word_checked_mul_protocol_quantity blob_price ⟨blob_gas⟩)) with
        | .some amount => (pure amount)
        | none => SailME.throw (none : (Option TransactionCosts))) ) : SailME
    (Option TransactionCosts) (BitVec 256) )
  let gas_cap ← (( do
    match (← (word_checked_mul_gas tx.max_fee tx.gas_limit)) with
    | .some amount => (pure amount)
    | none => SailME.throw (none : (Option TransactionCosts)) ) : SailME (Option TransactionCosts)
    (BitVec 256) )
  let blob_cap ← (( do
    match (← (word_checked_mul_protocol_quantity tx.max_blob_fee ⟨blob_gas⟩)) with
    | .some amount => (pure amount)
    | none => SailME.throw (none : (Option TransactionCosts)) ) : SailME (Option TransactionCosts)
    (BitVec 256) )
  let gas_and_value ← (( do
    match (word_checked_add gas_cap tx.value) with
    | .some amount => (pure amount)
    | none => SailME.throw (none : (Option TransactionCosts)) ) : SailME (Option TransactionCosts)
    (BitVec 256) )
  let upfront ← (( do
    match (word_checked_add gas_and_value blob_cap) with
    | .some amount => (pure amount)
    | none => SailME.throw (none : (Option TransactionCosts)) ) : SailME (Option TransactionCosts)
    (BitVec 256) )
  (pure (some
      { intrinsic := intrinsic,
        calldata_floor := floor,
        blob_gas := ⟨blob_gas⟩,
        blob_fee := blob_fee,
        upfront := upfront }))

def validated_word_product (value : word) (factor : gas) : SailM word := do
  match (← (word_checked_mul_gas value factor)) with
  | .some product => (pure product)
  | none => sailThrow ((InvalidBlock ExecutionInvalid))

def validated_gas_add (typ_0 : gas) (typ_1 : gas) : SailM gas := do
  let .Gas left : gas := typ_0
  let .Gas right : gas := typ_1
  if ((right ≤b (((2 ^i 63) -i 1) -i left)) : Bool)
  then (pure (Gas (left + right)))
  else sailThrow ((InvalidBlock ExecutionInvalid))

def validated_gas_sub_gas (typ_0 : gas) (typ_1 : gas) : SailM gas := do
  let .Gas left : gas := typ_0
  let .Gas right : gas := typ_1
  if ((right ≤b left) : Bool)
  then (pure (Gas (left -i right)))
  else sailThrow ((InvalidBlock ExecutionInvalid))

def validated_gas_sub_cost (typ_0 : gas) (typ_1 : gas_cost) : SailM gas := do
  let .Gas left : gas := typ_0
  let .GasCost right : gas_cost := typ_1
  if ((right ≤b left) : Bool)
  then (pure (Gas (left -i right)))
  else sailThrow ((InvalidBlock ExecutionInvalid))

def process_auth (au : Authorization) : SailM gas_refund := do
  let refund := GAS_REFUND_ZERO
  let authority := au.authority
  let chain_ok ← do
    (pure ((word_is_zero au.chain_id) || (au.chain_id == (← (word_of_nat (← readReg k_chain_id))))))
  if ((au.valid_sig && chain_ok) : Bool)
  then
    (do
      let _ ← do (k_access_account authority)
      let (is_deleg, _) ← do (k_deleg_target authority)
      let code_ok ← do (pure (((← (k_code_key authority)) == KECCAK_EMPTY) || is_deleg))
      let nonce_ok ← do (pure (((← (k_get_nonce authority))).value == (au.nonce).value))
      if ((code_ok && nonce_ok) : Bool)
      then
        (do
          let existed ← do (k_account_exists authority)
          if ((au.address == ZERO_ADDR) : Bool)
          then (k_clear_code authority)
          else (k_set_delegation authority au.address)
          (k_bump_nonce authority)
          if (existed : Bool)
          then
            (do
              (pure (gas_cost_to_refund (← (gas_constant_sub PER_EMPTY_ACCOUNT PER_AUTH_BASE)))))
          else (pure refund))
      else (pure refund))
  else (pure refund)

def process_auth_list (xs : (List Authorization)) : SailM gas_refund := do
  match xs with
  | [] => (pure GAS_REFUND_ZERO)
  | (a :: r) => (pure (gas_refund_add (← (process_auth a)) (← (process_auth_list r))))

def warm_access_list_addresses (xs : (List address)) : SailM Unit := do
  match xs with
  | [] => (pure ())
  | (a :: r) =>
    (do
      let _ ← do (k_access_account a)
      (warm_access_list_addresses r))

def warm_access_list_slots (xs : (List StorageKey)) : SailM Unit := do
  match xs with
  | [] => (pure ())
  | (k :: r) =>
    (do
      let _ ← do (k_slot_is_warm k.addr k.slot)
      (warm_access_list_slots r))

def prewarm (tx : Transaction) : SailM Unit := do
  let _ ← do (k_access_account tx.sender)
  let _ ← do
    if (tx.is_create : Bool)
    then (pure false)
    else (k_access_account tx.recipient)
  if ((fork_gteq (← readReg k_fork) Shanghai) : Bool)
  then
    (do
      let _ ← do (k_access_account (← (k_coinbase ())))
      (pure ()))
  else (pure ())
  let precompile_addresses : (Vector Nat 17) :=
    #v[17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1]
  let loop_i_lower := 0
  let loop_i_upper := 16
  let mut loop_vars := ()
  for i in [loop_i_lower:loop_i_upper:1]i do
    let () := loop_vars
    loop_vars ← do
      let p := (GetElem?.getElem! precompile_addresses i)
      if ((← (is_precompile ⟨p⟩)) : Bool)
      then
        (do
          let _ ← do (k_access_account (word_to_address (← (word_of_precompile_id ⟨p⟩))))
          (pure ()))
      else (pure ())
  (pure loop_vars)
  if ((← (is_precompile ⟨256⟩)) : Bool)
  then
    (do
      let _ ← do (k_access_account (word_to_address (← (word_of_precompile_id ⟨256⟩))))
      (pure ()))
  else (pure ())
  (warm_access_list_addresses tx.access_list_addresses)
  (warm_access_list_slots tx.access_list_slots)

def invalid_receipt (_ : Unit) : Receipt :=
  { tx_type := LegacyTx,
    success := false,
    valid := false,
    gas_used := GAS_ZERO,
    block_gas := GAS_ZERO,
    logs := [] }

def eff_gas_price_for (base_fee : word) (max_fee : word) (max_priority_fee : word) : (word × word) :=
  let price : (BitVec 256) :=
    match (word_checked_add base_fee max_priority_fee) with
    | .some uncapped =>
      (if ((word_ult max_fee uncapped) : Bool)
      then max_fee
      else uncapped)
    | none => max_fee
  let priority :=
    if ((word_ule base_fee price) : Bool)
    then (price - base_fee)
    else ZERO_WORD
  (price, priority)

def check_transaction_validity (tx : Transaction) : SailM TxValidity := do
  if ((! ((← (tx_sig_v_ok ⟨(← readReg k_chain_id)⟩ tx.tx_type tx.sig_v)) && (← (tx_auth_ok
             tx.pubkey tx.signing_hash tx.sig_r tx.sig_s)))) : Bool)
  then sailThrow ((InvalidBlock InvalidSignature))
  else (pure ())
  let (eff_gas_price, eff_priority_fee) ← do
    (pure (eff_gas_price_for (← readReg k_header).base_fee tx.max_fee tx.max_priority_fee))
  let sender := tx.sender
  let .ByteQuantity input_len := tx.input_src.len
  let nonce_before ← do
    (do
        let semanticResult ← (k_get_nonce sender)
        pure ((semanticResult).value))
  let blob_price ← do (blob_base_fee ⟨((← readReg k_header).excess_blob_gas).value⟩)
  let checked_costs ← do (transaction_costs tx blob_price)
  let costs_valid : Bool :=
    match checked_costs with
    | .some _ => true
    | none => false
  let costs : TransactionCosts :=
    match checked_costs with
    | .some costs => costs
    | none =>
      { intrinsic := GAS_COST_ZERO,
        calldata_floor := GAS_COST_ZERO,
        blob_gas := ⟨0⟩,
        blob_fee := ZERO_WORD,
        upfront := ZERO_WORD }
  let nonce_ok : Bool :=
    match (Option.map (fun semanticValue => (semanticValue).value) ((word_to_account_nonce tx.nonce))) with
    | .some nonce => (nonce == nonce_before)
    | none => false
  let (sender_deleg, _) ← do (k_deleg_target sender)
  let max_blobs ← (( do
    if ((fork_gteq (← readReg k_fork) Osaka) : Bool)
    then (pure 6)
    else
      (do
        if ((fork_gteq (← readReg k_fork) Prague) : Bool)
        then (pure 9)
        else (pure 6)) ) : SailM Nat )
  let blob_ok ← (( do
    if ((tx_is_blob tx.tx_type) : Bool)
    then
      (pure ((fork_gteq (← readReg k_fork) Cancun) && (← do
            (pure (((tx.blob_hashes.count).value != 0) && (← do
                  (pure (((tx.blob_hashes.count).value ≤b max_blobs) && ((! tx.is_create) && (← (blob_hashes_versioned
                            tx.blob_hashes)))))))))))
    else (pure true) ) : SailM Bool )
  let floor_ok ← (( do
    (pure ((fork_lt (← readReg k_fork) Prague) || (gas_cost_le_gas costs.calldata_floor
          tx.gas_limit))) ) : SailM Bool )
  let chain_id_ok ← (( do
    match tx.tx_type with
    | .LegacyTx => (pure true)
    | _ => (pure ((tx.chain_id).value == (← readReg k_chain_id))) ) : SailM Bool )
  let balance_ok ← (( do (pure (word_ule costs.upfront (← (k_get_balance sender)))) ) : SailM
    Bool )
  let sender_code_ok ← (( do (pure (((← (k_code_key sender)) == KECCAK_EMPTY) || sender_deleg))
    ) : SailM Bool )
  let intrinsic_ok : Bool := (gas_cost_le_gas costs.intrinsic tx.gas_limit)
  let fee_cap_ok ← (( do (pure (word_ule (← readReg k_header).base_fee tx.max_fee)) ) : SailM
    Bool )
  let blob_fee_cap_ok : Bool :=
    (((tx.blob_hashes.count).value == 0) || (word_ule blob_price tx.max_blob_fee))
  let initcode_ok ← (( do (pure ((! tx.is_create) || (← (initcode_size_allowed input_len)))) ) :
    SailM Bool )
  let priority_fee_ok : Bool := (word_ule tx.max_priority_fee tx.max_fee)
  let access_list_fork_ok ← (( do
    (pure ((! (tx_is_access_list tx.tx_type)) || (fork_gteq (← readReg k_fork) Berlin))) ) : SailM
    Bool )
  let dynamic_fee_fork_ok ← (( do
    (pure ((! (tx_is_dynamic_fee tx.tx_type)) || (fork_gteq (← readReg k_fork) London))) ) : SailM
    Bool )
  let set_code_target_ok : Bool := ((! (tx_is_set_code tx.tx_type)) || (! tx.is_create))
  let set_code_authorizations_ok : Bool :=
    ((! (tx_is_set_code tx.tx_type)) || (((tx.authorization_count).value != 0) : Bool))
  let set_code_fork_ok ← (( do
    (pure ((! (tx_is_set_code tx.tx_type)) || (fork_gteq (← readReg k_fork) Prague))) ) : SailM
    Bool )
  let nonce_incrementable : Bool := (nonce_before != ((2 ^i 64) -i 1))
  let block_gas_ok ← (( do (pure (gas_le tx.gas_limit (← readReg k_header).gas_limit)) ) : SailM
    Bool )
  let valid :=
    (costs_valid && (balance_ok && (sender_code_ok && (intrinsic_ok && (floor_ok && (fee_cap_ok && (blob_fee_cap_ok && (blob_ok && (initcode_ok && (priority_fee_ok && (access_list_fork_ok && (dynamic_fee_fork_ok && (set_code_target_ok && (set_code_authorizations_ok && (set_code_fork_ok && (chain_id_ok && (nonce_ok && (nonce_incrementable && block_gas_ok))))))))))))))))))
  (pure { valid := valid,
          sender := sender,
          nonce_before := ⟨nonce_before⟩,
          intrinsic_gas := costs.intrinsic,
          calldata_floor := costs.calldata_floor,
          blob_fee := costs.blob_fee,
          gas_price := eff_gas_price,
          priority_fee := eff_priority_fee })

def apply_transaction_upfront_effects (tx : Transaction) (v : TxValidity) : SailM gas_refund := do
  (k_sub_balance v.sender (← (validated_word_product v.gas_price tx.gas_limit)))
  if ((word_nonzero v.blob_fee) : Bool)
  then (k_sub_balance v.sender v.blob_fee)
  else (pure ())
  (k_bump_nonce v.sender)
  (prewarm tx)
  (process_auth_list tx.authorizations)

def enter_transaction_frame (tx : Transaction) (intrinsic : gas_cost) : SailM Unit := do
  writeReg pc BYTE_ZERO
  writeReg call_depth 0
  writeReg gas_remaining (← (validated_gas_sub_cost tx.gas_limit intrinsic))
  (stack_reset ())
  (memory_reset ())
  (returndata_clear ())
  writeReg calldata EMPTY_SLICE
  writeReg frame_code EMPTY_CODE
  writeReg frame_refund GAS_REFUND_ZERO
  writeReg frame_status (Running ())

/-- Type quantifiers: k_ex161560_ : Nat, 0 ≤ k_ex161560_ ∧ k_ex161560_ ≤ (2 ^ 64 - 1) -/
def run_create_transaction_frame (tx : Transaction) (sender : address) (nonce_before : account_nonce) : SailM Unit := do
  let nonce_before := (nonce_before).value
  let new_addr ← do (k_create_addr sender ⟨nonce_before⟩)
  let _ ← do (k_access_account new_addr)
  if ((← (k_account_occupied new_addr)) : Bool)
  then (exc_halt AddressCollision)
  else
    (do
      (k_mark_created new_addr)
      (k_clear_storage new_addr)
      (k_bump_nonce new_addr)
      if ((word_nonzero tx.value) : Bool)
      then (k_transfer sender new_addr tx.value)
      else (pure ())
      writeReg message { caller := sender,
                         address := new_addr,
                         code_address := new_addr,
                         value := tx.value,
                         is_static := false,
                         depth := ⟨0⟩ }
      writeReg frame_code (← (code_db_resolve (← (code_db_insert tx.input_src))))
      let deployed_code ← do (interpret ())
      if ((← (frame_succeeded ())) : Bool)
      then
        (do
          let dep_len := deployed_code.len
          let code_ok ← do
            (pure ((byte_quantity_le dep_len (← (max_code_size ()))) && ((byte_quantity_equal
                    dep_len BYTE_ZERO) || ((← (slice_byte deployed_code BYTE_ZERO)) != 0xEF#8))))
          if (code_ok : Bool)
          then
            (do
              let deposit := (gas_constant_scale_byte_quantity G_codedeposit dep_len)
              if ((gas_cost_le_gas deposit (← readReg gas_remaining)) : Bool)
              then
                (do
                  writeReg gas_remaining (← (gas_sub_cost_or_oog (← readReg gas_remaining)
                      deposit))
                  (k_deploy_code new_addr deployed_code))
              else (exc_halt OutOfGas))
          else (exc_halt OutOfGas))
      else (pure ()))

def run_call_transaction_frame (tx : Transaction) (sender : address) : SailM Unit := do
  let _ ← do (k_aload tx.recipient)
  if ((word_nonzero tx.value) : Bool)
  then (k_transfer sender tx.recipient tx.value)
  else (pure ())
  let precompile ← (( do
    match (Option.map (fun semanticValue => (semanticValue).value) ((word_to_precompile_id
      (address_to_word tx.recipient)))) with
    | .some number =>
      (do
        if ((← (is_precompile ⟨number⟩)) : Bool)
        then (pure (some number))
        else (pure none))
    | _ => (pure none) ) : SailM (Option Nat) )
  match precompile with
  | .some number =>
    (do
      match (← (precompile_gas ⟨number⟩ tx.input_src)) with
      | .some used =>
        (do
          if ((gas_cost_le_gas used (← readReg gas_remaining)) : Bool)
          then
            (do
              let result ← do (run_precompile_slice ⟨number⟩ tx.input_src)
              if (result.success : Bool)
              then
                (do
                  writeReg gas_remaining (← (gas_sub_cost_or_oog (← readReg gas_remaining) used))
                  if ((← (is_running ())) : Bool)
                  then writeReg frame_status (Halted (HaltReturn result.output))
                  else (pure ()))
              else (exc_halt OutOfGas))
          else (exc_halt OutOfGas))
      | _ => (exc_halt OutOfGas))
  | none =>
    (do
      writeReg calldata tx.input_src
      writeReg message { caller := sender,
                         address := tx.recipient,
                         code_address := tx.recipient,
                         value := tx.value,
                         is_static := false,
                         depth := ⟨0⟩ }
      let (tx_deleg, tx_dtgt) ← do (k_deleg_target tx.recipient)
      if (tx_deleg : Bool)
      then
        (do
          let _ ← do (k_access_account tx_dtgt)
          let _ ← do (k_aload tx_dtgt)
          (pure ()))
      else (pure ())
      writeReg frame_code (← (executable_code tx.recipient tx_deleg tx_dtgt))
      let _ ← do (interpret ())
      (pure ()))

def run_transaction_frame (tx : Transaction) (v : TxValidity) : SailM TxFrameResult := do
  let checkpoint ← do (k_state_checkpoint ())
  (enter_transaction_frame tx v.intrinsic_gas)
  if (tx.is_create : Bool)
  then (run_create_transaction_frame tx v.sender ⟨(v.nonce_before).value⟩)
  else (run_call_transaction_frame tx v.sender)
  let success ← do (frame_succeeded ())
  if ((! success) : Bool)
  then (k_revert checkpoint)
  else (pure ())
  (pure { success := success,
          gas_remaining := ← readReg gas_remaining,
          refund := ← if (success : Bool)
            then readReg frame_refund
            else (pure GAS_REFUND_ZERO) })

def settle_transaction (tx : Transaction) (v : TxValidity) (authorization_refund : gas_refund) (fr : TxFrameResult) : SailM Receipt := do
  let gas_left0 := fr.gas_remaining
  let gas_used0 ← do (validated_gas_sub_gas tx.gas_limit gas_left0)
  let refund_quotient ← (( do
    if ((fork_gteq (← readReg k_fork) London) : Bool)
    then (pure 5)
    else (pure 2) ) : SailM Nat )
  let refund_cap ← do (gas_quotient gas_used0 ⟨refund_quotient⟩)
  let refund := (capped_gas_refund (gas_refund_add authorization_refund fr.refund) refund_cap)
  let gas_left1 ← do (validated_gas_add gas_left0 refund)
  let gas_used1 ← do (validated_gas_sub_gas tx.gas_limit gas_left1)
  let floor ← (( do
    if ((fork_gteq (← readReg k_fork) Prague) : Bool)
    then
      (do
        let .GasCost floor_cost := v.calldata_floor
        let .Gas tx_limit := tx.gas_limit
        if ((floor_cost ≤b tx_limit) : Bool)
        then (pure (Gas floor_cost))
        else sailThrow ((InvalidBlock ExecutionInvalid)))
    else (pure GAS_ZERO) ) : SailM gas )
  let gas_used :=
    if ((gas_lt gas_used1 floor) : Bool)
    then floor
    else gas_used1
  let gas_left ← do (validated_gas_sub_gas tx.gas_limit gas_used)
  let block_gas ← do
    if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
    then
      (if ((gas_lt gas_used0 floor) : Bool)
      then (pure floor)
      else (pure gas_used0))
    else (pure gas_used)
  (k_add_balance v.sender (← (validated_word_product v.gas_price gas_left)))
  (k_add_balance (← (k_coinbase ())) (← (validated_word_product v.priority_fee gas_used)))
  (k_tx_merge ())
  (pure { tx_type := tx.tx_type,
          success := fr.success,
          valid := true,
          gas_used := gas_used,
          block_gas := block_gas,
          logs := ← (read_logs ()) })

def process_transaction (tx : Transaction) : SailM Receipt := do
  (k_tx_reset ())
  let validity ← do (check_transaction_validity tx)
  if ((! validity.valid) : Bool)
  then (pure (invalid_receipt ()))
  else
    (do
      (k_set_tx
        { origin := tx.sender,
          gas_price := validity.gas_price,
          blob_hashes := tx.blob_hashes })
      let authorization_refund ← do (apply_transaction_upfront_effects tx validity)
      let frame_result ← do (run_transaction_frame tx validity)
      (settle_transaction tx validity authorization_refund frame_result))

