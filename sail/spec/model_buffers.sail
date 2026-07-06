/* ======================================================================== */
/* SPEC BACKEND: PURE REFERENCE MODELS (BUFFERS)                            */
/*                                                                          */
/* Pure Sail reference implementations of the C-backed per-frame byte       */
/* buffers: EVM memory, the returndata slots, calldata descriptors, the     */
/* transaction-input buffer, and the active frame's code binding            */
/* (iface/memory.sail and the frame half of iface/code_db.sail; refines     */
/* ffi/memory.c, ffi/returndata.c, and ffi/code_db.c). In the C backend all */
/* of these are indexed by the memory frame depth, so they are modeled      */
/* together over that shared depth.                                         */
/* ======================================================================== */

/* ---- per-frame memory (refines ffi/memory.c) ----------------------------
 * Each frame's memory is a write log (latest write first); reads default
 * to 0x00, matching the C's lazily-grown zero-initialized buffer. Frames
 * form a stack with the head as the CURRENT frame; the frame depth (the
 * index the C keys its returndata/calldata/code arrays by) is
 * list_len(frames) - 1. host_mem_pop at the bottom frame clears its bytes
 * but keeps the frame, exactly like the C (which frees the buffer and
 * leaves h_top at 0). The C caps allocation at 2^40 bytes -- an offset gas
 * can never pay for -- making single-byte writes at offsets >= 2^40 and
 * whole range copies whose end crosses 2^40 no-ops; the model mirrors both
 * guards. The C's 1100-frame array bound exceeds the EVM depth limit
 * (1024 calls + the tx frame) and is not modeled. */

struct SpecMemByte = { mb_off : int, mb_val : byte }

register spec_hm_frames : list(list(SpecMemByte)) = [|[||]|]

/* the C allocation hard cap: offsets past it are unreachable under gas */
let SPEC_MEM_CAP : int = 1099511627776

val spec_mem_log_byte : (list(SpecMemByte), int) -> byte
function spec_mem_log_byte(log, off) =
    match log {
        [||] => 0x00,
        e :: t => if e.mb_off == off then {
            e.mb_val
        } else {
            spec_mem_log_byte(t, off)
        },
    }

val spec_mem_cur_frame : unit -> list(SpecMemByte)
function spec_mem_cur_frame() =
    match spec_hm_frames {
        f :: _ => f,
        [||] => [||],
    }

val spec_mem_depth : unit -> int
function spec_mem_depth() = list_len(spec_hm_frames) - 1

/* mem[off .. off+len) of the current frame, zero-defaulted, address order */
val spec_mem_read_range : (int, int) -> list(byte)
function spec_mem_read_range(off, len) = {
    let f = spec_mem_cur_frame();
    var out : list(byte) = [||];
    foreach (k from 0 to (len - 1)) {
        out = spec_mem_log_byte(f, off + (len - 1 - k)) :: out
    };
    out
}

val spec_mem_log_write_bytes : (list(SpecMemByte), int, list(byte)) -> list(SpecMemByte)
function spec_mem_log_write_bytes(log, off, bs) =
    match bs {
        [||] => log,
        b :: t => spec_mem_log_write_bytes(struct { mb_off = off, mb_val = b } :: log, off + 1, t),
    }

/* whole-copy-or-nothing range write into the current frame (the C acquires
 * one write pointer for the range: a no-op when empty or when the range end
 * crosses the allocation cap) */
val spec_mem_write_range : (int, list(byte)) -> unit
function spec_mem_write_range(dst, bs) = {
    let n = list_len(bs);
    if (n > 0) & (dst + n <= SPEC_MEM_CAP) then {
        match spec_hm_frames {
            f :: rest => spec_hm_frames = spec_mem_log_write_bytes(f, dst, bs) :: rest,
            [||] => (),
        }
    }
}

function host_mem_read(off : bits(64)) -> bits(8) = spec_mem_log_byte(spec_mem_cur_frame(), unsigned(off))

function host_mem_write(off : bits(64), v : bits(8)) -> unit =
    if unsigned(off) < SPEC_MEM_CAP then {
        match spec_hm_frames {
            f :: rest => spec_hm_frames = (struct { mb_off = unsigned(off), mb_val = v } :: f) :: rest,
            [||] => (),
        }
    }

