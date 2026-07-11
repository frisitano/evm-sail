/* ======================================================================== */
/* PRIMITIVES: STATELESS INPUT                                              */
/*                                                                          */
/* The decoded SszStatelessInput -- what decode_stateless_input             */
/* (host/block.sail) returns: pure input DATA, no verdicts, no decode       */
/* state. Wire-layout constants live with the SSZ reader (lib/ssz.sail).    */
/* Pure data -- no registers, no externs.                                   */
/* ======================================================================== */

/* Everything the single decode pass returns: the decoded block (header,
 * transactions, withdrawals) plus the witness-header results the validation
 * layer reads. The parent header travels as its RLP items so field decoding
 * (and its canonicity throws) stays at the original consumer sites. */
struct StatelessInput = {
    payload : ExecutionPayload,
    chain_config : ChainConfig,
    nodes : list(ByteSlice), /* witness state-node references (node DB source) */
    codes : list(ByteSlice), /* witness code references (code DB source)       */
    headers : list(ByteSlice), /* witness ancestor-header references              */
}
