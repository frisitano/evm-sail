/* ======================================================================== */
/* SINGLE-BLOCK / EEST RUNNER  (SSZ stateless input)                        */
/*                                                                          */
/* The native entry point that runs one EEST state-test through the model.  */
/* A test harness compiles this model ONCE, then feeds it one fixture per   */
/* invocation as an SSZ SszStatelessInput (the SAME structure the zkVM      */
/* guest consumes -- see host/block.sail). The runner decodes that input    */
/* through the shared decoders and runs the transaction(s). The data is     */
/* INPUT, not generated code -- there is no per-fixture recompilation.      */
/*                                                                          */
/* Unlike the guest (main.sail), the runner does NOT perform full-block     */
/* validation (block hash / receipts root / BAL / fork==Amsterdam). A state */
/* test is pure transaction execution against a pre-state: no block-start   */
/* system calls, no withdrawals, no block reward. So the runner reuses the  */
/* fork-agnostic SSZ *decode* path but runs each tx under the fork selected */
/* by chain_config (k_fork below), which is why it stays cross-fork while   */
/* the guest's validator is Amsterdam-locked.                               */
/*                                                                          */
/* Input contract: set_input() hands the model one SszStatelessInput; the   */
/* pre-state is the authenticated witness (execution_witness state nodes +  */
/* codes) plus a single DUMMY parent header whose state_root carries the    */
/* pre-state root -- the clean way to feed parent_state_root() the anchor.  */
/*                                                                          */
/* Output contract: NO byte stream and NO result plumbing in the model. The */
/* runner only accumulates gas into the shared zkvm_out_gas register; the   */
/* post-state root is computed C-SIDE by the harness dump calling the       */
/* model's own compute_state_root (test_utils.c evmsail_dump_snapshot, 'G'  */
/* section). A thrown InvalidBlock (malformed tx, deficient witness) is NOT */
/* caught here: it propagates out of main and the harness captures it from  */
/* the generated have_exception flag, reporting the case as an exception    */
/* instead of a bogus partial-state root mismatch.                          */
/* ======================================================================== */


/* Cumulative gas of the run, read C-side by the harness dump (test_utils.c
 * 'G' section). Declared per entry -- main.sail declares the guest's own copy
 * of the same C symbol; the entries are never compiled together. */
register zkvm_out_gas : int = 0

/* Decode one SszStatelessInput and run it: select the fork, authenticate the
 * witness pre-state, decode the header into k_header, run the transaction(s) by
 * SSZ index, and dump the post-state. A state test carries a single tx and no
 * withdrawals, so the shared per-tx STF unit (process_transaction) IS the block
 * -- no block-start system calls / block-end / rewards. */
function run_one() -> unit = {
    /* THE single traversal (host/block.sail decode_stateless_input): geometry,
       witness node/code/header stores, block header, every tx -- decoded once,
       returned as values; also installs k_header / the ancestor-hash table /
       k_parent_state_root. A state test carries a single DUMMY parent header
       whose state_root is the pre-state root (headers len 1, chain vacuously
       contiguous), so BLOCKHASH of the immediate parent resolves from it and
       anything deeper fail-closes to 0 (host/kernel/environment.sail k_blockhash). The
       world starts empty -- the ctypes harness calls evmsail_clear_memory
       before each run -- so there is no overlay wipe here. */
    let input = decode_stateless_input();
    k_fork = input.chain_config.fork;

    /* Run each tx decoded on demand by its SSZ index -- no materialized block. A
       malformed tx / bad signature (or a deficient witness during execution)
       THROWS InvalidBlock, which deliberately propagates uncaught: the harness
       reads the generated have_exception flag after the run and reports the
       case as an exception. Gas is accumulated into the shared register per tx
       so a multi-tx input keeps its progress observable. */
    zkvm_out_gas = 0;
    var rest : list(Transaction) = input.payload.block.body.transactions;
    var more : bool = true;
    while more do {
        match rest {
            [||] => more = false,
            tx :: r => {
                let res = process_transaction(tx);
                zkvm_out_gas = zkvm_out_gas + res.gas_used;
                rest = r
            },
        }
    }
}

/* One SszStatelessInput per invocation: the harness sets the input, runs, then
 * reads the result C-SIDE (snapshot dump: gas register + compute_state_root +
 * have_exception) and resets (clear_memory) before the next case. The
 * recomputed post-state root is the sole pass criterion (a commitment to the
 * FULL post-state, base + overlay). */
function main() -> unit = run_one()
