/*md
# Transaction signature rules

Signature validity, recovery parity, and sender authentication after the
transaction codec has established the envelope's canonical wire shape.
*/

/*md
## The sender authentication

`public_keys[i]` is a witness, not trusted input. The sender address is
derived from the key, then authenticated by recovering the signature's
address with its envelope `y_parity` and requiring the two addresses to
match. A forged key, wrong parity, or bad signature therefore rejects the
block.
*/
/*! Validates the selected signature scheme's `v` domain and returns its
    recovery parity. Legacy accepts `27`/`28` or an EIP-155 value binding the
    chain id; typed envelopes accept only an explicit zero-or-one parity.
    An invalid `v` rejects the block here, so an unvalidated parity is
    impossible to pass to sender recovery. */
function tx_signature_parity(chain_id : chain_identifier, scheme : TxSignatureScheme, v : word) -> y_parity =
    match scheme {
        LegacySignature => {
            let eip155_v = word_ule(35, v);
            let signature_chain_id = legacy_sig_chain_id(v);
            let expected_chain_id = word_of_chain_identifier(chain_id);
            if (v == 27) | (v == 28) | (eip155_v & (signature_chain_id == expected_chain_id)) then {
                let parity_bit = word_and(v, WORD_ONE);
                if parity_bit == WORD_ONE then {
                    0
                } else {
                    1
                }
            } else {
                fatal_error(InvalidSignature)
            }
        },
        TypedSignature => {
            if v == WORD_ZERO then {
                0
            } else if v == WORD_ONE then {
                1
            } else {
                fatal_error(InvalidSignature)
            }
        },
    }

/*! Authenticates a transaction: enforce the EIP-2 low-`s` bound, recover the
    signer selected by `y_parity`, and bind it to the address derived from the
    witnessed 65-byte public key. */
function tx_auth_valid(sender : address, h : hash, parity : y_parity, r : word, s : word) -> bool =
    let high_s = word_ult(SECP_N_HALF, s) in
    if high_s then {
        false
    } else {
        let (recovered, recovered_sender) = ecrecover_addr(h, parity, r, s);
        recovered & (recovered_sender == sender)
    }
