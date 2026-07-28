/* Production C representation constraints and whole-operation lowerings.
 *
 * Most of this splice refines semantic types so the C backend can select
 * native representations. The HTR accelerator replaces the complete
 * new-payload commitment operation, so its C implementation can consume the
 * already validated input references without materializing intermediate Sail
 * values. The base model and proof extractions retain the explicit HTR
 * equations; this external contract exists only in the optimized C splice. */

type word = range(0, 2 ^ 256 - 1)

type address = vector(20, inc, byte)

type b256 = vector(32, inc, byte)

/*
 * Optimized refinements of the base prelude's direct endian equations.
 * Canonical fixed bytes and native least-significant-first word limbs cross
 * this boundary directly; the standard model and proof extractions retain the
 * readable vector conversions.
 */
val hash_to_word = pure { c: "evmsail_hash_to_word" } : hash -> word

val word_to_hash = pure { c: "evmsail_word_to_hash" } : word -> hash

val hash_lt = pure { c: "evmsail_hash_lt" } : (hash, hash) -> bool

val word_to_address = pure { c: "evmsail_word_to_address" } : word -> address

val address_to_word = pure { c: "evmsail_address_to_word" } : address -> word

/* Valid execution gas is bounded by the greatest value reachable in the live
 * frame counter. Costs outside the same domain are rejected as unaffordable
 * by the optimized cost planners before they are materialized. */
