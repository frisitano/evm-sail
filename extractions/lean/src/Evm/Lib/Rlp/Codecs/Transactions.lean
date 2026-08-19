import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Host.RegionAccess
import Evm.Primitives.Crypto
import Evm.Primitives.Tx
import Evm.Lib.Rlp.Decoding
import Evm.Lib.Rlp.Codecs.TransactionSigning

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

/-! # Transaction RLP decoding

The EIP-2718 transaction envelope decoder: per-envelope destructuring into
the [Transaction][type-Transaction] type (one walk, against each
envelope's exact field shape), plus the EIP-2930 access-list
and EIP-7702 authorization-tuple decoders. Standalone and purely
structural: the envelope and public key are immutable stateless-input
spans; the signature rules and the cryptography live in their own modules. -/

/- Type quantifiers: atom_address_bound : Nat, atom_slot_bound : Nat, (source_valid_length atom_address_bound)
  ∧ (source_valid_length atom_slot_bound) -/
def undefined_AccessListDecode (atom_address_bound : Nat) (atom_slot_bound : Nat) : SailM (AccessListDecode atom_address_bound atom_slot_bound) := do
  (pure { address_count := ← (undefined_range 0 atom_address_bound),
          slot_count := ← (undefined_range 0 atom_slot_bound) })

def EMPTY_ACCESS_LIST_DECODE : (AccessListDecode 0 0) :=
  { address_count := 0,
    slot_count := 0 }

/-- Reclassifies transaction RLP content as an immutable input span. Every
cursor in this module originates in the transaction envelope. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def transaction_rlp_content (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) :=
  let ⟨_, ⟨_, content⟩⟩ := (rlp_item_content f)
  ((⟨_, ⟨_, (stateless_input_slice content.bytes content.len)⟩⟩ : (Sigma fun (k_off : Nat)
  => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))))

/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, k_address_bound : Nat, k_slot_bound
  : Nat, (source_valid_range k_source_off k_source_len) ∧
  (source_valid_length k_address_bound) ∧
  (source_valid_length k_slot_bound) ∧ (source_valid_length (k_slot_bound + k_source_len)), 0 ≤
  _reclimit -/
def _rec_decode_access_list_keys (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (addr : (Vector (BitVec 8) 20)) (tail : (AccessListDecode k_address_bound k_slot_bound)) (_reclimit : Nat) : SailM (AccessListDecode k_address_bound (k_slot_bound + k_source_len)) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then
        (pure { address_count := tail.address_count,
                slot_count := tail.slot_count })
      else
        (do
          let ⟨_, ⟨_, key⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor key.source.len)
          let _ ← (( do
            (pure { addr := addr,
                    slot := ← (rlp_decode_word key) }) ) : SailM StorageKey )
          let result ← do (_rec_decode_access_list_keys next addr tail _reclimit_pred)
          (pure { address_count := result.address_count,
                  slot_count := (result.slot_count + 1) })))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_address_bound : Nat, k_slot_bound :
  Nat, (source_valid_range k_source_off k_source_len) ∧
  (source_valid_length k_address_bound) ∧
  (source_valid_length k_slot_bound) ∧ (source_valid_length (k_slot_bound + k_source_len)) -/
def decode_access_list_keys (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (addr : (Vector (BitVec 8) 20)) (tail : (AccessListDecode k_address_bound k_slot_bound)) : SailM (AccessListDecode k_address_bound (k_slot_bound + k_source_len)) := do
  let _measure := (k_source_len : Int)
  if _sailIf0 : ((_measure <b 0) : Bool) = true
  then throw Error.Exit
  else
    (do
      (do
        let indexRefinedResult ← (_rec_decode_access_list_keys cursor addr tail (_measure + 1))
        pure (cast (by first | rfl | omega | (congr 1 <;> simp_all) | (congr 1 <;> omega) | (simp_all <;> omega) | (simp_all <;> rfl) | simp_all) (indexRefinedResult))))

/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_decode_access_list_entries (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (_reclimit : Nat) : SailM (AccessListDecode k_source_len k_source_len) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if _sailIf0 : ((k_source_len == 0) : Bool) = true
      then
        (pure ((cast (by first | rfl | omega | (congr 1 <;> simp_all) | (congr 1 <;> omega) | (simp_all <;> omega) | (simp_all <;> rfl) | simp_all) (EMPTY_ACCESS_LIST_DECODE)) : (AccessListDecode k_source_len k_source_len)))
      else
        (do
          let ⟨_, ⟨_, entry⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor entry.source.len)
          let entry_fields ← do (rlp_decode_list entry)
          let ⟨_, ⟨_, addr_f⟩⟩ ← do (rlp_decode_item entry_fields)
          let entry_fields := (rlp_cursor_advance entry_fields addr_f.source.len)
          let ⟨_, ⟨_, keys_f⟩⟩ ← do (rlp_decode_item entry_fields)
          let entry_fields := (rlp_cursor_advance entry_fields keys_f.source.len)
          (rlp_cursor_expect_end entry_fields)
          let address_word ← do (rlp_decode_word addr_f)
          let addr := (word_to_address address_word)
          let tail ← do (_rec_decode_access_list_entries next _reclimit_pred)
          let keys ← do (rlp_decode_list keys_f)
          let result ← do (decode_access_list_keys keys addr tail)
          (pure { address_count := (result.address_count + 1),
                  slot_count := result.slot_count })))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def decode_access_list_entries (cursor : (StatelessInputSliceFields k_source_off k_source_len)) : SailM (AccessListDecode k_source_len k_source_len) := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_decode_access_list_entries cursor (_measure + 1))

