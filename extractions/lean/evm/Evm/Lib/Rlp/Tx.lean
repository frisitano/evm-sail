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

/- Type quantifiers: value : Int, source_valid_length((value + 1)) -/
def rlp_item_count_increment (value : Int) : Nat :=
  (value +i 1)

/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, k_current : Nat, k_stop
  : Nat, k_address_bound : Nat, k_slot_bound : Nat, source_valid_range(k_source_off, k_source_len)
  ∧
  0 ≤ k_current ∧
  k_current ≤ k_stop ∧
  k_stop ≤ k_source_len ∧
  source_valid_length(k_address_bound) ∧
  source_valid_length(k_slot_bound) ∧ source_valid_length((k_slot_bound + (k_stop - k_current))), 0
  ≤ _reclimit -/
def _rec_decode_access_list_keys (cursor : (RlpCursorFields k_source_off k_source_len k_current k_stop)) (addr : address) (tail : (AccessListDecode k_address_bound k_slot_bound)) (_reclimit : Nat) : SailM (AccessListDecode k_address_bound (k_slot_bound + (k_stop - k_current))) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((rlp_cursor_empty ⟨_, ⟨_, ⟨_, ⟨_, cursor⟩⟩⟩⟩) : Bool)
      then
        (pure { addresses := tail.addresses,
                storage_slots := tail.storage_slots,
                address_count := tail.address_count,
                slot_count := tail.slot_count })
      else
        (do
          let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (key, next)⟩⟩⟩⟩ ← do
            (rlp_cursor_pop cursor)
          let storage_key ← (( do
            (pure { addr := addr,
                    slot := ← do
                        let publicField ← (do
                            let publicResult ← (rlp_ref_word
                            ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, key⟩⟩⟩⟩⟩⟩)
                            pure ((publicResult).value))
                        pure (⟨publicField⟩) }) ) : SailM StorageKey )
          let result ← do (_rec_decode_access_list_keys next addr tail _reclimit_pred)
          (pure { addresses := result.addresses,
                  storage_slots := (storage_key :: result.storage_slots),
                  address_count := result.address_count,
                  slot_count := (rlp_item_count_increment result.slot_count) })))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_current : Nat, k_stop : Nat, k_address_bound
  : Nat, k_slot_bound : Nat, source_valid_range(k_source_off, k_source_len) ∧
  0 ≤ k_current ∧
  k_current ≤ k_stop ∧
  k_stop ≤ k_source_len ∧
  source_valid_length(k_address_bound) ∧
  source_valid_length(k_slot_bound) ∧ source_valid_length((k_slot_bound + (k_stop - k_current))) -/
def decode_access_list_keys (cursor : (RlpCursorFields k_source_off k_source_len k_current k_stop)) (addr : address) (tail : (AccessListDecode k_address_bound k_slot_bound)) : SailM (AccessListDecode k_address_bound (k_slot_bound + (k_stop - k_current))) := do
  let _measure :=
    (let stop := k_stop
    let current := k_current
    (stop - current) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_decode_access_list_keys cursor addr tail (_measure + 1))

/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, k_current : Nat, k_stop
  : Nat, source_valid_range(k_source_off, k_source_len) ∧
  0 ≤ k_current ∧ k_current ≤ k_stop ∧ k_stop ≤ k_source_len, 0 ≤ _reclimit -/
def _rec_decode_access_list_entries (cursor : (RlpCursorFields k_source_off k_source_len k_current k_stop)) (_reclimit : Nat) : SailM (AccessListDecode (k_stop - k_current) (k_stop - k_current)) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((rlp_cursor_empty ⟨_, ⟨_, ⟨_, ⟨_, cursor⟩⟩⟩⟩) : Bool)
      then
        (pure { addresses := [],
                storage_slots := [],
                address_count := 0,
                slot_count := 0 })
      else
        (do
          let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (entry, next)⟩⟩⟩⟩ ← do
            (rlp_cursor_pop cursor)
          let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, ⟨k_ex415742_, ⟨k_ex415741_, ⟨k_ex415740_, ⟨k_ex415739_, (addr_f, entry_fields)⟩⟩⟩⟩⟩⟩⟩⟩ ← do
            (do
                let ⟨_, ⟨_, ⟨_, ⟨_, dependentArg0⟩⟩⟩⟩ ← (rlp_ref_cursor entry)
                let publicResult ← (rlp_cursor_pop dependentArg0)
                pure ((⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ((((publicResult).2).2).2).2⟩⟩⟩⟩⟩⟩⟩⟩ : (Sigma
                fun (k_ex415739_ : Nat) =>
                (Sigma fun (k_ex415740_ : Nat) =>
                (Sigma fun (k_ex415741_ : Nat) =>
                (Sigma fun (k_ex415742_ : Nat) =>
                (Sigma fun (k_content : Nat) =>
                (Sigma fun (k_content_len : Nat) =>
                (Sigma fun (k_next : Nat) =>
                (Sigma fun (k_full_len : Nat) =>
                ((RlpFieldRefFields k_ex415739_ k_ex415740_ k_ex415741_ k_full_len k_content k_content_len) × (RlpCursorFields k_ex415739_ k_ex415740_ k_next k_ex415742_)))))))))))))
          let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (keys_f, entry_fields)⟩⟩⟩⟩ ← do
            (rlp_cursor_pop entry_fields)
          (rlp_cursor_expect_end ⟨_, ⟨_, ⟨_, ⟨_, entry_fields⟩⟩⟩⟩)
          let addr ← do
            (pure (word_to_address
                ⟨((← (rlp_ref_word ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, addr_f⟩⟩⟩⟩⟩⟩))).value⟩))
          let tail ← do (_rec_decode_access_list_entries next _reclimit_pred)
          let ⟨_, ⟨_, result⟩⟩ ← do
            (do
                let ⟨_, ⟨_, ⟨_, ⟨_, dependentArg0⟩⟩⟩⟩ ← (rlp_ref_cursor keys_f)
                let publicResult ← (decode_access_list_keys dependentArg0 addr tail)
                pure ((⟨_, ⟨_, publicResult⟩⟩ : (Sigma fun (_ : Nat) =>
                (Sigma fun (_ : Nat) => _)))))
          (pure { addresses := (addr :: result.addresses),
                  storage_slots := result.storage_slots,
                  address_count := (rlp_item_count_increment result.address_count),
                  slot_count := result.slot_count })))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_current : Nat, k_stop : Nat, source_valid_range(k_source_off, k_source_len)
  ∧ 0 ≤ k_current ∧ k_current ≤ k_stop ∧ k_stop ≤ k_source_len -/
