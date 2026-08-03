/* Optimized C overrides for `sail/lib/ssz/stateless_input.sail`. */

function index_execution_witness(input_ref : StatelessInputRef) -> WitnessContext = {
    nodedb_reset();
    cycle_scope_start(38);
    index_witness_nodes(input_ref.witness_state);
    cycle_scope_end(38);
    cycle_scope_start(39);
    index_witness_codes(input_ref.witness_codes);
    cycle_scope_end(39);
    cycle_scope_start(40);
    let witness = index_witness_headers(input_ref.witness_headers);
    cycle_scope_end(40);
    k_parent_state_root = witness.parent_state_root;
    witness
}
