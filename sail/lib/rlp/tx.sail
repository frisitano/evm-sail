/* ======================================================================== */
/* RLP: TRANSACTION DECODE                                                  */
/*                                                                          */
/* The EIP-2718 transaction envelope decoder and everything it rests on:    */
/* per-envelope destructuring into the Transaction type (one walk, exact    */
/* field shape, mirroring EELS's per-envelope dataclasses), the EIP-2930    */
/* access-list and EIP-7702 authorization-tuple decoders, the signing       */
/* preimage hashes, and sender authentication (the witness public key is    */
/* verified against the tx signature, never trusted). Standalone: input is  */
/* a ByteSlice envelope + a ByteSlice public key, any source.               */
/* ======================================================================== */

/* EIP-155: the chain id encoded in a legacy signature v >= 35 */
function legacy_sig_chain_id(v : int) -> int =
    ediv_int(v - 35, 2)

/* Transaction and EIP-7702 authorization signing hashes. Spec definitions
 * materialize the canonical RLP preimage; build definitions can refine this
 * boundary without exposing a generic byte-list hash to common code. The tx
 * signing content is a contiguous witness span (the pre-signature fields'
 * RLP); build frames + hashes it C-side rather than materializing the list. */
/* Tx signing preimage hash. `content_src` spans the RLP of the pre-signature
 * fields in the witness; legacy EIP-155 txs append (chain_id, 0, 0), typed txs
 * prepend the type byte as a domain separator. */
function tx_signing_hash(ttype : int, content_src : ByteSlice, v : int) -> hash = {
    var content : list(byte) = read_byte_slice(content_src);
    if (ttype == 0) & (v >= 35) then {
        content = list_append(content, rlp_int(legacy_sig_chain_id(v)));
        content = list_append(content, [|0x80, 0x80|])
    };
    let domain : list(byte) = if ttype != 0 then {
            [|get_slice_int(8, ttype, 0)|]
        } else {
            [||]
        };
    keccak_rlp_payload(domain, content)
}

/* EIP-7702 authorization signing hash: 0x05 || rlp([chain_id, address, nonce]). */
function auth_signing_hash(chain_id : int, addr : address, nonce : int) -> hash = {
    var content : list(byte) = rlp_int(chain_id);
    content = list_append(content, rlp_addr(addr));
    content = list_append(content, rlp_int(nonce));
    keccak_rlp_payload([|0x05|], content)
}

/* Effective fee for a typed (EIP-1559) tx: the gas price actually paid is
 * min(max_fee_per_gas, base_fee + max_priority_fee), and the priority tip paid to
 * the coinbase is that price minus the base fee. Legacy / 2930 txs carry a single
 * gas_price, passed here as max_fee = max_priority = gas_price so this recovers
 * (gas_price, gas_price - base_fee). The priority is clamped at 0 so an invalid
 * sub-base-fee price (rejected later by the validity check) never underflows. */
function eff_gas_price_for(base_fee : word, max_fee : word, max_priority_fee : word) -> (word, word) = {
    let bf = unsigned(base_fee);
    let cap = unsigned(max_fee);
    let tip = unsigned(max_priority_fee);
    let price = if cap < bf + tip then {
            cap
        } else {
            bf + tip
        };
    let prio = if price >= bf then {
            price - bf
        } else {
            0
        };
    (word_of_int(price), word_of_int(prio))
}

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

/* ---- transaction sender authentication (EIP-2718 / EIP-155 + secp256k1) ------
 * public_keys[i] is a WITNESS, not trusted input: the guest authenticates it by
 * verifying tx i's signature against it over the signing hash (verify is cheaper
 * than recovery -- which is why the pubkey is supplied). The sender then derives
 * from the authenticated key. A forged/wrong pubkey fails verification, so the
 * block is rejected (successful_validation = false). */

/* n/2 of the secp256k1 group order (EIP-2 low-s malleability bound) */
let SECP_N_HALF : bits(256) = 0x7fffffffffffffffffffffffffffffff5d576e7357a4501ddfe92f46681b20a0
let SECP_N_FULL : bits(256) = 0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141

function secp256k1_verify(h : bits(256), r : word, s : word, x : bits(256), y : bits(256)) -> bool =
    secp256k1_verify_precompile(h, r, s, x, y)

function tx_sig_v_ok(ttype : int, v : int) -> bool =
    if ttype == 0 then {
        (v == 27) | (v == 28) | ((v >= 35) & (legacy_sig_chain_id(v) == cc_chain_id()))
    } else {
        (v == 0) | (v == 1)
    }

/* authenticate a tx: EIP-2 low-s, then bind its 65-byte witness public key
 * (0x04 ++ X ++ Y) to the signature over the signing hash */
