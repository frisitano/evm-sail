/* ======================================================================== */
/* BYTE-SLICE READS, TRANSACTION INPUT, AND CODE INDEXING                   */
/*                                                                          */
/* ByteSlice is the common read-only view used by calldata AND executable   */
/* frame code. The TX INPUT (ffi/memory.c) is the transaction's data span   */
/* (Transaction.input_src, usually the witness), BOUND once per tx; block   */
/* system calls STAGE their 32-byte word instead (the one value with no     */
/* backing span). The generic readers resolve any ByteSlice without a       */
/* parallel per-frame source descriptor.                                    */
/*                                                                          */
/* Naming: `bind` = set a reference, `stage` = copy a value.                */
/* Proof targets see the vals as bodyless axioms over abstract byte lists.  */
/* ======================================================================== */

/* ---- the transaction input (bound once per tx) ------------------------- */

val txdata_bind_source = impure { c: "txdata_bind_source" } : (bits(64), bits(64), bits(64)) -> bits(64)
val txdata_stage_word = impure { c: "txdata_stage_word" } : word -> bits(64)
val txdata_byte_at = impure { c: "txdata_byte_at" } : bits(64) -> bits(8)
val txdata_length = impure { c: "txdata_length" } : unit -> bits(64)

/* EIP-2028 / EIP-7623 calldata gas: nonzero input bytes, counted C-side */
val txdata_count_nonzero = impure { c: "txdata_count_nonzero" } : unit -> bits(64)

/* bind the tx input to a byte source; returns the bound length
 * (fail-closed 0 on a bad source) */
function txdata_bind(s : ByteSlice) -> int =
    unsigned(txdata_bind_source(byte_source_kind(s.source), s.off, s.len))

/* stage a block system call's 32-byte input word */
function txdata_stage(w : word) -> unit = {
    let _ = txdata_stage_word(w)
}

/* byte i of the bound input (0x00 past the end) */
function txdata_byte(i : int) -> byte = txdata_byte_at(get_slice_int(64, i, 0))

/* the bound input's length in bytes */
function txdata_len() -> int = unsigned(txdata_length())

/* ---- generic slice reads (serve calldata and executable code) ---------- */

