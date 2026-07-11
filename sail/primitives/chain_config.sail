/* ======================================================================== */
/* PRIMITIVES: CHAIN CONFIG                                                 */
/*                                                                          */
/* The decoded SszChainConfig: the chain id, the active fork -- BOTH as the */
/* raw SSZ ProtocolFork index (the guest validates it is EXACTLY the        */
/* Amsterdam index) and as the Fork whose execution rules apply (BPO forks  */
/* collapse to their base) -- and whether the fork's activation point has   */
/* been reached. Pure data -- no registers, no externs.                     */
/* ======================================================================== */

struct ChainConfig = {
    chain_id : int,
    fork_index : int, /* raw SSZ ProtocolFork index                     */
    fork : Fork, /* execution rules for that index                 */
    activation_active : bool, /* EELS _is_activation_active for this payload    */
}