function tx_auth_ok(pubkey : ByteSlice, h : bits(256), r : word, s : word) -> bool =
    if unsigned(s) > unsigned(SECP_N_HALF) then {
        false
    } else {
        let x = bytes_be256(read_byte_slice(sub_slice(pubkey, 1, 32)));
        let y = bytes_be256(read_byte_slice(sub_slice(pubkey, 33, 32)));
        secp256k1_verify(h, r, s, x, y)
    }

/* ---- EIP-7702 authorization-tuple recovery ---------------------------------
 * Each set-code authorization tuple [chainId, address, nonce, yParity, r, s] is
 * signed by the AUTHORITY, whose address is recovered (not witnessed) via
 * ecrecover over keccak(0x05 ++ rlp([chainId, address, nonce])). */

function ecrecover_addr(h : bits(256), yparity : int, r : word, s : word) -> (bool, address) =
    ecrecover_precompile(h, yparity, r, s)

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

/* legacy / 2930 carry one gas_price; the coinbase tip is still
 * gas_price - base_fee (the base fee is burned) */
function legacy_fees(base_fee : word, gp_f : RlpField) -> (word, word, word, word) = {
    let gp = rlp_field_uint_word(gp_f);
    let (_, pr) = eff_gas_price_for(base_fee, gp, gp);
    (gp, pr, gp, gp)
}

