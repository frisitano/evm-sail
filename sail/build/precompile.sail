/* ======================================================================== */
/* BUILD BACKEND: PRECOMPILE EXECUTION                                      */
/*                                                                          */
/* Runs a precompile over a pointer-backed input source without             */
/* materializing a Sail byte list: the C runner snapshots the source span   */
/* (memory or top-level tx data) and writes successful output directly into */
/* pending returndata. Refines run_precompile(id, read_byte_slice(source)). */
/* ======================================================================== */

val precompile_run_source_to_returndata_ffi = impure { _: "precompile_run_source_to_returndata" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bool

function run_precompile_slice(num : int, s : ByteSlice) -> bool =
    precompile_run_source_to_returndata_ffi(get_slice_int(64, num, 0), byte_source_kind(s.source), s.off, s.len)

function run_precompile_memory_input(num : int, off : int, len : int) -> bool =
    run_precompile_slice(num, memory_byte_slice(off, len))

function run_precompile_tx_input(num : int) -> bool =
    run_precompile_slice(num, tx_input_byte_slice(0, txdata_size()))
