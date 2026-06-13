/* ======================================================================== */
/* RLP ENCODING (YELLOW PAPER APPENDIX B)                                   */
/*                                                                          */
/* Recursive Length Prefix, the canonical serialization Ethereum uses for   */
/* accounts, block headers, transactions, receipts, and Merkle-Patricia     */
/* trie nodes. RLP encodes a recursive structure of byte strings and lists; */
/* it imposes no type semantics, only a minimal length-prefixed framing.    */
/*                                                                          */
/* The encoders here cover the cases the model emits. A single byte < 0x80  */
/* is its own encoding; a string of length n <= 55 takes a 0x80+n prefix; a */
/* longer string takes a 0xb7+|len| prefix followed by the big-endian       */
/* length. Lists mirror this with the 0xc0 / 0xf7 prefix bands over an      */
/* already-encoded payload. Integers encode as their minimal big-endian     */
/* byte string (no leading zeros).                                          */
/* ======================================================================== */

function rlp_byte(n : int) -> byte = get_slice_int(8, n, 0)

/* RLP-encode a byte string. */
function rlp_bytes(data : list(byte)) -> list(byte) = {
    let n = list_len(data);
    if n == 1 then {
        if unsigned(list_nth(data, 0, 0x00)) < 128 then {
            data
        } else {
            list_append([|rlp_byte(129)|], data)
        } /* 0x80 + 1 */
    } else {
        if n <= 55 then {
            list_append([|rlp_byte(128 + n)|], data)
        } else {
            let lb = int_to_be_bytes(n);
            list_append(list_append([|rlp_byte(183 + list_len(lb))|], lb), data)
        }
    }
}

/* Wrap an already-encoded item payload as an RLP list. */
function rlp_list(payload : list(byte)) -> list(byte) = {
    let n = list_len(payload);
    if n <= 55 then {
        list_append([|rlp_byte(192 + n)|], payload)
    } else {
        let lb = int_to_be_bytes(n);
        list_append(list_append([|rlp_byte(247 + list_len(lb))|], lb), payload)
    }
}

/* RLP-encode a non-negative integer as a minimal big-endian string. */
function rlp_int(n : int) -> list(byte) = rlp_bytes(int_to_be_bytes(n))

/* RLP-encode a fixed 32-byte word and a 20-byte address as byte strings. */
function rlp_word(w : word) -> list(byte) = rlp_bytes(word_to_bytes32(w))
function rlp_addr(a : address) -> list(byte) = rlp_bytes(address_to_bytes(a))
