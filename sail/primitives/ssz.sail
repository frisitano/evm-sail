
/*md
# SSZ collections

Source-backed SSZ collections: list references and cursors that decode
elements only when a consumer reaches them.
*/
/* The SSZ schema bounds are powers of two because these fields are List
 * capacities, not largest valid indices. A list may contain exactly its
 * declared maximum number of elements. */
/*! Maximum transactions in the execution-payload SSZ list. Provenance:
    Bellatrix `MAX_TRANSACTIONS_PER_PAYLOAD` and Amsterdam
    `SszExecutionPayload.transactions`. */
type transaction_count_bound : Int = 2 ^ 20
/*! Maximum withdrawals in the execution-payload SSZ list. Provenance:
    Capella `MAX_WITHDRAWALS_PER_PAYLOAD` and Amsterdam
    `SszExecutionPayload.withdrawals`. */
type withdrawal_count_bound : Int = 2 ^ 4
/*! Maximum blob commitments in the stateless-input SSZ list. */
type blob_commitment_count_bound : Int = 2 ^ 12
/*! Maximum witness trie nodes in the stateless-input SSZ list. */
type witness_node_count_bound : Int = 2 ^ 22
/*! Maximum witness code entries in the stateless-input SSZ list. */
type witness_code_count_bound : Int = 2 ^ 18
/*! Maximum witness headers in the stateless-input SSZ list. */
type witness_header_count_bound : Int = 2 ^ 8
/*! Maximum witnessed public keys in the stateless-input SSZ list. */
type public_key_count_bound : Int = 2 ^ 15
/*! Maximum deposit requests in their stateless-input SSZ list. */
type deposit_request_count_bound : Int = 2 ^ 13
/*! Maximum withdrawal requests in their stateless-input SSZ list. */
type withdrawal_request_count_bound : Int = 2 ^ 4
/*! Maximum consolidation requests in their stateless-input SSZ list. */
type consolidation_request_count_bound : Int = 2 ^ 1
/*! Maximum builder deposit requests in their stateless-input SSZ list. */
type builder_deposit_request_count_bound : Int = 2 ^ 6
/*! Maximum builder exit requests in their stateless-input SSZ list. */
type builder_exit_request_count_bound : Int = 2 ^ 4

/*! Maximum byte length of execution-payload extra data. Provenance:
    consensus `MAX_EXTRA_DATA_BYTES` and Amsterdam
    `SszExecutionPayload.extra_data`. */
type extra_data_length_bound : Int = 2 ^ 5
/*! Maximum byte length of one encoded transaction envelope. Provenance:
    Amsterdam `MAX_BYTES_PER_TRANSACTION` in
    `SszExecutionPayload.transactions`. */
type transaction_length_bound : Int = 2 ^ 30
/*! Maximum byte length of the block access list. Provenance: Amsterdam
    `SszExecutionPayload.block_access_list` uses
    `ByteList[MAX_BYTES_PER_TRANSACTION]`. */
type block_access_list_length_bound : Int = 2 ^ 30
/*! Maximum byte length of one witnessed trie node. */
type witness_node_length_bound : Int = 2 ^ 10

/* The stateless witness envelope admits 65,536-byte code entries. This is an
 * SSZ/host structural bound, independent of every fork's deployment limit. */
/*! Maximum byte length of one witnessed code entry. */
type witness_code_length_bound : Int = 2 ^ 16
/*! Maximum byte length of one witnessed parent header. */
type witness_header_length_bound : Int = 2 ^ 10

/*! Number of transactions in a schema-valid execution payload. */
type transaction_count = range(0, transaction_count_bound)
/*! Zero-based transaction position in a non-empty execution payload. */
type transaction_index = range(0, transaction_count_bound - 1)
/*! EIP-7928 change position: pre-execution system calls use zero,
    transactions use their one-based position, and post-execution system calls
    use `transaction_count + 1`. */
type block_access_index = range(0, transaction_count_bound + 1)

/*! A reference to an SSZ list: its byte span and schema-bounded element
    count. */
struct BoundedSszListRef('maximum : Int), source_valid_length('maximum) = {
    bytes : StatelessInputSlice,
    count : range(0, 'maximum),
    /* Zero means that this generic view carries no per-item schema bound. */
    max_item_length : source_length,
}

/*! A sequential cursor over a variable-element SSZ list. */
struct BoundedSszListCursor('maximum : Int), source_valid_length('maximum) = {
    items : BoundedSszListRef('maximum),
    index : range(0, 'maximum),
    current : source_pointer,
}

/*! A schema-bounded source reference to encoded transaction envelopes. */
type TransactionListRef = BoundedSszListRef(transaction_count_bound)
/*! A sequential cursor over encoded transaction envelopes. */
type TransactionListCursor = BoundedSszListCursor(transaction_count_bound)
/*! A schema-bounded source reference to withdrawals. */
type WithdrawalListRef = BoundedSszListRef(withdrawal_count_bound)
/*! A sequential cursor over withdrawals. */
type WithdrawalListCursor = BoundedSszListCursor(withdrawal_count_bound)
/*! A schema-bounded source reference to witnessed trie nodes. */
type WitnessNodeListRef = BoundedSszListRef(witness_node_count_bound)
/*! A sequential cursor over witnessed trie nodes. */
type WitnessNodeListCursor = BoundedSszListCursor(witness_node_count_bound)
/*! A schema-bounded source reference to witnessed code entries. */
type WitnessCodeListRef = BoundedSszListRef(witness_code_count_bound)
/*! A sequential cursor over witnessed code entries. */
type WitnessCodeListCursor = BoundedSszListCursor(witness_code_count_bound)
/*! A schema-bounded source reference to witnessed parent headers. */
type WitnessHeaderListRef = BoundedSszListRef(witness_header_count_bound)
/*! A sequential cursor over witnessed parent headers. */
type WitnessHeaderListCursor = BoundedSszListCursor(witness_header_count_bound)
