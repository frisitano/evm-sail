function k_log(a : address, topics : list(word), data : Bytes) -> unit = {
    log_append(a, topics, data)
}

function k_refund_add(delta : int) -> unit = {
    journal_push(JRefund(k_refund));
    k_refund = to_i64(k_refund + delta)
}

function k_emit_transfer_log(src : address, dst : address, v : word) -> unit = {
    if (k_fork < Amsterdam) | word_is_zero(v) | (src == dst) then {
        ()
    } else {
        log_append(
            EIP7708_SYSTEM_ADDRESS,
            [|EIP7708_TRANSFER_TOPIC, address_to_word(src), address_to_word(dst)|],
            BytesList(word_to_bytes32(v)),
        )
    }
}

function k_emit_burn_log(a : address, v : word) -> unit = {
    if (k_fork < Amsterdam) | word_is_zero(v) then {
        ()
    } else {
        log_append(EIP7708_SYSTEM_ADDRESS, [|EIP7708_BURN_TOPIC, address_to_word(a)|], BytesList(word_to_bytes32(v)))
    }
}
