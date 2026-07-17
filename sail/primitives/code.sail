/* Executable code and its PUSH-aware JUMPDEST analysis. */

/* One bit per code byte, with the lowest code offset in the least-significant
 * bit. Analysis writes one completed 256-byte chunk at a time. */
type JumpdestChunk = bits(256)

/* Opaque handle to one preallocated table; zero is empty. */
type JumpdestRef = bits(64)

let EMPTY_JUMPDEST_CHUNK : JumpdestChunk = 0x0000000000000000000000000000000000000000000000000000000000000000
let EMPTY_JUMPDEST_REF : JumpdestRef = 0x0000000000000000

/* The interpreter saves and restores the byte span and its resolved table
 * together. The code hash remains the stable code-DB key. */
struct Code = {
    bytes : ByteSlice,
    jumpdests : JumpdestRef,
}

let EMPTY_CODE : Code = struct { bytes = EMPTY_SLICE, jumpdests = EMPTY_JUMPDEST_REF }
