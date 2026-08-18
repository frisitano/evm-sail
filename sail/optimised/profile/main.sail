/* Optimized C overrides for `sail/main.sail`. */

function main() -> unit = {
    cycle_scope_start(0);
    scratch_reset();
    validation_debug_reset();
    let input_bytes = stateless_input();
    let input_ref = decode_stateless_input_ref(input_bytes);
    fatal_error_set_input(input_ref);
    verify_stateless_payload(input_ref);
    write_validation_result(input_ref, true);
    cycle_scope_end(0)
}
