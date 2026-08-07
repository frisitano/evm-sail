/* Optimized C overrides for `sail/lib/htr.sail`. */

/* Keep an explicit whole-operation boundary here: replacing the canonical
 * recursive body directly makes C representation specialization enter every
 * HTR helper before the splice is applied. */
val htr_new_payload_request_from_input = pure { c: "htr_new_payload_request" } : StatelessInputRef -> hash

function htr_new_payload_request(input_ref : StatelessInputRef) -> hash =
    htr_new_payload_request_from_input(input_ref)
