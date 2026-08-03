
/*md
# Transaction RLP decoding

The EIP-2718 transaction envelope decoder: per-envelope destructuring into
the [Transaction][type-Transaction] type (one walk, against each
envelope's exact field shape), plus the EIP-2930 access-list
and EIP-7702 authorization-tuple decoders. Standalone and purely
structural: the envelope and public key are immutable stateless-input
spans; the signature rules and the cryptography live in their own modules.
*/
/*! A decoded access list and the counts needed for intrinsic gas. */
struct AccessListDecode('address_bound : Int, 'slot_bound : Int),
    source_valid_length('address_bound) & source_valid_length('slot_bound) = {
    address_count : range(0, 'address_bound),
    slot_count : range(0, 'slot_bound),
}

let EMPTY_ACCESS_LIST_DECODE : AccessListDecode(0, 0) = struct { address_count = 0, slot_count = 0 }

/*! Reclassifies transaction RLP content as an immutable input span. Every
    cursor in this module originates in the transaction envelope. */
function transaction_rlp_content forall 'source_off 'source_len 'content_len,
                                   rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    StatelessInputSlice
) = {
    let content = rlp_item_content(f);
    stateless_input_slice(content.off, content.len)
}

/* The unconsumed key-list content span strictly decreases on every call. */
/*! Validates and counts the storage keys of one access-list entry. */
val decode_access_list_keys : forall 'source_off 'source_len 'address_bound 'slot_bound,
                                  source_valid_range('source_off, 'source_len)
                                & source_valid_length('address_bound)
                                & source_valid_length('slot_bound)
                                & source_valid_length('slot_bound + 'source_len). (
        RlpCursor('source_off, 'source_len),
        address,
        AccessListDecode('address_bound, 'slot_bound),
    ) -> AccessListDecode('address_bound, 'slot_bound + 'source_len)

function decode_access_list_keys(cursor, addr, tail) =
    if cursor.len == 0 then {
        struct { address_count = tail.address_count, slot_count = tail.slot_count }
    } else {
        let key = rlp_decode_item(cursor);
        let next = rlp_cursor_advance(cursor, key.source.len);
        let _ : StorageKey = struct { addr = addr, slot = rlp_decode_word(key) };
        let result = decode_access_list_keys(next, addr, tail);
        struct { address_count = result.address_count, slot_count = result.slot_count + 1 }
    }
termination_measure decode_access_list_keys(cursor, addr, tail) =
    cursor.len

/* The unconsumed access-list content span strictly decreases on every call. */
/*! Decodes the remaining access-list entries into a flat execution view. */
val decode_access_list_entries : forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). RlpCursor(
        'source_off,
        'source_len,
    ) -> AccessListDecode('source_len, 'source_len)

function decode_access_list_entries(cursor) =
    if cursor.len == 0 then {
        EMPTY_ACCESS_LIST_DECODE
    } else {
        let entry = rlp_decode_item(cursor);
        let next = rlp_cursor_advance(cursor, entry.source.len);
        let entry_fields = rlp_decode_list(entry);
        let addr_f = rlp_decode_item(entry_fields);
        let entry_fields = rlp_cursor_advance(entry_fields, addr_f.source.len);
        let keys_f = rlp_decode_item(entry_fields);
        let entry_fields = rlp_cursor_advance(entry_fields, keys_f.source.len);
        rlp_cursor_expect_end(entry_fields);

        let addr = word_to_address(rlp_decode_word(addr_f));
        let tail = decode_access_list_entries(next);
        let result = decode_access_list_keys(rlp_decode_list(keys_f), addr, tail);
        struct { address_count = result.address_count + 1, slot_count = result.slot_count }
    }
termination_measure decode_access_list_entries(cursor) =
    cursor.len

/*! Decodes an EIP-2930 access list — RLP `[[address, [slot, …]], …]` —
    without materializing its entries. */
function decode_access_list forall 'source_off 'source_len 'content_len,
                              rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    AccessListRef
) = {
    if f.source.len <= sizeof(transaction_length_bound) then {
        let decoded = decode_access_list_entries(rlp_decode_list(f));
        struct {
            encoded = transaction_rlp_content(f),
            address_count = decoded.address_count,
            slot_count = decoded.slot_count,
        }
    } else {
        throw InvalidBlock(RlpDecode)
    }
}

/* Validate the canonical RLP list of bytes32 blob versioned hashes once, then
 * retain its encoded content as a fixed-stride source view. Each element is
 * exactly 0xa0 followed by 32 bytes, so BLOBHASH can load item i at 33*i+1. */
/*md
## Constants

Typed transaction decoding uses the fixed RLP widths of versioned blob hashes
and the domain tags and field counts defined by the transaction EIPs.
*/
let BLOB_HASH_RLP_LENGTH : int(33) = 33
let BLOB_HASH_LENGTH : int(32) = WORD_BYTE_LENGTH

/* The unconsumed hash-list content span strictly decreases on every call. */
/*! Validates every fixed-width versioned-hash item and returns their count.
    The cursor exits immediately for an empty list and checks the version byte
    while each item is already live, avoiding a second fixed-width pass. */
function decode_blob_hash_items forall 'source_off 'source_len 'limit,
                                  source_valid_range('source_off, 'source_len) & transaction_blob_limit_value('limit). (
    cursor : RlpCursor('source_off, 'source_len),
    limit : int('limit),
    count : transaction_blob_count('limit),
) -> (
    transaction_blob_count('limit)
) =
    if cursor.len == 0 then {
        count
    } else {
        let item = rlp_decode_item(cursor);
        let next = rlp_cursor_advance(cursor, item.source.len);
        if    item.is_list
           |  item.source.len
           != BLOB_HASH_RLP_LENGTH
           |  item.content_len
           != BLOB_HASH_LENGTH
           |  slice_byte(item.source, 0)
           != 0xa0 then {
            throw InvalidBlock(RlpDecode)
        };
        if slice_byte(item.source, 1) != 0x01 then {
            throw InvalidBlock(ExecutionInvalid)
        };
        if count < limit then {
            decode_blob_hash_items(next, limit, count + 1)
        } else {
            throw InvalidBlock(RlpDecode)
        }
    }
termination_measure decode_blob_hash_items(cursor, limit, count) =
    cursor.len

/*! Validates the canonical RLP list of `bytes32` blob versioned hashes
    once, then retains its encoded content as a fixed-stride source view:
    each element is exactly `0xa0` followed by 32 bytes, so `BLOBHASH` can
    load item `i` at `33·i + 1`. */
function decode_blob_hashes forall 'source_off 'source_len 'content_len 'limit,
                              rlp_field_ref_valid('source_off, 'source_len, 'content_len) &
                                  transaction_blob_limit_value('limit). (
    f : RlpFieldRef('source_off, 'source_len, 'content_len),
    limit : int('limit),
) -> (
    BlobHashesFields('limit)
) = {
    let bytes = transaction_rlp_content(f);
    struct { bytes = bytes, count = decode_blob_hash_items(rlp_decode_list(f), limit, 0) }
}

/* ---- EIP-7702 authorization-tuple recovery ---------------------------------
 * Each set-code authorization tuple [chainId, address, nonce, yParity, r, s] is
 * signed by the AUTHORITY, whose address is recovered (not witnessed) via
 * ecrecover over keccak(0x05 ++ rlp([chainId, address, nonce])). */

/* decode an EIP-7702 authorization list into Authorizations,
 * recovering each authority. valid_sig records tuple-local signature validity
 * (recoverable, low-s, r/y parity ranges, incrementable nonce); process_auth then
 * applies the chain-id / account nonce / code checks. */
/* The unconsumed authorization-list content span strictly decreases on every call. */
/*! Structurally validates and counts the remaining authorization tuples.
    Signature recovery is deliberately deferred until execution. */
val validate_auth_tuples : forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
        RlpCursor('source_off, 'source_len),
        transaction_item_count,
    ) -> transaction_item_count

function validate_auth_tuples(cursor, count) =
    if cursor.len == 0 then {
        count
    } else {
        let tuple = rlp_decode_item(cursor);
        let next = rlp_cursor_advance(cursor, tuple.source.len);
        let fields = rlp_decode_list(tuple);
        let chain_f = rlp_decode_item(fields);
        let fields = rlp_cursor_advance(fields, chain_f.source.len);
        let addr_f = rlp_decode_item(fields);
        let fields = rlp_cursor_advance(fields, addr_f.source.len);
        let nonce_f = rlp_decode_item(fields);
        let fields = rlp_cursor_advance(fields, nonce_f.source.len);
        let y_f = rlp_decode_item(fields);
        let fields = rlp_cursor_advance(fields, y_f.source.len);
        let _ = rlp_decode_bool(y_f);
        let r_f = rlp_decode_item(fields);
        let fields = rlp_cursor_advance(fields, r_f.source.len);
        let s_f = rlp_decode_item(fields);
        let fields = rlp_cursor_advance(fields, s_f.source.len);
        rlp_cursor_expect_end(fields);
        let _ = rlp_decode_u256(chain_f);
        let _ = word_to_address(rlp_decode_word(addr_f));
        let _ = rlp_decode_uint64(nonce_f);
        let _ = rlp_decode_u256(r_f);
        let _ = rlp_decode_u256(s_f);
        if count < sizeof(transaction_length_bound) then {
            validate_auth_tuples(next, count + 1)
        } else {
            throw InvalidBlock(RlpDecode)
        }
    }
termination_measure validate_auth_tuples(cursor, count) =
    cursor.len

/*! Decodes an EIP-7702 authorization list into
    a validated source-backed cursor view. */
function decode_auth_list forall 'source_off 'source_len 'content_len,
                            rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    AuthorizationListRef
) = {
    if f.source.len <= sizeof(transaction_length_bound) then {
        authorization_list_ref(
            transaction_rlp_content(f),
            validate_auth_tuples(rlp_decode_list(f), 0),
        )
    } else {
        throw InvalidBlock(RlpDecode)
    }
}

/*! Decodes and signature-recovers one EIP-7702 authorization tuple. */
val decode_authorization : forall 'source_off 'source_len 'content_len,
                             rlp_field_ref_valid('source_off, 'source_len, 'content_len). RlpFieldRef(
        'source_off,
        'source_len,
        'content_len,
    ) -> Authorization
function decode_authorization(tuple) = {
    let fields = rlp_decode_list(tuple);
    let chain_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, chain_f.source.len);
    let addr_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, addr_f.source.len);
    let nonce_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, nonce_f.source.len);
    let y_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, y_f.source.len);
    let y = rlp_decode_bool(y_f);
    let r_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, r_f.source.len);
    let s_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, s_f.source.len);
    rlp_cursor_expect_end(fields);

    let chain_id = rlp_decode_u256(chain_f);
    let auth_nonce : account_nonce = rlp_decode_uint64(nonce_f);
    let r = rlp_decode_u256(r_f);
    let s = rlp_decode_u256(s_f);
    let auth_addr = word_to_address(rlp_decode_word(addr_f));
    let (ok, authority) : (bool, address) = match y {
        RlpOk(false) => ecrecover_addr(auth_signing_hash(chain_id, auth_addr, auth_nonce), 0, r, s),
        RlpOk(true) => ecrecover_addr(auth_signing_hash(chain_id, auth_addr, auth_nonce), 1, r, s),
        RlpInvalidValue() => (false, ZERO_ADDRESS),
    };
    struct {
        valid_sig =
              ok
            & word_ult(ZERO_WORD, r)
            & word_ult(r, SECP_N_FULL)
            & word_ult(ZERO_WORD, s)
            & word_ule(s, SECP_N_HALF)
            & (auth_nonce != sizeof(account_nonce_bound)),
        authority = authority,
        address = auth_addr,
        nonce = auth_nonce,
        chain_id = chain_id,
    }
}

/*! The calldata/initcode span of the data field within the envelope. The RLP
    cursor existentially hides its bounded source length, so this boundary
    reifies the already-established transaction-envelope invariant. */
function tx_input_span forall 'source_off 'source_len 'content_len,
                         rlp_field_ref_valid('source_off, 'source_len, 'content_len). (data :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    TransactionInputSlice
) = {
    let content = transaction_rlp_content(data);
    if content.len <= sizeof(transaction_length_bound) then {
        content
    } else {
        throw InvalidBlock(RlpDecode)
    }
}

/*! The span of the pre-signature fields — from the first field to the
    start of the signature — hashed by [tx_signing_hash][]. */
val tx_sig_span : forall 'first_source_off 'first_source_len 'first_content_len 'signature_source_off 'signature_source_len 'signature_content_len,
                       rlp_field_ref_valid('first_source_off, 'first_source_len, 'first_content_len)
                    &  rlp_field_ref_valid('signature_source_off, 'signature_source_len, 'signature_content_len)
                    &  'first_source_off
                    <= 'signature_source_off. (
        RlpFieldRef('first_source_off, 'first_source_len, 'first_content_len),
        RlpFieldRef('signature_source_off, 'signature_source_len, 'signature_content_len),
    ) -> StatelessInputSlice
function tx_sig_span(first, signature) = {
    let start = first.source.off;
    let stop = signature.source.off;
    let start_offset = start;
    let stop_offset = stop;
    if stop_offset < start_offset then {
        throw InvalidBlock(RlpDecode)
    } else {
        stateless_input_slice(start_offset, stop_offset - start_offset)
    }
}

/*! Decodes transaction gas structurally. Admission against the executing
    block's correlated gas limits belongs to transaction validation, after the
    envelope has been decoded. */
function rlp_decode_gas forall 'source_off 'source_len 'content_len,
                          rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    transaction_gas
) =
    rlp_decode_uint64(f)

/*! Decodes the payload fields of a legacy transaction. Kept separate from
    the envelope dispatcher so extraction backends compile each transaction
    shape as an independent function. */
function decode_legacy_tx forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    tx : TransactionInputSlice,
    pubkey : StatelessInputSlice,
    sender : address,
    fields : RlpCursor('source_off, 'source_len),
) -> (
    TransactionFields(blob_schedule_inactive_count)
) = {
    let nonce_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, nonce_f.source.len);
    let gp_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, gp_f.source.len);
    let gas_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, gas_f.source.len);
    let to_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, to_f.source.len);
    let value_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, value_f.source.len);
    let data_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, data_f.source.len);
    let v_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, v_f.source.len);
    let r_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, r_f.source.len);
    let s_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, s_f.source.len);
    rlp_cursor_expect_end(fields);
    let v = rlp_decode_word(v_f);
    let gp = rlp_decode_u256(gp_f);
    struct {
        tx_type = LegacyTx,
        sender = sender,
        raw = tx,
        nonce = rlp_decode_u256(nonce_f),
        chain_id = 0,
        gas_limit = rlp_decode_gas(gas_f),
        is_create = to_f.content_len == 0,
        recipient = word_to_address(rlp_decode_word(to_f)),
        value = rlp_decode_u256(value_f),
        input_src = tx_input_span(data_f),
        access_list = EMPTY_ACCESS_LIST_REF,
        max_fee = gp,
        max_blob_fee = ZERO_WORD,
        max_priority_fee = gp,
        authorizations = EMPTY_AUTHORIZATION_LIST_REF,
        blob_hashes = EMPTY_BLOB_HASHES,
        pubkey = pubkey,
        signing_hash = tx_signing_hash(LegacyTx, tx_sig_span(nonce_f, v_f), v),
        sig_v = v,
        sig_r = rlp_decode_u256(r_f),
        sig_s = rlp_decode_u256(s_f),
    }
}

