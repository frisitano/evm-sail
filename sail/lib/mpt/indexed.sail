/* Ordered keys for transaction, withdrawal, and receipt tries. */

struct RlpIndexCursor = {
    count : item_count,
    position : item_index,
}

struct RlpIndexItem = {
    index : item_index,
    key : TriePath,
    next_key : option(TriePath),
}

type rlp_index_byte_width = range(1, 8)

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

function trie_index_key(index : item_index) -> TriePath =
    if index == 0 then {
        path_new(sail_shiftleft(sail_zero_extend(0x80, 256), 248), 2)
    } else {
        if index <= 127 then {
            path_new(sail_shiftleft(word_of_nat(index), 248), 2)
        } else {
            let width = rlp_index_encoded_width(index);
            let byte_len = width + 1;
            let encoded = or_vec(sail_shiftleft(word_of_nat(128 + width), width * 8), word_of_nat(index));
            let align_shift = 256 - byte_len * 8;
            path_new(sail_shiftleft(encoded, align_shift), byte_len * 2)
        }
    }

function rlp_index_cursor(count : item_count) -> RlpIndexCursor =
    struct { count = count, position = 0 }

function rlp_index_cursor_empty(cursor : RlpIndexCursor) -> bool =
    cursor.count <= cursor.position

function rlp_index_single_count(count : item_count) -> item_count =
    if count == 0 then {
        0
    } else {
        let rest = protocol_quantity_decrement(count);
        if rest < 127 then { rest } else { 127 }
    }

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
