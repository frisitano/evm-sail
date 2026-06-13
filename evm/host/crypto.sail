/* ======================================================================== */
/* HOST crypto functions: the EVM precompiles and staged-input access        */
/*                                                                           */
/* Crypto is host mechanism, like memory and storage: the model defines      */
/* WHAT is hashed/verified (and all gas); the host computes it. keccak256    */
/* and sha256 are defined where they are used (lib/keccak.sail,              */
/* lib/sha256.sail) over the same binding. The acc_* byte-stream protocol    */
/* itself is confined to host/ffi.sail -- nothing outside the host files     */
/* speaks it.                                                                */
/* ======================================================================== */

/* The i-th byte of the STAGED precompile input, zero past the end. A
 * precompile's input is staged once (a snapshot of the caller's memory slice,
 * or the tx data); gas inspection (modexp header, blake2f rounds) reads the
 * few bytes it needs O(1) -- no Sail byte list. */
function precompile_input_byte(i : int) -> byte = acc_in_byte(get_slice_int(64, i, 0))

/* stage a precompile's input from a byte list (the tx-to-precompile path;
 * CALL-family staging is memory-sourced: stage_precompile_input, memory.sail) */
function stage_precompile_list(num : int, input : list(byte)) -> unit = {
  acc_begin(get_slice_int(64, num, 0));
  acc_push_all(input)
}

/* run the staged precompile (input already in the accelerator: acc_begin_mem
 * for a memory slice, acc_begin+acc_push_all for a list). The output stays in
 * the accelerator buffer for returndata adoption (hr_capture_acc). On failure
 * the CALL returns 0 (handled by the interpreter). */
function run_precompile_staged() -> bool = {
  let _ = acc_exec();
  unsigned(acc_ok()) == 1
}

/* list-in/list-out wrapper (vector tests; the EVM uses the staged path) */
function run_precompile(num : int, input : list(byte)) -> (bool, list(byte)) = {
  acc_begin(get_slice_int(64, num, 0));
  acc_push_all(input);
  let n = unsigned(acc_exec());
  let ok = unsigned(acc_ok()) == 1;
  if not_bool(ok) then (false, [||]) else (true, acc_read(n))
}
