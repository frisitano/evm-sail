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
    Combining validation and interpretation makes an invalid `v` impossible
    to pass to sender recovery. */
function tx_signature_parity(
    chain_id : chain_identifier,
    scheme : TxSignatureScheme,
    v : word,
) -> option(y_parity) = match scheme {
    LegacySignature => {
        if   (v == 27)
           | (v == 28)
           | (word_ule(35, v) & (legacy_sig_chain_id(v) == word_of_chain_identifier(chain_id)))
        then {
            Some(if word_and(v, WORD_ONE) == WORD_ONE then { 0 } else { 1 })
        } else {
            None()
        }
    },
    TypedSignature => {
        if v == WORD_ZERO then {
            Some(0)
        } else if v == WORD_ONE then {
            Some(1)
        } else {
            None()
        }
    },
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
