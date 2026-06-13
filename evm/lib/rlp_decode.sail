/* ======================================================================== */
/* RLP decode (the inverse of rlp.sail's encoders) — needed to read witness    */
/* trie nodes and account/storage leaves during stateless validation.         */
/*                                                                           */
/* One level is parsed at a time (rlp_split / rlp_node_items): each item is a   */
/* RlpField carrying whether it is a list, its content bytes (the string value */
/* or the list payload), and its full bytes (header+body, for descending into  */
/* an embedded child node). The MPT walk recurses one node at a time, so no    */
/* mutually-recursive RLP traversal / termination measure is needed here.      */
/* ======================================================================== */

struct RlpField = {
  rf_is_list : bool,
  rf_content : list(byte),   /* string value, or a list's payload bytes */
  rf_full    : list(byte)    /* full item bytes incl header (an embedded node) */
}

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

/* split the items in [start, stop) into a list of RlpField (single level) */
function rlp_split(b : list(byte), start : int, stop : int) -> list(RlpField) = {
    var out : list(RlpField) = [||];
    var pos : int = start;
    while pos < stop do {
        let (isl, cs, cl) = rlp_hdr(b, pos);
        let nxt = cs + cl;
        out =
            struct { rf_is_list = isl, rf_content = bytes_slice(b, cs, cl), rf_full = bytes_slice(b, pos, nxt - pos) } ::
                out;
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

/* content of an RlpField as a big-endian integer / 256-bit word */
function rlp_field_int(f : RlpField) -> int =
    rlp_be_int(f.rf_content, 0, list_len(f.rf_content))
function rlp_field_word(f : RlpField) -> word = {
    var acc : word = sail_zeros(256);
    let c = f.rf_content;
    let n = list_len(c);
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(bytes_get(c, k), 256))
    };
    acc
}

/* ---- S2a self-check: decode rlp([1, 2, 1024]) = c5 01 02 82 04 00 ------ */
function rlp_decode_selfcheck() -> unit = {
    let enc : list(byte) = [|0xc5, 0x01, 0x02, 0x82, 0x04, 0x00|];
    let items = rlp_node_items(enc);
    assert(list_len(items) == 3, "rlp list has 3 items");
    assert(
        rlp_field_int(list_nth(items, 0, struct { rf_is_list = false, rf_content = [||], rf_full = [||] })) == 1,
        "item0 == 1"
    );
    assert(
        rlp_field_int(list_nth(items, 1, struct { rf_is_list = false, rf_content = [||], rf_full = [||] })) == 2,
        "item1 == 2"
    );
    assert(
        rlp_field_int(list_nth(items, 2, struct { rf_is_list = false, rf_content = [||], rf_full = [||] })) == 1024,
        "item2 == 1024"
    )
}
