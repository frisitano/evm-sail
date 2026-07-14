/* ======================================================================== */
/* CONTRACT ADDRESS DERIVATION (CREATE / CREATE2)                           */
/* ======================================================================== */

/* CREATE address (Yellow Paper): low 20 bytes of
 * keccak256(rlp([sender, nonce])). */
function create_address(sender : address, nonce : u64) -> address = {
    let payload = list_append(rlp_addr(sender), rlp_int(unsigned(nonce)));
    word_to_address(keccak_rlp_payload([||], payload))
}

/* CREATE2 address (EIP-1014): low 20 bytes of
 * keccak256(0xff ++ sender ++ salt ++ init_hash). */
function create2_address(sender : address, salt : word, init_hash : hash) -> address =
    word_to_address(
        keccak256(
            [|
                BytesList([|0xff|]),
                BytesList(address_to_bytes(sender)),
                BytesList(word_to_bytes32(salt)),
                BytesList(word_to_bytes32(init_hash)),
            |],
        ),
    )
