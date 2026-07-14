/*md
# SSZ collections

Source-backed SSZ collections: list references and cursors that decode
elements only when a consumer reaches them.
*/

/*! A reference to an SSZ list: its byte span and element count. */
struct SszListRef = {
    bytes : ByteSlice,
    count : int,
}

/*! A sequential cursor over a variable-element SSZ list. */
struct SszListCursor = {
    items : SszListRef,
    index : int,
    current : int,
}
