
/*md
# Transaction RLP signing preimages

The signature semantics of transactions, between the crypto core
(secp recovery and the curve constants) and the envelope decoder:
the signing preimage hashes, the per-envelope `v`-range rule, and sender
authentication — binding the witnessed public key to a transaction's
signature, called from transaction validity.
*/
/*! The chain id encoded in a legacy signature with `v >= 35`
    (EIP-155). */
function legacy_sig_chain_id(v : word) -> word = {
    let adjusted_v = word_sub(v, 35);
    word_div(adjusted_v, 2)
}

/* Tx signing preimage hash. `content_src` spans the RLP of the pre-signature
 * fields in the witness and is copied once into the final contiguous
 * preimage; legacy EIP-155 txs append (chain_id, 0, 0), typed txs prepend the
 * type byte as a domain separator. */
let LEGACY_SIGNATURE_SUFFIX_LENGTH : int(2) = 2
let PUBLIC_KEY_BODY_LENGTH : int(64) = DOUBLE_WORD_BYTE_LENGTH

/*! The transaction signing-preimage hash. `content_src` spans the RLP of
    the pre-signature fields in the witness and is copied once into the final
    contiguous preimage; legacy EIP-155 transactions append
    `(chain_id, 0, 0)`, typed transactions prepend the type byte as a
    domain separator (EIP-2718). */
function tx_signing_hash(t : TxType, content_src : StatelessInputSlice, v : word) -> hash = {
    let tb : byte = tx_envelope_type(t);
    let eip155 = (tb == 0x00) & word_ule(35, v);
    let chain_id =
        if eip155 then legacy_sig_chain_id(v) else ZERO_WORD;
    let suffix_len : range(0, 35) =
        if eip155 then {
            let chain_id_length = rlp_uint_word_size(chain_id);
            let suffix_length = LEGACY_SIGNATURE_SUFFIX_LENGTH;
            chain_id_length + suffix_length
        } else {
            0
        };
    let content_length : transaction_byte_length =
        if content_src.len <= sizeof(transaction_length_bound) then content_src.len else fatal_error(RlpDecode);
    let suffix_length = suffix_len;
    let content_len : range(0, transaction_length_bound + 35) = content_length + suffix_length;
    let prefix_len = rlp_length_prefix_len(content_len);
    let type_len : range(0, 1) =
        if tb == 0x00 then 0 else 1;
    let preimage_len : range(0, transaction_length_bound + 69) = type_len + prefix_len + content_len;
    let encoder = rlp_encoder_begin(preimage_len);
    if tb != 0x00 then {
        scratch_push_byte(tb)
    };
    rlp_write_list_prefix(content_len);
    scratch_push_slice(content_src);
    if eip155 then {
        rlp_write_uint_word(chain_id);
        scratch_push_byte(0x80);
        scratch_push_byte(0x80)
    };

    let preimage = rlp_encoder_finish(encoder);
    let signing_hash = keccak256(preimage);
    rlp_encoder_rewind(encoder);
    signing_hash
}

/*! The EIP-7702 authorization signing hash:
    `keccak256(0x05 || rlp([chain_id, address, nonce]))`. */
function auth_signing_hash(chain_id : word, addr : address, nonce : account_nonce) -> hash = {
    let chain_id_length = rlp_uint_word_size(chain_id);
    let address_length = rlp_addr_size();
    let nonce_length = rlp_uint_size(nonce);
    let content_len : range(0, 87) = chain_id_length + address_length + nonce_length;
    let preimage_len : range(0, 121) = 1 + rlp_list_size(content_len);
    let encoder = rlp_encoder_begin(preimage_len);
    scratch_push_byte(0x05);
    rlp_write_list_prefix(content_len);
    rlp_write_uint_word(chain_id);
    rlp_write_addr(addr);
    rlp_write_uint(nonce);
    let encoded = rlp_encoder_finish(encoder);
    let signing_hash = keccak256(encoded);
    rlp_encoder_rewind(encoder);
    signing_hash
}
