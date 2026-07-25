
/*md
# Transaction decoding

The EIP-2718 transaction envelope decoder: per-envelope destructuring into
the [Transaction][type-Transaction] type (one walk, against each
envelope's exact field shape), plus the EIP-2930 access-list
and EIP-7702 authorization-tuple decoders. Standalone and purely
structural: input is a `ByteSlice` envelope and a `ByteSlice` public key
of any source; the signature rules and the cryptography live in their own
modules.
*/
/*! A decoded access list and the counts needed for intrinsic gas. */
struct AccessListDecode('address_bound : Int, 'slot_bound : Int),
    source_valid_length('address_bound) & source_valid_length('slot_bound) = {
    addresses : list(address),
    storage_slots : list(StorageKey),
    address_count : range(0, 'address_bound),
    slot_count : range(0, 'slot_bound),
}

let EMPTY_ACCESS_LIST_DECODE : AccessListDecode(0, 0) = struct {
        addresses = [||],
        storage_slots = [||],
        address_count = 0,
        slot_count = 0,
    }

/*! Advances a structurally bounded decoded-item count. */
val rlp_item_count_increment : forall 'value, source_valid_length('value + 1). int('value) -> int('value + 1)
function rlp_item_count_increment(value) = value + 1

/* Prepend this entry's keys while recursive unwind restores their wire order.
 * The unconsumed key-list content span strictly decreases on every call. */
/*! Decodes the storage keys of one access-list entry while preserving wire
    order during recursive unwind. */
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
        struct {
            addresses = tail.addresses,
            storage_slots = tail.storage_slots,
            address_count = tail.address_count,
            slot_count = tail.slot_count,
        }
    } else {
        let (key, next) = rlp_cursor_pop(cursor);
        let storage_key : StorageKey = struct { addr = addr, slot = rlp_ref_word(key) };
        let result = decode_access_list_keys(next, addr, tail);
        struct {
            addresses = result.addresses,
            storage_slots = storage_key :: result.storage_slots,
            address_count = result.address_count,
            slot_count = rlp_item_count_increment(result.slot_count),
        }
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
        struct { addresses = [||], storage_slots = [||], address_count = 0, slot_count = 0 }
    } else {
        let (entry, next) = rlp_cursor_pop(cursor);
        let (addr_f, entry_fields) = rlp_cursor_pop(rlp_ref_cursor(entry));
        let (keys_f, entry_fields) = rlp_cursor_pop(entry_fields);
        rlp_cursor_expect_end(entry_fields);

        let addr = word_to_address(rlp_ref_word(addr_f));
        let tail = decode_access_list_entries(next);
        let result = decode_access_list_keys(rlp_ref_cursor(keys_f), addr, tail);
        struct {
            addresses = addr :: result.addresses,
            storage_slots = result.storage_slots,
            address_count = rlp_item_count_increment(result.address_count),
            slot_count = result.slot_count,
        }
    }
termination_measure decode_access_list_entries(cursor) =
    cursor.len

/*! Decodes an EIP-2930 access list — RLP `[[address, [slot, …]], …]` —
    into the flat representation the transaction and intrinsic-gas
    accounting use: the addresses (one per entry) and the
    `(address, slot)` pairs. */
function decode_access_list forall 'source_off 'source_len 'content_len,
                              rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    (list(address), list(StorageKey), transaction_item_count, transaction_item_count)
) = {
    if f.source.len <= sizeof(transaction_length_bound) then {
        let decoded = decode_access_list_entries(rlp_ref_cursor(f));
        (decoded.addresses, decoded.storage_slots, decoded.address_count, decoded.slot_count)
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
/*! Validates every fixed-width versioned-hash item and returns their count. */
function decode_blob_hash_items forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    cursor : RlpCursor('source_off, 'source_len),
    count : transaction_blob_count,
) -> (
    transaction_blob_count
) =
    if cursor.len == 0 then {
        count
    } else {
        let (item, next) = rlp_cursor_pop(cursor);
        if    item.is_list
           |  item.source.len
           != BLOB_HASH_RLP_LENGTH
           |  item.content_len
           != BLOB_HASH_LENGTH
           |  slice_byte(item.source, 0)
           != 0xa0 then {
            throw InvalidBlock(RlpDecode)
        };
        if count < sizeof(transaction_blob_count_bound) then {
            decode_blob_hash_items(next, count + 1)
        } else {
            throw InvalidBlock(RlpDecode)
        }
    }
termination_measure decode_blob_hash_items(cursor, count) =
    cursor.len

/*! Validates the canonical RLP list of `bytes32` blob versioned hashes
    once, then retains its encoded content as a fixed-stride source view:
    each element is exactly `0xa0` followed by 32 bytes, so `BLOBHASH` can
    load item `i` at `33·i + 1`. */
function decode_blob_hashes forall 'source_off 'source_len 'content_len,
                              rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    BlobHashes
) = {
    let bytes = rlp_ref_content(f);
    struct { bytes = bytes, count = decode_blob_hash_items(rlp_ref_cursor(f), 0) }
}

