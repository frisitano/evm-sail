/*md
# The guest entry point

The zkVM guest's `main`: decode the stateless input, verify the payload,
and emit the public validation result. Any thrown `InvalidBlock` — even
during input decoding — resolves to an invalid verdict rather than a
crash, mirroring the EELS `verify_stateless_new_payload` try/except
structure.
*/

/*! A decoded input paired with its validation verdict. */
struct GuestValidation = {
    input_ref : StatelessInputRef,
    valid : bool,
}

/*! Decodes, verifies, and reports: `write_validation_result` on a
    decodable input (valid or not), `write_invalid_result` when decoding
    itself fails.

    *EELS*: `verify_stateless_new_payload`. */
function main() -> unit = {
    validation_debug_reset();
    cycle_scope_start(SCOPE_STATELESS_VALIDATION);
    let result : option(GuestValidation) = try {
        let input_ref = decode_stateless_input_ref();
        let validation : StatelessValidationResult = verify_stateless_payload(input_ref);
        let valid : bool = match validation {
            StatelessPayloadValid() => true,
            StatelessPayloadInvalid(failure) => {
                validation_debug_record(failure.scope, failure.reason);
                false
            },
        };
        Some(struct { input_ref = input_ref, valid = valid })
    } catch {
        InvalidBlock(reason) => {
            validation_debug_record(SCOPE_DECODE_INPUT, reason);
            None()
        },
    };
    cycle_scope_end(SCOPE_STATELESS_VALIDATION);
    match result {
        Some(result) => write_validation_result(result.input_ref, result.valid),
        None() => write_invalid_result(),
    }
}
