/* ======================================================================== */
/* EL-IR memory: byte-addressed, zero-initialized, lazily expanding          */
/*                                                                           */
/* Real EVM memory semantics: a flat byte array addressed from 0, growing in */
/* 32-byte words. Sail forbids recursive types (no in-model balanced tree)    */
/* and a list-backed memory is O(n) per access -- O(n^2) over a fill/copy     */
/* loop. So, like sail-riscv, memory lives in C (ffi/el_mem.c): a flat buffer  */
/* per call frame with O(1) read/write. Frames form a stack -- a sub-call gets */
/* fresh memory (mem_frame_enter) and the parent's is restored on return      */
/* (mem_frame_leave); mem_clear resets to a single empty frame per tx. The     */
/* Sail side keeps only the high-water mark (memory_size) for expansion gas.  */
/* ======================================================================== */

val el_mem_read  = impure "el_mem_read"  : bits(64) -> bits(8)
val el_mem_write = impure "el_mem_write" : (bits(64), bits(8)) -> unit
val el_mem_reset = impure "el_mem_reset" : unit -> unit
val el_mem_push  = impure "el_mem_push"  : unit -> unit
val el_mem_pop   = impure "el_mem_pop"   : unit -> unit

/* kept only as the type of the (now unused) StackFrame snapshot field */
struct MemByte = { mb_off : int, mb_val : byte }

register memory_size : int = 0   /* high-water mark, in bytes */

/* call-frame memory lifecycle (the C side owns the bytes) */
function mem_clear()       -> unit = el_mem_reset()
function mem_frame_enter() -> unit = el_mem_push()
function mem_frame_leave() -> unit = el_mem_pop()

/* --- raw byte access (O(1), C-backed) --- */

function mem_get_byte(off : int) -> byte = el_mem_read(get_slice_int(64, off, 0))

function mem_set_byte(off : int, v : byte) -> unit = {
  el_mem_write(get_slice_int(64, off, 0), v);
  if off + 1 > memory_size then memory_size = off + 1 else ()
}

/* --- word access (big-endian, 32 bytes) --- */

function mem_load(off : int) -> word = {
  var acc : word = word_of_int(0);
  foreach (k from 0 to 31) {
    acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(mem_get_byte(off + k), 256))
  };
  acc
}

function mem_store(off : int, w : word) -> unit = {
  foreach (k from 0 to 31) {
    let shift = 8 * (31 - k);
    let b : byte = (sail_shiftright(w, shift))[7 .. 0];
    mem_set_byte(off + k, b)
  }
}

function mem_store_byte(off : int, w : word) -> unit =
  mem_set_byte(off, w[7 .. 0])

/* --- byte-range read (for KECCAK / RETURN / LOG / CALL args), as a list --- */

function mem_read_range(off : int, len : int) -> list(byte) = {
  var out : list(byte) = [||];                      /* O(n): prepend then reverse */
  foreach (k from 0 to (len - 1)) {
    out = mem_get_byte(off + (len - 1 - k)) :: out
  };
  out
}

/* --- byte-range write (for CALLDATACOPY / CODECOPY / RETURNDATACOPY) --- */

function mem_write_range(off : int, src : list(byte)) -> unit = {
  /* consume src once (O(n)); list_nth-per-index would be O(n^2) on large copies */
  var rest : list(byte) = src;
  var k : int = 0;
  var more : bool = true;
  while more do {
    match rest { b :: t => { mem_set_byte(off + k, b); rest = t; k = k + 1 }, [||] => more = false }
  }
}

/* Current memory size in 32-byte words (rounded up). */
function mem_size_words() -> int = ediv_int(memory_size + 31, 32)
