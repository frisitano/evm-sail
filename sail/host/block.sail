/* ======================================================================== */
/* SSZ BLOCK DECODE  (shared: guest + runner)                               */
/*                                                                          */
/* The SszStatelessInput decode layer (header -> k_header register plus     */
/* per-index tx/withdrawal decoders), extracted from main.sail so           */
/* BOTH the zkVM guest (main.sail: full-block validation) and the EEST      */
/* runner (runner.sail: execute-tx + dump post-state) reach execution data  */
/* through ONE input structure. Depends only on the common layer (io.sail   */
/* SSZ accessors, rlp.sail, transaction.sail, mpt.sail witness walk); it    */
/* does NOT touch any entry-specific code.                                  */
/* ======================================================================== */

/* Register every witness code blob into the hash-keyed C code store. The C side
 * hashes the SSZ input span and copies the bytes into the owned code_db entry. */
function register_witness_codes(start : int, n : int, stop : int) -> unit = {
    foreach (i from 0 to (n - 1)) {
        code_db_store_witness_code(ssz_list_slice(start, n, stop, i))
    }
}

/* Parse the witness header chain, asserting it against the ALREADY-decoded
 * block: each header is hashed once (kept in the C headerhash store for
 * BLOCKHASH/EIP-2935, k_blockhash), the chain prefix must be contiguous
 * (header[i].parent_hash == keccak(header[i-1])), and the NEWEST header must
 * be the decoded block's parent (keccak(header[n-1]) == expected_parent).
 * Returns (chain contiguous, block parent hash matches, newest header's RLP
 * items); field DECODING from the captured items stays at the original
 * consumer sites (parent_state_root, parent_header_int_field), so a
 * non-canonical field still throws exactly where it used to. */
function index_witness_headers(
    start : int,
    n : int,
    stop : int,
    expected_parent : hash,
) -> (
    (bool, bool, list(RlpField))
) = {
    headerhash_reset();
    var prev : hash = ZERO_WORD;
    var ok : bool = n >= 1;
    var pitems : list(RlpField) = [||];
    foreach (i from 0 to (n - 1)) {
        let s = ssz_list_slice(start, n, stop, i);
        if (i > 0) | (i == n - 1) then {
            let items = rlp_node_items(read_byte_slice(s));
            if i > 0 then {
                if bytes_be256(list_nth(items, 0, RLP_NONE).content) != prev then {
                    ok = false
                }
            };
            if i == n - 1 then {
                pitems = items
            }
        };
        prev = keccak256(s);
        headerhash_push(prev)
    };
    (ok, prev == expected_parent, pitems)
}

/* the parent header's state_root (field 3) = pre-state root. The witness
 * headers run header[0] (oldest) .. header[n-1] (newest); the block being
 * validated extends the NEWEST, so its parent is header[n-1]. */
function parent_state_root(items : list(RlpField)) -> hash =
    bytes_be256(list_nth(items, 3, RLP_NONE).content)

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

/* Shared Transaction constructor: the per-envelope decode arms below supply
 * the envelope-specific pieces; everything scalar decodes here. */
