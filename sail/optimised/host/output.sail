/* Optimized C overrides for `sail/host/output.sail`. */

/*
 * Region-backed slice offsets are absolute validated pointers in optimized C.
 * The standard model retains offsets. Sub-slicing remains ordinary addition,
 * while the host resolver checks that the resulting span belongs to its
 * nominal backing region.
 */
val nonempty_output_buffer_slice = impure { c: "output_buffer_slice" } : forall 'len, source_valid_length('len). int(
        'len,
    ) -> OutputSlice

function output_buffer_slice(len) =
    if len == 0 then {
        EMPTY_OUTPUT_SLICE
    } else {
        nonempty_output_buffer_slice(len)
    }
