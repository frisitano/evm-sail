/* ======================================================================== */
/* PRIMITIVES: RLP FIELD                                                    */
/*                                                                          */
/* One decoded top-level RLP item, as the codec (lib/rlp/) yields it.       */
/* Pure data -- no registers, no externs.                                   */
/* ======================================================================== */

struct RlpField = {
    is_list : bool,
    content : list(byte), /* string value, or a list's payload bytes         */
    full : list(byte), /* full item bytes incl header (an embedded node)  */
}

let RLP_NONE : RlpField = struct { is_list = false, content = [||], full = [||] }

/* A decoded field that keeps offsets into its original byte source. */
struct RlpFieldRef = {
    source      : Bytes,
    is_list     : bool,
    full_off    : bits(64),
    full_len    : bits(64),
    content_off : bits(64),
    content_len : bits(64),
}

let RLP_REF_NONE : RlpFieldRef = struct {
        source = BytesList([||]),
        is_list = false,
        full_off = 0x0000000000000000,
        full_len = 0x0000000000000000,
        content_off = 0x0000000000000000,
        content_len = 0x0000000000000000,
    }
