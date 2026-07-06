/* ======================================================================== */
/* BUILD BACKEND: SOURCE-TAGGED HASHING                                     */
/*                                                                          */
/* Hash a byte source (Witness/Memory/TxInput/ActiveCode) directly via C,   */
/* without materializing a Sail byte list. The reference semantics is       */
/* bytes_keccak(s) = keccak256(read_byte_slice(s)) and the prefixed digest  */
/* is sha256(type_byte :: read_byte_slice(s)). The hybrid backend           */
/* materializes from the pure models instead (hybrid/crypto.sail); the      */
/* source-free crypto core shared with hybrid is build/crypto.sail.         */
/* ======================================================================== */

val host_source_keccak = impure { _: "host_source_keccak" } : (bits(64), bits(64), bits(64)) -> bits(256)
val host_source_sha256_prefixed = impure { _: "host_source_sha256_prefixed" } : (bits(8), bits(64), bits(64), bits(64)) -> bits(
        256,
    )

function bytes_keccak(s : ByteSlice) -> bits(256) =
    host_source_keccak(byte_source_kind(s.source), s.off, s.len)

function bytes_sha256_prefixed(prefix : byte, s : ByteSlice) -> bits(256) =
    host_source_sha256_prefixed(prefix, byte_source_kind(s.source), s.off, s.len)
