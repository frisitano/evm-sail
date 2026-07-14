/*md
# The public output

The guest's public output: the `hash_tree_root` of the new-payload
request, the validation verdict, and the echoed chain configuration.
*/

/*! Writes the output prefix: the request root (32 bytes big-endian), the
    verdict byte, and the SSZ framing of the echoed configuration. */
function write_result_prefix(root : hash, success : bool) -> unit = {
    foreach (i from 0 to 31) {
        public_output_write(sail_shiftright(root, 8 * (31 - i))[7 .. 0])
    };
    public_output_write(if success then { 0x01 } else { 0x00 });
    public_output_write(0x25);
    public_output_write(0x00);
    public_output_write(0x00);
    public_output_write(0x00)
}

/*! Emits the full public output for a decoded input: the request root
    computed from the input itself, the verdict, and the input's chain
    configuration echoed byte for byte. */
function write_validation_result(input_ref : StatelessInputRef, success : bool) -> unit = {
    cycle_scope_start(SCOPE_COMPUTE_OUTPUT_ROOT);
    let root = htr_new_payload_request(input_ref);
    cycle_scope_end(SCOPE_COMPUTE_OUTPUT_ROOT);

    cycle_scope_start(SCOPE_SERIALIZE_OUTPUT);
    write_result_prefix(root, success);
    let start = unsigned(input_ref.chain_config.off);
    let len = unsigned(input_ref.chain_config.len);
    var i : int = 0;
    while i < len do {
        public_output_write(ssz_src_byte(start + i));
        i = i + 1
    };
    cycle_scope_end(SCOPE_SERIALIZE_OUTPUT)
}

/*! Emits the failure output for an undecodable input: a zero root, a
    false verdict, and a default configuration frame. */
function write_invalid_result() -> unit = {
    cycle_scope_start(SCOPE_SERIALIZE_OUTPUT);
    write_result_prefix(0x0000000000000000000000000000000000000000000000000000000000000000, false);
    foreach (i from 0 to 35) {
        let b : byte = match i {
            8 => 0x0c,
            20 => 0x10,
            24 => 0x18,
            28 => 0x08,
            32 => 0x08,
            _ => 0x00,
        };
        public_output_write(b)
    };
    cycle_scope_end(SCOPE_SERIALIZE_OUTPUT)
}
