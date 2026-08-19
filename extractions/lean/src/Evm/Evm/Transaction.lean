import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Primitives.Code
import Evm.Host.RegionAccess
import Evm.Primitives.Crypto
import Evm.Primitives.Fork
import Evm.Primitives.Tx
import Evm.Primitives.Evm
import Evm.Host.Code
import Evm.Lib.Rlp.Decoding
import Evm.Lib.Tx
import Evm.Lib.Rlp.Codecs.Transactions
import Evm.Host.Stack
import Evm.Kernel.Environment
import Evm.Kernel.Storage
import Evm.Kernel.Logs
import Evm.Kernel.Accounts
import Evm.Kernel.Code
import Evm.Kernel.Selfdestruct
import Evm.Kernel.Lifecycle
import Evm.Evm.Machine
import Evm.Evm.Gas
import Evm.Evm.Precompiles
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

abbrev G_transaction : Nat := 21000

abbrev G_txcreate : Nat := 32000

abbrev G_txdatazero : Nat := 4

abbrev G_txdatanonzero : Nat := 16

abbrev G_access_list_address : Nat := 2400

abbrev G_access_list_storage_key : Nat := 1900

abbrev PER_AUTH_BASE : Nat := 12500

abbrev PER_EMPTY_ACCOUNT : Nat := 25000

abbrev AMSTERDAM_TX_BASE : Nat := 12000

abbrev AMSTERDAM_CREATE_ACCESS : Nat := 11000

abbrev AMSTERDAM_COLD_ACCOUNT_ACCESS : Nat := 3000

abbrev AMSTERDAM_TX_VALUE_COST : Nat := 4244

abbrev AMSTERDAM_TRANSFER_LOG_COST : Nat := 1756

abbrev AMSTERDAM_ACCESS_LIST_ADDRESS : Nat := 3000

abbrev AMSTERDAM_ACCESS_LIST_SLOT : Nat := 3000

abbrev AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR : Nat := 1280

abbrev AMSTERDAM_ACCESS_LIST_SLOT_FLOOR : Nat := 2048

abbrev AMSTERDAM_AUTH_BASE : Nat := 7816

abbrev AMSTERDAM_CALLDATA_FLOOR_BYTE : Nat := 64

def undefined_IntrinsicGasCost (_ : Unit) : SailM IntrinsicGasCost := do
  (pure { execution := ← (undefined_range 0 ((2 ^i 64) - 1)),
          state := ← (undefined_range 0 ((2 ^i 64) - 1)),
          calldata_floor := ← (undefined_range 0 ((2 ^i 64) - 1)) })

