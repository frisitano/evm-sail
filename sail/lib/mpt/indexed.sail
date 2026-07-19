/*md
# Ordered trie indices

Transaction, withdrawal, and receipt tries use RLP-encoded list indices as
keys. Their bytewise trie order differs from numeric order, so this cursor
emits indices directly in canonical key order.
*/

/*! The item count and next canonical-key position of an indexed trie. */
struct RlpIndexCursor = {
    count : item_count,
    position : item_index,
}

/*! One numeric index, its trie key, and the following key when present. */
struct RlpIndexItem = {
    index : item_index,
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

function rlp_index_cursor(count : item_count) -> RlpIndexCursor =
    struct { count = count, position = 0 }

function rlp_index_cursor_empty(cursor : RlpIndexCursor) -> bool =
    cursor.count <= cursor.position

/*! Counts the one-byte keys that sort before RLP index zero. */
function rlp_index_single_count(count : item_count) -> item_count =
    if count == 0 then {
        0
    } else {
        let rest = protocol_quantity_decrement(count);
        if rest < 127 then { rest } else { 127 }
    }

/*! Maps a canonical-key cursor position back to its numeric list index. */
function rlp_index_at_position(cursor : RlpIndexCursor) -> item_index = {
    if rlp_index_cursor_empty(cursor) then {
        throw InvalidBlock(WitnessDeficient)
    };
    let single_count = rlp_index_single_count(cursor.count);
    if cursor.position < single_count then {
        protocol_quantity_increment(cursor.position)
    } else {
        if cursor.position == single_count then {
            0
        } else {
            cursor.position
        }
    }
}

/*! Removes the next canonical-key item and advances the cursor. */
function rlp_index_cursor_pop(cursor : RlpIndexCursor) -> (RlpIndexItem, RlpIndexCursor) = {
    let index = rlp_index_at_position(cursor);
    let next_cursor : RlpIndexCursor = struct {
            count = cursor.count,
            position = protocol_quantity_increment(cursor.position),
        };
    let next_key = if rlp_index_cursor_empty(next_cursor) then {
            None()
        } else {
            Some(trie_index_key(rlp_index_at_position(next_cursor)))
        };
    (struct { index = index, key = trie_index_key(index), next_key = next_key }, next_cursor)
}
