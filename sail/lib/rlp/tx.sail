
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
struct AccessListDecode = {
    addresses : list(address),
    storage_slots : list(StorageKey),
    address_count : item_count,
    slot_count : item_count,
}

let EMPTY_ACCESS_LIST_DECODE : AccessListDecode = struct {
        addresses = [||],
        storage_slots = [||],
        address_count = 0,
        slot_count = 0,
    }

/* Prepend this entry's keys while recursive unwind restores their wire order.
 * The unconsumed key-list content span strictly decreases on every call. */
/*! Decodes the storage keys of one access-list entry while preserving wire
    order during recursive unwind. */
function decode_access_list_keys(cursor : RlpCursor, addr : address, tail : AccessListDecode) -> AccessListDecode =
    if rlp_cursor_empty(cursor) then {
        tail
    } else {
        let (key, next) = rlp_cursor_pop(cursor);
        let storage_key : StorageKey = struct { addr = addr, slot = rlp_ref_word(key) };
        let result = decode_access_list_keys(next, addr, tail);
        if result.slot_count == 2 ^ 64 - 1 then {
            throw InvalidBlock(RlpDecode)
        } else {
            struct {
                addresses = result.addresses,
                storage_slots = storage_key :: result.storage_slots,
                address_count = result.address_count,
                slot_count = protocol_quantity_increment(result.slot_count),
            }
        }
    }
termination_measure decode_access_list_keys(cursor, addr, tail) =
    let ByteQuantity(stop) = cursor.stop in
    let ByteQuantity(current) = cursor.current in
    stop - current

/* The unconsumed access-list content span strictly decreases on every call. */
/*! Decodes the remaining access-list entries into a flat execution view. */
function decode_access_list_entries(cursor : RlpCursor) -> AccessListDecode =
    if rlp_cursor_empty(cursor) then {
        EMPTY_ACCESS_LIST_DECODE
    } else {
        let (entry, next) = rlp_cursor_pop(cursor);
        let (addr_f, entry_fields) = rlp_cursor_pop(rlp_ref_cursor(entry));
        let (keys_f, entry_fields) = rlp_cursor_pop(entry_fields);
        rlp_cursor_expect_end(entry_fields);

        let addr = word_to_address(rlp_ref_word(addr_f));
        let tail = decode_access_list_entries(next);
        let result = decode_access_list_keys(rlp_ref_cursor(keys_f), addr, tail);
        if result.address_count == 2 ^ 64 - 1 then {
            throw InvalidBlock(RlpDecode)
        } else {
            struct {
                addresses = addr :: result.addresses,
                storage_slots = result.storage_slots,
                address_count = protocol_quantity_increment(result.address_count),
                slot_count = result.slot_count,
            }
        }
    }
termination_measure decode_access_list_entries(cursor) =
    let ByteQuantity(stop) = cursor.stop in
    let ByteQuantity(current) = cursor.current in
    stop - current

/*! Decodes an EIP-2930 access list — RLP `[[address, [slot, …]], …]` —
    into the flat representation the transaction and intrinsic-gas
    accounting use: the addresses (one per entry) and the
    `(address, slot)` pairs. */
function decode_access_list(f : RlpFieldRef) -> (list(address), list(StorageKey), item_count, item_count) = {
    let decoded = decode_access_list_entries(rlp_ref_cursor(f));
    (decoded.addresses, decoded.storage_slots, decoded.address_count, decoded.slot_count)
}

/* Validate the canonical RLP list of bytes32 blob versioned hashes once, then
 * retain its encoded content as a fixed-stride source view. Each element is
 * exactly 0xa0 followed by 32 bytes, so BLOBHASH can load item i at 33*i+1. */
/*md
## Constants

Typed transaction decoding uses the fixed RLP widths of versioned blob hashes
and the domain tags and field counts defined by the transaction EIPs.
*/
let BLOB_HASH_RLP_LENGTH : byte_length = ByteQuantity(33)
let BLOB_HASH_LENGTH : byte_length = WORD_BYTE_LENGTH

/* The unconsumed hash-list content span strictly decreases on every call. */
/*! Validates every fixed-width versioned-hash item and returns their count. */
function decode_blob_hash_items(cursor : RlpCursor, count : blob_count) -> blob_count =
    if rlp_cursor_empty(cursor) then {
        count
    } else {
        let (item, next) = rlp_cursor_pop(cursor);
        if    item.is_list
           |  item.full_len
           != BLOB_HASH_RLP_LENGTH
           |  item.content_len
           != BLOB_HASH_LENGTH
           |  slice_byte(item.source, item.full_off)
           != 0xa0 then {
            throw InvalidBlock(RlpDecode)
        };
        if count == 2 ^ 64 - 1 then {
            throw InvalidBlock(RlpDecode)
        };
        decode_blob_hash_items(next, protocol_quantity_increment(count))
    }
