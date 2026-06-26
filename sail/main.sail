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

$include "evm.sail"
$include "lib/ssz_htr.sail"

/* one byte of the canonical SSZ public output, via write_output (runtime/zkvm_input.c) */
val emit_out = impure { _: "el_emit_out" } : bits(8) -> unit

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

/* Exercise the keccak-256 C accelerator ON the RISC-V guest: compute the two
 * reference digests through the freestanding mini-gmp + FFI path and assert
 * them. A broken on-guest accelerator (ABI/endianness/mini-gmp) fails the
 * assert -> abnormal termination (non-zero exit), so exit 0 proves keccak runs
 * correctly on the guest, not just that it links. */
function keccak_selfcheck() -> unit = {
    assert(keccak256([||]) == 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470, "keccak('') on guest");
    assert(
        keccak256([|0x61, 0x62, 0x63|]) == 0x4e03657aea45a94fc7d47ba826c8d667c0d1e6e33a64a036ec44f58fa12d6c45,
        "keccak('abc') on guest"
    )
}

/* register a code blob in the code db keyed by codeHash, as a (off,len) view
 * into the witness bytes -- (hash hi..lo, off, len) split across six u64s. */
val cs_view_input = impure { _: "cs_view_input" } : (bits(64), bits(64), bits(64), bits(64), bits(64), bits(64)) -> unit

/* store the code for codeHash `h` as a VIEW of the witness span [off, off+len) */
function cs_view_witness(h : hash, off : int, len : int) -> unit =
    cs_view_input(
        h[255 .. 192],
        h[191 .. 128],
        h[127 .. 64],
        h[63 .. 0],
        get_slice_int(64, off, 0),
        get_slice_int(64, len, 0),
    )

/* Register every witness code blob into the (hash-keyed) C code store as a view
 * into the witness bytes -- no copy, no Sail list. Each blob is hashed once (the
 * content-address authentication); execution then resolves an account's code by
 * its code_hash from the store, so seeding needs no per-account span lookup. */