function mk_tx(
    t : TxType,
    sender : address,
    nonce_f : RlpField,
    gas_f : RlpField,
    to_f : RlpField,
    value_f : RlpField,
    fees : (word, word, word, word),
    input_src : ByteSlice,
    al : (list(address), list(StorageKey)),
    max_blob_fee : word,
    blob_hashes : list(word),
    auths : list(Authorization),
) -> (
    /* (gas_price, priority, max_fee, max_priority) */





    Transaction
) = {
    let (gas_price, priority, max_fee, max_priority) = fees;
    let (al_addrs, al_slots) = al;
    struct {
        tx_type = t,
        sender = sender,
        nonce = rlp_field_int(nonce_f),
        gas_price = gas_price,
        priority_fee = priority,
        gas_limit = to_gas(rlp_field_int(gas_f)),
        is_create = list_len(to_f.content) == 0, /* empty `to` = contract creation */
        recipient = word_to_address(rlp_field_word(to_f)),
        value = rlp_field_uint_word(value_f),
        input_src = input_src,
        access_list_addresses = al_addrs,
        access_list_slots = al_slots,
        blob_count = list_len(blob_hashes),
        max_fee = max_fee,
        max_blob_fee = max_blob_fee,
        max_priority_fee = max_priority,
        authorizations = auths,
        blob_hashes = blob_hashes,
    }
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
            mk_tx(
                LegacyTx,
                sender,
                nonce_f,
                gas_f,
                to_f,
                value_f,
                legacy_fees(base_fee, gp_f),
                tx_input_span(tx, payload, payload_rel, 5),
                ([||], [||]),
                ZERO_WORD,
                [||],
                [||],
            )
        },
        (
            1,
            _ ::
                (nonce_f :: (gp_f :: (gas_f :: (to_f :: (value_f :: (_ :: (al_f :: (v_f :: (r_f :: (s_f :: [||])))))))))),
        ) => {
            tx_sig_auth(pubkey, ttype, tx_sig_span(tx, payload_rel, sig_cs, fields, 8), v_f, r_f, s_f);
            mk_tx(
                AccessListTx,
                sender,
                nonce_f,
                gas_f,
                to_f,
                value_f,
                legacy_fees(base_fee, gp_f),
                tx_input_span(tx, payload, payload_rel, 6),
                decode_access_list(al_f),
                ZERO_WORD,
                [||],
                [||],
            )
        },
        (
            2,
            _ ::
                (nonce_f ::
                    (mp_f ::
                        (mf_f :: (gas_f :: (to_f :: (value_f :: (_ :: (al_f :: (v_f :: (r_f :: (s_f :: [||]))))))))))),
        ) => {
            tx_sig_auth(pubkey, ttype, tx_sig_span(tx, payload_rel, sig_cs, fields, 9), v_f, r_f, s_f);
            mk_tx(
                FeeMarketTx,
                sender,
                nonce_f,
                gas_f,
                to_f,
                value_f,
                dynamic_fees(base_fee, mp_f, mf_f),
                tx_input_span(tx, payload, payload_rel, 7),
                decode_access_list(al_f),
                ZERO_WORD,
                [||],
                [||],
            )
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
            mk_tx(
                BlobTx,
                sender,
                nonce_f,
                gas_f,
                to_f,
                value_f,
                dynamic_fees(base_fee, mp_f, mf_f),
                tx_input_span(tx, payload, payload_rel, 7),
                decode_access_list(al_f),
                rlp_field_uint_word(mbf_f),
                decode_word_list(bh_f),
                [||],
            )
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
            mk_tx(
                SetCodeTx,
                sender,
                nonce_f,
                gas_f,
                to_f,
                value_f,
                dynamic_fees(base_fee, mp_f, mf_f),
                tx_input_span(tx, payload, payload_rel, 7),
                decode_access_list(al_f),
                ZERO_WORD,
                [||],
                decode_auth_list(auth_f),
            )
        },
        _ => throw InvalidBlock(RlpDecode), /* unknown envelope type or wrong arity */
    }
}

/* ---- EIP-4788 / EIP-2935 block-start system calls ---------------------------
 * Before the transactions, the protocol issues unchecked system calls from
 * SYSTEM_ADDRESS to the beacon-roots (EIP-4788) and block-hash-history (EIP-2935)
 * contracts, writing the parent beacon-block root and the parent block hash into
 * their ring buffers. Each takes a 32-byte input, 30M gas, and bumps no nonce /
 * charges no fee / counts toward no block gas; it is skipped if the contract has
 * no code. Their committed storage writes shape the post-state root. */
function SYSTEM_ADDRESS() -> address = 0xfffffffffffffffffffffffffffffffffffffffe
function BEACON_ROOTS_ADDR() -> address = 0x000f3df6d732807ef1319fb7b8bb8522d0beac02
function HISTORY_STORAGE_ADDR() -> address = 0x0000f90827f1c53a10cb7a02335b175320002935

