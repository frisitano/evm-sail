
/*md
# Kernel scratch allocation

A bump cursor makes construction order and lifetime visible in the Sail
semantics while the bytes themselves remain in host-backed memory.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/
/*! The scratch arena's bump cursor; Sail owns allocation order. */
register scratch_cursor : source_pointer = BYTE_ZERO

/*! Marks the start of a scratch construction. */
function scratch_begin() -> source_pointer = scratch_cursor

/*! Advances the scratch bump cursor by `width` bytes within the byte domain. */
function scratch_advance(width : byte_length) -> unit = {
    let cursor : source_pointer = scratch_cursor;
    if cursor <= MAX_BYTE_QUANTITY then {
        if width <= MAX_BYTE_QUANTITY - cursor then {
            scratch_cursor = cursor + width
        } else {
            assert(false, "scratch cursor overflow")
        }
    } else {
        assert(false, "scratch cursor overflow")
    }
}

/*! Appends materialized bytes at the cursor. */
function scratch_push_bytes(data : list(byte), len : byte_length) -> unit = {
    if len != BYTE_ZERO then {
        assert(host_scratch_store_bytes(scratch_cursor, data, len), "scratch byte append");
        scratch_advance(len)
    }
}

/*! Appends a source-backed slice at the cursor. */
function scratch_push_slice(data : ByteSlice) -> unit = {
    if data.len != BYTE_ZERO then {
        assert(host_scratch_store_slice(scratch_cursor, data), "scratch slice append");
        scratch_advance(data.len)
    }
}

/*! The slice covering everything pushed since `start`. */
function scratch_finish(start : source_pointer) -> ByteSlice =
    let stop : source_pointer = scratch_cursor in
    if start <= stop then {
        let len : byte_length = stop - start;
        byte_slice(ScratchSource, start, len)
    } else {
        assert(false, "scratch finish mark");
        EMPTY_SLICE
    }

/*! Discards everything pushed since `mark`. */
function scratch_rewind(mark : source_pointer) -> unit =
    let cursor : source_pointer = scratch_cursor in
    if mark <= cursor then {
        scratch_cursor = mark;
        host_scratch_truncate(scratch_cursor)
    } else {
        assert(false, "scratch rewind mark")
    }

/*! Empties the arena (per-block lifetime). */
function scratch_reset() -> unit = {
    scratch_cursor = BYTE_ZERO;
    host_scratch_truncate(scratch_cursor)
}