/*! Decodes the payload fields of an EIP-2930 transaction. */
function decode_access_list_tx forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    tx : TransactionInputSlice,
    pubkey : StatelessInputSlice,
    sender : address,
    fields : RlpCursor('source_off, 'source_len),
) -> (
    TransactionFields(blob_schedule_inactive_count)
) = {
    let chain_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, chain_f.source.len);
    let nonce_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, nonce_f.source.len);
    let gp_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, gp_f.source.len);
    let gas_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, gas_f.source.len);
    let to_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, to_f.source.len);
    let value_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, value_f.source.len);
    let data_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, data_f.source.len);
    let al_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, al_f.source.len);
    let v_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, v_f.source.len);
    let r_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, r_f.source.len);
    let s_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, s_f.source.len);
    rlp_cursor_expect_end(fields);
    let v = rlp_decode_word(v_f);
    let gp = rlp_decode_u256(gp_f);
    let access_list = decode_access_list(al_f);
    struct {
        tx_type = AccessListTx,
        sender = sender,
        raw = tx,
        nonce = rlp_decode_u256(nonce_f),
        chain_id = rlp_decode_uint64(chain_f),
        gas_limit = rlp_decode_gas(gas_f),
        is_create = to_f.content_len == 0,
        recipient = word_to_address(rlp_decode_word(to_f)),
        value = rlp_decode_u256(value_f),
        input_src = tx_input_span(data_f),
        access_list = access_list,
        max_fee = gp,
        max_blob_fee = ZERO_WORD,
        max_priority_fee = gp,
        authorizations = EMPTY_AUTHORIZATION_LIST_REF,
        blob_hashes = EMPTY_BLOB_HASHES,
        pubkey = pubkey,
        signing_hash = tx_signing_hash(AccessListTx, tx_sig_span(chain_f, v_f), v),
        sig_v = v,
        sig_r = rlp_decode_u256(r_f),
        sig_s = rlp_decode_u256(s_f),
    }
}