/-- Reclassifies transaction initcode as executable after re-establishing the
enclosing SSZ transaction-envelope bound. This bound is structural and is
deliberately independent of the active protocol deployment limit. -/
/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, 0 ≤
  input_dependentWitness0 ∧
  0 ≤ input_dependentWitness1 ∧
  (input_dependentWitness0 + input_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ input_dependentWitness1 ∧ input_dependentWitness1 ≤ (2 ^ 30) -/
def transaction_initcode_slice (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Sigma fun
  (input_dependentWitness0 : Nat) =>
  (Sigma fun (input_dependentWitness1 : Nat) =>
  (CodeRegionSliceFields input_dependentWitness0 input_dependentWitness1))) := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  let input_slice := (stateless_input_slice input.bytes input.len)
  (code_db_intern_input ⟨_, ⟨_, input_slice⟩⟩)

def undefined_TxUpfrontResult (_ : Unit) : SailM TxUpfrontResult := do
  (pure { authorization_refund := ← (undefined_range 0 (12500 *i (2 ^i 30))),
          create_target_prestate_empty := ← (undefined_bool ()) })

/-- The EIP-2028 calldata cost: 4 gas per zero byte, 16 per nonzero. One
native pass counts the nonzero bytes; zero bytes are the
remainder. -/
/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, 0 ≤
  input_dependentWitness0 ∧
  0 ≤ input_dependentWitness1 ∧
  (input_dependentWitness0 + input_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ input_dependentWitness1 ∧ input_dependentWitness1 ≤ (2 ^ 30) -/
def calldata_cost (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM Nat := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  let nonzeroes ← do (slice_count_nonzero ⟨_, ⟨_, input⟩⟩)
  let input_len := input.len
  if ((nonzeroes ≤b input_len) : Bool)
  then
    (let zeroes := (input_len - nonzeroes)
    (pure ((G_txdatazero *i zeroes) + (G_txdatanonzero *i nonzeroes))))
  else (fatal_error ExecutionInvalid)

/-- The intrinsic gas of a transaction (YP §6.2, g_0): the 21000 base,
calldata cost, access-list cost (EIP-2930), authorization cost
(EIP-7702), and for creates the `G_txcreate` base plus EIP-3860
initcode words. -/
/- Type quantifiers: tx_dependentWitness0 : Nat, tx_dependentWitness0 = 0 ∨
  tx_dependentWitness0 = 6 ∨ tx_dependentWitness0 = 9 -/
def legacy_intrinsic_gas (tx : (Sigma fun (k_blob_limit : Nat) => (TransactionFields k_blob_limit))) : SailM Nat := do
  let tx_dependentWitness0 := (tx).1
  let tx := (tx).2
  let data_cost ← do (calldata_cost tx.input_src)
  let ⟨_, ⟨_, input⟩⟩ := tx.input_src
  let input_len := input.len
  let address_cost := (G_access_list_address *i tx.access_list.address_count)
  let slot_cost := (G_access_list_storage_key *i tx.access_list.slot_count)
  let ⟨_, authorizations⟩ := tx.authorizations
  let auth_cost := (PER_EMPTY_ACCOUNT *i authorizations.count)
  let common := ((((data_cost + G_transaction) + address_cost) + slot_cost) + auth_cost)
  if (tx.is_create : Bool)
  then (pure ((common + G_txcreate) + (← (transaction_initcode_gas input_len))))
  else (pure common)

/-- Computes the pre-Amsterdam EIP-7623 calldata floor cost. -/
/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, 0 ≤
  input_dependentWitness0 ∧
  0 ≤ input_dependentWitness1 ∧
  (input_dependentWitness0 + input_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ input_dependentWitness1 ∧ input_dependentWitness1 ≤ (2 ^ 30) -/
def legacy_calldata_floor (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM Nat := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  let nonzeroes ← do (slice_count_nonzero ⟨_, ⟨_, input⟩⟩)
  let input_len := input.len
  if ((nonzeroes ≤b input_len) : Bool)
  then
    (let zeroes := (input_len - nonzeroes)
    (pure (((10 *i zeroes) + (40 *i nonzeroes)) + G_transaction)))
  else (fatal_error ExecutionInvalid)

/-- Computes Amsterdam execution-gas charges for recipient access, value
transfer, and contract creation. -/
/- Type quantifiers: tx_dependentWitness0 : Nat, tx_dependentWitness0 = 0 ∨
  tx_dependentWitness0 = 6 ∨ tx_dependentWitness0 = 9 -/
def amsterdam_recipient_execution_cost (tx : (Sigma fun (k_blob_limit : Nat) =>
  (TransactionFields k_blob_limit))) : Nat :=
  let tx_dependentWitness0 := (tx).1
  let tx := (tx).2
  let transfers_value := (word_nonzero tx.value)
  if (tx.is_create : Bool)
  then
    (if (transfers_value : Bool)
    then (AMSTERDAM_CREATE_ACCESS + AMSTERDAM_TRANSFER_LOG_COST)
    else AMSTERDAM_CREATE_ACCESS)
  else
    (if ((bne tx.recipient tx.sender) : Bool)
    then
      (if (transfers_value : Bool)
      then ((AMSTERDAM_COLD_ACCOUNT_ACCESS + AMSTERDAM_TX_VALUE_COST) + AMSTERDAM_TRANSFER_LOG_COST)
      else AMSTERDAM_COLD_ACCOUNT_ACCESS)
    else 0)

/-- Computes the fork-specific execution/state intrinsic costs and calldata
floor. Amsterdam decomposes the transaction charge into the two gas
dimensions introduced by EIP-2780. -/
/- Type quantifiers: tx_dependentWitness0 : Nat, tx_dependentWitness0 = 0 ∨
  tx_dependentWitness0 = 6 ∨ tx_dependentWitness0 = 9 -/
def intrinsic_gas (tx : (Sigma fun (k_blob_limit : Nat) => (TransactionFields k_blob_limit))) : SailM IntrinsicGasCost := do
  let tx_dependentWitness0 := (tx).1
  let tx := (tx).2
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if ((profile.fork <b Amsterdam) : Bool)
  then
    (do
      let execution ← do (legacy_intrinsic_gas ⟨_, tx⟩)
      let calldata_floor ← do (legacy_calldata_floor tx.input_src)
      (pure { execution := execution,
              state := 0,
              calldata_floor := calldata_floor }))
  else
    (do
      let ⟨_, ⟨_, input⟩⟩ := tx.input_src
      let recipient := (amsterdam_recipient_execution_cost ⟨_, tx⟩)
      let address_count := tx.access_list.address_count
      let slot_count := tx.access_list.slot_count
      let access_execution :=
        ((((AMSTERDAM_ACCESS_LIST_ADDRESS *i address_count) + (AMSTERDAM_ACCESS_LIST_SLOT *i slot_count)) + (AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR *i address_count)) + (AMSTERDAM_ACCESS_LIST_SLOT_FLOOR *i slot_count))
      let ⟨_, authorizations⟩ := tx.authorizations
      let authorization_execution := (AMSTERDAM_AUTH_BASE *i authorizations.count)
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

/- Type quantifiers: blob_price : Nat, blob_gas : Nat, 0 ≤ blob_price ∧
  blob_price < (2 ^ 256) ∧
  0 ≤ blob_gas ∧ blob_gas ≤ (prague_blob_max_count * gas_per_blob_value) -/
def transaction_blob_fee (blob_price : Nat) (blob_gas : Nat) : Nat :=
  (blob_price *i blob_gas)

/- Type quantifiers: max_fee : Nat, gas_limit : Nat, value : Nat, max_blob_fee : Nat, blob_gas : Nat, 0
  ≤ max_fee ∧
  max_fee < (2 ^ 256) ∧
  0 ≤ gas_limit ∧
  gas_limit ≤ block_gas_limit_bound ∧
  0 ≤ value ∧
  value < (2 ^ 256) ∧
  0 ≤ max_blob_fee ∧
  max_blob_fee < (2 ^ 256) ∧
  0 ≤ blob_gas ∧ blob_gas ≤ (prague_blob_max_count * gas_per_blob_value) -/
def transaction_upfront_cost (max_fee : Nat) (gas_limit : Nat) (value : Nat) (max_blob_fee : Nat) (blob_gas : Nat) : Nat :=
  (((max_fee *i gas_limit) + value) + (max_blob_fee *i blob_gas))

/-- Computes transaction costs as mathematical naturals, narrowing only the
externally observable word-valued results. -/
/- Type quantifiers: k_ex551178_ : Nat, k_ex551177_ : Nat, tx_dependentWitness0 : Nat, profile_dependentWitness9
  : Nat, profile_dependentWitness8 : Nat, profile_dependentWitness7 : Nat, profile_dependentWitness6
  : Nat, profile_dependentWitness5 : Nat, profile_dependentWitness4 : Nat, profile_dependentWitness3
  : Nat, profile_dependentWitness2 : Nat, profile_dependentWitness1 : Nat, profile_dependentWitness0
  : Nat, profile_dependentWitness0 = 5 ∧
  profile_dependentWitness1 = 0 ∧
  profile_dependentWitness2 = 0 ∧
  profile_dependentWitness3 = 1 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 0 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 0 ∧ profile_dependentWitness9 = 2 ∨
  6 ≤ profile_dependentWitness0 ∧ profile_dependentWitness0 ≤ 9 ∧
  profile_dependentWitness1 = 0 ∧
  profile_dependentWitness2 = 0 ∧
  profile_dependentWitness3 = 1 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 0 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 0 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 10 ∧
  profile_dependentWitness1 = 0 ∧
  profile_dependentWitness2 = 0 ∧
  profile_dependentWitness3 = 1 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 0 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 11 ∧
  profile_dependentWitness1 = 3 ∧
  profile_dependentWitness2 = 6 ∧
  profile_dependentWitness3 = 3338477 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 12 ∧
  profile_dependentWitness1 = 6 ∧
  profile_dependentWitness2 = 9 ∧
  profile_dependentWitness3 = 5007716 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 9 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 13 ∧
  profile_dependentWitness1 = 6 ∧
  profile_dependentWitness2 = 9 ∧
  profile_dependentWitness3 = 5007716 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 24) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 14 ∧
  profile_dependentWitness1 = 10 ∧
  profile_dependentWitness2 = 15 ∧
  profile_dependentWitness3 = 8346193 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 24) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 15 ∧
  profile_dependentWitness1 = 14 ∧
  profile_dependentWitness2 = 21 ∧
  profile_dependentWitness3 = 11684671 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 24) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 16 ∧
  profile_dependentWitness1 = 14 ∧
  profile_dependentWitness2 = 21 ∧
  profile_dependentWitness3 = 11684671 ∧
  profile_dependentWitness4 = 65536 ∧
  profile_dependentWitness5 = 131072 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5, tx_dependentWitness0 = 0 ∨
  tx_dependentWitness0 = 6 ∨ tx_dependentWitness0 = 9, 0 ≤ k_ex551177_ ∧
  k_ex551177_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551178_ ∧ k_ex551178_ ≤ (256 * 11684671 + 21 * 2 ^ 17) -/
def transaction_costs (profile : (Sigma fun (k_fork : Nat) =>
  (Sigma fun (k_target : Nat) =>
  (Sigma fun (k_maximum : Nat) =>
  (Sigma fun (k_denominator : Nat) =>
  (Sigma fun (k_code_limit : Nat) =>
  (Sigma fun (k_initcode_limit : Nat) =>
  (Sigma fun (k_transaction_total_gas_limit : Nat) =>
  (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
  (Sigma fun (k_transaction_blob_limit : Nat) =>
  (Sigma fun (k_refund_divisor : Nat) =>
  (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))) (tx : (Sigma
  fun (k_blob_limit : Nat) => (TransactionFields k_blob_limit))) (gas_limit : Nat) (excess_blob_gas : Nat) : SailM TransactionCosts := do
  let profile_dependentWitness0 := (profile).1
  let profile_dependentWitness1 := ((profile).2).1
  let profile_dependentWitness2 := (((profile).2).2).1
  let profile_dependentWitness3 := ((((profile).2).2).2).1
  let profile_dependentWitness4 := (((((profile).2).2).2).2).1
  let profile_dependentWitness5 := ((((((profile).2).2).2).2).2).1
  let profile_dependentWitness6 := (((((((profile).2).2).2).2).2).2).1
  let profile_dependentWitness7 := ((((((((profile).2).2).2).2).2).2).2).1
  let profile_dependentWitness8 := (((((((((profile).2).2).2).2).2).2).2).2).1
  let profile_dependentWitness9 := ((((((((((profile).2).2).2).2).2).2).2).2).2).1
  let profile := ((((((((((profile).2).2).2).2).2).2).2).2).2).2
  let tx_dependentWitness0 := (tx).1
  let tx := (tx).2
  let intrinsic ← do (intrinsic_gas ⟨_, tx⟩)
  let blob_gas : Nat := ((2 ^i 17) *i tx.blob_hashes.count)
  let blob_fee_value ← (( do
    if ((blob_gas == 0) : Bool)
    then (pure 0)
    else
      (do
        let blob_price ← do
          (blob_base_fee profile.fork profile.blob_schedule profile.excess_blob_gas_limit
            excess_blob_gas)
        if ((blob_price ≤b tx.max_blob_fee) : Bool)
        then (pure (transaction_blob_fee blob_price blob_gas))
        else (fatal_error ExecutionInvalid)) ) : SailM Nat )
  let upfront_value :=
    (transaction_upfront_cost tx.max_fee gas_limit tx.value tx.max_blob_fee blob_gas)
  if (((blob_fee_value <b (2 ^i 256)) && (upfront_value <b (2 ^i 256))) : Bool)
  then
    (pure { intrinsic_execution := intrinsic.execution,
            intrinsic_state := intrinsic.state,
            calldata_floor := intrinsic.calldata_floor,
            blob_gas := blob_gas,
            blob_fee := (protocol_word blob_fee_value),
            upfront := (protocol_word upfront_value) })
  else (fatal_error ExecutionInvalid)

/- Type quantifiers: value : Nat, factor : Nat, 0 ≤ factor ∧ factor < (2 ^ 256), 0 ≤ value ∧
  value ≤ (2 ^ 256 - 1) -/
def validated_word_product (value : Nat) (factor : Nat) : SailM Nat := do
  let product := (value *i factor)
  if ((product <b (2 ^i 256)) : Bool)
  then (pure (protocol_word product))
  else (fatal_error ExecutionInvalid)

/- Type quantifiers: state_delta : Int, k_limit : Nat, k_regular : Nat, k_intrinsic_execution : Nat, k_intrinsic_state
  : Nat, k_calldata_floor : Nat, k_initial_execution : Nat, k_initial_state : Nat, execution : Nat, state
  : Nat, (transaction_initial_gas_relation k_limit k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_initial_execution k_initial_state)
  ∧ (live_gas_valid execution) ∧ (live_gas_valid state), ((- (2 * (2 ^ 64 - 1)))) ≤
  state_delta ∧ state_delta ≤ (2 * (2 ^ 64 - 1 + 2 ^ 24)) -/
def tx_frame_gas_snapshot (initial : (TransactionInitialGasFields k_limit k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_initial_execution k_initial_state)) (execution : Nat) (state : Nat) (state_delta : Int) : SailM (Sigma
  fun (k_syn_calldata_floor : Nat) =>
  (Sigma fun (k_remaining : Nat) =>
  (Sigma fun (k_state_used : Nat) =>
  (TxFrameGasSnapshotFields k_limit k_regular k_syn_calldata_floor k_remaining k_state_used)))) := do
  let limit := k_limit
  let regular := k_regular
  if _sailIf0 : ((execution ≤b limit) : Bool) = true
  then
    (do
      let room := (limit - execution)
      if _sailIf1 : ((state ≤b room) : Bool) = true
      then
        (do
          let remaining := (execution + state)
          let spent := (limit - remaining)
          let raw_state_used : Int := (k_intrinsic_state +i state_delta)
          if _sailIf2 : ((raw_state_used ≤b 0) : Bool) = true
          then
            (do
              if _sailIf3 : ((spent ≤b regular) : Bool) = true
              then
                (pure ((⟨_, ⟨_, ⟨_, (tx_frame_gas_snapshot_fields limit regular
                    k_calldata_floor remaining 0)⟩⟩⟩ : (Sigma fun (k_syn_calldata_floor : Nat)
                  =>
                  (Sigma fun (k_syn_remaining : Nat) =>
                  (Sigma fun (k_state_used : Nat) =>
                  (TxFrameGasSnapshotFields k_limit k_regular k_syn_calldata_floor k_syn_remaining k_state_used))))) : (Sigma
                  fun (k_syn_calldata_floor : Nat) =>
                  (Sigma fun (k_syn_remaining : Nat) =>
                  (Sigma fun (k_state_used : Nat) =>
                  (TxFrameGasSnapshotFields k_limit k_regular k_syn_calldata_floor k_syn_remaining k_state_used))))))
              else
                (do
                  (fatal_error ExecutionInvalid)))
          else
            (do
              let positive_state_used : Nat := raw_state_used
              if _sailIf3 : ((positive_state_used ≤b spent) : Bool) = true
              then
                (do
                  let bounded_state_used : Nat := positive_state_used
                  if _sailIf4 : (((spent - bounded_state_used) ≤b regular) : Bool) = true
                  then
                    (pure ((⟨_, ⟨_, ⟨_, (tx_frame_gas_snapshot_fields limit regular
                        k_calldata_floor remaining bounded_state_used)⟩⟩⟩ : (Sigma fun
                      (k_syn_calldata_floor : Nat) =>
                      (Sigma fun (k_syn_remaining : Nat) =>
                      (Sigma fun (k_state_used : Nat) =>
                      (TxFrameGasSnapshotFields k_limit k_regular k_syn_calldata_floor k_syn_remaining k_state_used))))) : (Sigma
                      fun (k_syn_calldata_floor : Nat) =>
                      (Sigma fun (k_syn_remaining : Nat) =>
                      (Sigma fun (k_state_used : Nat) =>
                      (TxFrameGasSnapshotFields k_limit k_regular k_syn_calldata_floor k_syn_remaining k_state_used))))))
                  else
                    (do
                      (fatal_error ExecutionInvalid)))
              else
                (do
                  (fatal_error ExecutionInvalid))))
      else
        (do
          (fatal_error ExecutionInvalid)))
  else
    (do
      (fatal_error ExecutionInvalid))

/- Type quantifiers: value : Nat, _total_limit : Nat, regular_limit : Nat, 0 ≤ value ∧
  0 ≤ regular_limit ∧
  regular_limit ≤ _total_limit ∧
  value ≤ _total_limit ∧ _total_limit ≤ block_gas_limit_bound -/
def transaction_gas_allowance_fields (value : Nat) (_total_limit : Nat) (regular_limit : Nat) : (TransactionGasAllowanceFields value (if ( value
  < regular_limit  : Bool) then value else regular_limit)) :=
  let regular :=
    if ((value <b regular_limit) : Bool)
    then value
    else regular_limit
  {  }

/- Type quantifiers: value : Nat, total_limit : Nat, regular_limit : Nat, 0 ≤ value ∧
  0 ≤ regular_limit ∧ regular_limit ≤ total_limit ∧ total_limit ≤ block_gas_limit_bound -/
def transaction_gas_allowance (value : Nat) (total_limit : Nat) (regular_limit : Nat) : SailM (Sigma
  fun (k_total : Nat) =>
  (Sigma fun (k_regular : Nat) => (TransactionGasAllowanceFields k_total k_regular))) := do
  if _sailIf0 : ((total_limit <b value) : Bool) = true
  then
    (do
      (fatal_error ExecutionInvalid))
  else
    (pure ((⟨_, ⟨_, (transaction_gas_allowance_fields value total_limit regular_limit)⟩⟩ : (Sigma
      fun (k_total : Nat) =>
      (Sigma fun (k_regular : Nat) => (TransactionGasAllowanceFields k_total k_regular)))) : (Sigma
      fun (k_total : Nat) =>
      (Sigma fun (k_regular : Nat) => (TransactionGasAllowanceFields k_total k_regular)))))

/- Type quantifiers: k_total : Nat, k_regular : Nat, intrinsic_execution : Nat, intrinsic_state :
  Nat, calldata_floor : Nat, 0 ≤ k_regular ∧
  k_regular ≤ k_total ∧
  k_total ≤ block_gas_limit_bound ∧
  0 ≤ intrinsic_execution ∧ 0 ≤ intrinsic_state ∧ 0 ≤ calldata_floor -/
def transaction_initial_gas (allowance : (TransactionGasAllowanceFields k_total k_regular)) (intrinsic_execution : Nat) (intrinsic_state : Nat) (calldata_floor : Nat) : SailM (Sigma
  fun (k_syn_intrinsic_execution : Nat) =>
  (Sigma fun (k_syn_intrinsic_state : Nat) =>
  (Sigma fun (k_syn_calldata_floor : Nat) =>
  (Sigma fun (k_execution : Nat) =>
  (Sigma fun (k_state : Nat) =>
  (TransactionInitialGasFields k_total k_regular k_syn_intrinsic_execution k_syn_intrinsic_state k_syn_calldata_floor k_execution k_state)))))) := do
  if _sailIf0 : ((k_total <b intrinsic_execution) : Bool) = true
  then
    (do
      (fatal_error ExecutionInvalid))
  else
    (do
      let after_execution := (k_total - intrinsic_execution)
      if _sailIf1 : ((after_execution <b intrinsic_state) : Bool) = true
      then
        (do
          (fatal_error ExecutionInvalid))
      else
        (do
          if _sailIf2 : (((k_regular <b intrinsic_execution) || (k_regular <b calldata_floor)) : Bool) = true
          then
            (do
              (fatal_error ExecutionInvalid))
          else
            (let available := (after_execution - intrinsic_state)
            let regular_budget := (k_regular - intrinsic_execution)
            if _sailIf3 : ((available <b regular_budget) : Bool) = true
            then
              (pure ((⟨_, ⟨_, ⟨_, ⟨_, ⟨_, (transaction_initial_gas_fields k_total
                  k_regular intrinsic_execution intrinsic_state calldata_floor available 0)⟩⟩⟩⟩⟩ : (Sigma
                fun (k_syn_intrinsic_execution : Nat) =>
                (Sigma fun (k_syn_intrinsic_state : Nat) =>
                (Sigma fun (k_syn_calldata_floor : Nat) =>
                (Sigma fun (k_execution : Nat) =>
                (Sigma fun (k_state : Nat) =>
                (TransactionInitialGasFields k_total k_regular k_syn_intrinsic_execution k_syn_intrinsic_state k_syn_calldata_floor k_execution k_state))))))) : (Sigma
                fun (k_syn_intrinsic_execution : Nat) =>
                (Sigma fun (k_syn_intrinsic_state : Nat) =>
                (Sigma fun (k_syn_calldata_floor : Nat) =>
                (Sigma fun (k_execution : Nat) =>
                (Sigma fun (k_state : Nat) =>
                (TransactionInitialGasFields k_total k_regular k_syn_intrinsic_execution k_syn_intrinsic_state k_syn_calldata_floor k_execution k_state))))))))
            else
              (pure ((⟨_, ⟨_, ⟨_, ⟨_, ⟨_, (transaction_initial_gas_fields k_total
                  k_regular intrinsic_execution intrinsic_state calldata_floor regular_budget
                  (available - regular_budget))⟩⟩⟩⟩⟩ : (Sigma fun
                (k_syn_intrinsic_execution : Nat) =>
                (Sigma fun (k_syn_intrinsic_state : Nat) =>
                (Sigma fun (k_syn_calldata_floor : Nat) =>
                (Sigma fun (k_execution : Nat) =>
                (Sigma fun (k_state : Nat) =>
                (TransactionInitialGasFields k_total k_regular k_syn_intrinsic_execution k_syn_intrinsic_state k_syn_calldata_floor k_execution k_state))))))) : (Sigma
                fun (k_syn_intrinsic_execution : Nat) =>
                (Sigma fun (k_syn_intrinsic_state : Nat) =>
                (Sigma fun (k_syn_calldata_floor : Nat) =>
                (Sigma fun (k_execution : Nat) =>
                (Sigma fun (k_state : Nat) =>
                (TransactionInitialGasFields k_total k_regular k_syn_intrinsic_execution k_syn_intrinsic_state k_syn_calldata_floor k_execution k_state)))))))))))

/-- Applies one EIP-7702 authorization: validates it against current
state, sets or clears the delegation, bumps the authority nonce, and
refunds if the authority already existed. The signature and chain id
are validated from the tuple alone before the authority's account is
read — a tuple rejected there touches no state, so its authority need
not be witnessed; every authority-state read is gated on those
checks. The authority is warmed before the code/nonce
checks, so a tuple later skipped still warms it. -/
def process_auth (au : Authorization) : SailM Nat := do
  let refund : Nat := 0
  let authority := au.authority
  let chain_id_is_zero := (word_is_zero au.chain_id)
  let expected_chain_id ← do (pure (word_of_chain_identifier (← readReg k_chain_id)))
  let chain_id_matches := (au.chain_id == expected_chain_id)
  if ((au.valid_sig && (chain_id_is_zero || chain_id_matches)) : Bool)
  then
    (do
      (k_account_mark_warm authority)
      let (is_deleg, _) ← do (k_deleg_target authority)
      let code_key ← do (k_code_key authority)
      let nonce ← do (k_get_nonce authority)
      if ((((code_key == KECCAK_EMPTY) || is_deleg) && ((nonce == au.nonce) : Bool)) : Bool)
      then
        (do
          let existed ← do (k_account_exists authority)
          if ((au.address == ZERO_ADDRESS) : Bool)
          then (k_clear_code authority)
          else (k_set_delegation authority au.address)
          (k_bump_nonce authority)
          if (existed : Bool)
          then
            (let refund : Nat := (PER_EMPTY_ACCOUNT - PER_AUTH_BASE)
            (pure refund))
          else (pure refund))
      else (pure refund))
  else (pure refund)

/-- Adds one authorization refund to the transaction-wide accumulator. The
decoded authorization count proves this guard unreachable in valid input;
spelling it at the narrowing boundary keeps proof extraction independent
of the Rocq backend's treatment of existential range indices. -/
/- Type quantifiers: k_ex551246_ : Nat, k_ex551245_ : Nat, 0 ≤ k_ex551245_ ∧
  k_ex551245_ ≤ 12500, 0 ≤ k_ex551246_ ∧ k_ex551246_ ≤ (12500 * 2 ^ 30) -/
def authorization_refund_add (item : Nat) (accumulated : Nat) : SailM Nat := do
  let bound := (12500 *i (2 ^i 30))
  if ((accumulated ≤b (bound - item)) : Bool)
  then (pure (item + accumulated))
  else (fatal_error ExecutionInvalid)

/- Type quantifiers: _reclimit : Nat, count : Nat, 0 ≤ count ∧ count ≤ ((2 ^ 24) / 7816), 0
  ≤ _reclimit -/
def _rec_process_auth_cursor (authorizations : PreparedAuthorizationList) (count : Nat) (_reclimit : Nat) : SailM Nat := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((count == 0) : Bool)
      then (pure 0)
      else
        (do
          let authorization ← do (prepared_authorization_head authorizations)
          let remaining ← do (prepared_authorization_tail authorizations count)
          let item_refund ← do (process_auth authorization)
          let remaining_refund ← do
            (_rec_process_auth_cursor remaining (count - 1) _reclimit_pred)
          (authorization_refund_add item_refund remaining_refund)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ ((2 ^ 24) / 7816) -/
def process_auth_cursor (authorizations : PreparedAuthorizationList) (count : Nat) : SailM Nat := do
  let _measure := (count : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_process_auth_cursor authorizations count (_measure + 1))

/-- Applies a prepared authorization collection in order. -/
def process_auth_list (authorizations : PreparedAuthorizationList) : SailM Nat := do
  (process_auth_cursor authorizations authorizations.count)

/-- Applies one Amsterdam authorization and charges its state-dependent
execution-gas and state-gas components. Tuple-local signature and chain
checks precede all authority-state reads; a valid tuple warms its authority
before checking code and nonce, as required by EIP-7702. -/
/- Type quantifiers: k_ex551254_ : Nat, k_ex551253_ : Nat, k_ex551252_ : Nat, k_ex551251_ : Bool, 0
  ≤ k_ex551252_ ∧ k_ex551252_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551253_ ∧
  k_ex551253_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551254_ ∧ k_ex551254_ ≤ (2 ^ 24) -/
def process_amsterdam_auth (au : Authorization) (sender : (Vector (BitVec 8) 20)) (current_target : (Vector (BitVec 8) 20)) (transfers_value : Bool) (carried_gas : Nat) (carried_state_gas : Nat) (carried_state_spill : Nat) : SailM (Bool × Nat × Nat × Nat) := SailME.run do
  let gas_after : Nat := carried_gas
  let state_gas_after : Nat := carried_state_gas
  let state_spill_after : Nat := carried_state_spill
  let authority := au.authority
  let chain_id_is_zero := (word_is_zero au.chain_id)
  let expected_chain_id ← do (pure (word_of_chain_identifier (← readReg k_chain_id)))
  let chain_id_matches := (au.chain_id == expected_chain_id)
  let (gas_after, state_gas_after, state_spill_after) ← (( do
    if ((au.valid_sig && (chain_id_is_zero || chain_id_matches)) : Bool)
    then
      (do
        (k_account_mark_warm authority)
        let (currently_delegated, _) ← do (k_deleg_target authority)
        let code_key ← do (k_code_key authority)
        let nonce ← do (k_get_nonce authority)
        let (gas_after, state_gas_after, state_spill_after) ← (( do
          if ((((code_key == KECCAK_EMPTY) || currently_delegated) && ((nonce == au.nonce) : Bool)) : Bool)
          then
            (do
              let seen ← do (authorization_tracker_seen authority)
              let delegated_before_tx ← do
                if (seen : Bool)
                then (authorization_tracker_originally_delegated authority)
                else (pure currently_delegated)
              let already_written :=
                (seen || ((authority == sender) || (transfers_value && (authority == current_target))))
              let account_exists ← do (k_account_exists authority)
              let account_missing := (! account_exists)
              let (gas_after, state_gas_after, state_spill_after) ← (( do
                if (account_missing : Bool)
                then
                  (do
                    let (state_gas_halt, next_gas, next_state_gas, next_state_spill) ← do
                      (charge_state_gas gas_after state_gas_after state_spill_after
                        G_amsterdam_state_new_account)
                    let gas_after : Nat := next_gas
                    let state_gas_after : Nat := next_state_gas
                    let state_spill_after : Nat := next_state_spill
                    if (state_gas_halt : Bool)
                    then
                      SailME.throw ((false, gas_after, state_gas_after, state_spill_after) : (Bool × Nat × Nat × Nat))
                    else (pure ())
                    (pure (gas_after, state_gas_after, state_spill_after)))
                else (pure (gas_after, state_gas_after, state_spill_after)) ) : SailME
                (Bool × Nat × Nat × Nat) (Nat × Nat × Nat) )
              let requires_account_write := (! already_written)
              let gas_after ← (( do
                if (requires_account_write : Bool)
                then
                  (do
                    let gas_after ← (( do
                      if ((gas_after <b G_amsterdam_account_write) : Bool)
                      then
                        SailME.throw ((false, GAS_ZERO, state_gas_after, state_spill_after) : (Bool × Nat × Nat × Nat))
                      else (pure (gas_sub gas_after G_amsterdam_account_write)) ) : SailME
                      (Bool × Nat × Nat × Nat) Nat )
                    (pure gas_after))
                else (pure gas_after) ) : SailME (Bool × Nat × Nat × Nat) Nat )
              let not_delegated_before_tx := (! delegated_before_tx)
              let delegation_set ← do (authorization_tracker_delegation_set authority)
              let delegation_not_set := (! delegation_set)
              let creates_delegation := (bne au.address ZERO_ADDRESS)
              let (gas_after, state_gas_after, state_spill_after) ← (( do
                if ((creates_delegation && (not_delegated_before_tx && delegation_not_set)) : Bool)
                then
                  (do
                    let (auth_state_gas_halt, auth_gas, auth_state_gas, auth_state_spill) ← do
                      (charge_state_gas gas_after state_gas_after state_spill_after
                        G_amsterdam_state_auth_base)
                    let gas_after : Nat := auth_gas
                    let state_gas_after : Nat := auth_state_gas
                    let state_spill_after : Nat := auth_state_spill
                    if (auth_state_gas_halt : Bool)
                    then
                      SailME.throw ((false, gas_after, state_gas_after, state_spill_after) : (Bool × Nat × Nat × Nat))
                    else (pure ())
                    (pure (gas_after, state_gas_after, state_spill_after)))
                else (pure (gas_after, state_gas_after, state_spill_after)) ) : SailME
                (Bool × Nat × Nat × Nat) (Nat × Nat × Nat) )
              if ((au.address == ZERO_ADDRESS) : Bool)
              then (k_clear_code authority)
              else (k_set_delegation authority au.address)
              (k_bump_nonce authority)
              let unseen := (! seen)
              let originally_delegated := (unseen && currently_delegated)
              (authorization_tracker_commit authority originally_delegated creates_delegation)
              (pure (gas_after, state_gas_after, state_spill_after)))
          else (pure (gas_after, state_gas_after, state_spill_after)) ) : SailME
          (Bool × Nat × Nat × Nat) (Nat × Nat × Nat) )
        (pure (gas_after, state_gas_after, state_spill_after)))
    else (pure (gas_after, state_gas_after, state_spill_after)) ) : SailME
    (Bool × Nat × Nat × Nat) (Nat × Nat × Nat) )
  (pure (true, gas_after, state_gas_after, state_spill_after))

/- Type quantifiers: _reclimit : Nat, state_spill : Nat, state_gas : Nat, gas : Nat, k_ex551256_ :
  Bool, count : Nat, 0 ≤ count ∧ count ≤ ((2 ^ 24) / 7816), 0 ≤ gas ∧ gas ≤ (2 ^ 64 - 1), 0
  ≤ state_gas ∧ state_gas ≤ (2 ^ 64 - 1), 0 ≤ state_spill ∧ state_spill ≤ (2 ^ 24), 0
  ≤ _reclimit -/
def _rec_process_amsterdam_auth_cursor (authorizations : PreparedAuthorizationList) (count : Nat) (sender : (Vector (BitVec 8) 20)) (current_target : (Vector (BitVec 8) 20)) (transfers_value : Bool) (gas : Nat) (state_gas : Nat) (state_spill : Nat) (_reclimit : Nat) : SailM (Bool × Nat × Nat × Nat) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((count == 0) : Bool)
      then (pure (true, gas, state_gas, state_spill))
      else
        (do
          let authorization ← do (prepared_authorization_head authorizations)
          let remaining ← do (prepared_authorization_tail authorizations count)
          let (processed, gas_after, state_gas_after, state_spill_after) ← do
            (process_amsterdam_auth authorization sender current_target transfers_value gas
              state_gas state_spill)
          if (processed : Bool)
          then
            (_rec_process_amsterdam_auth_cursor remaining (count - 1) sender current_target
              transfers_value gas_after state_gas_after state_spill_after _reclimit_pred)
          else (pure (false, gas_after, state_gas_after, state_spill_after))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: state_spill : Nat, state_gas : Nat, gas : Nat, k_ex551262_ : Bool, count : Nat, 0
  ≤ count ∧ count ≤ ((2 ^ 24) / 7816), 0 ≤ gas ∧ gas ≤ (2 ^ 64 - 1), 0 ≤ state_gas ∧
  state_gas ≤ (2 ^ 64 - 1), 0 ≤ state_spill ∧ state_spill ≤ (2 ^ 24) -/
def process_amsterdam_auth_cursor (authorizations : PreparedAuthorizationList) (count : Nat) (sender : (Vector (BitVec 8) 20)) (current_target : (Vector (BitVec 8) 20)) (transfers_value : Bool) (gas : Nat) (state_gas : Nat) (state_spill : Nat) : SailM (Bool × Nat × Nat × Nat) := do
  let _measure := (count : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else
    (_rec_process_amsterdam_auth_cursor authorizations count sender current_target transfers_value
      gas state_gas state_spill (_measure + 1))

/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_warm_access_list_keys (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (addr : (Vector (BitVec 8) 20)) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then (pure ())
      else
        (do
          let ⟨_, ⟨_, key⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor key.source.len)
          let slot ← do (rlp_decode_word key)
          (k_prewarm_slot addr slot)
          (_rec_warm_access_list_keys next addr _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def warm_access_list_keys (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (addr : (Vector (BitVec 8) 20)) : SailM Unit := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_warm_access_list_keys cursor addr (_measure + 1))

/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_warm_access_list (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then (pure ())
      else
        (do
          let ⟨_, ⟨_, entry⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor entry.source.len)
          let fields ← do (rlp_decode_list entry)
          let ⟨_, ⟨_, addr_f⟩⟩ ← do (rlp_decode_item fields)
          let fields := (rlp_cursor_advance fields addr_f.source.len)
          let ⟨_, ⟨_, keys_f⟩⟩ ← do (rlp_decode_item fields)
          let fields := (rlp_cursor_advance fields keys_f.source.len)
          (rlp_cursor_expect_end fields)
          let addr_word ← do (rlp_decode_word addr_f)
          let addr := (word_to_address addr_word)
          (k_account_mark_warm addr)
          let keys ← do (rlp_decode_list keys_f)
          (warm_access_list_keys keys addr)
          (_rec_warm_access_list next _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def warm_access_list (cursor : (StatelessInputSliceFields k_source_off k_source_len)) : SailM Unit := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_warm_access_list cursor (_measure + 1))

/-- Pre-warms the accessed-address set (EIP-2929): the sender, the call
target, and the access list (EIP-2930); EIP-3651 additionally warms the
coinbase from Shanghai onward. Active precompiles are a fork-derived warm
class in [k_account_is_warm][] and therefore need no table entries. -/
/- Type quantifiers: tx_dependentWitness0 : Nat, tx_dependentWitness0 = 0 ∨
  tx_dependentWitness0 = 6 ∨ tx_dependentWitness0 = 9 -/
def prewarm (tx : (Sigma fun (k_blob_limit : Nat) => (TransactionFields k_blob_limit))) : SailM Unit := do
  let tx_dependentWitness0 := (tx).1
  let tx := (tx).2
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  (k_account_mark_warm tx.sender)
  if (tx.is_create : Bool)
  then (pure ())
  else (k_account_mark_warm tx.recipient)
  if ((profile.fork ≥b Shanghai) : Bool)
  then
    (do
      let coinbase ← do (k_coinbase ())
      (k_account_mark_warm coinbase))
  else (pure ())
  let ⟨_, ⟨_, access_list⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) :=
    (tx.access_list.encoded : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))))
  (warm_access_list access_list)

/-- The EIP-1559 effective fee: the gas price actually paid is
`min(max_fee, base_fee + max_priority_fee)`, and the priority tip
paid to the coinbase is that price minus the base fee. Legacy and
EIP-2930 transactions carry a single `gas_price`, passed as
`max_fee = max_priority = gas_price`, so this recovers
`(gas_price, gas_price − base_fee)`. The priority is clamped at 0 so
an invalid sub-base-fee price (rejected later by validity) never
underflows. -/
/- Type quantifiers: k_ex551324_ : Nat, k_ex551323_ : Nat, k_ex551322_ : Nat, 0 ≤ k_ex551322_ ∧
  k_ex551322_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex551323_ ∧ k_ex551323_ ≤ (2 ^ 256 - 1), 0 ≤
  k_ex551324_ ∧ k_ex551324_ ≤ (2 ^ 256 - 1) -/
def eff_gas_price_for (base_fee : Nat) (max_fee : Nat) (max_priority_fee : Nat) : (Nat × Nat) :=
  let max_fee_below_base := (word_ule max_fee base_fee)
  let price : Nat :=
    if (max_fee_below_base : Bool)
    then max_fee
    else
      (let available_priority := (word_sub_word max_fee base_fee)
      let priority_within_cap := (word_ule max_priority_fee available_priority)
      if (priority_within_cap : Bool)
      then (word_add_word base_fee max_priority_fee)
      else max_fee)
  let base_fee_covered := (word_ule base_fee price)
  let priority :=
    if (base_fee_covered : Bool)
    then (word_sub_word price base_fee)
    else ZERO_WORD
  (price, priority)

/- Type quantifiers: tx_dependentWitness0 : Nat, k_total : Nat, k_regular : Nat, 0 ≤ k_regular ∧
  k_regular ≤ k_total ∧ k_total ≤ block_gas_limit_bound, tx_dependentWitness0 = 0 ∨
  tx_dependentWitness0 = 6 ∨ tx_dependentWitness0 = 9 -/
def check_transaction_validity (tx : (Sigma fun (k_blob_limit : Nat) =>
  (TransactionFields k_blob_limit))) (allowance : (TransactionGasAllowanceFields k_total k_regular)) : SailM (Sigma
  fun (k_intrinsic_execution : Nat) =>
  (Sigma fun (k_intrinsic_state : Nat) =>
  (Sigma fun (k_calldata_floor : Nat) =>
  (Sigma fun (k_execution : Nat) =>
  (Sigma fun (k_state : Nat) =>
  (TxValidityFields k_total k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state)))))) := do
  let tx_dependentWitness0 := (tx).1
  let tx := (tx).2
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let tx_semantics := (tx_type_semantics tx.tx_type)
  let parity ← do (tx_signature_parity (← readReg k_chain_id) tx_semantics.signature tx.sig_v)
  let authenticated ← do (tx_auth_valid tx.sender tx.signing_hash parity tx.sig_r tx.sig_s)
  let invalid_signature := (! authenticated)
  if (invalid_signature : Bool)
  then (fatal_error InvalidSignature)
  else (pure ())
  let gas_limit := k_total
  let (eff_gas_price, eff_priority_fee) ← do
    (pure (eff_gas_price_for (← readReg k_header).base_fee tx.max_fee tx.max_priority_fee))
  let sender := tx.sender
  let ⟨_, ⟨_, input⟩⟩ := tx.input_src
  let input_len := input.len
  let nonce_before ← do (k_get_nonce sender)
  let costs ← do
    (transaction_costs
      ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩
      ⟨_, tx⟩ gas_limit (← readReg k_header).excess_blob_gas)
  let expected_nonce := (word_of_account_nonce nonce_before)
  if ((tx.nonce != expected_nonce) : Bool)
  then (fatal_error ExecutionInvalid)
  else (pure ())
  let (sender_deleg, _) ← do (k_deleg_target sender)
  if (tx_semantics.blob : Bool)
  then
    (do
      if (((profile.fork <b Cancun) || (((tx.blob_hashes.count == 0) || tx.is_create) : Bool)) : Bool)
      then (fatal_error ExecutionInvalid)
      else (pure ()))
  else (pure ())
  if (((profile.fork ≥b Prague) && ((gas_limit <b costs.calldata_floor) : Bool)) : Bool)
  then (fatal_error ExecutionInvalid)
  else (pure ())
  let sender_balance ← do (k_get_balance sender)
  let upfront_affordable := (word_ule costs.upfront sender_balance)
  let insufficient_balance := (! upfront_affordable)
  if (insufficient_balance : Bool)
  then (fatal_error ExecutionInvalid)
  else (pure ())
  let sender_code_key ← do (k_code_key sender)
  let valid_sender_code := ((sender_code_key == KECCAK_EMPTY) || sender_deleg)
  let invalid_sender_code := (! valid_sender_code)
  if (invalid_sender_code : Bool)
  then (fatal_error ExecutionInvalid)
  else (pure ())
  if ((k_regular <b costs.calldata_floor) : Bool)
  then (fatal_error ExecutionInvalid)
  else (pure ())
  let base_fee_affordable ← do (pure (word_ule (← readReg k_header).base_fee tx.max_fee))
  let base_fee_exceeds_cap := (! base_fee_affordable)
  if (base_fee_exceeds_cap : Bool)
  then (fatal_error ExecutionInvalid)
  else (pure ())
  let valid_initcode_size ← do (initcode_size_allowed input_len)
  let invalid_initcode_size := (! valid_initcode_size)
  if ((tx.is_create && invalid_initcode_size) : Bool)
  then (fatal_error ExecutionInvalid)
  else (pure ())
  let valid_priority_fee := (word_ule tx.max_priority_fee tx.max_fee)
  let invalid_priority_fee := (! valid_priority_fee)
  if (invalid_priority_fee : Bool)
  then (fatal_error ExecutionInvalid)
  else (pure ())
  if ((profile.fork <b tx_semantics.minimum_fork) : Bool)
  then (fatal_error ExecutionInvalid)
  else (pure ())
  if ((tx_semantics.set_code && tx.is_create) : Bool)
  then (fatal_error ExecutionInvalid)
  else (pure ())
  let ⟨_, authorizations⟩ := tx.authorizations
  if ((tx_semantics.set_code && (authorizations.count == 0)) : Bool)
  then (fatal_error ExecutionInvalid)
  else (pure ())
  let typed_signature := (tx_semantics.signature == TypedSignature)
  if ((← if (typed_signature : Bool)
       then
         (do
           (pure (tx.chain_id != (← readReg k_chain_id))))
       else (pure false)) : Bool)
  then (fatal_error ExecutionInvalid)
  else (pure ())
  if ((nonce_before == ((2 ^i 64) - 1)) : Bool)
  then (fatal_error ExecutionInvalid)
  else (pure ())
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, initial_gas⟩⟩⟩⟩⟩ ← do
    (transaction_initial_gas allowance costs.intrinsic_execution costs.intrinsic_state
      costs.calldata_floor)
  (pure ((⟨_, ⟨_, ⟨_, ⟨_, ⟨_, (tx_validity_fields sender nonce_before initial_gas
      costs.blob_fee eff_gas_price eff_priority_fee)⟩⟩⟩⟩⟩ : (Sigma fun
    (k_intrinsic_execution : Nat) =>
    (Sigma fun (k_intrinsic_state : Nat) =>
    (Sigma fun (k_calldata_floor : Nat) =>
    (Sigma fun (k_execution : Nat) =>
    (Sigma fun (k_state : Nat) =>
    (TxValidityFields k_total k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))))))) : (Sigma
    fun (k_intrinsic_execution : Nat) =>
    (Sigma fun (k_intrinsic_state : Nat) =>
    (Sigma fun (k_calldata_floor : Nat) =>
    (Sigma fun (k_execution : Nat) =>
    (Sigma fun (k_state : Nat) =>
    (TxValidityFields k_total k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))))))))

/-- The upfront effects, taken before the execution snapshot so they persist
across a dispatched-frame revert: charge the full execution/state gas
limit and the EIP-4844 blob-gas fee, bump the sender nonce, and prewarm the
transaction access set. Before Amsterdam, EIP-7702 authorizations are also
applied here; Amsterdam applies them inside the separately reversible
top-frame preparation phase. -/
/- Type quantifiers: v_dependentWitness6 : Nat, v_dependentWitness5 : Nat, v_dependentWitness4 : Nat, v_dependentWitness3
  : Nat, v_dependentWitness2 : Nat, v_dependentWitness1 : Nat, v_dependentWitness0 : Nat, tx_dependentWitness0
  : Nat, tx_dependentWitness0 = 0 ∨ tx_dependentWitness0 = 6 ∨ tx_dependentWitness0 = 9, 0 ≤
  v_dependentWitness2 ∧
  0 ≤ v_dependentWitness3 ∧
  0 ≤ v_dependentWitness4 ∧
  v_dependentWitness2 ≤ v_dependentWitness1 ∧
  v_dependentWitness4 ≤ v_dependentWitness1 ∧
  v_dependentWitness1 ≤ v_dependentWitness0 ∧
  v_dependentWitness0 ≤ (2 ^ 64 - 1) ∧
  0 ≤ v_dependentWitness5 ∧
  0 ≤ v_dependentWitness6 ∧
  (v_dependentWitness5 + v_dependentWitness6 + v_dependentWitness2 + v_dependentWitness3) =
  v_dependentWitness0 ∧ v_dependentWitness5 ≤ (v_dependentWitness1 - v_dependentWitness2) -/
def apply_transaction_upfront_effects (tx : (Sigma fun (k_blob_limit : Nat) =>
  (TransactionFields k_blob_limit))) (v : (Sigma fun (k_limit : Nat) =>
  (Sigma fun (k_regular : Nat) =>
  (Sigma fun (k_intrinsic_execution : Nat) =>
  (Sigma fun (k_intrinsic_state : Nat) =>
  (Sigma fun (k_calldata_floor : Nat) =>
  (Sigma fun (k_execution : Nat) =>
  (Sigma fun (k_state : Nat) =>
  (TxValidityFields k_limit k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))))))))) (authorizations : PreparedAuthorizationList) : SailM TxUpfrontResult := do
  let tx_dependentWitness0 := (tx).1
  let tx := (tx).2
  let v_dependentWitness0 := (v).1
  let v_dependentWitness1 := ((v).2).1
  let v_dependentWitness2 := (((v).2).2).1
  let v_dependentWitness3 := ((((v).2).2).2).1
  let v_dependentWitness4 := (((((v).2).2).2).2).1
  let v_dependentWitness5 := ((((((v).2).2).2).2).2).1
  let v_dependentWitness6 := (((((((v).2).2).2).2).2).2).1
  let v := (((((((v).2).2).2).2).2).2).2
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let initial_gas := v.gas
  let create_target_prestate_empty ← (( do
    if (((profile.fork ≥b Amsterdam) && tx.is_create) : Bool)
    then
      (do
        let create_target ← do (k_create_addr v.sender v.nonce_before)
        let target_exists ← do (k_account_exists create_target)
        (pure (! target_exists)))
    else (pure false) ) : SailM Bool )
  let gas_debit ← do (validated_word_product v.gas_price initial_gas.admitted_limit)
  (k_sub_balance v.sender gas_debit)
  let has_blob_fee := (word_nonzero v.blob_fee)
  if (has_blob_fee : Bool)
  then (k_sub_balance v.sender v.blob_fee)
  else (pure ())
  (k_bump_nonce v.sender)
  (prewarm ⟨_, tx⟩)
  let authorization_refund ← do
    if ((profile.fork <b Amsterdam) : Bool)
    then (process_auth_list authorizations)
    else (pure 0)
  (pure { authorization_refund := authorization_refund,
          create_target_prestate_empty := create_target_prestate_empty })

/-- Resets the user-space machine for the transaction's top-level frame,
funding it with `gas_limit − intrinsic`. -/
/- Type quantifiers: v_dependentWitness6 : Nat, v_dependentWitness5 : Nat, v_dependentWitness4 : Nat, v_dependentWitness3
  : Nat, v_dependentWitness2 : Nat, v_dependentWitness1 : Nat, v_dependentWitness0 : Nat, 0 ≤
  v_dependentWitness2 ∧
  0 ≤ v_dependentWitness3 ∧
  0 ≤ v_dependentWitness4 ∧
  v_dependentWitness2 ≤ v_dependentWitness1 ∧
  v_dependentWitness4 ≤ v_dependentWitness1 ∧
  v_dependentWitness1 ≤ v_dependentWitness0 ∧
  v_dependentWitness0 ≤ (2 ^ 64 - 1) ∧
  0 ≤ v_dependentWitness5 ∧
  0 ≤ v_dependentWitness6 ∧
  (v_dependentWitness5 + v_dependentWitness6 + v_dependentWitness2 + v_dependentWitness3) =
  v_dependentWitness0 ∧ v_dependentWitness5 ≤ (v_dependentWitness1 - v_dependentWitness2) -/
def enter_transaction_frame (v : (Sigma fun (k_limit : Nat) =>
  (Sigma fun (k_regular : Nat) =>
  (Sigma fun (k_intrinsic_execution : Nat) =>
  (Sigma fun (k_intrinsic_state : Nat) =>
  (Sigma fun (k_calldata_floor : Nat) =>
  (Sigma fun (k_execution : Nat) =>
  (Sigma fun (k_state : Nat) =>
  (TxValidityFields k_limit k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))))))))) : SailM (Nat × Nat × Nat × Int × StackPointer × Nat × Nat) := do
  let v_dependentWitness0 := (v).1
  let v_dependentWitness1 := ((v).2).1
  let v_dependentWitness2 := (((v).2).2).1
  let v_dependentWitness3 := ((((v).2).2).2).1
  let v_dependentWitness4 := (((((v).2).2).2).2).1
  let v_dependentWitness5 := ((((((v).2).2).2).2).2).1
  let v_dependentWitness6 := (((((((v).2).2).2).2).2).2).1
  let v := (((((((v).2).2).2).2).2).2).2
  let initial_gas := v.gas
  let stack ← do (stack_reset ())
  (pure (initial_gas.execution_remaining, initial_gas.state_remaining, STATE_GAS_SPILL_ZERO, GAS_REFUND_ZERO, stack, MEMORY_BASE_ZERO, MEMORY_HEIGHT_ZERO))

def undefined_TransactionPreparation (_ : Unit) : SailM TransactionPreparation := do
  (pure { ready := ← (undefined_bool ()),
          delegated := ← (undefined_bool ()) })

/-- Charges Amsterdam's state-dependent top-level dispatch costs and installs
the code selected for execution. This phase deliberately performs no
revertible account mutation: its state-gas charges are therefore refilled
if the subsequently dispatched frame fails. -/
/- Type quantifiers: k_ex551409_ : Nat, k_ex551408_ : Nat, k_ex551407_ : Nat, v_dependentWitness6 :
  Nat, v_dependentWitness5 : Nat, v_dependentWitness4 : Nat, v_dependentWitness3 : Nat, v_dependentWitness2
  : Nat, v_dependentWitness1 : Nat, v_dependentWitness0 : Nat, tx_dependentWitness0 : Nat, tx_dependentWitness0
  = 0 ∨ tx_dependentWitness0 = 6 ∨ tx_dependentWitness0 = 9, 0 ≤ v_dependentWitness2 ∧
  0 ≤ v_dependentWitness3 ∧
  0 ≤ v_dependentWitness4 ∧
  v_dependentWitness2 ≤ v_dependentWitness1 ∧
  v_dependentWitness4 ≤ v_dependentWitness1 ∧
  v_dependentWitness1 ≤ v_dependentWitness0 ∧
  v_dependentWitness0 ≤ (2 ^ 64 - 1) ∧
  0 ≤ v_dependentWitness5 ∧
  0 ≤ v_dependentWitness6 ∧
  (v_dependentWitness5 + v_dependentWitness6 + v_dependentWitness2 + v_dependentWitness3) =
  v_dependentWitness0 ∧ v_dependentWitness5 ≤ (v_dependentWitness1 - v_dependentWitness2), 0 ≤
  k_ex551407_ ∧ k_ex551407_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551408_ ∧ k_ex551408_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex551409_ ∧ k_ex551409_ ≤ (2 ^ 24) -/
def prepare_amsterdam_transaction_dispatch (tx : (Sigma fun (k_blob_limit : Nat) =>
  (TransactionFields k_blob_limit))) (v : (Sigma fun (k_limit : Nat) =>
  (Sigma fun (k_regular : Nat) =>
  (Sigma fun (k_intrinsic_execution : Nat) =>
  (Sigma fun (k_intrinsic_state : Nat) =>
  (Sigma fun (k_calldata_floor : Nat) =>
  (Sigma fun (k_execution : Nat) =>
  (Sigma fun (k_state : Nat) =>
  (TxValidityFields k_limit k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))))))))) (upfront : TxUpfrontResult) (carried_gas : Nat) (carried_state_gas : Nat) (carried_state_spill : Nat) : SailM (TransactionPreparation × Nat × Nat × Nat × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))) × CalldataSlice) := SailME.run do
  let tx_dependentWitness0 := (tx).1
  let tx := (tx).2
  let v_dependentWitness0 := (v).1
  let v_dependentWitness1 := ((v).2).1
  let v_dependentWitness2 := (((v).2).2).1
  let v_dependentWitness3 := ((((v).2).2).2).1
  let v_dependentWitness4 := (((((v).2).2).2).2).1
  let v_dependentWitness5 := ((((((v).2).2).2).2).2).1
  let v_dependentWitness6 := (((((((v).2).2).2).2).2).2).1
  let v := (((((((v).2).2).2).2).2).2).2
  let gas_after : Nat := carried_gas
  let state_gas_after : Nat := carried_state_gas
  let state_spill_after : Nat := carried_state_spill
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let current_target ← do
    if (tx.is_create : Bool)
    then (k_create_addr v.sender v.nonce_before)
    else (pure tx.recipient)
  if _sailIf0 : (tx.is_create : Bool) = true
  then
    (do
      let (gas_after, state_gas_after, state_spill_after) ← (( do
        if (upfront.create_target_prestate_empty : Bool)
        then
          (do
            let (state_gas_halt, next_gas, next_state_gas, next_state_spill) ← do
              (charge_state_gas gas_after state_gas_after state_spill_after
                G_amsterdam_state_new_account)
            let gas_after : Nat := next_gas
            let state_gas_after : Nat := next_state_gas
            let state_spill_after : Nat := next_state_spill
            if (state_gas_halt : Bool)
            then
              SailME.throw (({ ready := false,
                               delegated := false } : TransactionPreparation), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (current_target : (Vector (BitVec 8) 20)), (current_target : (Vector (BitVec 8) 20)), ((⟨_, ⟨_, ((EMPTY_CODE).2).2⟩⟩ : (Sigma
                fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) : (Sigma
                fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))), (EMPTY_CALLDATA : CalldataSlice))
            else (pure ())
            (pure (gas_after, state_gas_after, state_spill_after)))
        else (pure (gas_after, state_gas_after, state_spill_after)) ) : SailME
        (TransactionPreparation × Nat × Nat × Nat × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Sigma
        fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))) × CalldataSlice)
        (Nat × Nat × Nat) )
      let ⟨_, ⟨_, initcode⟩⟩ ← do (transaction_initcode_slice tx.input_src)
      let code_id ← do (code_db_insert ⟨_, ⟨_, initcode⟩⟩ profile.fork)
      let ⟨_, ⟨_, code⟩⟩ ← do (code_db_resolve code_id)
      (pure (({ ready := true,
                delegated := false } : TransactionPreparation), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (current_target : (Vector (BitVec 8) 20)), (current_target : (Vector (BitVec 8) 20)), ((⟨_, ⟨_, code⟩⟩ : (Sigma
        fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) : (Sigma fun
        (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))), (EMPTY_CALLDATA : CalldataSlice))))
  else
    (do
      let calldata := (InputCalldata tx.input_src)
      let transfers_value := (word_nonzero tx.value)
      let recipient_empty ← do (k_account_is_empty tx.recipient)
      let (gas_after, state_gas_after, state_spill_after) ← (( do
        if ((transfers_value && recipient_empty) : Bool)
        then
          (do
            let (state_gas_halt, next_gas, next_state_gas, next_state_spill) ← do
              (charge_state_gas gas_after state_gas_after state_spill_after
                G_amsterdam_state_new_account)
            let gas_after : Nat := next_gas
            let state_gas_after : Nat := next_state_gas
            let state_spill_after : Nat := next_state_spill
            if (state_gas_halt : Bool)
            then
              SailME.throw (({ ready := false,
                               delegated := false } : TransactionPreparation), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (current_target : (Vector (BitVec 8) 20)), (current_target : (Vector (BitVec 8) 20)), ((⟨_, ⟨_, ((EMPTY_CODE).2).2⟩⟩ : (Sigma
                fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) : (Sigma
                fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))), (calldata : CalldataSlice))
            else (pure ())
            (pure (gas_after, state_gas_after, state_spill_after)))
        else (pure (gas_after, state_gas_after, state_spill_after)) ) : SailME
        (TransactionPreparation × Nat × Nat × Nat × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Sigma
        fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))) × CalldataSlice)
        (Nat × Nat × Nat) )
      let (delegated, delegate) ← do (k_deleg_target tx.recipient)
      let gas_after ← (( do
        if (delegated : Bool)
        then
          (do
            let warm ← do (k_account_is_warm delegate)
            let access_cost ← do (account_cost warm)
            let gas_after ← (( do
              if ((gas_after <b access_cost) : Bool)
              then
                SailME.throw (({ ready := false,
                                 delegated := false } : TransactionPreparation), (GAS_ZERO : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (current_target : (Vector (BitVec 8) 20)), (current_target : (Vector (BitVec 8) 20)), ((⟨_, ⟨_, ((EMPTY_CODE).2).2⟩⟩ : (Sigma
                  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) : (Sigma
                  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))), (calldata : CalldataSlice))
              else
                (do
                  let gas_after : Nat := (gas_sub gas_after access_cost)
                  (k_account_mark_warm delegate)
                  (pure gas_after)) ) : SailME
              (TransactionPreparation × Nat × Nat × Nat × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Sigma
              fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))) × CalldataSlice)
              Nat )
            (pure gas_after))
        else (pure gas_after) ) : SailME
        (TransactionPreparation × Nat × Nat × Nat × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Sigma
        fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))) × CalldataSlice)
        Nat )
      let code_address :=
        if (delegated : Bool)
        then delegate
        else current_target
      let ⟨_, ⟨_, code⟩⟩ ← do (executable_code tx.recipient delegated delegate)
      (pure (({ ready := true,
                delegated := delegated } : TransactionPreparation), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (current_target : (Vector (BitVec 8) 20)), (code_address : (Vector (BitVec 8) 20)), ((⟨_, ⟨_, code⟩⟩ : (Sigma
        fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) : (Sigma fun
        (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))), (calldata : CalldataSlice))))

/-- Runs a create transaction's top-level frame: derives the new address
from `(sender, nonce_before)`, fails outright on an address collision
(all gas consumed, no initcode runs — EIP-684/EIP-7610), and
otherwise deploys via the initcode path. -/
/- Type quantifiers: k_ex551429_ : Nat, carried_code_dependentWitness1 : Nat, carried_code_dependentWitness0
  : Nat, k_ex551424_ : Nat, k_ex551423_ : Nat, k_ex551422_ : Int, k_ex551421_ : Nat, k_ex551420_ :
  Nat, k_ex551419_ : Nat, k_ex551418_ : Nat, tx_dependentWitness0 : Nat, tx_dependentWitness0 = 0
  ∨ tx_dependentWitness0 = 6 ∨ tx_dependentWitness0 = 9, 0 ≤ k_ex551418_ ∧
  k_ex551418_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551419_ ∧ k_ex551419_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex551420_ ∧ k_ex551420_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551421_ ∧ k_ex551421_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1))))
  ≤ k_ex551422_ ∧ k_ex551422_ ≤ (199 * (2 ^ 64 - 1)), 0 ≤ k_ex551423_ ∧
  k_ex551423_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex551424_ ∧ k_ex551424_ ≤ (2 ^ 32 - 1), 0 ≤
  carried_code_dependentWitness0 ∧
  0 ≤ carried_code_dependentWitness1 ∧
  (carried_code_dependentWitness0 + carried_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ carried_code_dependentWitness1 ∧ (carried_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex551429_ ∧ k_ex551429_ ≤ (2 ^ 64 - 1) -/
def run_create_transaction_frame (tx : (Sigma fun (k_blob_limit : Nat) =>
  (TransactionFields k_blob_limit))) (sender : (Vector (BitVec 8) 20)) (nonce_before : Nat) (carried_gas : Nat) (carried_state_gas : Nat) (carried_state_spill : Nat) (carried_refund : Int) (carried_stack : StackPointer) (carried_memory_base : Nat) (carried_memory_height : Nat) (carried_code : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (carried_calldata : CalldataSlice) (state_gas_reservoir : Nat) : SailM (Nat × Nat × Nat × Int × FrameStatus × (Sigma
  fun (carried_code_dependentWitness0 : Nat) =>
  (Sigma fun (carried_code_dependentWitness1 : Nat) =>
  (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) := do
  let tx_dependentWitness0 := (tx).1
  let tx := (tx).2
  let carried_code_dependentWitness0 := (carried_code).1
  let carried_code_dependentWitness1 := ((carried_code).2).1
  let carried_code := ((carried_code).2).2
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let new_addr ← do (k_create_addr sender nonce_before)
  let gas_after : Nat := carried_gas
  let state_gas_after : Nat := carried_state_gas
  let state_spill_after : Nat := carried_state_spill
  let refund_after : Int := carried_refund
  let status_after : FrameStatus := (Running ())
  let output_after : (Sigma fun (carried_code_dependentWitness0 : Nat) =>
    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
    (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))) :=
    ((⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩ : (Sigma fun (carried_code_dependentWitness0 : Nat) =>
    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
    (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) : (Sigma
    fun (carried_code_dependentWitness0 : Nat) =>
    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
    (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))))
  (k_account_mark_warm new_addr)
  let occupied ← do (k_account_occupied new_addr)
  let (gas_after, output_after, refund_after, state_gas_after, state_spill_after, status_after) ← (( do
    if _sailIf0 : (occupied : Bool) = true
    then
      (do
        let gas_after : Nat := GAS_ZERO
        let exceptional ← do
          (exceptional_state state_gas_after state_spill_after state_gas_reservoir AddressCollision)
        let state_gas_after : Nat := exceptional.state_gas_remaining
        let state_spill_after : Nat := exceptional.state_gas_spilled
        let status_after : FrameStatus := exceptional.status
        (pure ((gas_after : Nat), (output_after : (Sigma fun (carried_code_dependentWitness0 : Nat)
          =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))), (refund_after : Int), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus))))
    else
      (do
        (k_mark_created new_addr)
        (k_clear_storage new_addr)
        (k_bump_nonce new_addr)
        let transfers_value := (word_nonzero tx.value)
        if (transfers_value : Bool)
        then (k_transfer sender new_addr tx.value)
        else (pure ())
        let frame_code : (Sigma fun (carried_code_dependentWitness0 : Nat) =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (CodeFields carried_code_dependentWitness0 carried_code_dependentWitness1))) :=
          ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma fun (carried_code_dependentWitness0 : Nat) =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (CodeFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) : (Sigma fun
          (carried_code_dependentWitness0 : Nat) =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (CodeFields carried_code_dependentWitness0 carried_code_dependentWitness1))))
        let frame_calldata : CalldataSlice := carried_calldata
        let (frame_calldata, frame_code) ← (( do
          if _sailIf1 : ((profile.fork <b Amsterdam) : Bool) = true
          then
            (do
              let ⟨_, ⟨_, initcode⟩⟩ ← do (transaction_initcode_slice tx.input_src)
              let code_id ← do (code_db_insert ⟨_, ⟨_, initcode⟩⟩ profile.fork)
              let ⟨_, ⟨_, frame_code⟩⟩ ← (code_db_resolve code_id)
              let frame_calldata : CalldataSlice := EMPTY_CALLDATA
              (pure ((frame_calldata : CalldataSlice), ((⟨_, ⟨_, frame_code⟩⟩ : (Sigma fun
                (carried_code_dependentWitness0 : Nat) =>
                (Sigma fun (carried_code_dependentWitness1 : Nat) =>
                (CodeFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) : (Sigma
                fun (carried_code_dependentWitness0 : Nat) =>
                (Sigma fun (carried_code_dependentWitness1 : Nat) =>
                (CodeFields carried_code_dependentWitness0 carried_code_dependentWitness1)))))))
          else
            (pure ((frame_calldata : CalldataSlice), (frame_code : (Sigma fun
              (carried_code_dependentWitness0 : Nat) =>
              (Sigma fun (carried_code_dependentWitness1 : Nat) =>
              (CodeFields carried_code_dependentWitness0 carried_code_dependentWitness1)))))) ) :
          SailM
          (CalldataSlice × (Sigma fun (carried_code_dependentWitness0 : Nat) =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (CodeFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) )
        let (tup__0, tup__1, tup__2, tup__3, tup__4, tup__5) ← do
          (interpret gas_after state_gas_after state_spill_after refund_after carried_stack
            carried_memory_base carried_memory_height sender new_addr new_addr tx.value
            state_gas_reservoir false 0 frame_code frame_calldata)
        let gas_after : Nat := tup__0
        let state_gas_after : Nat := tup__1
        let state_spill_after : Nat := tup__2
        let refund_after : Int := tup__3
        let status_after : FrameStatus := tup__4
        let output_after : (Sigma fun (carried_code_dependentWitness0 : Nat) =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))) :=
          (tup__5 : (Sigma fun (carried_code_dependentWitness0 : Nat) =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))))
        (pure ())
        let initcode_succeeded := (frame_succeeded status_after)
        let (gas_after, state_gas_after, state_spill_after, status_after) ← (( do
          if (initcode_succeeded : Bool)
          then
            (do
              let ⟨_, ⟨_, deployed_output⟩⟩ := output_after
              let dep_len := deployed_output.len
              let deployed_length := dep_len
              let valid_deployed_size ← do (deployed_code_size_allowed deployed_length)
              let valid_prefix ← do
                if (((profile.fork <b London) || (deployed_length == 0)) : Bool)
                then (pure true)
                else
                  (do
                    let first_byte ← do (output_byte ⟨_, ⟨_, deployed_output⟩⟩ 0)
                    (pure (first_byte != 0xEF#8)))
              let (gas_after, state_gas_after, state_spill_after, status_after) ← (( do
                if ((valid_deployed_size && valid_prefix) : Bool)
                then
                  (do
                    let deployment_charge ← do (code_deployment_execution_cost dep_len gas_after)
                    let (gas_after, state_gas_after, state_spill_after, status_after) ← (( do
                      if (deployment_charge.affordable : Bool)
                      then
                        (do
                          let execution_deposit := deployment_charge.cost
                          let gas_after : Nat := (gas_sub gas_after execution_deposit)
                          let state_deposit ← do (code_deployment_state_cost dep_len)
                          let deployment_halt : Bool := false
                          let (tup__0, tup__1, tup__2, tup__3) ← do
                            (charge_state_gas gas_after state_gas_after state_spill_after
                              state_deposit)
                          let deployment_halt : Bool := tup__0
                          let gas_after : Nat := tup__1
                          let state_gas_after : Nat := tup__2
                          let state_spill_after : Nat := tup__3
                          (pure ())
                          let (gas_after, state_gas_after, state_spill_after, status_after) ← (( do
                            if (deployment_halt : Bool)
                            then
                              (do
                                let gas_after : Nat := GAS_ZERO
                                let exceptional ← do
                                  (exceptional_state state_gas_after state_spill_after
                                    state_gas_reservoir OutOfGas)
                                let state_gas_after : Nat := exceptional.state_gas_remaining
                                let state_spill_after : Nat := exceptional.state_gas_spilled
                                let status_after : FrameStatus := exceptional.status
                                (pure (gas_after, state_gas_after, state_spill_after, status_after)))
                            else
                              (pure (gas_after, state_gas_after, state_spill_after, status_after)) )
                            : SailM (Nat × Nat × Nat × FrameStatus) )
                          let deployment_succeeded := (frame_succeeded status_after)
                          if (deployment_succeeded : Bool)
                          then
                            (do
                              let ⟨_, ⟨_, stored_code⟩⟩ ← do
                                (code_db_intern_output ⟨_, ⟨_, deployed_output⟩⟩)
                              (k_deploy_code new_addr ⟨_, ⟨_, stored_code⟩⟩))
                          else (pure ())
                          (pure (gas_after, state_gas_after, state_spill_after, status_after)))
                      else
                        (do
                          let gas_after : Nat := GAS_ZERO
                          let exceptional ← do
                            (exceptional_state state_gas_after state_spill_after state_gas_reservoir
                              OutOfGas)
                          let state_gas_after : Nat := exceptional.state_gas_remaining
                          let state_spill_after : Nat := exceptional.state_gas_spilled
                          let status_after : FrameStatus := exceptional.status
                          (pure (gas_after, state_gas_after, state_spill_after, status_after))) ) :
                      SailM (Nat × Nat × Nat × FrameStatus) )
                    (pure (gas_after, state_gas_after, state_spill_after, status_after)))
                else
                  (do
                    let gas_after : Nat := GAS_ZERO
                    let exceptional ← do
                      (exceptional_state state_gas_after state_spill_after state_gas_reservoir
                        OutOfGas)
                    let state_gas_after : Nat := exceptional.state_gas_remaining
                    let state_spill_after : Nat := exceptional.state_gas_spilled
                    let status_after : FrameStatus := exceptional.status
                    (pure (gas_after, state_gas_after, state_spill_after, status_after))) ) : SailM
                (Nat × Nat × Nat × FrameStatus) )
              (pure (gas_after, state_gas_after, state_spill_after, status_after)))
          else (pure (gas_after, state_gas_after, state_spill_after, status_after)) ) : SailM
          (Nat × Nat × Nat × FrameStatus) )
        (pure ((gas_after : Nat), (output_after : (Sigma fun (carried_code_dependentWitness0 : Nat)
          =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))), (refund_after : Int), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus))))
    ) : SailM
    (Nat × (Sigma fun (carried_code_dependentWitness0 : Nat) =>
    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
    (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))) × Int × Nat × Nat × FrameStatus)
    )
  (pure ((gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (refund_after : Int), (status_after : FrameStatus), (output_after : (Sigma
    fun (carried_code_dependentWitness0 : Nat) =>
    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
    (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))))))

/-- Runs a call transaction's top-level frame: transfers value, then either
runs a direct recipient precompile or interprets the selected code. At
Amsterdam the preparation phase has already resolved and charged a
recipient delegation; a delegated recipient never dispatches a precompile
directly. -/
/- Type quantifiers: k_ex551449_ : Nat, carried_code_dependentWitness1 : Nat, carried_code_dependentWitness0
  : Nat, k_ex551444_ : Nat, k_ex551443_ : Nat, k_ex551442_ : Int, k_ex551441_ : Nat, k_ex551440_ :
  Nat, k_ex551439_ : Nat, k_ex551438_ : Bool, tx_dependentWitness0 : Nat, tx_dependentWitness0 = 0
  ∨ tx_dependentWitness0 = 6 ∨ tx_dependentWitness0 = 9, 0 ≤ k_ex551439_ ∧
  k_ex551439_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551440_ ∧ k_ex551440_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex551441_ ∧ k_ex551441_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1)))) ≤ k_ex551442_ ∧
  k_ex551442_ ≤ (199 * (2 ^ 64 - 1)), 0 ≤ k_ex551443_ ∧ k_ex551443_ ≤ (2 ^ 32 - 1), 0 ≤
  k_ex551444_ ∧ k_ex551444_ ≤ (2 ^ 32 - 1), 0 ≤ carried_code_dependentWitness0 ∧
  0 ≤ carried_code_dependentWitness1 ∧
  (carried_code_dependentWitness0 + carried_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ carried_code_dependentWitness1 ∧ (carried_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex551449_ ∧ k_ex551449_ ≤ (2 ^ 64 - 1) -/
def run_call_transaction_frame (tx : (Sigma fun (k_blob_limit : Nat) =>
  (TransactionFields k_blob_limit))) (sender : (Vector (BitVec 8) 20)) (delegated : Bool) (carried_gas : Nat) (carried_state_gas : Nat) (carried_state_spill : Nat) (carried_refund : Int) (carried_stack : StackPointer) (carried_memory_base : Nat) (carried_memory_height : Nat) (carried_code_address : (Vector (BitVec 8) 20)) (carried_code : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (carried_calldata : CalldataSlice) (state_gas_reservoir : Nat) : SailM (Nat × Nat × Nat × Int × FrameStatus × (Sigma
  fun (carried_code_dependentWitness0 : Nat) =>
  (Sigma fun (carried_code_dependentWitness1 : Nat) =>
  (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) := do
  let tx_dependentWitness0 := (tx).1
  let tx := (tx).2
  let carried_code_dependentWitness0 := (carried_code).1
  let carried_code_dependentWitness1 := ((carried_code).2).1
  let carried_code := ((carried_code).2).2
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let gas_after : Nat := carried_gas
  let state_gas_after : Nat := carried_state_gas
  let state_spill_after : Nat := carried_state_spill
  let refund_after : Int := carried_refund
  let status_after : FrameStatus := (Running ())
  let output_after : (Sigma fun (carried_code_dependentWitness0 : Nat) =>
    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
    (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))) :=
    ((⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩ : (Sigma fun (carried_code_dependentWitness0 : Nat) =>
    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
    (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) : (Sigma
    fun (carried_code_dependentWitness0 : Nat) =>
    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
    (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))))
  let code_address : (Vector (BitVec 8) 20) := carried_code_address
  let frame_code : (Sigma fun (carried_code_dependentWitness0 : Nat) =>
    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
    (CodeFields carried_code_dependentWitness0 carried_code_dependentWitness1))) :=
    ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma fun (carried_code_dependentWitness0 : Nat) =>
    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
    (CodeFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) : (Sigma fun
    (carried_code_dependentWitness0 : Nat) =>
    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
    (CodeFields carried_code_dependentWitness0 carried_code_dependentWitness1))))
  let frame_calldata : CalldataSlice := carried_calldata
  let _ ← do (k_aload tx.recipient)
  let transfers_value := (word_nonzero tx.value)
  if (transfers_value : Bool)
  then (k_transfer sender tx.recipient tx.value)
  else (pure ())
  let selected_precompile ← do (precompile_id_for_address tx.recipient)
  let direct_precompile : Bool := false
  let direct_precompile : Bool :=
    if ((! delegated) : Bool)
    then (bne selected_precompile NotPrecompile)
    else direct_precompile
  let (gas_after, output_after, refund_after, state_gas_after, state_spill_after, status_after) ← (( do
    if _sailIf0 : (direct_precompile : Bool) = true
    then
      (do
        let ⟨_, ⟨_, input_src⟩⟩ : (Sigma fun (carried_code_dependentWitness0 : Nat) =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (StatelessInputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))) :=
          ((⟨_, ⟨_, ((tx.input_src).2).2⟩⟩ : (Sigma fun
          (carried_code_dependentWitness0 : Nat) =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (StatelessInputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) : (Sigma
          fun (carried_code_dependentWitness0 : Nat) =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (StatelessInputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))))
        let precompile_input := (InputCalldata ⟨_, ⟨_, input_src⟩⟩)
        let precompile_charge ← do (precompile_gas selected_precompile precompile_input gas_after)
        let (gas_after, output_after, state_gas_after, state_spill_after, status_after) ← (( do
          if _sailIf1 : (precompile_charge.affordable : Bool) = true
          then
            (do
              let used := precompile_charge.cost
              let result ← do (run_precompile_slice selected_precompile precompile_input)
              let (gas_after, output_after, state_gas_after, state_spill_after, status_after) ← (( do
                if _sailIf2 : (result.success : Bool) = true
                then
                  (let gas_after : Nat := (gas_sub gas_after used)
                  let ⟨_, ⟨_, output_after⟩⟩ : (Sigma fun
                    (carried_code_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
                    (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))) :=
                    (result.output : (Sigma fun (carried_code_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
                    (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))))
                  let halt_reason := (HaltReturn result.output)
                  let status_after : FrameStatus := (Halted halt_reason)
                  (pure ((gas_after : Nat), ((⟨_, ⟨_, output_after⟩⟩ : (Sigma fun
                    (carried_code_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
                    (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) : (Sigma
                    fun (carried_code_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
                    (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus))))
                else
                  (do
                    let gas_after : Nat := GAS_ZERO
                    let exceptional ← do
                      (exceptional_state state_gas_after state_spill_after state_gas_reservoir
                        OutOfGas)
                    let state_gas_after : Nat := exceptional.state_gas_remaining
                    let state_spill_after : Nat := exceptional.state_gas_spilled
                    let status_after : FrameStatus := exceptional.status
                    (pure ((gas_after : Nat), (output_after : (Sigma fun
                      (carried_code_dependentWitness0 : Nat) =>
                      (Sigma fun (carried_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus))))
                ) : SailM
                (Nat × (Sigma fun (carried_code_dependentWitness0 : Nat) =>
                (Sigma fun (carried_code_dependentWitness1 : Nat) =>
                (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))) × Nat × Nat × FrameStatus)
                )
              (pure ((gas_after : Nat), (output_after : (Sigma fun
                (carried_code_dependentWitness0 : Nat) =>
                (Sigma fun (carried_code_dependentWitness1 : Nat) =>
                (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus))))
          else
            (do
              let gas_after : Nat := GAS_ZERO
              let exceptional ← do
                (exceptional_state state_gas_after state_spill_after state_gas_reservoir OutOfGas)
              let state_gas_after : Nat := exceptional.state_gas_remaining
              let state_spill_after : Nat := exceptional.state_gas_spilled
              let status_after : FrameStatus := exceptional.status
              (pure ((gas_after : Nat), (output_after : (Sigma fun
                (carried_code_dependentWitness0 : Nat) =>
                (Sigma fun (carried_code_dependentWitness1 : Nat) =>
                (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus))))
          ) : SailM
          (Nat × (Sigma fun (carried_code_dependentWitness0 : Nat) =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))) × Nat × Nat × FrameStatus)
          )
        (pure ((gas_after : Nat), (output_after : (Sigma fun (carried_code_dependentWitness0 : Nat)
          =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))), (refund_after : Int), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus))))
    else
      (do
        let (code_address, frame_calldata, frame_code) ← (( do
          if _sailIf1 : ((profile.fork <b Amsterdam) : Bool) = true
          then
            (do
              let frame_calldata : CalldataSlice := (InputCalldata tx.input_src)
              let code_address : (Vector (BitVec 8) 20) := tx.recipient
              let (tx_deleg, tx_dtgt) ← do (k_deleg_target tx.recipient)
              if (tx_deleg : Bool)
              then
                (do
                  (k_account_mark_warm tx_dtgt)
                  let _ ← do (k_aload tx_dtgt)
                  (pure ()))
              else (pure ())
              let code_address : (Vector (BitVec 8) 20) :=
                if (tx_deleg : Bool)
                then tx_dtgt
                else code_address
              let ⟨_, ⟨_, frame_code⟩⟩ ← (executable_code tx.recipient tx_deleg tx_dtgt)
              (pure ((code_address : (Vector (BitVec 8) 20)), (frame_calldata : CalldataSlice), ((⟨_, ⟨_, frame_code⟩⟩ : (Sigma
                fun (carried_code_dependentWitness0 : Nat) =>
                (Sigma fun (carried_code_dependentWitness1 : Nat) =>
                (CodeFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) : (Sigma
                fun (carried_code_dependentWitness0 : Nat) =>
                (Sigma fun (carried_code_dependentWitness1 : Nat) =>
                (CodeFields carried_code_dependentWitness0 carried_code_dependentWitness1)))))))
          else
            (pure ((code_address : (Vector (BitVec 8) 20)), (frame_calldata : CalldataSlice), (frame_code : (Sigma
              fun (carried_code_dependentWitness0 : Nat) =>
              (Sigma fun (carried_code_dependentWitness1 : Nat) =>
              (CodeFields carried_code_dependentWitness0 carried_code_dependentWitness1)))))) ) :
          SailM
          ((Vector (BitVec 8) 20) × CalldataSlice × (Sigma fun
          (carried_code_dependentWitness0 : Nat) =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (CodeFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) )
        let (tup__0, tup__1, tup__2, tup__3, tup__4, tup__5) ← do
          (interpret gas_after state_gas_after state_spill_after refund_after carried_stack
            carried_memory_base carried_memory_height sender tx.recipient code_address tx.value
            state_gas_reservoir false 0 frame_code frame_calldata)
        let gas_after : Nat := tup__0
        let state_gas_after : Nat := tup__1
        let state_spill_after : Nat := tup__2
        let refund_after : Int := tup__3
        let status_after : FrameStatus := tup__4
        let ⟨_, ⟨_, output_after⟩⟩ : (Sigma fun (carried_code_dependentWitness0 : Nat) =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))) :=
          (tup__5 : (Sigma fun (carried_code_dependentWitness0 : Nat) =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))))
        (pure ((gas_after : Nat), ((⟨_, ⟨_, output_after⟩⟩ : (Sigma fun
          (carried_code_dependentWitness0 : Nat) =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) : (Sigma
          fun (carried_code_dependentWitness0 : Nat) =>
          (Sigma fun (carried_code_dependentWitness1 : Nat) =>
          (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))), (refund_after : Int), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus))))
    ) : SailM
    (Nat × (Sigma fun (carried_code_dependentWitness0 : Nat) =>
    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
    (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))) × Int × Nat × Nat × FrameStatus)
    )
  (pure ((gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (refund_after : Int), (status_after : FrameStatus), (output_after : (Sigma
    fun (carried_code_dependentWitness0 : Nat) =>
    (Sigma fun (carried_code_dependentWitness1 : Nat) =>
    (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1))))))

/- Type quantifiers: v_dependentWitness4 : Nat, v_dependentWitness3 : Nat, v_dependentWitness2 : Nat, v_dependentWitness1
  : Nat, v_dependentWitness0 : Nat, tx_dependentWitness0 : Nat, k_limit : Nat, k_regular : Nat, 0
  ≤ k_regular ∧ k_regular ≤ k_limit ∧ k_limit ≤ block_gas_limit_bound, tx_dependentWitness0
  = 0 ∨ tx_dependentWitness0 = 6 ∨ tx_dependentWitness0 = 9, 0 ≤ v_dependentWitness0 ∧
  0 ≤ v_dependentWitness1 ∧
  0 ≤ v_dependentWitness2 ∧
  v_dependentWitness0 ≤ k_regular ∧
  v_dependentWitness2 ≤ k_regular ∧
  k_regular ≤ k_limit ∧
  k_limit ≤ (2 ^ 64 - 1) ∧
  0 ≤ v_dependentWitness3 ∧
  0 ≤ v_dependentWitness4 ∧
  (v_dependentWitness3 + v_dependentWitness4 + v_dependentWitness0 + v_dependentWitness1) = k_limit
  ∧ v_dependentWitness3 ≤ (k_regular - v_dependentWitness0) -/
def run_legacy_transaction_frame (tx : (Sigma fun (k_blob_limit : Nat) =>
  (TransactionFields k_blob_limit))) (v : (Sigma fun (k_intrinsic_execution : Nat) =>
  (Sigma fun (k_intrinsic_state : Nat) =>
  (Sigma fun (k_calldata_floor : Nat) =>
  (Sigma fun (k_execution : Nat) =>
  (Sigma fun (k_state : Nat) =>
  (TxValidityFields k_limit k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))))))) : SailM (TxFrameResultFields k_limit k_regular) := do
  let tx_dependentWitness0 := (tx).1
  let tx := (tx).2
  let v_dependentWitness0 := (v).1
  let v_dependentWitness1 := ((v).2).1
  let v_dependentWitness2 := (((v).2).2).1
  let v_dependentWitness3 := ((((v).2).2).2).1
  let v_dependentWitness4 := (((((v).2).2).2).2).1
  let v := (((((v).2).2).2).2).2
  let initial_gas := v.gas
  (k_journal_checkpoint ())
  let (initial_execution_gas, initial_state_gas, initial_state_spill, initial_refund, initial_stack, initial_memory_base, initial_memory_height) ← do
    (enter_transaction_frame ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, v⟩⟩⟩⟩⟩⟩⟩)
  let state_gas_reservoir := initial_state_gas
  let (gas_after, state_gas_after, state_spill_after, refund_after, status_after, _) ← do
    if _sailIf0 : (tx.is_create : Bool) = true
    then
      (do
        (run_create_transaction_frame ⟨_, tx⟩ v.sender v.nonce_before initial_execution_gas
          initial_state_gas initial_state_spill initial_refund initial_stack initial_memory_base
          initial_memory_height ⟨_, ⟨_, ((EMPTY_CODE).2).2⟩⟩ EMPTY_CALLDATA
          state_gas_reservoir))
    else
      (do
        (run_call_transaction_frame ⟨_, tx⟩ v.sender false initial_execution_gas
          initial_state_gas initial_state_spill initial_refund initial_stack initial_memory_base
          initial_memory_height tx.recipient ⟨_, ⟨_, ((EMPTY_CODE).2).2⟩⟩ EMPTY_CALLDATA
          state_gas_reservoir))
  let success := (frame_succeeded status_after)
  let failed := (! success)
  if (failed : Bool)
  then (k_journal_revert ())
  else (k_journal_commit ())
  let state_delta := (frame_state_gas_used state_gas_reservoir state_gas_after state_spill_after)
  let retained_refund :=
    if (success : Bool)
    then refund_after
    else GAS_REFUND_ZERO
  (pure { success := success,
          gas := ← do
              let publicField ← (tx_frame_gas_snapshot initial_gas gas_after state_gas_after
                state_delta)
              pure (⟨_, ⟨_, ⟨_, (((publicField).2).2).2⟩⟩⟩),
          refund := retained_refund })

/- Type quantifiers: v_dependentWitness4 : Nat, v_dependentWitness3 : Nat, v_dependentWitness2 : Nat, v_dependentWitness1
  : Nat, v_dependentWitness0 : Nat, tx_dependentWitness0 : Nat, k_limit : Nat, k_regular : Nat, 0
  ≤ k_regular ∧ k_regular ≤ k_limit ∧ k_limit ≤ block_gas_limit_bound, tx_dependentWitness0
  = 0 ∨ tx_dependentWitness0 = 6 ∨ tx_dependentWitness0 = 9, 0 ≤ v_dependentWitness0 ∧
  0 ≤ v_dependentWitness1 ∧
  0 ≤ v_dependentWitness2 ∧
  v_dependentWitness0 ≤ k_regular ∧
  v_dependentWitness2 ≤ k_regular ∧
  k_regular ≤ k_limit ∧
  k_limit ≤ (2 ^ 64 - 1) ∧
  0 ≤ v_dependentWitness3 ∧
  0 ≤ v_dependentWitness4 ∧
  (v_dependentWitness3 + v_dependentWitness4 + v_dependentWitness0 + v_dependentWitness1) = k_limit
  ∧ v_dependentWitness3 ≤ (k_regular - v_dependentWitness0) -/
def run_amsterdam_transaction_frame (tx : (Sigma fun (k_blob_limit : Nat) =>
  (TransactionFields k_blob_limit))) (v : (Sigma fun (k_intrinsic_execution : Nat) =>
  (Sigma fun (k_intrinsic_state : Nat) =>
  (Sigma fun (k_calldata_floor : Nat) =>
  (Sigma fun (k_execution : Nat) =>
  (Sigma fun (k_state : Nat) =>
  (TxValidityFields k_limit k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))))))) (upfront : TxUpfrontResult) (authorizations : PreparedAuthorizationList) : SailM (TxFrameResultFields k_limit k_regular) := do
  let tx_dependentWitness0 := (tx).1
  let tx := (tx).2
  let v_dependentWitness0 := (v).1
  let v_dependentWitness1 := ((v).2).1
  let v_dependentWitness2 := (((v).2).2).1
  let v_dependentWitness3 := ((((v).2).2).2).1
  let v_dependentWitness4 := (((((v).2).2).2).2).1
  let v := (((((v).2).2).2).2).2
  let (entered_gas, entered_state_gas, entered_state_spill, entered_refund, entered_stack, entered_memory_base, entered_memory) ← do
    (enter_transaction_frame ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, v⟩⟩⟩⟩⟩⟩⟩)
  let gas_after : Nat := entered_gas
  let state_gas_after : Nat := entered_state_gas
  let state_spill_after : Nat := entered_state_spill
  let refund_after : Int := entered_refund
  let status_after : FrameStatus := (Running ())
  let output_after : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))) :=
    ((⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))))
  let initial_gas := v.gas
  (k_journal_checkpoint ())
  let preparation_reservoir := state_gas_after
  let current_target ← do
    if (tx.is_create : Bool)
    then (k_create_addr v.sender v.nonce_before)
    else (pure tx.recipient)
  (authorization_tracker_reset authorizations.count)
  let transfers_value := (word_nonzero tx.value)
  let preparation_ready : Bool := false
  let (tup__0, tup__1, tup__2, tup__3) ← do
    (process_amsterdam_auth_cursor authorizations authorizations.count v.sender current_target
      transfers_value gas_after state_gas_after state_spill_after)
  let preparation_ready : Bool := tup__0
  let gas_after : Nat := tup__1
  let state_gas_after : Nat := tup__2
  let state_spill_after : Nat := tup__3
  (pure ())
  let authorization_state_gas : Int := FRAME_STATE_GAS_DELTA_ZERO
  let delegated : Bool := false
  let execution_reservoir : Nat := state_gas_after
  let prepared_code_address : (Vector (BitVec 8) 20) := current_target
  let prepared_code : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))) :=
    ((⟨_, ⟨_, ((EMPTY_CODE).2).2⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))))
  let prepared_calldata : CalldataSlice := EMPTY_CALLDATA
  let (authorization_state_gas, delegated, execution_reservoir, gas_after, preparation_ready, prepared_calldata, prepared_code, prepared_code_address, state_gas_after, state_spill_after) ← (( do
    if _sailIf0 : (preparation_ready : Bool) = true
    then
      (do
        let authorization_state_gas : Int :=
          (frame_state_gas_used preparation_reservoir state_gas_after state_spill_after)
        let execution_reservoir : Nat := state_gas_after
        let state_spill_after : Nat := STATE_GAS_SPILL_ZERO
        let (preparation, prepared_gas, prepared_state_gas, prepared_state_spill, _, code_address, code, calldata) ← do
          (prepare_amsterdam_transaction_dispatch ⟨_, tx⟩
            ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, v⟩⟩⟩⟩⟩⟩⟩ upfront gas_after
            state_gas_after state_spill_after)
        let gas_after : Nat := prepared_gas
        let state_gas_after : Nat := prepared_state_gas
        let state_spill_after : Nat := prepared_state_spill
        let preparation_ready : Bool := preparation.ready
        let delegated : Bool := preparation.delegated
        let prepared_code_address : (Vector (BitVec 8) 20) := code_address
        let ⟨_, ⟨_, prepared_code⟩⟩ : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))) :=
          (code : (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))))
        let prepared_calldata : CalldataSlice := calldata
        (pure ((authorization_state_gas : Int), (delegated : Bool), (execution_reservoir : Nat), (gas_after : Nat), (preparation_ready : Bool), (prepared_calldata : CalldataSlice), ((⟨_, ⟨_, prepared_code⟩⟩ : (Sigma
          fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) : (Sigma fun
          (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))), (prepared_code_address : (Vector (BitVec 8) 20)), (state_gas_after : Nat), (state_spill_after : Nat))))
    else
      (pure ((authorization_state_gas : Int), (delegated : Bool), (execution_reservoir : Nat), (gas_after : Nat), (preparation_ready : Bool), (prepared_calldata : CalldataSlice), (prepared_code : (Sigma
        fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))), (prepared_code_address : (Vector (BitVec 8) 20)), (state_gas_after : Nat), (state_spill_after : Nat)))
    ) : SailM
    (Int × Bool × Nat × Nat × Bool × CalldataSlice × (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))) × (Vector (BitVec 8) 20) × Nat × Nat)
    )
  let preparation_failed := (! preparation_ready)
  if (preparation_failed : Bool)
  then
    (do
      (k_journal_revert ())
      let state_gas_after : Nat := preparation_reservoir
      let state_spill_after : Nat := STATE_GAS_SPILL_ZERO
      (pure { success := false,
              gas := ← do
                  let publicField ← (tx_frame_gas_snapshot initial_gas GAS_ZERO STATE_GAS_ZERO
                    FRAME_STATE_GAS_DELTA_ZERO)
                  pure (⟨_, ⟨_, ⟨_, (((publicField).2).2).2⟩⟩⟩),
              refund := GAS_REFUND_ZERO }))
  else
    (do
      (k_journal_checkpoint ())
      let (gas_after, refund_after, state_gas_after, state_spill_after, status_after) ← (( do
        if (tx.is_create : Bool)
        then
          (do
            let (tup__0, tup__1, tup__2, tup__3, tup__4, tup__5) ← do
              (run_create_transaction_frame ⟨_, tx⟩ v.sender v.nonce_before gas_after
                state_gas_after state_spill_after refund_after entered_stack entered_memory_base
                entered_memory prepared_code prepared_calldata execution_reservoir)
            let gas_after : Nat := tup__0
            let state_gas_after : Nat := tup__1
            let state_spill_after : Nat := tup__2
            let refund_after : Int := tup__3
            let status_after : FrameStatus := tup__4
            let ⟨_, ⟨_, output_after⟩⟩ : (Sigma fun (k_off : Nat) =>
              (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))) :=
              (tup__5 : (Sigma fun (k_off : Nat) =>
              (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))))
            (pure (gas_after, refund_after, state_gas_after, state_spill_after, status_after)))
        else
          (do
            let (tup__0, tup__1, tup__2, tup__3, tup__4, tup__5) ← do
              (run_call_transaction_frame ⟨_, tx⟩ v.sender delegated gas_after state_gas_after
                state_spill_after refund_after entered_stack entered_memory_base entered_memory
                prepared_code_address prepared_code prepared_calldata execution_reservoir)
            let gas_after : Nat := tup__0
            let state_gas_after : Nat := tup__1
            let state_spill_after : Nat := tup__2
            let refund_after : Int := tup__3
            let status_after : FrameStatus := tup__4
            let ⟨_, ⟨_, output_after⟩⟩ : (Sigma fun (k_off : Nat) =>
              (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))) :=
              (tup__5 : (Sigma fun (k_off : Nat) =>
              (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))))
            (pure (gas_after, refund_after, state_gas_after, state_spill_after, status_after))) ) :
        SailM (Nat × Int × Nat × Nat × FrameStatus) )
      let success := (frame_succeeded status_after)
      let failed := (! success)
      if (failed : Bool)
      then (k_journal_revert ())
      else (k_journal_commit ())
      (k_journal_commit ())
      let execution_state_delta :=
        (frame_state_gas_used execution_reservoir state_gas_after state_spill_after)
      let state_delta := (authorization_state_gas +i execution_state_delta)
      let retained_refund :=
        if (success : Bool)
        then refund_after
        else GAS_REFUND_ZERO
      (pure { success := success,
              gas := ← do
                  let publicField ← (tx_frame_gas_snapshot initial_gas gas_after state_gas_after
                    state_delta)
                  pure (⟨_, ⟨_, ⟨_, (((publicField).2).2).2⟩⟩⟩),
              refund := retained_refund }))

