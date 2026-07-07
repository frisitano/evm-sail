/* ======================================================================== */
/* STATELESS BLOCK-VALIDATION GUEST (zkVM)                                  */
/*                                                                          */
/* The stateless block-validation guest for the eth-act zkVM RISC-V target  */
/* (riscv64im_zicclsm). It decodes an SSZ SszStatelessInput from the        */
/* private input, validates the supplied witness against the committed pre- */
/* state root, executes the block over that witness-backed state, and       */
/* commits to the post-state -- emitting an SSZ                             */
/* SszStatelessValidationResult as the canonical public output. A proof of  */
/* this guest is a proof that the block is valid relative to the parent     */
/* state root.                                                              */
/*                                                                          */
/* Validation pipeline (main): re-root the witness to parent.state_root     */
/* (the one-shot pre-state check), seed the touched pre-state from the      */
/* authenticated leaves, run each tx (process_transaction), then check the  */
/* config selects the expected fork, the witness headers chain, the tx      */
/* receipt succeeded, and the recomputed post-state root equals             */
/* payload.state_root. A failing block is a NORMAL outcome: the guest       */
/* reports successful_validation = false, it does not abort.                */
/*                                                                          */
/* Target contract: NO printing, NO string building, NO libc IO, NO asserts */
/* on the happy path. read_input supplies the SSZ input via ssz_src;        */
/* write_output (runtime/zkvm_input.c) receives the SSZ result. Built GMP-  */
/* free by backing the stock Sail GMP ABI with a vendored, freestanding     */
/* mini-gmp (no libgmp linked) at EXACT 256-bit semantics; keccak-256 comes */
/* from a C accelerator (the eth-act c-interface-accelerators path),        */
/* needing no GMP and only memset from libc.                                */
/* ======================================================================== */

/* Output facts, read by the C harness after main(). All fit in 64 bits.     */
register zkvm_out_gas : int = 0 /* total gas used                     */
register zkvm_out_storage0 : int = 0 /* contract storage slot 0            */
register zkvm_out_wbal : int = 0 /* withdrawer balance (wei)           */
register zkvm_out_cbbal : int = 0 /* coinbase balance (wei)             */
register zkvm_out_tracelen : int = 0 /* trace stream length                */
register zkvm_out_success : int = 0 /* 1 if the tx receipt succeeded      */
function eoa() -> address = 0x00000000000000000000000000000000000005e0
function contract() -> address = 0x000000000000000000000000000000000000c0de
function coinbase_a() -> address = 0x000000000000000000000000000000000000c01b
function withdrawer() -> address = 0x0000000000000000000000000000000000004895

/* PUSH1 0x2a; PUSH1 0x00; SSTORE; STOP  (store 42 at slot 0) */
function contract_code() -> list(byte) = [|0x60, 0x2a, 0x60, 0x00, 0x55, 0x00|]

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

function emit_all(bs : list(byte)) -> unit = {
    let n = list_len(bs);
    foreach (i from 0 to (n - 1)) {
        emit_out(list_nth(bs, i, 0x00))
    }
}

/* Build + emit the canonical SSZ SszStatelessValidationResult:
 *   new_payload_request_root (32 BE) ++ successful_validation (1) ++
 *   chain_config offset (37, LE u32) ++ chain_config bytes (copied from input).
 * The chain_config is returned unchanged, so its SSZ bytes are exactly the
 * input's [cc_start, cc_end) region. */
function emit_result(zh : list(bits(256)), success : bool) -> unit = {
    let root = htr_new_payload_request(zh);
    var out : list(byte) = [||];
    foreach (k from 0 to 31) {
        out = sail_shiftright(root, 8 * (31 - k))[7 .. 0] :: out
    };
    out = (if success then 0x01 else 0x00) :: out;
    out = 0x25 :: out;
    out = 0x00 :: out;
    out = 0x00 :: out;
    out = 0x00 :: out;
    let s = cc_start();
    let e = cc_end();
    foreach (i from 0 to (e - s - 1)) {
        out = ssz_src_byte(s + i) :: out
    };
    let result = list_reverse(out);

    /* The canonical SSZ result is validated against the host's remerkleable
       encoding by the external conformance harness (revm-eest/stateless +
       zkvm/gen_vector over the fixture vectors), for both the success and fail
       paths -- so no fixture-specific baked self-check is kept here. */
    emit_all(result)
}

