
/*md
# Kernel scratch allocation

A bump cursor makes construction order and lifetime visible in the Sail
semantics while the bytes themselves remain in host-backed memory.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/
/*! The active scratch arena; its length is the Sail-owned high-water mark. */
register scratch_arena : ScratchSlice = EMPTY_SCRATCH_SLICE

/*! Adds two scratch lengths, preserving the selected host representation
    bound. The canonical specification uses mathematical naturals; optimized
    builds refine this equation with an explicit overflow check. */
val scratch_length_add : forall 'left 'right, source_valid_length('left) & source_valid_length('right). (
        int('left),
        int('right),
    ) -> source_length
function scratch_length_add(left, right) = left + right

/*! Marks the start of a scratch construction. */
function scratch_begin() -> source_pointer = {
    let arena = scratch_arena;
    arena.len
}

/*! Marks the start of a fixed-size construction and preallocates its append
    capacity without changing the visible cursor. Native writers may borrow
    pointers only after this operation has succeeded. */
val scratch_reserve : forall 'len, source_valid_length('len). int('len) -> source_pointer
function scratch_reserve(len) = {
    let arena = scratch_arena;
    assert(host_scratch_reserve(arena.len, len), "scratch reserve");
    arena.len
}

/*! Appends one byte without constructing a Sail list. */
function scratch_push_byte(data : byte) -> unit = {
    let arena = scratch_arena;
    match host_scratch_store_byte(arena.len, data) {
        ScratchRegionReady(expanded) => scratch_arena = expanded,
        ScratchRegionFailed() => assert(false, "scratch byte append"),
    }
}

/*! Appends a source-backed slice at the cursor. */
function stateless_input_scratch_push_slice(data : StatelessInputSlice) -> unit = {
    if data.len != 0 then {
        let arena = scratch_arena;
        match host_scratch_store_stateless_input(arena.len, data) {
            ScratchRegionReady(expanded) => scratch_arena = expanded,
            ScratchRegionFailed() => assert(false, "scratch slice append"),
        }
    }
}

function scratch_scratch_push_slice(data : ScratchSlice) -> unit = {
    if data.len != 0 then {
        let arena = scratch_arena;
        match host_scratch_store_scratch(arena.len, data) {
            ScratchRegionReady(expanded) => scratch_arena = expanded,
            ScratchRegionFailed() => assert(false, "scratch slice append"),
        }
    }
}

function log_data_scratch_push_slice(data : LogDataSlice) -> unit = {
    if data.len != 0 then {
        let arena = scratch_arena;
        match host_scratch_store_log_data(arena.len, data) {
            ScratchRegionReady(expanded) => scratch_arena = expanded,
            ScratchRegionFailed() => assert(false, "scratch slice append"),
        }
    }
}

function output_scratch_push_slice(data : OutputSlice) -> unit = {
    if data.len != 0 then {
        let arena = scratch_arena;
        match host_scratch_store_output(arena.len, data) {
            ScratchRegionReady(expanded) => scratch_arena = expanded,
            ScratchRegionFailed() => assert(false, "scratch slice append"),
        }
    }
}

overload scratch_push_slice = {
    stateless_input_scratch_push_slice,
    scratch_scratch_push_slice,
    log_data_scratch_push_slice,
    output_scratch_push_slice
}

/*! Appends a fixed 20-byte address at the cursor. */
function scratch_push_address(data : address) -> unit = {
    let arena = scratch_arena;
    match host_scratch_store_address(arena.len, data) {
        ScratchRegionReady(expanded) => scratch_arena = expanded,
        ScratchRegionFailed() => assert(false, "scratch address append"),
    }
}

/*! Appends a prefix of a fixed 32-byte value at the cursor. */
function scratch_push_b256(data : b256, len : range(0, 32)) -> unit = {
    if len != 0 then {
        let arena = scratch_arena;
        match host_scratch_store_b256(arena.len, data, len) {
            ScratchRegionReady(expanded) => scratch_arena = expanded,
            ScratchRegionFailed() => assert(false, "scratch fixed-byte append"),
        }
    }
}

/*! Appends a decreasing fixed 256-byte vector in canonical wire order. */
function scratch_push_fixed_bytes_256(data : vector(256, dec, byte)) -> unit = {
    let arena = scratch_arena;
    match host_scratch_store_fixed_bytes_256(arena.len, data) {
        ScratchRegionReady(expanded) => scratch_arena = expanded,
        ScratchRegionFailed() => assert(false, "scratch fixed-byte append"),
    }
}

/*! Appends the low `len` bytes of a word in canonical big-endian order. */
function scratch_push_word_be(data : word, len : range(0, 32)) -> unit = {
    if len != 0 then {
        let arena = scratch_arena;
        match host_scratch_store_word(arena.len, data, len) {
            ScratchRegionReady(expanded) => scratch_arena = expanded,
            ScratchRegionFailed() => assert(false, "scratch word append"),
        }
    }
}

/*! The slice covering everything pushed since `start`. */
function scratch_finish(start : source_pointer) -> ScratchSlice =
    let start_offset = start in
    let arena = scratch_arena in
    let stop_offset = arena.len in
    if start_offset <= stop_offset then {
        sub_slice(arena, start, stop_offset - start_offset)
    } else {
        assert(false, "scratch finish mark");
        EMPTY_SCRATCH_SLICE
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
    scratch_arena = EMPTY_SCRATCH_SLICE;
    host_scratch_truncate(0)
}
