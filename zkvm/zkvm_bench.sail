/* ======================================================================== */
/* World-state scaling micro-benchmark: N storage writes + N reads to distinct */
/* slots, exercising the kernel's list-based storage (st_update/st_lookup are   */
/* O(current size), so N accesses cost O(N^2)). N is read from the private      */
/* input (first 4 bytes LE). Measures whether the world state is O(N^2) before  */
/* committing to the per-tx BAL refactor.  GUEST=zkvm_bench.sail ./build.sh run  */
/* ======================================================================== */

$include "../evm/el_ir.sail"
$include "../examples/evm_primitives.sail"

val ssz_src_le = impure "ssz_src_le" : (int, int) -> bits(64)

register zkvm_out_gas      : int = 0
register zkvm_out_storage0 : int = 0
register zkvm_out_wbal     : int = 0
register zkvm_out_cbbal    : int = 0
register zkvm_out_tracelen : int = 0
register zkvm_out_success  : int = 0

function zkvm_run() -> unit = {
  let n : int = unsigned(ssz_src_le(0, 4));       /* N from the input */
  k_world_reset();
  let a : address = 0x000000000000000000000000000000000000c0de;
  foreach (k from 0 to (n - 1)) { k_sstore(a, word_of_int(k), word_of_int(k + 1)) };
  var sum : int = 0;
  foreach (k from 0 to (n - 1)) { sum = sum + unsigned(k_sload(a, word_of_int(k))) };
  zkvm_out_gas      = sum;
  zkvm_out_storage0 = n;
  zkvm_out_wbal     = 0;
  zkvm_out_cbbal    = 0;
  zkvm_out_success  = 1;
  zkvm_out_tracelen = list_len(ir_trace)
}