termination_measure decode_blob_hash_items(cursor, count) =
    let ByteQuantity(stop) = cursor.stop in
    let ByteQuantity(current) = cursor.current in
    stop - current

/*! Validates the canonical RLP list of `bytes32` blob versioned hashes
    once, then retains its encoded content as a fixed-stride source view:
    each element is exactly `0xa0` followed by 32 bytes, so `BLOBHASH` can
    load item `i` at `33·i + 1`. */
function decode_blob_hashes(f : RlpFieldRef) -> BlobHashes = {
    let bytes = sub_slice(f.source, f.content_off, f.content_len);
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
struct AuthorizationDecode = { authorizations : list(Authorization), count : item_count }

let EMPTY_AUTHORIZATION_DECODE : AuthorizationDecode = struct { authorizations = [||], count = 0 }

/* The unconsumed authorization-list content span strictly decreases on every call. */
/*! Decodes and signature-recovers the remaining EIP-7702 authorization
    tuples in wire order. */
function decode_auth_tuples(cursor : RlpCursor) -> AuthorizationDecode =
    if rlp_cursor_empty(cursor) then {
        EMPTY_AUTHORIZATION_DECODE
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
        let auth_nonce = rlp_ref_uint(nonce_f);
        let y_value = rlp_ref_uint(y_f);
        let y_valid = y_value <= 1;
        let y : y_parity = if y_value == 0 then { 0 } else { 1 };
        let r = rlp_ref_uint_word(r_f);
        let s = rlp_ref_uint_word(s_f);
        let auth_addr = word_to_address(rlp_ref_word(addr_f));
        let (ok, authority) = if y_valid then {
                ecrecover_addr(auth_signing_hash(chain_id, auth_addr, auth_nonce), y, r, s)
            } else {
                (false, ZERO_ADDR)
            };
        let nonce_valid = auth_nonce != 2 ^ 64 - 1;
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
        if tail.count == 2 ^ 64 - 1 then {
            throw InvalidBlock(RlpDecode)
        } else {
            struct {
                authorizations = authorization :: tail.authorizations,
                count = protocol_quantity_increment(tail.count),
            }
        }
    }
termination_measure decode_auth_tuples(cursor) =
    let ByteQuantity(stop) = cursor.stop in
    let ByteQuantity(current) = cursor.current in
    stop - current

/*! Decodes an EIP-7702 authorization list into
    [Authorization][type-Authorization] tuples, recovering each authority.
    Each set-code tuple `[chainId, address, nonce, yParity, r, s]` is
    signed by the authority, whose address is recovered (not witnessed)
    via `ecrecover` over `keccak256(0x05 ++ rlp([chainId, address,
    nonce]))`. `valid_sig` records tuple-local signature validity
    (recoverable, low-`s`, `r`/`y_parity` ranges, incrementable nonce);
    [process_auth][] then applies the chain-id / account nonce / code
    checks. */
function decode_auth_list(f : RlpFieldRef) -> (list(Authorization), item_count) = {
    let decoded = decode_auth_tuples(rlp_ref_cursor(f));
    (decoded.authorizations, decoded.count)
}

/*! The calldata/initcode span of the data field within the envelope. */
function tx_input_span(payload : ByteSlice, data : RlpFieldRef) -> ByteSlice =
    sub_slice(payload, data.content_off, data.content_len)

/*! The span of the pre-signature fields — from the first field to the
    start of the signature — hashed by [tx_signing_hash][]. */
function tx_sig_span(payload : ByteSlice, first : RlpFieldRef, signature : RlpFieldRef) -> ByteSlice = {
    let start = first.full_off;
    let stop = signature.full_off;
    if stop < start then {
        throw InvalidBlock(RlpDecode)
    } else {
        sub_slice(payload, start, stop - start)
    }
}

/*! Decodes transaction gas and enforces the protocol and fork-specific
    transaction bounds. */
function rlp_ref_gas(f : RlpFieldRef, fork : Fork) -> gas = {
    let value = rlp_ref_uint(f);
    if GAS_MAX_VALUE < value then {
        throw InvalidBlock(GasUsedExceedsLimit)
    };
    if (fork >= Osaka) & (OSAKA_TRANSACTION_GAS_LIMIT_VALUE < value) then {
        throw InvalidBlock(GasUsedExceedsLimit)
    } else {
        nat_to_gas(value)
    }
}

/*! Decodes one transaction from its raw envelope bytes — standalone and
    purely structural. `tx` is the EIP-2718 envelope as a byte slice (any
    source); `pubkey` the 65-byte witness public key (`0x04 ++ X ++ Y`)
    claimed for its sender. The sender derives from the key
    (`keccak256(X ‖ Y)[12:]`) and the signature material (signing hash,
    `v`/`r`/`s`, the key itself) is captured on the transaction;
    authentication happens at execution ([tx_auth_ok][]). No trust
    decision and no fee derivation happens here.

    A leading byte `≥ 0xc0` is a legacy (type-0) RLP list; otherwise it is
    the type byte (`0x01` EIP-2930, `0x02` EIP-1559, `0x03` EIP-4844,
    `0x04` EIP-7702) and the RLP payload starts right after it. Each
    envelope is destructured in one walk against its exact field shape; an
    unknown type byte or wrong arity is undecodable. The data field is
    never materialized: `input_src` carries its (source, offset, length)
    span, bound as the transaction input only when the tx runs. */
function rlp_decode_tx(tx : ByteSlice, pubkey : ByteSlice, fork : Fork) -> Transaction = {
    let sender = word_to_address(keccak256(sub_slice(pubkey, BYTE_ONE, PUBLIC_KEY_BODY_LENGTH)));
    let tx_len : byte_length = tx.len;
    let b0 : byte = if tx_len == BYTE_ZERO then {
            throw InvalidBlock(RlpDecode)
        } else {
            slice_byte(tx, BYTE_ZERO)
        };
    let ttype : byte = if b0[7 .. 6] == 0b11 then { 0x00 } else { b0 }; /* 0xc0 = RLP list tag */
    let typed = ttype != 0x00;
    let payload : ByteSlice = if typed then {
            if BYTE_ONE <= tx_len then {
                sub_slice(tx, BYTE_ONE, tx_len - BYTE_ONE)
            } else {
                throw InvalidBlock(RlpDecode)
            }
        } else {
            tx
        };
    let fields = rlp_node_cursor(payload);

    match ttype {
        0x00 => {
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
                is_create = to_f.content_len == BYTE_ZERO,
                recipient = word_to_address(rlp_ref_word(to_f)),
                value = rlp_ref_uint_word(value_f),
                input_src = tx_input_span(payload, data_f),
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
                signing_hash = tx_signing_hash(LegacyTx, tx_sig_span(payload, nonce_f, v_f), v),
                sig_v = v,
                sig_r = rlp_ref_uint_word(r_f),
                sig_s = rlp_ref_uint_word(s_f),
            }
        },
        0x01 => {
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
                chain_id = rlp_ref_uint(chain_f),
                gas_limit = rlp_ref_gas(gas_f, fork),
                is_create = to_f.content_len == BYTE_ZERO,
                recipient = word_to_address(rlp_ref_word(to_f)),
                value = rlp_ref_uint_word(value_f),
                input_src = tx_input_span(payload, data_f),
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
                signing_hash = tx_signing_hash(AccessListTx, tx_sig_span(payload, chain_f, v_f), v),
                sig_v = v,
                sig_r = rlp_ref_uint_word(r_f),
                sig_s = rlp_ref_uint_word(s_f),
            }
        },
        0x02 => {
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
                chain_id = rlp_ref_uint(chain_f),
                gas_limit = rlp_ref_gas(gas_f, fork),
                is_create = to_f.content_len == BYTE_ZERO,
                recipient = word_to_address(rlp_ref_word(to_f)),
                value = rlp_ref_uint_word(value_f),
                input_src = tx_input_span(payload, data_f),
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
                signing_hash = tx_signing_hash(FeeMarketTx, tx_sig_span(payload, chain_f, v_f), v),
                sig_v = v,
                sig_r = rlp_ref_uint_word(r_f),
                sig_s = rlp_ref_uint_word(s_f),
            }
        },
        0x03 => {
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
                chain_id = rlp_ref_uint(chain_f),
                gas_limit = rlp_ref_gas(gas_f, fork),
                is_create = to_f.content_len == BYTE_ZERO,
                recipient = word_to_address(rlp_ref_word(to_f)),
                value = rlp_ref_uint_word(value_f),
                input_src = tx_input_span(payload, data_f),
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
                signing_hash = tx_signing_hash(BlobTx, tx_sig_span(payload, chain_f, v_f), v),
                sig_v = v,
                sig_r = rlp_ref_uint_word(r_f),
                sig_s = rlp_ref_uint_word(s_f),
            }
        },
        0x04 => {
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
                nonce = word_of_nat(rlp_ref_uint(nonce_f)),
                chain_id = rlp_ref_uint(chain_f),
                gas_limit = rlp_ref_gas(gas_f, fork),
                is_create = to_f.content_len == BYTE_ZERO,
                recipient = word_to_address(rlp_ref_word(to_f)),
                value = rlp_ref_uint_word(value_f),
                input_src = tx_input_span(payload, data_f),
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
                signing_hash = tx_signing_hash(SetCodeTx, tx_sig_span(payload, chain_f, v_f), v),
                sig_v = v,
                sig_r = rlp_ref_uint_word(r_f),
                sig_s = rlp_ref_uint_word(s_f),
            }
        },
        _ => throw InvalidBlock(RlpDecode), /* unknown envelope type or wrong arity */
    }
}
