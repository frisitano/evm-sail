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
 * signing content is a contiguous stateless-input span (the pre-signature fields'
 * RLP); build frames + hashes it C-side rather than materializing the list. */
/* Tx signing preimage hash. `content_src` spans the RLP of the pre-signature
 * fields in the witness and is hashed as a SEGMENT (never materialized);
 * legacy EIP-155 txs append (chain_id, 0, 0), typed txs prepend the type
 * byte as a domain separator. */
function tx_signing_hash(t : TxType, content_src : ByteSlice, v : int) -> hash = {
    let tb = tx_type_byte(t);
    let suffix : list(byte) = if (tb == 0) & (v >= 35) then {
            list_append(rlp_int(legacy_sig_chain_id(v)), [|0x80, 0x80|])
        } else {
            [||]
        };
    let domain : list(byte) = if tb != 0 then {
            [|get_slice_int(8, tb, 0)|]
        } else {
            [||]
        };
    let content_len = unsigned(content_src.len) + list_len(suffix);
    keccak256(
        [|BytesList(domain), BytesList(rlp_list_prefix(content_len)), BytesSlice(content_src), BytesList(suffix)|],
    )
}

/* EIP-7702 authorization signing hash: 0x05 || rlp([chain_id, address, nonce]). */
function auth_signing_hash(chain_id : int, addr : address, nonce : int) -> hash = {
    let content = list_append(rlp_int(chain_id), list_append(rlp_addr(addr), rlp_int(nonce)));
    keccak_rlp_payload([|0x05|], content)
}

/* ---- transaction sender authentication (EIP-2718 / EIP-155 + secp256k1) ------
 * public_keys[i] is a WITNESS, not trusted input: the guest authenticates it by
 * verifying tx i's signature against it over the signing hash (verify is cheaper
 * than recovery -- which is why the pubkey is supplied). The sender then derives
 * from the authenticated key. A forged/wrong pubkey fails verification, so the
 * block is rejected (successful_validation = false). */

function tx_sig_v_ok(chain_id : int, t : TxType, v : int) -> bool = match t {
    LegacyTx => (v == 27) | (v == 28) | ((v >= 35) & (legacy_sig_chain_id(v) == chain_id)),
    _ => (v == 0) | (v == 1),
}

/* authenticate a tx: EIP-2 low-s, then bind its 65-byte witness public key
 * (0x04 ++ X ++ Y) to the signature over the signing hash */
function tx_auth_ok(pubkey : ByteSlice, h : bits(256), r : word, s : word) -> bool =
    if unsigned(s) > unsigned(SECP_N_HALF) then {
        false
    } else {
        let x = slice_load(pubkey, 1);
        let y = slice_load(pubkey, 33);
        secp256k1_verify(h, r, s, x, y)
    }
