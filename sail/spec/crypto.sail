/* ======================================================================== */
/* SPEC BACKEND: CRYPTO                                                     */
/*                                                                          */
/* Canonical definitions for hashing and the signing precompiles. They      */
/* materialize mathematical byte lists (read_byte_slice in                  */
/* model_buffers.sail) before hashing. Build roots replace the same         */
/* abstract functions with pointer-backed implementations in sail/build/.   */
/* keccak256 / sha256 are declared in iface/crypto.sail and stay abstract   */
/* here (the axiomatic crypto core); run_precompile is spec/precompile.     */
/* Shared consumers live in lib/consensus_hash.sail; only the true          */
/* refinement pairs are here.                                               */
/* ======================================================================== */

/* The canonical meaning of the streamed RLP-payload hash. */
function keccak_rlp_payload(domain : list(byte), payload : list(byte)) -> bits(256) = {
    var pre : list(byte) = domain;
    pre = list_append(pre, rlp_list_prefix(list_len(payload)));
    pre = list_append(pre, payload);
    keccak256(pre)
}

function bytes_keccak(s : ByteSlice) -> bits(256) =
    keccak256(read_byte_slice(s))

function bytes_sha256_prefixed(prefix : byte, s : ByteSlice) -> bits(256) =
    sha256(prefix :: read_byte_slice(s))

function keccak256_word(w : word) -> bits(256) =
    keccak256(word_to_bytes32(w))

function keccak256_address(a : address) -> bits(256) =
    keccak256(address_to_bytes(a))

function sha256_pair(a : bits(256), b : bits(256)) -> bits(256) =
    sha256(list_append(word_to_bytes32(a), word_to_bytes32(b)))

function create2_address(sender : address, salt : word, init_hash : hash) -> address = {
    var preimage : list(byte) = [|0xff|];
    preimage = list_append(preimage, address_to_bytes(sender));
    preimage = list_append(preimage, word_to_bytes32(salt));
    preimage = list_append(preimage, word_to_bytes32(init_hash));
    word_to_address(keccak256(preimage))
}

function secp256k1_verify_precompile(h : bits(256), r : word, s : word, x : bits(256), y : bits(256)) -> bool = {
    var inp : list(byte) = word_to_bytes32(h);
    inp = list_append(inp, word_to_bytes32(r));
    inp = list_append(inp, word_to_bytes32(s));
    inp = list_append(inp, word_to_bytes32(x));
    inp = list_append(inp, word_to_bytes32(y));
    let (_, out) = run_precompile(257, inp);
    list_len(out) == 32
}

function ecrecover_precompile(h : bits(256), yparity : int, r : word, s : word) -> (bool, address) = {
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
