/* ======================================================================== */
/* CALLDATA + TRANSACTION INPUT                                             */
/*                                                                          */
/* Two byte sources the evm layer reads but never mutates. CALLDATA (I_d):  */
/* the current frame's input data, bound per call (a caller-memory slice,   */
/* the empty string, or the whole tx input) and served to CALLDATALOAD /    */
/* SIZE / COPY. The TX INPUT: a REFERENCE to the transaction's data span    */
/* (Transaction.input_src, usually the witness), bound once per tx and      */
/* never copied; block system calls stage their 32-byte word instead.       */
/* Reads resolve through the reference (ffi/memory.c); proof targets see    */
/* bodyless axioms over abstract byte-list models.                          */
/* ======================================================================== */

val calldata_bind_memory = impure { c: "calldata_bind_memory" } : (bits(64), bits(64)) -> unit
val calldata_bind_empty = impure { c: "calldata_bind_empty" } : unit -> unit
val calldata_bind_tx_input = impure { c: "calldata_bind_tx_input" } : unit -> unit

/* txdata_set_source BINDS the tx input by reference (no copy; returns the
 * bound length, fail-closed 0 on a bad source); txdata_set_word stages a
 * 32-byte word (a block system call). txdata_count_nonzero counts nonzero
 * input bytes for the EIP-2028 / EIP-7623 calldata gas. */
val txdata_bind_source = impure { c: "txdata_bind_source" } : (bits(64), bits(64), bits(64)) -> bits(64)
function txdata_set_source(s : ByteSlice) -> int =
    unsigned(txdata_bind_source(byte_source_kind(s.source), s.off, s.len))
val txdata_stage_word = impure { c: "txdata_stage_word" } : word -> bits(64)
function txdata_set_word(w : word) -> int = unsigned(txdata_stage_word(w))
val txdata_count_nonzero = impure { c: "txdata_count_nonzero" } : unit -> bits(64)

val calldata_copy_to_memory = impure { c: "calldata_copy_to_memory" } : (bits(64), bits(64), bits(64)) -> unit

/* CALLDATALOAD: the 32-byte word at calldata offset i (zero-padded past end). */
val calldata_load_word = impure { c: "calldata_load_word" } : bits(64) -> word
val txdata_byte_at = impure { c: "txdata_byte_at" } : bits(64) -> bits(8)
val txdata_length = impure { c: "txdata_length" } : unit -> bits(64)

/* Intrinsic-gas inspection (EIP-2028 / EIP-7623 zero vs nonzero counting)
 * and length checks read the referenced input byte-addressed. */
function txdata_byte(i : int) -> byte = txdata_byte_at(get_slice_int(64, i, 0))
function txdata_size() -> int = unsigned(txdata_length())

function run_precompile_tx_input(num : int) -> bool =
    run_precompile_slice(num, tx_input_byte_slice(0, txdata_size()))

/* A past-end CALLDATALOAD reads all zeros; a past-end copy source zero-fills
 * the whole destination (EELS buffer_read). Source offsets pass through
 * clamp_source_offset (host/memory.sail) so the 64-bit truncation cannot
 * wrap a huge offset back into real data. */
function calldata_word(off : int) -> word =
    calldata_load_word(get_slice_int(64, clamp_source_offset(off), 0))

function calldata_copy_to_mem(dst : int, off : int, len : int) -> unit =
    calldata_copy_to_memory(
        get_slice_int(64, dst, 0),
        get_slice_int(64, clamp_source_offset(off), 0),
        get_slice_int(64, len, 0),
    )

function calldata_set_next(off : int, len : int) -> unit =
    calldata_bind_memory(get_slice_int(64, off, 0), get_slice_int(64, len, 0))
