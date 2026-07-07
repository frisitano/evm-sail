/* ======================================================================== */
/* BUILD BACKEND: CRYPTO CORE (source-free)                                 */
/*                                                                          */
/* Executable definitions for hashing and the signing precompiles that      */
/* carry NO byte source: the C host-bytes hash channel, the fixed-width     */
/* hash helpers, and the signature precompiles. Shared by EVM_BACKEND=build */
/* and EVM_BACKEND=hybrid -- both hash via the C channel; only the          */
/* source-tagged bytes_keccak / bytes_sha256_prefixed differ, so those live */
/* in build/crypto_source.sail (build) and hybrid/crypto.sail (hybrid). The */
/* spec backend's canonical byte-list definitions live in spec/crypto.sail; */
/* shared consumers in lib/consensus_hash.sail call these primitives.       */
/* ======================================================================== */

val host_keccak_word_ffi = impure { _: "host_keccak_word" } : word -> hash
val host_keccak_address_ffi = impure { _: "host_keccak_address" } : address -> hash
val host_keccak_create2_ffi = impure { _: "host_keccak_create2" } : (address, word, hash) -> hash
val host_create_address_ffi = impure { _: "host_create_address" } : (address, bits(64)) -> hash
val host_sha256_pair_ffi = impure { _: "host_sha256_pair" } : (hash, hash) -> hash

/* Block-header RLP is framed and hashed C-side. Args are in RLP field order;
 * logs_bloom (256B) and extraData are passed as witness offsets/length, the
 * remaining fields as fixed words / u64 scalars. */
val host_block_header_hash_ffi = impure { _: "host_block_header_hash" } : (
        hash,     /* parent_hash */
        address,  /* fee_recipient */
        hash,     /* state_root */
        hash,     /* transactions_root */
        hash,     /* receipts_root */
        bits(64), /* logs_bloom witness offset (length 256) */
        bits(64), /* number */
        bits(64), /* gas_limit */
        bits(64), /* gas_used */
        bits(64), /* timestamp */
        bits(64), /* extraData witness offset */
        bits(64), /* extraData length */
        hash,     /* prev_randao */
        word,     /* base_fee */
        hash,     /* withdrawals_root */
        bits(64), /* blob_gas_used */
        bits(64), /* excess_blob_gas */
        hash,     /* parent_beacon_block_root */
        hash,     /* requests_hash */
        hash,     /* block_access_list_hash */
        bits(64), /* slot_number */
    ) -> hash

/* Tx signing hash: frame the pre-signature RLP content (a witness span) with
 * the type-byte domain / list prefix / EIP-155 suffix and hash it C-side.
 * apply_155 selects the (chain_id, 0, 0) legacy suffix. */
val host_tx_signing_hash_ffi = impure { _: "host_tx_signing_hash" } : (
        bits(8),  /* tx type (0 = legacy, else typed domain byte) */
        bits(8),  /* apply EIP-155 (chain_id, 0, 0) suffix */
        word,     /* EIP-155 chain id */
        bits(64), /* content byte-source kind */
        bits(64), /* content offset */
        bits(64), /* content length */
    ) -> hash

/* EIP-7702 auth signing hash: frame 0x05 || rlp([chain_id, address, nonce]). */
val host_auth_signing_hash_ffi = impure { _: "host_auth_signing_hash" } : (word, address, bits(64)) -> hash
val host_sha256_digests3_ffi = impure { _: "host_sha256_digests3" } : (bits(8), hash, hash, hash) -> hash

val host_bytes_reset_ffi = impure { _: "host_bytes_reset" } : unit -> unit
val host_bytes_push_ffi = impure { _: "host_bytes_push" } : bits(8) -> unit
val host_bytes_push8_ffi = impure { _: "host_bytes_push8" } : bits(64) -> unit
val host_bytes_keccak_finish_ffi = impure { _: "host_bytes_keccak_finish" } : unit -> bits(256)
val host_bytes_sha256_finish_ffi = impure { _: "host_bytes_sha256_finish" } : unit -> bits(256)

val precompile_secp256k1_verify_hash_sig_pub_ffi = impure { _: "precompile_secp256k1_verify_hash_sig_pub" } : (
        hash,
        word,
        word,
        word,
        word,
    ) -> bool
val precompile_ecrecover_hash_sig_ffi = impure { _: "precompile_ecrecover_hash_sig" } : (hash, bits(64), word, word) -> bits(
        168,
    )

function build_pack8(
    b0 : byte,
    b1 : byte,
    b2 : byte,
    b3 : byte,
    b4 : byte,
    b5 : byte,
    b6 : byte,
    b7 : byte,
) -> (
    bits(64)
) =
    append(b0, append(b1, append(b2, append(b3, append(b4, append(b5, append(b6, b7)))))))