/-- Decodes an EIP-2930 access list — RLP `[[address, [slot, …]], …]` —
without materializing its entries. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def decode_access_list (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM AccessListRef := do
  if ((k_source_len ≤b (2 ^i 30)) : Bool)
  then
    (do
      let entries ← do (rlp_decode_list f)
      let decoded ← do (decode_access_list_entries entries)
      (pure { encoded := (transaction_rlp_content f),
              address_count := decoded.address_count,
              slot_count := decoded.slot_count }))
  else (fatal_error RlpDecode)

abbrev BLOB_HASH_RLP_LENGTH : Nat := 33

abbrev BLOB_HASH_LENGTH : Nat := 32

/-- Validates every fixed-width versioned-hash item and returns their count.
The cursor exits immediately for an empty list and checks the version byte
while each item is already live, avoiding a second fixed-width pass. -/
/- Type quantifiers: _reclimit : Nat, k_ex549349_ : Nat, k_source_off : Nat, k_source_len : Nat, limit
  : Nat, (source_valid_range k_source_off k_source_len) ∧ (transaction_blob_limit_value limit), 0
  ≤ k_ex549349_ ∧ k_ex549349_ ≤ limit, 0 ≤ _reclimit -/
def _rec_decode_blob_hash_items (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (limit : Nat) (count : Nat) (_reclimit : Nat) : SailM Nat := do
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
          let ⟨_, ⟨_, item⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor item.source.len)
          let item_prefix ← do (stateless_input_slice_byte ⟨_, ⟨_, item.source⟩⟩ 0)
          if ((item.is_list || ((item.source.len != BLOB_HASH_RLP_LENGTH) || ((item.content_len != BLOB_HASH_LENGTH) || (item_prefix != 0xA0#8)))) : Bool)
          then (fatal_error RlpDecode)
          else (pure ())
          let version ← do (stateless_input_slice_byte ⟨_, ⟨_, item.source⟩⟩ 1)
          if ((version != 0x01#8) : Bool)
          then (fatal_error ExecutionInvalid)
          else (pure ())
          if ((count <b limit) : Bool)
          then (_rec_decode_blob_hash_items next limit (count + 1) _reclimit_pred)
          else (fatal_error RlpDecode)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Validates every fixed-width versioned-hash item and returns their count.
The cursor exits immediately for an empty list and checks the version byte
while each item is already live, avoiding a second fixed-width pass. -/
/- Type quantifiers: count : Nat, k_source_off : Nat, k_source_len : Nat, limit : Nat, (source_valid_range k_source_off k_source_len)
  ∧ (transaction_blob_limit_value limit), 0 ≤ count ∧ count ≤ limit -/
def decode_blob_hash_items (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (limit : Nat) (count : Nat) : SailM Nat := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_decode_blob_hash_items cursor limit count (_measure + 1))

/-- Validates the canonical RLP list of `bytes32` blob versioned hashes
once, then retains its encoded content as a fixed-stride source view:
each element is exactly `0xa0` followed by 32 bytes, so `BLOBHASH` can
load item `i` at `33·i + 1`. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, limit : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len)
  ∧ (transaction_blob_limit_value limit) -/
def decode_blob_hashes (f : (RlpFieldRef k_source_off k_source_len k_content_len)) (limit : Nat) : SailM (BlobHashesFields limit) := do
  let ⟨_, ⟨_, bytes⟩⟩ := (transaction_rlp_content f)
  let items ← do (rlp_decode_list f)
  let count ← do (decode_blob_hash_items items limit 0)
  (pure { bytes := ⟨_, ⟨_, bytes⟩⟩,
          count := count })

/- Type quantifiers: _reclimit : Nat, count : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ count ∧ count ≤ (2 ^ 30), 0 ≤ _reclimit -/
def _rec_validate_auth_tuples (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (count : Nat) (_reclimit : Nat) : SailM Nat := do
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
          let ⟨_, ⟨_, tuple⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor tuple.source.len)
          let _ ← do (rlp_decode_list tuple)
          if ((count <b (2 ^i 30)) : Bool)
          then (_rec_validate_auth_tuples next (count + 1) _reclimit_pred)
          else (fatal_error RlpDecode)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: count : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ count ∧ count ≤ (2 ^ 30) -/
def validate_auth_tuples (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (count : Nat) : SailM Nat := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_validate_auth_tuples cursor count (_measure + 1))

/-- Decodes an EIP-7702 authorization list into
a validated source-backed cursor view. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def decode_auth_list (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM (Sigma fun
  (k_count : Nat) => (AuthorizationListRefFields k_count)) := do
  if _sailIf0 : ((k_source_len ≤b (2 ^i 30)) : Bool) = true
  then
    (do
      let ⟨_, ⟨_, content⟩⟩ := (transaction_rlp_content f)
      let tuples ← do (rlp_decode_list f)
      let count ← do (validate_auth_tuples tuples 0)
      (pure ((⟨_, (authorization_list_ref ⟨_, ⟨_, content⟩⟩ count)⟩ : (Sigma fun
        (k_count : Nat) => (AuthorizationListRefFields k_count))) : (Sigma fun (k_count : Nat) =>
        (AuthorizationListRefFields k_count)))))
  else
    (do
      (fatal_error RlpDecode))

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def decode_authorization (tuple : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Authorization := do
  let fields ← do (rlp_decode_list tuple)
  let ⟨_, ⟨_, chain_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields chain_f.source.len)
  let ⟨_, ⟨_, addr_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields addr_f.source.len)
  let ⟨_, ⟨_, nonce_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields nonce_f.source.len)
  let ⟨_, ⟨_, y_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields y_f.source.len)
  let y ← do (rlp_decode_bool y_f)
  let ⟨_, ⟨_, r_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields r_f.source.len)
  let ⟨_, ⟨_, s_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields s_f.source.len)
  (rlp_cursor_expect_end fields)
  let chain_id ← do (rlp_decode_u256 chain_f)
  let auth_nonce ← (( do (rlp_decode_uint64 nonce_f) ) : SailM Nat )
  let r ← do (rlp_decode_u256 r_f)
  let s ← do (rlp_decode_u256 s_f)
  let address_word ← do (rlp_decode_word addr_f)
  let auth_addr := (word_to_address address_word)
  let signing_hash ← do (auth_signing_hash chain_id auth_addr auth_nonce)
  let recovered ← (( do
    match y with
    | .RlpOk false => (ecrecover_addr signing_hash 0 r s)
    | .RlpOk true => (ecrecover_addr signing_hash 1 r s)
    | .RlpInvalidValue () =>
      (pure { success := false,
              address := ZERO_ADDRESS }) ) : SailM AddressResult )
  (pure { valid_sig := (recovered.success && ((word_ult ZERO_WORD r) && ((word_ult r SECP_N_FULL) && ((word_ult
                      ZERO_WORD s) && ((word_ule s SECP_N_HALF) && (auth_nonce != ((2 ^i 64) - 1))))))),
          authority := recovered.address,
          address := auth_addr,
          nonce := auth_nonce,
          chain_id := chain_id })

/- Type quantifiers: _reclimit : Nat, count : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ count ∧ count ≤ ((2 ^ 24) / 7816), 0 ≤ _reclimit -/
def _rec_prepare_authorization_entries (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (count : Nat) (_reclimit : Nat) : SailM (List Authorization) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((count == 0) : Bool)
      then
        (do
          if ((k_source_len != 0) : Bool)
          then (fatal_error RlpDecode)
          else (pure ())
          (pure []))
      else
        (do
          if ((k_source_len == 0) : Bool)
          then (fatal_error RlpDecode)
          else (pure ())
          let ⟨_, ⟨_, tuple⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor tuple.source.len)
          let authorization ← do (decode_authorization tuple)
          (pure (authorization :: (← (_rec_prepare_authorization_entries next (count - 1)
                _reclimit_pred))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: count : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ count ∧ count ≤ ((2 ^ 24) / 7816) -/
def prepare_authorization_entries (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (count : Nat) : SailM (List Authorization) := do
  let _measure := (count : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_prepare_authorization_entries cursor count (_measure + 1))

/-- Materializes a transaction's authorizations only after successful validity
has made this narrowing guard unreachable for protocol-valid input. -/
/- Type quantifiers: authorizations_dependentWitness0 : Nat, 0 ≤ authorizations_dependentWitness0
  ∧ authorizations_dependentWitness0 ≤ (2 ^ 30) -/
def prepare_authorizations (authorizations : (Sigma fun (k_count : Nat) =>
  (AuthorizationListRefFields k_count))) : SailM PreparedAuthorizationList := do
  let authorizations_dependentWitness0 := (authorizations).1
  let authorizations := (authorizations).2
  if ((authorizations.count ≤b ((2 ^i 24) / 7816)) : Bool)
  then
    (do
      let ⟨_, ⟨_, encoded⟩⟩ : (Sigma fun (k_off : Nat) =>
        (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) :=
        (authorizations.encoded : (Sigma fun (k_off : Nat) =>
        (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))))
      let entries ← do (prepare_authorization_entries encoded authorizations.count)
      (pure { entries := entries,
              count := authorizations.count }))
  else (fatal_error ExecutionInvalid)

/-- Reads the current prepared entry. Callers carry the decreasing count that
proves this operation is not applied to the empty collection. -/
def prepared_authorization_head (authorizations : PreparedAuthorizationList) : SailM Authorization := do
  match authorizations.entries with
  | (authorization :: _) => (pure authorization)
  | [] => (fatal_error ExecutionInvalid)

/- Type quantifiers: count : Nat, 0 < count ∧ count ≤ prepared_authorization_count_bound -/
def prepared_authorization_tail (authorizations : PreparedAuthorizationList) (count : Nat) : SailM PreparedAuthorizationList := do
  match authorizations.entries with
  | (_ :: entries) =>
    (pure { entries := entries,
            count := (count - 1) })
  | [] => (fatal_error ExecutionInvalid)

/-- The calldata/initcode span of the data field within the envelope. The RLP
cursor existentially hides its bounded source length, so this boundary
reifies the already-established transaction-envelope invariant. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def tx_input_span (data : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) := do
  let ⟨_, ⟨_, content⟩⟩ := (transaction_rlp_content data)
  if _sailIf0 : ((content.len ≤b (2 ^i 30)) : Bool) = true
  then
    (pure ((⟨_, ⟨_, content⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma fun
      (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))))
  else
    (do
      (fatal_error RlpDecode))

/- Type quantifiers: k_first_source_off : Nat, k_first_source_len : Nat, k_first_content_len : Nat, k_signature_source_off
  : Nat, k_signature_source_len : Nat, k_signature_content_len : Nat, (rlp_field_ref_valid k_first_source_off k_first_source_len k_first_content_len)
  ∧
  (rlp_field_ref_valid k_signature_source_off k_signature_source_len k_signature_content_len) ∧
  k_first_source_off ≤ k_signature_source_off -/
def tx_sig_span (first : (RlpFieldRef k_first_source_off k_first_source_len k_first_content_len)) (signature : (RlpFieldRef k_signature_source_off k_signature_source_len k_signature_content_len)) : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) :=
  let start := k_first_source_off
  let stop := k_signature_source_off
  let start_offset := start
  let stop_offset := stop
  ((⟨_, ⟨_, (stateless_input_slice start_offset (stop_offset - start_offset))⟩⟩ : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))))

/-- Decodes transaction gas structurally. Admission against the executing
block's correlated gas limits belongs to transaction validation, after the
envelope has been decoded. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_decode_gas (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Nat := do
  (rlp_decode_uint64 f)

/-- Decodes the payload fields of a legacy transaction. Kept separate from
the envelope dispatcher so extraction backends compile each transaction
shape as an independent function. -/
/- Type quantifiers: pubkey_dependentWitness1 : Nat, pubkey_dependentWitness0 : Nat, tx_dependentWitness1
  : Nat, tx_dependentWitness0 : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ tx_dependentWitness0 ∧
  0 ≤ tx_dependentWitness1 ∧ (tx_dependentWitness0 + tx_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ tx_dependentWitness1 ∧ tx_dependentWitness1 ≤ (2 ^ 30), 0 ≤ pubkey_dependentWitness0
  ∧
  0 ≤ pubkey_dependentWitness1 ∧
  (pubkey_dependentWitness0 + pubkey_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def decode_legacy_tx (tx : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (pubkey : (Sigma fun
  (tx_dependentWitness0 : Nat) =>
  (Sigma fun (tx_dependentWitness1 : Nat) =>
  (StatelessInputSliceFields tx_dependentWitness0 tx_dependentWitness1)))) (sender : (Vector (BitVec 8) 20)) (fields : (StatelessInputSliceFields k_source_off k_source_len)) : SailM (TransactionFields 0) := do
  let tx_dependentWitness0 := (tx).1
  let tx_dependentWitness1 := ((tx).2).1
  let tx := ((tx).2).2
  let pubkey_dependentWitness0 := (pubkey).1
  let pubkey_dependentWitness1 := ((pubkey).2).1
  let pubkey := ((pubkey).2).2
  let ⟨_, ⟨_, nonce_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields nonce_f.source.len)
  let ⟨_, ⟨_, gp_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields gp_f.source.len)
  let ⟨_, ⟨_, gas_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields gas_f.source.len)
  let ⟨_, ⟨_, to_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields to_f.source.len)
  let ⟨_, ⟨_, value_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields value_f.source.len)
  let ⟨_, ⟨_, data_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields data_f.source.len)
  let ⟨_, ⟨_, v_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields v_f.source.len)
  let ⟨_, ⟨_, r_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields r_f.source.len)
  let ⟨_, ⟨_, s_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields s_f.source.len)
  (rlp_cursor_expect_end fields)
  let v ← do (rlp_decode_word v_f)
  let gp ← do (rlp_decode_u256 gp_f)
  let recipient_word ← do (rlp_decode_word to_f)
  let recipient := (word_to_address recipient_word)
  let ⟨_, ⟨_, signing_span⟩⟩ := (tx_sig_span nonce_f v_f)
  let signing_hash ← do (tx_signing_hash LegacyTx ⟨_, ⟨_, signing_span⟩⟩ v)
  (pure { tx_type := LegacyTx,
          sender := sender,
          raw := ⟨_, ⟨_, tx⟩⟩,
          nonce := ← (rlp_decode_u256 nonce_f),
          chain_id := 0,
          gas_limit := ← (rlp_decode_gas gas_f),
          is_create := (to_f.content_len == 0),
          recipient := recipient,
          value := ← (rlp_decode_u256 value_f),
          input_src := ← do
              let publicField ← (tx_input_span data_f)
              pure (publicField),
          access_list := EMPTY_ACCESS_LIST_REF,
          max_fee := gp,
          max_blob_fee := ZERO_WORD,
          max_priority_fee := gp,
          authorizations := ⟨_, (EMPTY_AUTHORIZATION_LIST_REF).2⟩,
          blob_hashes := EMPTY_BLOB_HASHES,
          pubkey := ⟨_, ⟨_, pubkey⟩⟩,
          signing_hash := signing_hash,
          sig_v := v,
          sig_r := ← (rlp_decode_u256 r_f),
          sig_s := ← (rlp_decode_u256 s_f) })

