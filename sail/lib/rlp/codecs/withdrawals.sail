/*md
# Withdrawal RLP codec

The header-level commitments recomputed from the payload: the
transactions and withdrawals tries, the EIP-7685 requests hash, and the
block header hash itself.
*/

/*! The RLP of one withdrawal (EIP-4895), assembled in the scratch
    arena. */
function withdrawal_rlp(withdrawal : StatelessInputSliceLength(44)) -> ScratchSlice = {
    let index = decode_ssz_uint(withdrawal, WD_INDEX);
    let validator_index = decode_ssz_uint(withdrawal, WD_VALIDATOR_INDEX);
    let address = sub_slice(withdrawal, WD_ADDRESS, ADDRESS_BYTE_LENGTH);
    let amount = decode_ssz_uint(withdrawal, WD_AMOUNT);
    let index_length = rlp_uint_size(index);
    let validator_index_length = rlp_uint_size(validator_index);
    let address_length = rlp_slice_size(address);
    let amount_length = rlp_uint_size(amount);
    let content_length = index_length + validator_index_length + address_length + amount_length;
    if 48 < content_length then {
        fatal_error(RlpDecode);
        exit(())
    };
    let bounded_content_length : range(0, 48) = tmod_nat(content_length, 49);
    let content_len = bounded_content_length;
    let encoder = rlp_encoder_begin(rlp_list_size(content_len));
    rlp_write_list_prefix(content_len);
    rlp_write_uint(index);
    rlp_write_uint(validator_index);
    rlp_write_slice(address);
    rlp_write_uint(amount);
    rlp_encoder_finish(encoder)
}