def decode_access_list_entries (cursor : (RlpCursorFields k_source_off k_source_len k_current k_stop)) : SailM (AccessListDecode (k_stop - k_current) (k_stop - k_current)) := do
  let _measure :=
    (let stop := k_stop
    let current := k_current
    (stop - current) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_decode_access_list_entries cursor (_measure + 1))

/-- Decodes an EIP-2930 access list — RLP `[[address, [slot, …]], …]` —
into the flat representation the transaction and intrinsic-gas
accounting use: the addresses (one per entry) and the
`(address, slot)` pairs. -/
/- Type quantifiers: k_ex410121_ : Nat, k_ex410120_ : Nat, k_ex410119_ : Nat, k_ex410118_ : Nat, k_ex410117_
  : Nat, k_ex410116_ : Nat, 0 ≤ k_ex410116_ ∧ 0 ≤ k_ex410117_ ∧
  0 ≤ k_ex410118_ ∧
  0 ≤ k_ex410119_ ∧
  (k_ex410118_ + k_ex410119_) ≤ k_ex410117_ ∧
  0 ≤ k_ex410120_ ∧ 0 ≤ k_ex410121_ ∧ (k_ex410120_ + k_ex410121_) ≤ k_ex410117_ -/
def decode_access_list (f : RlpFieldRef) : SailM ((List address) × (List StorageKey) × transaction_item_count × transaction_item_count) := do
  let f := ((((((f).2).2).2).2).2).2
  let publicResult ← do
    if ((f.source.len ≤b (2 ^i 30)) : Bool)
    then
      (do
        let ⟨_, ⟨_, decoded⟩⟩ ← do
          (do
              let ⟨_, ⟨_, ⟨_, ⟨_, dependentArg0⟩⟩⟩⟩ ← (rlp_ref_cursor f)
              let publicResult ← (decode_access_list_entries dependentArg0)
              pure ((⟨_, ⟨_, publicResult⟩⟩ : (Sigma fun (k_ex415991_ : Nat) =>
              (Sigma fun (k_ex415992_ : Nat) =>
              (AccessListDecode (k_ex415992_ - k_ex415991_) (k_ex415992_ - k_ex415991_)))))))
        (pure (decoded.addresses, decoded.storage_slots, decoded.address_count, decoded.slot_count)))
    else sailThrow ((InvalidBlock RlpDecode))
  pure (((fun (semanticValue0, semanticValue1, semanticValue2, semanticValue3) => (semanticValue0, semanticValue1, ⟨semanticValue2⟩, ⟨semanticValue3⟩)) (publicResult)))

def BLOB_HASH_RLP_LENGTH : Nat := 33

def BLOB_HASH_LENGTH : Nat := WORD_BYTE_LENGTH

/-- Validates every fixed-width versioned-hash item and returns their count. -/
/- Type quantifiers: _reclimit : Nat, k_ex410130_ : Nat, k_ex410129_ : Nat, k_ex410128_ : Nat, k_ex410127_
  : Nat, k_ex410126_ : Nat, 0 ≤ k_ex410126_ ∧ 0 ≤ k_ex410127_ ∧
  0 ≤ k_ex410128_ ∧ k_ex410128_ ≤ k_ex410129_ ∧ k_ex410129_ ≤ k_ex410127_, 0 ≤
  k_ex410130_ ∧ k_ex410130_ ≤ 9, 0 ≤ _reclimit -/