/- Type quantifiers: v_dependentWitness4 : Nat, v_dependentWitness3 : Nat, v_dependentWitness2 : Nat, v_dependentWitness1
  : Nat, v_dependentWitness0 : Nat, tx_dependentWitness0 : Nat, k_limit : Nat, k_regular : Nat, 0
  ≤ k_regular ∧ k_regular ≤ k_limit ∧ k_limit ≤ block_gas_limit_bound, tx_dependentWitness0
  = 0 ∨ tx_dependentWitness0 = 6 ∨ tx_dependentWitness0 = 9, 0 ≤ v_dependentWitness0 ∧
  0 ≤ v_dependentWitness1 ∧
  0 ≤ v_dependentWitness2 ∧
  v_dependentWitness0 ≤ k_regular ∧
  v_dependentWitness2 ≤ k_regular ∧
  k_regular ≤ k_limit ∧
  k_limit ≤ (2 ^ 64 - 1) ∧
  0 ≤ v_dependentWitness3 ∧
  0 ≤ v_dependentWitness4 ∧
  (v_dependentWitness3 + v_dependentWitness4 + v_dependentWitness0 + v_dependentWitness1) = k_limit
  ∧ v_dependentWitness3 ≤ (k_regular - v_dependentWitness0) -/
def run_transaction_frame (tx : (Sigma fun (k_blob_limit : Nat) => (TransactionFields k_blob_limit))) (v : (Sigma
  fun (k_intrinsic_execution : Nat) =>
  (Sigma fun (k_intrinsic_state : Nat) =>
  (Sigma fun (k_calldata_floor : Nat) =>
  (Sigma fun (k_execution : Nat) =>
  (Sigma fun (k_state : Nat) =>
  (TxValidityFields k_limit k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))))))) (upfront : TxUpfrontResult) (authorizations : PreparedAuthorizationList) : SailM (TxFrameResultFields k_limit k_regular) := do
  let tx_dependentWitness0 := (tx).1
  let tx := (tx).2
  let v_dependentWitness0 := (v).1
  let v_dependentWitness1 := ((v).2).1
  let v_dependentWitness2 := (((v).2).2).1
  let v_dependentWitness3 := ((((v).2).2).2).1
  let v_dependentWitness4 := (((((v).2).2).2).2).1
  let v := (((((v).2).2).2).2).2
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if ((profile.fork ≥b Amsterdam) : Bool)
  then
    (run_amsterdam_transaction_frame ⟨_, tx⟩ ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, v⟩⟩⟩⟩⟩
      upfront authorizations)
  else (run_legacy_transaction_frame ⟨_, tx⟩ ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, v⟩⟩⟩⟩⟩)