/* Enter a top-level protocol system-call frame.
 *
 * The caller stages txdata before entering this helper.
 * Keeping frame initialization in one place gives the block driver a named
 * boundary: pre-transaction and post-transaction system calls differ in
 * validation policy, not in execution-frame construction.
 */
function enter_system_call_frame(tgt : address, input_len : int) -> unit = {
    k_snapshot();
    pc = 0;
    call_depth = 0;
    gas_remaining = to_gas(30000000);
    stack_clear();
    mem_clear();
    returndata_reset_all();
    memory_size = 0;
    frame_status = Running();
    calldata_bind_tx_input(); /* AFTER mem_clear: it points the base frame's calldata at the slot */
    message =
        struct {
            caller = SYSTEM_ADDRESS(),
            address = tgt,
            code_address = tgt,
            value = ZERO_WORD,
            data_len = input_len,
            is_static = false,
            depth = 0,
        };
    frame_code_addr(tgt)
}

function system_call(tgt : address, input : bits(256)) -> unit = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        () /* system contract absent -> skip (EIP-4788 / EIP-2935) */
    } else {
        let _ = txdata_set_word(input); /* the call's 32-byte calldata */
        enter_system_call_frame(tgt, 32);
        interpret();
        if frame_succeeded() then {
            k_commit()
        } else {
            k_revert()
        };
        k_tx_merge() /* system call is a top-level boundary: merge its storage */
    }
}

/* ---- EIP-7002 / EIP-7251 block-END system calls (general-purpose requests) ---
 * After the transactions and withdrawals, the protocol dequeues the queued
 * withdrawal (EIP-7002) and consolidation (EIP-7251) requests by a CHECKED
 * system call (SYSTEM_ADDRESS, EMPTY calldata, 30M gas) to each predeploy. The
 * dequeue clears the request queue in the contract's storage, shaping the
 * post-state root. "Checked" means: a missing contract (no code) OR a failed
 * call makes the whole block INVALID (returns false here). */
function WITHDRAWAL_REQUEST_ADDR() -> address = 0x00000961ef480eb55e80d19ad83579a64c007002
function CONSOLIDATION_REQUEST_ADDR() -> address = 0x0000bbddc7ce488642fb579f8b00f3a590007251

function system_call_checked(tgt : address) -> bool = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        false /* checked: a missing system predeploy invalidates the block */
    } else {
        let _ = txdata_set_source(byte_slice(WitnessSource, 0, 0)); /* empty calldata: the SYSTEM caller dequeues */
        enter_system_call_frame(tgt, 0);
        interpret();
        let ok = frame_succeeded();
        if ok then {
            k_commit()
        } else {
            k_revert() /* checked: a reverting/erroring system call invalidates the block */
        };
        k_tx_merge(); /* system call is a top-level boundary: merge its storage */
        ok
    }
}

/* Reject a malformed SszStatelessInput: the 2-byte schema id must be 1, the
 * 4 field offsets (LE u32 at body+{0,4,8,12}) must start at 16, be monotonic,
 * and end within the input, and the trailing public_keys region must be a whole
 * number of 65-byte keys (no truncation / trailing garbage). EELS's consumer
 * rejects such input with successful_validation=false; the guest must too rather
 * than echo garbage. */
function input_well_formed() -> bool = {
    let len = unsigned(ssz_src_len());
    if len < 18 then {
        false
    } else {
        if ssz_schema_id() != 1 then {
            false
        } else {
            let o0 = ssz_u32(2);
            let o1 = ssz_u32(6);
            let o2 = ssz_u32(10);
            let o3 = ssz_u32(14);
            (o0 == 16) & (o0 <= o1) & (o1 <= o2) & (o2 <= o3) & (2 + o3 <= len) & (emod_int(len - (2 + o3), 65) == 0)
        }
    }
}

/* Materialize the hash-keyed witness node store and anchor the authenticated
 * pre-state root. There is no eager re-root or harvest pass: every later
 * stateless read is a lazy walk from k_parent_state_root, and because the
 * node-db is keyed by keccak(node) each walk is itself a hash-chain proof --
 * witness material is authenticated exactly where it is consumed. The anchor
 * register is set only after the witness header chain validates.
 *
 * No execution-overlay wipe here: the world starts empty (fresh process for the
 * production guest; the ctypes harness calls evmsail_clear_memory before each
 * run -- test_utils.c), which is why the model no longer defines k_world_reset. */
