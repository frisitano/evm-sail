/* ======================================================================== */
/* WORLD-STATE THROUGHPUT BENCHMARK (zkVM)                                  */
/*                                                                          */
/* A throughput micro-benchmark for the zkVM guest, not a validator: it does */
/* N storage writes followed by N reads to distinct slots of one account,   */
/* measuring how the kernel's world-state storage scales. N is read from the */
/* private input (first 4 bytes, little-endian).                            */
/*                                                                          */
/* The list-based storage (st_update / st_lookup are O(current size)) makes  */
/* N accesses cost O(N^2); this run quantifies that before any per-tx        */
/* block-access-list (BAL) refactor. Drive it with                          */
/* GUEST=zkvm_bench.sail ./build.sh run. Shares the zkvm_out_* register      */
/* output convention with zkvm_block.sail.                                  */
/* ======================================================================== */

$include "../evm/evm.sail"

val ssz_src_le = impure { _: "ssz_src_le" } : (int, int) -> bits(64)

register zkvm_out_gas : int = 0
register zkvm_out_storage0 : int = 0
register zkvm_out_wbal : int = 0
register zkvm_out_cbbal : int = 0
register zkvm_out_tracelen : int = 0
register zkvm_out_success : int = 0

function zkvm_run() -> unit = {
    let n : int = unsigned(ssz_src_le(0, 4)); /* N: first 4 input bytes, LE */
    k_world_reset();
    let a : address = 0x000000000000000000000000000000000000c0de;
    foreach (k from 0 to (n - 1)) {
        k_sstore(a, word_of_int(k), word_of_int(k + 1))
    };
    var sum : int = 0;
    foreach (k from 0 to (n - 1)) {
        sum = sum + unsigned(k_sload(a, word_of_int(k)))
    };
    zkvm_out_gas = sum;
    zkvm_out_storage0 = n;
    zkvm_out_wbal = 0;
    zkvm_out_cbbal = 0;
    zkvm_out_success = 1;
    zkvm_out_tracelen = 0 /* the ELOp trace machinery was removed */
}