/*! Decodes the payload fields of an EIP-1559 transaction. */
function decode_fee_market_tx forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    tx : TransactionInputSlice,
    pubkey : StatelessInputSlice,
    sender : address,
    fields : RlpCursor('source_off, 'source_len),
) -> (
    TransactionFields(blob_schedule_inactive_count)
) = {
    let chain_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, chain_f.source.len);
    let nonce_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, nonce_f.source.len);
    let mp_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, mp_f.source.len);
    let mf_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, mf_f.source.len);
    let gas_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, gas_f.source.len);
    let to_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, to_f.source.len);
    let value_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, value_f.source.len);
    let data_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, data_f.source.len);
    let al_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, al_f.source.len);
    let v_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, v_f.source.len);
    let r_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, r_f.source.len);
    let s_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, s_f.source.len);
    rlp_cursor_expect_end(fields);
    let v = rlp_decode_word(v_f);
    let access_list = decode_access_list(al_f);
    struct {
        tx_type = FeeMarketTx,
        sender = sender,
        raw = tx,
        nonce = rlp_decode_u256(nonce_f),
        chain_id = rlp_decode_uint64(chain_f),
        gas_limit = rlp_decode_gas(gas_f),
        is_create = to_f.content_len == 0,
        recipient = word_to_address(rlp_decode_word(to_f)),
        value = rlp_decode_u256(value_f),
        input_src = tx_input_span(data_f),
        access_list = access_list,
        max_fee = rlp_decode_u256(mf_f),
        max_blob_fee = ZERO_WORD,
        max_priority_fee = rlp_decode_u256(mp_f),
        authorizations = EMPTY_AUTHORIZATION_LIST_REF,
        blob_hashes = EMPTY_BLOB_HASHES,
        pubkey = pubkey,
        signing_hash = tx_signing_hash(FeeMarketTx, tx_sig_span(chain_f, v_f), v),
        sig_v = v,
        sig_r = rlp_decode_u256(r_f),
        sig_s = rlp_decode_u256(s_f),
    }
}