/* MCOPY: overlap-safe -- the whole source range is read before writing */
function hm_move(dst : bits(64), src : bits(64), len : bits(64)) -> unit =
    if unsigned(len) > 0 then {
        spec_mem_write_range(unsigned(dst), spec_mem_read_range(unsigned(src), unsigned(len)))
    }

/* ---- calldata descriptors (refines ffi/memory.c cd_*) -------------------
 * A per-frame descriptor aliasing another byte source, resolved at READ
 * time: src = -2 denotes empty calldata, -1 the transaction input buffer,
 * and >= 0 the memory of the (suspended) frame at that absolute depth.
 * cd_set stages a pending descriptor over the CURRENT frame's memory;
 * host_mem_push adopts it for the new frame and clears it. The descriptor
 * list parallels the frame stack (the C's per-depth array is only ever
 * read at the current depth, so the stale higher-depth entries it keeps
 * are unobservable). */

struct SpecCdDesc = { cd_src : int, cd_off : int, cd_len : int }

register spec_cd : list(SpecCdDesc) = [|struct { cd_src = -2, cd_off = 0, cd_len = 0 }|]
register spec_cd_pending : SpecCdDesc = struct { cd_src = -2, cd_off = 0, cd_len = 0 }

/* ---- transaction input (refines ffi/memory.c txin_*) ---- */

register spec_txin : list(byte) = [||]

function txin_begin() -> unit = spec_txin = [||]

function txin_byte(b : bits(8)) -> unit = spec_txin = list_append(spec_txin, [|b|])

function txd_at(i : bits(64)) -> bits(8) = bytes_get(spec_txin, unsigned(i))

function txd_length() -> bits(64) = get_slice_int(64, list_len(spec_txin), 0)

/* one-call tx-input staging (the spec/hybrid refinement of iface/memory.sail):
 * materialize the whole input into the buffer. txdata_set_source lives after
 * read_byte_slice (below); the word/count refinements loop here. */
function txdata_set_word(w : word) -> int = {
    spec_txin = word_to_bytes32(w);
    32
}

function txd_count_nonzero() -> bits(64) = {
    let n = list_len(spec_txin);
    var c : int = 0;
    foreach (i from 0 to (n - 1)) {
        if bytes_get(spec_txin, i) != 0x00 then {
            c = c + 1
        }
    };
    get_slice_int(64, c, 0)
}

/* the current frame's calldata byte i (0 past the end; a memory-sourced
 * descriptor reads the source frame's write log, so never-written bytes of
 * an expansion-charged range read as zeros, like the C's cap check) */
val spec_cd_byte : int -> byte
function spec_cd_byte(i) = {
    let d : SpecCdDesc = match spec_cd {
        c :: _ => c,
        [||] => struct { cd_src = -2, cd_off = 0, cd_len = 0 },
    };
    if (i < 0) | (i >= d.cd_len) then {
        0x00
    } else {
        if d.cd_src == -1 then {
            bytes_get(spec_txin, i)
        } else {
            if d.cd_src >= 0 then {
                let idx = (list_len(spec_hm_frames) - 1) - d.cd_src;
                spec_mem_log_byte(list_nth(spec_hm_frames, idx, [||]), d.cd_off + i)
            } else {
                0x00
            }
        }
    }
}

function cd_set(off : bits(64), len : bits(64)) -> unit =
    spec_cd_pending = struct { cd_src = spec_mem_depth(), cd_off = unsigned(off), cd_len = unsigned(len) }

function cd_set_empty() -> unit = spec_cd_pending = struct { cd_src = -2, cd_off = 0, cd_len = 0 }

/* the CURRENT (tx-level) frame's calldata := the streamed tx input; the
 * length is captured now, matching the C */
function cd_set_tx() -> unit =
    match spec_cd {
        _ :: rest => spec_cd = struct { cd_src = -1, cd_off = 0, cd_len = list_len(spec_txin) } :: rest,
        [||] => spec_cd = [|struct { cd_src = -1, cd_off = 0, cd_len = list_len(spec_txin) }|],
    }

/* CALLDATACOPY: calldata[off .. off+len) -> memory[dst ..), zero-padded.
 * The C's uint64 wraparound guard on off+k is subsumed by unbounded int
 * arithmetic here: past-end reads are zeros by construction. */