/* 1559+: (max_priority_fee, max_fee) caps */
function dynamic_fees(base_fee : word, mp_f : RlpField, mf_f : RlpField) -> (word, word, word, word) = {
    let mp = rlp_field_uint_word(mp_f);
    let mf = rlp_field_uint_word(mf_f);
    let (gp, pr) = eff_gas_price_for(base_fee, mf, mp);
    (gp, pr, mf, mp)
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

/* authenticate: v-range per envelope, then the witness pubkey against the
 * signature over the signing hash -- throw at the point of detection */
function tx_sig_auth(
    pubkey : ByteSlice,
    ttype : int,
    sig_src : ByteSlice,
    v_f : RlpField,
    r_f : RlpField,
    s_f : RlpField,
) -> (
    unit
) = {
    let v = rlp_field_int(v_f);
    if not_bool(
        tx_sig_v_ok(ttype, v) &
            tx_auth_ok(pubkey, tx_signing_hash(ttype, sig_src, v), rlp_field_uint_word(r_f), rlp_field_uint_word(s_f)),
    ) then {
        throw InvalidBlock(InvalidSignature)
    }
}

/* Decode ONE transaction from its raw envelope bytes -- standalone: `tx` is the
 * EIP-2718 envelope as a byte slice (any source), `pubkey` the 65-byte witness
 * public key (0x04 ++ X ++ Y) claimed for its sender. The sender derives from
 * the key (keccak(X||Y)[12:]); the key is NOT trusted -- the tx's signature is
 * verified against it over the signing hash, so a forged key throws
 * InvalidBlock(InvalidSignature) (EELS recover_sender).
 *
 * A leading byte >= 0xc0 is a legacy (type-0) RLP list; otherwise it is the
 * type byte (0x01 EIP-2930, 0x02 EIP-1559, 0x03 EIP-4844, 0x04 EIP-7702) and
 * the RLP payload starts right after it. Each envelope is destructured in ONE
 * walk against its EXACT field shape (mirroring EELS's per-envelope
 * dataclasses); an unknown type byte or wrong arity is undecodable. The data
 * field is never materialized: input_src carries its (source, offset, length)
 * span, staged into the calldata backend only when the tx runs. */
function rlp_decode_tx(tx : ByteSlice, pubkey : ByteSlice, base_fee : word) -> Transaction = {
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
            tx_sig_auth(pubkey, ttype, tx_sig_span(tx, payload_rel, sig_cs, fields, 6), v_f, r_f, s_f);
            let (gas_price, priority, max_fee, max_priority) = legacy_fees(base_fee, gp_f);
            struct {
                tx_type = LegacyTx,
                sender = sender,
                nonce = rlp_field_int(nonce_f),
                gas_price = gas_price,
                priority_fee = priority,
                gas_limit = to_gas(rlp_field_int(gas_f)),
                is_create = list_len(to_f.content) == 0, /* empty `to` = contract creation */
                recipient = word_to_address(rlp_field_word(to_f)),
                value = rlp_field_uint_word(value_f),
                input_src = tx_input_span(tx, payload, payload_rel, 5),
                access_list_addresses = [||],
                access_list_slots = [||],
                blob_count = 0,
                max_fee = max_fee,
                max_blob_fee = ZERO_WORD,
                max_priority_fee = max_priority,
                authorizations = [||],
                blob_hashes = [||],
            }
        },
        (
            1,
            _ ::
                (nonce_f :: (gp_f :: (gas_f :: (to_f :: (value_f :: (_ :: (al_f :: (v_f :: (r_f :: (s_f :: [||])))))))))),
        ) => {
            tx_sig_auth(pubkey, ttype, tx_sig_span(tx, payload_rel, sig_cs, fields, 8), v_f, r_f, s_f);
            let (gas_price, priority, max_fee, max_priority) = legacy_fees(base_fee, gp_f);
            let (al_addrs, al_slots) = decode_access_list(al_f);
            struct {
                tx_type = AccessListTx,
                sender = sender,
                nonce = rlp_field_int(nonce_f),
                gas_price = gas_price,
                priority_fee = priority,
                gas_limit = to_gas(rlp_field_int(gas_f)),
                is_create = list_len(to_f.content) == 0, /* empty `to` = contract creation */
                recipient = word_to_address(rlp_field_word(to_f)),
                value = rlp_field_uint_word(value_f),
                input_src = tx_input_span(tx, payload, payload_rel, 6),
                access_list_addresses = al_addrs,
                access_list_slots = al_slots,
                blob_count = 0,
                max_fee = max_fee,
                max_blob_fee = ZERO_WORD,
                max_priority_fee = max_priority,
                authorizations = [||],
                blob_hashes = [||],
            }
        },
        (
            2,
            _ ::
                (nonce_f ::
                    (mp_f ::
                        (mf_f :: (gas_f :: (to_f :: (value_f :: (_ :: (al_f :: (v_f :: (r_f :: (s_f :: [||]))))))))))),
        ) => {
            tx_sig_auth(pubkey, ttype, tx_sig_span(tx, payload_rel, sig_cs, fields, 9), v_f, r_f, s_f);
            let (gas_price, priority, max_fee, max_priority) = dynamic_fees(base_fee, mp_f, mf_f);
            let (al_addrs, al_slots) = decode_access_list(al_f);
            struct {
                tx_type = FeeMarketTx,
                sender = sender,
                nonce = rlp_field_int(nonce_f),
                gas_price = gas_price,
                priority_fee = priority,
                gas_limit = to_gas(rlp_field_int(gas_f)),
                is_create = list_len(to_f.content) == 0, /* empty `to` = contract creation */
                recipient = word_to_address(rlp_field_word(to_f)),
                value = rlp_field_uint_word(value_f),
                input_src = tx_input_span(tx, payload, payload_rel, 7),
                access_list_addresses = al_addrs,
                access_list_slots = al_slots,
                blob_count = 0,
                max_fee = max_fee,
                max_blob_fee = ZERO_WORD,
                max_priority_fee = max_priority,
                authorizations = [||],
                blob_hashes = [||],
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
            tx_sig_auth(pubkey, ttype, tx_sig_span(tx, payload_rel, sig_cs, fields, 11), v_f, r_f, s_f);
            let (gas_price, priority, max_fee, max_priority) = dynamic_fees(base_fee, mp_f, mf_f);
            let (al_addrs, al_slots) = decode_access_list(al_f);
            let blob_hashes = decode_word_list(bh_f);
            struct {
                tx_type = BlobTx,
                sender = sender,
                nonce = rlp_field_int(nonce_f),
                gas_price = gas_price,
                priority_fee = priority,
                gas_limit = to_gas(rlp_field_int(gas_f)),
                is_create = list_len(to_f.content) == 0, /* empty `to` = contract creation */
                recipient = word_to_address(rlp_field_word(to_f)),
                value = rlp_field_uint_word(value_f),
                input_src = tx_input_span(tx, payload, payload_rel, 7),
                access_list_addresses = al_addrs,
                access_list_slots = al_slots,
                blob_count = list_len(blob_hashes),
                max_fee = max_fee,
                max_blob_fee = rlp_field_uint_word(mbf_f),
                max_priority_fee = max_priority,
                authorizations = [||],
                blob_hashes = blob_hashes,
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
            tx_sig_auth(pubkey, ttype, tx_sig_span(tx, payload_rel, sig_cs, fields, 10), v_f, r_f, s_f);
            let (gas_price, priority, max_fee, max_priority) = dynamic_fees(base_fee, mp_f, mf_f);
            let (al_addrs, al_slots) = decode_access_list(al_f);
            struct {
                tx_type = SetCodeTx,
                sender = sender,
                nonce = rlp_field_int(nonce_f),
                gas_price = gas_price,
                priority_fee = priority,
                gas_limit = to_gas(rlp_field_int(gas_f)),
                is_create = list_len(to_f.content) == 0, /* empty `to` = contract creation */
                recipient = word_to_address(rlp_field_word(to_f)),
                value = rlp_field_uint_word(value_f),
                input_src = tx_input_span(tx, payload, payload_rel, 7),
                access_list_addresses = al_addrs,
                access_list_slots = al_slots,
                blob_count = 0,
                max_fee = max_fee,
                max_blob_fee = ZERO_WORD,
                max_priority_fee = max_priority,
                authorizations = decode_auth_list(auth_f),
                blob_hashes = [||],
            }
        },
        _ => throw InvalidBlock(RlpDecode), /* unknown envelope type or wrong arity */
    }
}