/-- Decodes the payload fields of an EIP-2930 transaction. -/
/- Type quantifiers: pubkey_dependentWitness1 : Nat, pubkey_dependentWitness0 : Nat, tx_dependentWitness1
  : Nat, tx_dependentWitness0 : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ tx_dependentWitness0 ∧
  0 ≤ tx_dependentWitness1 ∧ (tx_dependentWitness0 + tx_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ tx_dependentWitness1 ∧ tx_dependentWitness1 ≤ (2 ^ 30), 0 ≤ pubkey_dependentWitness0
  ∧
  0 ≤ pubkey_dependentWitness1 ∧
  (pubkey_dependentWitness0 + pubkey_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def decode_access_list_tx (tx : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (pubkey : (Sigma fun
  (tx_dependentWitness0 : Nat) =>
  (Sigma fun (tx_dependentWitness1 : Nat) =>
  (StatelessInputSliceFields tx_dependentWitness0 tx_dependentWitness1)))) (sender : (Vector (BitVec 8) 20)) (fields : (StatelessInputSliceFields k_source_off k_source_len)) : SailM (TransactionFields 0) := do
  let tx_dependentWitness0 := (tx).1
  let tx_dependentWitness1 := ((tx).2).1
  let tx := ((tx).2).2
  let pubkey_dependentWitness0 := (pubkey).1
  let pubkey_dependentWitness1 := ((pubkey).2).1
  let pubkey := ((pubkey).2).2
  let ⟨_, ⟨_, chain_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields chain_f.source.len)
  let ⟨_, ⟨_, nonce_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields nonce_f.source.len)
  let ⟨_, ⟨_, gp_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields gp_f.source.len)
  let ⟨_, ⟨_, gas_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields gas_f.source.len)
  let ⟨_, ⟨_, to_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields to_f.source.len)
  let ⟨_, ⟨_, value_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields value_f.source.len)
  let ⟨_, ⟨_, data_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields data_f.source.len)
  let ⟨_, ⟨_, al_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields al_f.source.len)
  let ⟨_, ⟨_, v_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields v_f.source.len)
  let ⟨_, ⟨_, r_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields r_f.source.len)
  let ⟨_, ⟨_, s_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields s_f.source.len)
  (rlp_cursor_expect_end fields)
  let v ← do (rlp_decode_word v_f)
  let gp ← do (rlp_decode_u256 gp_f)
  let access_list ← do (decode_access_list al_f)
  let recipient_word ← do (rlp_decode_word to_f)
  let recipient := (word_to_address recipient_word)
  let ⟨_, ⟨_, signing_span⟩⟩ := (tx_sig_span chain_f v_f)
  let signing_hash ← do (tx_signing_hash AccessListTx ⟨_, ⟨_, signing_span⟩⟩ v)
  (pure { tx_type := AccessListTx,
          sender := sender,
          raw := ⟨_, ⟨_, tx⟩⟩,
          nonce := ← (rlp_decode_u256 nonce_f),
          chain_id := ← (rlp_decode_uint64 chain_f),
          gas_limit := ← (rlp_decode_gas gas_f),
          is_create := (to_f.content_len == 0),
          recipient := recipient,
          value := ← (rlp_decode_u256 value_f),
          input_src := ← do
              let publicField ← (tx_input_span data_f)
              pure (publicField),
          access_list := access_list,
          max_fee := gp,
          max_blob_fee := ZERO_WORD,
          max_priority_fee := gp,
          authorizations := ⟨_, (EMPTY_AUTHORIZATION_LIST_REF).2⟩,
          blob_hashes := EMPTY_BLOB_HASHES,
          pubkey := ⟨_, ⟨_, pubkey⟩⟩,
          signing_hash := signing_hash,
          sig_v := v,
          sig_r := ← (rlp_decode_u256 r_f),
          sig_s := ← (rlp_decode_u256 s_f) })