function cd_to_mem(dst : bits(64), off : bits(64), len : bits(64)) -> unit =
    if unsigned(len) > 0 then {
        let n = unsigned(len);
        let o = unsigned(off);
        var out : list(byte) = [||];
        foreach (k from 0 to (n - 1)) {
            out = spec_cd_byte(o + (n - 1 - k)) :: out
        };
        spec_mem_write_range(unsigned(dst), out)
    }

/* CALLDATALOAD: the 32-byte word at calldata offset i (zero-padded) */
function calldata_load_word(i : bits(64)) -> word = {
    var out : list(byte) = [||];
    foreach (k from 0 to 31) {
        out = spec_cd_byte(unsigned(i) + (31 - k)) :: out
    };
    bytes_be256(out)
}

/* ---- memory frame lifecycle (adopts the pending calldata descriptor) ---- */

function host_mem_reset() -> unit = {
    spec_hm_frames = [|[||]|];
    spec_cd = [|struct { cd_src = -2, cd_off = 0, cd_len = 0 }|];
    cd_set_empty()
}

function host_mem_push() -> unit = {
    spec_hm_frames = [||] :: spec_hm_frames;
    spec_cd = spec_cd_pending :: spec_cd;
    cd_set_empty()
}

function host_mem_pop() -> unit = {
    match spec_hm_frames {
        _ :: (f :: rest) => spec_hm_frames = f :: rest,
        /* popping the bottom frame clears its bytes but keeps the frame */
        _ => spec_hm_frames = [|[||]|],
    };
    match spec_cd {
        _ :: (c :: rest) => spec_cd = c :: rest,
        /* the C's pop never touches the descriptor array */
        _ => (),
    }
}

/* ---- returndata slots (refines ffi/returndata.c) -------------------------
 * One byte buffer per frame depth plus a PENDING buffer: a child's output
 * (RETURN/REVERT or a precompile) is captured into pending and adopted
 * into the caller's slot after the frame pops. Slots persist across
 * pop/push until cleared or re-adopted, exactly like the C's per-depth
 * array, so they are keyed by depth instead of stacked. */

struct SpecRdSlot = { rds_depth : int, rds_bytes : list(byte) }

register spec_rd_slots : list(SpecRdSlot) = [||]
register spec_rd_pending : list(byte) = [||]

val spec_rd_get : (list(SpecRdSlot), int) -> list(byte)
function spec_rd_get(ss, d) =
    match ss {
        [||] => [||],
        e :: t => if e.rds_depth == d then {
            e.rds_bytes
        } else {
            spec_rd_get(t, d)
        },
    }

val spec_rd_put : (list(SpecRdSlot), int, list(byte)) -> list(SpecRdSlot)
function spec_rd_put(ss, d, bs) =
    match ss {
        [||] => [|struct { rds_depth = d, rds_bytes = bs }|],
        e :: t => if e.rds_depth == d then {
            struct { rds_depth = d, rds_bytes = bs } :: t
        } else {
            e :: spec_rd_put(t, d, bs)
        },
    }

function returndata_reset_all() -> unit = {
    spec_rd_slots = [||];
    spec_rd_pending = [||]
}

function returndata_clear_current() -> unit = spec_rd_slots = spec_rd_put(spec_rd_slots, spec_mem_depth(), [||])

function returndata_discard_pending() -> unit = spec_rd_pending = [||]

function returndata_capture_memory(off : bits(64), len : bits(64)) -> unit =
    spec_rd_pending = spec_mem_read_range(unsigned(off), unsigned(len))

function returndata_adopt_pending() -> unit = {
    spec_rd_slots = spec_rd_put(spec_rd_slots, spec_mem_depth(), spec_rd_pending);
    spec_rd_pending = [||]
}

function returndata_current_len() -> bits(64) =
    get_slice_int(64, list_len(spec_rd_get(spec_rd_slots, spec_mem_depth())), 0)

function returndata_pending_len() -> bits(64) = get_slice_int(64, list_len(spec_rd_pending), 0)

function returndata_pending_byte_at(i : bits(64)) -> bits(8) = bytes_get(spec_rd_pending, unsigned(i))

/* RETURNDATACOPY: whole-copy-or-nothing -- a no-op when the source range
 * [off, off+len) is empty or leaves the current returndata */
