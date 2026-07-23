
/*md
# Transaction signature rules

The signature semantics of transactions, between the crypto core
(secp recovery and the curve constants) and the envelope decoder:
the signing preimage hashes, the per-envelope `v`-range rule, and sender
authentication — binding the witnessed public key to a transaction's
signature, called from transaction validity.
*/
/*! The chain id encoded in a legacy signature with `v >= 35`
    (EIP-155). */
function legacy_sig_chain_id(v : word) -> word =
    word_div(word_sub(v, 35), 2)

/* Tx signing preimage hash. `content_src` spans the RLP of the pre-signature
 * fields in the witness and is hashed as a SEGMENT (never materialized);
 * legacy EIP-155 txs append (chain_id, 0, 0), typed txs prepend the type
 * byte as a domain separator. */
let LEGACY_SIGNATURE_SUFFIX_LENGTH : int(2) = 2
let PUBLIC_KEY_BODY_LENGTH : int(64) = DOUBLE_WORD_BYTE_LENGTH

/*! The transaction signing-preimage hash. `content_src` spans the RLP of
    the pre-signature fields in the witness and is hashed as a segment
    (never materialized); legacy EIP-155 transactions append
    `(chain_id, 0, 0)`, typed transactions prepend the type byte as a
    domain separator (EIP-2718). */
function tx_signing_hash(t : TxType, content_src : ByteSlice, v : word) -> hash = {
    let tb = tx_type_byte(t);
    let eip155 = (tb == 0x00) & word_ule(35, v);
    let chain_id = if eip155 then {
            legacy_sig_chain_id(v)
        } else {
            ZERO_WORD
        };
    let suffix_len : range(0, 35) = if eip155 then {
            let chain_id_length = rlp_uint_word_size(chain_id);
            let suffix_length = LEGACY_SIGNATURE_SUFFIX_LENGTH;
            chain_id_length + suffix_length
        } else {
            0
        };
    let content_length : transaction_byte_length = if content_src.len <= sizeof(transaction_length_bound) then {
            content_src.len
        } else {
            throw InvalidBlock(RlpDecode)
        };
    let suffix_length = suffix_len;
    let content_len : range(0, transaction_length_bound + 35) = content_length + suffix_length;
    let (prefix, prefix_len) = rlp_list_prefix(content_len);
    if eip155 then {
        let mark = scratch_begin();
        rlp_write_uint_word(chain_id);
        scratch_push_bytes([|0x80, 0x80|], LEGACY_SIGNATURE_SUFFIX_LENGTH);
        let suffix = rlp_finish(mark);
        let signing_hash = keccak256([|bytes_list(prefix, prefix_len), BytesSlice(content_src), BytesSlice(suffix)|]);
        scratch_rewind(mark);
        signing_hash
    } else {
        if tb != 0x00 then {
            keccak256([|bytes_list([|tb|], 1), bytes_list(prefix, prefix_len), BytesSlice(content_src)|])
        } else {
            keccak256([|bytes_list(prefix, prefix_len), BytesSlice(content_src)|])
        }
    }
}

/*! The EIP-7702 authorization signing hash:
    `keccak256(0x05 || rlp([chain_id, address, nonce]))`. */
function auth_signing_hash(chain_id : word, addr : address, nonce : account_nonce) -> hash = {
    let chain_id_length = rlp_uint_word_size(chain_id);
    let address_length = rlp_addr_size();
    let nonce_length = rlp_uint_size(nonce);
    let content_len : range(0, 87) = chain_id_length + address_length + nonce_length;
    let mark = scratch_begin();
    scratch_push_bytes([|0x05|], 1);
    rlp_write_list_prefix(content_len);
    rlp_write_uint_word(chain_id);
    rlp_write_addr(addr);
    rlp_write_uint(nonce);
    let encoded = rlp_finish(mark);
    let signing_hash = keccak256_slice(encoded);
    scratch_rewind(mark);
    signing_hash
}

/*md
## The sender authentication

`public_keys[i]` is a witness, not trusted input. The sender address is
derived from the key, then authenticated by recovering the signature's
address with its envelope `y_parity` and requiring the two addresses to
match. A forged key, wrong parity, or bad signature therefore rejects the
block.
*/
/*! The per-envelope `v`-range rule: legacy accepts `27`/`28` or the
    EIP-155 form binding the chain id; typed envelopes accept
    `y_parity ∈ {0, 1}`. */
function tx_sig_v_valid(chain_id : chain_identifier, t : TxType, v : word) -> bool = match t {
    LegacyTx =>   (v == 27)
                | (v == 28)
                | (word_ule(35, v) & (legacy_sig_chain_id(v) == word_of_chain_identifier(chain_id))),
    _ => (v == WORD_ZERO) | (v == WORD_ONE),
}

/*! Extracts the recovery parity after [tx_sig_v_valid][] has validated the
    envelope-specific `v` domain. Legacy `27` and every EIP-155 parity-zero
    value are odd; typed envelopes carry the parity directly. */
function tx_y_parity(t : TxType, v : word) -> y_parity = match t {
    LegacyTx => if word_and(v, WORD_ONE) == WORD_ONE then { 0 } else { 1 },
    _ => if v == WORD_ZERO then { 0 } else { 1 },
}

/*! Authenticates a transaction: enforce the EIP-2 low-`s` bound, recover the
    signer selected by `y_parity`, and bind it to the address derived from the
    witnessed 65-byte public key. */
function tx_auth_valid(sender : address, h : hash, parity : y_parity, r : word, s : word) -> bool =
    if word_ult(SECP_N_HALF, s) then {
        false
    } else {
        let (recovered, recovered_sender) = ecrecover_addr(h, parity, r, s);
        recovered & (recovered_sender == sender)
    }
