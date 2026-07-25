import Evm.Flow
import Evm.Prelude
import Evm.Primitives.CycleScopes
import Evm.Host.CycleScopesDisabled
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Primitives.Code
import Evm.Primitives.Crypto
import Evm.Primitives.Tx
import Evm.Host.EvmByteSlice
import Evm.Host.Code
import Evm.Lib.Tx
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
import Evm.Evm.Interpreter

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

/-! # The transaction state transition

The per-transaction step of the Ethereum state transition (Yellow Paper
§6): validate the transaction, charge upfront gas, run it as the
top-level message call, then settle gas, refunds, and the coinbase fee.

Validity and intrinsic gas are user-space policy; every world effect is a
kernel syscall, and a transaction-level snapshot/commit/revert bounds the
whole transaction's world state. The typed-envelope rules covered here:
EIP-2718 (typed transactions), EIP-1559 (fee market: base fee + priority
tip caps), EIP-2930 (access lists), EIP-3860 (initcode size/gas),
EIP-4844 (blob transactions + blob-gas burn), EIP-7702 (set-code
authorizations), EIP-7623 (calldata floor), EIP-3607 (no transactions
from an account with code), EIP-7825 (gas cap). Gas refunds (`SSTORE`
clears) are capped per EIP-3529.

| Name | Value | Description |
| ---- | ----- | ----------- |
| `G_transaction` | `21000` | Per-transaction base cost |
| `G_txcreate` | `32000` | Create-transaction surcharge |
| `G_txdatazero` | `4` | Per zero calldata byte (EIP-2028) |
| `G_txdatanonzero` | `16` | Per nonzero calldata byte (EIP-2028) |
| `G_access_list_address` | `2400` | Per access-list address (EIP-2930) |
| `G_access_list_storage_key` | `1900` | Per access-list storage key (EIP-2930) |
| `PER_AUTH_BASE` | `12500` | Per authorization (EIP-7702) |
| `PER_EMPTY_ACCOUNT` | `25000` | Per authorization of a new account (EIP-7702) | -/

def G_transaction : Nat := 21000

def G_txcreate : Nat := 32000

def G_txdatazero : Nat := 4

def G_txdatanonzero : Nat := 16

def G_access_list_address : Nat := 2400

def G_access_list_storage_key : Nat := 1900

def PER_AUTH_BASE : Nat := 12500

def PER_EMPTY_ACCOUNT : Nat := 25000

def AMSTERDAM_TX_BASE : Nat := 12000

def AMSTERDAM_CREATE_ACCESS : Nat := 11000

def AMSTERDAM_COLD_ACCOUNT_ACCESS : Nat := 3000

def AMSTERDAM_TX_VALUE_COST : Nat := 4244

def AMSTERDAM_TRANSFER_LOG_COST : Nat := 1756

def AMSTERDAM_ACCESS_LIST_ADDRESS : Nat := 3000

def AMSTERDAM_ACCESS_LIST_SLOT : Nat := 3000

def AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR : Nat := 1280

def AMSTERDAM_ACCESS_LIST_SLOT_FLOOR : Nat := 2048

def AMSTERDAM_AUTH_BASE : Nat := 7816

def AMSTERDAM_CALLDATA_FLOOR_BYTE : Nat := 64

def AMSTERDAM_TX_MAX_GAS : transaction_gas := TRANSACTION_EXECUTION_GAS_LIMIT

def undefined_IntrinsicGasCost (_ : Unit) : SailM IntrinsicGasCost := do
  (pure { execution := ← (undefined_nat ()),
          state := ← (undefined_nat ()),
          calldata_floor := ← (undefined_nat ()) })

def undefined_TransactionCosts (_ : Unit) : SailM TransactionCosts := do
  (pure { intrinsic_execution := ← (undefined_nat ()),
          intrinsic_state := ← (undefined_nat ()),
          calldata_floor := ← (undefined_nat ()),
          blob_gas := ← (undefined_range 0 (9 *i (2 ^i 17))),
          blob_fee := ← (undefined_range 0 ((2 ^i 256) - 1)),
          upfront := ← (undefined_range 0 ((2 ^i 256) - 1)) })

/-- Reclassifies transaction initcode as executable after re-establishing the
enclosing SSZ transaction-envelope bound. This bound is structural and is
deliberately independent of the active protocol deployment limit. -/
/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, 0 ≤
  input_dependentWitness0 ∧ 0 ≤ input_dependentWitness1 ∧
  0 ≤ input_dependentWitness1 ∧ input_dependentWitness1 ≤ (2 ^ 30) -/
def transaction_initcode_slice (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
  (input_dependentWitness0 : Nat) =>
  (Sigma fun (input_dependentWitness1 : Nat) =>
  (EvmByteSliceFields input_dependentWitness0 input_dependentWitness1))) :=
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  ((code_slice input) : (Sigma fun (input_dependentWitness0 : Nat) =>
  (Sigma fun (input_dependentWitness1 : Nat) =>
  (EvmByteSliceFields input_dependentWitness0 input_dependentWitness1))))

def undefined_TxUpfrontResult (_ : Unit) : SailM TxUpfrontResult := do
  (pure { authorization_refund := ← (undefined_range (Neg.neg (199 *i ((2 ^i 64) - 1)))
              (199 *i ((2 ^i 64) - 1))),
          create_target_prestate_empty := ← (undefined_bool ()) })

def EMPTY_AMSTERDAM_AUTHORIZATION_STATE : AmsterdamAuthorizationState :=
  { seen_valid_authorities := [],
    originally_delegated := [],
    delegation_set_for := [] }

/-- The EIP-2028 calldata cost: 4 gas per zero byte, 16 per nonzero. One
native pass counts the nonzero bytes; zero bytes are the
remainder. -/
/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, 0 ≤
  input_dependentWitness0 ∧ 0 ≤ input_dependentWitness1 ∧
  0 ≤ input_dependentWitness1 ∧ input_dependentWitness1 ≤ (2 ^ 30) -/
