/* Source-backed SSZ collections. */

struct SszListRef = {
    bytes : ByteSlice,
    count : int,
}

/* Sequential cursor for a variable-element SSZ list. */
struct SszListCursor = {
    items : SszListRef,
    index : int,
    current : int,
}
