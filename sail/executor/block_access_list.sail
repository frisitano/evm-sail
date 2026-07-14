/*md
# The block access list (EIP-7928)

Reconstruction of the EIP-7928 block access list from the recorder's
per-account change streams, as source-backed RLP that is compared against
the payload's claimed bytes.
*/

/*! The reconstructed BAL: its RLP and its item count (bounded by
    `gas_limit / 2000`). */
struct EncodedBlockAccessList = {
    rlp : RlpBuilder,
    item_count : int,
}

/*! One `[tx_index, value]` change pair as RLP. */
function bal_index_word(index : bits(64), value : word) -> RlpBuilder = {
    var content = rlp_builder_encoded(rlp_int(unsigned(index)));
    content = rlp_builder_append_encoded(content, rlp_int_word(value));
    rlp_builder_list(content)
}

function bal_index_nonce(index : bits(64), value : bits(64)) -> RlpBuilder = {
    var content = rlp_builder_encoded(rlp_int(unsigned(index)));
    content = rlp_builder_append_encoded(content, rlp_int(unsigned(value)));
    rlp_builder_list(content)
}

function bal_index_code(index : bits(64), code_hash : hash) -> RlpBuilder = {
    var content = rlp_builder_encoded(rlp_int(unsigned(index)));
    let code = code_db_resolve(code_hash);
    content = rlp_builder_append(content, rlp_builder_slice(code.bytes));
    rlp_builder_list(content)
}

/*! The per-account storage-change list, slot-major. */
function bal_storage_changes(account : bits(64)) -> (RlpBuilder, int) = {
    let count = unsigned(bal_storage_change_count(account));
    var content = EMPTY_RLP_BUILDER;
    var slot_count : int = 0;
    var i : int = 0;
    while i < count do {
        let slot = bal_storage_change_slot(account, get_slice_int(64, i, 0));
        var changes = EMPTY_RLP_BUILDER;
        var j : int = i;
        while (j < count) & (bal_storage_change_slot(account, get_slice_int(64, j, 0)) == slot) do {
            let index = bal_storage_change_index(account, get_slice_int(64, j, 0));
            var last : int = j;
            while   (last + 1 < count)
                  & (bal_storage_change_slot(account, get_slice_int(64, last + 1, 0)) == slot)
                  & (bal_storage_change_index(account, get_slice_int(64, last + 1, 0)) == index) do {
                last = last + 1
            };
            changes = rlp_builder_append(
                changes,
                bal_index_word(index, bal_storage_change_value(account, get_slice_int(64, last, 0))),
            );
            j = last + 1
        };
        var slot_content = rlp_builder_encoded(rlp_int_word(slot));
        slot_content = rlp_builder_append(slot_content, rlp_builder_list(changes));
        content = rlp_builder_append(content, rlp_builder_list(slot_content));
        slot_count = slot_count + 1;
        i = j
    };
    (rlp_builder_list(content), slot_count)
}

function bal_storage_reads(account : bits(64)) -> (RlpBuilder, int) = {
    let read_count = unsigned(bal_storage_read_count(account));
    let change_count = unsigned(bal_storage_change_count(account));
    var content = EMPTY_RLP_BUILDER;
    var included : int = 0;
    var change : int = 0;
    var read : int = 0;
    while read < read_count do {
        let slot = bal_storage_read_slot(account, get_slice_int(64, read, 0));
        var next_read : int = read + 1;
        while (next_read < read_count) & (bal_storage_read_slot(account, get_slice_int(64, next_read, 0)) == slot) do {
            next_read = next_read + 1
        };
        while (change < change_count) &
            (unsigned(bal_storage_change_slot(account, get_slice_int(64, change, 0))) < unsigned(slot)) do {
            change = change + 1
        };
        let changed = (change < change_count) & (bal_storage_change_slot(account, get_slice_int(64, change, 0)) == slot);
        if not_bool(changed) then {
            content = rlp_builder_append_encoded(content, rlp_int_word(slot));
            included = included + 1
        };
        read = next_read
    };
    (rlp_builder_list(content), included)
}

function bal_balance_changes(account : bits(64)) -> RlpBuilder = {
    let count = unsigned(bal_balance_change_count(account));
    var content = EMPTY_RLP_BUILDER;
    var i : int = 0;
    while i < count do {
        let index = bal_balance_change_index(account, get_slice_int(64, i, 0));
        var last : int = i;
        while (last + 1 < count) & (bal_balance_change_index(account, get_slice_int(64, last + 1, 0)) == index) do {
            last = last + 1
        };
        content = rlp_builder_append(
            content,
            bal_index_word(index, bal_balance_change_value(account, get_slice_int(64, last, 0))),
        );
        i = last + 1
    };
    rlp_builder_list(content)
}

function bal_nonce_changes(account : bits(64)) -> RlpBuilder = {
    let count = unsigned(bal_nonce_change_count(account));
    var content = EMPTY_RLP_BUILDER;
    var i : int = 0;
    while i < count do {
        let index = bal_nonce_change_index(account, get_slice_int(64, i, 0));
        var maximum = bal_nonce_change_value(account, get_slice_int(64, i, 0));
        var next : int = i + 1;
        while (next < count) & (bal_nonce_change_index(account, get_slice_int(64, next, 0)) == index) do {
            let value = bal_nonce_change_value(account, get_slice_int(64, next, 0));
            if unsigned(value) > unsigned(maximum) then {
                maximum = value
            };
            next = next + 1
        };
        content = rlp_builder_append(content, bal_index_nonce(index, maximum));
        i = next
    };
    rlp_builder_list(content)
}

function bal_code_changes(account : bits(64)) -> RlpBuilder = {
    let count = unsigned(bal_code_change_count(account));
    var content = EMPTY_RLP_BUILDER;
    var i : int = 0;
    while i < count do {
        let index = bal_code_change_index(account, get_slice_int(64, i, 0));
        var last : int = i;
        while (last + 1 < count) & (bal_code_change_index(account, get_slice_int(64, last + 1, 0)) == index) do {
            last = last + 1
        };
        content = rlp_builder_append(
            content,
            bal_index_code(index, bal_code_change_hash(account, get_slice_int(64, last, 0))),
        );
        i = last + 1
    };
    rlp_builder_list(content)
}

function encode_block_access_list() -> EncodedBlockAccessList = {
    bal_prepare();
    let account_count = unsigned(bal_account_count());
    var content = EMPTY_RLP_BUILDER;
    var item_count : int = account_count;
    var i : int = 0;
    while i < account_count do {
        let account = get_slice_int(64, i, 0);
        let (storage_changes, changed_slots) = bal_storage_changes(account);
        let (storage_reads, read_slots) = bal_storage_reads(account);
        var account_content = rlp_builder_encoded(rlp_addr(bal_account_address(account)));
        account_content = rlp_builder_append(account_content, storage_changes);
        account_content = rlp_builder_append(account_content, storage_reads);
        account_content = rlp_builder_append(account_content, bal_balance_changes(account));
        account_content = rlp_builder_append(account_content, bal_nonce_changes(account));
        account_content = rlp_builder_append(account_content, bal_code_changes(account));
        content = rlp_builder_append(content, rlp_builder_list(account_content));
        item_count = item_count + changed_slots + read_slots;
        i = i + 1
    };
    struct { rlp = rlp_builder_list(content), item_count = item_count }
}
