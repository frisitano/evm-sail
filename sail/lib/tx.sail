/*md
# Transaction signature rules

The signature semantics of transactions, between the crypto core
(secp verify/recover and the curve constants) and the envelope decoder:
the signing preimage hashes, the per-envelope `v`-range rule, and sender
authentication — binding the witnessed public key to a transaction's
signature, called from transaction validity.
*/

/*! The chain id encoded in a legacy signature with `v >= 35`
    (EIP-155). */
function legacy_sig_chain_id(v : int) -> int =
    ediv_int(v - 35, 2)

/*! The transaction signing-preimage hash. `content_src` spans the RLP of
    the pre-signature fields in the witness and is hashed as a segment
    (never materialized); legacy EIP-155 transactions append
    `(chain_id, 0, 0)`, typed transactions prepend the type byte as a
    domain separator (EIP-2718). */
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

/*! The EIP-7702 authorization signing hash:
    `keccak256(0x05 || rlp([chain_id, address, nonce]))`. */
function auth_signing_hash(chain_id : int, addr : address, nonce : int) -> hash = {
    let content = list_append(rlp_int(chain_id), list_append(rlp_addr(addr), rlp_int(nonce)));
    keccak_rlp_payload([|0x05|], content)
}

/*md
## Sender authentication

`public_keys[i]` is a witness, not trusted input: the guest authenticates
it by verifying transaction `i`'s signature against it over the signing
hash (verification is cheaper than recovery — which is why the public key
is supplied at all). The sender then derives from the authenticated key. A
forged or wrong public key fails verification, so the block is rejected.
*/

/*! The per-envelope `v`-range rule: legacy accepts `27`/`28` or the
    EIP-155 form binding the chain id; typed envelopes accept
    `y_parity ∈ {0, 1}`. */
function tx_sig_v_ok(chain_id : int, t : TxType, v : int) -> bool = match t {
    LegacyTx => (v == 27) | (v == 28) | ((v >= 35) & (legacy_sig_chain_id(v) == chain_id)),
    _ => (v == 0) | (v == 1),
}

/*! Authenticates a transaction: the EIP-2 low-`s` bound, then binding its
    65-byte witness public key (`0x04 ++ X ++ Y`) to the signature over
    the signing hash. */
function tx_auth_ok(pubkey : ByteSlice, h : bits(256), r : word, s : word) -> bool =
    if unsigned(s) > unsigned(SECP_N_HALF) then {
        false
    } else {
        let x = slice_load(pubkey, 1);
        let y = slice_load(pubkey, 33);
        secp256k1_verify(h, r, s, x, y)
    }
