
/*md
# CREATE address RLP codec

The `CREATE` and `CREATE2` address rules (YP §7, EIP-1014).
*/
/*! The `CREATE` address (YP §7): the low 20 bytes of
    `keccak256(rlp([sender, nonce]))`. */
function create_address(sender : address, nonce : account_nonce) -> address = {
    let address_length = rlp_addr_size();
    let nonce_length = rlp_uint_word_size(nonce);
    let content_len = address_length + nonce_length;
    let encoded_len = rlp_list_size(content_len);
    let encoder = rlp_encoder_begin(encoded_len);
    rlp_write_list_prefix(content_len);
    rlp_write_addr(sender);
    rlp_write_uint(nonce);
    let encoded = rlp_encoder_finish(encoder);
    let digest = keccak256(encoded);
    let digest_word = hash_to_word(digest);
    let address = word_to_address(digest_word);
    rlp_encoder_rewind(encoder);
    address
}