val host_slice_byte = impure { c: "slice_byte_at" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(8)
val host_slice_load_word = impure { c: "slice_load_word" } : (bits(64), bits(64), bits(64), bits(64)) -> word
val host_slice_load_n_word = impure { c: "slice_load_n_word" } : (bits(64), bits(64), bits(64), bits(64), bits(64)) -> word
val host_slice_copy_to_memory = impure { c: "slice_copy_to_memory" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
val mem_establish_absolute = impure { c: "mem_establish_absolute" } : (bits(64), bits(64)) -> bits(64)

val code_db_lookup = impure { c: "code_db_lookup" } : hash -> option(Code)
val jumpdest_ref_contains = impure { c: "jumpdest_ref_contains" } : (JumpdestRef, bits(64), bits(64)) -> bool
val code_db_store_indexed_source = impure { c: "code_db_store_indexed_source" } : (
        bits(64),
        bits(64),
        bits(64),
        JumpdestBitmap,
    ) -> hash
val code_intern_indexed_delegation = impure { c: "code_intern_indexed_delegation" } : (address, JumpdestBitmap) -> hash
val code_db_read_delegation = impure { c: "code_db_read_delegation" } : hash -> bits(168)

let SOURCE_PAST_END : int = 16777216

function clamp_source_offset(off : int) -> int =
    if off > SOURCE_PAST_END then { SOURCE_PAST_END } else { off }

/* mint a slice of THIS frame's memory [off, off+len): the range is
 * established (it is already gas-charged) and referenced by ABSOLUTE arena
 * offset -- the frame is about to suspend, so the bytes are frozen for the
 * reader. EMPTY_SLICE on overflow/OOM (fail-closed). */
function mem_slice(off : int, len : int) -> ByteSlice =
    if len == 0 then {
        EMPTY_SLICE
    } else {
        let abs = mem_establish_absolute(get_slice_int(64, off, 0), get_slice_int(64, len, 0));
        if abs == 0xFFFFFFFFFFFFFFFF then {
            EMPTY_SLICE
        } else {
            struct { source = MemoryArenaSource, off = abs, len = get_slice_int(64, len, 0) }
        }
    }

/* The byte at slice offset `off`, zero past the end. */
function slice_byte(s : ByteSlice, off : int) -> byte =
    host_slice_byte(byte_source_kind(s.source), s.off, s.len, get_slice_int(64, clamp_source_offset(off), 0))

function read_byte_slice(s : ByteSlice) -> list(byte) = {
    let len = unsigned(s.len);
    var out : list(byte) = [||];
    var i : int = 0;
    while i < len do {
        out = slice_byte(s, i) :: out;
        i = i + 1
    };
    list_reverse(out)
}

function bytes_len(data : Bytes) -> int =
    match data {
        BytesList(bs) => list_len(bs),
        BytesSlice(s) => unsigned(s.len),
    }

function bytes_first(data : Bytes) -> byte =
    match data {
        BytesList(bs) => list_nth(bs, 0, 0x00),
        BytesSlice(s) => slice_byte(s, 0),
    }

function bytes_to_list(data : Bytes) -> list(byte) =
    match data {
        BytesList(bs) => bs,
        BytesSlice(s) => read_byte_slice(s),
    }

function bytes_at(data : Bytes, off : int) -> byte =
    match data {
        BytesList(bs) => bytes_get(bs, off),
        BytesSlice(s) => slice_byte(s, off),
    }

function bytes_range(data : Bytes, off : int, len : int) -> Bytes =
    match data {
        BytesList(bs) => BytesList(bytes_slice(bs, off, len)),
        BytesSlice(s) => BytesSlice(sub_slice(s, off, len)),
    }

/* The 32-byte word at slice offset `off`, zero-padded past the end. */
function slice_load(s : ByteSlice, off : int) -> word =
    host_slice_load_word(byte_source_kind(s.source), s.off, s.len, get_slice_int(64, clamp_source_offset(off), 0))

/* An n-byte big-endian word at slice offset `off`, right-aligned and
 * zero-padded past the end. Used by PUSH0..PUSH32. */
function slice_load_n(s : ByteSlice, off : int, n : int) -> word =
    host_slice_load_n_word(
        byte_source_kind(s.source),
        s.off,
        s.len,
        get_slice_int(64, clamp_source_offset(off), 0),
        get_slice_int(64, n, 0),
    )

/* slice `s` [off, off+len) into current memory at dst, zero-filling past
 * the end (EELS buffer_read). */
function slice_copy(s : ByteSlice, dst : int, off : int, len : int) -> unit =
    host_slice_copy_to_memory(
        byte_source_kind(s.source),
        s.off,
        s.len,
        get_slice_int(64, dst, 0),
        get_slice_int(64, clamp_source_offset(off), 0),
        get_slice_int(64, len, 0),
    )

/* ---- executable-code indexing ----------------------------------------- */

/* Analyze legacy EVM bytecode once. Bytes consumed as PUSH1..PUSH32
 * immediates are data, so an 0x5b inside them is not a jump destination.
 * Scanning is monotonic: completed 64-byte words are prepended, then the
 * short word list is reversed once. No host table exists until this pass has
 * produced the whole immutable result. */
function index_code(code : ByteSlice) -> AnalyzedCode = {
    let n = unsigned(code.len);
    let nwords = ediv_int(n + 63, 64);
    var words_rev : JumpdestBitmap = [||];
    var current : bits(64) = 0x0000000000000000;
    var current_index : int = 0;
    var i : int = 0;
    while i < n do {
        let target_index = ediv_int(i, 64);
        while current_index < target_index do {
            words_rev = current :: words_rev;
            current = 0x0000000000000000;
            current_index = current_index + 1
        };
        let op = unsigned(slice_byte(code, i));
        if op == 91 then {
            current = or_vec(current, sail_shiftleft(0x0000000000000001, emod_int(i, 64)))
        };
        if (op >= 96) & (op <= 127) then {
            i = i + 1 + (op - 95)
        } else {
            i = i + 1
        }
    };
    while current_index < nwords do {
        words_rev = current :: words_rev;
        current = 0x0000000000000000;
        current_index = current_index + 1
    };
    struct { bytes = code, jumpdests = list_reverse(words_rev) }
}

/* The C store is deliberately analysis-blind: it receives the immutable byte
 * source and the already-materialized table, and returns keccak(code). */
function code_db_store_indexed(code : AnalyzedCode) -> hash =
    code_db_store_indexed_source(byte_source_kind(code.bytes.source), code.bytes.off, code.bytes.len, code.jumpdests)

function code_db_index_and_store(code : ByteSlice) -> hash = code_db_store_indexed(index_code(code))

function code_db_resolve(h : hash) -> Code =
    if h == KECCAK_EMPTY then {
        EMPTY_CODE
    } else {
        match code_db_lookup(h) {
            Some(code) => code,
            None() => throw InvalidBlock(WitnessDeficient),
        }
    }
