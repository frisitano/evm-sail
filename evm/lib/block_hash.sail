/* ======================================================================== */
/* Block header RLP + block hash (Ethereum) in Sail                          */
/*                                                                           */
/* block_hash = keccak256(rlp(header)). Legacy/pre-1559 header is the 15-field */
/* form; rlp_header_legacy covers it. Self-contained (rlp + keccak); the       */
/* block hash authenticates a sealed block.                                   */
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
