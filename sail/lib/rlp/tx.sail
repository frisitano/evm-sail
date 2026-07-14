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

/*! Decodes an EIP-2930 access list — RLP `[[address, [slot, …]], …]` —
    into the flat representation the transaction and intrinsic-gas
    accounting use: the addresses (one per entry) and the
    `(address, slot)` pairs. */
function decode_access_list(f : RlpFieldRef) -> (list(address), list(StorageKey), int, int) = {
    var addresses_rev : list(address) = [||];
    var storage_slots_rev : list(StorageKey) = [||];
    var address_count : int = 0;
    var slot_count : int = 0;
    var entries = rlp_ref_cursor(f);
    while not_bool(rlp_cursor_empty(entries)) do {
        let (entry, next_entry) = rlp_cursor_pop(entries);
        entries = next_entry;

        let (addr_f, entry_fields) = rlp_cursor_pop(rlp_ref_cursor(entry));
        let (keys_f, entry_fields) = rlp_cursor_pop(entry_fields);
        rlp_cursor_expect_end(entry_fields);

        let addr = word_to_address(rlp_ref_word(addr_f));
        addresses_rev = addr :: addresses_rev;
        address_count = address_count + 1;

        var keys = rlp_ref_cursor(keys_f);
        while not_bool(rlp_cursor_empty(keys)) do {
            let (key, next_key) = rlp_cursor_pop(keys);
            keys = next_key;
            storage_slots_rev = struct { addr = addr, slot = rlp_ref_word(key) } :: storage_slots_rev;
            slot_count = slot_count + 1
        }
    };
    (list_reverse(addresses_rev), list_reverse(storage_slots_rev), address_count, slot_count)
}

/*! Validates the canonical RLP list of `bytes32` blob versioned hashes
    once, then retains its encoded content as a fixed-stride source view:
    each element is exactly `0xa0` followed by 32 bytes, so `BLOBHASH` can
    load item `i` at `33·i + 1`. */
function decode_blob_hashes(f : RlpFieldRef) -> BlobHashes = {
    let bytes : ByteSlice = match f.source {
        BytesSlice(source) => sub_slice(source, unsigned(f.content_off), unsigned(f.content_len)),
        BytesList(_) => throw InvalidBlock(RlpDecode),
    };
    var count : int = 0;
    var items = rlp_ref_cursor(f);
    while not_bool(rlp_cursor_empty(items)) do {
        let (item, next) = rlp_cursor_pop(items);
        items = next;
        if    item.is_list
           |  unsigned(item.full_len)
           != 33
           |  unsigned(item.content_len)
           != 32
           |  bytes_at(item.source, unsigned(item.full_off))
           != 0xa0 then {
            throw InvalidBlock(RlpDecode)
        };
        count = count + 1
    };
    struct { bytes = bytes, count = count }
}

/*! Decodes an EIP-7702 authorization list into
    [Authorization][type-Authorization] tuples, recovering each authority.
    Each set-code tuple `[chainId, address, nonce, yParity, r, s]` is
    signed by the authority, whose address is recovered (not witnessed)
    via `ecrecover` over `keccak256(0x05 ++ rlp([chainId, address,
    nonce]))`. `valid_sig` records tuple-local signature validity
    (recoverable, low-`s`, `r`/`y_parity` ranges, incrementable nonce);
    [process_auth][] then applies the chain-id / account nonce / code
    checks. */
function decode_auth_list(f : RlpFieldRef) -> (list(Authorization), int) = {
    var out_rev : list(Authorization) = [||];
    var count : int = 0;
    var tuples = rlp_ref_cursor(f);
    while not_bool(rlp_cursor_empty(tuples)) do {
        let (tuple, next_tuple) = rlp_cursor_pop(tuples);
        tuples = next_tuple;

        let (chain_f, fields) = rlp_cursor_pop(rlp_ref_cursor(tuple));
        let (addr_f, fields) = rlp_cursor_pop(fields);
        let (nonce_f, fields) = rlp_cursor_pop(fields);
        let (y_f, fields) = rlp_cursor_pop(fields);
        let (r_f, fields) = rlp_cursor_pop(fields);
        let (s_f, fields) = rlp_cursor_pop(fields);
        rlp_cursor_expect_end(fields);

        let chain_id = rlp_ref_int(chain_f);
        let auth_nonce = rlp_ref_int(nonce_f);
        let y = rlp_ref_int(y_f);
        let r = rlp_ref_uint_word(r_f);
        let s = rlp_ref_uint_word(s_f);
        let auth_addr = word_to_address(rlp_ref_word(addr_f));
        let (ok, authority) = ecrecover_addr(auth_signing_hash(chain_id, auth_addr, auth_nonce), y, r, s);
        out_rev =
            struct {
                valid_sig =
                      ok
                    & (y == 0 | y == 1)
                    & (unsigned(r) >= 1)
                    & (unsigned(r) < unsigned(SECP_N_FULL))
                    & (unsigned(s) >= 1)
                    & (unsigned(s) <= unsigned(SECP_N_HALF))
                    & (auth_nonce < 18446744073709551615),
                authority = authority,
                address = auth_addr,
                nonce = auth_nonce,
                chain_id = chain_id,
            } ::
                out_rev;
        count = count + 1
    };
    (list_reverse(out_rev), count)
}