/* ---- EIP-7702 authorization-tuple recovery ---------------------------------
 * Each set-code authorization tuple [chainId, address, nonce, yParity, r, s] is
 * signed by the AUTHORITY, whose address is recovered (not witnessed) via
 * ecrecover over keccak(0x05 ++ rlp([chainId, address, nonce])). */

/* decode an EIP-7702 authorization list into Authorizations,
 * recovering each authority. valid_sig records tuple-local signature validity
 * (recoverable, low-s, r/y parity ranges, incrementable nonce); process_auth then
 * applies the chain-id / account nonce / code checks. */
/*! A decoded EIP-7702 authorization sequence and its item count. */
struct AuthorizationDecode('bound : Int), source_valid_length('bound) = {
    authorizations : list(Authorization),
    count : range(0, 'bound),
}

let EMPTY_AUTHORIZATION_DECODE : AuthorizationDecode(0) = struct { authorizations = [||], count = 0 }

/* The unconsumed authorization-list content span strictly decreases on every call. */
/*! Decodes and signature-recovers the remaining EIP-7702 authorization
    tuples in wire order. */
val decode_auth_tuples : forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). RlpCursor(
        'source_off,
        'source_len,
    ) -> AuthorizationDecode('source_len)

function decode_auth_tuples(cursor) =
    if cursor.len == 0 then {
        struct { authorizations = [||], count = 0 }
    } else {
        let (tuple, next) = rlp_cursor_pop(cursor);
        let (chain_f, fields) = rlp_cursor_pop(rlp_ref_cursor(tuple));
        let (addr_f, fields) = rlp_cursor_pop(fields);
        let (nonce_f, fields) = rlp_cursor_pop(fields);
        let (y_f, fields) = rlp_cursor_pop(fields);
        let (r_f, fields) = rlp_cursor_pop(fields);
        let (s_f, fields) = rlp_cursor_pop(fields);
        rlp_cursor_expect_end(fields);

        let chain_id = rlp_ref_uint_word(chain_f);
        let auth_nonce : account_nonce = rlp_ref_uint64(nonce_f);
        let y_value = rlp_ref_uint64(y_f);
        let y_valid = y_value <= 1;
        let y : y_parity = if y_value == 0 then { 0 } else { 1 };
        let r = rlp_ref_uint_word(r_f);
        let s = rlp_ref_uint_word(s_f);
        let auth_addr = word_to_address(rlp_ref_word(addr_f));
        let (ok, authority) = if y_valid then {
                ecrecover_addr(auth_signing_hash(chain_id, auth_addr, auth_nonce), y, r, s)
            } else {
                (false, ZERO_ADDRESS)
            };
        let nonce_valid = auth_nonce != sizeof(account_nonce_bound);
        let authorization : Authorization = struct {
                valid_sig =
                      ok
                    & y_valid
                    & word_ult(ZERO_WORD, r)
                    & word_ult(r, SECP_N_FULL)
                    & word_ult(ZERO_WORD, s)
                    & word_ule(s, SECP_N_HALF)
                    & nonce_valid,
                authority = authority,
                address = auth_addr,
                nonce = auth_nonce,
                chain_id = chain_id,
            };
        let tail = decode_auth_tuples(next);
        struct { authorizations = authorization :: tail.authorizations, count = rlp_item_count_increment(tail.count) }
    }
termination_measure decode_auth_tuples(cursor) =
    cursor.len

/*! Decodes an EIP-7702 authorization list into
    [Authorization][type-Authorization] tuples, recovering each authority.
    Each set-code tuple `[chainId, address, nonce, yParity, r, s]` is
    signed by the authority, whose address is recovered (not witnessed)
    via `ecrecover` over `keccak256(0x05 ++ rlp([chainId, address,
    nonce]))`. `valid_sig` records tuple-local signature validity
    (recoverable, low-`s`, `r`/`y_parity` ranges, incrementable nonce);
    [process_auth][] then applies the chain-id / account nonce / code
    checks. */
function decode_auth_list forall 'source_off 'source_len 'content_len,
                            rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    (list(Authorization), transaction_item_count)
) = {
    if f.source.len <= sizeof(transaction_length_bound) then {
        let decoded = decode_auth_tuples(rlp_ref_cursor(f));
        (decoded.authorizations, decoded.count)
    } else {
        throw InvalidBlock(RlpDecode)
    }
}

