/* ======================================================================== */
/* PRIMITIVES: STATELESS SSZ INPUT LAYOUT                                   */
/*                                                                          */
/* The wire layout of the canonical SszStatelessInput -- every field offset */
/* and element stride as a named constant. Mirrors execution-specs          */
/* (projects/zkevm) forks/amsterdam stateless_ssz.py. The getters that      */
/* decode against this layout live in host/stateless_input.sail. Pure       */
/* constants -- no registers, no externs.                                   */
/* ======================================================================== */

/* The input starts with a 2-byte big-endian schema id; the SSZ body follows. */
let SSZ_BODY : int = 2

/* Width of one entry in an SSZ variable-field offset table (uint32-LE). */
let SSZ_OFF_BYTES : int = 4

/* SszStatelessInput: 4 variable fields, offset table at body+{0,4,8,12} */
let IN_NPR_OFF : int = 0 /* new_payload_request                   */
let IN_WITNESS_OFF : int = 4 /* execution_witness                     */
let IN_CHAIN_CONFIG_OFF : int = 8 /* chain_config                          */
let IN_PUBLIC_KEYS_OFF : int = 12 /* public_keys (= chain_config end)      */

/* SszNewPayloadRequest: execution_payload offset @0, versioned_hashes offset
 * @4, parent_beacon_block_root @8 (inline Bytes32), execution_requests offset
 * @40 (after the 32-byte root). */
let NPR_PAYLOAD_OFF : int = 0
let NPR_VHASHES_OFF : int = 4
let NPR_BEACON_ROOT : int = 8
let NPR_REQUESTS_OFF : int = 40

/* SszExecutionRequests: three variable lists, offset table at req+{0,4,8} */
let REQ_DEPOSITS_OFF : int = 0
let REQ_WITHDRAWALS_OFF : int = 4
let REQ_CONSOLIDATIONS_OFF : int = 8

/* SszExecutionPayload fixed-region field offsets (bytes from payload start) */
let PL_FEE_RECIPIENT : int = 32
let PL_STATE_ROOT : int = 52
let PL_RECEIPTS_ROOT : int = 84
let PL_LOGS_BLOOM : int = 116
let PL_PREV_RANDAO : int = 372
let PL_BLOCK_NUMBER : int = 404
let PL_GAS_LIMIT : int = 412
let PL_GAS_USED : int = 420
let PL_TIMESTAMP : int = 428
let PL_EXTRA_OFF : int = 436
let PL_BASE_FEE : int = 440
let PL_BLOCK_HASH : int = 472
let PL_TXS_OFF : int = 504
let PL_WDS_OFF : int = 508
let PL_BLOB_GAS_USED : int = 512
let PL_EXCESS_BLOB_GAS : int = 520
let PL_BAL_OFF : int = 528
let PL_SLOT_NUMBER : int = 532 /* EIP-7843 */

/* SszWithdrawal: FIXED 44-byte elements, field offsets within one element */
let WD_SIZE : int = 44
let WD_INDEX : int = 0
let WD_VALIDATOR_INDEX : int = 8
let WD_ADDRESS : int = 16
let WD_AMOUNT : int = 36

/* block_access_list (simplified): u32-LE count, then count fixed entries of
 * addr[20 bytes] ++ slot[32 bytes BE]. */
let BAL_COUNT_BYTES : int = 4
let BAL_ENTRY_SIZE : int = 52
let BAL_SLOT_OFF : int = 20

/* SszChainConfig: chain_id u64 @0, active_fork (SszForkConfig) offset @8 */
let CC_CHAIN_ID : int = 0
let CC_ACTIVE_FORK_OFF : int = 8

/* SszForkConfig: fork u64 @0 (ProtocolFork index; Amsterdam = 20 in current
 * projects/zkevm fixtures), activation (SszForkActivation) offset @8,
 * blob_schedule offset @12 */
let FC_FORK : int = 0
let FC_ACTIVATION_OFF : int = 8
let FC_BLOBSCHED_OFF : int = 12

/* SszForkActivation: block_number / timestamp, each an
 * SszOptionalForkActivationValue (List[u64,1]), offset table at act+{0,4} */
let FA_BLOCK_NUMBER_OFF : int = 0
let FA_TIMESTAMP_OFF : int = 4

/* SszExecutionWitness: 3 variable List fields, offset table at witness+{0,4,8} */
let WIT_STATE_OFF : int = 0
let WIT_CODES_OFF : int = 4
let WIT_HEADERS_OFF : int = 8
