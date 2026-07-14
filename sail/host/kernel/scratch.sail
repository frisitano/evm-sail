/* A bump cursor makes construction order and lifetime visible in the Sail
 * semantics while the bytes themselves remain in host-backed memory. */

register scratch_cursor : bits(64) = 0x0000000000000000

function scratch_begin() -> int = unsigned(scratch_cursor)

function scratch_push_bytes(data : list(byte)) -> unit = {
    let len = list_len(data);
    if len > 0 then {
        let width = get_slice_int(64, len, 0);
        assert(host_scratch_store_bytes(scratch_cursor, data, width), "scratch byte append");
        scratch_cursor = scratch_cursor + width
    }
}

function scratch_push_slice(data : ByteSlice) -> unit = {
    if data.len != 0x0000000000000000 then {
        assert(host_scratch_store_slice(scratch_cursor, data), "scratch slice append");
        scratch_cursor = scratch_cursor + data.len
    }
}

function scratch_finish(start : int) -> ByteSlice = {
    let stop = unsigned(scratch_cursor);
    assert((start >= 0) & (start <= stop), "scratch finish mark");
    byte_slice(ScratchSource, start, stop - start)
}

function scratch_rewind(mark : int) -> unit = {
    let stop = unsigned(scratch_cursor);
    assert((mark >= 0) & (mark <= stop), "scratch rewind mark");
    scratch_cursor = get_slice_int(64, mark, 0);
    host_scratch_truncate(scratch_cursor)
}

function scratch_reset() -> unit = {
    scratch_cursor = 0x0000000000000000;
    host_scratch_truncate(scratch_cursor)
}
