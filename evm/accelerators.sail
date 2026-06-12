/* ======================================================================== */
/* zkVM crypto accelerators (eth-act zkvm-standards c-interface-accelerators)  */
/*                                                                           */
/* ONE host boundary for all crypto -- keccak256, sha256, ripemd160, and the   */
/* EVM precompiles -- aligning with eth-act/zkvm-standards. The model streams   */
/* input bytes through a thin C shim (ffi/acc_shim.c) that calls the one-shot   */
/* standard functions in ffi/zkvm_accelerators.h; a real zkVM swaps the         */
/* reference impl (ffi/zkvm_accelerators.c) for native precompiles behind that  */
/* same header. Only mach_bits cross the FFI. ids: 0=keccak256, 2=sha256,       */
/* 3=ripemd160, 4=identity, 1/5..10 = EVM precompiles. Gas stays in Sail.       */
/* ======================================================================== */

val acc_begin = impure "acc_begin" : bits(64) -> unit
val acc_push  = impure "acc_push"  : bits(8)  -> unit
val acc_push8 = impure "acc_push8" : bits(64) -> unit
val acc_exec  = impure "acc_exec"  : unit -> bits(64)
val acc_ok    = impure "acc_ok"    : unit -> bits(8)
val acc_out   = impure "acc_out"   : bits(64) -> bits(8)
val acc_word  = impure "acc_word"  : bits(64) -> bits(64)

/* feed the input to the host, 8 bytes per FFI crossing (remainder one-by-one) */
function acc_push_all(msg : list(byte)) -> unit = match msg {
  b0 :: b1 :: b2 :: b3 :: b4 :: b5 :: b6 :: b7 :: r => {
    acc_push8(append(b0, append(b1, append(b2, append(b3, append(b4, append(b5, append(b6, b7))))))));
    acc_push_all(r)
  },
  b :: r => { acc_push(b); acc_push_all(r) },
  [||]   => ()
}

/* read `n` output bytes back from the host (O(n): prepend + reverse) */
function acc_read(n : int) -> list(byte) = {
  var out : list(byte) = [||];
  foreach (i from 0 to (n - 1)) { out = acc_out(get_slice_int(64, i, 0)) :: out };
  list_reverse(out)
}

/* a 32-byte-digest accelerator (keccak256 id 0, sha256 id 2): hash `msg` -> word.
 * The 32 output bytes are folded big-endian into bits(256). */
function acc_hash(id : int, msg : list(byte)) -> bits(256) = {
  acc_begin(get_slice_int(64, id, 0));
  acc_push_all(msg);
  let _ = acc_exec();
  let w0 : bits(64) = acc_word(get_slice_int(64, 0, 0));   /* 4 big-endian word reads */
  let w1 : bits(64) = acc_word(get_slice_int(64, 1, 0));
  let w2 : bits(64) = acc_word(get_slice_int(64, 2, 0));
  let w3 : bits(64) = acc_word(get_slice_int(64, 3, 0));
  append(w0, append(w1, append(w2, w3)))
}

/* run EVM precompile `num` over `input`; (success, output). On failure the CALL
 * returns 0 (handled by the interpreter). */
function run_precompile(num : int, input : list(byte)) -> (bool, list(byte)) = {
  acc_begin(get_slice_int(64, num, 0));
  acc_push_all(input);
  let n = unsigned(acc_exec());
  let ok = unsigned(acc_ok()) == 1;
  if not_bool(ok) then (false, [||]) else (true, acc_read(n))
}