def _rec_decode_blob_hash_items (cursor : RlpCursor) (count : transaction_blob_count) (_reclimit : Nat) : SailM transaction_blob_count := do
  let cursor := ((((cursor).2).2).2).2
  let count := (count).value
  let publicResult ← do
    match _reclimit with
    | 0 =>
      (do
        assert false "recursion limit reached"
        throw Error.Exit)
    | _reclimit_pred + 1 =>
      (do
        if ((rlp_cursor_empty ⟨_, ⟨_, ⟨_, ⟨_, cursor⟩⟩⟩⟩) : Bool)
        then (pure count)
        else
          (do
            let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (item, next)⟩⟩⟩⟩ ← do
              (rlp_cursor_pop cursor)
            if ((item.is_list || ((item.full_len != BLOB_HASH_RLP_LENGTH) || ((item.content_len != BLOB_HASH_LENGTH) || ((← (slice_byte
                           ⟨_, ⟨_, item.source⟩⟩ item.full_off)) != 0xA0#8)))) : Bool)
            then sailThrow ((InvalidBlock RlpDecode))
            else (pure ())
            if ((count <b 9) : Bool)
            then
              (do
                  let publicResult ← (_rec_decode_blob_hash_items
                  ⟨_, ⟨_, ⟨_, ⟨_, next⟩⟩⟩⟩ ⟨(count + 1)⟩ _reclimit_pred)
                  pure ((publicResult).value))
            else sailThrow ((InvalidBlock RlpDecode))))
  pure (⟨publicResult⟩)
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Validates every fixed-width versioned-hash item and returns their count. -/
/- Type quantifiers: count : Nat, k_ex410139_ : Nat, k_ex410138_ : Nat, k_ex410137_ : Nat, k_ex410136_
  : Nat, 0 ≤ k_ex410136_ ∧ 0 ≤ k_ex410137_ ∧
  0 ≤ k_ex410138_ ∧ k_ex410138_ ≤ k_ex410139_ ∧ k_ex410139_ ≤ k_ex410137_, 0 ≤ count ∧
  count ≤ 9 -/
def decode_blob_hash_items (cursor : RlpCursor) (count : transaction_blob_count) : SailM transaction_blob_count := do
  let cursor := ((((cursor).2).2).2).2
  let count := (count).value
  let publicResult ← do
    let _measure :=
      (let stop := cursor.stop
      let current := cursor.current
      (stop - current) : Int)
    if ((_measure <b 0) : Bool)
    then throw Error.Exit
    else
      (do
          let publicResult ← (_rec_decode_blob_hash_items
          ⟨_, ⟨_, ⟨_, ⟨_, cursor⟩⟩⟩⟩ ⟨count⟩ (_measure + 1))
          pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Validates the canonical RLP list of `bytes32` blob versioned hashes
once, then retains its encoded content as a fixed-stride source view:
each element is exactly `0xa0` followed by 32 bytes, so `BLOBHASH` can
load item `i` at `33·i + 1`. -/
/- Type quantifiers: k_ex410153_ : Nat, k_ex410152_ : Nat, k_ex410151_ : Nat, k_ex410150_ : Nat, k_ex410149_
  : Nat, k_ex410148_ : Nat, 0 ≤ k_ex410148_ ∧ 0 ≤ k_ex410149_ ∧
  0 ≤ k_ex410150_ ∧
  0 ≤ k_ex410151_ ∧
  (k_ex410150_ + k_ex410151_) ≤ k_ex410149_ ∧
  0 ≤ k_ex410152_ ∧ 0 ≤ k_ex410153_ ∧ (k_ex410152_ + k_ex410153_) ≤ k_ex410149_ -/
def decode_blob_hashes (f : RlpFieldRef) : SailM BlobHashes := do
  let f := ((((((f).2).2).2).2).2).2
  let bytes := (sub_slice f.source f.content_off f.content_len)
  (pure { bytes := ⟨_, ⟨_, bytes⟩⟩,
          count := ← do
              let publicField ← (do
                  let publicResult ← (decode_blob_hash_items
                  (⟨_, ⟨_, ⟨_, ⟨_, (((((← (rlp_ref_cursor f))).2).2).2).2⟩⟩⟩⟩ : (Sigma
                  fun (k_source_off : Nat) =>
                  (Sigma fun (k_source_len : Nat) =>
                  (Sigma fun (k_current : Nat) =>
                  (Sigma fun (k_stop : Nat) =>
                  (RlpCursorFields k_source_off k_source_len k_current k_stop)))))) ⟨0⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩) })

def EMPTY_AUTHORIZATION_DECODE : (AuthorizationDecode 0) :=
  { authorizations := [],
    count := 0 }

/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, k_current : Nat, k_stop
  : Nat, source_valid_range(k_source_off, k_source_len) ∧
  0 ≤ k_current ∧ k_current ≤ k_stop ∧ k_stop ≤ k_source_len, 0 ≤ _reclimit -/
def _rec_decode_auth_tuples (cursor : (RlpCursorFields k_source_off k_source_len k_current k_stop)) (_reclimit : Nat) : SailM (AuthorizationDecode (k_stop - k_current)) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((rlp_cursor_empty ⟨_, ⟨_, ⟨_, ⟨_, cursor⟩⟩⟩⟩) : Bool)
      then
        (pure { authorizations := [],
                count := 0 })
      else
        (do
          let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (tuple, next)⟩⟩⟩⟩ ← do
            (rlp_cursor_pop cursor)
          let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, ⟨k_ex416475_, ⟨k_ex416474_, ⟨k_ex416473_, ⟨k_ex416472_, (chain_f, fields)⟩⟩⟩⟩⟩⟩⟩⟩ ← do
            (do
                let ⟨_, ⟨_, ⟨_, ⟨_, dependentArg0⟩⟩⟩⟩ ← (rlp_ref_cursor tuple)
                let publicResult ← (rlp_cursor_pop dependentArg0)
                pure ((⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ((((publicResult).2).2).2).2⟩⟩⟩⟩⟩⟩⟩⟩ : (Sigma
                fun (k_ex416472_ : Nat) =>
                (Sigma fun (k_ex416473_ : Nat) =>
                (Sigma fun (k_ex416474_ : Nat) =>
                (Sigma fun (k_ex416475_ : Nat) =>
                (Sigma fun (k_content : Nat) =>
                (Sigma fun (k_content_len : Nat) =>
                (Sigma fun (k_next : Nat) =>
                (Sigma fun (k_full_len : Nat) =>
                ((RlpFieldRefFields k_ex416472_ k_ex416473_ k_ex416474_ k_full_len k_content k_content_len) × (RlpCursorFields k_ex416472_ k_ex416473_ k_next k_ex416475_)))))))))))))
          let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (addr_f, fields)⟩⟩⟩⟩ ← do
            (rlp_cursor_pop fields)
          let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (nonce_f, fields)⟩⟩⟩⟩ ← do
            (rlp_cursor_pop fields)
          let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (y_f, fields)⟩⟩⟩⟩ ← do
            (rlp_cursor_pop fields)
          let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (r_f, fields)⟩⟩⟩⟩ ← do
            (rlp_cursor_pop fields)
          let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (s_f, fields)⟩⟩⟩⟩ ← do
            (rlp_cursor_pop fields)
          (rlp_cursor_expect_end ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩)
          let chain_id ← do
            (do
                let publicResult ← (rlp_ref_uint_word
                ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, chain_f⟩⟩⟩⟩⟩⟩)
                pure ((publicResult).value))
          let auth_nonce ← do
            (do
                let publicResult ← (rlp_ref_account_nonce
                ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, nonce_f⟩⟩⟩⟩⟩⟩)
                pure ((publicResult).value))
          let y_value ← do
            (do
                let publicResult ← (rlp_ref_uint64
                ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, y_f⟩⟩⟩⟩⟩⟩)
                pure ((publicResult).value))
          let y_valid := (y_value ≤b 1)
          let y : Nat :=
            if ((y_value == 0) : Bool)
            then 0
            else 1
          let r ← do
            (do
                let publicResult ← (rlp_ref_uint_word
                ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, r_f⟩⟩⟩⟩⟩⟩)
                pure ((publicResult).value))
          let s ← do
            (do
                let publicResult ← (rlp_ref_uint_word
                ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, s_f⟩⟩⟩⟩⟩⟩)
                pure ((publicResult).value))
          let auth_addr ← do
            (pure (word_to_address
                ⟨((← (rlp_ref_word ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, addr_f⟩⟩⟩⟩⟩⟩))).value⟩))
          let (ok, authority) ← do
            if (y_valid : Bool)
            then
              (ecrecover_addr (← (auth_signing_hash ⟨chain_id⟩ auth_addr ⟨auth_nonce⟩))
                ⟨y⟩ ⟨r⟩ ⟨s⟩)
            else (pure (false, ZERO_ADDRESS))
          let nonce_valid := (auth_nonce != ((2 ^i 64) - 1))
          let authorization : Authorization :=
            { valid_sig := (ok && (y_valid && ((word_ult (ZERO_WORD).value r) && ((word_ult r
                          (SECP_N_FULL).value) && ((word_ult (ZERO_WORD).value s) && ((word_ule s
                              (SECP_N_HALF).value) && nonce_valid)))))),
              authority := authority,
              address := auth_addr,
              nonce := ⟨auth_nonce⟩,
              chain_id := ⟨chain_id⟩ }
          let tail ← do (_rec_decode_auth_tuples next _reclimit_pred)
          (pure { authorizations := (authorization :: tail.authorizations),
                  count := (rlp_item_count_increment tail.count) })))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_current : Nat, k_stop : Nat, source_valid_range(k_source_off, k_source_len)
  ∧ 0 ≤ k_current ∧ k_current ≤ k_stop ∧ k_stop ≤ k_source_len -/
def decode_auth_tuples (cursor : (RlpCursorFields k_source_off k_source_len k_current k_stop)) : SailM (AuthorizationDecode (k_stop - k_current)) := do
  let _measure :=
    (let stop := k_stop
    let current := k_current
    (stop - current) : Int)
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
/- Type quantifiers: k_ex410201_ : Nat, k_ex410200_ : Nat, k_ex410199_ : Nat, k_ex410198_ : Nat, k_ex410197_
  : Nat, k_ex410196_ : Nat, 0 ≤ k_ex410196_ ∧ 0 ≤ k_ex410197_ ∧
  0 ≤ k_ex410198_ ∧
  0 ≤ k_ex410199_ ∧
  (k_ex410198_ + k_ex410199_) ≤ k_ex410197_ ∧
  0 ≤ k_ex410200_ ∧ 0 ≤ k_ex410201_ ∧ (k_ex410200_ + k_ex410201_) ≤ k_ex410197_ -/
def decode_auth_list (f : RlpFieldRef) : SailM ((List Authorization) × transaction_item_count) := do
  let f := ((((((f).2).2).2).2).2).2
  let publicResult ← do
    if ((f.source.len ≤b (2 ^i 30)) : Bool)
    then
      (do
        let ⟨_, ⟨_, decoded⟩⟩ ← do
          (do
              let ⟨_, ⟨_, ⟨_, ⟨_, dependentArg0⟩⟩⟩⟩ ← (rlp_ref_cursor f)
              let publicResult ← (decode_auth_tuples dependentArg0)
              pure ((⟨_, ⟨_, publicResult⟩⟩ : (Sigma fun (k_ex417017_ : Nat) =>
              (Sigma fun (k_ex417018_ : Nat) => (AuthorizationDecode (k_ex417018_ - k_ex417017_)))))))
        (pure (decoded.authorizations, decoded.count)))
    else sailThrow ((InvalidBlock RlpDecode))
  pure (((fun (semanticValue0, semanticValue1) => (semanticValue0, ⟨semanticValue1⟩)) (publicResult)))

/-- The calldata/initcode span of the data field within the envelope. The RLP
cursor existentially hides its bounded source length, so this boundary
reifies the already-established transaction-envelope invariant. -/
/- Type quantifiers: k_ex410213_ : Nat, k_ex410212_ : Nat, k_ex410211_ : Nat, k_ex410210_ : Nat, k_ex410209_
  : Nat, k_ex410208_ : Nat, 0 ≤ k_ex410208_ ∧ 0 ≤ k_ex410209_ ∧
  0 ≤ k_ex410210_ ∧
  0 ≤ k_ex410211_ ∧
  (k_ex410210_ + k_ex410211_) ≤ k_ex410209_ ∧
  0 ≤ k_ex410212_ ∧ 0 ≤ k_ex410213_ ∧ (k_ex410212_ + k_ex410213_) ≤ k_ex410209_ -/
def tx_input_span (data : RlpFieldRef) : SailM TransactionEvmByteSlice := do
  let data := ((((((data).2).2).2).2).2).2
  let content := (rlp_ref_content data)
  if ((content.len ≤b (2 ^i 30)) : Bool)
  then
    (pure ((⟨_, ⟨_, content⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  else
    (do
      sailThrow ((InvalidBlock RlpDecode)))

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_first_full_off : Nat, k_first_full_len
  : Nat, k_first_content_off : Nat, k_first_content_len : Nat, k_signature_full_off : Nat, k_signature_full_len
  : Nat, k_signature_content_off : Nat, k_signature_content_len : Nat, source_valid_range(k_source_off, k_source_len)
  ∧
  0 ≤ k_first_full_off ∧
  0 ≤ k_first_full_len ∧
  (k_first_full_off + k_first_full_len) ≤ k_source_len ∧
  0 ≤ k_first_content_off ∧
  0 ≤ k_first_content_len ∧
  (k_first_content_off + k_first_content_len) ≤ k_source_len ∧
  0 ≤ k_signature_full_off ∧
  0 ≤ k_signature_full_len ∧
  (k_signature_full_off + k_signature_full_len) ≤ k_source_len ∧
  0 ≤ k_signature_content_off ∧
  0 ≤ k_signature_content_len ∧
  (k_signature_content_off + k_signature_content_len) ≤ k_source_len -/
def tx_sig_span (first : (RlpFieldRefFields k_source_off k_source_len k_first_full_off k_first_full_len k_first_content_off k_first_content_len)) (signature : (RlpFieldRefFields k_source_off k_source_len k_signature_full_off k_signature_full_len k_signature_content_off k_signature_content_len)) : SailM EvmByteSlice := do
  let start := k_first_full_off
  let stop := k_signature_full_off
  let start_offset := start
  let stop_offset := stop
  if ((stop_offset <b start_offset) : Bool)
  then
    (do
      sailThrow ((InvalidBlock RlpDecode)))
  else
    (pure ((⟨_, ⟨_, (sub_slice first.source start (stop_offset - start_offset))⟩⟩ : (Sigma
      fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
      (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))

/-- Decodes transaction gas and enforces the protocol and fork-specific
transaction bounds. -/
/- Type quantifiers: k_ex410239_ : Nat, k_ex410238_ : Nat, k_ex410237_ : Nat, k_ex410236_ : Nat, k_ex410235_
  : Nat, k_ex410234_ : Nat, 0 ≤ k_ex410234_ ∧ 0 ≤ k_ex410235_ ∧
  0 ≤ k_ex410236_ ∧
  0 ≤ k_ex410237_ ∧
  (k_ex410236_ + k_ex410237_) ≤ k_ex410235_ ∧
  0 ≤ k_ex410238_ ∧ 0 ≤ k_ex410239_ ∧ (k_ex410238_ + k_ex410239_) ≤ k_ex410235_ -/
def rlp_ref_gas (f : RlpFieldRef) (fork : Fork) : SailM transaction_gas := do
  let f := ((((((f).2).2).2).2).2).2
  let value ← do (rlp_ref_uint ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, f⟩⟩⟩⟩⟩⟩)
  if (((fork_gteq fork Osaka) && ((fork_lt fork Amsterdam) && (OSAKA_TRANSACTION_GAS_LIMIT_VALUE <b value))) : Bool)
  then sailThrow ((InvalidBlock GasUsedExceedsLimit))
  else (pure value)

/-- Decodes the payload fields of a legacy transaction. Kept separate from
the envelope dispatcher so extraction backends compile each transaction
shape as an independent function. -/
/- Type quantifiers: k_ex410255_ : Nat, k_ex410254_ : Nat, k_ex410253_ : Nat, k_ex410252_ : Nat, k_ex410247_
  : Nat, k_ex410246_ : Nat, k_ex410243_ : Nat, k_ex410242_ : Nat, 0 ≤ k_ex410242_ ∧
  0 ≤ k_ex410243_ ∧ 0 ≤ k_ex410243_ ∧ k_ex410243_ ≤ (2 ^ 30), 0 ≤ k_ex410246_ ∧
  0 ≤ k_ex410247_, 0 ≤ k_ex410252_ ∧ 0 ≤ k_ex410253_ ∧
  0 ≤ k_ex410254_ ∧ k_ex410254_ ≤ k_ex410255_ ∧ k_ex410255_ ≤ k_ex410253_ -/
def decode_legacy_tx (tx : TransactionEvmByteSlice) (pubkey : EvmByteSlice) (fork : Fork) (sender : address) (fields : RlpCursor) : SailM Transaction := do
  let tx := ((tx).2).2
  let pubkey := ((pubkey).2).2
  let fields := ((((fields).2).2).2).2
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (nonce_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (gp_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (gas_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (to_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (value_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (data_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (v_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (r_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (s_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  (rlp_cursor_expect_end ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩)
  let v ← do
    (do
        let publicResult ← (rlp_ref_word ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, v_f⟩⟩⟩⟩⟩⟩)
        pure ((publicResult).value))
  let gp ← do
    (do
        let publicResult ← (rlp_ref_uint_word
        ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, gp_f⟩⟩⟩⟩⟩⟩)
        pure ((publicResult).value))
  (pure { tx_type := LegacyTx,
          sender := sender,
          raw := ⟨_, ⟨_, tx⟩⟩,
          nonce := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, nonce_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          chain_id := 0,
          gas_limit := ← (rlp_ref_gas ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, gas_f⟩⟩⟩⟩⟩⟩
              fork),
          is_create := (to_f.content_len == 0),
          recipient := ← (pure (word_to_address
                ⟨((← (rlp_ref_word ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, to_f⟩⟩⟩⟩⟩⟩))).value⟩)),
          value := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, value_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          input_src := ← do
              let publicField ← (tx_input_span
                ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, data_f⟩⟩⟩⟩⟩⟩)
              pure (publicField),
          access_list_addresses := [],
          access_list_address_count := ⟨0⟩,
          access_list_slots := [],
          access_list_slot_count := ⟨0⟩,
          max_fee := ⟨gp⟩,
          max_blob_fee := ⟨(ZERO_WORD).value⟩,
          max_priority_fee := ⟨gp⟩,
          authorizations := [],
          authorization_count := ⟨0⟩,
          blob_hashes := EMPTY_BLOB_HASHES,
          pubkey := ⟨_, ⟨_, pubkey⟩⟩,
          signing_hash := ← (tx_signing_hash LegacyTx (← (tx_sig_span nonce_f v_f)) ⟨v⟩),
          sig_v := ⟨v⟩,
          sig_r := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, r_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          sig_s := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, s_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩) })

/-- Decodes the payload fields of an EIP-2930 transaction. -/
/- Type quantifiers: k_ex410271_ : Nat, k_ex410270_ : Nat, k_ex410269_ : Nat, k_ex410268_ : Nat, k_ex410263_
  : Nat, k_ex410262_ : Nat, k_ex410259_ : Nat, k_ex410258_ : Nat, 0 ≤ k_ex410258_ ∧
  0 ≤ k_ex410259_ ∧ 0 ≤ k_ex410259_ ∧ k_ex410259_ ≤ (2 ^ 30), 0 ≤ k_ex410262_ ∧
  0 ≤ k_ex410263_, 0 ≤ k_ex410268_ ∧ 0 ≤ k_ex410269_ ∧
  0 ≤ k_ex410270_ ∧ k_ex410270_ ≤ k_ex410271_ ∧ k_ex410271_ ≤ k_ex410269_ -/
def decode_access_list_tx (tx : TransactionEvmByteSlice) (pubkey : EvmByteSlice) (fork : Fork) (sender : address) (fields : RlpCursor) : SailM Transaction := do
  let tx := ((tx).2).2
  let pubkey := ((pubkey).2).2
  let fields := ((((fields).2).2).2).2
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (chain_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (nonce_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (gp_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (gas_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (to_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (value_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (data_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (al_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (v_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (r_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (s_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  (rlp_cursor_expect_end ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩)
  let v ← do
    (do
        let publicResult ← (rlp_ref_word ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, v_f⟩⟩⟩⟩⟩⟩)
        pure ((publicResult).value))
  let gp ← do
    (do
        let publicResult ← (rlp_ref_uint_word
        ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, gp_f⟩⟩⟩⟩⟩⟩)
        pure ((publicResult).value))
  let (al_addrs, al_slots, al_addr_count, al_slot_count) ← do
    (do
        let publicResult ← (decode_access_list
        ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, al_f⟩⟩⟩⟩⟩⟩)
        pure (((fun (semanticValue0, semanticValue1, semanticValue2, semanticValue3) => (semanticValue0, semanticValue1, (semanticValue2).value, (semanticValue3).value)) (publicResult))))
  (pure { tx_type := AccessListTx,
          sender := sender,
          raw := ⟨_, ⟨_, tx⟩⟩,
          nonce := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, nonce_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          chain_id := ← (rlp_ref_chain_identifier
              ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, chain_f⟩⟩⟩⟩⟩⟩),
          gas_limit := ← (rlp_ref_gas ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, gas_f⟩⟩⟩⟩⟩⟩
              fork),
          is_create := (to_f.content_len == 0),
          recipient := ← (pure (word_to_address
                ⟨((← (rlp_ref_word ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, to_f⟩⟩⟩⟩⟩⟩))).value⟩)),
          value := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, value_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          input_src := ← do
              let publicField ← (tx_input_span
                ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, data_f⟩⟩⟩⟩⟩⟩)
              pure (publicField),
          access_list_addresses := al_addrs,
          access_list_address_count := ⟨al_addr_count⟩,
          access_list_slots := al_slots,
          access_list_slot_count := ⟨al_slot_count⟩,
          max_fee := ⟨gp⟩,
          max_blob_fee := ⟨(ZERO_WORD).value⟩,
          max_priority_fee := ⟨gp⟩,
          authorizations := [],
          authorization_count := ⟨0⟩,
          blob_hashes := EMPTY_BLOB_HASHES,
          pubkey := ⟨_, ⟨_, pubkey⟩⟩,
          signing_hash := ← (tx_signing_hash AccessListTx (← (tx_sig_span chain_f v_f)) ⟨v⟩),
          sig_v := ⟨v⟩,
          sig_r := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, r_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          sig_s := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, s_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩) })

/-- Decodes the payload fields of an EIP-1559 transaction. -/
/- Type quantifiers: k_ex410287_ : Nat, k_ex410286_ : Nat, k_ex410285_ : Nat, k_ex410284_ : Nat, k_ex410279_
  : Nat, k_ex410278_ : Nat, k_ex410275_ : Nat, k_ex410274_ : Nat, 0 ≤ k_ex410274_ ∧
  0 ≤ k_ex410275_ ∧ 0 ≤ k_ex410275_ ∧ k_ex410275_ ≤ (2 ^ 30), 0 ≤ k_ex410278_ ∧
  0 ≤ k_ex410279_, 0 ≤ k_ex410284_ ∧ 0 ≤ k_ex410285_ ∧
  0 ≤ k_ex410286_ ∧ k_ex410286_ ≤ k_ex410287_ ∧ k_ex410287_ ≤ k_ex410285_ -/
def decode_fee_market_tx (tx : TransactionEvmByteSlice) (pubkey : EvmByteSlice) (fork : Fork) (sender : address) (fields : RlpCursor) : SailM Transaction := do
  let tx := ((tx).2).2
  let pubkey := ((pubkey).2).2
  let fields := ((((fields).2).2).2).2
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (chain_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (nonce_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (mp_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (mf_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (gas_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (to_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (value_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (data_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (al_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (v_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (r_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (s_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  (rlp_cursor_expect_end ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩)
  let v ← do
    (do
        let publicResult ← (rlp_ref_word ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, v_f⟩⟩⟩⟩⟩⟩)
        pure ((publicResult).value))
  let (al_addrs, al_slots, al_addr_count, al_slot_count) ← do
    (do
        let publicResult ← (decode_access_list
        ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, al_f⟩⟩⟩⟩⟩⟩)
        pure (((fun (semanticValue0, semanticValue1, semanticValue2, semanticValue3) => (semanticValue0, semanticValue1, (semanticValue2).value, (semanticValue3).value)) (publicResult))))
  (pure { tx_type := FeeMarketTx,
          sender := sender,
          raw := ⟨_, ⟨_, tx⟩⟩,
          nonce := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, nonce_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          chain_id := ← (rlp_ref_chain_identifier
              ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, chain_f⟩⟩⟩⟩⟩⟩),
          gas_limit := ← (rlp_ref_gas ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, gas_f⟩⟩⟩⟩⟩⟩
              fork),
          is_create := (to_f.content_len == 0),
          recipient := ← (pure (word_to_address
                ⟨((← (rlp_ref_word ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, to_f⟩⟩⟩⟩⟩⟩))).value⟩)),
          value := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, value_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          input_src := ← do
              let publicField ← (tx_input_span
                ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, data_f⟩⟩⟩⟩⟩⟩)
              pure (publicField),
          access_list_addresses := al_addrs,
          access_list_address_count := ⟨al_addr_count⟩,
          access_list_slots := al_slots,
          access_list_slot_count := ⟨al_slot_count⟩,
          max_fee := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, mf_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          max_blob_fee := ⟨(ZERO_WORD).value⟩,
          max_priority_fee := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, mp_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          authorizations := [],
          authorization_count := ⟨0⟩,
          blob_hashes := EMPTY_BLOB_HASHES,
          pubkey := ⟨_, ⟨_, pubkey⟩⟩,
          signing_hash := ← (tx_signing_hash FeeMarketTx (← (tx_sig_span chain_f v_f)) ⟨v⟩),
          sig_v := ⟨v⟩,
          sig_r := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, r_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          sig_s := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, s_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩) })

/-- Decodes the payload fields of an EIP-4844 transaction. -/
/- Type quantifiers: k_ex410303_ : Nat, k_ex410302_ : Nat, k_ex410301_ : Nat, k_ex410300_ : Nat, k_ex410295_
  : Nat, k_ex410294_ : Nat, k_ex410291_ : Nat, k_ex410290_ : Nat, 0 ≤ k_ex410290_ ∧
  0 ≤ k_ex410291_ ∧ 0 ≤ k_ex410291_ ∧ k_ex410291_ ≤ (2 ^ 30), 0 ≤ k_ex410294_ ∧
  0 ≤ k_ex410295_, 0 ≤ k_ex410300_ ∧ 0 ≤ k_ex410301_ ∧
  0 ≤ k_ex410302_ ∧ k_ex410302_ ≤ k_ex410303_ ∧ k_ex410303_ ≤ k_ex410301_ -/
def decode_blob_tx (tx : TransactionEvmByteSlice) (pubkey : EvmByteSlice) (fork : Fork) (sender : address) (fields : RlpCursor) : SailM Transaction := do
  let tx := ((tx).2).2
  let pubkey := ((pubkey).2).2
  let fields := ((((fields).2).2).2).2
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (chain_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (nonce_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (mp_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (mf_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (gas_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (to_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (value_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (data_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (al_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (mbf_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (bh_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (v_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (r_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (s_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  (rlp_cursor_expect_end ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩)
  let v ← do
    (do
        let publicResult ← (rlp_ref_word ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, v_f⟩⟩⟩⟩⟩⟩)
        pure ((publicResult).value))
  let (al_addrs, al_slots, al_addr_count, al_slot_count) ← do
    (do
        let publicResult ← (decode_access_list
        ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, al_f⟩⟩⟩⟩⟩⟩)
        pure (((fun (semanticValue0, semanticValue1, semanticValue2, semanticValue3) => (semanticValue0, semanticValue1, (semanticValue2).value, (semanticValue3).value)) (publicResult))))
  let blob_hashes ← do
    (decode_blob_hashes ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, bh_f⟩⟩⟩⟩⟩⟩)
  (pure { tx_type := BlobTx,
          sender := sender,
          raw := ⟨_, ⟨_, tx⟩⟩,
          nonce := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, nonce_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          chain_id := ← (rlp_ref_chain_identifier
              ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, chain_f⟩⟩⟩⟩⟩⟩),
          gas_limit := ← (rlp_ref_gas ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, gas_f⟩⟩⟩⟩⟩⟩
              fork),
          is_create := (to_f.content_len == 0),
          recipient := ← (pure (word_to_address
                ⟨((← (rlp_ref_word ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, to_f⟩⟩⟩⟩⟩⟩))).value⟩)),
          value := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, value_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          input_src := ← do
              let publicField ← (tx_input_span
                ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, data_f⟩⟩⟩⟩⟩⟩)
              pure (publicField),
          access_list_addresses := al_addrs,
          access_list_address_count := ⟨al_addr_count⟩,
          access_list_slots := al_slots,
          access_list_slot_count := ⟨al_slot_count⟩,
          max_fee := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, mf_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          max_blob_fee := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, mbf_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          max_priority_fee := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, mp_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          authorizations := [],
          authorization_count := ⟨0⟩,
          blob_hashes := blob_hashes,
          pubkey := ⟨_, ⟨_, pubkey⟩⟩,
          signing_hash := ← (tx_signing_hash BlobTx (← (tx_sig_span chain_f v_f)) ⟨v⟩),
          sig_v := ⟨v⟩,
          sig_r := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, r_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          sig_s := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, s_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩) })

/-- Decodes the payload fields of an EIP-7702 transaction. -/
/- Type quantifiers: k_ex410319_ : Nat, k_ex410318_ : Nat, k_ex410317_ : Nat, k_ex410316_ : Nat, k_ex410311_
  : Nat, k_ex410310_ : Nat, k_ex410307_ : Nat, k_ex410306_ : Nat, 0 ≤ k_ex410306_ ∧
  0 ≤ k_ex410307_ ∧ 0 ≤ k_ex410307_ ∧ k_ex410307_ ≤ (2 ^ 30), 0 ≤ k_ex410310_ ∧
  0 ≤ k_ex410311_, 0 ≤ k_ex410316_ ∧ 0 ≤ k_ex410317_ ∧
  0 ≤ k_ex410318_ ∧ k_ex410318_ ≤ k_ex410319_ ∧ k_ex410319_ ≤ k_ex410317_ -/
def decode_set_code_tx (tx : TransactionEvmByteSlice) (pubkey : EvmByteSlice) (fork : Fork) (sender : address) (fields : RlpCursor) : SailM Transaction := do
  let tx := ((tx).2).2
  let pubkey := ((pubkey).2).2
  let fields := ((((fields).2).2).2).2
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (chain_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (nonce_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (mp_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (mf_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (gas_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (to_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (value_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (data_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (al_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (auth_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (v_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (r_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (s_f, fields)⟩⟩⟩⟩ ← do
    (rlp_cursor_pop fields)
  (rlp_cursor_expect_end ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩)
  let v ← do
    (do
        let publicResult ← (rlp_ref_word ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, v_f⟩⟩⟩⟩⟩⟩)
        pure ((publicResult).value))
  let (al_addrs, al_slots, al_addr_count, al_slot_count) ← do
    (do
        let publicResult ← (decode_access_list
        ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, al_f⟩⟩⟩⟩⟩⟩)
        pure (((fun (semanticValue0, semanticValue1, semanticValue2, semanticValue3) => (semanticValue0, semanticValue1, (semanticValue2).value, (semanticValue3).value)) (publicResult))))
  let (authorizations, authorization_count) ← do
    (do
        let publicResult ← (decode_auth_list
        ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, auth_f⟩⟩⟩⟩⟩⟩)
        pure (((fun (semanticValue0, semanticValue1) => (semanticValue0, (semanticValue1).value)) (publicResult))))
  (pure { tx_type := SetCodeTx,
          sender := sender,
          raw := ⟨_, ⟨_, tx⟩⟩,
          nonce := ← do
              let publicField ← (pure ((word_of_account_nonce
                  ⟨((← (rlp_ref_account_nonce
                    ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, nonce_f⟩⟩⟩⟩⟩⟩))).value⟩)).value)
              pure (⟨publicField⟩),
          chain_id := ← (rlp_ref_chain_identifier
              ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, chain_f⟩⟩⟩⟩⟩⟩),
          gas_limit := ← (rlp_ref_gas ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, gas_f⟩⟩⟩⟩⟩⟩
              fork),
          is_create := (to_f.content_len == 0),
          recipient := ← (pure (word_to_address
                ⟨((← (rlp_ref_word ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, to_f⟩⟩⟩⟩⟩⟩))).value⟩)),
          value := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, value_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          input_src := ← do
              let publicField ← (tx_input_span
                ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, data_f⟩⟩⟩⟩⟩⟩)
              pure (publicField),
          access_list_addresses := al_addrs,
          access_list_address_count := ⟨al_addr_count⟩,
          access_list_slots := al_slots,
          access_list_slot_count := ⟨al_slot_count⟩,
          max_fee := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, mf_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          max_blob_fee := ⟨(ZERO_WORD).value⟩,
          max_priority_fee := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, mp_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          authorizations := authorizations,
          authorization_count := ⟨authorization_count⟩,
          blob_hashes := EMPTY_BLOB_HASHES,
          pubkey := ⟨_, ⟨_, pubkey⟩⟩,
          signing_hash := ← (tx_signing_hash SetCodeTx (← (tx_sig_span chain_f v_f)) ⟨v⟩),
          sig_v := ⟨v⟩,
          sig_r := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, r_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          sig_s := ← do
              let publicField ← (do
                  let publicResult ← (rlp_ref_uint_word
                  ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, s_f⟩⟩⟩⟩⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩) })

/- Type quantifiers: k_tx_off : Nat, k_tx_len : Nat, k_public_key_off : Nat, source_valid_range(k_tx_off, k_tx_len)
  ∧
  0 ≤ k_tx_len ∧
  k_tx_len ≤ transaction_length_bound ∧ source_valid_range(k_public_key_off, 65) -/
def rlp_decode_tx (tx : (EvmByteSliceFields k_tx_off k_tx_len)) (pubkey : (EvmByteSliceFields k_public_key_off 65)) (fork : Fork) : SailM Transaction := do
  let sender ← do
    (pure (word_to_address
        ⟨((hash_to_word
          (← (keccak256_slice ⟨_, ⟨_, (sub_slice pubkey 1 PUBLIC_KEY_BODY_LENGTH)⟩⟩)))).value⟩))
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
    if (typed : Bool)
    then
      (do
        if ((1 ≤b tx_length) : Bool)
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
  let ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩ ← do (rlp_node_cursor ⟨_, ⟨_, payload⟩⟩)
  match ttype with
  | 0x00 =>
    (decode_legacy_tx ⟨_, ⟨_, tx⟩⟩ ⟨_, ⟨_, pubkey⟩⟩ fork sender
      ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩)
  | 0x01 =>
    (decode_access_list_tx ⟨_, ⟨_, tx⟩⟩ ⟨_, ⟨_, pubkey⟩⟩ fork sender
      ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩)
  | 0x02 =>
    (decode_fee_market_tx ⟨_, ⟨_, tx⟩⟩ ⟨_, ⟨_, pubkey⟩⟩ fork sender
      ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩)
  | 0x03 =>
    (decode_blob_tx ⟨_, ⟨_, tx⟩⟩ ⟨_, ⟨_, pubkey⟩⟩ fork sender
      ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩)
  | 0x04 =>
    (decode_set_code_tx ⟨_, ⟨_, tx⟩⟩ ⟨_, ⟨_, pubkey⟩⟩ fork sender
      ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩)
  | _ => sailThrow ((InvalidBlock RlpDecode))

