/* Optimized C overrides for `sail/main.sail`. */

function main() -> unit = {
    cycle_scope_start(0);
    scratch_reset();
    validation_debug_reset();
    let result : option(GuestValidation) = try {
        let input_ref = decode_stateless_input_ref(stateless_input());
        let validation : StatelessValidationResult = verify_stateless_payload(input_ref);
        let valid : bool = match validation {
            StatelessPayloadValid() => true,
            StatelessPayloadInvalid(failure) => {
                validation_debug_record(failure.stage, failure.reason);
                false
            },
        };
        Some(struct { input_ref = input_ref, valid = valid })
    } catch {
        InvalidBlock(reason) => {
            validation_debug_record(VALIDATION_STAGE_DECODE_INPUT, reason);
            None()
        },
        Overflow() => None(),
    };
    match result {
        Some(result) => write_validation_result(result.input_ref, result.valid),
        None() => write_invalid_result(),
    };
    cycle_scope_end(0)
}
