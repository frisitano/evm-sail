
/*md
# Transactions, logs, and receipts

The decoded transaction and its EIP-7702 authorization tuple, the log
record, and the receipt. Pure data — no registers, no externs.
*/
/*! The EIP-2718 envelope type: the single transaction discriminant. Its
    closed semantic descriptor below derives fork and feature requirements;
    transactions do not store redundant boolean flags. */
enum TxType = {
    /* type 0: pre-EIP-2718 RLP list */
    LegacyTx,
    /* type 1: EIP-2930 access list */
    AccessListTx,
    /* type 2: EIP-1559 fee market */
    FeeMarketTx,
    /* type 3: EIP-4844 blob */
    BlobTx,
    /* type 4: EIP-7702 set code */
    SetCodeTx,
}

/*! The canonical EIP-2718 wire discriminant. Keeping the reversible encoding
    as a mapping gives decoders and encoders one authoritative definition:
    encoders apply it forwards, while decoders validate and apply it
    backwards before dispatching on the semantic transaction type. */
mapping tx_envelope_type : TxType <-> byte = {
    LegacyTx <-> 0x00,
    AccessListTx <-> 0x01,
    FeeMarketTx <-> 0x02,
    BlobTx <-> 0x03,
    SetCodeTx <-> 0x04,
}

/*! The two transaction-signature encodings. Legacy transactions use the
    original/EIP-155 `v` domain; every EIP-2718 typed envelope carries an
    explicit zero-or-one parity. */
enum TxSignatureScheme = { LegacySignature, TypedSignature }

/*! Protocol requirements determined solely by an EIP-2718 envelope type.
    Computing this descriptor once prevents validation from repeatedly
    dispatching on the same closed transaction-type algebra. */
struct TxTypeSemantics = {
    minimum_fork : Fork,
    signature : TxSignatureScheme,
    blob : bool,
    set_code : bool,
}

/*! Derives the protocol requirements of one transaction envelope. */
function tx_type_semantics(t : TxType) -> TxTypeSemantics = match t {
    LegacyTx => struct {
        minimum_fork = Frontier,
        signature = LegacySignature,
        blob = false,
        set_code = false,
    },
    AccessListTx => struct {
        minimum_fork = Berlin,
        signature = TypedSignature,
        blob = false,
        set_code = false,
    },
    FeeMarketTx => struct {
        minimum_fork = London,
        signature = TypedSignature,
        blob = false,
        set_code = false,
    },
    BlobTx => struct {
        minimum_fork = Cancun,
        signature = TypedSignature,
        blob = true,
        set_code = false,
    },
    SetCodeTx => struct {
        minimum_fork = Prague,
        signature = TypedSignature,
        blob = false,
        set_code = true,
    },
}

/*md
## Constants

The empty source-backed collection constants initialize transactions without
blob hashes, access-list entries, or authorization tuples.
*/

/*! An EIP-7702 set-code authorization tuple. RLP decoding recovers the
    authority and validates the signature (`s <= n/2`, `y_parity`, `r`);
    `valid_sig` records that result, and [process_auth][] applies the
    chain-id/nonce/code checks. */
struct Authorization = {
    valid_sig : bool, authority : address, address : address,
    nonce : account_nonce, chain_id : word,
}

/*! The source-backed EIP-4844 versioned blob hashes of a transaction. The
    limit index records which fork-selected transaction range admitted the
    observed count. It is carried by the enclosing transaction rather than
    hidden in a nested existential, so consumers retain `count <= limit`. */
