/* Optimized C overrides for `sail/kernel/logs.sail`. */

/* Retained log bytes already live in a stable block-lifetime arena. Cross the
 * host boundary as one pointer-bearing slice rather than reconstructing it
 * from an integer arena coordinate in generated C. */
val optimized_log_data_slice = impure { c: "host_log_data_slice" } : log_store_index -> LogDataSlice

function read_log_data(index : log_store_index) -> LogDataSlice =
    optimized_log_data_slice(index)
