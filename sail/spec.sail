/* ======================================================================== */
/* SPEC BACKEND: pure reference bodies for the host interface               */
/*                                                                          */
/* The EVM_BACKEND=spec definitions of the abstract vals declared in        */
/* host/io.sail (crypto, oracle, precompiles) and lib/mpt.sail (trie        */
/* node child refs): the canonical, list-typed reference models -- what     */
/* the Coq export (proof/) turns into definitions and what build.sail       */
/* refines with pointer-backed C. keccak256 / sha256 themselves stay        */
/* abstract here (the axiomatic crypto core); every other body is a         */
/* materialized-byte-list derivation over them. Selected when               */
/* EVM_BACKEND=spec; the executable counterpart is build.sail.              */
/* ======================================================================== */

/* ---- private input oracle -- bulk readers as pure derivations of ssz_src_byte,
 * so LE / BE reads are consistent views over one bounded input byte vector. ---- */

function ssz_src_le(b : int, n : int) -> bits(64) = {
    var acc : bits(64) = 0x0000000000000000;
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(acc, sail_shiftleft(sail_zero_extend(ssz_src_byte(b + k), 64), 8 * k))
    };
    acc
}

function ssz_src_be(b : int, n : int) -> bits(64) = {
    var acc : bits(64) = 0x0000000000000000;
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(ssz_src_byte(b + k), 64))
    };
    acc
}

/* ---- precompiles -- the abstract executor over a materialized input list. The
 * run_precompile_* wrappers (host/io.sail) stay abstract in spec; the meaning
 * is run_precompile(id, read_byte_slice(source)). ---- */

val run_precompile : (int, list(byte)) -> (bool, list(byte))

/* ---- crypto -- canonical hashing + signing precompiles over materialized byte
 * lists (read_byte_slice, host/memory.sail). Shared consumers live in
 * lib/consensus_hash.sail; only the true refinement pairs are here. ---- */

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

/* EIP-7702 authorization signing hash: 0x05 || rlp([chain_id, address, nonce]).
 * The tuple's three signed fields are canonical for any valid authorization, so
 * both backends re-encode them from the decoded values; build frames + hashes
 * C-side (build.sail). */
function auth_signing_hash(chain_id : int, addr : address, nonce : int) -> hash = {
    var content : list(byte) = rlp_int(chain_id);
    content = list_append(content, rlp_addr(addr));
    content = list_append(content, rlp_int(nonce));
    keccak_rlp_payload([|0x05|], content)
}

/* Tx signing preimage hash. `content_src` spans the RLP of the pre-signature
 * fields in the witness; legacy EIP-155 txs append (chain_id, 0, 0), typed txs
 * prepend the type byte as a domain separator. Build frames + hashes this
 * C-side (build.sail). */
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

function sha256_digests3(mask : bits(8), d0 : bits(256), d1 : bits(256), d2 : bits(256)) -> bits(256) = {
    var payload : list(byte) = [||];
    if mask[0] == bitone then {
        payload = list_append(payload, word_to_bytes32(d0))
    };
    if mask[1] == bitone then {
        payload = list_append(payload, word_to_bytes32(d1))
    };
    if mask[2] == bitone then {
        payload = list_append(payload, word_to_bytes32(d2))
    };
    sha256(payload)
}

function create2_address(sender : address, salt : word, init_hash : hash) -> address = {
    var preimage : list(byte) = [|0xff|];
    preimage = list_append(preimage, address_to_bytes(sender));
    preimage = list_append(preimage, word_to_bytes32(salt));
    preimage = list_append(preimage, word_to_bytes32(init_hash));
    word_to_address(keccak256(preimage))
}

/* CREATE address: low 20 bytes of keccak(rlp([sender, nonce])). Build encodes
 * and hashes the two-item list in C (build.sail); the two backends must
 * agree on the RLP list framing. */
function create_address(sender : address, nonce : int) -> address = {
    var payload : list(byte) = rlp_addr(sender);
    payload = list_append(payload, rlp_int(nonce));
    word_to_address(keccak_rlp_payload([||], payload))
}

/* Block hash: keccak(rlp([...header fields...])). Build re-encodes the header
 * in C from the scalar fields plus the logs_bloom / extraData witness spans
 * (build.sail); this materialized RLP body is the canonical field
 * layout both backends must agree on. */
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
    payload = list_append(payload, rlp_int(k_header.blob_gas_used));
    payload = list_append(payload, rlp_int(k_header.excess_blob_gas));
    payload = list_append(payload, rlp_word(k_header.parent_beacon_block_root));
    payload = list_append(payload, rlp_word(requests_hash));
    payload = list_append(payload, rlp_word(block_access_list_hash));
    payload = list_append(payload, rlp_int(k_header.slot_number));
    keccak_rlp_payload([||], payload)
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

/* ---- MPT node encode + hash as canonical byte lists (lib/mpt.sail helpers); the
 * arena is the abstract write side of the byte-source interface here. ---- */

function branch_child_ref(mask : bits(16), children : list(list(byte))) -> list(byte) =
    child_ref(branch_node_rlp(mask, children))

function extension_child_ref(key : list(nibble), childref : list(byte)) -> list(byte) =
    child_ref(extension_node_rlp(key, childref))

/* leaf child ref: compose rlp([hp(key,leaf), value]) and hash it. The value is
 * materialized from its byte source via the read_byte_slice axiom -- for update
 * leaves that source is the value arena, so read_byte_slice(arena_intern(l)) = l
 * (the arena read/write law) recovers the canonical account/storage RLP. */
function leaf_child_ref(key : list(nibble), value : ByteSlice) -> list(byte) =
    child_ref(leaf_node_rlp(key, read_byte_slice(value)))

/* update-leaf values: the canonical RLP interned into the value arena. account:
 * rlp([nonce, balance, storageRoot, codeHash]); storage: rlp_int(value). */
function account_leaf_value(nonce : int, balance : word, sroot : hash, chash : hash) -> ByteSlice =
    arena_intern(account_rlp(nonce, balance, sroot, chash))

function storage_leaf_value(value : word) -> ByteSlice =
    arena_intern(rlp_int(unsigned(value)))

/* the arena is a build-side C buffer; in spec it is the abstract write side of
 * the byte-source interface (arena_intern, an axiom) and needs no reset. */
function trie_arena_reset() -> unit = ()