/*! The calldata/initcode span of the data field within the envelope. */
function tx_input_span(payload : ByteSlice, data : RlpFieldRef) -> ByteSlice =
    sub_slice(payload, unsigned(data.content_off), unsigned(data.content_len))

/*! The span of the pre-signature fields — from the first field to the
    start of the signature — hashed by [tx_signing_hash][]. */
function tx_sig_span(payload : ByteSlice, first : RlpFieldRef, signature : RlpFieldRef) -> ByteSlice = {
    let start = unsigned(first.full_off);
    let stop = unsigned(signature.full_off);
    if stop < start then {
        throw InvalidBlock(RlpDecode)
    } else {
        sub_slice(payload, start, stop - start)
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
function rlp_decode_tx(tx : ByteSlice, pubkey : ByteSlice) -> Transaction = {
    let sender = word_to_address(keccak256(sub_slice(pubkey, 1, 64)));
    let tx_len = unsigned(tx.len);

    let b0 : int = if tx_len == 0 then {
            throw InvalidBlock(RlpDecode)
        } else {
            unsigned(slice_byte(tx, 0))
        };
    let ttype : int = if b0 >= 192 then { 0 } else { b0 }; /* 0xc0 = RLP list tag */
    let typed = ttype != 0;
    let payload : ByteSlice = if typed then {
            sub_slice(tx, 1, tx_len - 1)
        } else {
            tx
        };
    let fields = rlp_node_refs(BytesSlice(payload));

    match (ttype, fields) {
        (0, nonce_f :: (gp_f :: (gas_f :: (to_f :: (value_f :: (data_f :: (v_f :: (r_f :: (s_f :: [||]))))))))) => {
            let v = rlp_ref_int(v_f);
            let gp = rlp_ref_uint_word(gp_f);
            struct {
                tx_type = LegacyTx,
                sender = sender,
                raw = tx,
                nonce = rlp_ref_int(nonce_f),
                chain_id = 0,
                gas_limit = to_gas(rlp_ref_int(gas_f)),
                is_create = unsigned(to_f.content_len) == 0,
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
        (
            1,
            chain_f ::
                (nonce_f ::
                    (gp_f :: (gas_f :: (to_f :: (value_f :: (data_f :: (al_f :: (v_f :: (r_f :: (s_f :: [||])))))))))),
        ) => {
            let v = rlp_ref_int(v_f);
            let gp = rlp_ref_uint_word(gp_f);
            let (al_addrs, al_slots, al_addr_count, al_slot_count) = decode_access_list(al_f);
            struct {
                tx_type = AccessListTx,
                sender = sender,
                raw = tx,
                nonce = rlp_ref_int(nonce_f),
                chain_id = rlp_ref_int(chain_f),
                gas_limit = to_gas(rlp_ref_int(gas_f)),
                is_create = unsigned(to_f.content_len) == 0,
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
        (
            2,
            chain_f ::
                (nonce_f ::
                    (mp_f ::
                        (mf_f :: (gas_f :: (to_f :: (value_f :: (data_f :: (al_f :: (v_f :: (r_f :: (s_f :: [||]))))))))))),
        ) => {
            let v = rlp_ref_int(v_f);
            let (al_addrs, al_slots, al_addr_count, al_slot_count) = decode_access_list(al_f);
            struct {
                tx_type = FeeMarketTx,
                sender = sender,
                raw = tx,
                nonce = rlp_ref_int(nonce_f),
                chain_id = rlp_ref_int(chain_f),
                gas_limit = to_gas(rlp_ref_int(gas_f)),
                is_create = unsigned(to_f.content_len) == 0,
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
        (
            3,
            chain_f ::
                (nonce_f ::
                    (mp_f ::
                        (mf_f ::
                            (gas_f ::
                                (to_f ::
                                    (value_f ::
                                        (data_f :: (al_f :: (mbf_f :: (bh_f :: (v_f :: (r_f :: (s_f :: [||]))))))))))))),
        ) => {
            let v = rlp_ref_int(v_f);
            let (al_addrs, al_slots, al_addr_count, al_slot_count) = decode_access_list(al_f);
            let blob_hashes = decode_blob_hashes(bh_f);
            struct {
                tx_type = BlobTx,
                sender = sender,
                raw = tx,
                nonce = rlp_ref_int(nonce_f),
                chain_id = rlp_ref_int(chain_f),
                gas_limit = to_gas(rlp_ref_int(gas_f)),
                is_create = unsigned(to_f.content_len) == 0,
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
        (
            4,
            chain_f ::
                (nonce_f ::
                    (mp_f ::
                        (mf_f ::
                            (gas_f ::
                                (to_f :: (value_f :: (data_f :: (al_f :: (auth_f :: (v_f :: (r_f :: (s_f :: [||])))))))))))),
        ) => {
            let v = rlp_ref_int(v_f);
            let (al_addrs, al_slots, al_addr_count, al_slot_count) = decode_access_list(al_f);
            let (authorizations, authorization_count) = decode_auth_list(auth_f);
            struct {
                tx_type = SetCodeTx,
                sender = sender,
                raw = tx,
                nonce = rlp_ref_int(nonce_f),
                chain_id = rlp_ref_int(chain_f),
                gas_limit = to_gas(rlp_ref_int(gas_f)),
                is_create = unsigned(to_f.content_len) == 0,
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
