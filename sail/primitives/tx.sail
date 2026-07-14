/*md
# Transactions, logs, and receipts

The decoded transaction and its EIP-7702 authorization tuple, the log
record, and the receipt. Pure data — no registers, no externs.
*/

/*! The EIP-2718 envelope type: the single transaction discriminant
    (revm's `TransactionType`; execution-specs uses one dataclass per
    variant). The type-derived predicates at the end of this module read
    it — no boolean flags are stored. */
enum TxType = {
    LegacyTx, /* type 0: pre-EIP-2718 RLP list  */
    AccessListTx, /* type 1: EIP-2930 access list   */
    FeeMarketTx, /* type 2: EIP-1559 fee market    */
    BlobTx, /* type 3: EIP-4844 blob          */
    SetCodeTx, /* type 4: EIP-7702 set code      */
}

/*! An EIP-7702 set-code authorization tuple. RLP decoding recovers the
    authority and validates the signature (`s <= n/2`, `y_parity`, `r`);
    `valid_sig` records that result, and [process_auth][] applies the
    chain-id/nonce/code checks. */
struct Authorization = {
    valid_sig : bool, authority : address, address : address,
    nonce : int, chain_id : int,
}

/*! The source-backed EIP-4844 versioned blob hashes of a transaction. */
struct BlobHashes = {
    bytes : ByteSlice,
    count : int,
}

let EMPTY_BLOB_HASHES : BlobHashes = struct { bytes = EMPTY_SLICE, count = 0 }

/*! A decoded transaction. Covers the EIP-2718 typed envelopes 0–4:
    legacy, EIP-2930 (access list), EIP-1559 (fee market), EIP-4844
    (blob), and EIP-7702 (set code); the type-specific fields are
    validity-relevant per their EIP. */
struct Transaction = {
  tx_type : TxType,          /* EIP-2718 envelope type (the discriminant)    */
  sender : address, nonce : int,
  chain_id : int,             /* typed-envelope chain id; 0 for legacy txs    */
  gas_limit : gas, is_create : bool, recipient : address, value : word,
  raw : ByteSlice,             /* the EIP-2718 envelope span this tx decoded from
                                  (the transactions-trie leaf value)             */
  input_src : ByteSlice,       /* calldata / creation initcode as a host byte source
                                  (normally a span of the stateless input)          */
  access_list_addresses : list(address),   /* EIP-2930 access-list addresses (one per entry) */
  access_list_address_count : int,
  access_list_slots  : list(StorageKey),   /* EIP-2930 access-list (address, slot) pairs      */
  access_list_slot_count : int,
  max_fee : word,             /* EIP-1559 max_fee_per_gas (cap, for validity)     */
  max_blob_fee : word,        /* EIP-4844 max_fee_per_blob_gas (cap, for validity)*/
  max_priority_fee : word,        /* EIP-1559 max_priority_fee_per_gas (cap, for validity) */
  authorizations : list(Authorization), /* EIP-7702 set-code authorization tuples      */
  authorization_count : int,
  blob_hashes : BlobHashes,   /* source-backed EIP-4844 BLOBHASH operands         */
  pubkey : ByteSlice,          /* witnessed sender public key (0x04 ++ X ++ Y span);
                                  NOT trusted -- verified against the signature at
                                  execution (EELS recover_sender_from_public_key) */
  signing_hash : hash,         /* keccak of the signing preimage (EIP-155 / EIP-2718
                                  domain applied), computed structurally at decode  */
  sig_v : int,                 /* v (legacy, EIP-155-bearing) / y_parity (typed)   */
  sig_r : word,
  sig_s : word
}

/*! A log record (YP §4.4.1, the log series): emitting address, 0–4
    topics (`LOG0`–`LOG4`), and the data bytes. */
struct LogEntry = {
    address : address,
    topics : list(word),
    data : ByteSlice, /* span into the C log-data arena (LogDataSource); the arena
                         persists across the BLOCK (per-tx reset keeps it) so
                         receipt-held entries stay valid until block validation */
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
struct Receipt = { tx_type : TxType, success : bool, valid : bool, gas_used : gas, block_gas : gas, logs : list(LogEntry) }

/*! The envelope type byte, as it appears in tx and receipt encodings. */
function tx_type_byte(t : TxType) -> int = match t {
    LegacyTx => 0,
    AccessListTx => 1,
    FeeMarketTx => 2,
    BlobTx => 3,
    SetCodeTx => 4,
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