type gas = range(0, 2 ^ 64 - 1)
type transaction_gas = range(0, 2 ^ 64 - 1)
type gas_cost = range(0, 2 ^ 64 - 1)
type modexp_factor = range(0, 3 * (2 ^ 64 - 1) + 257)
type modexp_product = range(
    0,
    (3 * (2 ^ 64 - 1) + 257) * (3 * (2 ^ 64 - 1) + 257),
)
type modexp_osaka_extra = range(0, div(2 ^ 64 - 1, 16))
type modexp_pre_osaka_extra = range(0, div(3 * (2 ^ 64 - 1) + 2, 8))
type bls_msm_coefficient = range(0, 45000 * (2 ^ 16 - 1))
type bls_msm_product = range(
    0,
    45000 * (2 ^ 16 - 1) * (2 ^ 64 - 1),
)
type bls_msm_tail_product = range(0, 45000 * (2 ^ 16 - 1) * 999)
type linear_gas_variable_product = range(0, 45000 * (2 ^ 64 - 1))
type linear_gas_product = range(0, 45000 * 2 ^ 64)
type live_gas_valid('value : Int) -> Bool =
    0 <= 'value & 'value <= 2 ^ 64 - 1

/* Production live gas already lies below the word modulus, so the canonical
 * total-function reduction is the identity at this boundary. */
function push_gas(value : gas) -> unit = push_word(U256(value))

/* Exact signed state-gas bounds follow the value's lifecycle.  A single
 * frame subtracts two u64 counters and adds at most the EIP-7825 regular-pool
 * spill.  Amsterdam combines at most the authorization and execution frames,
 * then settlement adds one u64 intrinsic charge.  These are representation
 * invariants derived from the source domains, not protocol gas caps. */
type frame_state_gas_delta = range(
    -(2 ^ 64 - 1),
    (2 ^ 64 - 1) + transaction_execution_gas_limit_value,
)
type state_gas_delta = range(
    -(2 * (2 ^ 64 - 1)),
    2 * ((2 ^ 64 - 1) + transaction_execution_gas_limit_value),
)
type transaction_state_gas_delta = range(
    -(2 * (2 ^ 64 - 1)),
    3 * (2 ^ 64 - 1) + 2 * transaction_execution_gas_limit_value,
)

/* The stateless-input ABI carries header quantities as SSZ uint64 values.
 * Collection counts and indices cannot exceed the enclosing 64-bit-addressed
 * byte source. */
type block_number = range(0, 2 ^ 64 - 1)
type block_timestamp = range(0, 2 ^ 64 - 1)
type block_gas = range(0, 2 ^ 64 - 1)

/* These values enter the optimized model through SSZ uint64 fields.  Keep
 * that proven lifecycle representation until an opcode explicitly
 * materializes an EVM word with `U256`. */
val word_of_block_number : block_number -> block_number
function word_of_block_number(value : block_number) -> block_number = value
val word_of_block_timestamp : block_timestamp -> block_timestamp
function word_of_block_timestamp(value : block_timestamp) -> block_timestamp = value

/* The production host exposes byte arenas through 64-bit positions and
 * lengths.  These aliases remain semantically distinct at function
 * boundaries while sharing the same finite optimized representation. */
type host_valid_range('off : Int, 'len : Int) -> Bool =
    0 <= 'off & 0 <= 'len & 'off + 'len <= 2 ^ 64 - 1
type source_valid_range('off : Int, 'len : Int) -> Bool =
    0 <= 'off & 0 <= 'len & 'off + 'len <= 2 ^ 64 - 1
type host_valid_access('value : Int) -> Bool =
    0 <= 'value & 'value <= 2 ^ 64 - 1
type source_valid_length('value : Int) -> Bool =
    0 <= 'value & 'value <= 2 ^ 64 - 1
type host_access = range(0, 2 ^ 64 - 1)
type source_pointer = range(0, 2 ^ 64 - 1)
type source_length = range(0, 2 ^ 64 - 1)
type journal_checkpoint = range(0, 2 ^ 64 - 1)

function scratch_length_add(left, right) =
    if right <= 2 ^ 64 - 1 - left then {
        left + right
    } else {
        assert(false, "scratch length overflow");
        0
    }

/* Successful memory plans are bounded by their exact expansion cost and the
 * live gas value. Host-sized offsets are introduced only after that semantic
 * check; there is no precomputed memory ceiling in the production model. */
type memory_valid_range('off : Int, 'len : Int) -> Bool =
    0 <= 'off & 0 <= 'len & 'off + 'len + 32 <= 2 ^ 64 - 1
type memory_pointer = range(0, 2 ^ 64 - 1)
type memory_length = range(0, 2 ^ 64 - 1)
type code_pointer = range(0, 2 ^ 64 - 1)
type code_length = range(0, 2 ^ 64 - 1)
type code_valid_length('len : Int) -> Bool =
    0 <= 'len & 'len + 32 <= 2 ^ 64 - 1
type code_chunk_index = range(0, 2 ^ 64 - 1)
type item_count = range(0, 2 ^ 64 - 1)
type item_index = range(0, 2 ^ 64 - 1)

/*
 * Region-backed slice offsets are absolute validated pointers in optimized C.
 * The standard model retains offsets. Sub-slicing remains ordinary addition,
 * while the host resolver checks that the resulting span belongs to its
 * nominal backing region.
 */
val optimized_output_buffer_slice = impure { c: "evmsail_output_buffer_slice" } : forall 'len,
                                                                                    source_valid_length('len). int('len) -> OutputSlice

function output_buffer_slice(len) =
    if len == 0 then {
        EMPTY_OUTPUT_SLICE
    } else {
        optimized_output_buffer_slice(len)
    }

/* RLP emitted into scratch shares the host's 64-bit byte-address domain.
 * Recursive list sizing is exact, then narrowed once at this materialization
 * boundary. */
type rlp_scratch_length = range(0, 2 ^ 64 - 1)

type rlp_natural_valid('value : Int) -> Bool =
    0 <= 'value & 'value <= 2 ^ 64 - 1

type rlp_natural_increment_valid('value : Int) -> Bool =
    0 <= 'value & 'value < 2 ^ 64 - 1

type rlp_natural_size = range(1, 33)

function rlp_uint_nat_size(n) =
    rlp_uint_word_size(U256(n))

function rlp_write_uint_nat(n) =
    rlp_write_uint_word(U256(n))

function optimized_rlp_scratch_slice_size(length : rlp_scratch_length, first : byte) -> rlp_scratch_length = {
    if (length == 1) & (first[7] == bitzero) then {
        1
    } else {
        rlp_scratch_length_add(length, rlp_scratch_small_length(rlp_length_prefix_len(length)))
    }
}

function rlp_scratch_region_size(data : ScratchSlice) -> rlp_scratch_length =
    optimized_rlp_scratch_slice_size(
        data.len,
        if data.len == 0 then {
            0x00
        } else {
            slice_byte(data, 0)
        },
    )

function rlp_log_data_size(data : LogDataSlice) -> rlp_scratch_length =
    optimized_rlp_scratch_slice_size(
        data.len,
        if data.len == 0 then {
            0x00
        } else {
            slice_byte(data, 0)
        },
    )

function rlp_input_scratch_slice_size(data : StatelessInputSlice) -> rlp_scratch_length =
    optimized_rlp_scratch_slice_size(
        data.len,
        if data.len == 0 then {
            0x00
        } else {
            slice_byte(data, 0)
        },
    )

function rlp_scratch_scratch_slice_size(data : ScratchSlice) -> rlp_scratch_length =
    optimized_rlp_scratch_slice_size(
        data.len,
        if data.len == 0 then {
            0x00
        } else {
            slice_byte(data, 0)
        },
    )

function rlp_log_scratch_slice_size(data : LogDataSlice) -> rlp_scratch_length =
    optimized_rlp_scratch_slice_size(
        data.len,
        if data.len == 0 then {
            0x00
        } else {
            slice_byte(data, 0)
        },
    )

function rlp_scratch_length_add(left, right) =
    if right <= 2 ^ 64 - 1 - left then {
        left + right
    } else {
        throw InvalidBlock(RlpDecode)
    }

/*
 * Complete preimage refinements. The standard model retains the explicit
 * scratch/RLP equations; optimized C computes the same exact widths and
 * writes each complete preimage once into borrowed shared scratch.
 */
val optimized_create2_address = impure { c: "evmsail_optimized_create2_address" } : (address, word, hash) -> address

function create2_address(sender : address, salt : word, init_hash : hash) -> address =
    optimized_create2_address(sender, salt, init_hash)

val optimized_tx_signing_hash = impure { c: "evmsail_optimized_tx_signing_hash" } : (TxType, StatelessInputSlice, word) -> hash

function tx_signing_hash(t : TxType, content_src : StatelessInputSlice, v : word) -> hash =
    optimized_tx_signing_hash(t, content_src, v)

val optimized_sha256_request_digest = impure { c: "evmsail_optimized_sha256_request_digest" } : (
        byte,
        StatelessInputSlice,
    ) -> hash

function sha256_request_digest(request_type : byte, request : StatelessInputSlice) -> hash =
    optimized_sha256_request_digest(request_type, request)

val optimized_execution_requests_hash = impure { c: "evmsail_optimized_execution_requests_hash" } : StatelessInputRef -> hash

function execution_requests_hash(input_ref : StatelessInputRef) -> hash =
    optimized_execution_requests_hash(input_ref)

val optimized_block_header_hash = impure { c: "evmsail_optimized_block_header_hash" } : (
        BlockHeader,
        hash,
        hash,
        hash,
        hash,
    ) -> hash

function block_header_hash(
    header : BlockHeader,
    transactions_root : hash,
    withdrawals_root : hash,
    requests_hash : hash,
    block_access_list_hash : hash,
) -> (
    hash
) =
    optimized_block_header_hash(header, transactions_root, withdrawals_root, requests_hash, block_access_list_hash)

val optimized_htr_new_payload_request = pure { c: "evmsail_htr_new_payload_request" } : StatelessInputRef -> hash

function htr_new_payload_request(input_ref : StatelessInputRef) -> hash =
    optimized_htr_new_payload_request(input_ref)

/*
 * Optimized interpreter refinement. The base model retains the explicit
 * fetch/decode/run loop; optimized C combines those three dispatch layers
 * while continuing to call the generated Sail per-opcode semantic functions.
 * This definition replaces `interpret` from `evm/interpreter.sail` when the
 * optimized C splice is selected.
 */
val optimized_interpret = impure { c: "evmsail_interpret" } : unit -> OutputSlice

function interpret() -> OutputSlice =
    optimized_interpret()

/*
 * The optimized receipt encoder hashes its retained C log range directly into
 * the final reserved RLP span and updates the block's 32-word accumulator in
 * the same pass. There is no per-receipt LogsBloom aggregate or 256-byte copy.
 */
val optimized_scratch_store_receipt_logs_bloom = impure { c: "scratch_store_receipt_logs_bloom" } : forall 'off,
                                                                                                      host_valid_access(
                                                                                                          'off,
                                                                                                      ). (
        int('off),
        log_store_index,
        log_store_index,
    ) -> ScratchRegionResult('off + 256)

function receipt_write_logs_bloom(receipt : Receipt, _bloom : LogsBloom) -> unit = {
    rlp_write_string_prefix(LOGS_BLOOM_BYTE_LENGTH, 0x00);
    let arena = scratch_arena;
    match optimized_scratch_store_receipt_logs_bloom(arena.len, receipt.logs.start, receipt.logs.count) {
        ScratchRegionReady(expanded) => scratch_arena = expanded,
        ScratchRegionFailed() => assert(false, "scratch receipt bloom append"),
    }
}

/*
 * The computed bloom uses the fixed decreasing-index representation while
 * the header retains canonical SSZ wire order. Compare those representations
 * directly at the source boundary without materializing the header bloom.
 */
val optimized_logs_bloom_matches_ref = pure { c: "evmsail_logs_bloom_matches_ref" } : (LogsBloom, LogsBloomRef) -> bool

function logs_bloom_matches_ref(computed : LogsBloom, reference : LogsBloomRef) -> bool =
    optimized_logs_bloom_matches_ref(computed, reference)

/*
 * Optimized state-root refinement. The standard model keeps the explicit
 * witness walker and canonical MPT builder; optimized C links an equivalent
 * fixed-stack implementation over the same host iterators and node database.
 * The optimized ABI raises the same Sail InvalidBlock exception on witness or
 * RLP failure as the standard implementation.
 */
$[c_throws]
val optimized_compute_state_root = impure { c: "evmsail_compute_state_root" } : hash -> hash

function compute_state_root() -> hash =
    optimized_compute_state_root(k_parent_state_root)

/*
 * All ordered protocol tries use the same optimized fixed-stack builder as
 * state/storage roots. Transaction and withdrawal roots cross as complete
 * operations so optimized C can consume their already validated SSZ list
 * references directly. The standard model retains the explicit RLP(index)
 * cursor, withdrawal encoding, and recursive subtree equations.
 */
$[c_throws]
val optimized_transaction_trie_root = impure { c: "evmsail_transaction_trie_root" } : TransactionListRef -> hash
$[c_throws]
val optimized_withdrawals_trie_root = impure { c: "evmsail_withdrawals_trie_root" } : WithdrawalListRef -> hash

function transaction_trie_root(txs : TransactionListRef) -> hash =
    optimized_transaction_trie_root(txs)

function withdrawals_trie_root(wds : WithdrawalListRef) -> hash =
    optimized_withdrawals_trie_root(wds)

/*
 * Complete optimized state accesses. The standard account/storage modules
 * remain the readable specification of overlay ordering, BAL effects,
 * authenticated witness fallback, caching, and transaction-original values.
 * Optimized C performs each complete operation against the same host-owned
 * tables in one crossing.
 */
$[c_throws]
val optimized_k_aload = impure { c: "evmsail_k_aload" } : (hash, address) -> Account
$[c_throws]
val optimized_k_sload = impure { c: "evmsail_k_sload" } : (hash, address, word) -> StorageValue
val optimized_k_sstore = impure { c: "evmsail_k_sstore" } : (address, word, word, word) -> unit
val optimized_store_account = impure { c: "evmsail_store_account" } : (address, Account) -> unit
val optimized_store_account_info = impure { c: "evmsail_store_account_info" } : (address, Account, AccountInfo) -> unit

function k_aload(a : address) -> Account =
    optimized_k_aload(k_parent_state_root, a)

function k_sload(a : address, s : word) -> StorageValue =
    optimized_k_sload(k_parent_state_root, a, s)

function k_sstore(a : address, s : word, value : StorageValue) -> unit =
    optimized_k_sstore(a, s, value.curr, value.orig)

function store_account(a : address, account : Account) -> unit =
    optimized_store_account(a, account)

function store_account_info(a : address, account : Account, info : AccountInfo) -> unit =
    optimized_store_account_info(a, account, info)

/*
 * The standard lifecycle module keeps the explicit account-then-storage
 * transaction merge. Optimized C consumes the host-owned rows directly in
 * insertion order, avoiding generated aggregate construction and sorting.
 */
val optimized_tx_merge = impure { c: "evmsail_tx_merge" } : (bool, bool, block_access_index) -> unit

function k_tx_merge() -> unit =
    optimized_tx_merge(protocol_profile_fork(k_protocol_profile) >= Cancun, protocol_profile_fork(k_protocol_profile) >= Amsterdam, k_block_access_index)

/*
 * Index source-backed witness nodes in one operation. The standard model
 * retains the explicit SSZ cursor and per-node hash equation.
 */
$[c_throws]
val optimized_index_witness_nodes = impure { c: "evmsail_index_witness_nodes" } : WitnessNodeListRef -> unit

function index_witness_nodes(nodes : WitnessNodeListRef) -> unit =
    optimized_index_witness_nodes(nodes)

/*
 * Likewise, keep the standard PUSH-aware opcode scan in Sail while allowing
 * the optimized build to consume and index the complete source-backed code
 * list in one call.
 */
$[c_throws]
val optimized_index_witness_codes = impure { c: "evmsail_index_witness_codes" } : (WitnessCodeListRef, bool) -> unit

function index_witness_codes(codes : WitnessCodeListRef) -> unit =
    optimized_index_witness_codes(codes, protocol_profile_fork(k_protocol_profile) >= Amsterdam)

/*
 * The standard model retains execution-ordered, length-prefixed receipt bytes
 * in scratch and feeds them to the shared recursive indexed-trie equations.
 * The optimized build instead copies each temporary encoding into one
 * reusable C arena and stores its offset/length in execution order. Its
 * high-level root operation owns RLP(index) key generation, canonical trie
 * ordering, and MPT construction entirely in C.
 */
val optimized_receipt_table_reset = impure { c: "evmsail_receipt_table_reset" } : unit -> unit
$[c_throws]
val optimized_receipt_table_push = impure { c: "evmsail_receipt_table_push" } : (transaction_count, ScratchSlice) -> unit
$[c_throws]
val optimized_receipt_table_root = impure { c: "evmsail_receipt_table_root" } : transaction_count -> hash
val optimized_block_logs_bloom_reset = impure { c: "evmsail_block_logs_bloom_reset" } : unit -> unit
val optimized_block_logs_bloom = impure { c: "evmsail_block_logs_bloom" } : unit -> LogsBloom

function receipt_accumulator_empty() -> ReceiptAccumulator = {
    optimized_receipt_table_reset();
    optimized_block_logs_bloom_reset();
    struct { records_start = 0, count = 0, cumulative_gas_used = 0, bloom = EMPTY_LOGS_BLOOM }
}

function receipt_accumulator_push(
    acc : ReceiptAccumulator,
    receipt : Receipt,
    next_count : transaction_count,
) -> (
    ReceiptAccumulator
) = {
    let cumulative : block_gas = conserved_gas_add(acc.cumulative_gas_used, receipt.gas_used);
    let mark = scratch_begin();
    let value = receipt_encoded(receipt, cumulative, EMPTY_LOGS_BLOOM);
    optimized_receipt_table_push(acc.count, value);
    scratch_rewind(mark);
    struct {
        records_start = acc.records_start,
        count = next_count,
        cumulative_gas_used = cumulative,
        bloom = acc.bloom,
    }
}

function receipt_accumulator_root(acc : ReceiptAccumulator) -> hash =
    optimized_receipt_table_root(acc.count)

function receipt_accumulator_bloom(_acc : ReceiptAccumulator) -> LogsBloom =
    optimized_block_logs_bloom()

/* Re-establish executable cursor headroom after a CodeSlice has crossed a
 * non-dependent aggregate such as transaction or frame output. All callers
 * derive this condition from their source bound; failure is an internal
 * representation violation rather than a protocol code-size decision. */
function validated_code_slice(bytes : CodeRegionSlice) -> CodeSlice = {
    let length = bytes.len;
    if length + 32 <= 2 ^ 64 - 1 then {
        code_slice(bytes)
    } else {
        assert(false, "executable code cursor headroom");
        EMPTY_CODE_SLICE
    }
}

/* Canonical callers establish a shared admitted-gas budget. Reify that proof
 * obligation at the few credit boundaries so production uint64 addition can
 * never wrap if an internal invariant is violated. */
function conserved_gas_add(left, right) =
    if right <= sizeof(block_gas_limit_bound) - left then {
        left + right
    } else {
        throw InvalidBlock(ExecutionInvalid)
    }

function rlp_decode_gas(f, limit) = {
    let value = rlp_decode_uint64(f);
    if (limit != 0) & (limit < value) then {
        throw InvalidBlock(GasUsedExceedsLimit)
    } else {
        value
    }
}

/* Narrow a transaction-controlled word range into the host domain through
 * the Yellow Paper memory-cost equation. If cumulative C_mem exceeds every
 * representable live-gas value, no production frame can afford the range.
 * The dependent mem_cost result lets the solver derive the host-sized byte
 * range without a separately maintained memory ceiling. */
val memory_expansion_cost : forall 'required 'available, source_valid_length('required) & live_gas_valid('available). (
        int('required),
        int('available),
    ) -> range(0, 'available)

function memory_expansion_cost(required_size, available) = {
    let new_words = memory_word_count(required_size);
    let old_words = memory_word_count(evm_memory_high_water());
    if new_words <= old_words then {
        0
    } else {
        let old_cost = bounded_mem_cost(old_words);
        let new_cost = bounded_mem_cost(new_words);
        if old_cost <= new_cost & new_cost <= old_cost + available then {
            let cost : range(0, 'available) = new_cost - old_cost;
            cost
        } else {
            exc_halt(OutOfGas);
            0
        }
    }
}

val memory_expansion : forall 'size 'available, 0 <= 'size & 'size < 2 ^ 256 & live_gas_valid('available). (
        word,
        int('size),
        int('available),
    ) -> MemoryExpansion('available)

function memory_expansion(start, size, available) = {
    if size == 0 then {
        struct { range = EMPTY_MEMORY_RANGE, required_size = 0, cost = 0 }
    } else {
        if start <= sizeof(2 ^ 64 - 1) & size <= sizeof(2 ^ 64 - 1) then {
            let (bounded_start as 'bounded_start) :
                    {'bounded_start,
                     0 <= 'bounded_start & 'bounded_start <= 2 ^ 64 - 1.
                     int('bounded_start)} = start;
            let (bounded_size as 'bounded_size) :
                    {'bounded_size,
                     'bounded_size == 'size & 0 <= 'bounded_size & 'bounded_size <= 2 ^ 64 - 1.
                     int('bounded_size)} = size;
            let required = bounded_start + bounded_size;
            let (words as 'words) = memory_word_count(required);
            let (bounded_words as 'bounded_words) :
                    {'bounded_words,
                     'bounded_words == 'words & 0 <= 'bounded_words & 'bounded_words <= block_gas_limit_bound.
                     int('bounded_words)} = words;
            let expansion_cost : int(3 * 'bounded_words + div('bounded_words * 'bounded_words, 512)) = bounded_mem_cost(
                bounded_words,
            );
            if expansion_cost <= sizeof(block_gas_limit_bound) then {
                let required_size : memory_length = required;
                struct {
                    range = memory_range(bounded_start, bounded_size),
                    required_size = required_size,
                    cost = memory_expansion_cost(required_size, available),
                }
            } else {
                exc_halt(OutOfGas);
                struct { range = EMPTY_MEMORY_RANGE, required_size = 0, cost = 0 }
            }
        } else {
            exc_halt(OutOfGas);
            struct { range = EMPTY_MEMORY_RANGE, required_size = 0, cost = 0 }
        }
    }
}

val memory_pair_expansion : forall 'left_start 'left_size 'right_start 'right_size 'available,
                                 0
                              <= 'left_start
                              &  'left_start
                              <  2 ^ 256
                              &  0
                              <= 'left_size
                              &  'left_size
                              <  2 ^ 256
                              &  0
                              <= 'right_start
                              &  'right_start
                              <  2 ^ 256
                              &  0
                              <= 'right_size
                              &  'right_size
                              <  2 ^ 256
                              &  live_gas_valid('available). (
        int('left_start),
        int('left_size),
        int('right_start),
        int('right_size),
        int('available),
    ) -> MemoryPairExpansion('available)

function memory_pair_expansion(left_start, left_size, right_start, right_size, available) = {
    let left_live = left_size != 0;
    let right_live = right_size != 0;
    let left_fits = not_bool(left_live) | (left_start <= sizeof(2 ^ 64 - 1) & left_size <= sizeof(2 ^ 64 - 1));
    let right_fits = not_bool(right_live) | (right_start <= sizeof(2 ^ 64 - 1) & right_size <= sizeof(2 ^ 64 - 1));
    if left_fits & right_fits then {
        /* Narrow only live operands. A zero-sized range ignores its offset, so
         * binding both components to zero also prevents an unused 256-bit
         * offset from entering native cursor arithmetic. */
        let bounded_left_start : host_access = if left_live then { left_start } else { 0 };
        let bounded_left_size : host_access = if left_live then { left_size } else { 0 };
        let bounded_right_start : host_access = if right_live then { right_start } else { 0 };
        let bounded_right_size : host_access = if right_live then { right_size } else { 0 };
        let left_end = bounded_left_start + bounded_left_size;
        let right_end = bounded_right_start + bounded_right_size;
        let required = if left_end < right_end then { right_end } else { left_end };
        let (words as 'words) = memory_word_count(required);
        let (bounded_words as 'bounded_words) :
                {'bounded_words,
                 'bounded_words == 'words & 0 <= 'bounded_words & 'bounded_words <= block_gas_limit_bound.
                 int('bounded_words)} = words;
        let expansion_cost : int(3 * 'bounded_words + div('bounded_words * 'bounded_words, 512)) = bounded_mem_cost(
            bounded_words,
        );
        if expansion_cost <= sizeof(block_gas_limit_bound) then {
            let required_size : memory_length = required;
            struct {
                left =
                    if left_live then {
                        memory_range(bounded_left_start, bounded_left_size)
                    } else {
                        EMPTY_MEMORY_RANGE
                    },
                right =
                    if right_live then {
                        memory_range(bounded_right_start, bounded_right_size)
                    } else {
                        EMPTY_MEMORY_RANGE
                    },
                required_size = required_size,
                cost = memory_expansion_cost(required_size, available),
            }
        } else {
            exc_halt(OutOfGas);
            struct { left = EMPTY_MEMORY_RANGE, right = EMPTY_MEMORY_RANGE, required_size = 0, cost = 0 }
        }
    } else {
        exc_halt(OutOfGas);
        struct { left = EMPTY_MEMORY_RANGE, right = EMPTY_MEMORY_RANGE, required_size = 0, cost = 0 }
    }
}