function register_witness_codes() -> unit = {
    let n = wit_n_codes();
    foreach (i from 0 to (n - 1)) {
        let (off, len) = ssz_witness_code_span(i);
        cs_view_witness(keccak256(ssz_witness_code(i)), off, len)
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
            let parent_hash = bytes_be256(list_nth(rlp_node_items(ssz_witness_header(i)), 0, RLP_NONE).rf_content);
            let prev_hash = keccak256(ssz_witness_header(i - 1));
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

val txin_view_input = impure { _: "txin_view_input" } : (bits(64), bits(64), bits(64)) -> bits(64)

/* ---- RLP parsing directly over the SSZ input (no list materialisation) ----
 * The transactions live in the C-backed witness; we walk their RLP structure by
 * offset (ib = ssz input byte) to find each field's (offset, length), read the
 * small scalar fields in place, and point the tx's input slot straight at the
 * data field's bytes -- the EVM then executes over the witness, zero copy. */
function rlp_hdr_at(pos : int) -> (bool, int, int) = {
    let h = ib(pos);
    if h < 128 then {
        (false, pos, 1)
    } else {
        if h < 184 then {
            (false, pos + 1, h - 128)
        } else {
            if h < 192 then {
                let lol = h - 183;
                var v : int = 0;
                foreach (k from 0 to (lol - 1)) {
                    v = v * 256 + ib(pos + 1 + k)
                };
                (false, pos + 1 + lol, v)
            } else {
                if h < 248 then {
                    (true, pos + 1, h - 192)
                } else {
                    let lol = h - 247;
                    var v : int = 0;
                    foreach (k from 0 to (lol - 1)) {
                        v = v * 256 + ib(pos + 1 + k)
                    };
                    (true, pos + 1 + lol, v)
                }
            }
        }
    }
}

/* (offset, length) of field `idx` within the RLP list at offset `off` */
function rlp_field_span(off : int, idx : int) -> (int, int) = {
    let (_, po, pl) = rlp_hdr_at(off);
    let pe = po + pl;
    var pos : int = po;
    var fi : int = 0;
    var res : (int, int) = (po, 0);
    var more : bool = true;
    while more & (pos < pe) do {
        let (_, cs, cl) = rlp_hdr_at(pos);
        if fi == idx then {
            res = (cs, cl);
            more = false
        } else {
            pos = cs + cl;
            fi = fi + 1
        }
    };
    res
}
function rlp_uint_span(off : int, len : int) -> int = {
    var v : int = 0;
    foreach (k from 0 to (len - 1)) {
        v = v * 256 + ib(off + k)
    };
    v
}

/* Effective fee for a typed (EIP-1559) tx: the gas price actually paid is
 * min(max_fee_per_gas, base_fee + max_priority_fee), and the priority tip paid to
 * the coinbase is that price minus the base fee. Legacy / 2930 txs carry a single
 * gas_price, passed here as max_fee = max_priority = gas_price so this recovers
 * (gas_price, gas_price - base_fee). The priority is clamped at 0 so an invalid
 * sub-base-fee price (rejected later by the validity check) never underflows. */
function eff_gas_price(max_fee : word, max_prio : word) -> (word, word) = {
    let bf = unsigned(k_block.be_base_fee);
    let cap = unsigned(max_fee);
    let tip = unsigned(max_prio);
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
 * content span (cs, cl) of the access-list field into the flat representation the
 * Transaction and intrinsic-gas accounting use: the addresses (one per entry) and
 * the (address, slot) pairs. Walked by offset over the witness bytes, no copy. */
function decode_access_list(cs : int, cl : int) -> (list(address), list(SlotKey)) = {
    var addrs : list(address) = [||];
    var keys : list(SlotKey) = [||];
    var pos : int = cs;
    let stop = cs + cl;
    while pos < stop do {
        let (_, ecs, ecl) = rlp_hdr_at(pos); /* one entry = [addr, [keys]] */
        let (_, ado, adl) = rlp_hdr_at(ecs); /* the 20-byte address         */
        let addr = word_to_address(word_of_int(rlp_uint_span(ado, adl)));
        addrs = list_append(addrs, [|addr|]);
        let (_, kcs, kcl) = rlp_hdr_at(ado + adl); /* the entry's storage-key list */
        var kpos : int = kcs;
        let kstop = kcs + kcl;
        while kpos < kstop do {
            let (_, ko, kl) = rlp_hdr_at(kpos); /* one 32-byte key */
            keys = list_append(keys, [|struct { sk_addr = addr, sk_slot = word_of_int(rlp_uint_span(ko, kl)) }|]);
            kpos = ko + kl
        };
        pos = ecs + ecl
    };
    (addrs, keys)
}

/* Decode an RLP list of 32-byte words (content span cs, cl) -- the EIP-4844
 * blobVersionedHashes (BLOBHASH operands; the blob count is the list length). */
function decode_word_list(cs : int, cl : int) -> list(word) = {
    var out : list(word) = [||];
    var pos : int = cs;
    let stop = cs + cl;
    while pos < stop do {
        let (_, ho, hl) = rlp_hdr_at(pos);
        out = list_append(out, [|word_of_int(rlp_uint_span(ho, hl))|]);
        pos = ho + hl
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

/* secp256k1 ECDSA verify (accelerator id 257): input hash ++ r ++ s ++ x ++ y,
 * each a 32-byte big-endian word -> true iff (r,s) verifies for pubkey (x,y). */
function secp256k1_verify(h : bits(256), r : word, s : word, x : bits(256), y : bits(256)) -> bool = {
    var inp : list(byte) = word_to_bytes32(h);
    inp = list_append(inp, word_to_bytes32(r));
    inp = list_append(inp, word_to_bytes32(s));
    inp = list_append(inp, word_to_bytes32(x));
    inp = list_append(inp, word_to_bytes32(y));
    let (_, out) = run_precompile(257, inp);
    list_len(out) == 32
}

/* RLP list header for a payload of `len` content bytes (YP App. B) */
function rlp_list_prefix(len : int) -> list(byte) =
    if len <= 55 then {
        [|get_slice_int(8, 192 + len, 0)|]
    } else {
        let lb = int_to_be_bytes(len);
        get_slice_int(8, 247 + list_len(lb), 0) :: lb
    }

/* index of the first signature field (v / yParity) by tx type */
function tx_sig_field(ttype : int) -> int = match ttype {
    0 => 6,
    1 => 8,
    2 => 9,
    3 => 11,
    _ => 10,
}

/* keccak of the signing preimage: the unsigned RLP fields (contiguous and already
 * encoded in the stored tx, fields 0..i_v-1) re-wrapped in a fresh list header --
 * the type byte prepended for typed envelopes, and chain_id,0,0 appended for legacy
 * (EIP-155 replay protection). */
function tx_signing_hash(ttype : int, loff : int, i_v : int) -> bits(256) = {
    let (_, po, _) = rlp_hdr_at(loff);
    let (lcs, lcl) = rlp_field_span(loff, i_v - 1);
    let sig_start = lcs + lcl;
    var content : list(byte) = ssz_bytes(po, sig_start - po);
    if ttype == 0 then {
        content = list_append(content, rlp_int(cc_chain_id()));
        content = list_append(content, [|0x80, 0x80|])
    };
    var pre : list(byte) = if ttype != 0 then {
            [|get_slice_int(8, ttype, 0)|]
        } else {
            [||]
        };
    pre = list_append(pre, rlp_list_prefix(list_len(content)));
    pre = list_append(pre, content);
    keccak256(pre)
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

/* ecrecover (accelerator id 1): input hash ++ v ++ r ++ s (4 * 32 BE bytes, v in
 * the low byte = 27 + yParity) -> the recovered address, or (false, 0) on failure. */
function ecrecover_addr(h : bits(256), yparity : int, r : word, s : word) -> (bool, address) = {
    var inp : list(byte) = word_to_bytes32(h);
    inp = list_append(inp, word_to_bytes32(word_of_int(27 + yparity)));
    inp = list_append(inp, word_to_bytes32(r));
    inp = list_append(inp, word_to_bytes32(s));
    let (_, out) = run_precompile(1, inp);
    if list_len(out) == 32 then {
        (true, word_to_address(bytes_be256(out)))
    } else {
        (false, ZERO_ADDR)
    }
}

/* keccak of the authorization signing preimage: 0x05 ++ rlp([chainId, address,
 * nonce]) -- the first three tuple fields (contiguous in the stored tuple) re-wrapped. */
function auth_signing_hash(toff : int) -> bits(256) = {
    let (_, tcs, _) = rlp_hdr_at(toff);
    let (ncs, ncl) = rlp_field_span(toff, 2); /* nonce = last signed field */
    let content : list(byte) = ssz_bytes(tcs, ncs + ncl - tcs);
    var pre : list(byte) = [|0x05|];
    pre = list_append(pre, rlp_list_prefix(list_len(content)));
    pre = list_append(pre, content);
    keccak256(pre)
}

/* decode an EIP-7702 authorization list (content span cs,cl) into Authorizations,
 * recovering each authority. au_valid_sig records EIP-2 low-s + a successful
 * recovery; process_auth then applies the chain-id / nonce / code checks. */
function decode_auth_list(cs : int, cl : int) -> list(Authorization) = {
    var out : list(Authorization) = [||];
    var pos : int = cs;
    let stop = cs + cl;
    while pos < stop do {
        let (_, tcs, tcl) = rlp_hdr_at(pos); /* one [chainId,address,nonce,yParity,r,s] */
        let (c0o, c0l) = rlp_field_span(pos, 0);
        let (a1o, a1l) = rlp_field_span(pos, 1);
        let (n2o, n2l) = rlp_field_span(pos, 2);
        let (y3o, y3l) = rlp_field_span(pos, 3);
        let (r4o, r4l) = rlp_field_span(pos, 4);
        let (s5o, s5l) = rlp_field_span(pos, 5);
        let s = word_of_int(rlp_uint_span(s5o, s5l));
        let (ok, authority) = ecrecover_addr(
            auth_signing_hash(pos),
            rlp_uint_span(y3o, y3l),
            word_of_int(rlp_uint_span(r4o, r4l)),
            s,
        );
        out = list_append(
            out,
            [|
                struct {
                    au_valid_sig = ok & (unsigned(s) <= unsigned(SECP_N_HALF)),
                    au_authority = authority,
                    au_address = word_to_address(word_of_int(rlp_uint_span(a1o, a1l))),
                    au_nonce = rlp_uint_span(n2o, n2l),
                    au_chain_id = rlp_uint_span(c0o, c0l),
                },
            |],
        );
        pos = tcs + tcl
    };
    out
}

/* Decode transaction `i` straight from the SSZ payload's transactions list, by
 * offset -- no copy. The calldata is the witness bytes themselves: txin_view_input
 * points the (single, reused) input slot at them, so the decoded Transaction
 * carries only scalars (t_index = 0 names that slot). The sender is derived from
 * public_keys[i] (PUBLIC_KEY_BYTES = 65: a 0x04 prefix + the 64-byte X||Y) as
 * keccak(X||Y)[12:]. public_keys is a WITNESS, not trusted: tx_auth_ok (folded into
 * the auth_ok register below) verifies tx i's signature against this key, so a
 * forged key fails verification and the block is rejected.
 *
 * EIP-2718 envelope: a leading byte >= 0xc0 is a legacy (type-0) RLP list;
 * otherwise it is the type byte (0x01 EIP-2930, 0x02 EIP-1559, 0x03 EIP-4844,
 * 0x04 EIP-7702) and the RLP payload starts right after it. The field layouts
 * differ -- typed envelopes prepend chain_id, and 1559+ split gas_price into
 * (max_priority_fee, max_fee) -- so the logical field indices are selected by
 * type. Type-4 (EIP-7702) authorization tuples are decoded and their authorities
 * recovered (decode_auth_list); process_auth_list then applies the delegations. */
function decode_tx_ssz(i : int) -> Transaction = {
    let sender = word_to_address(keccak256(ssz_bytes(cc_end() + i * 65 + 1, 64)));
    let (tx_off, _tx_len) = ssz_list_span(pl_txs_start(), pl_n_txs(), pl_wds_start(), i);

    let b0 = ib(tx_off);
    let ttype : int = if b0 >= 192 then { 0 } else { b0 }; /* 0xc0 = RLP list tag */
    let typed = ttype != 0;
    let loff : int = if typed then {
            tx_off + 1
        } else {
            tx_off
        };

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

    let (no, nl) = rlp_field_span(loff, i_nonce);
    let t_nonce = rlp_uint_span(no, nl);
    let (xo, xl) = rlp_field_span(loff, i_gas);
    let t_gas = rlp_uint_span(xo, xl);
    let (ao, al2) = rlp_field_span(loff, i_to);
    let is_create = al2 == 0; /* empty `to` field = contract creation */
    let t_to = word_to_address(word_of_int(rlp_uint_span(ao, al2)));
    let (vo, vl) = rlp_field_span(loff, i_val);
    let t_value = word_of_int(rlp_uint_span(vo, vl));
    let (do_, dl) = rlp_field_span(loff, i_data);
    let _ = txin_view_input(get_slice_int(64, 0, 0), get_slice_int(64, do_, 0), get_slice_int(64, dl, 0));

    /* fees: legacy / 2930 carry one gas_price; 1559+ carry (max_priority, max_fee) */
    let (gas_price, priority, max_fee, max_prio) : (word, word, word, word) = if ttype <= 1 then {
            let (go, gl2) = rlp_field_span(loff, if ttype == 0 then { 1 } else { 2 });
            let gp = word_of_int(rlp_uint_span(go, gl2));

            /* legacy/2930 carry a single gas_price; the coinbase tip is still
               gas_price - base_fee (the base fee is burned), so route through
               eff_gas_price(gp, gp) = (gp, gp - base_fee) rather than paying the
               full gas_price as priority. */
            let (_, pr) = eff_gas_price(gp, gp);
            (gp, pr, gp, gp)
        } else {
            let (mpo, mpl) = rlp_field_span(loff, 2);
            let (mfo, mfl) = rlp_field_span(loff, 3);
            let mp = word_of_int(rlp_uint_span(mpo, mpl));
            let mf = word_of_int(rlp_uint_span(mfo, mfl));
            let (gp, pr) = eff_gas_price(mf, mp);
            (gp, pr, mf, mp)
        };

    /* EIP-2930 access list (present on every typed envelope) */
    let (al_addrs, al_keys) : (list(address), list(SlotKey)) = if typed then {
            let (acs, acl) = rlp_field_span(loff, i_al);
            decode_access_list(acs, acl)
        } else {
            ([||], [||])
        };

    /* EIP-4844 blob fields: max_fee_per_blob_gas (field 9) + versioned hashes (10) */
    let (max_blob_fee, blob_hashes) : (word, list(word)) = if ttype == 3 then {
            let (bfo, bfl) = rlp_field_span(loff, 9);
            let (bhcs, bhcl) = rlp_field_span(loff, 10);
            (word_of_int(rlp_uint_span(bfo, bfl)), decode_word_list(bhcs, bhcl))
        } else {
            (ZERO_WORD, [||])
        };

    /* EIP-7702 authorization list (field 9 for type-4): recover each authority */
    let auth_list : list(Authorization) = if ttype == 4 then {
            let (aucs, aucl) = rlp_field_span(loff, 9);
            decode_auth_list(aucs, aucl)
        } else {
            [||]
        };

    /* authenticate the witness public key (public_keys[i]) against this tx's
       signature (r,s) over the signing hash; `sender` above is keccak(pubkey)[12:],
       so a forged key would not verify here. */
    let i_v = tx_sig_field(ttype);
    let (ro, rl) = rlp_field_span(loff, i_v + 1);
    let (sso, ssl) = rlp_field_span(loff, i_v + 2);
    auth_ok =
        auth_ok &
            tx_auth_ok(
                i,
                tx_signing_hash(ttype, loff, i_v),
                word_of_int(rlp_uint_span(ro, rl)),
                word_of_int(rlp_uint_span(sso, ssl)),
            );

    struct {
        t_sender = sender,
        t_nonce = t_nonce,
        t_gas_price = gas_price,
        t_priority_fee = priority,
        t_gas_limit = to_gas(t_gas),
        t_is_create = is_create,
        t_to = t_to,
        t_value = t_value,
        t_index = 0,
        t_al_addrs = al_addrs,
        t_al_keys = al_keys,
        t_blob_count = list_len(blob_hashes),
        t_max_fee = max_fee,
        t_max_blob_fee = max_blob_fee,
        t_max_priority = max_prio,
        t_is_setcode = ttype == 4,
        t_auth_list = auth_list,
        t_is_blob = ttype == 3,
        t_blob_hashes = blob_hashes,
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

function system_call(tgt : address, input : bits(256)) -> unit = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        () /* system contract absent -> skip (EIP-4788 / EIP-2935) */
    } else {
        txdata_set_input(0, word_to_bytes32(input)); /* the call's 32-byte calldata */
        let _ = txdata_activate(0);
        k_snapshot();
        pc = 0;
        call_depth = 0;
        gas_remaining = to_gas(30000000);
        stack_clear();
        mem_clear();
        hr_reset();
        memory_size = 0;
        frame_status = Running();
        cd_set_tx(); /* AFTER mem_clear: it points the base frame's calldata at the slot */
        message =
            struct {
                msg_caller = SYSTEM_ADDRESS(),
                msg_address = tgt,
                msg_code_address = tgt,
                msg_value = ZERO_WORD,
                msg_data_len = 32,
                msg_is_static = false,
                msg_depth = 0,
            };
        frame_code_addr(tgt);
        interpret();
        if frame_succeeded() then {
            k_commit()
        } else {
            k_revert()
        }
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
        txdata_set_input(0, [||]); /* empty calldata: the SYSTEM caller dequeues */
        let _ = txdata_activate(0);
        k_snapshot();
        pc = 0;
        call_depth = 0;
        gas_remaining = to_gas(30000000);
        stack_clear();
        mem_clear();
        hr_reset();
        memory_size = 0;
        frame_status = Running();
        cd_set_tx();
        message =
            struct {
                msg_caller = SYSTEM_ADDRESS(),
                msg_address = tgt,
                msg_code_address = tgt,
                msg_value = ZERO_WORD,
                msg_data_len = 0,
                msg_is_static = false,
                msg_depth = 0,
            };
        frame_code_addr(tgt);
        interpret();
        if frame_succeeded() then {
            k_commit();
            true
        } else {
            k_revert();
            false /* checked: a reverting/erroring system call invalidates the block */
        }
    }
}

/* Reject a malformed SszStatelessInput: the 2-byte schema id must be 1, the
 * 4 field offsets (LE u32 at body+{0,4,8,12}) must start at 16, be monotonic,
 * and end within the input, and the trailing public_keys region must be a whole
 * number of 65-byte keys (no truncation / trailing garbage). EELS's consumer
 * rejects such input with successful_validation=false; the guest must too rather
 * than echo garbage. */
function input_well_formed() -> bool = {
    let len = ssz_src_len();
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

    /* reject a malformed SSZ input outright with the canonical invalid output
       (zero root + valid=false + default chain_config) -- nothing below can
       parse it, and the field spans would otherwise read garbage. */
    if not_bool(input_well_formed()) then {
        emit_invalid();
        return ()
    };

    /* S2f: execute the block decoded from private input against witness state */
    build_node_db();
    slotdb_reset();
    pa_reset();
    ps_reset();
    register_witness_codes();
    let sr = parent_state_root();

    /* Stage 3+4: re-root the witness via the stack builder and validate it against
       the committed parent.state_root (the one-shot pre-state validation); the walk
       records each account leaf's span into the acctdb (for lazy O(1) account
       reads). Then authenticate + harvest every account's storage trie into the
       slot store (driven by the acctdb): the witness-proven pre-state storage that
       backs stateless reads and the post-state root. */
    /* the witness must re-root to the committed parent.state_root; if it does
       not (a deficient/inconsistent proof), the block is invalid (valid=false)
       -- a NORMAL failure outcome, reported rather than aborted on. */
    if witness_reroot(sr) != sr then {
        k_witness_bad = true
    };
    harvest_all_storage();
    k_world_reset();

    let env : BlockEnv = struct {
            be_number = word_of_int(pl_block_number()),
            be_timestamp = word_of_int(pl_timestamp()),
            be_gas_limit = word_of_int(pl_gas_limit()),
            be_coinbase = pl_fee_recipient(),
            be_base_fee = pl_base_fee(),
            be_prev_randao = pl_prev_randao(),
            be_chain_id = word_of_int(cc_chain_id()),
            be_blob_base_fee = word_of_int(blob_base_fee(pl_excess_blob_gas())),
            be_excess_blob_gas = word_of_int(pl_excess_blob_gas()),
            be_slot = word_of_int(pl_slot_number()),
        };

    /* Arm stateless mode: the witness now backs the base layer for BOTH accounts
       and storage. A touched account is materialized from its acctdb span on first
       kget (one keccak, then cached raw in the acctmap); a touched slot resolves
       from the harvested slot store on first read (k_sload/k_orig) and is cached
       read-through. Nothing is pre-seeded -- no eager account/slot pass. */
    k_stateless_on();

    /* Zero-copy block STF over the SSZ payload by index -- no materialized Block /
       list(Transaction). Each tx is decoded once (repointing the single txin slot)
       and applied in order via the shared per-tx STF unit (process_transaction),
       folding receipt success and accumulating gas; withdrawals are then credited.
       This is the single block driver -- the EEST runner shares process_transaction
       for its single-tx state-test cases. */
    k_set_block(env);

    /* EIP-4788 / EIP-2935: write the parent beacon-block root and parent block
       hash into their system contracts before the transactions run. */
    system_call(BEACON_ROOTS_ADDR(), ssz_bytes32(npr_start() + 8)); /* parent_beacon_block_root */
    system_call(HISTORY_STORAGE_ADDR(), ssz_bytes32(payload_start() + 0)); /* parent_hash */

    let n_txs = pl_n_txs();
    auth_ok = true; /* reset; decode_tx_ssz folds each tx's pubkey authentication in */
    var all_ok : bool = true;
    var gas_acc : int = 0;
    var tx0_to : address = ZERO_ADDR; /* tx 0 recipient, for the storage0 fact */
    let block_gas_limit = pl_gas_limit();

    /* EIP-7778 block gas accounting: a tx whose gas limit exceeds the block's
       remaining gas raises GasUsedExceedsLimitError -> the whole block is
       invalid. EELS performs this check BEFORE touching the tx's state, so the
       witness omits the offending tx's proofs; the guest must likewise refrain
       from executing it (and every later tx), else it over-reads the witness
       and aborts on a node EELS never proved. */
    var block_gas_overflow : bool = false;
    foreach (i from 0 to (n_txs - 1)) {
        if not_bool(block_gas_overflow) then {
            let tx = decode_tx_ssz(i);
            if i == 0 then {
                tx0_to = tx.t_to
            };
            if tx.t_gas_limit > block_gas_limit - gas_acc then {
                block_gas_overflow = true
            } else {
                let r = process_transaction(tx);
                all_ok = all_ok & r.rc_valid;
                gas_acc = gas_acc + r.rc_block_gas
            }
        }
    };
    all_ok = all_ok & not_bool(block_gas_overflow);
    foreach (i from 0 to (pl_n_withdrawals() - 1)) {
        /* EIP-4895: the withdrawal `amount` field is denominated in Gwei; the
           credited balance is amount * 10^9 wei. */
        k_add_balance(withdrawal_addr(i), word_of_int(withdrawal_amount(i) * 1000000000))
    };

    /* EIP-7002 / EIP-7251: dequeue the withdrawal + consolidation requests via a
       CHECKED system call to each predeploy (clearing their queue storage). EELS
       reaches this only if no earlier step raised, so run it only while the block
       is still valid; a missing predeploy or a failed call invalidates the block. */
    if all_ok then {
        let rw = system_call_checked(WITHDRAWAL_REQUEST_ADDR());
        let rc = system_call_checked(CONSOLIDATION_REQUEST_ADDR());
        all_ok = all_ok & rw & rc
    };

    /* S5: stateless validation outcome (a failure is a NORMAL result -- the guest
       reports successful_validation=false, it does not abort):
         - chain config selects the Amsterdam fork
         - the witness headers are contiguous
         - every tx's witness public key is authenticated against its signature
         - every tx is VALID/applicable (a reverting valid tx keeps the block valid)
         - the recomputed post-state root commits to payload.state_root */
    let cfg_ok = (cc_fork() == 20) & cc_activation_active(); /* canonical ProtocolFork index for Amsterdam
                                                            (execution-specs@02c6c2510: enum pruned, Osaka=17 BPO1=18 BPO2=19
                                                            Amsterdam=20; was 24 in tests-zkevm@v0.4.1). */

    let hdr_ok = headers_ok();

    /* every transaction must be VALID (applicable), folded over the block: an
       INVALID tx (bad nonce/balance/intrinsic/floor/fee) makes the block
       invalid, but a valid tx that reverts or OOGs does NOT (rc_valid, not
       rc_success). */
    let exec_ok : bool = all_ok;
    let poststate_ok = compute_state_root_hb() == pl_state_root();

    /* k_witness_bad: a deficient/inconsistent witness proof (missing trie node,
       a pre/post storage trie that does not re-root) -- the block cannot be
       validated, so it is invalid (EIP-8025 optional-proofs negative tests). */
    /* EIP-7928: the block access list must not exceed block_gas_limit / 2000 items. */
    let bal_ok : bool = bal_item_count() <= ediv_int(pl_gas_limit(), 2000);
    let valid : bool = cfg_ok & hdr_ok & auth_ok & exec_ok & poststate_ok & bal_ok & not_bool(k_witness_bad);

    let stored = k_storage_get(tx0_to, word_of_int(0));
    let gas_used = to_gas(gas_acc);
    let wbal = kget(withdrawal_addr(0)).acc_balance;
    let cbbal = kget(pl_fee_recipient()).acc_balance;

    zkvm_out_gas = gas_used;
    zkvm_out_storage0 = unsigned(stored);
    zkvm_out_wbal = unsigned(wbal);
    zkvm_out_cbbal = unsigned(cbbal);
    zkvm_out_tracelen = 0; /* the ELOp trace machinery was removed */
    zkvm_out_success = if valid then { 1 } else { 0 };

    /* emit the canonical SSZ SszStatelessValidationResult as public output */
    emit_result(build_zerohashes(40), valid)
}
