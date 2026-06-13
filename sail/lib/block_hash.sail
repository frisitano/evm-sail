/* ======================================================================== */
/* BLOCK HEADER RLP + BLOCK HASH (Yellow Paper section 4.3)                  */
/*                                                                          */
/* The block hash is keccak256(rlp(H)) over the header H -- the commitment    */
/* that seals a block and chains it to its parent (each header carries its    */
/* parent's hash). The header is RLP-encoded field-by-field in YP order, then */
/* hashed.                                                                    */
/*                                                                          */
/* rlp_header_legacy covers the original 15-field header (frontier through    */
/* the fork before EIP-1559 introduced the baseFeePerGas field). Later forks  */
/* append further fields (baseFeePerGas, withdrawalsRoot, blob gas, ...) to    */
/* the same RLP-list framing.                                                 */
/* ======================================================================== */

function zero_bytes(n : int) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to (n - 1)) {
        let _ = k;
        out = list_append(out, [|0x00|])
    };
    out
}

/* RLP of the legacy 15-field block header (frontier..london-minus-baseFee). */
function rlp_header_legacy(
    parent : word,
    uncle : word,
    coinbase : address,
    sroot : word,
    txroot : word,
    rroot : word,
    bloom : list(byte),
    difficulty : int,
    number : int,
    gaslimit : int,
    gasused : int,
    timestamp : int,
    extra : list(byte),
    mix : word,
    nonce : list(byte),
) -> (
    list(byte)
) = {
    var p : list(byte) = [||];
    p = list_append(p, rlp_word(parent));
    p = list_append(p, rlp_word(uncle));
    p = list_append(p, rlp_addr(coinbase));
    p = list_append(p, rlp_word(sroot));
    p = list_append(p, rlp_word(txroot));
    p = list_append(p, rlp_word(rroot));
    p = list_append(p, rlp_bytes(bloom));
    p = list_append(p, rlp_int(difficulty));
    p = list_append(p, rlp_int(number));
    p = list_append(p, rlp_int(gaslimit));
    p = list_append(p, rlp_int(gasused));
    p = list_append(p, rlp_int(timestamp));
    p = list_append(p, rlp_bytes(extra));
    p = list_append(p, rlp_word(mix));
    p = list_append(p, rlp_bytes(nonce));
    rlp_list(p)
}

function block_hash_legacy(
    parent : word,
    uncle : word,
    coinbase : address,
    sroot : word,
    txroot : word,
    rroot : word,
    bloom : list(byte),
    difficulty : int,
    number : int,
    gaslimit : int,
    gasused : int,
    timestamp : int,
    extra : list(byte),
    mix : word,
    nonce : list(byte),
) -> (
    bits(256)
) =
    keccak256(
        rlp_header_legacy(
            parent,
            uncle,
            coinbase,
            sroot,
            txroot,
            rroot,
            bloom,
            difficulty,
            number,
            gaslimit,
            gasused,
            timestamp,
            extra,
            mix,
            nonce,
        ),
    )
