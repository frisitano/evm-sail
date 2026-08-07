/*md
# Protocol quantities

Semantic numeric domains used outside the EVM's 256-bit word algebra.
Protocol fields retain their wire bounds, structural counters carry the
limits imposed by their data structures, and byte positions remain exact
non-negative quantities after operand validation.

## Types

The aliases below name the semantic role of protocol and structural
quantities while preserving their mathematical values. Where the protocol or
data structure supplies a bound, the alias records it explicitly.

*/

/* Protocol quantities are mathematical naturals with semantic bounds. The
 * canonical model never uses a fixed-width bitvector merely because a wire
 * encoding or host implementation happens to use 64 bits. */

/* Type-level bounds have kind Int because Sail has one kind for numeric type
 * expressions. Runtime values below remain non-negative ranges or naturals.
 * `sizeof(bound_name)` is used only when code needs the bound as a value. */

/*! The largest account nonce admitted by EIP-2681. */
type account_nonce_bound : Int = 2 ^ 64 - 1
/*! An account transaction-count nonce (EIP-2681). */
type account_nonce = range(0, account_nonce_bound)

/*! Largest value represented by an SSZ `uint64`. This is a wire-schema
    constraint, sourced from the consensus `ExecutionPayload` container and
    the Amsterdam `SszExecutionPayload`, rather than an implementation word
    size. */
type ssz_uint_bound : Int = 2 ^ 64 - 1

/*! An execution block number. Provenance: the execution-payload SSZ schema
    declares `block_number: uint64`. The execution rules do not impose a
    tighter supported-fork bound. */
type block_number = range(0, ssz_uint_bound)
/*! An execution block timestamp in seconds. Provenance: the
    execution-payload SSZ schema declares `timestamp: uint64`; no tighter
    supported-fork protocol bound is applied. */
type block_timestamp = range(0, ssz_uint_bound)

/*! Blob gas charged per blob, `2^17` (EIP-4844). */
type gas_per_blob_value : Int = 2 ^ 17
/*! Named blob-schedule constants retained by the profile domains below. */
type blob_schedule_inactive_count : Int = 0
type cancun_blob_target_count : Int = 3
type prague_blob_target_count : Int = 6
type bpo1_blob_target_count : Int = 10
type bpo2_blob_target_count : Int = 14
type cancun_blob_max_count : Int = 6
type prague_blob_max_count : Int = 9
type bpo1_blob_max_count : Int = 15
type bpo2_blob_max_count : Int = 21

/*! Largest fee-update fraction in the supported blob schedules: BPO2's
    `BLOB_BASE_FEE_UPDATE_FRACTION = 11684671` (EIP-8135). */
type blob_fee_update_fraction_bound : Int = 11684671
/*! A conservative exponent at which an EIP-4844 blob base fee can no longer
    fit in the EVM's 256-bit word domain: `e^256 > 2^256`. */
type blob_fee_word_exponent_limit : Int = 256

