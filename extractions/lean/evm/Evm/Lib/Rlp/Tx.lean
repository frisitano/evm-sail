import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
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

noncomputable section
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

def EMPTY_ACCESS_LIST_DECODE : AccessListDecode :=
  { addresses := []
    storage_slots := []
    address_count := 0
    slot_count := 0 }

partial def decode_access_list_keys (cursor : RlpCursor) (addr : (BitVec 160)) (tail : AccessListDecode) : SailM AccessListDecode := do
  if ((rlp_cursor_empty cursor) : Bool)
  then (pure tail)
  else
    (do
      let (key, next) ← do (rlp_cursor_pop cursor)
      let storage_key ← (( do
        (pure { addr := addr
                slot := ← (rlp_ref_word key) }) ) : SailM StorageKey )
      let result ← do (decode_access_list_keys next addr tail)
      if ((result.slot_count == ((2 ^i 64) -i 1)) : Bool)
      then sailThrow ((InvalidBlock RlpDecode))
      else
        (pure { addresses := result.addresses
                storage_slots := (storage_key :: result.storage_slots)
                address_count := result.address_count
                slot_count := ← (protocol_quantity_increment result.slot_count) }))
termination_by (let (cursor, addr, tail) := (cursor, addr, tail)
let .ByteQuantity stop := cursor.stop
let .ByteQuantity current := cursor.current
(stop -i current)).toNat

partial def decode_access_list_entries (cursor : RlpCursor) : SailM AccessListDecode := do
  if ((rlp_cursor_empty cursor) : Bool)
  then (pure EMPTY_ACCESS_LIST_DECODE)
  else
    (do
      let (entry, next) ← do (rlp_cursor_pop cursor)
      let (addr_f, entry_fields) ← do (rlp_cursor_pop (← (rlp_ref_cursor entry)))
      let (keys_f, entry_fields) ← do (rlp_cursor_pop entry_fields)
      (rlp_cursor_expect_end entry_fields)
      let addr ← do (pure (word_to_address (← (rlp_ref_word addr_f))))
      let tail ← do (decode_access_list_entries next)
      let result ← do (decode_access_list_keys (← (rlp_ref_cursor keys_f)) addr tail)
      if ((result.address_count == ((2 ^i 64) -i 1)) : Bool)
      then sailThrow ((InvalidBlock RlpDecode))
      else
        (pure { addresses := (addr :: result.addresses)
                storage_slots := result.storage_slots
                address_count := ← (protocol_quantity_increment result.address_count)
                slot_count := result.slot_count }))
termination_by (let cursor := cursor
let .ByteQuantity stop := cursor.stop
let .ByteQuantity current := cursor.current
(stop -i current)).toNat

def decode_access_list (f : RlpFieldRef) : SailM ((List (BitVec 160)) × (List StorageKey) × Nat × Nat) := do
  let decoded ← do (decode_access_list_entries (← (rlp_ref_cursor f)))
  (pure (decoded.addresses, decoded.storage_slots, decoded.address_count, decoded.slot_count))

def BLOB_HASH_RLP_LENGTH : byte_length := (ByteQuantity 33)

def BLOB_HASH_LENGTH : byte_length := WORD_BYTE_LENGTH

