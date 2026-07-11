/* ======================================================================== */
/* TRANSACTION SIGNATURE RULES                                              */
/*                                                                          */
/* The signature semantics of transactions, between the crypto core         */
/* (primitives/crypto.sail: secp verify / recover, curve constants) and     */
/* the envelope decoder (lib/rlp/tx.sail): the signing preimage hashes,     */
/* the per-envelope v-range rule, and sender authentication -- binding the  */
/* witnessed public key to a tx's signature (EELS                           */
/* recover_sender_from_public_key), called from transaction validity.       */
/* ======================================================================== */

/* EIP-155: the chain id encoded in a legacy signature v >= 35 */
function legacy_sig_chain_id(v : int) -> int =
    ediv_int(v - 35, 2)

/* Transaction and EIP-7702 authorization signing hashes. Spec definitions
 * materialize the canonical RLP preimage; build definitions can refine this
 * boundary without exposing a generic byte-list hash to common code. The tx
 * signing content is a contiguous witness span (the pre-signature fields'
 * RLP); build frames + hashes it C-side rather than materializing the list. */
/* Tx signing preimage hash. `content_src` spans the RLP of the pre-signature
 * fields in the witness; legacy EIP-155 txs append (chain_id, 0, 0), typed txs
 * prepend the type byte as a domain separator. */
function tx_signing_hash(t : TxType, content_src : ByteSlice, v : int) -> hash = {
    let tb = tx_type_byte(t);
    var content : list(byte) = read_byte_slice(content_src);
    if (tb == 0) & (v >= 35) then {
        content = list_append(content, rlp_int(legacy_sig_chain_id(v)));
        content = list_append(content, [|0x80, 0x80|])
    };
    let domain : list(byte) = if tb != 0 then {
            [|get_slice_int(8, tb, 0)|]
        } else {
            [||]
        };
    keccak_rlp_payload(domain, content)
}

/* EIP-7702 authorization signing hash: 0x05 || rlp([chain_id, address, nonce]). */
function auth_signing_hash(chain_id : int, addr : address, nonce : int) -> hash = {
    var content : list(byte) = rlp_int(chain_id);
    content = list_append(content, rlp_addr(addr));
    content = list_append(content, rlp_int(nonce));
    keccak_rlp_payload([|0x05|], content)
}

/* ---- transaction sender authentication (EIP-2718 / EIP-155 + secp256k1) ------
 * public_keys[i] is a WITNESS, not trusted input: the guest authenticates it by
 * verifying tx i's signature against it over the signing hash (verify is cheaper
 * than recovery -- which is why the pubkey is supplied). The sender then derives
 * from the authenticated key. A forged/wrong pubkey fails verification, so the
 * block is rejected (successful_validation = false). */

function tx_sig_v_ok(t : TxType, v : int) -> bool = match t {
    LegacyTx => (v == 27) | (v == 28) | ((v >= 35) & (legacy_sig_chain_id(v) == cc_chain_id())),
    _ => (v == 0) | (v == 1),
}

/* authenticate a tx: EIP-2 low-s, then bind its 65-byte witness public key
 * (0x04 ++ X ++ Y) to the signature over the signing hash */
function tx_auth_ok(pubkey : ByteSlice, h : bits(256), r : word, s : word) -> bool =
    if unsigned(s) > unsigned(SECP_N_HALF) then {
        false
    } else {
        let x = bytes_be256(read_byte_slice(sub_slice(pubkey, 1, 32)));
        let y = bytes_be256(read_byte_slice(sub_slice(pubkey, 33, 32)));
        secp256k1_verify(h, r, s, x, y)
    }
