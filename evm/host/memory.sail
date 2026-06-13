/* ======================================================================== */
/* Per-frame EVM memory: byte-addressed, zero-initialized, lazily expanding          */
/*                                                                           */
/* Real EVM memory semantics: a flat byte array addressed from 0, growing in */
/* 32-byte words. Sail forbids recursive types (no in-model balanced tree)    */
/* and a list-backed memory is O(n) per access -- O(n^2) over a fill/copy     */
/* loop. So, like sail-riscv, memory lives in C (ffi/host_mem.c): a flat buffer  */
/* per call frame with O(1) read/write. Frames form a stack -- a sub-call gets */
/* fresh memory (mem_frame_enter) and the parent's is restored on return      */
/* (mem_frame_leave); mem_clear resets to a single empty frame per tx. The     */
/* Sail side keeps only the high-water mark (memory_size) for expansion gas.  */
/* ======================================================================== */

val host_mem_read  = impure "host_mem_read"  : bits(64) -> bits(8)
val host_mem_write = impure "host_mem_write" : (bits(64), bits(8)) -> unit
val host_mem_reset = impure "host_mem_reset" : unit -> unit
val host_mem_push  = impure "host_mem_push"  : unit -> unit
val host_mem_pop   = impure "host_mem_pop"   : unit -> unit

/* kept only as the type of the (now unused) StackFrame snapshot field */
struct MemByte = { mb_off : int, mb_val : byte }

register memory_size : int = 0   /* high-water mark, in bytes */

/* call-frame memory lifecycle (the C side owns the bytes) */
function mem_clear()       -> unit = host_mem_reset()
function mem_frame_enter() -> unit = host_mem_push()
function mem_frame_leave() -> unit = host_mem_pop()

/* --- raw byte access (O(1), C-backed) --- */

function mem_get_byte(off : int) -> byte = host_mem_read(get_slice_int(64, off, 0))

function mem_set_byte(off : int, v : byte) -> unit = {
  host_mem_write(get_slice_int(64, off, 0), v);
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

/* Current memory size in 32-byte words (rounded up). */
function mem_size_words() -> int = ediv_int(memory_size + 31, 32)

/* ---- range-based operations (zero/one-copy: both sides live in C) ---- */
val hm_move   = impure "hm_move"   : (bits(64), bits(64), bits(64)) -> unit
val hc_to_mem = impure "hc_to_mem" : (bits(64), bits(64), bits(64)) -> unit
/* stage a precompile's input: ONE C-side snapshot of mem[off .. off+len) (the
 * child message's calldata). A snapshot, not an alias: the identity precompile's
 * output may be written back over a region overlapping its input. */
function stage_precompile_input(id : int, off : int, len : int) -> unit =
  acc_begin_mem(get_slice_int(64, id, 0), get_slice_int(64, off, 0), get_slice_int(64, len, 0))

/* MCOPY: overlapping-safe memory-to-memory copy (one C memmove) */
function mem_mcopy(dst : int, src : int, len : int) -> unit =
  hm_move(get_slice_int(64, dst, 0), get_slice_int(64, src, 0), get_slice_int(64, len, 0))

/* CODECOPY: the current frame's code (already in the C buffer) into memory */
function mem_codecopy(dst : int, off : int, len : int) -> unit =
  hc_to_mem(get_slice_int(64, dst, 0), get_slice_int(64, off, 0), get_slice_int(64, len, 0))

/* KECCAK-256 directly over the memory range [off, off+len): the bytes go from
 * the C memory frame into the C accelerator without an intermediate list */
function mem_keccak(off : int, len : int) -> word = {
  acc_begin_mem(get_slice_int(64, 0, 0), get_slice_int(64, off, 0), get_slice_int(64, len, 0));
  let _ = acc_exec();
  let w0 : bits(64) = acc_word(get_slice_int(64, 0, 0));
  let w1 : bits(64) = acc_word(get_slice_int(64, 1, 0));
  let w2 : bits(64) = acc_word(get_slice_int(64, 2, 0));
  let w3 : bits(64) = acc_word(get_slice_int(64, 3, 0));
  append(w0, append(w1, append(w2, w3)))
}

/* ---- C-backed RETURNDATA (per call frame; see ffi/acc_shim.c) ---- */
val hr_reset        = impure "hr_reset"        : unit -> unit
val hr_clear        = impure "hr_clear"        : unit -> unit
val hr_discard      = impure "hr_discard"      : unit -> unit
val hr_capture      = impure "hr_capture"      : (bits(64), bits(64)) -> unit
val hr_capture_acc  = impure "hr_capture_acc"  : unit -> unit
val hr_adopt        = impure "hr_adopt"        : unit -> unit
val hr_len          = impure "hr_len"          : unit -> bits(64)
val hr_pending_len  = impure "hr_pending_len"  : unit -> bits(64)
val hr_pending_byte = impure "hr_pending_byte" : bits(64) -> bits(8)
val hr_to_mem       = impure "hr_to_mem"       : (bits(64), bits(64), bits(64)) -> unit
val hr_out_region   = impure "hr_out_region"   : (bits(64), bits(64)) -> unit

function returndata_size() -> int = unsigned(hr_len())
function returndata_capture(off : int, len : int) -> unit =
  hr_capture(get_slice_int(64, off, 0), get_slice_int(64, len, 0))
function returndata_copy_to_mem(dst : int, off : int, len : int) -> unit =
  hr_to_mem(get_slice_int(64, dst, 0), get_slice_int(64, off, 0), get_slice_int(64, len, 0))
function returndata_out_region(dst : int, want : int) -> unit =
  hr_out_region(get_slice_int(64, dst, 0), get_slice_int(64, want, 0))

/* materialize the pending child output (a CREATE's deployed code) as a list */
function pending_output_list() -> list(byte) = {
  let n = unsigned(hr_pending_len());
  var out : list(byte) = [||];
  foreach (i from 0 to (n - 1)) {
    out = hr_pending_byte(get_slice_int(64, n - 1 - i, 0)) :: out
  };
  out
}

/* ---- C-backed CALLDATA (per call frame; alias into the parent's memory) ---- */
val cd_set       = impure "cd_set"       : (bits(64), bits(64)) -> unit
val cd_set_empty = impure "cd_set_empty" : unit -> unit
val cd_set_tx    = impure "cd_set_tx"    : unit -> unit
val txd_begin    = impure "txd_begin"    : unit -> unit
val txd_byte     = impure "txd_byte"     : bits(8) -> unit
val cd_len       = impure "cd_len"       : unit -> bits(64)
val cd_word      = impure "cd_word"      : bits(64) -> word
val cd_to_mem    = impure "cd_to_mem"    : (bits(64), bits(64), bits(64)) -> unit

function calldata_word(off : int) -> word = cd_word(get_slice_int(64, off, 0))
function calldata_copy_to_mem(dst : int, off : int, len : int) -> unit =
  cd_to_mem(get_slice_int(64, dst, 0), get_slice_int(64, off, 0), get_slice_int(64, len, 0))
function calldata_set_next(off : int, len : int) -> unit =
  cd_set(get_slice_int(64, off, 0), get_slice_int(64, len, 0))
/* stream the transaction input once; returns its length */
function txdata_load(data : list(byte)) -> int = {
  txd_begin();
  var n : int = 0;
  var rest : list(byte) = data;
  var more : bool = true;
  while more do {
    match rest { b :: t => { txd_byte(b); n = n + 1; rest = t }, [||] => more = false }
  };
  n
}
