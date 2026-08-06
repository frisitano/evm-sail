/* Optimized C overrides for `sail/lib/rlp/codecs/receipts.sail`. */

/*
 * The optimized receipt encoder hashes its retained C log range directly into
 * the final reserved RLP span and updates the block's 32-word accumulator in
 * the same pass. There is no per-receipt LogsBloom aggregate or 256-byte copy.
 */
val optimized_scratch_store_receipt_logs_bloom = impure { c: "scratch_store_receipt_logs_bloom" } : forall 'off,
                                                                                                      host_valid_access(
                                                                                                          'off,
                                                                                                      ). (
        int('off),
        log_store_index,
        log_store_index,
    ) -> ScratchSliceLength('off + 256)

function receipt_write_logs_bloom(receipt : Receipt) -> unit = {
    rlp_write_string_prefix(LOGS_BLOOM_BYTE_LENGTH, 0x00);
    let arena = scratch_arena;
    scratch_arena = optimized_scratch_store_receipt_logs_bloom(
        arena.len,
        receipt.logs.start,
        receipt.logs.count,
    )
}