/- Type quantifiers: _limit : Nat, total : Int, remaining : Nat, cap : Nat, ((- gas_refund_bound))
  ≤ total ∧
  total ≤ (gas_refund_bound + authorization_refund_per_item * transaction_length_bound) ∧
  0 ≤ remaining ∧
  remaining ≤ _limit ∧
  _limit ≤ block_gas_limit_bound ∧ 0 ≤ cap ∧ cap ≤ (_limit - remaining) -/
def remaining_gas_after_refund (_limit : Nat) (total : Int) (remaining : Nat) (cap : Nat) : Nat :=
  let refund :=
    if ((total ≤b 0) : Bool)
    then 0
    else
      (if ((total ≤b cap) : Bool)
      then total
      else cap)
  (remaining + refund)

/- Type quantifiers: authorization_refund : Nat, v_dependentWitness4 : Nat, v_dependentWitness3 :
  Nat, v_dependentWitness2 : Nat, v_dependentWitness1 : Nat, v_dependentWitness0 : Nat, tx_dependentWitness0
  : Nat, k_limit : Nat, k_regular : Nat, 0 ≤ k_regular ∧
  k_regular ≤ k_limit ∧ k_limit ≤ block_gas_limit_bound, tx_dependentWitness0 = 0 ∨
  tx_dependentWitness0 = 6 ∨ tx_dependentWitness0 = 9, 0 ≤ v_dependentWitness0 ∧
  0 ≤ v_dependentWitness1 ∧
  0 ≤ v_dependentWitness2 ∧
  v_dependentWitness0 ≤ k_regular ∧
  v_dependentWitness2 ≤ k_regular ∧
  k_regular ≤ k_limit ∧
  k_limit ≤ (2 ^ 64 - 1) ∧
  0 ≤ v_dependentWitness3 ∧
  0 ≤ v_dependentWitness4 ∧
  (v_dependentWitness3 + v_dependentWitness4 + v_dependentWitness0 + v_dependentWitness1) = k_limit
  ∧ v_dependentWitness3 ≤ (k_regular - v_dependentWitness0), 0 ≤ authorization_refund ∧
  authorization_refund ≤ (12500 * 2 ^ 30) -/
