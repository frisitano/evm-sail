/* ======================================================================== */
/* PRIMITIVES: TRANSACTION + CALL-FRAME TYPES                               */
/*                                                                          */
/* The decoded transaction and its EIP-7702 authorization tuple, the log    */
/* record, and the receipt. Pure data -- no registers, no externs.          */
/* ======================================================================== */

/* An EIP-7702 set-code authorization tuple. The runner pre-recovers the
 * authority and validates the signature (s <= n/2, y_parity, r); valid_sig
 * records that result, and process_auth applies the chain-id/nonce/code checks. */
struct Authorization = {
    valid_sig : bool, authority : address, address : address,
    nonce : int, chain_id : int,
}

/* A transaction. Covers the EIP-2718 typed envelopes 0..4: legacy, EIP-2930
 * (access list), EIP-1559 (fee market), EIP-4844 (blob), EIP-7702 (set code);
 * the type-specific fields below are validity-relevant per their EIP. */
struct Transaction = {
  tx_type : int,             /* EIP-2718 envelope type: 0 legacy, 1..4 typed */
  sender : address, nonce : int, gas_price : word, priority_fee : word,
  gas_limit : gas, is_create : bool, recipient : address, value : word,
  input_src : ByteSlice,       /* calldata / creation initcode as a host byte source
                                  (the witness span for a stateless tx, or the
                                  already-staged native-runner input) -- staged in
                                  one op, never streamed byte-by-byte in the guest */
  access_list_addresses : list(address),   /* EIP-2930 access-list addresses (one per entry) */
  access_list_slots  : list(StorageKey),   /* EIP-2930 access-list (address, slot) pairs      */
  is_access_list : bool,      /* type-1 (EIP-2930) envelope, even if the list is empty */
  is_dynamic_fee : bool,      /* EIP-1559-style fee-market envelope (types 2/3/4) */
  blob_count : int,           /* EIP-4844 number of blob versioned hashes         */
  max_fee : word,             /* EIP-1559 max_fee_per_gas (cap, for validity)     */
  max_blob_fee : word,        /* EIP-4844 max_fee_per_blob_gas (cap, for validity)*/
  max_priority_fee : word,        /* EIP-1559 max_priority_fee_per_gas (cap, for validity) */
  is_set_code : bool,          /* type-4 (EIP-7702) envelope (even with 0 tuples)  */
  authorizations : list(Authorization), /* EIP-7702 set-code authorization tuples      */
  is_blob : bool,             /* EIP-4844 type-3 marker (blob validity rules)     */
  blob_hashes : list(word)    /* EIP-4844 versioned hashes (BLOBHASH operands)    */
}

/* A LOG record (Yellow Paper section 4.4.1, the log series): emitting address,
 * 0..4 topics (LOG0..LOG4), and the data bytes. */
struct LogEntry     = { address : address, topics : list(word), data : list(byte) }

/* success: the top-level frame returned/stopped without reverting or
 * exceptionally halting (used for create-tx code deposit, log inclusion).
 * valid: the tx passed transaction VALIDITY (nonce/balance/intrinsic/floor/
 * fee/blob/7702 rules) and was applied to the block. A valid tx that REVERTS
 * or runs out of gas during execution still has valid = true -- it is a
 * legitimate block member (gas charged, nonce bumped). Block validity (the
 * stateless guest's successful_validation) folds valid, NOT success:
 * only an INVALID/inapplicable tx makes the containing block invalid. */
/* block_gas: EIP-7778 block-gas accounting WITHOUT the EIP-3529 refund
 * (= max(gas_used_before_refund, calldata_floor)). The block driver sums this
 * for the block gas-availability check and the header gas_used; it differs from
 * gas_used (the receipt's refunded gas) precisely by the refund. */
struct Receipt = { tx_type : int, success : bool, valid : bool, gas_used : gas, block_gas : gas, logs : list(LogEntry) }
