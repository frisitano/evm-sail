
/*md
# Kernel scratch allocation

A bump cursor makes construction order and lifetime visible in the Sail
semantics while the bytes themselves remain in host-backed memory.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/
/*! The active scratch arena; its length is the Sail-owned high-water mark. */
register scratch_arena : ByteSlice = byte_slice(ScratchSource, 0, 0)

/*! Marks the start of a scratch construction. */
function scratch_begin() -> source_pointer = {
    let arena = scratch_arena;
    arena.len
}

/*! Appends materialized bytes at the cursor. */
val scratch_push_bytes : forall 'len, source_valid_length('len). (list(byte), int('len)) -> unit
function scratch_push_bytes(data, len) = {
    if len != 0 then {
        let arena = scratch_arena;
        match host_scratch_store_bytes(arena.len, data, len) {
            ByteRegionReady(expanded) => scratch_arena = expanded,
            ByteRegionFailed() => assert(false, "scratch byte append"),
        }
    }
}

/*! Appends a source-backed slice at the cursor. */
function scratch_push_slice(data : ByteSlice) -> unit = {
    if data.len != 0 then {
        let arena = scratch_arena;
        match host_scratch_store_slice(arena.len, data) {
            ByteRegionReady(expanded) => scratch_arena = expanded,
            ByteRegionFailed() => assert(false, "scratch slice append"),
        }
    }
}

/*! Appends a prefix of a fixed 32-byte value at the cursor. */
function scratch_push_b256(data : b256, len : range(0, 32)) -> unit = {
    if len != 0 then {
        let arena = scratch_arena;
        match host_scratch_store_b256(arena.len, data, len) {
            ByteRegionReady(expanded) => scratch_arena = expanded,
            ByteRegionFailed() => assert(false, "scratch fixed-byte append"),
        }
    }
}

/*! The slice covering everything pushed since `start`. */
function scratch_finish(start : source_pointer) -> ByteSlice =
    let start_offset = start in
    let arena = scratch_arena in
    let stop_offset = arena.len in
    if start_offset <= stop_offset then {
        sub_slice(arena, start, stop_offset - start_offset)
    } else {
        assert(false, "scratch finish mark");
        EMPTY_SLICE
    }

/*! Discards everything pushed since `mark`. */
function scratch_rewind(mark : source_pointer) -> unit =
    let mark_offset = mark in
    let arena = scratch_arena in
    let cursor_offset = arena.len in
    if mark_offset <= cursor_offset then {
        scratch_arena = sub_slice(arena, 0, mark);
        host_scratch_truncate(mark)
    } else {
        assert(false, "scratch rewind mark")
    }

/*! Empties the arena (per-block lifetime). */
function scratch_reset() -> unit = {
    scratch_arena = byte_slice(ScratchSource, 0, 0);
    host_scratch_truncate(0)
}