/*! Decodes the payload fields of an EIP-4844 transaction. */
function decode_blob_tx forall 'source_off 'source_len 'blob_limit,
                          source_valid_range('source_off, 'source_len) & transaction_blob_limit_value('blob_limit). (
    tx : TransactionInputSlice,
    pubkey : StatelessInputSlice,
    blob_limit : int('blob_limit),
    sender : address,
    fields : RlpCursor('source_off, 'source_len),
) -> (
    TransactionFields('blob_limit)
) = {
    let chain_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, chain_f.source.len);
    let nonce_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, nonce_f.source.len);
    let mp_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, mp_f.source.len);
    let mf_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, mf_f.source.len);
    let gas_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, gas_f.source.len);
    let to_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, to_f.source.len);
    let value_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, value_f.source.len);
    let data_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, data_f.source.len);
    let al_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, al_f.source.len);
    let mbf_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, mbf_f.source.len);
    let bh_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, bh_f.source.len);
    let v_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, v_f.source.len);
    let r_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, r_f.source.len);
    let s_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, s_f.source.len);
    rlp_cursor_expect_end(fields);
    let v = rlp_decode_word(v_f);
    let access_list = decode_access_list(al_f);
    let blob_hashes = decode_blob_hashes(bh_f, blob_limit);
    struct {
        tx_type = BlobTx,
        sender = sender,
        raw = tx,
        nonce = rlp_decode_u256(nonce_f),
        chain_id = rlp_decode_uint64(chain_f),
        gas_limit = rlp_decode_gas(gas_f),
        is_create = to_f.content_len == 0,
        recipient = word_to_address(rlp_decode_word(to_f)),
        value = rlp_decode_u256(value_f),
        input_src = tx_input_span(data_f),
        access_list = access_list,
        max_fee = rlp_decode_u256(mf_f),
        max_blob_fee = rlp_decode_u256(mbf_f),
        max_priority_fee = rlp_decode_u256(mp_f),
        authorizations = EMPTY_AUTHORIZATION_LIST_REF,
        blob_hashes = blob_hashes,
        pubkey = pubkey,
        signing_hash = tx_signing_hash(BlobTx, tx_sig_span(chain_f, v_f), v),
        sig_v = v,
        sig_r = rlp_decode_u256(r_f),
        sig_s = rlp_decode_u256(s_f),
    }
}