/*! The calldata/initcode span of the data field within the envelope. The RLP
    cursor existentially hides its bounded source length, so this boundary
    reifies the already-established transaction-envelope invariant. */
function tx_input_span forall 'source_off 'source_len 'content_len,
                         rlp_field_ref_valid('source_off, 'source_len, 'content_len). (data :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    TransactionByteSlice
) = {
    let content = rlp_ref_content(data);
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
    ) -> ByteSlice
function tx_sig_span(first, signature) = {
    let start = first.source.off;
    let stop = signature.source.off;
    let start_offset = start;
    let stop_offset = stop;
    if stop_offset < start_offset then {
        throw InvalidBlock(RlpDecode)
    } else {
        byte_slice(first.source.source, start_offset, stop_offset - start_offset)
    }
}

/*! Decodes transaction gas and enforces the protocol and fork-specific
    transaction bounds. */
function rlp_ref_gas forall 'source_off 'source_len 'content_len,
                       rlp_field_ref_valid('source_off, 'source_len, 'content_len). (
    f : RlpFieldRef('source_off, 'source_len, 'content_len),
    fork : Fork,
) -> (
    transaction_gas
) = {
    let value = rlp_ref_uint(f);
    if (fork >= Osaka) & (fork < Amsterdam) & (OSAKA_TRANSACTION_GAS_LIMIT_VALUE < value) then {
        throw InvalidBlock(GasUsedExceedsLimit)
    } else {
        value
    }
}

/*! Decodes the payload fields of a legacy transaction. Kept separate from
    the envelope dispatcher so extraction backends compile each transaction
    shape as an independent function. */
function decode_legacy_tx forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    tx : TransactionByteSlice,
    pubkey : ByteSlice,
    fork : Fork,
    sender : address,
    fields : RlpCursor('source_off, 'source_len),
) -> (
    Transaction
) = {
    let (nonce_f, fields) = rlp_cursor_pop(fields);
    let (gp_f, fields) = rlp_cursor_pop(fields);
    let (gas_f, fields) = rlp_cursor_pop(fields);
    let (to_f, fields) = rlp_cursor_pop(fields);
    let (value_f, fields) = rlp_cursor_pop(fields);
    let (data_f, fields) = rlp_cursor_pop(fields);
    let (v_f, fields) = rlp_cursor_pop(fields);
    let (r_f, fields) = rlp_cursor_pop(fields);
    let (s_f, fields) = rlp_cursor_pop(fields);
    rlp_cursor_expect_end(fields);
    let v = rlp_ref_word(v_f);
    let gp = rlp_ref_uint_word(gp_f);
    struct {
        tx_type = LegacyTx,
        sender = sender,
        raw = tx,
        nonce = rlp_ref_uint_word(nonce_f),
        chain_id = 0,
        gas_limit = rlp_ref_gas(gas_f, fork),
        is_create = to_f.content_len == 0,
        recipient = word_to_address(rlp_ref_word(to_f)),
        value = rlp_ref_uint_word(value_f),
        input_src = tx_input_span(data_f),
        access_list_addresses = [||],
        access_list_address_count = 0,
        access_list_slots = [||],
        access_list_slot_count = 0,
        max_fee = gp,
        max_blob_fee = ZERO_WORD,
        max_priority_fee = gp,
        authorizations = [||],
        authorization_count = 0,
        blob_hashes = EMPTY_BLOB_HASHES,
        pubkey = pubkey,
        signing_hash = tx_signing_hash(LegacyTx, tx_sig_span(nonce_f, v_f), v),
        sig_v = v,
        sig_r = rlp_ref_uint_word(r_f),
        sig_s = rlp_ref_uint_word(s_f),
    }
}

