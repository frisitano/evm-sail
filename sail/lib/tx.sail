/*md
# Transaction signature rules

Signature validity, recovery parity, and sender authentication after the
transaction codec has established the envelope's canonical wire shape.
*/

/*md
## The sender recovery

The input carries no sender or public key. The sender is the address recovered
from the signature over the signing hash, selected by the envelope `y_parity`.
A bad `r`, `s`, or `v` therefore rejects the block.
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

/*! Recovers the sender of a transaction: enforce the EIP-2 low-`s` bound and
    the `(0, n)` range of `r`, then recover the signer selected by
    `y_parity`. Any failure rejects the block. */
function tx_recover_sender(h : hash, parity : y_parity, r : word, s : word) -> address = {
    let valid_range =   word_ult(ZERO_WORD, r)
                      & word_ult(r, SECP_N_FULL)
                      & word_ult(ZERO_WORD, s)
                      & word_ule(s, SECP_N_HALF);
    let invalid_range = not_bool(valid_range);
    if invalid_range then {
        fatal_error(InvalidSignature)
    };
    let recovered = ecrecover_addr(h, parity, r, s);
    if recovered.success then {
        recovered.address
    } else {
        fatal_error(InvalidSignature)
    }
}
