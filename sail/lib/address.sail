
/*md
# Contract address derivation

The `CREATE` and `CREATE2` address rules (YP §7, EIP-1014).
*/
/*! The `CREATE` address (YP §7): the low 20 bytes of
    `keccak256(rlp([sender, nonce]))`. */
function create_address(sender : address, nonce : account_nonce) -> address = {
    let content_len = rlp_addr_size() + rlp_uint_size(nonce);
    let encoded_len = rlp_list_size(content_len);
    let mark = scratch_begin();
    rlp_write_list_prefix(content_len);
    rlp_write_addr(sender);
    rlp_write_uint(nonce);
    let encoded = rlp_finish(mark, encoded_len);
    let address = word_to_address(keccak256_slice(encoded));
    scratch_rewind(mark);
    address
}

/*! The `CREATE2` address (EIP-1014): the low 20 bytes of
    `keccak256(0xff ++ sender ++ salt ++ keccak256(initcode))`. */
function create2_address(sender : address, salt : word, init_hash : hash) -> address =
    word_to_address(
        keccak256(
            [|
                bytes_list([|0xff|], BYTE_ONE),
                bytes_list(address_to_bytes(sender), ADDRESS_BYTE_LENGTH),
                bytes_list(word_to_bytes32(salt), WORD_BYTE_LENGTH),
                bytes_list(word_to_bytes32(init_hash), WORD_BYTE_LENGTH),
            |],
        ),
    )
