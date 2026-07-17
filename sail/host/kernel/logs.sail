/*md
# State: logs and refunds

Log emission (YP §4.4.1) — including the EIP-7708 transfer and burn
logs — and the gas-refund counter.
*/

/*! Appends a log record (YP §4.4.1) to the transaction's log series. */
function k_log(a : address, topics : list(word), data : Bytes) -> unit = {
    log_append(a, topics, data)
}

/*! Emits the EIP-7708 transfer log for a nonzero, non-self value
    transfer (Amsterdam onward). */
function k_emit_transfer_log(src : address, dst : address, v : word) -> unit = {
    if (k_fork < Amsterdam) | word_is_zero(v) | (src == dst) then {
        ()
    } else {
        log_append(
            EIP7708_SYSTEM_ADDRESS,
            [|EIP7708_TRANSFER_TOPIC, address_to_word(src), address_to_word(dst)|],
            bytes_list(word_to_bytes32(v), WORD_BYTE_LENGTH),
        )
    }
}

/*! Emits the EIP-7708 burn log when a selfdestruct deletion burns a
    nonzero balance (Amsterdam onward). */
function k_emit_burn_log(a : address, v : word) -> unit = {
    if (k_fork < Amsterdam) | word_is_zero(v) then {
        ()
    } else {
        log_append(
            EIP7708_SYSTEM_ADDRESS,
            [|EIP7708_BURN_TOPIC, address_to_word(a)|],
            bytes_list(word_to_bytes32(v), WORD_BYTE_LENGTH),
        )
    }
}