/-- Decodes the payload fields of an EIP-1559 transaction. -/
/- Type quantifiers: pubkey_dependentWitness1 : Nat, pubkey_dependentWitness0 : Nat, tx_dependentWitness1
  : Nat, tx_dependentWitness0 : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ tx_dependentWitness0 ∧
  0 ≤ tx_dependentWitness1 ∧ (tx_dependentWitness0 + tx_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ tx_dependentWitness1 ∧ tx_dependentWitness1 ≤ (2 ^ 30), 0 ≤ pubkey_dependentWitness0
  ∧
  0 ≤ pubkey_dependentWitness1 ∧
  (pubkey_dependentWitness0 + pubkey_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def decode_fee_market_tx (tx : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (pubkey : (Sigma fun
  (tx_dependentWitness0 : Nat) =>
  (Sigma fun (tx_dependentWitness1 : Nat) =>
  (StatelessInputSliceFields tx_dependentWitness0 tx_dependentWitness1)))) (sender : (Vector (BitVec 8) 20)) (fields : (StatelessInputSliceFields k_source_off k_source_len)) : SailM (TransactionFields 0) := do
  let tx_dependentWitness0 := (tx).1
  let tx_dependentWitness1 := ((tx).2).1
  let tx := ((tx).2).2
  let pubkey_dependentWitness0 := (pubkey).1
  let pubkey_dependentWitness1 := ((pubkey).2).1
  let pubkey := ((pubkey).2).2
  let ⟨_, ⟨_, chain_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields chain_f.source.len)
  let ⟨_, ⟨_, nonce_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields nonce_f.source.len)
  let ⟨_, ⟨_, mp_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields mp_f.source.len)
  let ⟨_, ⟨_, mf_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields mf_f.source.len)
  let ⟨_, ⟨_, gas_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields gas_f.source.len)
  let ⟨_, ⟨_, to_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields to_f.source.len)
  let ⟨_, ⟨_, value_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields value_f.source.len)
  let ⟨_, ⟨_, data_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields data_f.source.len)
  let ⟨_, ⟨_, al_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields al_f.source.len)
  let ⟨_, ⟨_, v_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields v_f.source.len)
  let ⟨_, ⟨_, r_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields r_f.source.len)
  let ⟨_, ⟨_, s_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields s_f.source.len)
  (rlp_cursor_expect_end fields)
  let v ← do (rlp_decode_word v_f)
  let access_list ← do (decode_access_list al_f)
  let recipient_word ← do (rlp_decode_word to_f)
  let recipient := (word_to_address recipient_word)
  let ⟨_, ⟨_, signing_span⟩⟩ := (tx_sig_span chain_f v_f)
  let signing_hash ← do (tx_signing_hash FeeMarketTx ⟨_, ⟨_, signing_span⟩⟩ v)
  (pure { tx_type := FeeMarketTx,
          sender := sender,
          raw := ⟨_, ⟨_, tx⟩⟩,
          nonce := ← (rlp_decode_u256 nonce_f),
          chain_id := ← (rlp_decode_uint64 chain_f),
          gas_limit := ← (rlp_decode_gas gas_f),
          is_create := (to_f.content_len == 0),
          recipient := recipient,
          value := ← (rlp_decode_u256 value_f),
          input_src := ← do
              let publicField ← (tx_input_span data_f)
              pure (publicField),
          access_list := access_list,
          max_fee := ← (rlp_decode_u256 mf_f),
          max_blob_fee := ZERO_WORD,
          max_priority_fee := ← (rlp_decode_u256 mp_f),
          authorizations := ⟨_, (EMPTY_AUTHORIZATION_LIST_REF).2⟩,
          blob_hashes := EMPTY_BLOB_HASHES,
          pubkey := ⟨_, ⟨_, pubkey⟩⟩,
          signing_hash := signing_hash,
          sig_v := v,
          sig_r := ← (rlp_decode_u256 r_f),
          sig_s := ← (rlp_decode_u256 s_f) })

