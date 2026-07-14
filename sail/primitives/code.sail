/* Executable code and its PUSH-aware JUMPDEST analysis. */

/* One little-endian bitmap word per 64 code bytes. */
type JumpdestBitmap = list(bits(64))

/* One-based word offset in the host's packed bitmap arena; zero is empty. */
type JumpdestRef = bits(64)

let EMPTY_JUMPDEST_BITMAP : JumpdestBitmap = [||]
let EMPTY_JUMPDEST_REF : JumpdestRef = 0x0000000000000000

/* The interpreter saves and restores the byte span and its resolved table
 * together. The code hash remains the stable code-DB key. */
struct Code = {
    bytes : ByteSlice,
    jumpdests : JumpdestRef,
}

let EMPTY_CODE : Code = struct { bytes = EMPTY_SLICE, jumpdests = EMPTY_JUMPDEST_REF }