function decode_block_header_ssz(p : int, npr : int) -> BlockHeader =
    struct {
        number = pl_block_number(p),
        timestamp = pl_timestamp(p),
        gas_limit = pl_gas_limit(p),
        gas_used = pl_gas_used(p),
        prev_randao = pl_prev_randao(p),
        base_fee = pl_base_fee(p),
        blob_gas_used = pl_blob_gas_used(p),
        excess_blob_gas = pl_excess_blob_gas(p),
        state_root = pl_state_root(p),
        receipts_root = pl_receipts_root(p),
        logs_bloom = pl_logs_bloom(p),
        block_hash = pl_block_hash(p),
        fee_recipient = pl_fee_recipient(p),
        parent_hash = ssz_bytes32(p + 0),
        parent_beacon_block_root = ssz_bytes32(npr + NPR_BEACON_ROOT),
        slot_number = pl_slot_number(p),
    }

function withdrawal_at(wds : int, i : int) -> Withdrawal =
    struct {
        index = withdrawal_index(wds, i),
        validator_index = withdrawal_validator_index(wds, i),
        address = withdrawal_addr(wds, i),
        amount = withdrawal_amount(wds, i),
    }

/* Everything the single decode pass returns: the decoded block (header,
 * transactions, withdrawals) plus the witness-header results the validation
 * layer reads. The parent header travels as its RLP items so field decoding
 * (and its canonicity throws) stays at the original consumer sites. */
struct StatelessInput = {
    block : Block,
    fork : int,
    headers_ok : bool, /* witness header chain contiguous              */
    parent_hash_ok : bool, /* newest witness header IS the block's parent  */
    parent_header_items : list(RlpField),
}

/* THE single traversal over the stateless input. In one pass, in container
 * order: the geometry (each offset once), the witness node DB and code DB
 * (C-side stores), the header chain (hashed once each, hashes into the C
 * headerhash store for BLOCKHASH, newest split once), the block header, and
 * every transaction (sender recovery + signature verification, as EELS
 * decodes the block body before executing it) and withdrawal. Also installs
 * the execution-context registers (k_header, k_n_headers,
 * k_parent_state_root) the EVM reads ambiently. Throws InvalidBlock on a
 * broken header chain, malformed tx, or bad signature -- same sites, same
 * order as the previous incremental decode. */
