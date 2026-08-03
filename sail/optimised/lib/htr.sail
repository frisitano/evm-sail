/* Optimized C overrides for `sail/lib/htr.sail`. */

val optimized_htr_new_payload_request = pure { c: "htr_new_payload_request" } : StatelessInputRef -> hash

function htr_new_payload_request(input_ref : StatelessInputRef) -> hash =
    optimized_htr_new_payload_request(input_ref)
