/*md
# The guest entry point

The zkVM guest's `main`: decode the stateless input, verify the payload,
and emit the public validation result. Any thrown `InvalidBlock` — even
during input decoding — resolves to an invalid verdict rather than a
crash: validation of an undecodable input is simply unsuccessful.
*/

/*! A decoded input paired with its validation verdict. */
struct GuestValidation = {
    input_ref : StatelessInputRef,
    valid : bool,
}

/*! Decodes, verifies, and reports: `write_validation_result` on a
    decodable input (valid or not), `write_invalid_result` when decoding
    itself fails. */
function main() -> unit = {
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
    }
}