/* Exercise the fixed-width (GMP-free) sailfix runtime's hard arithmetic paths
 * on-guest: 256-bit MUL+truncate, DIV/MOD, the 512-bit MULMOD product, EXP
 * (square-and-multiply), and signed SDIV/SMOD over the negative get_slice_int
 * path. Ground truth computed independently (Python). */
function arith_selfcheck() -> unit = {
    let a : word = 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff;
    let two : word = word_of_int(2);
    assert(alu_mul(a, two) == 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, "alu_mul");
    assert(alu_div(a, two) == 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, "alu_div");
    assert(alu_mod(a, word_of_int(3)) == word_of_int(0), "alu_mod");
    assert(alu_mulmod(a, a, word_of_int(7)) == word_of_int(1), "alu_mulmod 512-bit");

    /* alu_exp (square-and-multiply, 256x 512-bit mulmod) dropped from the hot path:
       ~17M instructions and redundant — EEST validates sailfix's full arithmetic,
       and alu_mulmod above already exercises the 512-bit product path. */
    let neg7 : word = word_of_int(-7);
    assert(alu_sdiv(neg7, two) == 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd, "alu_sdiv");
    assert(alu_smod(neg7, two) == 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, "alu_smod")
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
function decode_access_list(f : RlpField) -> (list(address), list(SlotKey)) = {
    var addresses : list(address) = [||];
    var storage_slots : list(SlotKey) = [||];
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
                                [|struct { address = addr, slot = rlp_field_word(key) }|],
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

/* folded over the block's transactions: every witness pubkey authenticated */
register auth_ok : bool = true

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

    let b0 = if tx_len == 0 then {
            rlp_decode_ok = false;
            0
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
    let (decoded_access_list_addresses, decoded_access_list_slots) : (list(address), list(SlotKey)) = if typed then {
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
    auth_ok =
        auth_ok & tx_sig_v_ok(ttype, sig_v) & tx_auth_ok(i, tx_signing_hash(ttype, sig_content_src, sig_v), sig_r, sig_s);

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
    cd_set_tx(); /* AFTER mem_clear: it points the base frame's calldata at the slot */
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

struct BlockExecutionResult = {
    all_ok : bool,
    gas_acc : int,
    blob_gas_acc : int,
    first_tx_recipient : address,
    block_gas_overflow : bool,
    blob_gas_overflow : bool,
    receipts : list(Receipt),
}

struct Withdrawal = {
    index : int,
    validator_index : int,
    address : address,
    amount : int,
}

struct BlockBody = {
    transactions : list(Transaction),
    withdrawals : list(Withdrawal),
}

struct Block = {
    header : BlockHeader,
    body : BlockBody,
}

/* Materialize the hash-keyed witness node store, anchor the authenticated
 * pre-state root, then reset the execution overlay. There is no eager re-root
 * or harvest pass: every later stateless read is a lazy walk from
 * k_parent_state_root, and because the node-db is keyed by keccak(node) each
 * walk is itself a hash-chain proof -- witness material is authenticated
 * exactly where it is consumed. The anchor register is set only after the
 * witness header chain validates. */
function authenticate_parent_state() -> unit = {
    build_node_db();
    register_witness_codes();
    if headers_ok() then {
        k_parent_state_root = parent_state_root()
    } else {
        k_witness_bad = true
    };
    k_world_reset()
}

function block_env_from_header(header : BlockHeader) -> BlockEnv =
    struct {
        number = word_of_int(header.number),
        timestamp = word_of_int(header.timestamp),
        gas_limit = word_of_int(header.gas_limit),
        coinbase = header.fee_recipient,
        base_fee = header.base_fee,
        prev_randao = header.prev_randao,
        chain_id = word_of_int(cc_chain_id()),
        blob_base_fee = word_of_int(blob_base_fee(header.excess_blob_gas)),
        excess_blob_gas = word_of_int(header.excess_blob_gas),
        slot = word_of_int(header.slot_number),
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

function decode_transactions_ssz(base_fee : word) -> list(Transaction) = {
    auth_ok = true;
    var out : list(Transaction) = [||];
    foreach (i from 0 to (pl_n_txs() - 1)) {
        if rlp_decode_ok then {
            out = decode_tx_ssz(i, base_fee) :: out
        }
    };
    list_reverse(out)
}

function decode_withdrawals_ssz() -> list(Withdrawal) = {
    var out : list(Withdrawal) = [||];
    foreach (i from 0 to (pl_n_withdrawals() - 1)) {
        out =
            struct {
                index = withdrawal_index(i),
                validator_index = withdrawal_validator_index(i),
                address = withdrawal_addr(i),
                amount = withdrawal_amount(i),
            } ::
                out
    };
    list_reverse(out)
}

function decode_block_ssz() -> Block = {
    let header = decode_block_header_ssz();
    struct {
        header = header,
        body =
            struct { transactions = decode_transactions_ssz(header.base_fee), withdrawals = decode_withdrawals_ssz() },
    }
}

function parent_header_int_field(idx : int) -> int = {
    let items = rlp_node_items(ssz_witness_header(wit_n_headers() - 1));
    rlp_field_int(rlp_item(items, idx))
}

function expected_payload_excess_blob_gas() -> int =
    next_excess_blob_gas(parent_header_int_field(18), parent_header_int_field(17), parent_header_int_field(15))

function run_block_start_system_calls(header : BlockHeader) -> unit = {
    /* EIP-4788 / EIP-2935: write the parent beacon-block root and parent block
       hash into their system contracts before the transactions run. */
    system_call(BEACON_ROOTS_ADDR(), header.parent_beacon_block_root);
    system_call(HISTORY_STORAGE_ADDR(), header.parent_hash)
}

function execute_block_transactions(transactions : list(Transaction), block_gas_limit : int) -> BlockExecutionResult = {
    var all_ok : bool = true;
    var gas_acc : int = 0;
    var blob_gas_acc : int = 0;
    var tx0_to : address = ZERO_ADDR; /* tx 0 recipient, for the storage0 fact */
    var block_gas_overflow : bool = false;
    var blob_gas_overflow : bool = false;
    var receipts_rev : list(Receipt) = [||];

    /* EIP-7778 block gas accounting: a tx whose gas limit exceeds the block's
       remaining gas raises GasUsedExceedsLimitError -> the whole block is
       invalid. EELS performs this check BEFORE touching the tx's state, so the
       witness omits the offending tx's proofs; the guest must likewise refrain
       from executing it (and every later tx), else it over-reads the witness
       and aborts on a node EELS never proved. */
    var txs : list(Transaction) = transactions;
    var i : int = 0;
    var more : bool = true;
    while more do {
        match txs {
            tx :: rest => {
                if not_bool(block_gas_overflow | blob_gas_overflow | not_bool(rlp_decode_ok)) then {
                    let tx_blob_gas = tx.blob_count * GAS_PER_BLOB;
                    if i == 0 then {
                        tx0_to = tx.recipient
                    };
                    if not_bool(rlp_decode_ok) then {
                        all_ok = false
                    } else {
                        if tx.gas_limit > block_gas_limit - gas_acc then {
                            block_gas_overflow = true
                        } else {
                            if tx_blob_gas > blob_max_gas_per_block() - blob_gas_acc then {
                                blob_gas_overflow = true
                            } else {
                                let r = process_transaction(tx);
                                all_ok = all_ok & r.valid;
                                gas_acc = gas_acc + r.block_gas;
                                if r.valid then {
                                    receipts_rev = r :: receipts_rev;
                                    blob_gas_acc = blob_gas_acc + tx_blob_gas
                                }
                            }
                        }
                    }
                };
                txs = rest;
                i = i + 1
            },
            [||] => more = false,
        }
    };
    struct {
        all_ok = all_ok & rlp_decode_ok & not_bool(block_gas_overflow) & not_bool(blob_gas_overflow),
        gas_acc = gas_acc,
        blob_gas_acc = blob_gas_acc,
        first_tx_recipient = tx0_to,
        block_gas_overflow = block_gas_overflow,
        blob_gas_overflow = blob_gas_overflow,
        receipts = list_reverse(receipts_rev),
    }
}

function apply_withdrawals(withdrawals : list(Withdrawal)) -> unit = {
    var rest : list(Withdrawal) = withdrawals;
    var more : bool = true;
    while more do {
        match rest {
            w :: tail => {
                /* EIP-4895: the withdrawal `amount` field is denominated in Gwei; the
                   credited balance is amount * 10^9 wei. */
                k_add_balance(w.address, word_of_int(w.amount * 1000000000));
                rest = tail
            },
            [||] => more = false,
        }
    }
}

function run_checked_block_end_system_calls(all_ok : bool) -> bool = {
    /* EIP-7002 / EIP-7251: dequeue the withdrawal + consolidation requests via a
       CHECKED system call to each predeploy (clearing their queue storage). EELS
       reaches this only if no earlier step raised, so run it only while the block
       is still valid; a missing predeploy or a failed call invalidates the block. */
    if all_ok then {
        let rw = system_call_checked(WITHDRAWAL_REQUEST_ADDR());
        let rc = system_call_checked(CONSOLIDATION_REQUEST_ADDR());
        all_ok & rw & rc
    } else {
        false
    }
}

function trie_index_key(i : int) -> list(nibble) =
    to_nibbles(rlp_int(i))

function transaction_trie_root() -> hash = {
    var updates : list(TriePatch) = [||];
    let n = pl_n_txs();
    foreach (i from 0 to (n - 1)) {
        let (off, len) = ssz_list_span(pl_txs_start(), n, pl_wds_start(), i);
        updates = patch_insert_sorted(patch_leaf(trie_index_key(i), byte_slice(WitnessSource, off, len)), updates)
    };
    trie_root(EMPTY_TRIE_ROOT, updates)
}

function withdrawal_rlp(w : Withdrawal) -> list(byte) = {
    var payload : list(byte) = [||];
    payload = list_append(payload, rlp_int(w.index));
    payload = list_append(payload, rlp_int(w.validator_index));
    payload = list_append(payload, rlp_addr(w.address));
    payload = list_append(payload, rlp_int(w.amount));
    rlp_list(payload)
}

function withdrawals_trie_root(withdrawals : list(Withdrawal)) -> hash = {
    trie_arena_reset();
    var updates : list(TriePatch) = [||];
    var rest : list(Withdrawal) = withdrawals;
    var i : int = 0;
    var more : bool = true;
    while more do {
        match rest {
            w :: tail => {
                updates = patch_insert_sorted(patch_leaf(trie_index_key(i), arena_intern(withdrawal_rlp(w))), updates);
                rest = tail;
                i = i + 1
            },
            [||] => more = false,
        }
    };
    trie_root(EMPTY_TRIE_ROOT, updates)
}

function bloom_set_bit(bloom : list(byte), bit_to_set : int) -> list(byte) = {
    let bit_index = 2047 - bit_to_set;
    let byte_index = ediv_int(bit_index, 8);
    let bit_value = pow2(7 - emod_int(bit_index, 8));
    let old = bytes_get(bloom, byte_index);
    list_set(bloom, byte_index, or_vec(old, get_slice_int(8, bit_value, 0)))
}

/* One of a LOG entry's bloom filter bits (Yellow Paper section 4.4.1):
 * bytes (2i, 2i+1) of the entry keccak hash, mod 2048. The entry is always
 * a 20-byte address or a 32-byte topic, so callers hash it via the
 * fixed-width keccak256_address / keccak256_word FFI -- no list(byte)
 * reaches the build hash on this path. */
function bloom_bit(h : bits(256), i : int) -> int = {
    let hb = word_to_bytes32(h);
    let j = i * 2;
    emod_int(unsigned(bytes_get(hb, j)) * 256 + unsigned(bytes_get(hb, j + 1)), 2048)
}

function bloom_add_entry_hash(bloom : list(byte), h : bits(256)) -> list(byte) = {
    var out : list(byte) = bloom;
    foreach (i from 0 to 2) {
        out = bloom_set_bit(out, bloom_bit(h, i))
    };
    out
}

function bloom_add_topics(bloom : list(byte), topics : list(word)) -> list(byte) = {
    var out : list(byte) = bloom;
    var rest : list(word) = topics;
    var more : bool = true;
    while more do {
        match rest {
            topic :: tail => {
                out = bloom_add_entry_hash(out, keccak256_word(topic));
                rest = tail
            },
            [||] => more = false,
        }
    };
    out
}

function bloom_add_log(bloom : list(byte), log : LogEntry) -> list(byte) = {
    let with_address = bloom_add_entry_hash(bloom, keccak256_address(log.address));
    bloom_add_topics(with_address, log.topics)
}

function logs_bloom_for_logs(logs : list(LogEntry)) -> list(byte) = {
    var out : list(byte) = zero_bytes(256);
    var rest : list(LogEntry) = logs;
    var more : bool = true;
    while more do {
        match rest {
            log :: tail => {
                out = bloom_add_log(out, log);
                rest = tail
            },
            [||] => more = false,
        }
    };
    out
}

function receipts_bloom(receipts : list(Receipt)) -> list(byte) = {
    var out : list(byte) = zero_bytes(256);
    var rest : list(Receipt) = receipts;
    var more : bool = true;
    while more do {
        match rest {
            receipt :: tail => {
                var logs : list(LogEntry) = receipt.logs;
                var more_logs : bool = true;
                while more_logs do {
                    match logs {
                        log :: log_tail => {
                            out = bloom_add_log(out, log);
                            logs = log_tail
                        },
                        [||] => more_logs = false,
                    }
                };
                rest = tail
            },
            [||] => more = false,
        }
    };
    out
}

function bytes_equal(a : list(byte), b : list(byte)) -> bool = {
    if list_len(a) == list_len(b) then {
        var ok : bool = true;
        foreach (i from 0 to (list_len(a) - 1)) {
            ok = ok & (bytes_get(a, i) == bytes_get(b, i))
        };
        ok
    } else {
        false
    }
}

function topics_rlp(topics : list(word)) -> list(byte) = {
    var payload : list(byte) = [||];
    var rest : list(word) = topics;
    var more : bool = true;
    while more do {
        match rest {
            topic :: tail => {
                payload = list_append(payload, rlp_word(topic));
                rest = tail
            },
            [||] => more = false,
        }
    };
    rlp_list(payload)
}

function log_entry_rlp(log : LogEntry) -> list(byte) = {
    var payload : list(byte) = [||];
    payload = list_append(payload, rlp_addr(log.address));
    payload = list_append(payload, topics_rlp(log.topics));
    payload = list_append(payload, rlp_bytes(log.data));
    rlp_list(payload)
}

function logs_rlp(logs : list(LogEntry)) -> list(byte) = {
    var payload : list(byte) = [||];
    var rest : list(LogEntry) = logs;
    var more : bool = true;
    while more do {
        match rest {
            log :: tail => {
                payload = list_append(payload, log_entry_rlp(log));
                rest = tail
            },
            [||] => more = false,
        }
    };
    rlp_list(payload)
}

function receipt_payload_rlp(r : Receipt, cumulative_gas_used : int) -> list(byte) = {
    var payload : list(byte) = [||];
    payload = list_append(payload, rlp_int(if r.success then { 1 } else { 0 }));
    payload = list_append(payload, rlp_int(cumulative_gas_used));
    payload = list_append(payload, rlp_bytes(logs_bloom_for_logs(r.logs)));
    payload = list_append(payload, logs_rlp(r.logs));
    rlp_list(payload)
}

function receipt_encoded(r : Receipt, cumulative_gas_used : int) -> list(byte) = {
    let payload = receipt_payload_rlp(r, cumulative_gas_used);
    if r.tx_type == 0 then {
        payload
    } else {
        let type_byte : byte = get_slice_int(8, r.tx_type, 0);
        type_byte :: payload
    }
}

function receipts_trie_root(receipts : list(Receipt)) -> hash = {
    trie_arena_reset();
    var updates : list(TriePatch) = [||];
    var rest : list(Receipt) = receipts;
    var i : int = 0;
    var cumulative : int = 0;
    var more : bool = true;
    while more do {
        match rest {
            r :: tail => {
                cumulative = cumulative + r.gas_used;
                updates = patch_insert_sorted(patch_leaf(trie_index_key(i), arena_intern(receipt_encoded(r, cumulative))), updates);
                rest = tail;
                i = i + 1
            },
            [||] => more = false,
        }
    };
    trie_root(EMPTY_TRIE_ROOT, updates)
}

function parent_hash_ok(header : BlockHeader) -> bool =
    if wit_n_headers() < 1 then {
        false
    } else {
        header.parent_hash == witness_keccak(ssz_witness_header_slice(wit_n_headers() - 1))
    }

function validate_block_outputs(block : Block, exec_ok : bool, block_result : BlockExecutionResult) -> bool = {
    /* S5: stateless validation outcome (a failure is a NORMAL result -- the guest
       reports successful_validation=false, it does not abort):
         - chain config selects the Amsterdam fork
         - the witness headers are contiguous
         - RLP scalar fields are canonical integers
         - every tx's witness public key is authenticated against its signature
         - every tx is VALID/applicable (a reverting valid tx keeps the block valid)
         - the recomputed post-state root commits to payload.state_root */
    let header = block.header;
    let fork_ok = cc_fork() == 20; /* canonical ProtocolFork index for Amsterdam
                                  (execution-specs@02c6c2510: enum pruned, Osaka=17 BPO1=18 BPO2=19
                                  Amsterdam=20). */

    let active_ok = cc_activation_active();
    let cfg_ok = fork_ok & active_ok;

    let hdr_ok = headers_ok();
    let gas_used_ok = block_result.gas_acc == header.gas_used;
    let blob_gas_used_ok = block_result.blob_gas_acc == header.blob_gas_used;
    let excess_blob_gas_ok = header.excess_blob_gas == expected_payload_excess_blob_gas();
    let computed_root = compute_state_root();
    let expected_root = header.state_root;
    let poststate_ok = computed_root == expected_root;
    let receipts_root = receipts_trie_root(block_result.receipts);
    let receipts_root_ok = receipts_root == header.receipts_root;
    let logs_bloom_ok = bytes_equal(receipts_bloom(block_result.receipts), header.logs_bloom);
    let transactions_root = transaction_trie_root();
    let withdrawals_root = withdrawals_trie_root(block.body.withdrawals);
    let requests_hash = execution_requests_hash();
    let block_access_list_hash = witness_keccak(witness_slice(pl_bal_start(), payload_end() - pl_bal_start()));
    let block_hash = block_header_hash(
        header,
        transactions_root,
        withdrawals_root,
        requests_hash,
        block_access_list_hash,
    );
    let block_hash_ok = block_hash == header.block_hash;
    let parent_ok = parent_hash_ok(header);

    /* k_witness_bad: a deficient/inconsistent witness proof (missing trie node,
       a pre/post storage trie that does not re-root) -- the block cannot be
       validated, so it is invalid (EIP-8025 optional-proofs negative tests). */
    /* EIP-7928: the block access list must not exceed block_gas_limit / 2000 items. */
    let bal_limit = ediv_int(header.gas_limit, 2000);
    let bal_ok : bool = bal_item_count() <= bal_limit;

      cfg_ok
    & hdr_ok
    & rlp_decode_ok
    & auth_ok
    & exec_ok
    & gas_used_ok
    & blob_gas_used_ok
    & excess_blob_gas_ok
    & poststate_ok
    & receipts_root_ok
    & logs_bloom_ok
    & block_hash_ok
    & parent_ok
    & bal_ok
    & not_bool(k_witness_bad)
}

function first_withdrawal_address(withdrawals : list(Withdrawal)) -> address =
    match withdrawals {
        w :: _ => w.address,
        [||] => ZERO_ADDR,
    }

/* canonical "invalid input" output: a 32-byte ZERO new_payload_request_root,
 * successful_validation=false, and a FIXED default ChainConfig (the input's is
 * unparseable, so it cannot be echoed). The 36 default-config bytes are the SSZ
 * of the empty ChainConfig the EEST consumer emits for rejected input. */
function emit_invalid() -> unit = {
    let dflt : list(byte) = [|
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x0c,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x10,
            0x00,
            0x00,
            0x00,
            0x18,
            0x00,
            0x00,
            0x00,
            0x08,
            0x00,
            0x00,
            0x00,
            0x08,
            0x00,
            0x00,
            0x00,
        |];
    var out : list(byte) = [||];
    foreach (k from 0 to 31) {
        out = 0x00 :: out
    };
    out = 0x00 :: out; /* successful_validation = false */
    out = 0x25 :: out; /* chain_config offset = 37 (LE u32) */
    out = 0x00 :: out;
    out = 0x00 :: out;
    out = 0x00 :: out;
    emit_all(list_reverse(out));
    emit_all(dflt)
}

function main() -> unit = {
    /* input-independent self-checks (valid for any vector): the C accelerators
       and the GMP-free fixed-width int/bits runtime. The per-stage fixture
       self-checks (ssz/rlp/witness/htr) were incremental scaffolding subsumed by
       the end-to-end success/fail paths + the success-path output check. */
    keccak_selfcheck();
    sha256_selfcheck();
    arith_selfcheck();
    rlp_decode_ok = true;

    /* reject a malformed SSZ input outright with the canonical invalid output
       (zero root + valid=false + default chain_config) -- nothing below can
       parse it, and the field spans would otherwise read garbage. */
    if not_bool(input_well_formed()) then {
        emit_invalid();
        return ()
    };

    /* S2f: decode the block from private input, then execute it against witness
       state. Transactions and withdrawals are materialized here; execution only
       stages a transaction's input into the byte-addressed backend when that
       transaction is actually run. */
    let block : Block = decode_block_ssz();
    authenticate_parent_state();

    /* The witness backs the base layer for BOTH accounts and storage (the
       witness base backend, host/base_witness.sail, statically linked into this
       entry). A touched account is proven present-or-absent by a lazy Merkle
       walk from the authenticated parent root on first load_account (then
       cached raw in the acctmap); a touched slot is proven the same way from
       the account's storage root on first read (k_sload/k_orig) and seeded into
       the working set. Nothing is pre-seeded -- no eager account/slot pass. */

    /* Block STF over the materialized block: system calls, each decoded tx in
       order via the shared per-tx STF unit, then withdrawals and block-end
       system calls. */
    k_set_block(block_env_from_header(block.header));
    run_block_start_system_calls(block.header);

    let block_result = execute_block_transactions(block.body.transactions, block.header.gas_limit);
    apply_withdrawals(block.body.withdrawals);
    let exec_ok : bool = run_checked_block_end_system_calls(block_result.all_ok);
    let valid : bool = validate_block_outputs(block, exec_ok, block_result);

    let stored = k_storage_get(block_result.first_tx_recipient, word_of_int(0));
    let gas_used = to_gas(block_result.gas_acc);
    let wbal = load_account(first_withdrawal_address(block.body.withdrawals)).balance;
    let cbbal = load_account(block.header.fee_recipient).balance;

    zkvm_out_gas = gas_used;
    zkvm_out_storage0 = unsigned(stored);
    zkvm_out_wbal = unsigned(wbal);
    zkvm_out_cbbal = unsigned(cbbal);
    zkvm_out_tracelen = 0; /* the ELOp trace machinery was removed */
    zkvm_out_success = if valid then { 1 } else { 0 };

    /* emit the canonical SSZ SszStatelessValidationResult as public output */
    emit_result(build_zerohashes(40), valid)
}