/-- Decodes the payload fields of an EIP-4844 transaction. -/
/- Type quantifiers: pubkey_dependentWitness1 : Nat, pubkey_dependentWitness0 : Nat, tx_dependentWitness1
  : Nat, tx_dependentWitness0 : Nat, k_source_off : Nat, k_source_len : Nat, blob_limit : Nat, (source_valid_range k_source_off k_source_len)
  ∧ (transaction_blob_limit_value blob_limit), 0 ≤ tx_dependentWitness0 ∧
  0 ≤ tx_dependentWitness1 ∧ (tx_dependentWitness0 + tx_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ tx_dependentWitness1 ∧ tx_dependentWitness1 ≤ (2 ^ 30), 0 ≤ pubkey_dependentWitness0
  ∧
  0 ≤ pubkey_dependentWitness1 ∧
  (pubkey_dependentWitness0 + pubkey_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def decode_blob_tx (tx : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (pubkey : (Sigma fun
  (tx_dependentWitness0 : Nat) =>
  (Sigma fun (tx_dependentWitness1 : Nat) =>
  (StatelessInputSliceFields tx_dependentWitness0 tx_dependentWitness1)))) (blob_limit : Nat) (sender : (Vector (BitVec 8) 20)) (fields : (StatelessInputSliceFields k_source_off k_source_len)) : SailM (TransactionFields blob_limit) := do
  let tx_dependentWitness0 := (tx).1
  let tx_dependentWitness1 := ((tx).2).1
  let tx := ((tx).2).2
  let pubkey_dependentWitness0 := (pubkey).1
  let pubkey_dependentWitness1 := ((pubkey).2).1
  let pubkey := ((pubkey).2).2
  let ⟨_, ⟨_, chain_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields chain_f.source.len)
  let ⟨_, ⟨_, nonce_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields nonce_f.source.len)
  let ⟨_, ⟨_, mp_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields mp_f.source.len)
  let ⟨_, ⟨_, mf_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields mf_f.source.len)
  let ⟨_, ⟨_, gas_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields gas_f.source.len)
  let ⟨_, ⟨_, to_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields to_f.source.len)
  let ⟨_, ⟨_, value_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields value_f.source.len)
  let ⟨_, ⟨_, data_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields data_f.source.len)
  let ⟨_, ⟨_, al_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields al_f.source.len)
  let ⟨_, ⟨_, mbf_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields mbf_f.source.len)
  let ⟨_, ⟨_, bh_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields bh_f.source.len)
  let ⟨_, ⟨_, v_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields v_f.source.len)
  let ⟨_, ⟨_, r_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields r_f.source.len)
  let ⟨_, ⟨_, s_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields s_f.source.len)
  (rlp_cursor_expect_end fields)
  let v ← do (rlp_decode_word v_f)
  let access_list ← do (decode_access_list al_f)
  let blob_hashes ← do (decode_blob_hashes bh_f blob_limit)
  let recipient_word ← do (rlp_decode_word to_f)
  let recipient := (word_to_address recipient_word)
  let ⟨_, ⟨_, signing_span⟩⟩ := (tx_sig_span chain_f v_f)
  let signing_hash ← do (tx_signing_hash BlobTx ⟨_, ⟨_, signing_span⟩⟩ v)
  (pure { tx_type := BlobTx,
          sender := sender,
          raw := ⟨_, ⟨_, tx⟩⟩,
          nonce := ← (rlp_decode_u256 nonce_f),
          chain_id := ← (rlp_decode_uint64 chain_f),
          gas_limit := ← (rlp_decode_gas gas_f),
          is_create := (to_f.content_len == 0),
          recipient := recipient,
          value := ← (rlp_decode_u256 value_f),
          input_src := ← do
              let publicField ← (tx_input_span data_f)
              pure (publicField),
          access_list := access_list,
          max_fee := ← (rlp_decode_u256 mf_f),
          max_blob_fee := ← (rlp_decode_u256 mbf_f),
          max_priority_fee := ← (rlp_decode_u256 mp_f),
          authorizations := ⟨_, (EMPTY_AUTHORIZATION_LIST_REF).2⟩,
          blob_hashes := blob_hashes,
          pubkey := ⟨_, ⟨_, pubkey⟩⟩,
          signing_hash := signing_hash,
          sig_v := v,
          sig_r := ← (rlp_decode_u256 r_f),
          sig_s := ← (rlp_decode_u256 s_f) })

