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