function returndata_copy_current_to_memory(dst : bits(64), off : bits(64), len : bits(64)) -> unit = {
    let cur = spec_rd_get(spec_rd_slots, spec_mem_depth());
    if (unsigned(len) > 0) & (unsigned(off) + unsigned(len) <= list_len(cur)) then {
        spec_mem_write_range(unsigned(dst), bytes_slice(cur, unsigned(off), unsigned(len)))
    }
}

/* the CALL-family out region: copies min(want, len) prefix bytes */
function returndata_copy_current_prefix_to_memory(dst : bits(64), want : bits(64)) -> unit = {
    let cur = spec_rd_get(spec_rd_slots, spec_mem_depth());
    let n = if unsigned(want) < list_len(cur) then {
            unsigned(want)
        } else {
            list_len(cur)
        };
    if n > 0 then {
        spec_mem_write_range(unsigned(dst), bytes_slice(cur, 0, n))
    }
}

/* ---- frame code bindings (refines ffi/code_db.c fc_*) --------------------
 * The active frame's code view, keyed by frame depth (persisting across
 * pop/push until rebound, like the C's descriptor array). A binding copies
 * the bytes it aliases: the C binds pointers, but every source is
 * immutable while bound (store entries are only re-streamed while empty,
 * and Sail callers stream only when code_db_stream_code_begin returns 1),
 * so the copy is observationally equal. JUMPDEST validity is recomputed
 * from the bound bytes by a pure scan instead of the C's precomputed
 * bitmap: position i is valid iff it holds 0x5b and is not inside a
 * PUSH1..PUSH32 immediate. */

struct SpecFcSlot = { fcs_depth : int, fcs_code : list(byte) }

register spec_fc : list(SpecFcSlot) = [||]

val spec_fc_get : (list(SpecFcSlot), int) -> list(byte)
function spec_fc_get(ss, d) =
    match ss {
        [||] => [||],
        e :: t => if e.fcs_depth == d then {
            e.fcs_code
        } else {
            spec_fc_get(t, d)
        },
    }

val spec_fc_put : (list(SpecFcSlot), int, list(byte)) -> list(SpecFcSlot)
function spec_fc_put(ss, d, c) =
    match ss {
        [||] => [|struct { fcs_depth = d, fcs_code = c }|],
        e :: t => if e.fcs_depth == d then {
            struct { fcs_depth = d, fcs_code = c } :: t
        } else {
            e :: spec_fc_put(t, d, c)
        },
    }

val spec_fc_cur : unit -> list(byte)
function spec_fc_cur() = spec_fc_get(spec_fc, spec_mem_depth())

/* current frame's code := the store entry for codeHash h; its length,
 * or an empty binding and 0 when h is not stored with bytes */
function frame_code_bind_stored(h : hash) -> bits(64) = {
    let (found, c) = spec_code_find(spec_code_db, h);
    if found & (list_len(c) > 0) then {
        spec_fc = spec_fc_put(spec_fc, spec_mem_depth(), c);
        get_slice_int(64, list_len(c), 0)
    } else {
        spec_fc = spec_fc_put(spec_fc, spec_mem_depth(), [||]);
        0x0000000000000000
    }
}

/* the NEXT child frame's code := a copy of THIS frame's memory range */
function frame_code_bind_memory(off : bits(64), len : bits(64)) -> bits(64) = {
    let child = spec_mem_depth() + 1;
    if unsigned(len) == 0 then {
        spec_fc = spec_fc_put(spec_fc, child, [||]);
        0x0000000000000000
    } else {
        spec_fc = spec_fc_put(spec_fc, child, spec_mem_read_range(unsigned(off), unsigned(len)));
        len
    }
}

/* current frame's code := the streamed tx input (a create-tx's initcode) */
function frame_code_bind_tx_input() -> bits(64) = {
    let n = list_len(spec_txin);
    if n == 0 then {
        spec_fc = spec_fc_put(spec_fc, spec_mem_depth(), [||]);
        0x0000000000000000
    } else {
        spec_fc = spec_fc_put(spec_fc, spec_mem_depth(), spec_txin);
        get_slice_int(64, n, 0)
    }
}

function frame_code_clear() -> unit = spec_fc = spec_fc_put(spec_fc, spec_mem_depth(), [||])

function frame_code_byte(i : bits(64)) -> bits(8) = bytes_get(spec_fc_cur(), unsigned(i))

