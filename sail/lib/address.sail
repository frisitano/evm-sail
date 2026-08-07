/*md
# CREATE2 address derivation

The fixed-width EIP-1014 preimage does not use RLP and remains separate from
the CREATE address codec.
*/

/*! The `CREATE2` address (EIP-1014): the low 20 bytes of
    `keccak256(0xff ++ sender ++ salt ++ keccak256(initcode))`. */
function create2_address(sender : address, salt : word, init_hash : hash) -> address = {
    let mark = scratch_reserve(85);
    scratch_push_byte(0xff);
    scratch_push_address(sender);
    let salt_hash = word_to_hash(salt);
    scratch_push_b256(salt_hash, WORD_BYTE_LENGTH);
    scratch_push_b256(init_hash, WORD_BYTE_LENGTH);
    let preimage = scratch_finish(mark);
    let digest = keccak256(preimage);
    scratch_rewind(mark);
    let digest_word = hash_to_word(digest);
    word_to_address(digest_word)
}
