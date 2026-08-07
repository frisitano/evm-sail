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

/*! One numeric index and its canonical trie key. The numeric index addresses
    the source collection directly; the key places that item in the trie. */
struct RlpIndexItem('maximum : Int), rlp_index_valid_maximum('maximum) = {
    index : range(0, 'maximum - 1),
    key : TriePath,
}

/*! The item count, next canonical-key position, and its cached lookup
    descriptor. Each `rlp(index)` key is therefore constructed exactly once.
    `current` is meaningful iff `position < count`; the pair already carries
    the exhaustion state, so no separate presence wrapper exists. */
struct RlpIndexCursor('maximum : Int), rlp_index_valid_maximum('maximum) = {
    count : range(0, 'maximum),
    position : range(0, 'maximum),
    current : RlpIndexItem('maximum),
}

/*! The minimal nonzero byte width of a supported RLP list index. */
type rlp_index_byte_width = range(1, 8)
/*! A numeric index admitted by an execution-payload indexed trie. */
type rlp_index = range(0, transaction_count_bound - 1)

/*! Returns the minimal byte width of an indexed-trie position. */
function rlp_index_encoded_width(value : rlp_index) -> rlp_index_byte_width =
    if value < 256 then {
        1
    } else if value < 65536 then {
        2
    } else if value < 16777216 then {
        3
    } else if value < 4294967296 then {
        4
    } else if value < 1099511627776 then {
        5
    } else if value < 281474976710656 then {
        6
    } else if value < 72057594037927936 then {
        7
    } else {
        8
    }

/*! The transactions/withdrawals-trie key for list index `i`:
    `rlp(i)` as a nibble path (YP §4.4.2). */
function trie_index_key(index : rlp_index) -> TriePath =
    if index == 0 then {
        let empty_path = path_empty();
        path_append_byte(empty_path, 0x80)
    } else if index <= 127 then {
        let empty_path = path_empty();
        let encoded_index = get_slice_int(8, index, 0);
        path_append_byte(empty_path, encoded_index)
    } else {
        let width = rlp_index_encoded_width(index);
        let empty_path = path_empty();
        let encoded_prefix = get_slice_int(8, 128 + width, 0);
        var path = path_append_byte(empty_path, encoded_prefix);
        var remaining : range(0, 8) = width;
        while remaining != 0 termination_measure(remaining) do {
            let current_remaining = remaining;
            let byte_offset : range(0, 7) =
                if 0 < current_remaining then current_remaining - 1 else fatal_error(WitnessDeficient);
            let shift : range(0, 56) = byte_offset * 8;
            let encoded_index = get_slice_int(8, index, shift);
            path = path_append_byte(path, encoded_index);
            remaining = byte_offset
        };
        path
    }

/*! Maps a canonical-key cursor position back to its numeric list index. */
function rlp_index_at_position forall ('maximum : Int), rlp_index_valid_maximum('maximum). (cursor :
    RlpIndexCursor('maximum)) -> (
    range(0, 'maximum - 1)
) = {
    let count = cursor.count;
    let position = cursor.position;
    if position < count then {
        let rest : range(0, 'maximum - 1) = count - 1;
        let single_count : range(0, 'maximum - 1) =
            if rest < 127 then rest else 127;
        if position < single_count then {
            position + 1
        } else if position == single_count then {
            0
        } else {
            position
        }
    } else {
        fatal_error(WitnessDeficient)
    }
}

/*! Starts canonical RLP-index traversal for a bounded collection. */
function rlp_index_cursor forall ('maximum : Int), rlp_index_valid_maximum('maximum). (count : range(0, 'maximum)) -> (
    RlpIndexCursor('maximum)
) = {
    var cursor : RlpIndexCursor('maximum) = struct {
            count = count,
            position = 0,
            current = struct { index = 0, key = path_empty() },
        };
    if count != 0 then {
        let index = rlp_index_at_position(cursor);
        cursor.current = struct { index = index, key = trie_index_key(index) }
    };
    cursor
}

/*! Whether canonical RLP-index traversal has consumed every index. */
function rlp_index_cursor_empty forall ('maximum : Int), rlp_index_valid_maximum('maximum). (cursor :
    RlpIndexCursor('maximum)) -> (
    bool
) =
    cursor.position == cursor.count

/*! Returns the cached numeric index and trie key without advancing. */
function rlp_index_cursor_peek forall ('maximum : Int), rlp_index_valid_maximum('maximum). (cursor :
    RlpIndexCursor('maximum)) -> (
    RlpIndexItem('maximum)
) =
    if cursor.position < cursor.count then {
        cursor.current
    } else {
        fatal_error(WitnessDeficient)
    }

/*! Consumes the current position and returns its fully populated successor. */
function rlp_index_cursor_advance forall ('maximum : Int), rlp_index_valid_maximum('maximum). (cursor :
    RlpIndexCursor('maximum)) -> (
    RlpIndexCursor('maximum)
) = {
    let count = cursor.count;
    let position = cursor.position;
    if position < count then {
        var next : RlpIndexCursor('maximum) = struct {
                count = count,
                position = position + 1,
                current = struct { index = 0, key = path_empty() },
            };
        if next.position < count then {
            let index = rlp_index_at_position(next);
            next.current = struct { index = index, key = trie_index_key(index) }
        };
        next
    } else {
        fatal_error(WitnessDeficient)
    }
}

/*! Removes the cached indexed item and advances the cursor. */
function rlp_index_cursor_pop forall ('maximum : Int), rlp_index_valid_maximum('maximum). (cursor :
    RlpIndexCursor('maximum)) -> (
    (RlpIndexItem('maximum), RlpIndexCursor('maximum))
) = {
    let item = rlp_index_cursor_peek(cursor);
    (item, rlp_index_cursor_advance(cursor))
}
