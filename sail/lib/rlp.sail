/* ======================================================================== */
/* RLP ENCODE / DECODE (YELLOW PAPER APPENDIX B)                            */
/*                                                                          */
/* Recursive Length Prefix, the canonical serialization Ethereum uses for   */
/* accounts, block headers, transactions, receipts, and Merkle-Patricia     */
/* trie nodes. RLP encodes a recursive structure of byte strings and lists; */
/* it imposes no type semantics, only a minimal length-prefixed framing.    */
/*                                                                          */
/* A byte < 0x80 is its own encoding; strings of length n <= 55 use         */
/* 0x80+n; longer strings use 0xb7+|len| followed by the big-endian length. */
/* Lists mirror this with the 0xc0 / 0xf7 prefix bands over an already      */
/* encoded payload. Integers encode as their minimal big-endian byte string */
/* with no leading zeros. Decoding proceeds one level at a time:            */
/* RlpField.content is the string value or list payload, and RlpField.full  */
/* is the complete item bytes used for embedded trie nodes and signing.     */
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

/* RLP list header for a payload of `len` content bytes. */
function rlp_list_prefix(len : int) -> list(byte) =
    if len <= 55 then {
        [|rlp_byte(192 + len)|]
    } else {
        let lb = int_to_be_bytes(len);
        rlp_byte(247 + list_len(lb)) :: lb
    }

/* Wrap an already-encoded item payload as an RLP list. */
function rlp_list(payload : list(byte)) -> list(byte) = {
    list_append(rlp_list_prefix(list_len(payload)), payload)
}

/* RLP-encode a non-negative integer as a minimal big-endian string. */
function rlp_int(n : int) -> list(byte) = rlp_bytes(int_to_be_bytes(n))

/* RLP-encode a fixed 32-byte word and a 20-byte address as byte strings. */
function rlp_word(w : word) -> list(byte) = rlp_bytes(word_to_bytes32(w))
function rlp_addr(a : address) -> list(byte) = rlp_bytes(address_to_bytes(a))

function word_to_min_bytes(w : word) -> list(byte) = {
    let n = word_be_len(w);
    var out : list(byte) = [||];
    foreach (k from 0 to (n - 1)) {
        out = list_append(out, [|sail_shiftright(w, 8 * (n - 1 - k))[7 .. 0]|])
    };
    out
}

function rlp_int_word(w : word) -> list(byte) = rlp_bytes(word_to_min_bytes(w))

function zero_bytes(n : int) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (i from 0 to (n - 1)) {
        out = 0x00 :: out
    };
    out
}

/* CREATE address (Yellow Paper): low 20 bytes of
 * keccak256(rlp([sender, nonce])). */
val create_address : (address, int) -> address

/* CREATE2 address (EIP-1014): low 20 bytes of
 * keccak256(0xff ++ sender ++ salt ++ init_hash). */
val create2_address : (address, word, hash) -> address

function legacy_sig_chain_id(v : int) -> int =
    ediv_int(v - 35, 2)

struct RlpField = {
  is_list : bool,
  content : list(byte),   /* string value, or a list's payload bytes */
  full    : list(byte)    /* full item bytes incl header (an embedded node) */
}

let RLP_NONE : RlpField = struct { is_list = false, content = [||], full = [||] }

/* Malformed RLP is a normal invalid-block outcome: the decoders THROW
 * InvalidBlock(RlpDecode) at the point of detection, caught once at the guest /
 * runner top level (mirroring EELS `raise` on a decoding error). */

/* big-endian integer from n content bytes at offset pos */
function rlp_be_int(b : list(byte), pos : int, n : int) -> int = {
    var acc : int = 0;
    foreach (k from 0 to (n - 1)) {
        acc = acc * 256 + unsigned(bytes_get(b, pos + k))
    };
    acc
}

