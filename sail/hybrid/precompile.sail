/* ======================================================================== */
/* HYBRID BACKEND: PRECOMPILE EXECUTION                                     */
/*                                                                          */
/* Precompile execution has no source-free C entry point (the C runner      */
/* snapshots source bytes and writes pending returndata internally), and    */
/* under hybrid those source buffers are EMPTY -- they are modeled in Sail. */
/* So run_precompile_memory_input / run_precompile_tx_input are stubbed to  */
/* plain failure: a precompile CALL under hybrid behaves like a failed call.*/
/* Fixtures exercised under hybrid must not execute precompiles by address  */
/* (sender authentication is fine: it uses the direct secp256k1/ecrecover   */
/* externs in hybrid/crypto.sail, which carry no byte source).              */
/* ======================================================================== */

function run_precompile_memory_input(_ : int, _ : int, _ : int) -> bool = false

function run_precompile_tx_input(_ : int) -> bool = false