struct BlobHashesFields('limit : Int), transaction_blob_limit_value('limit) = {
    bytes : StatelessInputSlice,
    count : transaction_blob_count('limit),
}

let EMPTY_BLOB_HASHES : BlobHashesFields(blob_schedule_inactive_count) = struct {
        bytes = EMPTY_STATELESS_INPUT_SLICE,
        count = 0,
    } :
        BlobHashesFields(blob_schedule_inactive_count)

/*! A byte span contained by one SSZ transaction envelope. This structural
    bound is independent of fork-specific calldata and initcode limits. */
type TransactionInputSlice = {
    'off 'len,
    source_valid_range('off, 'len) & 0 <= 'len & 'len <= transaction_length_bound.
    StatelessInputSliceFields('off, 'len)
}

/*! A collection count whose entries are encoded inside one transaction. */
type transaction_item_count = range(0, transaction_length_bound)

/*! A validated EIP-2930 access-list content span. Entries remain in their
    canonical RLP encoding and are consumed with a cursor when prewarming. */
struct AccessListRef = {
    encoded : StatelessInputSlice,
    address_count : transaction_item_count,
    slot_count : transaction_item_count,
}

let EMPTY_ACCESS_LIST_REF : AccessListRef = struct {
        encoded = EMPTY_STATELESS_INPUT_SLICE,
        address_count = 0,
        slot_count = 0,
    }

/*! A validated EIP-7702 authorization-list content span. Its dependent count
    remains visible to execution, so recursive authorization processing can
    derive refund bounds from the number of admitted tuples. */
struct AuthorizationListRefFields(
    'count : Int,
), 0 <= 'count & 'count <= transaction_length_bound = {
    encoded : StatelessInputSlice,
    count : int('count),
}

type AuthorizationListRef = {
    'count,
    0 <= 'count & 'count <= transaction_length_bound.
    AuthorizationListRefFields('count)
}

val authorization_list_ref : forall 'count, 0 <= 'count & 'count <= transaction_length_bound. (
        StatelessInputSlice,
        int('count),
    ) -> AuthorizationListRefFields('count)
function authorization_list_ref(encoded, count) =
    struct { encoded = encoded, count = count }

let EMPTY_AUTHORIZATION_LIST_REF : AuthorizationListRef = authorization_list_ref(EMPTY_STATELESS_INPUT_SLICE, 0)

/*! A byte length contained by one SSZ transaction envelope. */
type transaction_byte_length = range(0, transaction_length_bound)

/*! The greatest EIP-2028 calldata charge admitted by an SSZ transaction. */
type transaction_calldata_cost = range(0, 16 * transaction_length_bound)

/*! The greatest EIP-7623 calldata floor admitted by an SSZ transaction. */
type transaction_calldata_floor_cost = range(0, 40 * transaction_length_bound + 21000)

/*! The greatest EIP-3860 initcode charge admitted by an SSZ transaction. */
type transaction_initcode_cost = range(0, 2 * div(transaction_length_bound + 31, 32))

/*! A decoded transaction. Covers the EIP-2718 typed envelopes 0–4:
    legacy, EIP-2930 (access list), EIP-1559 (fee market), EIP-4844
    (blob), and EIP-7702 (set code); the type-specific fields are
    validity-relevant per their EIP. */
struct TransactionFields('blob_limit : Int), transaction_blob_limit_value('blob_limit) = {
  /* EIP-2718 envelope type (the discriminant) */
  tx_type : TxType,
  sender : address, nonce : word,
  /* typed-envelope chain id; 0 for legacy txs */
  chain_id : chain_identifier,
  gas_limit : transaction_gas, is_create : bool, recipient : address, value : word,
  /* the EIP-2718 envelope span this tx decoded from
     (the transactions-trie leaf value) */
  raw : StatelessInputSlice,
  /* calldata / creation initcode as a host byte source
     (normally a span of the stateless input) */
  input_src : TransactionInputSlice,
  /* source-backed EIP-2930 access-list entries */
  access_list : AccessListRef,
  /* EIP-1559 max_fee_per_gas (cap, for validity) */
  max_fee : word,
  /* EIP-4844 max_fee_per_blob_gas (cap, for validity) */
  max_blob_fee : word,
  /* EIP-1559 max_priority_fee_per_gas (cap, for validity) */
  max_priority_fee : word,
  /* source-backed EIP-7702 set-code authorization tuples */
  authorizations : AuthorizationListRef,
  /* source-backed EIP-4844 BLOBHASH operands */
  blob_hashes : BlobHashesFields('blob_limit),
  /* witnessed sender public key (0x04 ++ X ++ Y span); NOT trusted -- its
     derived address must equal the signer recovered at execution */
  pubkey : StatelessInputSlice,
  /* keccak of the signing preimage (EIP-155 / EIP-2718 domain applied),
     computed structurally at decode */
  signing_hash : hash,
  /* v (legacy, EIP-155-bearing) / y_parity (typed) */
  sig_v : word,
  sig_r : word,
  sig_s : word
}

/*! A decoded transaction retains the fork-selected blob-count limit that was
    applied while decoding its envelope. Non-blob transactions carry the
    inactive zero limit. */
type Transaction = {
    'blob_limit,
    transaction_blob_limit_value('blob_limit).
    TransactionFields('blob_limit)
}

/*! The bounded topic operands of one `LOG0`–`LOG4` instruction. Keeping the
    arity in the constructor avoids allocating a Sail list for at most four
    stack words. */
union LogTopics = {
    /* `LOG0`: no topics */
    LogTopics0 : unit,
    /* `LOG1`: one topic */
    LogTopics1 : word,
    /* `LOG2`: two topics in stack-pop order */
    LogTopics2 : (word, word),
    /* `LOG3`: three topics in stack-pop order */
    LogTopics3 : (word, word, word),
    /* `LOG4`: four topics in stack-pop order */
    LogTopics4 : (word, word, word, word),
}

/*! A host log-store cursor. It is an opaque bounded collection position,
    not an EVM quantity; the host rejects positions outside the current
    block's retained log series. */
type log_store_index_bound : Int = 2 ^ 64 - 1
type log_store_index = range(0, log_store_index_bound)

/*! Advances a valid log cursor without fixed-width wrapping. */
function log_store_index_increment(value : log_store_index) -> log_store_index =
    if value < sizeof(log_store_index_bound) then {
        value + 1
    } else {
        assert(false, "log store index overflow");
        0
    }

/*! Adds a relative log offset to its series start without wrapping. */
function log_store_index_add(left : log_store_index, right : log_store_index) -> log_store_index =
    if right <= sizeof(log_store_index_bound) - left then {
        left + right
    } else {
        assert(false, "log store index overflow");
        0
    }

/*! A consecutive transaction-local view into the block-lifetime host log
    store. Reverted frame records are removed before this view is captured. */
struct LogSeriesRef = {
    start : log_store_index,
    count : log_store_index,
}

/*! A transaction receipt. `success` means the top-level frame
    returned/stopped without reverting or exceptionally halting (used for
    create-tx code deposit and log inclusion). Transaction-invalidity is an
    invalid-block exception and therefore never produces a receipt. A valid
    transaction that reverts or runs out of gas still has `success = false`
    and produces a legitimate receipt. `execution_gas` is the execution-gas
    contribution to Amsterdam
    block accounting *without* the EIP-3529 refund
    (`max(execution_gas_before_refund, calldata_floor)`); it
    differs from `gas_used` (the receipt's refunded gas) precisely by the
    refund and excludes `state_gas`. */
type receipt_gas_relation(
    'limit : Int,
    'regular_limit : Int,
    'gas_used : Int,
    'execution_gas : Int,
    'state_gas : Int,
) -> Bool =
       0 <= 'limit
    &  'limit <= block_gas_limit_bound
    &  0 <= 'regular_limit
    &  'regular_limit <= 'limit
    &  0 <= 'gas_used
    &  'gas_used <= 'limit
    &  0 <= 'execution_gas
    &  'execution_gas <= 'regular_limit
    &  0 <= 'state_gas
    &  'state_gas <= 'limit
    &  'gas_used <= 'execution_gas + 'state_gas

/*! The concrete receipt record indexed by its admitted and consumed gas
    quantities. */
struct ReceiptFields(
    'limit : Int,
    'regular_limit : Int,
    'gas_used : Int,
    'execution_gas : Int,
    'state_gas : Int,
), receipt_gas_relation(
    'limit,
    'regular_limit,
    'gas_used,
    'execution_gas,
    'state_gas,
) = {
    tx_type : TxType,
    success : bool,
    gas_used : int('gas_used),
    execution_gas : int('execution_gas),
    state_gas : int('state_gas),
    logs : LogSeriesRef,
}

/*! A receipt retaining the transaction limits that bounded each gas
    contribution. */
type ReceiptWithin(
    'limit : Int,
    'regular_limit : Int,
) = {
    'gas_used 'execution_gas 'state_gas,
    receipt_gas_relation(
        'limit,
        'regular_limit,
        'gas_used,
        'execution_gas,
        'state_gas,
    ).
    ReceiptFields(
        'limit,
        'regular_limit,
        'gas_used,
        'execution_gas,
        'state_gas,
    )
}

/*! A receipt whose originating transaction limit is not needed. */
type Receipt = {
    'limit 'regular_limit 'gas_used 'execution_gas 'state_gas,
    receipt_gas_relation(
        'limit,
        'regular_limit,
        'gas_used,
        'execution_gas,
        'state_gas,
    ).
    ReceiptFields(
        'limit,
        'regular_limit,
        'gas_used,
        'execution_gas,
        'state_gas,
    )
}
