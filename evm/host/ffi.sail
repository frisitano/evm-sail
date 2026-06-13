/* ======================================================================== */
/* FFI BINDING: the accelerator byte-stream protocol                         */
/*                                                                           */
/* This file is mechanism only -- NO semantics. Sail lists cannot cross the  */
/* C boundary, so byte inputs stream through a small stateful protocol       */
/* (begin / push / exec / read), 8 bytes per crossing. The C side is         */
/* ffi/acc_shim.c; the wire interface follows eth-act/zkvm-standards         */
/* c-interface-accelerators (one header, ffi/zkvm_accelerators.h, backed by  */
/* industry libraries on the host and by native precompiles in a zkVM        */
/* guest). Semantic crypto functions over this binding live in               */
/* host/crypto.sail; memory-sourced staging (no list, one C-side copy) in    */
/* host/memory.sail. ids: 0=keccak256, 2=sha256, 3=ripemd160, 4=identity,    */
/* 1/5..17/256 = EVM precompiles.                                            */
/* ======================================================================== */

val acc_begin = impure "acc_begin" : bits(64) -> unit
val acc_push  = impure "acc_push"  : bits(8)  -> unit
val acc_push8 = impure "acc_push8" : bits(64) -> unit
val acc_exec  = impure "acc_exec"  : unit -> bits(64)
val acc_ok    = impure "acc_ok"    : unit -> bits(8)
val acc_out   = impure "acc_out"   : bits(64) -> bits(8)
val acc_word  = impure "acc_word"  : bits(64) -> bits(64)
val acc_in_byte   = impure "acc_in_byte"   : bits(64) -> bits(8)
val acc_begin_mem = impure "acc_begin_mem" : (bits(64), bits(64), bits(64)) -> unit
val acc_begin_txd = impure "acc_begin_txd" : bits(64) -> unit

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
