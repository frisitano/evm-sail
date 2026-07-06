/* ======================================================================== */
/* HYBRID BACKEND: SOURCE-TAGGED HASHING                                    */
/*                                                                          */
/* EVM_BACKEND=hybrid shares the source-free crypto core with build         */
/* (build/crypto.sail: the C host-bytes hash channel, fixed-width helpers,  */
/* and the signature precompiles). Only the source-tagged hashes differ:    */
/* under hybrid the Memory/TxInput/ActiveCode buffers are modeled in Sail   */
/* (they are EMPTY in C), so bytes_keccak / bytes_sha256_prefixed           */
/* materialize the bytes from the models (read_byte_slice, model_buffers)   */
/* and hash the list, exactly as spec/crypto.sail does. Source-backed code  */
/* storage is model_buffers.sail; precompile execution is                   */
/* hybrid/precompile.sail.                                                  */
/* ======================================================================== */

function bytes_keccak(s : ByteSlice) -> bits(256) =
    keccak256(read_byte_slice(s))

function bytes_sha256_prefixed(prefix : byte, s : ByteSlice) -> bits(256) =
    sha256(prefix :: read_byte_slice(s))