def settle_transaction (tx : (Sigma fun (k_blob_limit : Nat) => (TransactionFields k_blob_limit))) (v : (Sigma
  fun (k_intrinsic_execution : Nat) =>
  (Sigma fun (k_intrinsic_state : Nat) =>
  (Sigma fun (k_calldata_floor : Nat) =>
  (Sigma fun (k_execution : Nat) =>
  (Sigma fun (k_state : Nat) =>
  (TxValidityFields k_limit k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))))))) (authorization_refund : Nat) (fr : (TxFrameResultFields k_limit k_regular)) : SailM (Sigma
  fun (k_state_gas : Nat) =>
  (Sigma fun (k_execution_gas : Nat) =>
  (Sigma fun (k_gas_used : Nat) =>
  (ReceiptFields k_limit k_regular k_gas_used k_execution_gas k_state_gas)))) := do
  let tx_dependentWitness0 := (tx).1
  let tx := (tx).2
  let v_dependentWitness0 := (v).1
  let v_dependentWitness1 := ((v).2).1
  let v_dependentWitness2 := (((v).2).2).1
  let v_dependentWitness3 := ((((v).2).2).2).1
  let v_dependentWitness4 := (((((v).2).2).2).2).1
  let v := (((((v).2).2).2).2).2
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let ⟨_, ⟨_, ⟨_, gas_snapshot⟩⟩⟩ := fr.gas
  let gas_limit := gas_snapshot.admitted_limit
  let gas_left := gas_snapshot.remaining
  let gas_used0 := (gas_limit - gas_left)
  let refund_quotient := profile.refund_divisor
  let refund_cap := (gas_used0 / refund_quotient)
  let total_refund := (authorization_refund +i fr.refund)
  let gas_left := (remaining_gas_after_refund gas_limit total_refund gas_left refund_cap)
  let gas_used1 := (gas_limit - gas_left)
  let floor :=
    if ((profile.fork ≥b Prague) : Bool)
    then gas_snapshot.calldata_floor
    else 0
  let gas_used : Nat :=
    if ((gas_used1 <b floor) : Bool)
    then floor
    else gas_used1
  let gas_left := (gas_limit - gas_used)
  let tx_state_gas := gas_snapshot.state_used
  let unrefunded_execution_gas : Nat := ((gas_limit - gas_snapshot.remaining) - tx_state_gas)
  let execution_gas : Nat :=
    if ((unrefunded_execution_gas <b floor) : Bool)
    then floor
    else unrefunded_execution_gas
  let state_gas : Nat := tx_state_gas
  let sender_refund ← do (validated_word_product v.gas_price gas_left)
  (k_add_balance v.sender sender_refund)
  let coinbase ← do (k_coinbase ())
  let priority_payment ← do (validated_word_product v.priority_fee gas_used)
  (k_add_balance coinbase priority_payment)
  (k_tx_merge ())
  let logs ← do (read_logs ())
  let gas_used_value := gas_used
  let execution_gas_value := execution_gas
  let state_gas_value := state_gas
  if _sailIf0 : ((gas_used_value ≤b (execution_gas_value + state_gas_value)) : Bool) = true
  then
    (pure ((receipt_within gas_limit gas_snapshot.regular_limit tx.tx_type fr.success gas_used_value
        execution_gas_value state_gas_value logs) : (Sigma fun (k_syn_state_gas : Nat) =>
      (Sigma fun (k_syn_execution_gas : Nat) =>
      (Sigma fun (k_syn_gas_used : Nat) =>
      (ReceiptFields k_limit k_regular k_syn_gas_used k_syn_execution_gas k_syn_state_gas))))))
  else
    (do
      (fatal_error ExecutionInvalid))