/*! Decodes the payload fields of an EIP-2930 transaction. */
function decode_access_list_tx forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    tx : TransactionByteSlice,
    pubkey : ByteSlice,
    fork : Fork,
    sender : address,
    fields : RlpCursor('source_off, 'source_len),
) -> (
    Transaction
) = {
    let (chain_f, fields) = rlp_cursor_pop(fields);
    let (nonce_f, fields) = rlp_cursor_pop(fields);
    let (gp_f, fields) = rlp_cursor_pop(fields);
    let (gas_f, fields) = rlp_cursor_pop(fields);
    let (to_f, fields) = rlp_cursor_pop(fields);
    let (value_f, fields) = rlp_cursor_pop(fields);
    let (data_f, fields) = rlp_cursor_pop(fields);
    let (al_f, fields) = rlp_cursor_pop(fields);
    let (v_f, fields) = rlp_cursor_pop(fields);
    let (r_f, fields) = rlp_cursor_pop(fields);
    let (s_f, fields) = rlp_cursor_pop(fields);
    rlp_cursor_expect_end(fields);
    let v = rlp_ref_word(v_f);
    let gp = rlp_ref_uint_word(gp_f);
    let (al_addrs, al_slots, al_addr_count, al_slot_count) = decode_access_list(al_f);
    struct {
        tx_type = AccessListTx,
        sender = sender,
        raw = tx,
        nonce = rlp_ref_uint_word(nonce_f),
        chain_id = rlp_ref_uint64(chain_f),
        gas_limit = rlp_ref_gas(gas_f, fork),
        is_create = to_f.content_len == 0,
        recipient = word_to_address(rlp_ref_word(to_f)),
        value = rlp_ref_uint_word(value_f),
        input_src = tx_input_span(data_f),
        access_list_addresses = al_addrs,
        access_list_address_count = al_addr_count,
        access_list_slots = al_slots,
        access_list_slot_count = al_slot_count,
        max_fee = gp,
        max_blob_fee = ZERO_WORD,
        max_priority_fee = gp,
        authorizations = [||],
        authorization_count = 0,
        blob_hashes = EMPTY_BLOB_HASHES,
        pubkey = pubkey,
        signing_hash = tx_signing_hash(AccessListTx, tx_sig_span(chain_f, v_f), v),
        sig_v = v,
        sig_r = rlp_ref_uint_word(r_f),
        sig_s = rlp_ref_uint_word(s_f),
    }
}

/*! Decodes the payload fields of an EIP-1559 transaction. */
function decode_fee_market_tx forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    tx : TransactionByteSlice,
    pubkey : ByteSlice,
    fork : Fork,
    sender : address,
    fields : RlpCursor('source_off, 'source_len),
) -> (
    Transaction
) = {
    let (chain_f, fields) = rlp_cursor_pop(fields);
    let (nonce_f, fields) = rlp_cursor_pop(fields);
    let (mp_f, fields) = rlp_cursor_pop(fields);
    let (mf_f, fields) = rlp_cursor_pop(fields);
    let (gas_f, fields) = rlp_cursor_pop(fields);
    let (to_f, fields) = rlp_cursor_pop(fields);
    let (value_f, fields) = rlp_cursor_pop(fields);
    let (data_f, fields) = rlp_cursor_pop(fields);
    let (al_f, fields) = rlp_cursor_pop(fields);
    let (v_f, fields) = rlp_cursor_pop(fields);
    let (r_f, fields) = rlp_cursor_pop(fields);
    let (s_f, fields) = rlp_cursor_pop(fields);
    rlp_cursor_expect_end(fields);
    let v = rlp_ref_word(v_f);
    let (al_addrs, al_slots, al_addr_count, al_slot_count) = decode_access_list(al_f);
    struct {
        tx_type = FeeMarketTx,
        sender = sender,
        raw = tx,
        nonce = rlp_ref_uint_word(nonce_f),
        chain_id = rlp_ref_uint64(chain_f),
        gas_limit = rlp_ref_gas(gas_f, fork),
        is_create = to_f.content_len == 0,
        recipient = word_to_address(rlp_ref_word(to_f)),
        value = rlp_ref_uint_word(value_f),
        input_src = tx_input_span(data_f),
        access_list_addresses = al_addrs,
        access_list_address_count = al_addr_count,
        access_list_slots = al_slots,
        access_list_slot_count = al_slot_count,
        max_fee = rlp_ref_uint_word(mf_f),
        max_blob_fee = ZERO_WORD,
        max_priority_fee = rlp_ref_uint_word(mp_f),
        authorizations = [||],
        authorization_count = 0,
        blob_hashes = EMPTY_BLOB_HASHES,
        pubkey = pubkey,
        signing_hash = tx_signing_hash(FeeMarketTx, tx_sig_span(chain_f, v_f), v),
        sig_v = v,
        sig_r = rlp_ref_uint_word(r_f),
        sig_s = rlp_ref_uint_word(s_f),
    }
}

