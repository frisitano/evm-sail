/* ======================================================================== */
/* PRIMITIVES: STATELESS INPUT                                              */
/*                                                                          */
/* The decoded semantic input. Its block body remains source-backed:        */
/* transactions and withdrawals are decoded only when a consumer reaches    */
/* the corresponding SSZ list element.                                      */
/* ======================================================================== */

struct StatelessInput = {
    payload : ExecutionPayload,
    chain_config : ChainConfig,
}

struct WitnessContext = {
    parent_hash : hash,
    parent_state_root : hash,
    parent_base_fee_per_gas : word,
    parent_blob_gas_used : blob_gas,
    parent_excess_blob_gas : blob_gas,
}