function decode_stateless_input() -> StatelessInput = {
    /* geometry: each container offset read exactly once, held in locals */
    let npr = SSZ_BODY + ssz_u32(SSZ_BODY + IN_NPR_OFF);
    let wit = SSZ_BODY + ssz_u32(SSZ_BODY + IN_WITNESS_OFF);
    let cc = SSZ_BODY + ssz_u32(SSZ_BODY + IN_CHAIN_CONFIG_OFF);
    let pubkeys = SSZ_BODY + ssz_u32(SSZ_BODY + IN_PUBLIC_KEYS_OFF); /* = chain_config end */
    let p = npr + ssz_u32(npr + NPR_PAYLOAD_OFF);
    let p_end = npr + ssz_u32(npr + NPR_VHASHES_OFF);
    let txs = p + ssz_u32(p + PL_TXS_OFF);
    let wds = p + ssz_u32(p + PL_WDS_OFF);
    let bal = p + ssz_u32(p + PL_BAL_OFF);
    let n_txs = if txs == wds then {
            0
        } else {
            ediv_int(ssz_u32(txs), SSZ_OFF_BYTES)
        };
    let n_wds = ediv_int(bal - wds, WD_SIZE);
    let ws = wit + ssz_u32(wit + WIT_STATE_OFF);
    let wc = wit + ssz_u32(wit + WIT_CODES_OFF);
    let wh = wit + ssz_u32(wit + WIT_HEADERS_OFF);
    let n_ws = if ws == wc then {
            0
        } else {
            ediv_int(ssz_u32(ws), SSZ_OFF_BYTES)
        };
    let n_wc = if wc == wh then {
            0
        } else {
            ediv_int(ssz_u32(wc), SSZ_OFF_BYTES)
        };
    let n_wh = if wh == cc then {
            0
        } else {
            ediv_int(ssz_u32(wh), SSZ_OFF_BYTES)
        };

    /* the block itself (payload region, first in container order); each tx is
       an envelope slice + its 65-byte witness pubkey, decoded standalone */
    let header = decode_block_header_ssz(p, npr);
    k_set_header(header);
    var txs_rev : list(Transaction) = [||];
    foreach (i from 0 to (n_txs - 1)) {
        txs_rev =
            rlp_decode_tx(ssz_list_slice(txs, n_txs, wds, i), witness_byte_slice(pubkeys + i * 65, 65), header.base_fee) ::
                txs_rev
    };
    var wds_rev : list(Withdrawal) = [||];
    foreach (i from 0 to (n_wds - 1)) {
        wds_rev = withdrawal_at(wds, i) :: wds_rev
    };

    /* the witness region, asserted against the decoded block */
    build_node_db(ws, n_ws, wc);
    register_witness_codes(wc, n_wc, wh);
    let (hok, pok, pitems) = index_witness_headers(wh, n_wh, cc, header.parent_hash);
    k_n_headers = n_wh;
    if hok then {
        k_parent_state_root = parent_state_root(pitems)
    } else {
        throw InvalidBlock(WitnessDeficient)
    };
    struct {
        block =
            struct {
                header = header,
                body =
                    struct {
                        transactions = list_reverse(txs_rev),
                        withdrawals = list_reverse(wds_rev),
                        block_access_list = witness_byte_slice(bal, p_end - bal),
                    },
            },
        fork = cc_fork(),
        headers_ok = hok,
        parent_hash_ok = pok,
        parent_header_items = pitems,
    }
}

/* Block hash (val in host/io.sail): keccak(rlp([...header fields...])), the
 * canonical field layout. logs_bloom / extraData are re-read from their
 * witness spans. */
function block_header_hash(
    transactions_root : hash,
    withdrawals_root : hash,
    requests_hash : hash,
    block_access_list_hash : hash,
) -> (
    hash
) = {
    var payload : list(byte) = rlp_word(k_header.parent_hash);
    payload = list_append(payload, rlp_word(EMPTY_OMMER_HASH));
    payload = list_append(payload, rlp_addr(k_header.fee_recipient));
    payload = list_append(payload, rlp_word(k_header.state_root));
    payload = list_append(payload, rlp_word(transactions_root));
    payload = list_append(payload, rlp_word(k_header.receipts_root));
    payload = list_append(payload, rlp_bytes(k_header.logs_bloom));
    payload = list_append(payload, rlp_int(0));
    payload = list_append(payload, rlp_int(k_header.number));
    payload = list_append(payload, rlp_int(k_header.gas_limit));
    payload = list_append(payload, rlp_int(k_header.gas_used));
    payload = list_append(payload, rlp_int(k_header.timestamp));
    payload = list_append(payload, rlp_bytes(ssz_bytes(pl_extra_start(), pl_txs_start() - pl_extra_start())));
    payload = list_append(payload, rlp_word(k_header.prev_randao));
    payload = list_append(payload, rlp_bytes(zero_bytes(8)));
    payload = list_append(payload, rlp_int_word(k_header.base_fee));
    payload = list_append(payload, rlp_word(withdrawals_root));
    payload = list_append(payload, rlp_int(unsigned(k_header.blob_gas_used)));
    payload = list_append(payload, rlp_int(unsigned(k_header.excess_blob_gas)));
    payload = list_append(payload, rlp_word(k_header.parent_beacon_block_root));
    payload = list_append(payload, rlp_word(requests_hash));
    payload = list_append(payload, rlp_word(block_access_list_hash));
    payload = list_append(payload, rlp_int(unsigned(k_header.slot_number)));
    keccak_rlp_payload([||], payload)
}
