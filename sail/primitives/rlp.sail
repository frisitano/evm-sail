/* ======================================================================== */
/* PRIMITIVES: RLP FIELD                                                    */
/*                                                                          */
/* A decoded field that keeps offsets into its original byte source.        */
/* Pure data -- no registers, no externs.                                   */
/* ======================================================================== */

struct RlpFieldRef = {
    source      : Bytes,
    is_list     : bool,
    full_off    : bits(64),
    full_len    : bits(64),
    content_off : bits(64),
    content_len : bits(64),
}

/* A one-pass view over the immediate children of an RLP list. */
struct RlpCursor = {
    source : Bytes,
    current : int,
    stop : int,
    valid : bool,
}

/* An RLP byte sequence represented without flattening its source-backed
 * payloads. Segments are accumulated in reverse order so appending a new
 * encoded item never walks the already-built prefix. */
struct RlpBuilder = {
    segments_rev : list(Bytes),
    len : int,
}

let EMPTY_RLP_BUILDER : RlpBuilder = struct { segments_rev = [||], len = 0 }
