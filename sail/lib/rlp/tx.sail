/* ======================================================================== */
/* RLP: TRANSACTION DECODE                                                  */
/*                                                                          */
/* The EIP-2718 transaction envelope decoder: per-envelope destructuring    */
/* into the Transaction type (one walk, exact field shape, mirroring        */
/* EELS's per-envelope dataclasses), plus the EIP-2930 access-list and      */
/* EIP-7702 authorization-tuple decoders. Standalone and purely             */
/* structural: input is a ByteSlice envelope + a ByteSlice public key, any  */
/* source; signature RULES live in lib/tx.sail, the crypto in               */
/* primitives/crypto.sail.                                                  */
/* ======================================================================== */

/* Decode an EIP-2930 access list (RLP [[address, [slot, ...]], ...]) from the
 * access-list field into the flat representation the Transaction and intrinsic-
 * gas accounting use: the addresses (one per entry) and the (address, slot) pairs. */
function decode_access_list(f : RlpField) -> (list(address), list(StorageKey)) = {
    var addresses : list(address) = [||];
    var storage_slots : list(StorageKey) = [||];
    var entries : list(RlpField) = rlp_list_items(f);
    var more_entries : bool = true;
    while more_entries do {
        match entries {
            entry :: rest_entries => {
                /* one entry = [addr, [keys]], destructured in one walk (exact arity) */
                let (addr_f, keys_f) : (RlpField, RlpField) = match rlp_list_items(entry) {
                    a :: (k :: [||]) => (a, k),
                    _ => throw InvalidBlock(RlpDecode),
                };
                let addr = word_to_address(rlp_field_word(addr_f));
                addresses = list_append(addresses, [|addr|]);
                var keys : list(RlpField) = rlp_list_items(keys_f);
                var more_keys : bool = true;
                while more_keys do {
                    match keys {
                        key :: rest_keys => {
                            storage_slots = list_append(
                                storage_slots,
                                [|struct { addr = addr, slot = rlp_field_word(key) }|],
                            );
                            keys = rest_keys
                        },
                        [||] => more_keys = false,
                    }
                };
                entries = rest_entries
            },
            [||] => more_entries = false,
        }
    };
    (addresses, storage_slots)
}

/* Decode an RLP list of 32-byte words -- the EIP-4844
 * blobVersionedHashes (BLOBHASH operands; the blob count is the list length). */
function decode_word_list(f : RlpField) -> list(word) = {
    var out : list(word) = [||];
    var items : list(RlpField) = rlp_list_items(f);
    var more : bool = true;
    while more do {
        match items {
            item :: rest => {
                out = list_append(out, [|rlp_field_word(item)|]);
                items = rest
            },
            [||] => more = false,
        }
    };
    out
}

/* ---- EIP-7702 authorization-tuple recovery ---------------------------------
 * Each set-code authorization tuple [chainId, address, nonce, yParity, r, s] is
 * signed by the AUTHORITY, whose address is recovered (not witnessed) via
 * ecrecover over keccak(0x05 ++ rlp([chainId, address, nonce])). */

/* decode an EIP-7702 authorization list into Authorizations,
 * recovering each authority. valid_sig records tuple-local signature validity
 * (recoverable, low-s, r/y parity ranges, incrementable nonce); process_auth then
 * applies the chain-id / account nonce / code checks. */
function decode_auth_list(f : RlpField) -> list(Authorization) = {
    var out : list(Authorization) = [||];
    var tuples : list(RlpField) = rlp_list_items(f);
    var more : bool = true;
    while more do {
        match tuples {
            tuple :: rest => {
                /* one [chainId,address,nonce,yParity,r,s], destructured in one
                   walk; a tuple of any other arity is undecodable (EELS
                   decodes to the exact Authorization shape) */
                let (chain_f, addr_f, nonce_f, y_f, r_f, s_f) :
                    (RlpField, RlpField, RlpField, RlpField, RlpField, RlpField) = match rlp_list_items(tuple) {
                    c :: (a :: (n :: (y :: (r :: (s :: [||]))))) => (c, a, n, y, r, s),
                    _ => throw InvalidBlock(RlpDecode),
                };
                let chain_id = rlp_field_int(chain_f);
                let auth_nonce = rlp_field_int(nonce_f);
                let y = rlp_field_int(y_f);
                let r = rlp_field_uint_word(r_f);
                let s = rlp_field_uint_word(s_f);
                let auth_addr = word_to_address(rlp_field_word(addr_f));
                let (ok, authority) = ecrecover_addr(auth_signing_hash(chain_id, auth_addr, auth_nonce), y, r, s);
                out = list_append(
                    out,
                    [|
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
                        },
                    |],
                );
                tuples = rest
            },
            [||] => more = false,
        }
    };
    out
}

/* the data field's content span within a tx envelope: never materialized */
function tx_input_span(tx : ByteSlice, payload : list(byte), payload_rel : int, i_data : int) -> ByteSlice =
    sub_slice(
        tx,
        payload_rel + rlp_top_item_content_offset(payload, i_data),
        rlp_top_item_content_length(payload, i_data),
    )

