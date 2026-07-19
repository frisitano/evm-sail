
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
    word_divmod(word_sub(v, limb_to_word(0x0000000000000023)), limb_to_word(0x0000000000000002)).quotient

/* Tx signing preimage hash. `content_src` spans the RLP of the pre-signature
 * fields in the witness and is hashed as a SEGMENT (never materialized);
 * legacy EIP-155 txs append (chain_id, 0, 0), typed txs prepend the type
 * byte as a domain separator. */
let LEGACY_SIGNATURE_SUFFIX_LENGTH : byte_length = ByteQuantity(2)
let PUBLIC_KEY_BODY_LENGTH : byte_length = DOUBLE_WORD_BYTE_LENGTH

/*! The transaction signing-preimage hash. `content_src` spans the RLP of
    the pre-signature fields in the witness and is hashed as a segment
    (never materialized); legacy EIP-155 transactions append
    `(chain_id, 0, 0)`, typed transactions prepend the type byte as a
    domain separator (EIP-2718). */
function tx_signing_hash(t : TxType, content_src : ByteSlice, v : word) -> hash = {
    let tb = tx_type_byte(t);
    let eip155 = (tb == 0x00) & word_ule(limb_to_word(0x0000000000000023), v);
    let chain_id = if eip155 then {
            legacy_sig_chain_id(v)
        } else {
            ZERO_WORD
        };
    let suffix_len = if eip155 then {
            rlp_uint_word_size(chain_id) + LEGACY_SIGNATURE_SUFFIX_LENGTH
        } else {
            BYTE_ZERO
        };
    let content_len = content_src.len + suffix_len;
    let (prefix, prefix_len) = rlp_list_prefix(content_len);
    if eip155 then {
        let mark = scratch_begin();
        rlp_write_uint_word(chain_id);
        scratch_push_bytes([|0x80, 0x80|], LEGACY_SIGNATURE_SUFFIX_LENGTH);
        let suffix = rlp_finish(mark, suffix_len);
        let signing_hash = keccak256([|bytes_list(prefix, prefix_len), BytesSlice(content_src), BytesSlice(suffix)|]);
        scratch_rewind(mark);
        signing_hash
    } else {
        if tb != 0x00 then {
            keccak256([|bytes_list([|tb|], BYTE_ONE), bytes_list(prefix, prefix_len), BytesSlice(content_src)|])
        } else {
            keccak256([|bytes_list(prefix, prefix_len), BytesSlice(content_src)|])
        }
    }
}

/*! The EIP-7702 authorization signing hash:
    `keccak256(0x05 || rlp([chain_id, address, nonce]))`. */
function auth_signing_hash(chain_id : word, addr : address, nonce : account_nonce) -> hash = {
    var content_len = rlp_uint_word_size(chain_id);
    content_len = content_len + rlp_addr_size();
    content_len = content_len + rlp_protocol_quantity_size(nonce);
    let encoded_len = BYTE_ONE + rlp_list_size(content_len);
    let mark = scratch_begin();
    scratch_push_bytes([|0x05|], BYTE_ONE);
    rlp_write_list_prefix(content_len);
    rlp_write_uint_word(chain_id);
    rlp_write_addr(addr);
    rlp_write_protocol_quantity(nonce);
    let encoded = rlp_finish(mark, encoded_len);
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
function tx_sig_v_ok(chain_id : chain_identifier, t : TxType, v : word) -> bool = match t {
    LegacyTx =>   (v == limb_to_word(0x000000000000001b))
                | (v == limb_to_word(0x000000000000001c))
                | (word_ule(limb_to_word(0x0000000000000023), v) &
                      (legacy_sig_chain_id(v) == word_of_protocol_quantity(chain_id))),
    _ => (v == WORD_ZERO) | (v == WORD_ONE),
}

/*! Extracts the recovery parity after [tx_sig_v_ok][] has validated the
    envelope-specific `v` domain. Legacy `27` and every EIP-155 parity-zero
    value are odd; typed envelopes carry the parity directly. */
function tx_y_parity(t : TxType, v : word) -> y_parity = match t {
    LegacyTx => if word_and(v, WORD_ONE) == WORD_ONE then { 0 } else { 1 },
    _ => if v == WORD_ZERO then { 0 } else { 1 },
}

/*! Authenticates a transaction: enforce the EIP-2 low-`s` bound, recover the
    signer selected by `y_parity`, and bind it to the address derived from the
    witnessed 65-byte public key. */
function tx_auth_ok(sender : address, h : hash, parity : y_parity, r : word, s : word) -> bool =
    if word_ult(SECP_N_HALF, s) then {
        false
    } else {
        let (recovered, recovered_sender) = ecrecover_addr(h, parity, r, s);
        recovered & (recovered_sender == sender)
    }
