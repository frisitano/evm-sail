import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Bytes
import Evm.Primitives.Crypto
import Evm.Primitives.Tx
import Evm.Host.EvmByteSlice
import Evm.Lib.Rlp.Rlp
import Evm.Lib.Tx

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

/-! # Transaction decoding

The EIP-2718 transaction envelope decoder: per-envelope destructuring into
the [Transaction][type-Transaction] type (one walk, against each
envelope's exact field shape), plus the EIP-2930 access-list
and EIP-7702 authorization-tuple decoders. Standalone and purely
structural: input is a `EvmByteSlice` envelope and a `EvmByteSlice` public key
of any source; the signature rules and the cryptography live in their own
modules. -/

def EMPTY_ACCESS_LIST_DECODE : (AccessListDecode 0 0) :=
  { addresses := [],
    storage_slots := [],
    address_count := 0,
    slot_count := 0 }

/- Type quantifiers: value : Int, (source_valid_length (value + 1)) -/
def rlp_item_count_increment (value : Int) : Nat :=
  (value +i 1)

/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, k_address_bound : Nat, k_slot_bound
  : Nat, (source_valid_range k_source_off k_source_len) ∧
  (source_valid_length k_address_bound) ∧
  (source_valid_length k_slot_bound) ∧ (source_valid_length (k_slot_bound + k_source_len)), 0 ≤
  _reclimit -/
def _rec_decode_access_list_keys (cursor : (EvmByteSliceFields k_source_off k_source_len)) (addr : (Vector (BitVec 8) 20)) (tail : (AccessListDecode k_address_bound k_slot_bound)) (_reclimit : Nat) : SailM (AccessListDecode k_address_bound (k_slot_bound + k_source_len)) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then
        (pure { addresses := tail.addresses,
                storage_slots := tail.storage_slots,
                address_count := tail.address_count,
                slot_count := tail.slot_count })
      else
        (do
          let ⟨key_content_len, ⟨key_full_len, (key, next)⟩⟩ ← do (rlp_cursor_pop cursor)
          let storage_key ← (( do
            (pure { addr := addr,
                    slot := ← (rlp_ref_word key) }) ) : SailM StorageKey )
          let result ← do (_rec_decode_access_list_keys next addr tail _reclimit_pred)
          (pure { addresses := result.addresses,
                  storage_slots := (storage_key :: result.storage_slots),
                  address_count := result.address_count,
                  slot_count := (rlp_item_count_increment result.slot_count) })))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_address_bound : Nat, k_slot_bound :
  Nat, (source_valid_range k_source_off k_source_len) ∧
  (source_valid_length k_address_bound) ∧
  (source_valid_length k_slot_bound) ∧ (source_valid_length (k_slot_bound + k_source_len)) -/
def decode_access_list_keys (cursor : (EvmByteSliceFields k_source_off k_source_len)) (addr : (Vector (BitVec 8) 20)) (tail : (AccessListDecode k_address_bound k_slot_bound)) : SailM (AccessListDecode k_address_bound (k_slot_bound + k_source_len)) := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_decode_access_list_keys cursor addr tail (_measure + 1))

/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_decode_access_list_entries (cursor : (EvmByteSliceFields k_source_off k_source_len)) (_reclimit : Nat) : SailM (AccessListDecode k_source_len k_source_len) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then
        (pure { addresses := [],
                storage_slots := [],
                address_count := 0,
                slot_count := 0 })
      else
        (do
          let ⟨entry_content_len, ⟨entry_full_len, (entry, next)⟩⟩ ← do
            (rlp_cursor_pop cursor)
          let ⟨addr_f_content_len, ⟨addr_f_full_len, (addr_f, entry_fields)⟩⟩ ← do
            (rlp_cursor_pop (← (rlp_ref_cursor entry)))
          let ⟨keys_f_content_len, ⟨keys_f_full_len, (keys_f, entry_fields)⟩⟩ ← do
            (rlp_cursor_pop entry_fields)
          (rlp_cursor_expect_end entry_fields)
          let addr ← do (pure (word_to_address (← (rlp_ref_word addr_f))))
          let tail ← do (_rec_decode_access_list_entries next _reclimit_pred)
          let result ← do (decode_access_list_keys (← (rlp_ref_cursor keys_f)) addr tail)
          (pure { addresses := (addr :: result.addresses),
                  storage_slots := result.storage_slots,
                  address_count := (rlp_item_count_increment result.address_count),
                  slot_count := result.slot_count })))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def decode_access_list_entries (cursor : (EvmByteSliceFields k_source_off k_source_len)) : SailM (AccessListDecode k_source_len k_source_len) := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_decode_access_list_entries cursor (_measure + 1))