/* the signing preimage content: the RLP of fields[0..i_v), a contiguous span
 * starting at the payload list's content offset (sig_cs) */
function tx_sig_span(tx : ByteSlice, payload_rel : int, sig_cs : int, fields : list(RlpField), i_v : int) -> ByteSlice =
    sub_slice(tx, payload_rel + sig_cs, rlp_fields_prefix_len(fields, i_v))

/* Decode ONE transaction from its raw envelope bytes -- standalone and purely
 * STRUCTURAL: `tx` is the EIP-2718 envelope as a byte slice (any source),
 * `pubkey` the 65-byte witness public key (0x04 ++ X ++ Y) claimed for its
 * sender. The sender derives from the key (keccak(X||Y)[12:]) and the
 * signature material (signing hash, v/r/s, the key itself) is CAPTURED on the
 * Transaction; authentication happens at execution (check_transaction_validity
 * -> tx_auth_ok), mirroring EELS recover_sender_from_public_key inside
 * check_transaction. No trust decision and no fee derivation happens here.
 *
 * A leading byte >= 0xc0 is a legacy (type-0) RLP list; otherwise it is the
 * type byte (0x01 EIP-2930, 0x02 EIP-1559, 0x03 EIP-4844, 0x04 EIP-7702) and
 * the RLP payload starts right after it. Each envelope is destructured in ONE
 * walk against its EXACT field shape (mirroring EELS's per-envelope
 * dataclasses); an unknown type byte or wrong arity is undecodable. The data
 * field is never materialized: input_src carries its (source, offset, length)
 * span, bound as the transaction input only when the tx runs. */