/*! Decodes the payload fields of an EIP-4844 transaction. */
function decode_blob_tx forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    tx : TransactionByteSlice,
    pubkey : ByteSlice,
    fork : Fork,
    sender : address,
    fields : RlpCursor('source_off, 'source_len),
) -> (
    Transaction
) = {
    let (chain_f, fields) = rlp_cursor_pop(fields);
    let (nonce_f, fields) = rlp_cursor_pop(fields);
    let (mp_f, fields) = rlp_cursor_pop(fields);
    let (mf_f, fields) = rlp_cursor_pop(fields);
    let (gas_f, fields) = rlp_cursor_pop(fields);
    let (to_f, fields) = rlp_cursor_pop(fields);
    let (value_f, fields) = rlp_cursor_pop(fields);
    let (data_f, fields) = rlp_cursor_pop(fields);
    let (al_f, fields) = rlp_cursor_pop(fields);
    let (mbf_f, fields) = rlp_cursor_pop(fields);
    let (bh_f, fields) = rlp_cursor_pop(fields);
    let (v_f, fields) = rlp_cursor_pop(fields);
    let (r_f, fields) = rlp_cursor_pop(fields);
    let (s_f, fields) = rlp_cursor_pop(fields);
    rlp_cursor_expect_end(fields);
    let v = rlp_ref_word(v_f);
    let (al_addrs, al_slots, al_addr_count, al_slot_count) = decode_access_list(al_f);
    let blob_hashes = decode_blob_hashes(bh_f);
    struct {
        tx_type = BlobTx,
        sender = sender,
        raw = tx,
        nonce = rlp_ref_uint_word(nonce_f),
        chain_id = rlp_ref_uint64(chain_f),
        gas_limit = rlp_ref_gas(gas_f, fork),
        is_create = to_f.content_len == 0,
        recipient = word_to_address(rlp_ref_word(to_f)),
        value = rlp_ref_uint_word(value_f),
        input_src = tx_input_span(data_f),
        access_list_addresses = al_addrs,
        access_list_address_count = al_addr_count,
        access_list_slots = al_slots,
        access_list_slot_count = al_slot_count,
        max_fee = rlp_ref_uint_word(mf_f),
        max_blob_fee = rlp_ref_uint_word(mbf_f),
        max_priority_fee = rlp_ref_uint_word(mp_f),
        authorizations = [||],
        authorization_count = 0,
        blob_hashes = blob_hashes,
        pubkey = pubkey,
        signing_hash = tx_signing_hash(BlobTx, tx_sig_span(chain_f, v_f), v),
        sig_v = v,
        sig_r = rlp_ref_uint_word(r_f),
        sig_s = rlp_ref_uint_word(s_f),
    }
}