/*! Decodes the payload fields of an EIP-7702 transaction. */
function decode_set_code_tx forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    tx : TransactionInputSlice,
    pubkey : StatelessInputSlice,
    sender : address,
    fields : RlpCursor('source_off, 'source_len),
) -> (
    TransactionFields(blob_schedule_inactive_count)
) = {
    let chain_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, chain_f.source.len);
    let nonce_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, nonce_f.source.len);
    let mp_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, mp_f.source.len);
    let mf_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, mf_f.source.len);
    let gas_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, gas_f.source.len);
    let to_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, to_f.source.len);
    let value_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, value_f.source.len);
    let data_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, data_f.source.len);
    let al_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, al_f.source.len);
    let auth_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, auth_f.source.len);
    let v_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, v_f.source.len);
    let r_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, r_f.source.len);
    let s_f = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, s_f.source.len);
    rlp_cursor_expect_end(fields);
    let v = rlp_decode_word(v_f);
    let access_list = decode_access_list(al_f);
    let authorizations = decode_auth_list(auth_f);
    struct {
        tx_type = SetCodeTx,
        sender = sender,
        raw = tx,
        nonce = word_of_account_nonce(rlp_decode_uint64(nonce_f)),
        chain_id = rlp_decode_uint64(chain_f),
        gas_limit = rlp_decode_gas(gas_f),
        is_create = to_f.content_len == 0,
        recipient = word_to_address(rlp_decode_word(to_f)),
        value = rlp_decode_u256(value_f),
        input_src = tx_input_span(data_f),
        access_list = access_list,
        max_fee = rlp_decode_u256(mf_f),
        max_blob_fee = ZERO_WORD,
        max_priority_fee = rlp_decode_u256(mp_f),
        authorizations = authorizations,
        blob_hashes = EMPTY_BLOB_HASHES,
        pubkey = pubkey,
        signing_hash = tx_signing_hash(SetCodeTx, tx_sig_span(chain_f, v_f), v),
        sig_v = v,
        sig_r = rlp_decode_u256(r_f),
        sig_s = rlp_decode_u256(s_f),
    }
}

