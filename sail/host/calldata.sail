/* ======================================================================== */
/* CALLDATA + TRANSACTION INPUT                                             */
/*                                                                          */
/* Two byte sources the evm layer reads but never mutates. CALLDATA (I_d):  */
/* the current frame's input data, bound per call (a caller-memory slice,   */
/* the empty string, or the whole tx input) and served to CALLDATALOAD /    */
/* SIZE / COPY. The TX INPUT: the transaction's data field, staged into C   */
/* ONCE per transaction; intrinsic-gas inspection (EIP-2028 / EIP-7623 zero */
/* vs nonzero counting), tx-level precompile calls, and the pre-state       */
/* decode read it byte-addressed. Bytes live in C (ffi/code_db.c            */
/* frame-buffer machinery); proof targets see bodyless axioms over          */
/* abstract byte-list models.                                               */
/* ======================================================================== */

val cd_set = impure { c: "cd_set" } : (bits(64), bits(64)) -> unit
val cd_set_empty = impure { c: "cd_set_empty" } : unit -> unit
val cd_set_tx = impure { c: "cd_set_tx" } : unit -> unit

/* Byte-loop-free tx-input staging (each done C-side in one call).
 * txdata_set_source stages a byte source (the witness span / native-runner
 * input) and txdata_set_word a 32-byte word (a block system call); each returns
 * the staged length. txd_count_nonzero counts nonzero staged bytes for the
 * EIP-2028 / EIP-7623 calldata gas. */
val txin_set_from_source = impure { c: "txin_set_from_source" } : (bits(64), bits(64), bits(64)) -> bits(64)
function txdata_set_source(s : ByteSlice) -> int =
    unsigned(txin_set_from_source(byte_source_kind(s.source), s.off, s.len))
val txin_set_word = impure { c: "txin_set_word" } : word -> bits(64)
function txdata_set_word(w : word) -> int = unsigned(txin_set_word(w))
val txd_count_nonzero = impure { c: "txd_count_nonzero" } : unit -> bits(64)

/* Streamed tx-input staging: retained ONLY for the native-runner pre-state
 * decode path (a scalar at a time); the stateless guest stages the span in one
 * memcpy. */
val txin_begin = impure { c: "txin_begin" } : unit -> unit
val txin_byte = impure { c: "txin_byte" } : bits(8) -> unit
val cd_to_mem = impure { c: "cd_to_mem" } : (bits(64), bits(64), bits(64)) -> unit

/* CALLDATALOAD: the 32-byte word at calldata offset i (zero-padded past end). */
val calldata_load_word = impure { c: "calldata_load_word" } : bits(64) -> word
val txd_at = impure { c: "txd_at" } : bits(64) -> bits(8)
val txd_length = impure { c: "txd_length" } : unit -> bits(64)

/* The transaction input, streamed into the C buffer ONCE per tx. Intrinsic-gas
 * inspection (EIP-2028 / EIP-7623 zero vs nonzero byte counting) and length
 * checks read it here, byte-addressed, rather than re-walking a Sail list. */
function txdata_byte(i : int) -> byte = txd_at(get_slice_int(64, i, 0))
function txdata_size() -> int = unsigned(txd_length())

function run_precompile_tx_input(num : int) -> bool =
    run_precompile_slice(num, tx_input_byte_slice(0, txdata_size()))
function txdata_read_range(off : int, len : int) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to (len - 1)) {
        out = txdata_byte(off + (len - 1 - k)) :: out
    };
    out
}

/* clamp a beyond-2^64 source offset to a beyond-any-calldata sentinel (as
 * calldata_copy_to_mem does): the 64-bit truncation below would otherwise wrap a
 * huge offset back into real calldata. A past-end CALLDATALOAD reads all zeros. */
function calldata_word(off : int) -> word =
    calldata_load_word(get_slice_int(64, if off > 16777216 then { 16777216 } else { off }, 0))

/* clamp a beyond-2^64 source offset to a beyond-any-calldata sentinel so the
 * 64-bit truncation cannot wrap back into real calldata: a past-end source
 * offset must zero-fill the whole destination (EELS buffer_read). */
function calldata_copy_to_mem(dst : int, off : int, len : int) -> unit = {
    let o = if off > 16777216 then { 16777216 } else { off };
    cd_to_mem(get_slice_int(64, dst, 0), get_slice_int(64, o, 0), get_slice_int(64, len, 0))
}
function calldata_set_next(off : int, len : int) -> unit =
    cd_set(get_slice_int(64, off, 0), get_slice_int(64, len, 0))
