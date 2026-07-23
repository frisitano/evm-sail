/*md
# Ordered trie indices

Transaction, withdrawal, and receipt tries use RLP-encoded list indices as
keys. Their bytewise trie order differs from numeric order, so this cursor
emits indices directly in canonical key order.
*/

/*! A supported indexed-trie collection maximum. Transaction lists provide the
    largest schema bound among transactions, receipts, and withdrawals. */
type rlp_index_valid_maximum('maximum : Int) -> Bool =
    0 < 'maximum & 'maximum <= transaction_count_bound

/*! The item count and next canonical-key position of a schema-bounded indexed
    trie. Both retain the enclosing collection's semantic maximum. */
struct RlpIndexCursor('maximum : Int), rlp_index_valid_maximum('maximum) = {
    count : range(0, 'maximum),
    position : range(0, 'maximum),
}

/*! One numeric index, its trie key, and the following key when present. */
struct RlpIndexItem('maximum : Int), rlp_index_valid_maximum('maximum) = {
    index : range(0, 'maximum - 1),
    key : TriePath,
    next_key : option(TriePath),
}

/*! The minimal nonzero byte width of a supported RLP list index. */
type rlp_index_byte_width = range(1, 8)

/*! Decrements a positive RLP index byte width. */
function rlp_index_byte_width_decrement(value : range(0, 8)) -> range(0, 7) = {
    assert(0 < value);
    value - 1
}

/*! Returns the minimal byte width of an indexed-trie position. */
function rlp_index_encoded_width(value : item_index) -> rlp_index_byte_width =
    if value < 256 then {
        1
    } else {
        if value < 65536 then {
            2
        } else {
            if value < 16777216 then {
                3
            } else {
                if value < 4294967296 then {
                    4
                } else {
                    if value < 1099511627776 then {
                        5
                    } else {
                        if value < 281474976710656 then {
                            6
                        } else {
                            if value < 72057594037927936 then { 7 } else { 8 }
                        }
                    }
                }
            }
        }
    }

/*! The transactions/withdrawals-trie key for list index `i`:
    `rlp(i)` as a nibble path (YP §4.4.2). */
function trie_index_key(index : item_index) -> TriePath =
    if index == 0 then {
        path_append_byte(path_empty(), 0x80)
    } else {
        if index <= 127 then {
            path_append_byte(path_empty(), get_slice_int(8, index, 0))
        } else {
            let width = rlp_index_encoded_width(index);
            var path = path_append_byte(path_empty(), get_slice_int(8, 128 + width, 0));
            var remaining : range(0, 8) = width;
            foreach (_offset from 0 to 7) {
                if remaining != 0 then {
                    let byte_offset = rlp_index_byte_width_decrement(remaining);
                    let shift : range(0, 56) = byte_offset * 8;
                    path = path_append_byte(path, get_slice_int(8, index, shift));
                    remaining = byte_offset
                }
            };
            path
        }
    }

/*! Starts canonical RLP-index traversal for a bounded collection. */
function rlp_index_cursor forall ('maximum : Int), rlp_index_valid_maximum('maximum). (count : range(0, 'maximum)) -> (
    RlpIndexCursor('maximum)
) =
    struct { count = count, position = 0 }

/*! Whether canonical RLP-index traversal has consumed every index. */
function rlp_index_cursor_empty forall ('maximum : Int), rlp_index_valid_maximum('maximum). (cursor :
    RlpIndexCursor('maximum)) -> (
    bool
) =
    cursor.count <= cursor.position

/*! Maps a canonical-key cursor position back to its numeric list index. */
function rlp_index_at_position forall ('maximum : Int), rlp_index_valid_maximum('maximum). (cursor :
    RlpIndexCursor('maximum)) -> (
    range(0, 'maximum - 1)
) = {
    let count = cursor.count;
    let position = cursor.position;
    if position < count then {
        let rest : range(0, 'maximum - 1) = count - 1;
        let single_count : range(0, 'maximum - 1) = if rest < 127 then { rest } else { 127 };
        if position < single_count then {
            position + 1
        } else {
            if position == single_count then { 0 } else { position }
        }
    } else {
        throw InvalidBlock(WitnessDeficient)
    }
}

/*! Removes the next canonical-key item and advances the cursor. */
function rlp_index_cursor_pop forall ('maximum : Int), rlp_index_valid_maximum('maximum). (cursor :
    RlpIndexCursor('maximum)) -> (
    (RlpIndexItem('maximum), RlpIndexCursor('maximum))
) = {
    let count = cursor.count;
    let position = cursor.position;
    if position < count then {
        let index = rlp_index_at_position(cursor);
        let next_cursor : RlpIndexCursor('maximum) = struct { count = count, position = position + 1 };
        let next_key = if rlp_index_cursor_empty(next_cursor) then {
                None()
            } else {
                Some(trie_index_key(rlp_index_at_position(next_cursor)))
            };
        (struct { index = index, key = trie_index_key(index), next_key = next_key }, next_cursor)
    } else {
        throw InvalidBlock(WitnessDeficient)
    }
}