function frame_code_length() -> bits(64) = get_slice_int(64, list_len(spec_fc_cur()), 0)

/* ---- byte-source materialization (the spec/hybrid backing of the byte
 * sources: read a source into a Sail list). Build reads sources directly via
 * FFI and never materializes, so these live only here. read_byte_slice is
 * the reader that iface/crypto.sail's bytes_keccak and iface/code_db.sail's
 * code_db_store_slice hash/store under spec/hybrid. ---- */

function read_active_code_slice(off : int, len : int) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to (len - 1)) {
        out = frame_code_byte(get_slice_int(64, off + (len - 1 - k), 0)) :: out
    };
    out
}

function read_byte_slice(s : ByteSlice) -> list(byte) =
    match s.source {
        WitnessSource => ssz_bytes(unsigned(s.off), unsigned(s.len)),
        MemorySource => mem_read_range(unsigned(s.off), unsigned(s.len)),
        TxInputSource => txdata_read_range(unsigned(s.off), unsigned(s.len)),
        ActiveCodeSource => read_active_code_slice(unsigned(s.off), unsigned(s.len)),
    }

/* Source-backed code storage under spec/hybrid: materialize the bytes, then
 * store the list under its keccak hash (the content-addressed code store,
 * model_state.sail). Build's build/code_db.sail copies the span C-side. */
function code_db_store_slice(s : ByteSlice) -> unit =
    code_db_store_code(bytes_keccak(s), read_byte_slice(s))

/* Stage the tx input from a byte source (spec/hybrid): materialize the source
 * bytes into the input buffer. Build copies the span C-side in one memcpy. */
function txdata_set_source(s : ByteSlice) -> int = {
    spec_txin = read_byte_slice(s);
    list_len(spec_txin)
}

/* drop the first n bytes (empty past the end; no padding) */
val spec_bytes_drop : (list(byte), int) -> list(byte)
function spec_bytes_drop(c, n) =
    if n <= 0 then {
        c
    } else {
        match c {
            [||] => [||],
            _ :: t => spec_bytes_drop(t, n - 1),
        }
    }

/* is position i of code c a valid JUMPDEST? A left-to-right opcode scan:
 * PUSH1..PUSH32 (0x60..0x7f = 96..127) skip their immediate bytes, so an
 * 0x5b inside an immediate is not a destination; past-end is invalid. */
val spec_jumpdest_scan : (list(byte), int) -> bool
function spec_jumpdest_scan(c, i) =
    match c {
        [||] => false,
        b :: t => if i <= 0 then {
            (i == 0) & (b == 0x5b)
        } else {
            if (unsigned(b) >= 96) & (unsigned(b) <= 127) then {
                let skip = unsigned(b) - 95;
                if i <= skip then {
                    false
                } else {
                    spec_jumpdest_scan(spec_bytes_drop(t, skip), i - 1 - skip)
                }
            } else {
                spec_jumpdest_scan(t, i - 1)
            }
        },
    }

function frame_jumpdest_valid(i : bits(64)) -> bool = spec_jumpdest_scan(spec_fc_cur(), unsigned(i))

/* CODECOPY: code[off .. off+len) -> memory[dst ..), zero-padded past end */
function frame_code_copy_to_memory(dst : bits(64), off : bits(64), len : bits(64)) -> unit =
    if unsigned(len) > 0 then {
        spec_mem_write_range(unsigned(dst), bytes_slice(spec_fc_cur(), unsigned(off), unsigned(len)))
    }

/* the n-byte PUSH immediate starting at offset i, right-aligned (n > 32
 * clamps to 32; past-end bytes read as zero) */
function frame_push_immediate_word(i : bits(64), n : bits(64)) -> word = {
    let cnt = if unsigned(n) < 32 then {
            unsigned(n)
        } else {
            32
        };
    bytes_be256(bytes_slice(spec_fc_cur(), unsigned(i), cnt))
}

/* EXTCODECOPY: stored code(h)[off .. off+len) -> memory[dst ..), padded */
function code_db_copy_stored_code_to_memory(h : hash, dst : bits(64), off : bits(64), len : bits(64)) -> unit =
    if unsigned(len) > 0 then {
        let (_, c) = spec_code_find(spec_code_db, h);
        spec_mem_write_range(unsigned(dst), bytes_slice(c, unsigned(off), unsigned(len)))
    }
