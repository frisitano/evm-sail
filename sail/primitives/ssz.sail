/* Source-backed SSZ collections. */

struct SszListRef = {
    bytes : ByteSlice,
    count : item_count,
}

/* Sequential cursor for a variable-element SSZ list. */
struct SszListCursor = {
    items : SszListRef,
    index : item_index,
    current : source_pointer,
}