function build_hash_push_bytes(bs : list(byte)) -> unit = {
    var rest : list(byte) = bs;
    var more : bool = true;
    while more do {
        match rest {
            b0 :: (b1 :: (b2 :: (b3 :: (b4 :: (b5 :: (b6 :: (b7 :: t))))))) => {
                host_bytes_push8_ffi(build_pack8(b0, b1, b2, b3, b4, b5, b6, b7));
                rest = t
            },
            b :: t => {
                host_bytes_push_ffi(b);
                rest = t
            },
            [||] => more = false,
        }
    }
}

/* The list-typed hash primitives, streamed through the C byte channel. */
function keccak256(bs : list(byte)) -> bits(256) = {
    host_bytes_reset_ffi();
    build_hash_push_bytes(bs);
    host_bytes_keccak_finish_ffi()
}

function sha256(bs : list(byte)) -> bits(256) = {
    host_bytes_reset_ffi();
    build_hash_push_bytes(bs);
    host_bytes_sha256_finish_ffi()
}

/* Refines keccak256(domain ++ rlp_list_prefix(len(payload)) ++ payload)
 * by streaming the three parts without concatenating them. */
function keccak_rlp_payload(domain : list(byte), payload : list(byte)) -> bits(256) = {
    host_bytes_reset_ffi();
    build_hash_push_bytes(domain);
    build_hash_push_bytes(rlp_list_prefix(list_len(payload)));
    build_hash_push_bytes(payload);
    host_bytes_keccak_finish_ffi()
}

function keccak256_word(w : word) -> bits(256) = host_keccak_word_ffi(w)

function keccak256_address(a : address) -> bits(256) = host_keccak_address_ffi(a)

function sha256_pair(a : bits(256), b : bits(256)) -> bits(256) = host_sha256_pair_ffi(a, b)

function sha256_digests3(mask : bits(8), d0 : bits(256), d1 : bits(256), d2 : bits(256)) -> bits(256) =
    host_sha256_digests3_ffi(mask, d0, d1, d2)

function create2_address(sender : address, salt : word, init_hash : hash) -> address =
    word_to_address(host_keccak_create2_ffi(sender, salt, init_hash))

/* CREATE address: the RLP two-item list [sender, nonce] is framed and hashed in
 * C (nonce is a u64 account nonce). Refines keccak256(rlp([sender, nonce])). */
function create_address(sender : address, nonce : int) -> address =
    word_to_address(host_create_address_ffi(sender, get_slice_int(64, nonce, 0)))

/* Block hash: build re-reads logs_bloom / extraData from the witness (they are
 * decoded from there) and passes the remaining header fields as scalars, so the
 * whole RLP header is framed and hashed C-side with no Sail list. Refines the
 * spec/crypto.sail block_header_hash body. */
function block_header_hash(
    header : BlockHeader,
    transactions_root : hash,
    withdrawals_root : hash,
    requests_hash : hash,
    block_access_list_hash : hash,
) -> hash = {
    let extra_off = pl_extra_start();
    let extra_len = pl_txs_start() - extra_off;
    host_block_header_hash_ffi(
        header.parent_hash,
        header.fee_recipient,
        header.state_root,
        transactions_root,
        header.receipts_root,
        get_slice_int(64, payload_start() + PL_LOGS_BLOOM, 0),
        get_slice_int(64, header.number, 0),
        get_slice_int(64, header.gas_limit, 0),
        get_slice_int(64, header.gas_used, 0),
        get_slice_int(64, header.timestamp, 0),
        get_slice_int(64, extra_off, 0),
        get_slice_int(64, extra_len, 0),
        header.prev_randao,
        header.base_fee,
        withdrawals_root,
        get_slice_int(64, header.blob_gas_used, 0),
        get_slice_int(64, header.excess_blob_gas, 0),
        header.parent_beacon_block_root,
        requests_hash,
        block_access_list_hash,
        get_slice_int(64, header.slot_number, 0),
    )
}

function tx_signing_hash(ttype : int, content_src : ByteSlice, v : int) -> hash = {
    let apply_155 : bool = (ttype == 0) & (v >= 35);
    let chain_id : word = if apply_155 then get_slice_int(256, legacy_sig_chain_id(v), 0) else sail_zeros(256);
    host_tx_signing_hash_ffi(
        get_slice_int(8, ttype, 0),
        if apply_155 then 0x01 else 0x00,
        chain_id,
        byte_source_kind(content_src.source),
        content_src.off,
        content_src.len,
    )
}

function auth_signing_hash(chain_id : int, addr : address, nonce : int) -> hash =
    host_auth_signing_hash_ffi(get_slice_int(256, chain_id, 0), addr, get_slice_int(64, nonce, 0))

function secp256k1_verify_precompile(h : bits(256), r : word, s : word, x : bits(256), y : bits(256)) -> bool =
    precompile_secp256k1_verify_hash_sig_pub_ffi(h, r, s, x, y)

function ecrecover_precompile(h : bits(256), yparity : int, r : word, s : word) -> (bool, address) = {
    let recovered = precompile_ecrecover_hash_sig_ffi(h, get_slice_int(64, yparity, 0), r, s);
    (recovered[160] == bitone, recovered[159 .. 0])
}
