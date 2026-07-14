/*md
# Stateless input

The decoded semantic input. Its block body remains source-backed:
transactions and withdrawals are decoded only when a consumer reaches the
corresponding SSZ list element.
*/

/*! The guest's decoded input: the execution payload and the chain
    configuration it executes under. */
struct StatelessInput = {
    payload : ExecutionPayload,
    chain_config : ChainConfig,
}

/*! Authenticated facts about the parent block that validation rules
    compare the payload header against. */
struct WitnessContext = {
    parent_hash : hash,
    parent_state_root : hash,
    parent_base_fee_per_gas : int,
    parent_blob_gas_used : int,
    parent_excess_blob_gas : int,
}
