/* Optimized C validation boundary for `sail/main.sail`. The optimized SSZ
 * decoder retains the decoded input for `fatal_error`. */
function main() -> unit = {
    scratch_reset();
    validation_debug_reset();
    let input_ref = decode_stateless_input_ref(stateless_input());
    fatal_error_set_input(input_ref);
    verify_stateless_payload(input_ref);
    write_validation_result(input_ref, true)
}
