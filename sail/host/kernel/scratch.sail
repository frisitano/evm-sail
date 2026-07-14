/*md
# Kernel scratch allocation

A bump cursor makes construction order and lifetime visible in the Sail
semantics while the bytes themselves remain in host-backed memory.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

/*! The scratch arena's bump cursor; Sail owns allocation order. */
register scratch_cursor : bits(64) = 0x0000000000000000

/*! Marks the start of a scratch construction. */
function scratch_begin() -> int = unsigned(scratch_cursor)

/*! Appends materialized bytes at the cursor. */
function scratch_push_bytes(data : list(byte)) -> unit = {
    let len = list_len(data);
    if len > 0 then {
        let width = get_slice_int(64, len, 0);
        assert(host_scratch_store_bytes(scratch_cursor, data, width), "scratch byte append");
        scratch_cursor = scratch_cursor + width
    }
}

/*! Appends a source-backed slice at the cursor. */
function scratch_push_slice(data : ByteSlice) -> unit = {
    if data.len != 0x0000000000000000 then {
        assert(host_scratch_store_slice(scratch_cursor, data), "scratch slice append");
        scratch_cursor = scratch_cursor + data.len
    }
}

/*! The slice covering everything pushed since `start`. */
function scratch_finish(start : int) -> ByteSlice = {
    let stop = unsigned(scratch_cursor);
    assert((start >= 0) & (start <= stop), "scratch finish mark");
    byte_slice(ScratchSource, start, stop - start)
}

/*! Discards everything pushed since `mark`. */
function scratch_rewind(mark : int) -> unit = {
    let stop = unsigned(scratch_cursor);
    assert((mark >= 0) & (mark <= stop), "scratch rewind mark");
    scratch_cursor = get_slice_int(64, mark, 0);
    host_scratch_truncate(scratch_cursor)
}

/*! Empties the arena (per-block lifetime). */
function scratch_reset() -> unit = {
    scratch_cursor = 0x0000000000000000;
    host_scratch_truncate(scratch_cursor)
}