def calldata_cost (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM Nat := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  let nonzeroes ← do (slice_count_nonzero ⟨_, ⟨_, input⟩⟩)
  let input_len := input.len
  if ((nonzeroes ≤b input_len) : Bool)
  then
    (let zeroes := (input_len - nonzeroes)
    (pure ((G_txdatazero *i zeroes) + (G_txdatanonzero *i nonzeroes))))
  else sailThrow ((InvalidBlock ExecutionInvalid))

/-- EIP-7623 calldata tokens: each zero byte counts 1, each nonzero
byte 4. -/
/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, 0 ≤
  input_dependentWitness0 ∧ 0 ≤ input_dependentWitness1 ∧
  0 ≤ input_dependentWitness1 ∧ input_dependentWitness1 ≤ (2 ^ 30) -/
def calldata_tokens (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM Nat := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  let nonzeroes ← do (slice_count_nonzero ⟨_, ⟨_, input⟩⟩)
  let input_len := input.len
  let zeroes ← (( do
    if ((nonzeroes ≤b input_len) : Bool)
    then (pure (input_len - nonzeroes))
    else sailThrow ((InvalidBlock ExecutionInvalid)) ) : SailM Nat )
  (pure (zeroes + (4 *i nonzeroes)))

/-- EIP-4844: every blob versioned hash must carry
`VERSIONED_HASH_VERSION_KZG` (`0x01`). -/
/- Type quantifiers: k_ex417299_ : Nat, k_ex417298_ : Nat, 0 ≤ k_ex417298_ ∧ k_ex417298_ ≤ 9, 1
  ≤ k_ex417299_ ∧ k_ex417299_ ≤ 265 -/
def validate_blob_hash_version_at (hashes : BlobHashes) (remaining : Nat) (offset : Nat) : SailM Nat := do
  if ((remaining == 0) : Bool)
  then (pure 0)
  else
    (do
      if (((← (slice_byte hashes.bytes offset)) != 0x01#8) : Bool)
      then sailThrow ((InvalidBlock ExecutionInvalid))
      else (pure (transaction_blob_count_decrement remaining)))

/-- Rejects any transaction blob hash whose version byte is not `0x01`. -/
def validate_blob_hash_versions (hashes : BlobHashes) : SailM Unit := do
  let remaining : Nat := hashes.count
  let remaining ← (validate_blob_hash_version_at hashes remaining 1)
  let remaining ← (validate_blob_hash_version_at hashes remaining 34)
  let remaining ← (validate_blob_hash_version_at hashes remaining 67)
  let remaining ← (validate_blob_hash_version_at hashes remaining 100)
  let remaining ← (validate_blob_hash_version_at hashes remaining 133)
  let remaining ← (validate_blob_hash_version_at hashes remaining 166)
  let remaining ← (validate_blob_hash_version_at hashes remaining 199)
  let remaining ← (validate_blob_hash_version_at hashes remaining 232)
  let remaining ← (validate_blob_hash_version_at hashes remaining 265)
  if ((remaining != 0) : Bool)
  then sailThrow ((InvalidBlock ExecutionInvalid))
  else (pure ())

/-- The intrinsic gas of a transaction (YP §6.2, g_0): the 21000 base,
calldata cost, access-list cost (EIP-2930), authorization cost
(EIP-7702), and for creates the `G_txcreate` base plus EIP-3860
initcode words. -/
def legacy_intrinsic_gas (tx : Transaction) : SailM Nat := do
  let data_cost ← do (calldata_cost tx.input_src)
  let ⟨_, ⟨_, input⟩⟩ := tx.input_src
  let input_len := input.len
  let address_cost := (G_access_list_address *i tx.access_list_address_count)
  let slot_cost := (G_access_list_storage_key *i tx.access_list_slot_count)
  let auth_cost := (PER_EMPTY_ACCOUNT *i tx.authorization_count)
  let common := ((((data_cost + G_transaction) + address_cost) + slot_cost) + auth_cost)
  if (tx.is_create : Bool)
  then (pure ((common + G_txcreate) + (← (transaction_initcode_gas input_len))))
  else (pure common)

/-- Computes the pre-Amsterdam EIP-7623 calldata floor cost. -/
/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, 0 ≤
  input_dependentWitness0 ∧ 0 ≤ input_dependentWitness1 ∧
  0 ≤ input_dependentWitness1 ∧ input_dependentWitness1 ≤ (2 ^ 30) -/
def legacy_calldata_floor (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM Nat := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  let nonzeroes ← do (slice_count_nonzero ⟨_, ⟨_, input⟩⟩)
  let input_len := input.len
  if ((nonzeroes ≤b input_len) : Bool)
  then
    (let zeroes := (input_len - nonzeroes)
    (pure (((10 *i zeroes) + (40 *i nonzeroes)) + G_transaction)))
  else sailThrow ((InvalidBlock ExecutionInvalid))

/-- Computes Amsterdam execution-gas charges for recipient access, value
transfer, and contract creation. -/
def amsterdam_recipient_execution_cost (tx : Transaction) : Nat :=
  if (tx.is_create : Bool)
  then
    (if ((word_nonzero tx.value) : Bool)
    then (AMSTERDAM_CREATE_ACCESS + AMSTERDAM_TRANSFER_LOG_COST)
    else AMSTERDAM_CREATE_ACCESS)
  else
    (if ((bne tx.recipient tx.sender) : Bool)
    then
      (if ((word_nonzero tx.value) : Bool)
      then ((AMSTERDAM_COLD_ACCOUNT_ACCESS + AMSTERDAM_TX_VALUE_COST) + AMSTERDAM_TRANSFER_LOG_COST)
      else AMSTERDAM_COLD_ACCOUNT_ACCESS)
    else 0)

/-- Computes the fork-specific execution/state intrinsic costs and calldata
floor. Amsterdam decomposes the transaction charge into the two gas
dimensions introduced by EIP-2780. -/
def intrinsic_gas (tx : Transaction) : SailM IntrinsicGasCost := do
  if ((fork_lt (← readReg k_fork) Amsterdam) : Bool)
  then
    (pure { execution := ← (legacy_intrinsic_gas tx),
            state := 0,
            calldata_floor := ← (legacy_calldata_floor tx.input_src) })
  else
    (do
      let ⟨_, ⟨_, input⟩⟩ := tx.input_src
      let recipient := (amsterdam_recipient_execution_cost tx)
      let address_count := tx.access_list_address_count
      let slot_count := tx.access_list_slot_count
      let access_execution :=
        ((((AMSTERDAM_ACCESS_LIST_ADDRESS *i address_count) + (AMSTERDAM_ACCESS_LIST_SLOT *i slot_count)) + (AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR *i address_count)) + (AMSTERDAM_ACCESS_LIST_SLOT_FLOOR *i slot_count))
      let authorization_execution := (AMSTERDAM_AUTH_BASE *i tx.authorization_count)
      let create_execution ← do
        if (tx.is_create : Bool)
        then (transaction_initcode_gas input.len)
        else (pure 0)
      let execution ← do
        (pure ((((((← (calldata_cost tx.input_src)) + AMSTERDAM_TX_BASE) + recipient) + access_execution) + authorization_execution) + create_execution))
      let input_length := input.len
      let floor :=
        (((((AMSTERDAM_CALLDATA_FLOOR_BYTE *i input_length) + AMSTERDAM_TX_BASE) + recipient) + (AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR *i address_count)) + (AMSTERDAM_ACCESS_LIST_SLOT_FLOOR *i slot_count))
      (pure { execution := execution,
              state := 0,
              calldata_floor := floor }))

/-- Returns the active fork's maximum blob count per transaction. -/
def max_blobs_per_transaction (_ : Unit) : SailM Nat := do
  if ((fork_gteq (← readReg k_fork) Osaka) : Bool)
  then (pure 6)
  else
    (do
      if ((fork_gteq (← readReg k_fork) Prague) : Bool)
      then (pure 9)
      else (pure 6))

/-- Converts a validated transaction blob count to blob gas. -/
/- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ 9 -/
def transaction_blob_gas_for_count (count : Nat) : SailM Nat := do
  let active_maximum ← do (max_blobs_per_transaction ())
  if (((active_maximum ≤b 9) && (count ≤b active_maximum)) : Bool)
  then (pure (131072 *i count))
  else sailThrow ((InvalidBlock ExecutionInvalid))

/-- Computes all transaction costs, rejecting arithmetic beyond word bounds. -/
/- Type quantifiers: k_ex417308_ : Nat, k_ex417307_ : Nat, 0 ≤ k_ex417307_ ∧
  k_ex417307_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex417308_ ∧ k_ex417308_ ≤ (2 ^ 256 - 1) -/
def transaction_costs (tx : Transaction) (gas_limit : Nat) (blob_price : Nat) : SailM TransactionCosts := do
  let intrinsic ← do (intrinsic_gas tx)
  let blob_gas ← do (transaction_blob_gas_for_count tx.blob_hashes.count)
  let blob_fee ← do (blob_word_mul blob_price blob_gas)
  let execution_cap ← do (blob_word_mul tx.max_fee gas_limit)
  let blob_cap ← do (blob_word_mul tx.max_blob_fee blob_gas)
  let upfront ← do (blob_word_add (← (blob_word_add execution_cap tx.value)) blob_cap)
  (pure { intrinsic_execution := intrinsic.execution,
          intrinsic_state := intrinsic.state,
          calldata_floor := intrinsic.calldata_floor,
          blob_gas := blob_gas,
          blob_fee := blob_fee,
          upfront := upfront })

/- Type quantifiers: value : Nat, factor : Nat, 0 ≤ factor ∧ factor < (2 ^ 256), 0 ≤ value ∧
  value ≤ (2 ^ 256 - 1) -/
def validated_word_product (value : Nat) (factor : Nat) : SailM Nat := do
  (blob_word_mul value factor)

/-- Adds gas quantities whose sum was established to remain valid. -/
/- Type quantifiers: k_ex417311_ : Nat, k_ex417310_ : Nat, 0 ≤ k_ex417310_, 0 ≤ k_ex417311_ -/
def validated_gas_add (left_gas : Nat) (right_gas : Nat) : Nat :=
  (conserved_gas_add left_gas right_gas)

/- Type quantifiers: left : Nat, right : Nat, 0 ≤ left ∧ 0 ≤ right -/
def validated_gas_sub (left : Nat) (right : Nat) : SailM Nat := do
  if ((right ≤b left) : Bool)
  then (pure (left - right))
  else sailThrow ((InvalidBlock ExecutionInvalid))

/-- Validates the transaction gas limit against the SSZ-backed block limit and
exposes that bound to the remainder of the transaction lifecycle. -/
/- Type quantifiers: k_ex417313_ : Nat, k_ex417312_ : Nat, 0 ≤ k_ex417312_, 0 ≤ k_ex417313_ ∧
  k_ex417313_ ≤ (2 ^ 64 - 1) -/
def admitted_transaction_gas_limit (value : Nat) (block_limit : Nat) : SailM Nat := do
  if ((block_limit <b value) : Bool)
  then sailThrow ((InvalidBlock ExecutionInvalid))
  else (pure value)

/-- Applies one EIP-7702 authorization: validates it against current
state, sets or clears the delegation, bumps the authority nonce, and
refunds if the authority already existed. The signature and chain id
are validated from the tuple alone before the authority's account is
read — a tuple rejected there touches no state, so its authority need
not be witnessed; every authority-state read is gated on those
checks. The authority is warmed before the code/nonce
checks, so a tuple later skipped still warms it. -/
def process_auth (au : Authorization) : SailM Int := do
  let refund : Int := GAS_REFUND_ZERO
  let authority := au.authority
  if ((← if (au.valid_sig : Bool)
       then
         (do
           if ((word_is_zero au.chain_id) : Bool)
           then (pure true)
           else
             (do
               (pure (au.chain_id == (word_of_chain_identifier (← readReg k_chain_id))))))
       else (pure false)) : Bool)
  then
    (do
      let _ ← do (k_access_account authority)
      let (is_deleg, _) ← do (k_deleg_target authority)
      if ((← if ((((← (k_code_key authority)) == KECCAK_EMPTY) || is_deleg) : Bool)
           then
             (do
               (pure ((← (k_get_nonce authority)) == au.nonce)))
           else (pure false)) : Bool)
      then
        (do
          let existed ← do (k_account_exists authority)
          if ((au.address == ZERO_ADDRESS) : Bool)
          then (k_clear_code authority)
          else (k_set_delegation authority au.address)
          (k_bump_nonce authority)
          if (existed : Bool)
          then
            (let refund : Int := (PER_EMPTY_ACCOUNT - PER_AUTH_BASE)
            (pure refund))
          else (pure refund))
      else (pure refund))
  else (pure refund)

/-- Applies an authorization list in order. -/
def process_auth_list (xs : (List Authorization)) : SailM Int := do
  match xs with
  | [] => (pure GAS_REFUND_ZERO)
  | (a :: r) => (validated_refund_add (← (process_auth a)) (← (process_auth_list r)))

/-- Whether an Amsterdam authorization address has already appeared in a
successfully applied tuple. -/
def authorization_address_seen (a : (Vector (BitVec 8) 20)) (xs : (List (Vector (BitVec 8) 20))) : Bool :=
  match xs with
  | [] => false
  | (h :: t) => ((a == h) || (authorization_address_seen a t))

/-- Applies one Amsterdam authorization and charges its state-dependent
execution-gas and state-gas components. Tuple-local signature and chain
checks precede all authority-state reads; a valid tuple warms its authority
before checking code and nonce, as required by EIP-7702. -/
/- Type quantifiers: k_ex417314_ : Bool -/
def process_amsterdam_auth (au : Authorization) (sender : (Vector (BitVec 8) 20)) (current_target : (Vector (BitVec 8) 20)) (transfers_value : Bool) (auth_state : AmsterdamAuthorizationState) : SailM AmsterdamAuthorizationState := do
  let next := auth_state
  let authority := au.authority
  if ((← if (au.valid_sig : Bool)
       then
         (do
           if ((word_is_zero au.chain_id) : Bool)
           then (pure true)
           else
             (do
               (pure (au.chain_id == (word_of_chain_identifier (← readReg k_chain_id))))))
       else (pure false)) : Bool)
  then
    (do
      let _ ← do (k_access_account authority)
      let (currently_delegated, _) ← do (k_deleg_target authority)
      if ((← if ((((← (k_code_key authority)) == KECCAK_EMPTY) || currently_delegated) : Bool)
           then
             (do
               (pure ((← (k_get_nonce authority)) == au.nonce)))
           else (pure false)) : Bool)
      then
        (do
          let seen := (authorization_address_seen authority auth_state.seen_valid_authorities)
          let delegated_before_tx :=
            if (seen : Bool)
            then (authorization_address_seen authority auth_state.originally_delegated)
            else currently_delegated
          let already_written :=
            (seen || ((authority == sender) || (transfers_value && (authority == current_target))))
          if ((! (← (k_account_exists authority))) : Bool)
          then (charge_state_gas G_amsterdam_state_new_account)
          else (pure ())
          if (((← (is_running ())) && (! already_written)) : Bool)
          then (charge G_amsterdam_account_write)
          else (pure ())
          if (((← (is_running ())) && ((bne au.address ZERO_ADDRESS) && ((! delegated_before_tx) && (! (authorization_address_seen
                       authority auth_state.delegation_set_for))))) : Bool)
          then (charge_state_gas G_amsterdam_state_auth_base)
          else (pure ())
          if ((← (is_running ())) : Bool)
          then
            (do
              if ((au.address == ZERO_ADDRESS) : Bool)
              then (k_clear_code authority)
              else (k_set_delegation authority au.address)
              (k_bump_nonce authority)
              let next : AmsterdamAuthorizationState :=
                { next with seen_valid_authorities := (authority :: next.seen_valid_authorities) }
              let next : AmsterdamAuthorizationState :=
                if (((! seen) && currently_delegated) : Bool)
                then { next with originally_delegated := (authority :: next.originally_delegated) }
                else next
              if (((bne au.address ZERO_ADDRESS) && (! (authorization_address_seen authority
                       next.delegation_set_for))) : Bool)
              then (pure { next with delegation_set_for := (authority :: next.delegation_set_for) })
              else (pure next))
          else (pure next))
      else (pure next))
  else (pure next)

/-- Applies Amsterdam authorizations in wire order, stopping immediately when
one of their execution-gas or state-gas charges exhausts the transaction. -/
/- Type quantifiers: k_ex417315_ : Bool -/
def process_amsterdam_auth_list (xs : (List Authorization)) (sender : (Vector (BitVec 8) 20)) (current_target : (Vector (BitVec 8) 20)) (transfers_value : Bool) (auth_state : AmsterdamAuthorizationState) : SailM AmsterdamAuthorizationState := do
  match xs with
  | [] => (pure auth_state)
  | (a :: r) =>
    (do
      let next ← do (process_amsterdam_auth a sender current_target transfers_value auth_state)
      if ((← (is_running ())) : Bool)
      then (process_amsterdam_auth_list r sender current_target transfers_value next)
      else (pure next))

/-- Warms every access-list address (EIP-2930/EIP-2929 prewarming). -/
def warm_access_list_addresses (xs : (List (Vector (BitVec 8) 20))) : SailM Unit := do
  match xs with
  | [] => (pure ())
  | (a :: r) =>
    (do
      let _ ← do (k_access_account a)
      (warm_access_list_addresses r))

/-- Warms every access-list storage slot (EIP-2930/EIP-2929
prewarming). -/
def warm_access_list_slots (xs : (List StorageKey)) : SailM Unit := do
  match xs with
  | [] => (pure ())
  | (k :: r) =>
    (do
      let _ ← do (k_slot_is_warm k.addr k.slot)
      (warm_access_list_slots r))

/-- Pre-warms the accessed-address set (EIP-2929): the sender, the call
target, the active fork's precompiles, and the access list
(EIP-2930); EIP-3651 additionally warms the coinbase from Shanghai
onward. -/
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
      if ((← (precompile_active_at_fork p)) : Bool)
      then
        (do
          let _ ← do (k_access_account (precompile_id_to_address p))
          (pure ()))
      else (pure ())
  (pure loop_vars)
  if ((← (precompile_active_at_fork 256)) : Bool)
  then
    (do
      let _ ← do (k_access_account (precompile_id_to_address 256))
      (pure ()))
  else (pure ())
  (warm_access_list_addresses tx.access_list_addresses)
  (warm_access_list_slots tx.access_list_slots)

/-- The EIP-1559 effective fee: the gas price actually paid is
`min(max_fee, base_fee + max_priority_fee)`, and the priority tip
paid to the coinbase is that price minus the base fee. Legacy and
EIP-2930 transactions carry a single `gas_price`, passed as
`max_fee = max_priority = gas_price`, so this recovers
`(gas_price, gas_price − base_fee)`. The priority is clamped at 0 so
an invalid sub-base-fee price (rejected later by validity) never
underflows. -/
/- Type quantifiers: k_ex417318_ : Nat, k_ex417317_ : Nat, k_ex417316_ : Nat, 0 ≤ k_ex417316_ ∧
  k_ex417316_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex417317_ ∧ k_ex417317_ ≤ (2 ^ 256 - 1), 0 ≤
  k_ex417318_ ∧ k_ex417318_ ≤ (2 ^ 256 - 1) -/
def eff_gas_price_for (base_fee : Nat) (max_fee : Nat) (max_priority_fee : Nat) : (Nat × Nat) :=
  let price : Nat :=
    if ((word_ule max_fee base_fee) : Bool)
    then max_fee
    else
      (let available_priority := (word_sub_word max_fee base_fee)
      if ((word_ule max_priority_fee available_priority) : Bool)
      then (word_add_word base_fee max_priority_fee)
      else max_fee)
  let priority :=
    if ((word_ule base_fee price) : Bool)
    then (word_sub_word price base_fee)
    else ZERO_WORD
  (price, priority)

/-- Transaction validity (YP §6.2). First
authenticates the witnessed public key against the signature over the
signing hash — a forged key or bad `v`/`r`/`s` makes the whole block
invalid regardless of the verdict — then applies the per-envelope
validity rules (nonce, balance, intrinsic gas, fee caps, blob rules,
EIP-3607, EIP-7825) and derives the effective prices. -/
def check_transaction_validity (tx : Transaction) : SailM TxValidity := do
  if ((! (tx_sig_v_valid (← readReg k_chain_id) tx.tx_type tx.sig_v)) : Bool)
  then sailThrow ((InvalidBlock InvalidSignature))
  else
    (do
      let parity := (tx_y_parity tx.tx_type tx.sig_v)
      if ((! (← (tx_auth_valid tx.sender tx.signing_hash parity tx.sig_r tx.sig_s))) : Bool)
      then sailThrow ((InvalidBlock InvalidSignature))
      else
        (do
          let gas_limit ← do
            (admitted_transaction_gas_limit tx.gas_limit (← readReg k_header).gas_limit)
          let (eff_gas_price, eff_priority_fee) ← do
            (pure (eff_gas_price_for (← readReg k_header).base_fee tx.max_fee tx.max_priority_fee))
          let sender := tx.sender
          let ⟨_, ⟨_, input⟩⟩ := tx.input_src
          let input_len := input.len
          let nonce_before ← do (k_get_nonce sender)
          let blob_price ← do (blob_base_fee (← readReg k_header).excess_blob_gas)
          let costs ← do (transaction_costs tx gas_limit blob_price)
          let nonce ← (( do
            match (word_to_account_nonce tx.nonce) with
            | .some nonce => (pure nonce)
            | none => sailThrow ((InvalidBlock ExecutionInvalid)) ) : SailM Nat )
          if ((nonce != nonce_before) : Bool)
          then sailThrow ((InvalidBlock ExecutionInvalid))
          else
            (do
              let (sender_deleg, _) ← do (k_deleg_target sender)
              let max_blobs ← do (max_blobs_per_transaction ())
              if ((tx_is_blob tx.tx_type) : Bool)
              then
                (do
                  if (((fork_lt (← readReg k_fork) Cancun) || (((tx.blob_hashes.count == 0) || (((max_blobs <b tx.blob_hashes.count) || tx.is_create) : Bool)) : Bool)) : Bool)
                  then sailThrow ((InvalidBlock ExecutionInvalid))
                  else (validate_blob_hash_versions tx.blob_hashes))
              else (pure ())
              if (((fork_gteq (← readReg k_fork) Prague) && ((gas_limit <b costs.calldata_floor) : Bool)) : Bool)
              then sailThrow ((InvalidBlock ExecutionInvalid))
              else
                (do
                  if ((! (word_ule costs.upfront (← (k_get_balance sender)))) : Bool)
                  then sailThrow ((InvalidBlock ExecutionInvalid))
                  else
                    (do
                      if ((! (((← (k_code_key sender)) == KECCAK_EMPTY) || sender_deleg)) : Bool)
                      then sailThrow ((InvalidBlock ExecutionInvalid))
                      else
                        (do
                          let intrinsic_total := (costs.intrinsic_execution + costs.intrinsic_state)
                          if ((gas_limit <b intrinsic_total) : Bool)
                          then sailThrow ((InvalidBlock ExecutionInvalid))
                          else
                            (do
                              if (((fork_gteq (← readReg k_fork) Amsterdam) && ((AMSTERDAM_TX_MAX_GAS <b costs.intrinsic_execution) : Bool)) : Bool)
                              then sailThrow ((InvalidBlock ExecutionInvalid))
                              else
                                (do
                                  if (((fork_gteq (← readReg k_fork) Amsterdam) && ((AMSTERDAM_TX_MAX_GAS <b costs.calldata_floor) : Bool)) : Bool)
                                  then sailThrow ((InvalidBlock ExecutionInvalid))
                                  else
                                    (do
                                      if ((! (word_ule (← readReg k_header).base_fee tx.max_fee)) : Bool)
                                      then sailThrow ((InvalidBlock ExecutionInvalid))
                                      else
                                        (do
                                          if (((tx.blob_hashes.count != 0) && (! (word_ule
                                                   blob_price tx.max_blob_fee))) : Bool)
                                          then sailThrow ((InvalidBlock ExecutionInvalid))
                                          else
                                            (do
                                              if ((← if (tx.is_create : Bool)
                                                   then
                                                     (do
                                                       (pure (! (← (initcode_size_allowed
                                                               input_len)))))
                                                   else (pure false)) : Bool)
                                              then sailThrow ((InvalidBlock ExecutionInvalid))
                                              else
                                                (do
                                                  if ((! (word_ule tx.max_priority_fee tx.max_fee)) : Bool)
                                                  then sailThrow ((InvalidBlock ExecutionInvalid))
                                                  else
                                                    (do
                                                      if ((← if ((tx_is_access_list tx.tx_type) : Bool)
                                                           then
                                                             (do
                                                               (pure (fork_lt (← readReg k_fork)
                                                                   Berlin)))
                                                           else (pure false)) : Bool)
                                                      then
                                                        sailThrow ((InvalidBlock ExecutionInvalid))
                                                      else
                                                        (do
                                                          if ((← if ((tx_is_dynamic_fee tx.tx_type) : Bool)
                                                               then
                                                                 (do
                                                                   (pure (fork_lt
                                                                       (← readReg k_fork) London)))
                                                               else (pure false)) : Bool)
                                                          then
                                                            sailThrow ((InvalidBlock
                                                              ExecutionInvalid))
                                                          else
                                                            (do
                                                              if (((tx_is_set_code tx.tx_type) && tx.is_create) : Bool)
                                                              then
                                                                sailThrow ((InvalidBlock
                                                                  ExecutionInvalid))
                                                              else
                                                                (do
                                                                  if (((tx_is_set_code tx.tx_type) && ((tx.authorization_count == 0) : Bool)) : Bool)
                                                                  then
                                                                    sailThrow ((InvalidBlock
                                                                      ExecutionInvalid))
                                                                  else
                                                                    (do
                                                                      if ((← if ((tx_is_set_code
                                                                                tx.tx_type) : Bool)
                                                                           then
                                                                             (do
                                                                               (pure (fork_lt
                                                                                   (← readReg k_fork)
                                                                                   Prague)))
                                                                           else (pure false)) : Bool)
                                                                      then
                                                                        sailThrow ((InvalidBlock
                                                                          ExecutionInvalid))
                                                                      else
                                                                        (do
                                                                          if ((← do
                                                                               match tx.tx_type with
                                                                               | .LegacyTx =>
                                                                                 (pure false)
                                                                               | _ =>
                                                                                 (pure (tx.chain_id != (← readReg k_chain_id)))) : Bool)
                                                                          then
                                                                            sailThrow ((InvalidBlock
                                                                              ExecutionInvalid))
                                                                          else
                                                                            (do
                                                                              if ((nonce_before == ((2 ^i 64) - 1)) : Bool)
                                                                              then
                                                                                sailThrow ((InvalidBlock
                                                                                  ExecutionInvalid))
                                                                              else
                                                                                (pure { sender := sender,
                                                                                        nonce_before := nonce_before,
                                                                                        gas_limit := gas_limit,
                                                                                        intrinsic_execution_gas := costs.intrinsic_execution,
                                                                                        intrinsic_state_gas := costs.intrinsic_state,
                                                                                        calldata_floor := costs.calldata_floor,
                                                                                        blob_fee := costs.blob_fee,
                                                                                        gas_price := eff_gas_price,
                                                                                        priority_fee := eff_priority_fee }))))))))))))))))))))

/-- The upfront effects, taken before the execution snapshot so they persist
across a dispatched-frame revert: charge the full execution/state gas
limit and the EIP-4844 blob-gas fee, bump the sender nonce, and prewarm the
transaction access set. Before Amsterdam, EIP-7702 authorizations are also
applied here; Amsterdam applies them inside the separately reversible
top-frame preparation phase. -/
def apply_transaction_upfront_effects (tx : Transaction) (v : TxValidity) : SailM TxUpfrontResult := do
  let create_target_prestate_empty ← (( do
    if (((fork_gteq (← readReg k_fork) Amsterdam) && tx.is_create) : Bool)
    then (pure (! (← (k_account_exists (← (k_create_addr v.sender v.nonce_before))))))
    else (pure false) ) : SailM Bool )
  (k_sub_balance v.sender (← (validated_word_product v.gas_price v.gas_limit)))
  if ((word_nonzero v.blob_fee) : Bool)
  then (k_sub_balance v.sender v.blob_fee)
  else (pure ())
  (k_bump_nonce v.sender)
  (prewarm tx)
  let authorization_refund ← do
    if ((fork_lt (← readReg k_fork) Amsterdam) : Bool)
    then (process_auth_list tx.authorizations)
    else (pure GAS_REFUND_ZERO)
  (pure { authorization_refund := authorization_refund,
          create_target_prestate_empty := create_target_prestate_empty })

/-- Resets the user-space machine for the transaction's top-level frame,
funding it with `gas_limit − intrinsic`. -/
/- Type quantifiers: k_ex417321_ : Nat, k_ex417320_ : Nat, k_ex417319_ : Nat, 0 ≤ k_ex417319_ ∧
  k_ex417319_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex417320_, 0 ≤ k_ex417321_ -/
def enter_transaction_frame (gas_limit : Nat) (intrinsic_execution : Nat) (intrinsic_state : Nat) : SailM Unit := do
  writeReg pc 0
  writeReg call_depth 0
  let after_execution_intrinsic ← do (validated_gas_sub gas_limit intrinsic_execution)
  let available_gas ← do (validated_gas_sub after_execution_intrinsic intrinsic_state)
  if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
  then
    (do
      let execution_budget ← do (validated_gas_sub AMSTERDAM_TX_MAX_GAS intrinsic_execution)
      if ((available_gas <b execution_budget) : Bool)
      then
        (do
          writeReg gas_remaining available_gas
          writeReg state_gas_remaining GAS_ZERO)
      else
        (do
          writeReg gas_remaining execution_budget
          let remaining_state_gas ← do (validated_gas_sub available_gas execution_budget)
          writeReg state_gas_remaining remaining_state_gas))
  else
    (do
      writeReg gas_remaining available_gas
      writeReg state_gas_remaining GAS_ZERO)
  writeReg state_gas_spilled STATE_GAS_SPILL_ZERO
  writeReg message { caller := ZERO_ADDRESS,
                     address := ZERO_ADDRESS,
                     code_address := ZERO_ADDRESS,
                     value := ZERO_WORD,
                     state_gas_reservoir := ← readReg state_gas_remaining,
                     is_static := false,
                     depth := 0 }
  (stack_reset ())
  (memory_reset ())
  (returndata_clear ())
  writeReg calldata ⟨_, ⟨_, EMPTY_SLICE⟩⟩
  writeReg frame_code EMPTY_CODE
  writeReg frame_refund GAS_REFUND_ZERO
  writeReg frame_status (Running ())

/-- Charges Amsterdam's state-dependent top-level dispatch costs and installs
the code selected for execution. This phase deliberately performs no
revertible account mutation: its state-gas charges are therefore refilled
if the subsequently dispatched frame fails. The result records whether a
call recipient delegated, which disables direct precompile dispatch. -/
def prepare_amsterdam_transaction_dispatch (tx : Transaction) (v : TxValidity) (upfront : TxUpfrontResult) : SailM Bool := do
  let current_target ← do
    if (tx.is_create : Bool)
    then (k_create_addr v.sender v.nonce_before)
    else (pure tx.recipient)
  writeReg message { caller := v.sender,
                     address := current_target,
                     code_address := current_target,
                     value := tx.value,
                     state_gas_reservoir := ← readReg state_gas_remaining,
                     is_static := false,
                     depth := 0 }
  if (tx.is_create : Bool)
  then
    (do
      if (upfront.create_target_prestate_empty : Bool)
      then (charge_state_gas G_amsterdam_state_new_account)
      else (pure ())
      if ((← (is_running ())) : Bool)
      then
        writeReg frame_code (← (code_db_resolve
            (← (code_db_insert (transaction_initcode_slice tx.input_src) (← readReg k_fork)))))
      else (pure ())
      (pure false))
  else
    (do
      writeReg calldata ⟨_, ⟨_, ((tx.input_src).2).2⟩⟩
      if ((← if ((word_nonzero tx.value) : Bool)
           then
             (do
               (k_account_is_empty tx.recipient))
           else (pure false)) : Bool)
      then (charge_state_gas G_amsterdam_state_new_account)
      else (pure ())
      let delegated : Bool := false
      let delegate := ZERO_ADDRESS
      let (delegate, delegated) ← (( do
        if ((← (is_running ())) : Bool)
        then
          (do
            let (is_delegated, target) ← do (k_deleg_target tx.recipient)
            let delegated : Bool := is_delegated
            let delegate : (Vector (BitVec 8) 20) := target
            if (delegated : Bool)
            then
              (do
                let warm ← do (k_access_account delegate)
                (charge (← (account_cost warm))))
            else (pure ())
            (pure (delegate, delegated)))
        else (pure (delegate, delegated)) ) : SailM ((Vector (BitVec 8) 20) × Bool) )
      if ((← (is_running ())) : Bool)
      then
        (do
          if (delegated : Bool)
          then writeReg message { (← readReg message) with code_address := delegate }
          else (pure ())
          writeReg frame_code (← (executable_code tx.recipient delegated delegate)))
      else (pure ())
      (pure delegated))

/-- Runs a create transaction's top-level frame: derives the new address
from `(sender, nonce_before)`, fails outright on an address collision
(all gas consumed, no initcode runs — EIP-684/EIP-7610), and
otherwise deploys via the initcode path. -/
/- Type quantifiers: k_ex417322_ : Nat, 0 ≤ k_ex417322_ ∧ k_ex417322_ ≤ (2 ^ 64 - 1) -/
def run_create_transaction_frame (tx : Transaction) (sender : (Vector (BitVec 8) 20)) (nonce_before : Nat) : SailM Unit := do
  let new_addr ← do (k_create_addr sender nonce_before)
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
      if ((fork_lt (← readReg k_fork) Amsterdam) : Bool)
      then
        (do
          writeReg message { caller := sender,
                             address := new_addr,
                             code_address := new_addr,
                             value := tx.value,
                             state_gas_reservoir := ← readReg state_gas_remaining,
                             is_static := false,
                             depth := 0 }
          writeReg frame_code (← (code_db_resolve
              (← (code_db_insert (transaction_initcode_slice tx.input_src) (← readReg k_fork))))))
      else (pure ())
      let ⟨_, ⟨_, deployed_code⟩⟩ ← do (interpret ())
      if ((← (frame_succeeded ())) : Bool)
      then
        (do
          let dep_len := deployed_code.len
          let deployed_length := dep_len
          if ((← if ((← (deployed_code_size_allowed deployed_length)) : Bool)
               then
                 (do
                   if ((fork_lt (← readReg k_fork) London) : Bool)
                   then (pure true)
                   else
                     (do
                       if ((deployed_length == 0) : Bool)
                       then (pure true)
                       else
                         (do
                           (pure ((← (slice_byte ⟨_, ⟨_, deployed_code⟩⟩ 0)) != 0xEF#8)))))
               else (pure false)) : Bool)
          then
            (do
              match (← (code_deployment_execution_cost dep_len (← readReg gas_remaining))) with
              | .some execution_deposit =>
                (do
                  writeReg gas_remaining (← (gas_sub_or_oog (← readReg gas_remaining)
                      execution_deposit))
                  (charge_deployment_state_gas (← (code_deployment_state_cost dep_len)))
                  if ((← (frame_succeeded ())) : Bool)
                  then
                    (k_deploy_code new_addr (validated_code_slice ⟨_, ⟨_, deployed_code⟩⟩))
                  else (pure ()))
              | none =>
                (do
                  if ((fork_lt (← readReg k_fork) Homestead) : Bool)
                  then
                    (do
                      writeReg gas_remaining GAS_ZERO
                      (k_deploy_code new_addr EMPTY_CODE_SLICE))
                  else (exc_halt OutOfGas)))
          else (exc_halt OutOfGas))
      else (pure ()))

/-- Runs a call transaction's top-level frame: transfers value, then either
runs a direct recipient precompile or interprets the selected code. At
Amsterdam the preparation phase has already resolved and charged a
recipient delegation; a delegated recipient never dispatches a precompile
directly. -/
/- Type quantifiers: k_ex417323_ : Bool -/
def run_call_transaction_frame (tx : Transaction) (sender : (Vector (BitVec 8) 20)) (delegated : Bool) : SailM Unit := do
  let _ ← do (k_aload tx.recipient)
  if ((word_nonzero tx.value) : Bool)
  then (k_transfer sender tx.recipient tx.value)
  else (pure ())
  let selected_precompile ← do (precompile_number tx.recipient)
  if (((! delegated) && (selected_precompile != 0)) : Bool)
  then
    (do
      let number : Nat := selected_precompile
      match (← (precompile_gas number tx.input_src (← readReg gas_remaining))) with
      | .some used =>
        (do
          let result ← do (run_precompile_slice number tx.input_src)
          if (result.success : Bool)
          then
            (do
              writeReg gas_remaining (← (gas_sub_or_oog (← readReg gas_remaining) used))
              if ((← (is_running ())) : Bool)
              then writeReg frame_status (Halted (HaltReturn result.output))
              else (pure ()))
          else (exc_halt OutOfGas))
      | _ => (exc_halt OutOfGas))
  else
    (do
      if ((fork_lt (← readReg k_fork) Amsterdam) : Bool)
      then
        (do
          writeReg calldata ⟨_, ⟨_, ((tx.input_src).2).2⟩⟩
          writeReg message { caller := sender,
                             address := tx.recipient,
                             code_address := tx.recipient,
                             value := tx.value,
                             state_gas_reservoir := ← readReg state_gas_remaining,
                             is_static := false,
                             depth := 0 }
          let (tx_deleg, tx_dtgt) ← do (k_deleg_target tx.recipient)
          if (tx_deleg : Bool)
          then
            (do
              let _ ← do (k_access_account tx_dtgt)
              let _ ← do (k_aload tx_dtgt)
              (pure ()))
          else (pure ())
          writeReg frame_code (← (executable_code tx.recipient tx_deleg tx_dtgt)))
      else (pure ())
      let ⟨_, ⟨_, _⟩⟩ ← do (interpret ())
      (pure ()))

/-- Runs a pre-Amsterdam top-level frame under one execution snapshot. -/
def run_legacy_transaction_frame (tx : Transaction) (v : TxValidity) : SailM TxFrameResult := do
  let checkpoint ← do (k_state_checkpoint ())
  (enter_transaction_frame v.gas_limit v.intrinsic_execution_gas v.intrinsic_state_gas)
  if (tx.is_create : Bool)
  then (run_create_transaction_frame tx v.sender v.nonce_before)
  else (run_call_transaction_frame tx v.sender false)
  let success ← do (frame_succeeded ())
  if ((! success) : Bool)
  then (k_revert checkpoint)
  else (pure ())
  (pure { success := success,
          execution_gas_remaining := ← readReg gas_remaining,
          state_gas_remaining := ← readReg state_gas_remaining,
          state_gas_used := ← (frame_state_gas_used ()),
          refund := ← if (success : Bool)
            then readReg frame_refund
            else (pure GAS_REFUND_ZERO) })

/-- Runs an Amsterdam top-level frame with two rollback boundaries. The
preparation checkpoint contains EIP-7702 authorization writes and is
reverted only when preparation itself runs out of gas. Once preparation
succeeds, the execution checkpoint lets a failed call/create revert while
retaining valid authorization writes and their state-gas charge. -/
def run_amsterdam_transaction_frame (tx : Transaction) (v : TxValidity) (upfront : TxUpfrontResult) : SailM TxFrameResult := do
  (enter_transaction_frame v.gas_limit v.intrinsic_execution_gas v.intrinsic_state_gas)
  let preparation_checkpoint ← do (k_state_checkpoint ())
  let preparation_reservoir ← do readReg state_gas_remaining
  let current_target ← do
    if (tx.is_create : Bool)
    then (k_create_addr v.sender v.nonce_before)
    else (pure tx.recipient)
  let _ ← do
    (process_amsterdam_auth_list tx.authorizations v.sender current_target (word_nonzero tx.value)
      EMPTY_AMSTERDAM_AUTHORIZATION_STATE)
  let authorization_state_gas : Int := FRAME_STATE_GAS_DELTA_ZERO
  let delegated : Bool := false
  let (authorization_state_gas, delegated) ← (( do
    if ((← (is_running ())) : Bool)
    then
      (do
        let authorization_state_gas ← (frame_state_gas_used ())
        writeReg message { (← readReg message) with state_gas_reservoir := ← readReg state_gas_remaining }
        writeReg state_gas_spilled STATE_GAS_SPILL_ZERO
        let delegated ← (prepare_amsterdam_transaction_dispatch tx v upfront)
        (pure (authorization_state_gas, delegated)))
    else (pure (authorization_state_gas, delegated)) ) : SailM (Int × Bool) )
  if ((! (← (is_running ()))) : Bool)
  then
    (do
      (k_revert preparation_checkpoint)
      writeReg message { (← readReg message) with state_gas_reservoir := preparation_reservoir }
      writeReg state_gas_remaining preparation_reservoir
      writeReg state_gas_spilled STATE_GAS_SPILL_ZERO
      (pure { success := false,
              execution_gas_remaining := ← readReg gas_remaining,
              state_gas_remaining := ← readReg state_gas_remaining,
              state_gas_used := STATE_GAS_DELTA_ZERO,
              refund := GAS_REFUND_ZERO }))
  else
    (do
      let execution_checkpoint ← do (k_state_checkpoint ())
      if (tx.is_create : Bool)
      then (run_create_transaction_frame tx v.sender v.nonce_before)
      else (run_call_transaction_frame tx v.sender delegated)
      let success ← do (frame_succeeded ())
      if ((! success) : Bool)
      then (k_revert execution_checkpoint)
      else (pure ())
      (pure { success := success,
              execution_gas_remaining := ← readReg gas_remaining,
              state_gas_remaining := ← readReg state_gas_remaining,
              state_gas_used := ← (pure (authorization_state_gas +i (← (frame_state_gas_used ())))),
              refund := ← if (success : Bool)
                then readReg frame_refund
                else (pure GAS_REFUND_ZERO) }))

/-- Runs the fork-appropriate top-level transaction frame. -/
def run_transaction_frame (tx : Transaction) (v : TxValidity) (upfront : TxUpfrontResult) : SailM TxFrameResult := do
  if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
  then (run_amsterdam_transaction_frame tx v upfront)
  else (run_legacy_transaction_frame tx v)

/- Type quantifiers: total : Int, cap : Nat, 0 ≤ cap, ((- (199 * (2 ^ 64 - 1)))) ≤ total ∧
  total ≤ (199 * (2 ^ 64 - 1)) -/
def capped_transaction_refund (total : Int) (cap : Nat) : Nat :=
  if ((total ≤b 0) : Bool)
  then 0
  else
    (if ((total ≤b ((2 ^i 64) - 1)) : Bool)
    then
      (let admitted_total : Nat := total
      if ((admitted_total ≤b cap) : Bool)
      then admitted_total
      else cap)
    else cap)

/-- Narrows the final signed state-gas charge into the admitted block-gas
domain after clamping credits to zero. -/
/- Type quantifiers: value : Int -/
def admitted_transaction_state_gas (value : Int) : SailM Nat := do
  if ((value ≤b 0) : Bool)
  then (pure GAS_ZERO)
  else
    (do
      if ((value ≤b ((2 ^i 64) - 1)) : Bool)
      then (pure value)
      else sailThrow ((InvalidBlock ExecutionInvalid)))

/-- Settlement (YP §6.2 g*, A_r): applies the capped refund
(`gas_used/5` from London, `gas_used/2` before), the EIP-7623
calldata floor (Prague+), and the EIP-7778 block-gas rule (Amsterdam+:
the block charges the unrefunded, floored gas); returns unused gas to
the sender, pays the coinbase the priority fee, merges the
transaction into the block layer, and emits the receipt. -/
/- Type quantifiers: k_ex417326_ : Int, ((- (199 * (2 ^ 64 - 1)))) ≤ k_ex417326_ ∧
  k_ex417326_ ≤ (199 * (2 ^ 64 - 1)) -/
def settle_transaction (tx : Transaction) (v : TxValidity) (authorization_refund : Int) (fr : TxFrameResult) : SailM Receipt := do
  let gas_left0 := (validated_gas_add fr.execution_gas_remaining fr.state_gas_remaining)
  let gas_used0 ← do (validated_gas_sub v.gas_limit gas_left0)
  let refund_quotient ← (( do
    if ((fork_gteq (← readReg k_fork) London) : Bool)
    then (pure 5)
    else (pure 2) ) : SailM Nat )
  let refund_cap : Nat := (gas_used0 / refund_quotient)
  let total_refund ← do (validated_refund_add authorization_refund fr.refund)
  let refund := (capped_transaction_refund total_refund refund_cap)
  let gas_left1 := (validated_gas_add gas_left0 refund)
  let gas_used1 ← do (validated_gas_sub v.gas_limit gas_left1)
  let floor ← (( do
    if ((fork_gteq (← readReg k_fork) Prague) : Bool)
    then
      (do
        let floor_cost := v.calldata_floor
        let tx_limit := v.gas_limit
        if ((floor_cost ≤b tx_limit) : Bool)
        then (pure floor_cost)
        else sailThrow ((InvalidBlock ExecutionInvalid)))
    else (pure 0) ) : SailM Nat )
  let gas_used : Nat :=
    if ((gas_used1 <b floor) : Bool)
    then floor
    else gas_used1
  let gas_left ← do (validated_gas_sub v.gas_limit gas_used)
  let raw_state_gas : Int := (fr.state_gas_used +i v.intrinsic_state_gas)
  let tx_state_gas ← do (admitted_transaction_state_gas raw_state_gas)
  let execution_before_floor : Nat := GAS_ZERO
  let execution_before_floor ← (( do
    if ((tx_state_gas ≤b gas_used0) : Bool)
    then
      (do
        let reduced_execution_gas ← do (validated_gas_sub gas_used0 tx_state_gas)
        let execution_before_floor : Nat := reduced_execution_gas
        (pure execution_before_floor))
    else (pure execution_before_floor) ) : SailM Nat )
  let execution_gas ← do
    if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
    then
      (if ((execution_before_floor <b floor) : Bool)
      then (pure floor)
      else (pure execution_before_floor))
    else (pure gas_used)
  let state_gas ← do
    if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
    then (pure tx_state_gas)
    else (pure GAS_ZERO)
  (k_add_balance v.sender (← (validated_word_product v.gas_price gas_left)))
  (k_add_balance (← (k_coinbase ())) (← (validated_word_product v.priority_fee gas_used)))
  (k_tx_merge ())
  (pure { tx_type := tx.tx_type,
          success := fr.success,
          gas_used := gas_used,
          execution_gas := execution_gas,
          state_gas := state_gas,
          logs := ← (read_logs ()) })

/-- The complete per-transaction step: reset, validate (an invalid
transaction throws before state changes), apply upfront effects, run the
frame, and settle. -/
def process_transaction (tx : Transaction) : SailM Receipt := do
  let _ : Unit := (cycle_scope_start SCOPE_TX_RESET)
  (k_tx_reset ())
  let _ : Unit := (cycle_scope_end SCOPE_TX_RESET)
  let _ : Unit := (cycle_scope_start SCOPE_TX_VALIDATE)
  let validity ← do (check_transaction_validity tx)
  let _ : Unit := (cycle_scope_end SCOPE_TX_VALIDATE)
  (k_set_tx
    { origin := tx.sender,
      gas_price := validity.gas_price,
      blob_hashes := tx.blob_hashes })
  let _ : Unit := (cycle_scope_start SCOPE_TX_UPFRONT)
  let upfront ← do (apply_transaction_upfront_effects tx validity)
  let _ : Unit := (cycle_scope_end SCOPE_TX_UPFRONT)
  let _ : Unit := (cycle_scope_start SCOPE_TX_FRAME)
  let frame_result ← do (run_transaction_frame tx validity upfront)
  let _ : Unit := (cycle_scope_end SCOPE_TX_FRAME)
  let _ : Unit := (cycle_scope_start SCOPE_TX_SETTLE)
  let receipt ← do (settle_transaction tx validity upfront.authorization_refund frame_result)
  let _ : Unit := (cycle_scope_end SCOPE_TX_SETTLE)
  (pure receipt)

