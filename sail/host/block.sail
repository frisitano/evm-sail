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
function register_witness_codes() -> unit = {
    let n = wit_n_codes();
    foreach (i from 0 to (n - 1)) {
        code_db_store_witness_code(ssz_witness_code_slice(i))
    }
}

/* validate_headers: the witness headers form a contiguous chain
 * (header[i].parent_hash == keccak(header[i-1])). */
function headers_ok() -> bool = {
    let n = wit_n_headers();
    if n < 1 then {
        false
    } else {
        var ok : bool = true;
        foreach (i from 1 to (n - 1)) {
            let parent_hash = bytes_be256(list_nth(rlp_node_items(ssz_witness_header(i)), 0, RLP_NONE).content);
            let prev_hash = witness_keccak(ssz_witness_header_slice(i - 1));
            if parent_hash != prev_hash then {
                ok = false
            }
        };
        ok
    }
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
                let parts = rlp_list_items(entry); /* one entry = [addr, [keys]] */
                let addr = word_to_address(rlp_field_word(rlp_item(parts, 0)));
                addresses = list_append(addresses, [|addr|]);
                var keys : list(RlpField) = rlp_list_items(rlp_item(parts, 1));
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

/* index of the first signature field (v / yParity) by tx type */
function tx_sig_field(ttype : int) -> int = match ttype {
    0 => 6,
    1 => 8,
    2 => 9,
    3 => 11,
    _ => 10,
}

function tx_sig_v_ok(ttype : int, v : int) -> bool =
    if ttype == 0 then {
        (v == 27) | (v == 28) | ((v >= 35) & (legacy_sig_chain_id(v) == cc_chain_id()))
    } else {
        (v == 0) | (v == 1)
    }

/* authenticate tx i: EIP-2 low-s, then bind public_keys[i] to the signature */
function tx_auth_ok(i : int, h : bits(256), r : word, s : word) -> bool =
    if unsigned(s) > unsigned(SECP_N_HALF) then {
        false
    } else {
        secp256k1_verify(h, r, s, ssz_bytes32(cc_end() + i * 65 + 1), ssz_bytes32(cc_end() + i * 65 + 33))
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
                let fields = rlp_list_items(tuple); /* one [chainId,address,nonce,yParity,r,s] */
                let chain_id = rlp_field_int(rlp_item(fields, 0));
                let auth_nonce = rlp_field_int(rlp_item(fields, 2));
                let y = rlp_field_int(rlp_item(fields, 3));
                let r = rlp_field_uint_word(rlp_item(fields, 4));
                let s = rlp_field_uint_word(rlp_item(fields, 5));
                let auth_addr = word_to_address(rlp_field_word(rlp_item(fields, 1)));
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
                            address = word_to_address(rlp_field_word(rlp_item(fields, 1))),
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

/* Decode transaction `i` from the SSZ payload's transactions list by
 * materializing its RLP bytes, then using the shared RLP decoder. The calldata
 * field is copied into the owned txdata buffer before execution.
 * The sender is derived from public_keys[i] (PUBLIC_KEY_BYTES = 65: a 0x04 prefix
 * + the 64-byte X||Y) as keccak(X||Y)[12:]. public_keys is a WITNESS, not trusted:
 * tx_auth_ok (folded into the auth_ok register below) verifies tx i's signature
 * against this key, so a forged key fails verification and the block is rejected.
 *
 * EIP-2718 envelope: a leading byte >= 0xc0 is a legacy (type-0) RLP list;
 * otherwise it is the type byte (0x01 EIP-2930, 0x02 EIP-1559, 0x03 EIP-4844,
 * 0x04 EIP-7702) and the RLP payload starts right after it. The field layouts
 * differ -- typed envelopes prepend chain_id, and 1559+ split gas_price into
 * (max_priority_fee, max_fee) -- so the logical field indices are selected by
 * type. Type-4 (EIP-7702) authorization tuples are decoded and their authorities
 * recovered (decode_auth_list); process_auth_list then applies the delegations. */
function decode_tx_ssz(i : int, base_fee : word) -> Transaction = {
    let sender = word_to_address(witness_keccak(witness_slice(cc_end() + i * 65 + 1, 64)));
    let (tx_off, tx_len) = ssz_list_span(pl_txs_start(), pl_n_txs(), pl_wds_start(), i);
    let tx_bytes = ssz_bytes(tx_off, tx_len);

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
    let fields = rlp_node_items(payload);

    /* logical field index per type (see each EIP's layout) */
    let i_nonce : int = if typed then { 1 } else { 0 };
    let i_gas : int = match ttype {
        0 => 2,
        1 => 3,
        _ => 4,
    };
    let i_to : int = match ttype {
        0 => 3,
        1 => 4,
        _ => 5,
    };
    let i_val : int = match ttype {
        0 => 4,
        1 => 5,
        _ => 6,
    };
    let i_data : int = match ttype {
        0 => 5,
        1 => 6,
        _ => 7,
    };
    let i_al : int = if ttype == 1 then { 7 } else { 8 }; /* typed only */

    let to_f = rlp_item(fields, i_to);
    let data_f = rlp_item(fields, i_data);

    /* The tx input bytes are a contiguous span of the witness: the data field's
       RLP content, at `tx_off (+1 for a typed envelope) + its offset within the
       payload`. Carry that span so the build backend stages calldata in one
       memcpy from the witness rather than streaming a materialized list. */
    let input_off = (if typed then {
            tx_off + 1
        } else {
            tx_off
        }) +
            rlp_top_item_content_offset(payload, i_data);
    let input_src = byte_slice(WitnessSource, input_off, list_len(data_f.content));
    let nonce = rlp_field_int(rlp_item(fields, i_nonce));
    let transaction_gas_limit = rlp_field_int(rlp_item(fields, i_gas));
    let is_create = list_len(to_f.content) == 0; /* empty `to` field = contract creation */
    let recipient = word_to_address(rlp_field_word(to_f));
    let value = rlp_field_uint_word(rlp_item(fields, i_val));

    /* fees: legacy / 2930 carry one gas_price; 1559+ carry (max_priority, max_fee) */
    let (gas_price, priority, max_fee, max_priority_fee_cap) : (word, word, word, word) = if ttype <= 1 then {
            let gp = rlp_field_uint_word(rlp_item(fields, if ttype == 0 then { 1 } else { 2 }));

            /* legacy/2930 carry a single gas_price; the coinbase tip is still
               gas_price - base_fee (the base fee is burned), so route through
               eff_gas_price_for(base_fee, gp, gp) = (gp, gp - base_fee) rather than paying the
               full gas_price as priority. */
            let (_, pr) = eff_gas_price_for(base_fee, gp, gp);
            (gp, pr, gp, gp)
        } else {
            let mp = rlp_field_uint_word(rlp_item(fields, 2));
            let mf = rlp_field_uint_word(rlp_item(fields, 3));
            let (gp, pr) = eff_gas_price_for(base_fee, mf, mp);
            (gp, pr, mf, mp)
        };

    /* EIP-2930 access list (present on every typed envelope) */
    let (decoded_access_list_addresses, decoded_access_list_slots) : (list(address), list(StorageKey)) = if typed then {
            decode_access_list(rlp_item(fields, i_al))
        } else {
            ([||], [||])
        };

    /* EIP-4844 blob fields: max_fee_per_blob_gas (field 9) + versioned hashes (10) */
    let (max_blob_fee, blob_hashes) : (word, list(word)) = if ttype == 3 then {
            (rlp_field_uint_word(rlp_item(fields, 9)), decode_word_list(rlp_item(fields, 10)))
        } else {
            (ZERO_WORD, [||])
        };

    /* EIP-7702 authorization list (field 9 for type-4): recover each authority */
    let decoded_authorizations : list(Authorization) = if ttype == 4 then {
            decode_auth_list(rlp_item(fields, 9))
        } else {
            [||]
        };

    /* authenticate the witness public key (public_keys[i]) against this tx's
       signature (r,s) over the signing hash; `sender` above is keccak(pubkey)[12:],
       so a forged key would not verify here. */
    let i_v = tx_sig_field(ttype);
    let sig_v = rlp_field_int(rlp_item(fields, i_v));
    let sig_r = rlp_field_uint_word(rlp_item(fields, i_v + 1));
    let sig_s = rlp_field_uint_word(rlp_item(fields, i_v + 2));

    /* The signing preimage content is the RLP of fields[0..i_v), a contiguous
       span of the witness tx bytes: it begins at the payload list's content
       offset (after tx_off, +1 for a typed envelope) and runs for the summed
       full-encoding length of those fields. The build backend frames + hashes
       this span C-side instead of materializing the field list. */
    let (_, sig_cs, _) = rlp_hdr(payload, 0);
    let sig_content_off = (if typed then tx_off + 1 else tx_off) + sig_cs;
    let sig_content_src = byte_slice(WitnessSource, sig_content_off, rlp_fields_prefix_len(fields, i_v));

    /* A tx whose signature does not authenticate its witness public key makes the
       block invalid -- throw at the point of detection (EELS recover_sender). */
    if not_bool(tx_sig_v_ok(ttype, sig_v) & tx_auth_ok(i, tx_signing_hash(ttype, sig_content_src, sig_v), sig_r, sig_s)) then {
        throw InvalidBlock(InvalidSignature)
    };

    struct {
        tx_type = ttype,
        sender = sender,
        nonce = nonce,
        gas_price = gas_price,
        priority_fee = priority,
        gas_limit = to_gas(transaction_gas_limit),
        is_create = is_create,
        recipient = recipient,
        value = value,
        input_src = input_src,
        access_list_addresses = decoded_access_list_addresses,
        access_list_slots = decoded_access_list_slots,
        is_access_list = ttype == 1,
        is_dynamic_fee = (ttype == 2) | (ttype == 3) | (ttype == 4),
        blob_count = list_len(blob_hashes),
        max_fee = max_fee,
        max_blob_fee = max_blob_fee,
        max_priority_fee = max_priority_fee_cap,
        is_set_code = ttype == 4,
        authorizations = decoded_authorizations,
        is_blob = ttype == 3,
        blob_hashes = blob_hashes,
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
function authenticate_parent_state() -> unit = {
    build_node_db();
    register_witness_codes();

    /* Publish the authenticated ancestor-header count to the register BLOCKHASH
       reads (k_blockhash, host/kernel.sail). The guest gets its real witness chain;
       the runner gets its single dummy parent (wit_n_headers() == 1). */
    k_n_headers = wit_n_headers();
    if headers_ok() then {
        k_parent_state_root = parent_state_root()
    } else {
        throw InvalidBlock(WitnessDeficient)
    }
}

function decode_block_header_ssz() -> BlockHeader =
    struct {
        number = pl_block_number(),
        timestamp = pl_timestamp(),
        gas_limit = pl_gas_limit(),
        gas_used = pl_gas_used(),
        prev_randao = pl_prev_randao(),
        base_fee = pl_base_fee(),
        blob_gas_used = pl_blob_gas_used(),
        excess_blob_gas = pl_excess_blob_gas(),
        state_root = pl_state_root(),
        receipts_root = pl_receipts_root(),
        logs_bloom = pl_logs_bloom(),
        block_hash = pl_block_hash(),
        fee_recipient = pl_fee_recipient(),
        parent_hash = ssz_bytes32(payload_start() + 0),
        parent_beacon_block_root = ssz_bytes32(npr_start() + 8),
        slot_number = pl_slot_number(),
    }

/* Build the Withdrawal at index i from the SSZ payload accessors. Transient: the
 * withdrawal set is consumed by index (apply_withdrawals / withdrawals_trie_root),
 * never materialized as a block-wide list. */
function withdrawal_at(i : int) -> Withdrawal =
    struct {
        index = withdrawal_index(i),
        validator_index = withdrawal_validator_index(i),
        address = withdrawal_addr(i),
        amount = withdrawal_amount(i),
    }

/* Decode ONLY the header into the k_header register. Transactions and withdrawals
 * are NOT materialized here: execution loops over their SSZ indices and decodes
 * each on demand (decode_tx_ssz / withdrawal_at), so the full block never lives in
 * program state. A malformed tx / bad signature THROWS during that decode. */
function decode_block_ssz() -> unit = k_set_header(decode_block_header_ssz())

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