/- Type quantifiers: tx_dependentWitness0 : Nat, k_total : Nat, k_regular : Nat, 0 ≤ k_regular ∧
  k_regular ≤ k_total ∧ k_total ≤ block_gas_limit_bound, tx_dependentWitness0 = 0 ∨
  tx_dependentWitness0 = 6 ∨ tx_dependentWitness0 = 9 -/
def process_transaction (tx : (Sigma fun (k_blob_limit : Nat) => (TransactionFields k_blob_limit))) (allowance : (TransactionGasAllowanceFields k_total k_regular)) : SailM (Sigma
  fun (k_state_gas : Nat) =>
  (Sigma fun (k_execution_gas : Nat) =>
  (Sigma fun (k_gas_used : Nat) =>
  (ReceiptFields k_total k_regular k_gas_used k_execution_gas k_state_gas)))) := do
  let tx_dependentWitness0 := (tx).1
  let tx := (tx).2
  (k_tx_reset ())
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, validity⟩⟩⟩⟩⟩ ← do
    (check_transaction_validity ⟨_, tx⟩ allowance)
  let authorizations ← do (prepare_authorizations tx.authorizations)
  let environment := (tx_env tx.sender validity.gas_price tx.blob_hashes)
  (k_set_tx ⟨_, environment⟩)
  let upfront ← do
    (apply_transaction_upfront_effects ⟨_, tx⟩
      ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, validity⟩⟩⟩⟩⟩⟩⟩ authorizations)
  let frame_result ← do
    (run_transaction_frame ⟨_, tx⟩ ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, validity⟩⟩⟩⟩⟩ upfront
      authorizations)
  (settle_transaction ⟨_, tx⟩ ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, validity⟩⟩⟩⟩⟩
    upfront.authorization_refund frame_result)

