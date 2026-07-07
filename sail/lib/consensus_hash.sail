/* ======================================================================== */
/* SHARED CONSENSUS HASHING                                                 */
/*                                                                          */
/* Consensus-critical derivations written once, against the abstract hash   */
/* primitives keccak256 / sha256 / keccak_rlp_payload (iface/crypto.sail).  */
/* Both backends execute exactly this code; only the primitives differ      */
/* (spec: axioms over materialized lists; build: streamed C host-bytes).    */
/* Nothing here may call an FFI hook directly.                              */
/* ======================================================================== */

/* keccak256 of an RLP list wrapping `payload` (already-encoded items). */
function keccak_rlp_list(payload : list(byte)) -> bits(256) =
    keccak_rlp_payload([||], payload)

/* SHA-256 self-check against the standard "" and "abc" reference vectors. */
function sha256_selfcheck() -> unit = {
    assert(sha256([||]) == 0xe3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855, "sha256('') on guest");
    assert(
        sha256([|0x61, 0x62, 0x63|]) == 0xba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad,
        "sha256('abc') on guest"
    )
}

/* Keccak self-check against Ethereum's "" and "abc" vectors. */
function keccak_selfcheck() -> unit = {
    assert(keccak256([||]) == 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470, "keccak('') on guest");
    assert(
        keccak256([|0x61, 0x62, 0x63|]) == 0x4e03657aea45a94fc7d47ba826c8d667c0d1e6e33a64a036ec44f58fa12d6c45,
        "keccak('abc') on guest"
    )
}

/* MPT child reference: nodes under 32 bytes embed inline, larger nodes are
 * referenced by their keccak hash (Yellow Paper appendix D). */
function child_ref(enc : list(byte)) -> list(byte) =
    if list_len(enc) < 32 then {
        enc
    } else {
        rlp_bytes(word_to_bytes32(keccak256(enc)))
    }

function trie_ref_to_root(r : list(byte)) -> bits(256) =
    match r {
        0xa0 :: h => if list_len(h) == 32 then {
            bytes_be256(h)
        } else {
            keccak256(r)
        },
        _ => keccak256(r),
    }

/* Hash full code bytes, store them in the code database, and return codeHash. */
function code_hash_and_store(c : list(byte)) -> hash = {
    let h : hash = keccak256(c);
    code_db_store_code(h, c);
    h
}

function code_db_store_witness_code(s : witness_slice) -> unit =
    code_db_store_slice(witness_byte_slice(s))

/* The tx signing preimage hash: typed txs prepend the type byte as a domain
 * separator; legacy EIP-155 txs append (chain_id, 0, 0) to the content. */
function tx_signing_hash(ttype : int, fields : list(RlpField), i_v : int, v : int) -> bits(256) = {
    var content : list(byte) = rlp_fields_encoded_prefix(fields, i_v);
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
function auth_signing_hash(fields : list(RlpField)) -> bits(256) = {
    let content : list(byte) = rlp_fields_encoded_prefix(fields, 3);
    keccak_rlp_payload([|0x05|], content)
}

/* EIP-7685 commitment: sha256 over the concatenation of each present request
 * type's 32-byte per-type digest (deposit, withdrawal, consolidation, in that
 * order; an empty request body contributes nothing). Each digest hashes a
 * witness span (source-based); the final concat-and-hash is a refinement pair
 * (sha256_digests3) so the build backend never streams the <=96-byte payload
 * list -- the present-digest mask and the three digests cross as fixed words. */
function execution_requests_hash() -> hash = {
    let l0 = wreq_start() - dep_start();
    let l1 = creq_start() - wreq_start();
    let l2 = req_end() - creq_start();
    let d0 : hash = if l0 != 0 then sha256_request_digest(0, witness_slice(dep_start(), l0)) else sail_zeros(256);
    let d1 : hash = if l1 != 0 then sha256_request_digest(1, witness_slice(wreq_start(), l1)) else sail_zeros(256);
    let d2 : hash = if l2 != 0 then sha256_request_digest(2, witness_slice(creq_start(), l2)) else sail_zeros(256);
    let m = (if l0 != 0 then 1 else 0) + (if l1 != 0 then 2 else 0) + (if l2 != 0 then 4 else 0);
    sha256_digests3(get_slice_int(8, m, 0), d0, d1, d2)
}
