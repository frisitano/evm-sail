/* Build backend: C FFI bindings for the content-addressed code store
 * (ffi/code_db.c). The active frame code view is bound separately
 * (build/frame_code.sail). */

val code_db_stream_code_begin_ffi = impure { _: "code_db_stream_code_begin" } : hash -> bits(64)
function code_db_stream_code_begin(h : hash) -> bits(64) = code_db_stream_code_begin_ffi(h)

val code_db_stream_code_byte_ffi = impure { _: "code_db_stream_code_byte" } : bits(8) -> unit
function code_db_stream_code_byte(b : bits(8)) -> unit = code_db_stream_code_byte_ffi(b)

val code_db_stored_code_length_ffi = impure { _: "code_db_stored_code_length" } : hash -> bits(64)
function code_db_stored_code_length(h : hash) -> bits(64) = code_db_stored_code_length_ffi(h)

val code_db_copy_stored_code_to_memory_ffi = impure { _: "code_db_copy_stored_code_to_memory" } : (
        hash,
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
function code_db_copy_stored_code_to_memory(h : hash, dst : bits(64), off : bits(64), len : bits(64)) -> unit =
    code_db_copy_stored_code_to_memory_ffi(h, dst, off, len)

val code_db_read_delegation_ffi = impure { _: "code_db_read_delegation" } : hash -> bits(168)
function code_db_read_delegation(h : hash) -> bits(168) = code_db_read_delegation_ffi(h)

/* Stateless witness path: store code straight from a byte source span,
 * hashed and bitmapped C-side without materializing a Sail list. */
val code_db_store_source_ffi = impure { _: "code_db_store_source" } : (bits(64), bits(64), bits(64)) -> unit
function code_db_store_slice(s : ByteSlice) -> unit =
    code_db_store_source_ffi(byte_source_kind(s.source), s.off, s.len)

/* Deploy-path interning: one C call hashes + stores the blob from its host byte
 * source (the pending returndata buffer / the C-built designation) and returns
 * the codeHash -- no Sail byte loop. */
val code_intern_returndata_ffi = impure { _: "code_intern_returndata" } : unit -> hash
function code_intern_returndata() -> hash = code_intern_returndata_ffi()

val code_intern_delegation_ffi = impure { _: "code_intern_delegation" } : address -> hash
function code_intern_delegation(a : address) -> hash = code_intern_delegation_ffi(a)