function rlp_decode_tx(tx : ByteSlice, pubkey : ByteSlice) -> Transaction = {
    let sender = word_to_address(keccak256(sub_slice(pubkey, 1, 64)));
    let tx_len = unsigned(tx.len);
    let tx_bytes = read_byte_slice(tx);

    let b0 : int = if tx_len == 0 then {
            throw InvalidBlock(RlpDecode)
        } else {
            unsigned(bytes_get(tx_bytes, 0))
        };
    let ttype : int = if b0 >= 192 then { 0 } else { b0 }; /* 0xc0 = RLP list tag */
    let typed = ttype != 0;
    let payload : list(byte) = if typed then {
            bytes_slice(tx_bytes, 1, tx_len - 1)
        } else {
            tx_bytes
        };
    let payload_rel : int = if typed then { 1 } else { 0 }; /* payload offset in `tx` */
    let fields = rlp_node_items(payload);

    /* the signing preimage content offset: the payload list's content start */
    let (_, sig_cs, _) = rlp_hdr(payload, 0);

    match (ttype, fields) {
        (0, nonce_f :: (gp_f :: (gas_f :: (to_f :: (value_f :: (_ :: (v_f :: (r_f :: (s_f :: [||]))))))))) => {
            let v = rlp_field_int(v_f);
            let gp = rlp_field_uint_word(gp_f);
            struct {
                tx_type = LegacyTx,
                sender = sender,
                raw = tx,
                nonce = rlp_field_int(nonce_f),
                gas_limit = to_gas(rlp_field_int(gas_f)),
                is_create = list_len(to_f.content) == 0, /* empty `to` = contract creation */
                recipient = word_to_address(rlp_field_word(to_f)),
                value = rlp_field_uint_word(value_f),
                input_src = tx_input_span(tx, payload, payload_rel, 5),
                access_list_addresses = [||],
                access_list_slots = [||],
                blob_count = 0,
                max_fee = gp,
                max_blob_fee = ZERO_WORD,
                max_priority_fee = gp,
                authorizations = [||],
                blob_hashes = [||],
                pubkey = pubkey,
                signing_hash = tx_signing_hash(LegacyTx, tx_sig_span(tx, payload_rel, sig_cs, fields, 6), v),
                sig_v = v,
                sig_r = rlp_field_uint_word(r_f),
                sig_s = rlp_field_uint_word(s_f),
            }
        },
        (
            1,
            _ ::
                (nonce_f :: (gp_f :: (gas_f :: (to_f :: (value_f :: (_ :: (al_f :: (v_f :: (r_f :: (s_f :: [||])))))))))),
        ) => {
            let v = rlp_field_int(v_f);
            let gp = rlp_field_uint_word(gp_f);
            let (al_addrs, al_slots) = decode_access_list(al_f);
            struct {
                tx_type = AccessListTx,
                sender = sender,
                raw = tx,
                nonce = rlp_field_int(nonce_f),
                gas_limit = to_gas(rlp_field_int(gas_f)),
                is_create = list_len(to_f.content) == 0, /* empty `to` = contract creation */
                recipient = word_to_address(rlp_field_word(to_f)),
                value = rlp_field_uint_word(value_f),
                input_src = tx_input_span(tx, payload, payload_rel, 6),
                access_list_addresses = al_addrs,
                access_list_slots = al_slots,
                blob_count = 0,
                max_fee = gp,
                max_blob_fee = ZERO_WORD,
                max_priority_fee = gp,
                authorizations = [||],
                blob_hashes = [||],
                pubkey = pubkey,
                signing_hash = tx_signing_hash(AccessListTx, tx_sig_span(tx, payload_rel, sig_cs, fields, 8), v),
                sig_v = v,
                sig_r = rlp_field_uint_word(r_f),
                sig_s = rlp_field_uint_word(s_f),
            }
        },
        (
            2,
            _ ::
                (nonce_f ::
                    (mp_f ::
                        (mf_f :: (gas_f :: (to_f :: (value_f :: (_ :: (al_f :: (v_f :: (r_f :: (s_f :: [||]))))))))))),
        ) => {
            let v = rlp_field_int(v_f);
            let (al_addrs, al_slots) = decode_access_list(al_f);
            struct {
                tx_type = FeeMarketTx,
                sender = sender,
                raw = tx,
                nonce = rlp_field_int(nonce_f),
                gas_limit = to_gas(rlp_field_int(gas_f)),
                is_create = list_len(to_f.content) == 0, /* empty `to` = contract creation */
                recipient = word_to_address(rlp_field_word(to_f)),
                value = rlp_field_uint_word(value_f),
                input_src = tx_input_span(tx, payload, payload_rel, 7),
                access_list_addresses = al_addrs,
                access_list_slots = al_slots,
                blob_count = 0,
                max_fee = rlp_field_uint_word(mf_f),
                max_blob_fee = ZERO_WORD,
                max_priority_fee = rlp_field_uint_word(mp_f),
                authorizations = [||],
                blob_hashes = [||],
                pubkey = pubkey,
                signing_hash = tx_signing_hash(FeeMarketTx, tx_sig_span(tx, payload_rel, sig_cs, fields, 9), v),
                sig_v = v,
                sig_r = rlp_field_uint_word(r_f),
                sig_s = rlp_field_uint_word(s_f),
            }
        },
        (
            3,
            _ ::
                (nonce_f ::
                    (mp_f ::
                        (mf_f ::
                            (gas_f ::
                                (to_f ::
                                    (value_f :: (_ :: (al_f :: (mbf_f :: (bh_f :: (v_f :: (r_f :: (s_f :: [||]))))))))))))),
        ) => {
            let v = rlp_field_int(v_f);
            let (al_addrs, al_slots) = decode_access_list(al_f);
            let blob_hashes = decode_word_list(bh_f);
            struct {
                tx_type = BlobTx,
                sender = sender,
                raw = tx,
                nonce = rlp_field_int(nonce_f),
                gas_limit = to_gas(rlp_field_int(gas_f)),
                is_create = list_len(to_f.content) == 0, /* empty `to` = contract creation */
                recipient = word_to_address(rlp_field_word(to_f)),
                value = rlp_field_uint_word(value_f),
                input_src = tx_input_span(tx, payload, payload_rel, 7),
                access_list_addresses = al_addrs,
                access_list_slots = al_slots,
                blob_count = list_len(blob_hashes),
                max_fee = rlp_field_uint_word(mf_f),
                max_blob_fee = rlp_field_uint_word(mbf_f),
                max_priority_fee = rlp_field_uint_word(mp_f),
                authorizations = [||],
                blob_hashes = blob_hashes,
                pubkey = pubkey,
                signing_hash = tx_signing_hash(BlobTx, tx_sig_span(tx, payload_rel, sig_cs, fields, 11), v),
                sig_v = v,
                sig_r = rlp_field_uint_word(r_f),
                sig_s = rlp_field_uint_word(s_f),
            }
        },
        (
            4,
            _ ::
                (nonce_f ::
                    (mp_f ::
                        (mf_f ::
                            (gas_f ::
                                (to_f :: (value_f :: (_ :: (al_f :: (auth_f :: (v_f :: (r_f :: (s_f :: [||])))))))))))),
        ) => {
            let v = rlp_field_int(v_f);
            let (al_addrs, al_slots) = decode_access_list(al_f);
            struct {
                tx_type = SetCodeTx,
                sender = sender,
                raw = tx,
                nonce = rlp_field_int(nonce_f),
                gas_limit = to_gas(rlp_field_int(gas_f)),
                is_create = list_len(to_f.content) == 0, /* empty `to` = contract creation */
                recipient = word_to_address(rlp_field_word(to_f)),
                value = rlp_field_uint_word(value_f),
                input_src = tx_input_span(tx, payload, payload_rel, 7),
                access_list_addresses = al_addrs,
                access_list_slots = al_slots,
                blob_count = 0,
                max_fee = rlp_field_uint_word(mf_f),
                max_blob_fee = ZERO_WORD,
                max_priority_fee = rlp_field_uint_word(mp_f),
                authorizations = decode_auth_list(auth_f),
                blob_hashes = [||],
                pubkey = pubkey,
                signing_hash = tx_signing_hash(SetCodeTx, tx_sig_span(tx, payload_rel, sig_cs, fields, 10), v),
                sig_v = v,
                sig_r = rlp_field_uint_word(r_f),
                sig_s = rlp_field_uint_word(s_f),
            }
        },
        _ => throw InvalidBlock(RlpDecode), /* unknown envelope type or wrong arity */
    }
}