/*! Decodes the payload fields of an EIP-7702 transaction. */
function decode_set_code_tx forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    tx : TransactionByteSlice,
    pubkey : ByteSlice,
    fork : Fork,
    sender : address,
    fields : RlpCursor('source_off, 'source_len),
) -> (
    Transaction
) = {
    let (chain_f, fields) = rlp_cursor_pop(fields);
    let (nonce_f, fields) = rlp_cursor_pop(fields);
    let (mp_f, fields) = rlp_cursor_pop(fields);
    let (mf_f, fields) = rlp_cursor_pop(fields);
    let (gas_f, fields) = rlp_cursor_pop(fields);
    let (to_f, fields) = rlp_cursor_pop(fields);
    let (value_f, fields) = rlp_cursor_pop(fields);
    let (data_f, fields) = rlp_cursor_pop(fields);
    let (al_f, fields) = rlp_cursor_pop(fields);
    let (auth_f, fields) = rlp_cursor_pop(fields);
    let (v_f, fields) = rlp_cursor_pop(fields);
    let (r_f, fields) = rlp_cursor_pop(fields);
    let (s_f, fields) = rlp_cursor_pop(fields);
    rlp_cursor_expect_end(fields);
    let v = rlp_ref_word(v_f);
    let (al_addrs, al_slots, al_addr_count, al_slot_count) = decode_access_list(al_f);
    let (authorizations, authorization_count) = decode_auth_list(auth_f);
    struct {
        tx_type = SetCodeTx,
        sender = sender,
        raw = tx,
        nonce = word_of_account_nonce(rlp_ref_uint64(nonce_f)),
        chain_id = rlp_ref_uint64(chain_f),
        gas_limit = rlp_ref_gas(gas_f, fork),
        is_create = to_f.content_len == 0,
        recipient = word_to_address(rlp_ref_word(to_f)),
        value = rlp_ref_uint_word(value_f),
        input_src = tx_input_span(data_f),
        access_list_addresses = al_addrs,
        access_list_address_count = al_addr_count,
        access_list_slots = al_slots,
        access_list_slot_count = al_slot_count,
        max_fee = rlp_ref_uint_word(mf_f),
        max_blob_fee = ZERO_WORD,
        max_priority_fee = rlp_ref_uint_word(mp_f),
        authorizations = authorizations,
        authorization_count = authorization_count,
        blob_hashes = EMPTY_BLOB_HASHES,
        pubkey = pubkey,
        signing_hash = tx_signing_hash(SetCodeTx, tx_sig_span(chain_f, v_f), v),
        sig_v = v,
        sig_r = rlp_ref_uint_word(r_f),
        sig_s = rlp_ref_uint_word(s_f),
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
val rlp_decode_tx : forall 'tx_off 'tx_len 'public_key_off,
                         source_valid_range('tx_off, 'tx_len)
                      &  0
                      <= 'tx_len
                      &  'tx_len
                      <= transaction_length_bound
                      &  source_valid_range('public_key_off, 65). (
        ByteSliceFields('tx_off, 'tx_len),
        ByteSliceFields('public_key_off, 65),
        Fork,
    ) -> Transaction
function rlp_decode_tx(tx, pubkey, fork) = {
    let sender = word_to_address(hash_to_word(keccak256(sub_slice(pubkey, 1, PUBLIC_KEY_BODY_LENGTH))));
    let tx_length = tx.len;
    let b0 : byte = if tx_length == 0 then {
            throw InvalidBlock(RlpDecode)
        } else {
            slice_byte(tx, 0)
        };
    let ttype : byte = if b0[7 .. 6] == 0b11 then { 0x00 } else { b0 }; /* 0xc0 = RLP list tag */
    let typed = ttype != 0x00;
    let payload : TransactionByteSlice = if typed then {
            if 1 <= tx_length then {
                sub_slice(tx, 1, tx_length - 1)
            } else {
                throw InvalidBlock(RlpDecode)
            }
        } else {
            tx
        };
    let fields = rlp_node_cursor(payload);

    match ttype {
        0x00 => decode_legacy_tx(tx, pubkey, fork, sender, fields),
        0x01 => decode_access_list_tx(tx, pubkey, fork, sender, fields),
        0x02 => decode_fee_market_tx(tx, pubkey, fork, sender, fields),
        0x03 => decode_blob_tx(tx, pubkey, fork, sender, fields),
        0x04 => decode_set_code_tx(tx, pubkey, fork, sender, fields),
        _ => throw InvalidBlock(RlpDecode), /* unknown envelope type or wrong arity */
    }
}