/-- Decodes the payload fields of an EIP-7702 transaction. -/
/- Type quantifiers: pubkey_dependentWitness1 : Nat, pubkey_dependentWitness0 : Nat, tx_dependentWitness1
  : Nat, tx_dependentWitness0 : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ tx_dependentWitness0 ∧
  0 ≤ tx_dependentWitness1 ∧ (tx_dependentWitness0 + tx_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ tx_dependentWitness1 ∧ tx_dependentWitness1 ≤ (2 ^ 30), 0 ≤ pubkey_dependentWitness0
  ∧
  0 ≤ pubkey_dependentWitness1 ∧
  (pubkey_dependentWitness0 + pubkey_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def decode_set_code_tx (tx : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (pubkey : (Sigma fun
  (tx_dependentWitness0 : Nat) =>
  (Sigma fun (tx_dependentWitness1 : Nat) =>
  (StatelessInputSliceFields tx_dependentWitness0 tx_dependentWitness1)))) (sender : (Vector (BitVec 8) 20)) (fields : (StatelessInputSliceFields k_source_off k_source_len)) : SailM (TransactionFields 0) := do
  let tx_dependentWitness0 := (tx).1
  let tx_dependentWitness1 := ((tx).2).1
  let tx := ((tx).2).2
  let pubkey_dependentWitness0 := (pubkey).1
  let pubkey_dependentWitness1 := ((pubkey).2).1
  let pubkey := ((pubkey).2).2
  let ⟨_, ⟨_, chain_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields chain_f.source.len)
  let ⟨_, ⟨_, nonce_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields nonce_f.source.len)
  let ⟨_, ⟨_, mp_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields mp_f.source.len)
  let ⟨_, ⟨_, mf_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields mf_f.source.len)
  let ⟨_, ⟨_, gas_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields gas_f.source.len)
  let ⟨_, ⟨_, to_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields to_f.source.len)
  let ⟨_, ⟨_, value_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields value_f.source.len)
  let ⟨_, ⟨_, data_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields data_f.source.len)
  let ⟨_, ⟨_, al_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields al_f.source.len)
  let ⟨_, ⟨_, auth_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields auth_f.source.len)
  let ⟨_, ⟨_, v_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields v_f.source.len)
  let ⟨_, ⟨_, r_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields r_f.source.len)
  let ⟨_, ⟨_, s_f⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields s_f.source.len)
  (rlp_cursor_expect_end fields)
  let v ← do (rlp_decode_word v_f)
  let access_list ← do (decode_access_list al_f)
  let ⟨_, authorizations⟩ ← do (decode_auth_list auth_f)
  let decoded_nonce ← do (rlp_decode_uint64 nonce_f)
  let nonce := (word_of_account_nonce decoded_nonce)
  let recipient_word ← do (rlp_decode_word to_f)
  let recipient := (word_to_address recipient_word)
  let ⟨_, ⟨_, signing_span⟩⟩ := (tx_sig_span chain_f v_f)
  let signing_hash ← do (tx_signing_hash SetCodeTx ⟨_, ⟨_, signing_span⟩⟩ v)
  (pure { tx_type := SetCodeTx,
          sender := sender,
          raw := ⟨_, ⟨_, tx⟩⟩,
          nonce := nonce,
          chain_id := ← (rlp_decode_uint64 chain_f),
          gas_limit := ← (rlp_decode_gas gas_f),
          is_create := (to_f.content_len == 0),
          recipient := recipient,
          value := ← (rlp_decode_u256 value_f),
          input_src := ← do
              let publicField ← (tx_input_span data_f)
              pure (publicField),
          access_list := access_list,
          max_fee := ← (rlp_decode_u256 mf_f),
          max_blob_fee := ZERO_WORD,
          max_priority_fee := ← (rlp_decode_u256 mp_f),
          authorizations := ⟨_, authorizations⟩,
          blob_hashes := EMPTY_BLOB_HASHES,
          pubkey := ⟨_, ⟨_, pubkey⟩⟩,
          signing_hash := signing_hash,
          sig_v := v,
          sig_r := ← (rlp_decode_u256 r_f),
          sig_s := ← (rlp_decode_u256 s_f) })

