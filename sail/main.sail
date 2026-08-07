/*md
# The guest entry point

The zkVM guest's `main`: decode the stateless input, verify the payload,
and emit the public validation result. Fatal failures publish the invalid
result and terminate inside `fatal_error`; only success returns here.
*/

/*! Retains the decoded request for terminal failure output. This is a no-op in
    semantic extraction and a small host-context update in C. */
val fatal_error_set_input = impure { c: "fatal_error_set_input" } : StatelessInputRef -> unit

function fatal_error_set_input(_input_ref) = ()

/*! Decodes, verifies, and reports the successful validation result. */
function main() -> unit = {
    scratch_reset();
    validation_debug_reset();
    let input_bytes = stateless_input();
    let input_ref = decode_stateless_input_ref(input_bytes);
    fatal_error_set_input(input_ref);
    verify_stateless_payload(input_ref);
    write_validation_result(input_ref, true)
}