/-- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ (2 ^ 64 - 1) -/
partial def decode_blob_hash_items (cursor : RlpCursor) (count : Nat) : SailM Nat := do
  if ((rlp_cursor_empty cursor) : Bool)
  then (pure count)
  else
    (do
      let (item, next) ← do (rlp_cursor_pop cursor)
      if ((item.is_list || ((byte_quantity_not_equal item.full_len BLOB_HASH_RLP_LENGTH) || ((byte_quantity_not_equal
                 item.content_len BLOB_HASH_LENGTH) || ((← (slice_byte item.source item.full_off)) != 0xA0#8)))) : Bool)
      then sailThrow ((InvalidBlock RlpDecode))
      else (pure ())
      if ((count == ((2 ^i 64) -i 1)) : Bool)
      then sailThrow ((InvalidBlock RlpDecode))
      else (pure ())
      (decode_blob_hash_items next (← (protocol_quantity_increment count))))
termination_by (let (cursor, count) := (cursor, count)
let .ByteQuantity stop := cursor.stop
let .ByteQuantity current := cursor.current
(stop -i current)).toNat

def decode_blob_hashes (f : RlpFieldRef) : SailM BlobHashes := do
  let bytes ← do (sub_slice f.source f.content_off f.content_len)
  (pure { bytes := bytes
          count := ← (decode_blob_hash_items (← (rlp_ref_cursor f)) 0) })

def EMPTY_AUTHORIZATION_DECODE : AuthorizationDecode :=
  { authorizations := []
    count := 0 }

partial def decode_auth_tuples (cursor : RlpCursor) : SailM AuthorizationDecode := do
  if ((rlp_cursor_empty cursor) : Bool)
  then (pure EMPTY_AUTHORIZATION_DECODE)
  else
    (do
      let (tuple, next) ← do (rlp_cursor_pop cursor)
      let (chain_f, fields) ← do (rlp_cursor_pop (← (rlp_ref_cursor tuple)))
      let (addr_f, fields) ← do (rlp_cursor_pop fields)
      let (nonce_f, fields) ← do (rlp_cursor_pop fields)
      let (y_f, fields) ← do (rlp_cursor_pop fields)
      let (r_f, fields) ← do (rlp_cursor_pop fields)
      let (s_f, fields) ← do (rlp_cursor_pop fields)
      (rlp_cursor_expect_end fields)
      let chain_id ← do (rlp_ref_uint_word chain_f)
      let auth_nonce ← do (rlp_ref_uint nonce_f)
      let y_value ← do (rlp_ref_uint y_f)
      let y_valid := (y_value ≤b 1)
      let y : y_parity :=
        if ((y_value == 0) : Bool)
        then 0
        else 1
      let r ← do (rlp_ref_uint_word r_f)
      let s ← do (rlp_ref_uint_word s_f)
      let auth_addr ← do (pure (word_to_address (← (rlp_ref_word addr_f))))
      let (ok, authority) ← do
        if (y_valid : Bool)
        then (ecrecover_addr (← (auth_signing_hash chain_id auth_addr auth_nonce)) y r s)
        else (pure (false, ZERO_ADDR))
      let nonce_valid := (auth_nonce != ((2 ^i 64) -i 1))
      let authorization : Authorization :=
        { valid_sig := (ok && (y_valid && ((word_ult ZERO_WORD r) && ((word_ult r SECP_N_FULL) && ((word_ult
                        ZERO_WORD s) && ((word_ule s SECP_N_HALF) && nonce_valid))))))
          authority := authority
          address := auth_addr
          nonce := auth_nonce
          chain_id := chain_id }
      let tail ← do (decode_auth_tuples next)
      if ((tail.count == ((2 ^i 64) -i 1)) : Bool)
      then sailThrow ((InvalidBlock RlpDecode))
      else
        (pure { authorizations := (authorization :: tail.authorizations)
                count := ← (protocol_quantity_increment tail.count) }))
termination_by (let cursor := cursor
let .ByteQuantity stop := cursor.stop
let .ByteQuantity current := cursor.current
(stop -i current)).toNat

def decode_auth_list (f : RlpFieldRef) : SailM ((List Authorization) × Nat) := do
  let decoded ← do (decode_auth_tuples (← (rlp_ref_cursor f)))
  (pure (decoded.authorizations, decoded.count))

def tx_input_span (payload : EvmByteSlice) (data : RlpFieldRef) : SailM EvmByteSlice := do
  (sub_slice payload data.content_off data.content_len)

def tx_sig_span (payload : EvmByteSlice) (first : RlpFieldRef) (signature : RlpFieldRef) : SailM EvmByteSlice := do
  let start := first.full_off
  let stop := signature.full_off
  if ((byte_quantity_lt stop start) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (sub_slice payload start (← (byte_quantity_sub stop start)))

def rlp_ref_gas (f : RlpFieldRef) (fork : Fork) : SailM gas := do
  let value ← do (rlp_ref_uint f)
  if ((GAS_MAX_VALUE <b value) : Bool)
  then sailThrow ((InvalidBlock GasUsedExceedsLimit))
  else (pure ())
  if (((fork_gteq fork Osaka) && (OSAKA_TRANSACTION_GAS_LIMIT_VALUE <b value)) : Bool)
  then sailThrow ((InvalidBlock GasUsedExceedsLimit))
  else (nat_to_gas value)

def rlp_decode_tx (tx : EvmByteSlice) (pubkey : EvmByteSlice) (fork : Fork) : SailM Transaction := do
  let sender ← do
    (pure (word_to_address
        (← (keccak256_slice (← (sub_slice pubkey BYTE_ONE PUBLIC_KEY_BODY_LENGTH))))))
  let tx_len : byte_length := tx.len
  let b0 ← (( do
    if ((byte_quantity_equal tx_len BYTE_ZERO) : Bool)
    then sailThrow ((InvalidBlock RlpDecode))
    else (slice_byte tx BYTE_ZERO) ) : SailM byte )
  let ttype : byte :=
    if (((Sail.BitVec.extractLsb b0 7 6) == 0b11#2) : Bool)
    then 0x00#8
    else b0
  let typed := (ttype != 0x00#8)
  let payload ← (( do
    if (typed : Bool)
    then
      (do
        if ((byte_quantity_le BYTE_ONE tx_len) : Bool)
        then (sub_slice tx BYTE_ONE (← (byte_quantity_sub tx_len BYTE_ONE)))
        else sailThrow ((InvalidBlock RlpDecode)))
    else (pure tx) ) : SailM EvmByteSlice )
  let fields ← do (rlp_node_cursor payload)
  match ttype with
  | 0x00 =>
    (do
      let (nonce_f, fields) ← do (rlp_cursor_pop fields)
      let (gp_f, fields) ← do (rlp_cursor_pop fields)
      let (gas_f, fields) ← do (rlp_cursor_pop fields)
      let (to_f, fields) ← do (rlp_cursor_pop fields)
      let (value_f, fields) ← do (rlp_cursor_pop fields)
      let (data_f, fields) ← do (rlp_cursor_pop fields)
      let (v_f, fields) ← do (rlp_cursor_pop fields)
      let (r_f, fields) ← do (rlp_cursor_pop fields)
      let (s_f, fields) ← do (rlp_cursor_pop fields)
      (rlp_cursor_expect_end fields)
      let v ← do (rlp_ref_word v_f)
      let gp ← do (rlp_ref_uint_word gp_f)
      (pure { tx_type := LegacyTx
              sender := sender
              raw := tx
              nonce := ← (rlp_ref_uint_word nonce_f)
              chain_id := 0
              gas_limit := ← (rlp_ref_gas gas_f fork)
              is_create := (byte_quantity_equal to_f.content_len BYTE_ZERO)
              recipient := ← (pure (word_to_address (← (rlp_ref_word to_f))))
              value := ← (rlp_ref_uint_word value_f)
              input_src := ← (tx_input_span payload data_f)
              access_list_addresses := []
              access_list_address_count := 0
              access_list_slots := []
              access_list_slot_count := 0
              max_fee := gp
              max_blob_fee := ZERO_WORD
              max_priority_fee := gp
              authorizations := []
              authorization_count := 0
              blob_hashes := EMPTY_BLOB_HASHES
              pubkey := pubkey
              signing_hash := ← (tx_signing_hash LegacyTx (← (tx_sig_span payload nonce_f v_f))
                  v)
              sig_v := v
              sig_r := ← (rlp_ref_uint_word r_f)
              sig_s := ← (rlp_ref_uint_word s_f) }))
  | 0x01 =>
    (do
      let (chain_f, fields) ← do (rlp_cursor_pop fields)
      let (nonce_f, fields) ← do (rlp_cursor_pop fields)
      let (gp_f, fields) ← do (rlp_cursor_pop fields)
      let (gas_f, fields) ← do (rlp_cursor_pop fields)
      let (to_f, fields) ← do (rlp_cursor_pop fields)
      let (value_f, fields) ← do (rlp_cursor_pop fields)
      let (data_f, fields) ← do (rlp_cursor_pop fields)
      let (al_f, fields) ← do (rlp_cursor_pop fields)
      let (v_f, fields) ← do (rlp_cursor_pop fields)
      let (r_f, fields) ← do (rlp_cursor_pop fields)
      let (s_f, fields) ← do (rlp_cursor_pop fields)
      (rlp_cursor_expect_end fields)
      let v ← do (rlp_ref_word v_f)
      let gp ← do (rlp_ref_uint_word gp_f)
      let (al_addrs, al_slots, al_addr_count, al_slot_count) ← do (decode_access_list al_f)
      (pure { tx_type := AccessListTx
              sender := sender
              raw := tx
              nonce := ← (rlp_ref_uint_word nonce_f)
              chain_id := ← (rlp_ref_uint chain_f)
              gas_limit := ← (rlp_ref_gas gas_f fork)
              is_create := (byte_quantity_equal to_f.content_len BYTE_ZERO)
              recipient := ← (pure (word_to_address (← (rlp_ref_word to_f))))
              value := ← (rlp_ref_uint_word value_f)
              input_src := ← (tx_input_span payload data_f)
              access_list_addresses := al_addrs
              access_list_address_count := al_addr_count
              access_list_slots := al_slots
              access_list_slot_count := al_slot_count
              max_fee := gp
              max_blob_fee := ZERO_WORD
              max_priority_fee := gp
              authorizations := []
              authorization_count := 0
              blob_hashes := EMPTY_BLOB_HASHES
              pubkey := pubkey
              signing_hash := ← (tx_signing_hash AccessListTx
                  (← (tx_sig_span payload chain_f v_f)) v)
              sig_v := v
              sig_r := ← (rlp_ref_uint_word r_f)
              sig_s := ← (rlp_ref_uint_word s_f) }))
  | 0x02 =>
    (do
      let (chain_f, fields) ← do (rlp_cursor_pop fields)
      let (nonce_f, fields) ← do (rlp_cursor_pop fields)
      let (mp_f, fields) ← do (rlp_cursor_pop fields)
      let (mf_f, fields) ← do (rlp_cursor_pop fields)
      let (gas_f, fields) ← do (rlp_cursor_pop fields)
      let (to_f, fields) ← do (rlp_cursor_pop fields)
      let (value_f, fields) ← do (rlp_cursor_pop fields)
      let (data_f, fields) ← do (rlp_cursor_pop fields)
      let (al_f, fields) ← do (rlp_cursor_pop fields)
      let (v_f, fields) ← do (rlp_cursor_pop fields)
      let (r_f, fields) ← do (rlp_cursor_pop fields)
      let (s_f, fields) ← do (rlp_cursor_pop fields)
      (rlp_cursor_expect_end fields)
      let v ← do (rlp_ref_word v_f)
      let (al_addrs, al_slots, al_addr_count, al_slot_count) ← do (decode_access_list al_f)
      (pure { tx_type := FeeMarketTx
              sender := sender
              raw := tx
              nonce := ← (rlp_ref_uint_word nonce_f)
              chain_id := ← (rlp_ref_uint chain_f)
              gas_limit := ← (rlp_ref_gas gas_f fork)
              is_create := (byte_quantity_equal to_f.content_len BYTE_ZERO)
              recipient := ← (pure (word_to_address (← (rlp_ref_word to_f))))
              value := ← (rlp_ref_uint_word value_f)
              input_src := ← (tx_input_span payload data_f)
              access_list_addresses := al_addrs
              access_list_address_count := al_addr_count
              access_list_slots := al_slots
              access_list_slot_count := al_slot_count
              max_fee := ← (rlp_ref_uint_word mf_f)
              max_blob_fee := ZERO_WORD
              max_priority_fee := ← (rlp_ref_uint_word mp_f)
              authorizations := []
              authorization_count := 0
              blob_hashes := EMPTY_BLOB_HASHES
              pubkey := pubkey
              signing_hash := ← (tx_signing_hash FeeMarketTx
                  (← (tx_sig_span payload chain_f v_f)) v)
              sig_v := v
              sig_r := ← (rlp_ref_uint_word r_f)
              sig_s := ← (rlp_ref_uint_word s_f) }))
  | 0x03 =>
    (do
      let (chain_f, fields) ← do (rlp_cursor_pop fields)
      let (nonce_f, fields) ← do (rlp_cursor_pop fields)
      let (mp_f, fields) ← do (rlp_cursor_pop fields)
      let (mf_f, fields) ← do (rlp_cursor_pop fields)
      let (gas_f, fields) ← do (rlp_cursor_pop fields)
      let (to_f, fields) ← do (rlp_cursor_pop fields)
      let (value_f, fields) ← do (rlp_cursor_pop fields)
      let (data_f, fields) ← do (rlp_cursor_pop fields)
      let (al_f, fields) ← do (rlp_cursor_pop fields)
      let (mbf_f, fields) ← do (rlp_cursor_pop fields)
      let (bh_f, fields) ← do (rlp_cursor_pop fields)
      let (v_f, fields) ← do (rlp_cursor_pop fields)
      let (r_f, fields) ← do (rlp_cursor_pop fields)
      let (s_f, fields) ← do (rlp_cursor_pop fields)
      (rlp_cursor_expect_end fields)
      let v ← do (rlp_ref_word v_f)
      let (al_addrs, al_slots, al_addr_count, al_slot_count) ← do (decode_access_list al_f)
      let blob_hashes ← do (decode_blob_hashes bh_f)
      (pure { tx_type := BlobTx
              sender := sender
              raw := tx
              nonce := ← (rlp_ref_uint_word nonce_f)
              chain_id := ← (rlp_ref_uint chain_f)
              gas_limit := ← (rlp_ref_gas gas_f fork)
              is_create := (byte_quantity_equal to_f.content_len BYTE_ZERO)
              recipient := ← (pure (word_to_address (← (rlp_ref_word to_f))))
              value := ← (rlp_ref_uint_word value_f)
              input_src := ← (tx_input_span payload data_f)
              access_list_addresses := al_addrs
              access_list_address_count := al_addr_count
              access_list_slots := al_slots
              access_list_slot_count := al_slot_count
              max_fee := ← (rlp_ref_uint_word mf_f)
              max_blob_fee := ← (rlp_ref_uint_word mbf_f)
              max_priority_fee := ← (rlp_ref_uint_word mp_f)
              authorizations := []
              authorization_count := 0
              blob_hashes := blob_hashes
              pubkey := pubkey
              signing_hash := ← (tx_signing_hash BlobTx (← (tx_sig_span payload chain_f v_f)) v)
              sig_v := v
              sig_r := ← (rlp_ref_uint_word r_f)
              sig_s := ← (rlp_ref_uint_word s_f) }))
  | 0x04 =>
    (do
      let (chain_f, fields) ← do (rlp_cursor_pop fields)
      let (nonce_f, fields) ← do (rlp_cursor_pop fields)
      let (mp_f, fields) ← do (rlp_cursor_pop fields)
      let (mf_f, fields) ← do (rlp_cursor_pop fields)
      let (gas_f, fields) ← do (rlp_cursor_pop fields)
      let (to_f, fields) ← do (rlp_cursor_pop fields)
      let (value_f, fields) ← do (rlp_cursor_pop fields)
      let (data_f, fields) ← do (rlp_cursor_pop fields)
      let (al_f, fields) ← do (rlp_cursor_pop fields)
      let (auth_f, fields) ← do (rlp_cursor_pop fields)
      let (v_f, fields) ← do (rlp_cursor_pop fields)
      let (r_f, fields) ← do (rlp_cursor_pop fields)
      let (s_f, fields) ← do (rlp_cursor_pop fields)
      (rlp_cursor_expect_end fields)
      let v ← do (rlp_ref_word v_f)
      let (al_addrs, al_slots, al_addr_count, al_slot_count) ← do (decode_access_list al_f)
      let (authorizations, authorization_count) ← do (decode_auth_list auth_f)
      (pure { tx_type := SetCodeTx
              sender := sender
              raw := tx
              nonce := ← (word_of_nat (← (rlp_ref_uint nonce_f)))
              chain_id := ← (rlp_ref_uint chain_f)
              gas_limit := ← (rlp_ref_gas gas_f fork)
              is_create := (byte_quantity_equal to_f.content_len BYTE_ZERO)
              recipient := ← (pure (word_to_address (← (rlp_ref_word to_f))))
              value := ← (rlp_ref_uint_word value_f)
              input_src := ← (tx_input_span payload data_f)
              access_list_addresses := al_addrs
              access_list_address_count := al_addr_count
              access_list_slots := al_slots
              access_list_slot_count := al_slot_count
              max_fee := ← (rlp_ref_uint_word mf_f)
              max_blob_fee := ZERO_WORD
              max_priority_fee := ← (rlp_ref_uint_word mp_f)
              authorizations := authorizations
              authorization_count := authorization_count
              blob_hashes := EMPTY_BLOB_HASHES
              pubkey := pubkey
              signing_hash := ← (tx_signing_hash SetCodeTx (← (tx_sig_span payload chain_f v_f))
                  v)
              sig_v := v
              sig_r := ← (rlp_ref_uint_word r_f)
              sig_s := ← (rlp_ref_uint_word s_f) }))
  | _ => sailThrow ((InvalidBlock RlpDecode))