/- Type quantifiers: k_tx_off : Nat, k_tx_len : Nat, k_public_key_off : Nat, blob_limit : Nat, (source_valid_range k_tx_off k_tx_len)
  ∧
  0 ≤ k_tx_len ∧
  k_tx_len ≤ transaction_length_bound ∧
  (source_valid_range k_public_key_off 65) ∧ (transaction_blob_limit_value blob_limit) -/
def rlp_decode_tx (tx : (StatelessInputSliceFields k_tx_off k_tx_len)) (pubkey : (StatelessInputSliceFields k_public_key_off 65)) (blob_limit : Nat) : SailM (Sigma
  fun (k_syn_blob_limit : Nat) => (TransactionFields k_syn_blob_limit)) := do
  let public_key_body := (stateless_input_sub_slice pubkey 1 PUBLIC_KEY_BODY_LENGTH)
  let public_key_hash ← do (stateless_input_keccak256 ⟨_, ⟨_, public_key_body⟩⟩)
  let public_key_word := (hash_to_word public_key_hash)
  let sender := (word_to_address public_key_word)
  let tx_length := k_tx_len
  let b0 ← (( do
    if ((tx_length == 0) : Bool)
    then (fatal_error RlpDecode)
    else (stateless_input_slice_byte ⟨_, ⟨_, tx⟩⟩ 0) ) : SailM (BitVec 8) )
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
          (pure ((⟨_, ⟨_, (stateless_input_sub_slice tx 1 (tx_length - 1))⟩⟩ : (Sigma fun
            (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma
            fun (k_off : Nat) =>
            (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))))
        else
          (do
            (fatal_error RlpDecode)))
    else
      (pure ((⟨_, ⟨_, tx⟩⟩ : (Sigma fun (k_off : Nat) =>
        (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma fun
        (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))))) ) :
    SailM
    (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))
    )
  let payload_input : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) :=
    ((⟨_, ⟨_, payload⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma fun
    (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))))
  let ⟨_, ⟨_, fields⟩⟩ ← do (rlp_node_cursor payload_input)
  let tx_type ← (( do
    match ttype with
    | 0x00 => (pure LegacyTx)
    | 0x01 => (pure AccessListTx)
    | 0x02 => (pure FeeMarketTx)
    | 0x03 => (pure BlobTx)
    | 0x04 => (pure SetCodeTx)
    | _ => (fatal_error RlpDecode) ) : SailM TxType )
  match tx_type with
  | .LegacyTx =>
    (do
      let decoded ← do
        (decode_legacy_tx ⟨_, ⟨_, tx⟩⟩ ⟨_, ⟨_, pubkey⟩⟩ sender fields)
      (pure ((pack_transaction decoded) : (Sigma fun (k_syn_blob_limit : Nat) =>
        (TransactionFields k_syn_blob_limit)))))
  | .AccessListTx =>
    (do
      let decoded ← do
        (decode_access_list_tx ⟨_, ⟨_, tx⟩⟩ ⟨_, ⟨_, pubkey⟩⟩ sender fields)
      (pure ((pack_transaction decoded) : (Sigma fun (k_syn_blob_limit : Nat) =>
        (TransactionFields k_syn_blob_limit)))))
  | .FeeMarketTx =>
    (do
      let decoded ← do
        (decode_fee_market_tx ⟨_, ⟨_, tx⟩⟩ ⟨_, ⟨_, pubkey⟩⟩ sender fields)
      (pure ((pack_transaction decoded) : (Sigma fun (k_syn_blob_limit : Nat) =>
        (TransactionFields k_syn_blob_limit)))))
  | .BlobTx =>
    (do
      let decoded ← do
        (decode_blob_tx ⟨_, ⟨_, tx⟩⟩ ⟨_, ⟨_, pubkey⟩⟩ blob_limit sender fields)
      (pure ((pack_transaction decoded) : (Sigma fun (k_syn_blob_limit : Nat) =>
        (TransactionFields k_syn_blob_limit)))))
  | .SetCodeTx =>
    (do
      let decoded ← do
        (decode_set_code_tx ⟨_, ⟨_, tx⟩⟩ ⟨_, ⟨_, pubkey⟩⟩ sender fields)
      (pure ((pack_transaction decoded) : (Sigma fun (k_syn_blob_limit : Nat) =>
        (TransactionFields k_syn_blob_limit)))))

