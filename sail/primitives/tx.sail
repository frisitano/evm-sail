
/*md
# Transactions, logs, and receipts

The decoded transaction and its EIP-7702 authorization tuple, the log
record, and the receipt. Pure data — no registers, no externs.
*/
/*! The EIP-2718 envelope type: the single transaction discriminant
    The type-derived predicates at the end of this module read it — no
    boolean flags are stored. */
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

let OSAKA_TRANSACTION_GAS_LIMIT_VALUE : protocol_quantity = 16777216
let OSAKA_TRANSACTION_GAS_LIMIT : gas = Gas(16777216)

/*! An EIP-7702 set-code authorization tuple. RLP decoding recovers the
    authority and validates the signature (`s <= n/2`, `y_parity`, `r`);
    `valid_sig` records that result, and [process_auth][] applies the
    chain-id/nonce/code checks. */
struct Authorization = {
    valid_sig : bool, authority : address, address : address,
    nonce : account_nonce, chain_id : word,
}

/*! The source-backed EIP-4844 versioned blob hashes of a transaction. */
struct BlobHashes = {
    bytes : ByteSlice,
    count : blob_count,
}

let EMPTY_BLOB_HASHES : BlobHashes = struct { bytes = EMPTY_SLICE, count = 0 }

/*! A decoded transaction. Covers the EIP-2718 typed envelopes 0–4:
    legacy, EIP-2930 (access list), EIP-1559 (fee market), EIP-4844
    (blob), and EIP-7702 (set code); the type-specific fields are
    validity-relevant per their EIP. */
struct Transaction = {
  /* EIP-2718 envelope type (the discriminant) */
  tx_type : TxType,
  sender : address, nonce : word,
  /* typed-envelope chain id; 0 for legacy txs */
  chain_id : chain_identifier,
  gas_limit : gas, is_create : bool, recipient : address, value : word,
  /* the EIP-2718 envelope span this tx decoded from
     (the transactions-trie leaf value) */
  raw : ByteSlice,
  /* calldata / creation initcode as a host byte source
     (normally a span of the stateless input) */
  input_src : ByteSlice,
  /* EIP-2930 access-list addresses (one per entry) */
  access_list_addresses : list(address),
  access_list_address_count : item_count,
  /* EIP-2930 access-list (address, slot) pairs */
  access_list_slots : list(StorageKey),
  access_list_slot_count : item_count,
  /* EIP-1559 max_fee_per_gas (cap, for validity) */
  max_fee : word,
  /* EIP-4844 max_fee_per_blob_gas (cap, for validity) */
  max_blob_fee : word,
  /* EIP-1559 max_priority_fee_per_gas (cap, for validity) */
  max_priority_fee : word,
  /* EIP-7702 set-code authorization tuples */
  authorizations : list(Authorization),
  authorization_count : item_count,
  /* source-backed EIP-4844 BLOBHASH operands */
  blob_hashes : BlobHashes,
  /* witnessed sender public key (0x04 ++ X ++ Y span); NOT trusted -- its
     derived address must equal the signer recovered at execution */
  pubkey : ByteSlice,
  /* keccak of the signing preimage (EIP-155 / EIP-2718 domain applied),
     computed structurally at decode */
  signing_hash : hash,
  /* v (legacy, EIP-155-bearing) / y_parity (typed) */
  sig_v : word,
  sig_r : word,
  sig_s : word
}

/*! A log record (YP §4.4.1, the log series): emitting address, 0–4
    topics (`LOG0`–`LOG4`), and the data bytes. */
struct LogEntry = {
    address : address,
    topics : list(word),
    /* span into the log-data store (LogDataSource); the store persists across
       the BLOCK (per-tx reset keeps it) so receipt-held entries stay valid
       until block validation */
    data : ByteSlice,
}

/*! A transaction receipt. `success` means the top-level frame
    returned/stopped without reverting or exceptionally halting (used for
    create-tx code deposit and log inclusion). `valid` means the tx passed
    transaction validity (nonce/balance/intrinsic/floor/fee/blob/EIP-7702
    rules) and was applied to the block — a valid tx that reverts or runs
    out of gas still has `valid = true`; it is a legitimate block member
    (gas charged, nonce bumped). Block validity folds `valid`, not
    `success`: only an invalid, inapplicable tx makes the containing block
    invalid. `block_gas` is EIP-7778 block-gas accounting *without* the
    EIP-3529 refund (`max(gas_used_before_refund, calldata_floor)`); it
    differs from `gas_used` (the receipt's refunded gas) precisely by the
    refund. */
struct Receipt = {
    tx_type : TxType,
    success : bool,
    valid : bool,
    gas_used : gas,
    block_gas : gas,
    logs : list(LogEntry),
}

/*! The envelope type byte, as it appears in tx and receipt encodings. */
function tx_type_byte(t : TxType) -> byte = match t {
    LegacyTx => 0x00,
    AccessListTx => 0x01,
    FeeMarketTx => 0x02,
    BlobTx => 0x03,
    SetCodeTx => 0x04,
}

/*! Whether the type carries an EIP-2930 access list envelope (type 1). */
function tx_is_access_list(t : TxType) -> bool = match t {
    AccessListTx => true,
    _ => false,
}

/*! Whether the type is an EIP-1559-style fee-market envelope (types
    2/3/4). */
function tx_is_dynamic_fee(t : TxType) -> bool = match t {
    FeeMarketTx => true,
    BlobTx => true,
    SetCodeTx => true,
    _ => false,
}

/*! Whether the type is an EIP-4844 blob transaction (type 3). */
function tx_is_blob(t : TxType) -> bool = match t {
    BlobTx => true,
    _ => false,
}

/*! Whether the type is an EIP-7702 set-code transaction (type 4). */
function tx_is_set_code(t : TxType) -> bool = match t {
    SetCodeTx => true,
    _ => false,
}
