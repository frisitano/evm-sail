/*md
# Code

Executable code and its PUSH-aware `JUMPDEST` analysis (YP §9.4.3).
*/

/*! The valid-`JUMPDEST` bitmap: one little-endian word per 64 code
    bytes. */
type JumpdestBitmap = list(bits(64))

/*! A one-based word offset into the host's packed bitmap arena; zero
    denotes the empty bitmap. */
type JumpdestRef = bits(64)

let EMPTY_JUMPDEST_BITMAP : JumpdestBitmap = [||]
let EMPTY_JUMPDEST_REF : JumpdestRef = 0x0000000000000000

/*! Executable code: its byte span and resolved `JUMPDEST` table. The
    interpreter saves and restores both together; the code hash remains
    the stable code-DB key. */
struct Code = {
    bytes : ByteSlice,
    jumpdests : JumpdestRef,
}

let EMPTY_CODE : Code = struct { bytes = EMPTY_SLICE, jumpdests = EMPTY_JUMPDEST_REF }
