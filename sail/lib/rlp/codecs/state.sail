
/*md
# State trie RLP codec

Ethereum account and storage tries over the generic MPT core: secure-trie
reads for stateless execution, and the post-state root computation
(YP §4.1).
*/
/*! Decodes an account trie leaf — `rlp([nonce, balance, storage_root,
    code_hash])` — into an [AccountInfo][type-AccountInfo]; empty
    root/hash fields decode to their empty-sentinel digests. */
function decode_state_account(value : StatelessInputSlice) -> AccountInfo = {
    let fields = rlp_node_cursor(value);
    let nonce = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, nonce.source.len);
    let balance = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, balance.source.len);
    let storage = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, storage.source.len);
    let code = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, code.source.len);
    rlp_cursor_expect_end(fields);
    let storage_root = if storage.content_len == 0 then {
            EMPTY_TRIE_ROOT
        } else {
            word_to_hash(rlp_decode_word(storage))
        };
    let code_hash = if code.content_len == 0 then {
            KECCAK_EMPTY
        } else {
            word_to_hash(rlp_decode_word(code))
        };
    struct {
        nonce = rlp_decode_uint64(nonce),
        balance = rlp_decode_u256(balance),
        storage_root = storage_root,
        code_hash = code_hash,
    }
}

/*md
## State leaf encoding
*/
/*! Encodes a nonzero storage value as its minimal RLP integer leaf payload. */
function encode_storage_value(value : word) -> ScratchSlice = {
    let encoded_len = rlp_uint_word_size(value);
    let encoder = rlp_encoder_begin(encoded_len);
    rlp_write_uint_word(value);
    rlp_encoder_finish(encoder)
}

/*! Encodes an account trie leaf with its recomputed storage root. */
function encode_state_account(info : AccountInfo, storage_root : hash) -> ScratchSlice = {
    let nonce_length = rlp_uint_size(info.nonce);
    let balance_length = rlp_uint_word_size(info.balance);
    let storage_root_length = rlp_word_size();
    let code_hash_length = rlp_word_size();
    let content_len = nonce_length + balance_length + storage_root_length + code_hash_length;
    let encoder = rlp_encoder_begin(rlp_list_size(content_len));
    rlp_write_list_prefix(content_len);
    rlp_write_uint(info.nonce);
    rlp_write_uint_word(info.balance);
    rlp_write_word(hash_to_word(storage_root));
    rlp_write_word(hash_to_word(info.code_hash));
    rlp_encoder_finish(encoder)
}