type blob_schedule_target_value('value : Int) -> Bool =
       'value == blob_schedule_inactive_count
    |  'value == cancun_blob_target_count
    |  'value == prague_blob_target_count
    |  'value == bpo1_blob_target_count
    |  'value == bpo2_blob_target_count

type blob_schedule_max_value('value : Int) -> Bool =
       'value == blob_schedule_inactive_count
    |  'value == cancun_blob_max_count
    |  'value == prague_blob_max_count
    |  'value == bpo1_blob_max_count
    |  'value == bpo2_blob_max_count

/*! A target selected by one of the blob schedules supported by the schema.
    This is distinct from an observed blob count, which may be any value in
    its contiguous range. */
type blob_schedule_target_count = {
    'value,
    blob_schedule_target_value('value).
    int('value)
}
/*! A maximum selected by one of the blob schedules supported by the schema. */
type blob_schedule_max_count = {
    'value,
    blob_schedule_max_value('value).
    int('value)
}
/*! The fork-selected maximum number of blobs carried by one transaction.
    Zero denotes a profile before blob transactions activate. */
type transaction_blob_limit_value('value : Int) -> Bool =
       'value == blob_schedule_inactive_count
    |  'value == cancun_blob_max_count
    |  'value == prague_blob_max_count
type transaction_blob_limit = {
    'value,
    transaction_blob_limit_value('value).
    int('value)
}

/*! A transaction blob count under one fork-selected transaction limit.
    The limit is a finite-set profile parameter; the observed count occupies
    the complete contiguous range beneath that selected limit. */
type transaction_blob_count('limit : Int) = range(0, 'limit)

/*! Blob gas used by one supported block. The existential count retains that
    every value is exactly a multiple of `GAS_PER_BLOB`; profile-indexed
    decoding applies the selected schedule's tighter range before values enter
    this heterogeneous header domain. */
type blob_gas_used = {
    'count,
       0 <= 'count
    &  'count <= bpo2_blob_max_count.
    int(gas_per_blob_value * 'count)
}

/*! Blob gas contributed by one transaction. Profile-indexed blob-hash
    decoding establishes the selected limit before this derived quantity is
    widened to the common transaction-cost domain. */
type transaction_blob_gas = {
    'count,
       0 <= 'count
    &  'count <= prague_blob_max_count.
    int(gas_per_blob_value * 'count)
}
/*! Wire ceiling for the EIP-4844 `excess_blob_gas` header field.
    Provenance: both the Deneb consensus `ExecutionPayload` and Amsterdam's
    stateless SSZ payload encode the field as `uint64`. */
type excess_blob_gas_wire_bound : Int = ssz_uint_bound

/* Union ceiling of the fork-indexed inductive equations retained by
 * `ProtocolProfile`. The expression deliberately uses the largest admitted
 * denominator and block growth rather than embedding a precomputed result. */
type excess_blob_gas_reachable_bound : Int =
    blob_fee_word_exponent_limit * blob_fee_update_fraction_bound
    + bpo2_blob_max_count * gas_per_blob_value
/*! The supported-fork reachable accumulated excess-blob-gas value. */
type excess_blob_gas_bound : Int = excess_blob_gas_reachable_bound
type excess_blob_gas = range(0, excess_blob_gas_bound)

/*! The positive denominator governing excess-blob-gas fee adjustment. */
type blob_fee_update_fraction = range(1, blob_fee_update_fraction_bound)

/*! Named EIP-4844/BPO fee-update fractions. */
type inactive_blob_fee_update_fraction : Int = 1
type cancun_blob_fee_update_fraction : Int = 3338477
type prague_blob_fee_update_fraction : Int = 5007716
type bpo1_blob_fee_update_fraction : Int = 8346193
type bpo2_blob_fee_update_fraction : Int = 11684671

type blob_schedule_fee_update_fraction_value('value : Int) -> Bool =
       'value == inactive_blob_fee_update_fraction
    |  'value == cancun_blob_fee_update_fraction
    |  'value == prague_blob_fee_update_fraction
    |  'value == bpo1_blob_fee_update_fraction
    |  'value == bpo2_blob_fee_update_fraction

/*! A fee-update denominator selected by one of the supported blob schedules.
    Observed arithmetic still uses the broader positive denominator domain;
    configuration itself can only choose these protocol constants. */
type blob_schedule_fee_update_fraction = {
    'value,
    blob_schedule_fee_update_fraction_value('value).
    int('value)
}

/*! Fork-selected limits retained in the validated protocol profile. A fork
    without a stricter transaction-gas cap admits the complete SSZ `uint64`
    block-gas domain; the concrete header limit is applied separately. */
type pre_amsterdam_deployed_code_size_limit : Int = 24576
type amsterdam_deployed_code_size_limit : Int = 65536
type inactive_initcode_size_limit : Int = 0
type pre_amsterdam_initcode_size_limit : Int = 49152
type amsterdam_initcode_size_limit : Int = 131072
type eip7825_transaction_gas_limit : Int = 2 ^ 24

type protocol_deployed_code_size_limit_value('value : Int) -> Bool =
       'value == pre_amsterdam_deployed_code_size_limit
    |  'value == amsterdam_deployed_code_size_limit
type protocol_deployed_code_size_limit = {
    'value,
    protocol_deployed_code_size_limit_value('value).
    int('value)
}

type protocol_initcode_size_limit_value('value : Int) -> Bool =
       'value == inactive_initcode_size_limit
    |  'value == pre_amsterdam_initcode_size_limit
    |  'value == amsterdam_initcode_size_limit
type protocol_initcode_size_limit = {
    'value,
    protocol_initcode_size_limit_value('value).
    int('value)
}

type protocol_transaction_total_gas_limit_value('value : Int) -> Bool =
       'value == eip7825_transaction_gas_limit
    |  'value == ssz_uint_bound
type protocol_transaction_total_gas_limit = {
    'value,
    protocol_transaction_total_gas_limit_value('value).
    int('value)
}

type protocol_transaction_regular_gas_limit_value('value : Int) -> Bool =
       'value == eip7825_transaction_gas_limit
    |  'value == ssz_uint_bound
type protocol_transaction_regular_gas_limit = {
    'value,
    protocol_transaction_regular_gas_limit_value('value).
    int('value)
}

/*! Largest chain identifier admitted by the typed-transaction wire decoder. */
type chain_identifier_bound : Int = 2 ^ 64 - 1

/*! A chain identifier. Typed-transaction chain identifiers and the stateless
    chain configuration are decoded as unsigned 64-bit integers. */
type chain_identifier = range(0, chain_identifier_bound)

/*! A beacon-chain slot number. Provenance: EIP-7843 and Amsterdam's
    stateless SSZ payload declare this field as `uint64`. */
type slot_number = range(0, ssz_uint_bound)
/*! An EIP-4895 withdrawal index, encoded as SSZ `uint64`. */
type withdrawal_index = range(0, ssz_uint_bound)
/*! An EIP-4895 validator index, encoded as SSZ `uint64`. */
type validator_index = range(0, ssz_uint_bound)
/*! An EIP-4895 withdrawal amount in gwei, encoded as SSZ `uint64`. */
type withdrawal_amount = range(0, ssz_uint_bound)

/*! An eight-byte unsigned integer read from the stateless-input SSZ schema.
    This is a transport type; decoders widen or validate it into the semantic
    field type at the container boundary. */
type ssz_uint = range(0, 2 ^ 64 - 1)

/* SSZ offsets are relative positions encoded as little-endian uint32 values.
 * They are not general protocol quantities: keeping the wire bound in the
 * type lets optimized builds retain 32-bit arithmetic until an offset is
 * resolved against an absolute source position. */
/*! A container-relative offset carried by an SSZ `uint32`. */
type ssz_offset = range(0, 2 ^ 32 - 1)
/*! An index into a table of four-byte SSZ offsets. */
type ssz_offset_index = range(0, 2 ^ 30 - 1)

/* Small structural domains carry their protocol bounds directly. */
/*! The EVM call-frame depth ceiling (Yellow Paper `I_e`). */
type call_depth_limit : Int = 1024
/*! The nesting depth of an execution frame. */
type frame_depth = range(0, call_depth_limit)
/*! The immediate-byte width of a PUSH instruction. */
type push_width = range(0, 32)
/*! The number of words on an operand stack. */
type operand_stack_height = range(0, 1024)
/*! A zero-based index from the top of the operand stack. */
type stack_index = range(0, 1023)
/*! A nonzero operand-stack position used by DUP and SWAP. */
type stack_operation_index = range(1, 16)
/*! The one-based deep-stack index decoded by EIP-8024 DUPN and SWAPN. */
type deep_stack_index = range(17, 235)
/*! The number of indexed topics attached to one log. */
type log_topic_count = range(0, 4)
/*! The parity bit used by transaction signatures. */
type y_parity = range(0, 1)
/*! A depth in a 64-level binary Merkle tree. */
type merkle_depth = range(0, 64)
/*! An EVM instruction byte. */
type opcode = range(0, 255)
/*! The closed first-order selector for the precompile catalog. Availability,
    gas pricing, and execution are separate interpreters of this identifier so
    their protocol equations remain explicit without function-valued records. */
enum PrecompileId = {
    NotPrecompile,
    Ecrecover,
    Sha256,
    Ripemd160,
    Identity,
    Modexp,
    Bn254Add,
    Bn254Mul,
    Bn254Pairing,
    Blake2f,
    KzgPointEvaluation,
    BlsG1Add,
    BlsG1Msm,
    BlsG2Add,
    BlsG2Msm,
    BlsPairing,
    BlsMapFpToG1,
    BlsMapFp2ToG2,
    P256Verify,
}
type precompile_id = PrecompileId
/*! The round count supplied to the BLAKE2 compression precompile. */
type blake2_rounds = range(0, 2 ^ 32 - 1)
/*! A fixed-point discount factor used by BLS precompile pricing. */
type bls_discount = range(0, 2 ^ 16 - 1)
/*! A bit position in the 2048-bit log bloom. */
type bloom_bit_index = range(0, 2047)
/*! A 64-bit word position in the log bloom. */
type bloom_u64_index = range(0, 31)
/*! A bit position within a log-bloom u64. */
type bloom_u64_bit = range(0, 63)
/*! The number of authenticated ancestor hashes available to `BLOCKHASH`. */
type ancestor_hash_count = range(0, 256)
/*! An index into the 256 most recent ancestor block hashes. */
type ancestor_index = range(0, 255)

/*! Embeds an EIP-2681 account nonce in the EVM word domain. */
function word_of_account_nonce(value : account_nonce) -> account_nonce = value

/*! Embeds an EIP-4895 withdrawal amount in the EVM word domain. */
function word_of_withdrawal_amount(value : withdrawal_amount) -> withdrawal_amount = value

/*! Embeds an EIP-7843 slot number in the EVM word domain. */
function word_of_slot_number(value : slot_number) -> slot_number = value

/*! Embeds the SSZ-bounded block number in the EVM word domain. The uint64
    schema proof makes a runtime 256-bit range check unnecessary. */
function word_of_block_number(value : block_number) -> block_number = value

/*! Embeds the SSZ-bounded block timestamp in the EVM word domain. The uint64
    schema proof makes a runtime 256-bit range check unnecessary. */
function word_of_block_timestamp(value : block_timestamp) -> block_timestamp = value

/*! Converts a chain identifier to the value exposed by CHAINID. */
function word_of_chain_identifier(value : chain_identifier) -> chain_identifier = value

/* Each host byte arena has its own semantic address-space boundary. Keeping
 * the bounds nominally separate allows an executable backend to refine one
 * arena without changing the others. The initial executable profile admits
 * uint32-sized regions; tighter protocol bounds remain attached to the
 * structures which have them. Raw EVM operands remain words until the opcode
 * has established their specified overflow behaviour. */
/*! Default capacity of an implementation-owned byte arena. */
type default_host_region_bound : Int = 2 ^ 32 - 1
/*! Immutable stateless-input envelope capacity. */
type stateless_input_region_bound : Int = default_host_region_bound
/*! Executor scratch-arena capacity. */
type scratch_region_bound : Int = default_host_region_bound
/*! Shared per-frame EVM-memory arena capacity. */
type memory_region_bound : Int = default_host_region_bound
/*! Content-addressed executable-code arena capacity. */
type code_region_bound : Int = default_host_region_bound
/*! Retained transaction-log data arena capacity. */
type log_data_region_bound : Int = default_host_region_bound
/*! Guest output-buffer capacity. */
type output_region_bound : Int = default_host_region_bound

/*! Whether an offset/length pair is representable by a generic host
    interface. Region-bearing values use the more specific predicates below. */
type host_valid_range('off : Int, 'len : Int) -> Bool =
    0 <= 'off & 0 <= 'len & 'off + 'len <= default_host_region_bound
/*! Common bound for relative source coordinates used by generic cursor
    operations. Nominal slices retain their region-specific invariant. */
type source_valid_range('off : Int, 'len : Int) -> Bool =
    0 <= 'off & 0 <= 'len & 'off + 'len <= default_host_region_bound
type stateless_input_valid_range('off : Int, 'len : Int) -> Bool =
    0 <= 'off & 0 <= 'len & 'off + 'len <= stateless_input_region_bound
type scratch_valid_range('off : Int, 'len : Int) -> Bool =
    0 <= 'off & 0 <= 'len & 'off + 'len <= scratch_region_bound
type memory_region_valid_range('off : Int, 'len : Int) -> Bool =
    0 <= 'off & 0 <= 'len & 'off + 'len <= memory_region_bound
type code_region_valid_range('off : Int, 'len : Int) -> Bool =
    0 <= 'off & 0 <= 'len & 'off + 'len <= code_region_bound
type log_data_valid_range('off : Int, 'len : Int) -> Bool =
    0 <= 'off & 0 <= 'len & 'off + 'len <= log_data_region_bound
type output_region_valid_range('off : Int, 'len : Int) -> Bool =
    0 <= 'off & 0 <= 'len & 'off + 'len <= output_region_bound
/*! Whether one host byte quantity is representable. Unlike
    `host_valid_range`, this does not assert that two quantities can be added
    without overflow. */
type host_valid_access('value : Int) -> Bool =
    0 <= 'value & 'value <= default_host_region_bound
/*! Whether one relative source coordinate is representable. */
type source_valid_length('value : Int) -> Bool =
    0 <= 'value & 'value <= default_host_region_bound
/*! A position representable by a generic host byte-store interface. */
type host_access = range(0, default_host_region_bound)
/*! An absolute byte position in a named source region. */
type source_pointer = range(0, default_host_region_bound)
/*! A byte length or regular-layout count derived from a source region. */
type source_length = range(0, default_host_region_bound)
/*! A coordinate in the immutable stateless-input envelope. */
type stateless_input_pointer = range(0, stateless_input_region_bound)
/*! A length in the immutable stateless-input envelope. */
type stateless_input_length = range(0, stateless_input_region_bound)
/*! A coordinate in the executor scratch arena. */
type scratch_pointer = range(0, scratch_region_bound)
/*! A length in the executor scratch arena. */
type scratch_length = range(0, scratch_region_bound)
/*! A coordinate in retained log-data storage. */
type log_data_pointer = range(0, log_data_region_bound)
/*! A length in retained log-data storage. */
type log_data_length = range(0, log_data_region_bound)
/*! A coordinate in the guest output region. */
type output_pointer = range(0, output_region_bound)
/*! A length in the guest output region. */
type output_length = range(0, output_region_bound)
/*! A relative coordinate in calldata. Calldata may borrow either immutable
    stateless input or the active EVM-memory frame, so this is the semantic
    boundary for that explicit sum type rather than a generic host quantity. */
type calldata_pointer = range(0, default_host_region_bound)
/*! A byte length in calldata, independent of which calldata variant owns the
    bytes. */
type calldata_length = range(0, default_host_region_bound)
/*! The number of bytes loaded into one EVM word. */
type word_byte_count = range(0, 32)
/*! A cursor into a transaction-local journal-owned worklist. */
type journal_cursor = range(0, default_host_region_bound)
/*! The monotonically increasing storage incarnation owned by an account.
    Generation zero is reserved for the absence of a generation. */
type storage_generation = range(0, default_host_region_bound)

/*! An absolute byte position in the shared EVM-memory arena. */
type memory_pointer = range(0, memory_region_bound)
/*! A materialized length or allocation size in the EVM-memory arena. */
type memory_length = range(0, memory_region_bound)

/*! Largest possible initial/live gas value from the SSZ-backed block-gas
    domain. Fork and header limits may make this ceiling smaller; this union
    ceiling is sufficient to prove one representation valid for every
    supported profile. Runtime affordability continues to use remaining gas. */
type memory_expansion_proof_gas_ceiling : Int = ssz_uint_bound
/*! First memory word count whose quadratic Yellow Paper charge alone exceeds
    `memory_expansion_proof_gas_ceiling`:
    `(2^37)^2 / 512 = 2^65 > 2^64 - 1`. This is a derived proof threshold,
    not a runtime memory or host-allocation limit. */
type memory_unaffordable_word_threshold : Int = 2 ^ 37
/*! Largest whole-word endpoint which can precede that threshold. */
type memory_affordable_word_bound : Int =
    memory_unaffordable_word_threshold - 1
/*! Largest byte endpoint retained by optimized expansion planning after the
    maximum-gas proof. Each attempted expansion is still checked against its
    exact live `gas_remaining`. */
type memory_affordable_byte_bound : Int =
    32 * memory_affordable_word_bound
/*! Largest word count representable by the current EVM-memory region. */
type memory_region_word_bound : Int = 2 ^ 27
/*! Whether a word count may be presented to the cumulative memory-cost
    equation. This follows from the independently configured host-region
    boundary; the exact live-gas check remains the protocol affordability
    rule. */
type memory_cost_input('words : Int) -> Bool =
    0 <= 'words

/* A range describes the mathematical EVM address selected by an opcode.  Its
 * materializability is established separately by the memory-expansion cost:
 * an unaffordable range halts before it reaches the host byte store. */
/*! The invariant carried by every canonical EVM memory range. */
type memory_valid_range('off : Int, 'len : Int) -> Bool =
    0 <= 'off & 0 <= 'len & 'off + 'len <= memory_region_bound

/*! A memory endpoint which may be presented to the expansion-cost equation.
    The canonical specification admits every mathematical byte position;
    executable splices may strengthen this representation boundary after
    proving that every excluded endpoint is unaffordable. */
type memory_expansion_endpoint('required : Int) -> Bool =
    0 <= 'required

/*! Relates a canonical memory operand to its exclusive endpoint. A zero-size
    operand accesses no memory, so both its retained range and endpoint are
    canonicalized to zero. */
type memory_access_relation(
    'off : Int,
    'len : Int,
    'required : Int,
) -> Bool =
    memory_valid_range('off, 'len)
    & memory_expansion_endpoint('required)
    & (
          ('len == 0 & 'off == 0 & 'required == 0)
        | (0 < 'len & 'required == 'off + 'len)
    )

/*! The indexed fields of an EVM memory range. */
struct MemoryRangeFields('off : Int, 'len : Int), memory_valid_range('off, 'len) = {
    off : int('off),
    len : int('len),
}

/*! A memory range retaining its offset, length, and containment proof. */
type MemoryRange = {
    'off 'len,
    memory_valid_range('off, 'len). MemoryRangeFields('off, 'len)
}

/*! One logical EVM memory operand together with the exact endpoint which
    contributes to the shared expansion plan. */
struct MemoryAccessFields('off : Int, 'len : Int, 'required : Int),
    memory_access_relation('off, 'len, 'required) = {
    range : MemoryRangeFields('off, 'len),
    required_size : int('required),
}

/*! A memory operand retaining its range/endpoint relationship existentially. */
type MemoryAccess = {
    'off 'len 'required,
    memory_access_relation('off, 'len, 'required).
    MemoryAccessFields('off, 'len, 'required)
}

/*! Constructs a memory range whose validity is established by its caller. */
val memory_range : forall 'off 'len, memory_valid_range('off, 'len). (int('off), int('len)) -> MemoryRangeFields(
        'off,
        'len,
    )
function memory_range(off, len) = struct { off = off, len = len }

/*! The canonical inactive range used for a zero-sized operand or halt. */
let EMPTY_MEMORY_RANGE : MemoryRangeFields(0, 0) = memory_range(0, 0)

/*! The canonical inactive memory operand. */
let EMPTY_MEMORY_ACCESS : MemoryAccessFields(0, 0, 0) = struct { range = EMPTY_MEMORY_RANGE, required_size = 0 }

/*! An absolute byte position in the code arena. */
type code_pointer = range(0, code_region_bound)
/*! A contract-code length. */
type code_length = range(0, code_region_bound)
/*! The representation invariant required of executable code, including
    enough cursor headroom for a complete PUSH32 immediate. */
type code_valid_length('len : Int) -> Bool =
    0 <= 'len & 'len + 32 <= code_region_bound
/*! Embeds an arbitrary mathematical byte count in an EVM word after checking
    the EVM-word boundary. */
val word_of_nat_byte_count : forall 'value, 0 <= 'value. int('value) -> word
function word_of_nat_byte_count(value) =
    if value < 2 ^ 256 then {
        u256(value)
    } else {
        assert(false);
        WORD_ZERO
    }

/*! Embeds a representable source byte count in an EVM word. Optimized models
    use the source-length invariant to select their native-width instance. */
val word_of_source_byte_count : forall 'value, source_valid_length('value). int('value) -> word
function word_of_source_byte_count(value) = word_of_nat_byte_count(value)

/* Rounded quantities such as MSIZE retain the guarded mathematical arm;
 * source/code cursors select the representation-bounded arm. */
overload word_of_byte_count = {word_of_source_byte_count, word_of_nat_byte_count}