/* header of the item at pos: (is_list, content_start, content_len) */
function rlp_hdr(b : list(byte), pos : int) -> (bool, int, int) = {
    let h = unsigned(bytes_get(b, pos));
    if h < 128 then {
        (false, pos, 1)
        /* single byte */
    } else {
        if h < 184 then {
            (false, pos + 1, h - 128)
            /* short string (0x80..0xb7) */
        } else {
            if h < 192 then {
                /* long string (0xb8..0xbf) */
                let lol = h - 183;
                (false, pos + 1 + lol, rlp_be_int(b, pos + 1, lol))
            } else {
                if h < 248 then {
                    (true, pos + 1, h - 192)
                    /* short list (0xc0..0xf7) */
                } else {
                    /* long list (0xf8..0xff) */
                    let lol = h - 247;
                    (true, pos + 1 + lol, rlp_be_int(b, pos + 1, lol))
                }
            }
        }
    }
}

/* RLP's scalar integer convention: zero is the empty string (0x80), positive
 * integers are shortest big-endian byte strings, and the surrounding RLP string
 * header must itself be canonical. Raw byte strings use rlp_field_word instead. */
function rlp_uint_item_canonical(full : list(byte), content : list(byte)) -> bool = {
    let n = list_len(content);
    let fl = list_len(full);
    if fl == 0 then {
        false
    } else {
        if n == 0 then {
            if fl == 1 then {
                unsigned(bytes_get(full, 0)) == 128
            } else {
                false
            }
        } else {
            let first = unsigned(bytes_get(content, 0));
            if first == 0 then {
                false
            } else {
                if (n == 1) & (first < 128) then {
                    if fl == 1 then {
                        unsigned(bytes_get(full, 0)) == first
                    } else {
                        false
                    }
                } else {
                    if n <= 55 then {
                        if fl == n + 1 then {
                            unsigned(bytes_get(full, 0)) == 128 + n
                        } else {
                            false
                        }
                    } else {
                        let len_bytes = int_to_be_bytes(n);
                        let lol = list_len(len_bytes);
                        if fl == 1 + lol + n then {
                            var ok : bool = unsigned(bytes_get(full, 0)) == 183 + lol;
                            foreach (k from 0 to (lol - 1)) {
                                ok = ok & (bytes_get(full, 1 + k) == bytes_get(len_bytes, k))
                            };
                            ok
                        } else {
                            false
                        }
                    }
                }
            }
        }
    }
}

/* split the items in [start, stop) into a list of RlpField (single level) */
function rlp_split(b : list(byte), start : int, stop : int) -> list(RlpField) = {
    var out : list(RlpField) = [||];
    var pos : int = start;
    while pos < stop do {
        let (isl, cs, cl) = rlp_hdr(b, pos);
        let nxt = cs + cl;
        out = struct { is_list = isl, content = bytes_slice(b, cs, cl), full = bytes_slice(b, pos, nxt - pos) } :: out;
        pos = nxt
    };
    list_reverse(out)
}

/* decode a full RLP node (always a list) into its top-level items */
function rlp_node_items(node : list(byte)) -> list(RlpField) = {
    if list_len(node) == 0 then {
        [||]
    } else {
        let (isl, cs, cl) = rlp_hdr(node, 0);
        if isl then {
            rlp_split(node, cs, cs + cl)
        } else {
            [||]
        }
    }
}

function rlp_item(items : list(RlpField), idx : int) -> RlpField = {
    if (idx >= 0) & (idx < list_len(items)) then {
        list_nth(items, idx, RLP_NONE)
    } else {
        throw InvalidBlock(RlpDecode)
    }
}

/* The content start offset (into `node`) of top-level item `idx` of an RLP list.
 * Mirrors rlp_split's header walk without materializing item bytes, so a caller
 * can map a decoded field back to its contiguous span in the underlying witness
 * buffer. 0 if node is not a list or idx is out of range; pair it with the
 * item's already-decoded content length. */
function rlp_top_item_content_offset(node : list(byte), idx : int) -> int = {
    if list_len(node) == 0 then {
        0
    } else {
        let (isl, cs, cl) = rlp_hdr(node, 0);
        if not_bool(isl) then {
            0
        } else {
            let stop = cs + cl;
            var pos : int = cs;
            var j : int = 0;
            var res : int = 0;
            var more : bool = true;
            while (pos < stop) & more do {
                let (_, ics, icl) = rlp_hdr(node, pos);
                if j == idx then {
                    res = ics;
                    more = false
                } else {
                    j = j + 1;
                    pos = ics + icl
                }
            };
            res
        }
    }
}

