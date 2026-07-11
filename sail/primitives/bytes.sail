/* ======================================================================== */
/* PRIMITIVES: BYTE SOURCES                                                 */
/* ======================================================================== */

/* The abstract byte-source vocabulary: a ByteSlice denotes a byte sequence
   inside one of the host's C-backed stores (witness input, frame memory, tx
   input, active code, trie arena) without materializing it. */

/* Byte-source slices identify contiguous bytes without saying whether the
 * backend materializes them. The spec backend defines their meaning as
 * mathematical byte lists; the build backend resolves them to pointer/length
 * pairs when possible. */
enum ByteSource = { WitnessSource, MemorySource, TxInputSource, ActiveCodeSource, TrieArenaSource }

type source_offset = bits(64)
type source_len    = bits(64)

function to_source_offset(i : int) -> source_offset = get_slice_int(64, i, 0)
function to_source_len(i : int) -> source_len = get_slice_int(64, i, 0)

struct ByteSlice = {
    source : ByteSource,
    off    : source_offset,
    len    : source_len,
}

function byte_slice(src : ByteSource, off : int, len : int) -> ByteSlice =
    struct { source = src, off = to_source_offset(off), len = to_source_len(len) }

function byte_source_kind(src : ByteSource) -> bits(64) =
    match src {
        WitnessSource => 0x0000000000000001,
        MemorySource => 0x0000000000000002,
        TxInputSource => 0x0000000000000003,
        ActiveCodeSource => 0x0000000000000004,
        TrieArenaSource => 0x0000000000000005,
    }

/* the empty byte source (zero-length); a placeholder ByteSlice for item/patch
 * fields whose value role is unused (subtree refs, deletes). */
let EMPTY_SLICE : ByteSlice = struct { source = WitnessSource, off = 0x0000000000000000, len = 0x0000000000000000 }

function witness_byte_slice(off : int, len : int) -> ByteSlice =
    byte_slice(WitnessSource, off, len)

/* a relative sub-range of a slice (same source) */
function sub_slice(s : ByteSlice, off : int, len : int) -> ByteSlice =
    byte_slice(s.source, unsigned(s.off) + off, len)

function memory_byte_slice(off : int, len : int) -> ByteSlice =
    byte_slice(MemorySource, off, len)

function tx_input_byte_slice(off : int, len : int) -> ByteSlice =
    byte_slice(TxInputSource, off, len)

function active_code_byte_slice(off : int, len : int) -> ByteSlice =
    byte_slice(ActiveCodeSource, off, len)

/* Big-endian byte serializations used by RLP, the tries, and address derivation. */
function word_to_bytes32(w : word) -> list(byte) = {
    var out : list(byte) = [||]; /* O(n): prepend big-endian */
    foreach (k from 0 to 31) {
        out = sail_shiftright(w, 8 * k)[7 .. 0] :: out
    };
    out
}
function address_to_bytes(a : address) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to 19) {
        out = sail_shiftright(a, 8 * k)[7 .. 0] :: out
    };
    out
}