/*! Decodes one transaction from its raw envelope bytes — standalone and
    purely structural. `tx` is the EIP-2718 envelope as a byte slice (any
    source); `pubkey` the 65-byte witness public key (`0x04 ++ X ++ Y`)
    claimed for its sender. The sender derives from the key
    (`keccak256(X ‖ Y)[12:]`) and the signature material (signing hash,
    `v`/`r`/`s`, the key itself) is captured on the transaction;
    authentication happens at execution ([tx_auth_valid][]). No trust
    decision and no fee derivation happens here.

    A leading byte `≥ 0xc0` is a legacy (type-0) RLP list; otherwise it is
    the type byte (`0x01` EIP-2930, `0x02` EIP-1559, `0x03` EIP-4844,
    `0x04` EIP-7702) and the RLP payload starts right after it. Each
    envelope is destructured in one walk against its exact field shape; an
    unknown type byte or wrong arity is undecodable. The data field is
    never materialized: `input_src` carries its (source, offset, length)
    span, bound as the transaction input only when the tx runs. */
val rlp_decode_tx : forall 'tx_off 'tx_len 'public_key_off 'blob_limit,
                         source_valid_range('tx_off, 'tx_len)
                      &  0
                      <= 'tx_len
                      &  'tx_len
                      <= transaction_length_bound
                      &  source_valid_range('public_key_off, 65)
                      &  transaction_blob_limit_value('blob_limit). (
        StatelessInputSliceFields('tx_off, 'tx_len),
        StatelessInputSliceFields('public_key_off, 65),
        int('blob_limit),
    ) -> Transaction
function rlp_decode_tx(tx, pubkey, blob_limit) = {
    let sender = word_to_address(hash_to_word(keccak256(sub_slice(pubkey, 1, PUBLIC_KEY_BODY_LENGTH))));
    let tx_length = tx.len;
    let b0 : byte = if tx_length == 0 then {
            throw InvalidBlock(RlpDecode)
        } else {
            slice_byte(tx, 0)
        };
    let ttype : byte = if b0[7 .. 6] == 0b11 then { 0x00 } else { b0 }; /* 0xc0 = RLP list tag */
    let typed = ttype != 0x00;
    let payload : TransactionInputSlice = if typed then {
            if 1 <= tx_length then {
                sub_slice(tx, 1, tx_length - 1)
            } else {
                throw InvalidBlock(RlpDecode)
            }
        } else {
            tx
        };
    let payload_input : StatelessInputSlice = payload;
    let fields = rlp_node_cursor(payload_input);

    if not_bool(tx_envelope_type_backwards_matches(ttype)) then {
        throw InvalidBlock(RlpDecode)
    };
    let tx_type : TxType = tx_envelope_type(ttype);
    match tx_type {
        LegacyTx => decode_legacy_tx(tx, pubkey, sender, fields),
        AccessListTx => decode_access_list_tx(tx, pubkey, sender, fields),
        FeeMarketTx => decode_fee_market_tx(tx, pubkey, sender, fields),
        BlobTx => decode_blob_tx(tx, pubkey, blob_limit, sender, fields),
        SetCodeTx => decode_set_code_tx(tx, pubkey, sender, fields),
    }
}