/* Like rlp_top_item_content_offset but returns the item's FULL start offset
 * (its header byte), so an embedded child node's contiguous witness span can be
 * recovered. 0 if not a list or idx is out of range. */
function rlp_top_item_full_offset(node : list(byte), idx : int) -> int = {
    if list_len(node) == 0 then {
        0
    } else {
        let (isl, cs, cl) = rlp_hdr(node, 0);
        if not_bool(isl) then {
            0
        } else {
            let stop = cs + cl;
            var pos : int = cs;
            var j : int = 0;
            var res : int = 0;
            var more : bool = true;
            while (pos < stop) & more do {
                let (_, ics, icl) = rlp_hdr(node, pos);
                if j == idx then {
                    res = pos;
                    more = false
                } else {
                    j = j + 1;
                    pos = ics + icl
                }
            };
            res
        }
    }
}

function rlp_list_items(f : RlpField) -> list(RlpField) =
    if f.is_list then {
        rlp_node_items(f.full)
    } else {
        throw InvalidBlock(RlpDecode)
    }

/* Total byte length of the first `count` fields' full encodings -- the length
 * of their concatenated RLP without materializing it, used to size the
 * signing-content witness span. */
function rlp_fields_prefix_len(items : list(RlpField), count : int) -> int = {
    var total : int = 0;
    var rest : list(RlpField) = items;
    var i : int = 0;
    var more : bool = true;
    while more & (i < count) do {
        match rest {
            f :: t => {
                total = total + list_len(f.full);
                rest = t;
                i = i + 1
            },
            [||] => more = false,
        }
    };
    total
}

/* Transaction and EIP-7702 authorization signing hashes. Spec definitions
 * materialize the canonical RLP preimage; build definitions can refine this
 * boundary without exposing a generic byte-list hash to common code. The tx
 * signing content is a contiguous witness span (the pre-signature fields'
 * RLP); build frames + hashes it C-side rather than materializing the list. */
val tx_signing_hash : (int, ByteSlice, int) -> hash
val auth_signing_hash : (int, address, int) -> hash

/* content of an RlpField as a big-endian integer / 256-bit word */
function rlp_field_int(f : RlpField) -> int = {
    if not_bool(f.is_list) & rlp_uint_item_canonical(f.full, f.content) then {
        rlp_be_int(f.content, 0, list_len(f.content))
    } else {
        throw InvalidBlock(RlpDecode)
    }
}
function rlp_field_word(f : RlpField) -> word = {
    var acc : word = sail_zeros(256);
    let c = f.content;
    let n = list_len(c);
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(bytes_get(c, k), 256))
    };
    acc
}

function rlp_field_uint_word(f : RlpField) -> word = {
    if not_bool(f.is_list) & rlp_uint_item_canonical(f.full, f.content) then {
        rlp_field_word(f)
    } else {
        throw InvalidBlock(RlpDecode)
    }
}

/* ---- self-check: decode rlp([1, 2, 1024]) = c5 01 02 82 04 00 --------- */
function rlp_decode_selfcheck() -> unit = {
    let enc : list(byte) = [|0xc5, 0x01, 0x02, 0x82, 0x04, 0x00|];
    let items = rlp_node_items(enc);
    assert(list_len(items) == 3, "rlp list has 3 items");
    assert(rlp_field_int(list_nth(items, 0, RLP_NONE)) == 1, "item0 == 1");
    assert(rlp_field_int(list_nth(items, 1, RLP_NONE)) == 2, "item1 == 2");
    assert(rlp_field_int(list_nth(items, 2, RLP_NONE)) == 1024, "item2 == 1024");
    assert(not_bool(rlp_uint_item_canonical([|0x00|], [|0x00|])), "integer zero is not byte 0x00");
    assert(not_bool(rlp_uint_item_canonical([|0x81, 0x01|], [|0x01|])), "integer 1 is not length-prefixed")
}