/-- Decodes an EIP-2930 access list — RLP `[[address, [slot, …]], …]` —
into the flat representation the transaction and intrinsic-gas
accounting use: the addresses (one per entry) and the
`(address, slot)` pairs. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def decode_access_list (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM ((List (Vector (BitVec 8) 20)) × (List StorageKey) × Nat × Nat) := do
  if ((k_source_len ≤b (2 ^i 30)) : Bool)
  then
    (do
      let decoded ← do (decode_access_list_entries (← (rlp_ref_cursor f)))
      (pure (decoded.addresses, decoded.storage_slots, decoded.address_count, decoded.slot_count)))
  else sailThrow ((InvalidBlock RlpDecode))

def BLOB_HASH_RLP_LENGTH : Nat := 33

def BLOB_HASH_LENGTH : Nat := WORD_BYTE_LENGTH

/-- Validates every fixed-width versioned-hash item and returns their count. -/
/- Type quantifiers: _reclimit : Nat, k_ex416135_ : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ k_ex416135_ ∧ k_ex416135_ ≤ 9, 0 ≤ _reclimit -/
def _rec_decode_blob_hash_items (cursor : (EvmByteSliceFields k_source_off k_source_len)) (count : Nat) (_reclimit : Nat) : SailM Nat := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then (pure count)
      else
        (do
          let ⟨item_content_len, ⟨item_full_len, (item, next)⟩⟩ ← do
            (rlp_cursor_pop cursor)
          if ((← if (item.is_list : Bool)
               then (pure true)
               else
                 (do
                   if ((item_full_len != BLOB_HASH_RLP_LENGTH) : Bool)
                   then (pure true)
                   else
                     (do
                       if ((item_content_len != BLOB_HASH_LENGTH) : Bool)
                       then (pure true)
                       else
                         (do
                           (pure ((← (slice_byte ⟨_, ⟨_, item.source⟩⟩ 0)) != 0xA0#8)))))) : Bool)
          then sailThrow ((InvalidBlock RlpDecode))
          else
            (do
              if ((count <b 9) : Bool)
              then (_rec_decode_blob_hash_items next (count + 1) _reclimit_pred)
              else sailThrow ((InvalidBlock RlpDecode)))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Validates every fixed-width versioned-hash item and returns their count. -/
/- Type quantifiers: count : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ count ∧ count ≤ 9 -/
def decode_blob_hash_items (cursor : (EvmByteSliceFields k_source_off k_source_len)) (count : Nat) : SailM Nat := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_decode_blob_hash_items cursor count (_measure + 1))

/-- Validates the canonical RLP list of `bytes32` blob versioned hashes
once, then retains its encoded content as a fixed-stride source view:
each element is exactly `0xa0` followed by 32 bytes, so `BLOBHASH` can
load item `i` at `33·i + 1`. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def decode_blob_hashes (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM BlobHashes := do
  let ⟨_, ⟨_, bytes⟩⟩ := (rlp_ref_content f)
  (pure { bytes := ⟨_, ⟨_, bytes⟩⟩,
          count := ← (decode_blob_hash_items (← (rlp_ref_cursor f)) 0) })

def EMPTY_AUTHORIZATION_DECODE : (AuthorizationDecode 0) :=
  { authorizations := [],
    count := 0 }

/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_decode_auth_tuples (cursor : (EvmByteSliceFields k_source_off k_source_len)) (_reclimit : Nat) : SailM (AuthorizationDecode k_source_len) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then
        (pure { authorizations := [],
                count := 0 })
      else
        (do
          let ⟨tuple_content_len, ⟨tuple_full_len, (tuple, next)⟩⟩ ← do
            (rlp_cursor_pop cursor)
          let ⟨chain_f_content_len, ⟨chain_f_full_len, (chain_f, fields)⟩⟩ ← do
            (rlp_cursor_pop (← (rlp_ref_cursor tuple)))
          let ⟨addr_f_content_len, ⟨addr_f_full_len, (addr_f, fields)⟩⟩ ← do
            (rlp_cursor_pop fields)
          let ⟨nonce_f_content_len, ⟨nonce_f_full_len, (nonce_f, fields)⟩⟩ ← do
            (rlp_cursor_pop fields)
          let ⟨y_f_content_len, ⟨y_f_full_len, (y_f, fields)⟩⟩ ← do
            (rlp_cursor_pop fields)
          let ⟨r_f_content_len, ⟨r_f_full_len, (r_f, fields)⟩⟩ ← do
            (rlp_cursor_pop fields)
          let ⟨s_f_content_len, ⟨s_f_full_len, (s_f, fields)⟩⟩ ← do
            (rlp_cursor_pop fields)
          (rlp_cursor_expect_end fields)
          let chain_id ← do (rlp_ref_uint_word chain_f)
          let auth_nonce ← (( do (rlp_ref_uint64 nonce_f) ) : SailM Nat )
          let y_value ← do (rlp_ref_uint64 y_f)
          let y_valid := (y_value ≤b 1)
          let y : Nat :=
            if ((y_value == 0) : Bool)
            then 0
            else 1
          let r ← do (rlp_ref_uint_word r_f)
          let s ← do (rlp_ref_uint_word s_f)
          let auth_addr ← do (pure (word_to_address (← (rlp_ref_word addr_f))))
          let (ok, authority) ← do
            if (y_valid : Bool)
            then (ecrecover_addr (← (auth_signing_hash chain_id auth_addr auth_nonce)) y r s)
            else (pure (false, ZERO_ADDRESS))
          let nonce_valid := (auth_nonce != ((2 ^i 64) - 1))
          let authorization : Authorization :=
            { valid_sig := (ok && (y_valid && ((word_ult ZERO_WORD r) && ((word_ult r SECP_N_FULL) && ((word_ult
                            ZERO_WORD s) && ((word_ule s SECP_N_HALF) && nonce_valid)))))),
              authority := authority,
              address := auth_addr,
              nonce := auth_nonce,
              chain_id := chain_id }
          let tail ← do (_rec_decode_auth_tuples next _reclimit_pred)
          (pure { authorizations := (authorization :: tail.authorizations),
                  count := (rlp_item_count_increment tail.count) })))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def decode_auth_tuples (cursor : (EvmByteSliceFields k_source_off k_source_len)) : SailM (AuthorizationDecode k_source_len) := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_decode_auth_tuples cursor (_measure + 1))

/-- Decodes an EIP-7702 authorization list into
[Authorization][type-Authorization] tuples, recovering each authority.
Each set-code tuple `[chainId, address, nonce, yParity, r, s]` is
signed by the authority, whose address is recovered (not witnessed)
via `ecrecover` over `keccak256(0x05 ++ rlp([chainId, address,
nonce]))`. `valid_sig` records tuple-local signature validity
(recoverable, low-`s`, `r`/`y_parity` ranges, incrementable nonce);
[process_auth][] then applies the chain-id / account nonce / code
checks. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def decode_auth_list (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM ((List Authorization) × Nat) := do
  if ((k_source_len ≤b (2 ^i 30)) : Bool)
  then
    (do
      let decoded ← do (decode_auth_tuples (← (rlp_ref_cursor f)))
      (pure (decoded.authorizations, decoded.count)))
  else sailThrow ((InvalidBlock RlpDecode))

/-- The calldata/initcode span of the data field within the envelope. The RLP
cursor existentially hides its bounded source length, so this boundary
reifies the already-established transaction-envelope invariant. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def tx_input_span (data : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  let ⟨_, ⟨_, content⟩⟩ := (rlp_ref_content data)
  if _sailIf0 : ((content.len ≤b (2 ^i 30)) : Bool) = true
  then
    (pure ((⟨_, ⟨_, content⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  else
    (do
      sailThrow ((InvalidBlock RlpDecode)))

/- Type quantifiers: k_first_source_off : Nat, k_first_source_len : Nat, k_first_content_len : Nat, k_signature_source_off
  : Nat, k_signature_source_len : Nat, k_signature_content_len : Nat, (rlp_field_ref_valid k_first_source_off k_first_source_len k_first_content_len)
  ∧
  (rlp_field_ref_valid k_signature_source_off k_signature_source_len k_signature_content_len) ∧
  k_first_source_off ≤ k_signature_source_off -/
def tx_sig_span (first : (RlpFieldRef k_first_source_off k_first_source_len k_first_content_len)) (signature : (RlpFieldRef k_signature_source_off k_signature_source_len k_signature_content_len)) : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) :=
  let start := k_first_source_off
  let stop := k_signature_source_off
  let start_offset := start
  let stop_offset := stop
  ((⟨_, ⟨_, (byte_slice first.source.source start_offset (stop_offset - start_offset))⟩⟩ : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))

/-- Decodes transaction gas and enforces the protocol and fork-specific
transaction bounds. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_ref_gas (f : (RlpFieldRef k_source_off k_source_len k_content_len)) (fork : Fork) : SailM Nat := do
  let value ← do (rlp_ref_uint f)
  if (((fork_gteq fork Osaka) && ((fork_lt fork Amsterdam) && (OSAKA_TRANSACTION_GAS_LIMIT_VALUE <b value))) : Bool)
  then sailThrow ((InvalidBlock GasUsedExceedsLimit))
  else (pure value)

/-- Decodes the payload fields of a legacy transaction. Kept separate from
the envelope dispatcher so extraction backends compile each transaction
shape as an independent function. -/
/- Type quantifiers: pubkey_dependentWitness1 : Nat, pubkey_dependentWitness0 : Nat, tx_dependentWitness1
  : Nat, tx_dependentWitness0 : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ tx_dependentWitness0 ∧ 0 ≤ tx_dependentWitness1 ∧
  0 ≤ tx_dependentWitness1 ∧ tx_dependentWitness1 ≤ (2 ^ 30), 0 ≤ pubkey_dependentWitness0
  ∧ 0 ≤ pubkey_dependentWitness1 -/
def decode_legacy_tx (tx : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (pubkey : (Sigma fun
  (tx_dependentWitness0 : Nat) =>
  (Sigma fun (tx_dependentWitness1 : Nat) =>
  (EvmByteSliceFields tx_dependentWitness0 tx_dependentWitness1)))) (fork : Fork) (sender : (Vector (BitVec 8) 20)) (fields : (EvmByteSliceFields k_source_off k_source_len)) : SailM Transaction := do
  let tx_dependentWitness0 := (tx).1
  let tx_dependentWitness1 := ((tx).2).1
  let tx := ((tx).2).2
  let pubkey_dependentWitness0 := (pubkey).1
  let pubkey_dependentWitness1 := ((pubkey).2).1
  let pubkey := ((pubkey).2).2
  let ⟨nonce_f_content_len, ⟨nonce_f_full_len, (nonce_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨gp_f_content_len, ⟨gp_f_full_len, (gp_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨gas_f_content_len, ⟨gas_f_full_len, (gas_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨to_f_content_len, ⟨to_f_full_len, (to_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨value_f_content_len, ⟨value_f_full_len, (value_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨data_f_content_len, ⟨data_f_full_len, (data_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨v_f_content_len, ⟨v_f_full_len, (v_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨r_f_content_len, ⟨r_f_full_len, (r_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨s_f_content_len, ⟨s_f_full_len, (s_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  (rlp_cursor_expect_end fields)
  let v ← do (rlp_ref_word v_f)
  let gp ← do (rlp_ref_uint_word gp_f)
  (pure { tx_type := LegacyTx,
          sender := sender,
          raw := ⟨_, ⟨_, tx⟩⟩,
          nonce := ← (rlp_ref_uint_word nonce_f),
          chain_id := 0,
          gas_limit := ← (rlp_ref_gas gas_f fork),
          is_create := (to_f_content_len == 0),
          recipient := ← (pure (word_to_address (← (rlp_ref_word to_f)))),
          value := ← (rlp_ref_uint_word value_f),
          input_src := ← do
              let publicField ← (tx_input_span data_f)
              pure (publicField),
          access_list_addresses := [],
          access_list_address_count := 0,
          access_list_slots := [],
          access_list_slot_count := 0,
          max_fee := gp,
          max_blob_fee := ZERO_WORD,
          max_priority_fee := gp,
          authorizations := [],
          authorization_count := 0,
          blob_hashes := EMPTY_BLOB_HASHES,
          pubkey := ⟨_, ⟨_, pubkey⟩⟩,
          signing_hash := ← (tx_signing_hash LegacyTx (tx_sig_span nonce_f v_f) v),
          sig_v := v,
          sig_r := ← (rlp_ref_uint_word r_f),
          sig_s := ← (rlp_ref_uint_word s_f) })

/-- Decodes the payload fields of an EIP-2930 transaction. -/
/- Type quantifiers: pubkey_dependentWitness1 : Nat, pubkey_dependentWitness0 : Nat, tx_dependentWitness1
  : Nat, tx_dependentWitness0 : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ tx_dependentWitness0 ∧ 0 ≤ tx_dependentWitness1 ∧
  0 ≤ tx_dependentWitness1 ∧ tx_dependentWitness1 ≤ (2 ^ 30), 0 ≤ pubkey_dependentWitness0
  ∧ 0 ≤ pubkey_dependentWitness1 -/
def decode_access_list_tx (tx : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (pubkey : (Sigma fun
  (tx_dependentWitness0 : Nat) =>
  (Sigma fun (tx_dependentWitness1 : Nat) =>
  (EvmByteSliceFields tx_dependentWitness0 tx_dependentWitness1)))) (fork : Fork) (sender : (Vector (BitVec 8) 20)) (fields : (EvmByteSliceFields k_source_off k_source_len)) : SailM Transaction := do
  let tx_dependentWitness0 := (tx).1
  let tx_dependentWitness1 := ((tx).2).1
  let tx := ((tx).2).2
  let pubkey_dependentWitness0 := (pubkey).1
  let pubkey_dependentWitness1 := ((pubkey).2).1
  let pubkey := ((pubkey).2).2
  let ⟨chain_f_content_len, ⟨chain_f_full_len, (chain_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨nonce_f_content_len, ⟨nonce_f_full_len, (nonce_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨gp_f_content_len, ⟨gp_f_full_len, (gp_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨gas_f_content_len, ⟨gas_f_full_len, (gas_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨to_f_content_len, ⟨to_f_full_len, (to_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨value_f_content_len, ⟨value_f_full_len, (value_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨data_f_content_len, ⟨data_f_full_len, (data_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨al_f_content_len, ⟨al_f_full_len, (al_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨v_f_content_len, ⟨v_f_full_len, (v_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨r_f_content_len, ⟨r_f_full_len, (r_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨s_f_content_len, ⟨s_f_full_len, (s_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  (rlp_cursor_expect_end fields)
  let v ← do (rlp_ref_word v_f)
  let gp ← do (rlp_ref_uint_word gp_f)
  let (al_addrs, al_slots, al_addr_count, al_slot_count) ← do (decode_access_list al_f)
  (pure { tx_type := AccessListTx,
          sender := sender,
          raw := ⟨_, ⟨_, tx⟩⟩,
          nonce := ← (rlp_ref_uint_word nonce_f),
          chain_id := ← (rlp_ref_uint64 chain_f),
          gas_limit := ← (rlp_ref_gas gas_f fork),
          is_create := (to_f_content_len == 0),
          recipient := ← (pure (word_to_address (← (rlp_ref_word to_f)))),
          value := ← (rlp_ref_uint_word value_f),
          input_src := ← do
              let publicField ← (tx_input_span data_f)
              pure (publicField),
          access_list_addresses := al_addrs,
          access_list_address_count := al_addr_count,
          access_list_slots := al_slots,
          access_list_slot_count := al_slot_count,
          max_fee := gp,
          max_blob_fee := ZERO_WORD,
          max_priority_fee := gp,
          authorizations := [],
          authorization_count := 0,
          blob_hashes := EMPTY_BLOB_HASHES,
          pubkey := ⟨_, ⟨_, pubkey⟩⟩,
          signing_hash := ← (tx_signing_hash AccessListTx (tx_sig_span chain_f v_f) v),
          sig_v := v,
          sig_r := ← (rlp_ref_uint_word r_f),
          sig_s := ← (rlp_ref_uint_word s_f) })

/-- Decodes the payload fields of an EIP-1559 transaction. -/
/- Type quantifiers: pubkey_dependentWitness1 : Nat, pubkey_dependentWitness0 : Nat, tx_dependentWitness1
  : Nat, tx_dependentWitness0 : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ tx_dependentWitness0 ∧ 0 ≤ tx_dependentWitness1 ∧
  0 ≤ tx_dependentWitness1 ∧ tx_dependentWitness1 ≤ (2 ^ 30), 0 ≤ pubkey_dependentWitness0
  ∧ 0 ≤ pubkey_dependentWitness1 -/
def decode_fee_market_tx (tx : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (pubkey : (Sigma fun
  (tx_dependentWitness0 : Nat) =>
  (Sigma fun (tx_dependentWitness1 : Nat) =>
  (EvmByteSliceFields tx_dependentWitness0 tx_dependentWitness1)))) (fork : Fork) (sender : (Vector (BitVec 8) 20)) (fields : (EvmByteSliceFields k_source_off k_source_len)) : SailM Transaction := do
  let tx_dependentWitness0 := (tx).1
  let tx_dependentWitness1 := ((tx).2).1
  let tx := ((tx).2).2
  let pubkey_dependentWitness0 := (pubkey).1
  let pubkey_dependentWitness1 := ((pubkey).2).1
  let pubkey := ((pubkey).2).2
  let ⟨chain_f_content_len, ⟨chain_f_full_len, (chain_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨nonce_f_content_len, ⟨nonce_f_full_len, (nonce_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨mp_f_content_len, ⟨mp_f_full_len, (mp_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨mf_f_content_len, ⟨mf_f_full_len, (mf_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨gas_f_content_len, ⟨gas_f_full_len, (gas_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨to_f_content_len, ⟨to_f_full_len, (to_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨value_f_content_len, ⟨value_f_full_len, (value_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨data_f_content_len, ⟨data_f_full_len, (data_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨al_f_content_len, ⟨al_f_full_len, (al_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨v_f_content_len, ⟨v_f_full_len, (v_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨r_f_content_len, ⟨r_f_full_len, (r_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨s_f_content_len, ⟨s_f_full_len, (s_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  (rlp_cursor_expect_end fields)
  let v ← do (rlp_ref_word v_f)
  let (al_addrs, al_slots, al_addr_count, al_slot_count) ← do (decode_access_list al_f)
  (pure { tx_type := FeeMarketTx,
          sender := sender,
          raw := ⟨_, ⟨_, tx⟩⟩,
          nonce := ← (rlp_ref_uint_word nonce_f),
          chain_id := ← (rlp_ref_uint64 chain_f),
          gas_limit := ← (rlp_ref_gas gas_f fork),
          is_create := (to_f_content_len == 0),
          recipient := ← (pure (word_to_address (← (rlp_ref_word to_f)))),
          value := ← (rlp_ref_uint_word value_f),
          input_src := ← do
              let publicField ← (tx_input_span data_f)
              pure (publicField),
          access_list_addresses := al_addrs,
          access_list_address_count := al_addr_count,
          access_list_slots := al_slots,
          access_list_slot_count := al_slot_count,
          max_fee := ← (rlp_ref_uint_word mf_f),
          max_blob_fee := ZERO_WORD,
          max_priority_fee := ← (rlp_ref_uint_word mp_f),
          authorizations := [],
          authorization_count := 0,
          blob_hashes := EMPTY_BLOB_HASHES,
          pubkey := ⟨_, ⟨_, pubkey⟩⟩,
          signing_hash := ← (tx_signing_hash FeeMarketTx (tx_sig_span chain_f v_f) v),
          sig_v := v,
          sig_r := ← (rlp_ref_uint_word r_f),
          sig_s := ← (rlp_ref_uint_word s_f) })

/-- Decodes the payload fields of an EIP-4844 transaction. -/
/- Type quantifiers: pubkey_dependentWitness1 : Nat, pubkey_dependentWitness0 : Nat, tx_dependentWitness1
  : Nat, tx_dependentWitness0 : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ tx_dependentWitness0 ∧ 0 ≤ tx_dependentWitness1 ∧
  0 ≤ tx_dependentWitness1 ∧ tx_dependentWitness1 ≤ (2 ^ 30), 0 ≤ pubkey_dependentWitness0
  ∧ 0 ≤ pubkey_dependentWitness1 -/
def decode_blob_tx (tx : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (pubkey : (Sigma fun
  (tx_dependentWitness0 : Nat) =>
  (Sigma fun (tx_dependentWitness1 : Nat) =>
  (EvmByteSliceFields tx_dependentWitness0 tx_dependentWitness1)))) (fork : Fork) (sender : (Vector (BitVec 8) 20)) (fields : (EvmByteSliceFields k_source_off k_source_len)) : SailM Transaction := do
  let tx_dependentWitness0 := (tx).1
  let tx_dependentWitness1 := ((tx).2).1
  let tx := ((tx).2).2
  let pubkey_dependentWitness0 := (pubkey).1
  let pubkey_dependentWitness1 := ((pubkey).2).1
  let pubkey := ((pubkey).2).2
  let ⟨chain_f_content_len, ⟨chain_f_full_len, (chain_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨nonce_f_content_len, ⟨nonce_f_full_len, (nonce_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨mp_f_content_len, ⟨mp_f_full_len, (mp_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨mf_f_content_len, ⟨mf_f_full_len, (mf_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨gas_f_content_len, ⟨gas_f_full_len, (gas_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨to_f_content_len, ⟨to_f_full_len, (to_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨value_f_content_len, ⟨value_f_full_len, (value_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨data_f_content_len, ⟨data_f_full_len, (data_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨al_f_content_len, ⟨al_f_full_len, (al_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨mbf_f_content_len, ⟨mbf_f_full_len, (mbf_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨bh_f_content_len, ⟨bh_f_full_len, (bh_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨v_f_content_len, ⟨v_f_full_len, (v_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨r_f_content_len, ⟨r_f_full_len, (r_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨s_f_content_len, ⟨s_f_full_len, (s_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  (rlp_cursor_expect_end fields)
  let v ← do (rlp_ref_word v_f)
  let (al_addrs, al_slots, al_addr_count, al_slot_count) ← do (decode_access_list al_f)
  let blob_hashes ← do (decode_blob_hashes bh_f)
  (pure { tx_type := BlobTx,
          sender := sender,
          raw := ⟨_, ⟨_, tx⟩⟩,
          nonce := ← (rlp_ref_uint_word nonce_f),
          chain_id := ← (rlp_ref_uint64 chain_f),
          gas_limit := ← (rlp_ref_gas gas_f fork),
          is_create := (to_f_content_len == 0),
          recipient := ← (pure (word_to_address (← (rlp_ref_word to_f)))),
          value := ← (rlp_ref_uint_word value_f),
          input_src := ← do
              let publicField ← (tx_input_span data_f)
              pure (publicField),
          access_list_addresses := al_addrs,
          access_list_address_count := al_addr_count,
          access_list_slots := al_slots,
          access_list_slot_count := al_slot_count,
          max_fee := ← (rlp_ref_uint_word mf_f),
          max_blob_fee := ← (rlp_ref_uint_word mbf_f),
          max_priority_fee := ← (rlp_ref_uint_word mp_f),
          authorizations := [],
          authorization_count := 0,
          blob_hashes := blob_hashes,
          pubkey := ⟨_, ⟨_, pubkey⟩⟩,
          signing_hash := ← (tx_signing_hash BlobTx (tx_sig_span chain_f v_f) v),
          sig_v := v,
          sig_r := ← (rlp_ref_uint_word r_f),
          sig_s := ← (rlp_ref_uint_word s_f) })

/-- Decodes the payload fields of an EIP-7702 transaction. -/
/- Type quantifiers: pubkey_dependentWitness1 : Nat, pubkey_dependentWitness0 : Nat, tx_dependentWitness1
  : Nat, tx_dependentWitness0 : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ tx_dependentWitness0 ∧ 0 ≤ tx_dependentWitness1 ∧
  0 ≤ tx_dependentWitness1 ∧ tx_dependentWitness1 ≤ (2 ^ 30), 0 ≤ pubkey_dependentWitness0
  ∧ 0 ≤ pubkey_dependentWitness1 -/
def decode_set_code_tx (tx : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (pubkey : (Sigma fun
  (tx_dependentWitness0 : Nat) =>
  (Sigma fun (tx_dependentWitness1 : Nat) =>
  (EvmByteSliceFields tx_dependentWitness0 tx_dependentWitness1)))) (fork : Fork) (sender : (Vector (BitVec 8) 20)) (fields : (EvmByteSliceFields k_source_off k_source_len)) : SailM Transaction := do
  let tx_dependentWitness0 := (tx).1
  let tx_dependentWitness1 := ((tx).2).1
  let tx := ((tx).2).2
  let pubkey_dependentWitness0 := (pubkey).1
  let pubkey_dependentWitness1 := ((pubkey).2).1
  let pubkey := ((pubkey).2).2
  let ⟨chain_f_content_len, ⟨chain_f_full_len, (chain_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨nonce_f_content_len, ⟨nonce_f_full_len, (nonce_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨mp_f_content_len, ⟨mp_f_full_len, (mp_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨mf_f_content_len, ⟨mf_f_full_len, (mf_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨gas_f_content_len, ⟨gas_f_full_len, (gas_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨to_f_content_len, ⟨to_f_full_len, (to_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨value_f_content_len, ⟨value_f_full_len, (value_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨data_f_content_len, ⟨data_f_full_len, (data_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨al_f_content_len, ⟨al_f_full_len, (al_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨auth_f_content_len, ⟨auth_f_full_len, (auth_f, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨v_f_content_len, ⟨v_f_full_len, (v_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨r_f_content_len, ⟨r_f_full_len, (r_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  let ⟨s_f_content_len, ⟨s_f_full_len, (s_f, fields)⟩⟩ ← do (rlp_cursor_pop fields)
  (rlp_cursor_expect_end fields)
  let v ← do (rlp_ref_word v_f)
  let (al_addrs, al_slots, al_addr_count, al_slot_count) ← do (decode_access_list al_f)
  let (authorizations, authorization_count) ← do (decode_auth_list auth_f)
  (pure { tx_type := SetCodeTx,
          sender := sender,
          raw := ⟨_, ⟨_, tx⟩⟩,
          nonce := ← (pure (word_of_account_nonce (← (rlp_ref_uint64 nonce_f)))),
          chain_id := ← (rlp_ref_uint64 chain_f),
          gas_limit := ← (rlp_ref_gas gas_f fork),
          is_create := (to_f_content_len == 0),
          recipient := ← (pure (word_to_address (← (rlp_ref_word to_f)))),
          value := ← (rlp_ref_uint_word value_f),
          input_src := ← do
              let publicField ← (tx_input_span data_f)
              pure (publicField),
          access_list_addresses := al_addrs,
          access_list_address_count := al_addr_count,
          access_list_slots := al_slots,
          access_list_slot_count := al_slot_count,
          max_fee := ← (rlp_ref_uint_word mf_f),
          max_blob_fee := ZERO_WORD,
          max_priority_fee := ← (rlp_ref_uint_word mp_f),
          authorizations := authorizations,
          authorization_count := authorization_count,
          blob_hashes := EMPTY_BLOB_HASHES,
          pubkey := ⟨_, ⟨_, pubkey⟩⟩,
          signing_hash := ← (tx_signing_hash SetCodeTx (tx_sig_span chain_f v_f) v),
          sig_v := v,
          sig_r := ← (rlp_ref_uint_word r_f),
          sig_s := ← (rlp_ref_uint_word s_f) })

/- Type quantifiers: k_tx_off : Nat, k_tx_len : Nat, k_public_key_off : Nat, (source_valid_range k_tx_off k_tx_len)
  ∧
  0 ≤ k_tx_len ∧
  k_tx_len ≤ transaction_length_bound ∧ (source_valid_range k_public_key_off 65) -/
def rlp_decode_tx (tx : (EvmByteSliceFields k_tx_off k_tx_len)) (pubkey : (EvmByteSliceFields k_public_key_off 65)) (fork : Fork) : SailM Transaction := do
  let sender ← do
    (pure (word_to_address
        (hash_to_word
          (← (keccak256_slice ⟨_, ⟨_, (sub_slice pubkey 1 PUBLIC_KEY_BODY_LENGTH)⟩⟩)))))
  let tx_length := k_tx_len
  let b0 ← (( do
    if ((tx_length == 0) : Bool)
    then sailThrow ((InvalidBlock RlpDecode))
    else (slice_byte ⟨_, ⟨_, tx⟩⟩ 0) ) : SailM (BitVec 8) )
  let ttype : (BitVec 8) :=
    if (((Sail.BitVec.extractLsb b0 7 6) == 0b11#2) : Bool)
    then 0x00#8
    else b0
  let typed := (ttype != 0x00#8)
  let ⟨_, ⟨_, payload⟩⟩ ← (( do
    if _sailIf0 : (typed : Bool) = true
    then
      (do
        if _sailIf1 : ((1 ≤b tx_length) : Bool) = true
        then
          (pure ((⟨_, ⟨_, (sub_slice tx 1 (tx_length - 1))⟩⟩ : (Sigma fun (k_off : Nat) =>
            (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat)
            => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
        else
          (do
            sailThrow ((InvalidBlock RlpDecode))))
    else
      (pure ((⟨_, ⟨_, tx⟩⟩ : (Sigma fun (k_off : Nat) =>
        (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
        (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))) ) : SailM
    (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) )
  let ⟨_, ⟨_, fields⟩⟩ ← do (rlp_node_cursor ⟨_, ⟨_, payload⟩⟩)
  match ttype with
  | 0x00 => (decode_legacy_tx ⟨_, ⟨_, tx⟩⟩ ⟨_, ⟨_, pubkey⟩⟩ fork sender fields)
  | 0x01 => (decode_access_list_tx ⟨_, ⟨_, tx⟩⟩ ⟨_, ⟨_, pubkey⟩⟩ fork sender fields)
  | 0x02 => (decode_fee_market_tx ⟨_, ⟨_, tx⟩⟩ ⟨_, ⟨_, pubkey⟩⟩ fork sender fields)
  | 0x03 => (decode_blob_tx ⟨_, ⟨_, tx⟩⟩ ⟨_, ⟨_, pubkey⟩⟩ fork sender fields)
  | 0x04 => (decode_set_code_tx ⟨_, ⟨_, tx⟩⟩ ⟨_, ⟨_, pubkey⟩⟩ fork sender fields)
  | _ => sailThrow ((InvalidBlock RlpDecode))

