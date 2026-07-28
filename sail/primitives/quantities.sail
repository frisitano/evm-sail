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

/*! An execution block number. The Yellow Paper header scalar and EIP-1559
    arithmetic impose no fixed-width semantic bound; the stateless-input SSZ
    transport performs its own `uint64` decoding. */
type block_number = nat
/*! An execution block timestamp in seconds. As with the Yellow Paper header
    scalar, the canonical semantic value is a natural; SSZ width is checked at
    the input boundary rather than becoming the protocol type. */
type block_timestamp = nat

/*! Blob gas charged per blob, `2^17` (EIP-4844). */
type gas_per_blob_value : Int = 2 ^ 17

/*! A blob count admitted by the largest supported block schedule. The active
    schedule selects a tighter maximum below. */
type blob_count = range(0, 21)
/*! A blob count carried by one EIP-4844 transaction. The active protocol
    profile restricts this further to zero, six, or nine blobs. */
type transaction_blob_count = range(0, 9)
/*! A target selected by one of the blob schedules supported by the schema.
    This is distinct from an observed blob count, which may be any value in
    its contiguous range. */
type blob_schedule_target_count = {0, 3, 6, 10, 14}
/*! A maximum selected by one of the blob schedules supported by the schema. */
type blob_schedule_max_count = {0, 6, 9, 15, 21}
/*! The fork-selected maximum number of blobs carried by one transaction.
    Zero denotes a profile before blob transactions activate. */
type transaction_blob_limit = {0, 6, 9}
/*! Blob gas used by one supported block. The existential count records that
    every inhabitant is exactly a multiple of `GAS_PER_BLOB`; the active
    schedule selects a tighter maximum below. */
type blob_gas_used = {
    'count,
    0 <= 'count & 'count <= 21.
    int(gas_per_blob_value * 'count)
}
/*! Blob gas contributed by one transaction, retaining the exact blob count
    from which it was derived. */
type transaction_blob_gas = {
    'count,
    0 <= 'count & 'count <= 9.
    int(gas_per_blob_value * 'count)
}
/*! The accumulating EIP-4844 `excess_blob_gas` header field, encoded as
    `uint64` and not bounded by a single block's blob count. */
type excess_blob_gas_bound : Int = 2 ^ 64 - 1
/*! An accumulated excess-blob-gas value carried by a block header. */
type excess_blob_gas = range(0, excess_blob_gas_bound)

/*! Largest fee-update fraction in the supported blob schedules: BPO2's
    `BLOB_BASE_FEE_UPDATE_FRACTION = 11684671` (EIP-8135). */
type blob_fee_update_fraction_bound : Int = 11684671
/*! The positive denominator governing excess-blob-gas fee adjustment. */
type blob_fee_update_fraction = range(1, blob_fee_update_fraction_bound)
/*! A fee-update denominator selected by one of the supported blob schedules.
    Observed arithmetic still uses the broader positive denominator domain;
    configuration itself can only choose these protocol constants. */
type blob_schedule_fee_update_fraction = {1, 3338477, 5007716, 8346193, 11684671}

/*! Fork-selected limits retained in the validated protocol profile. Zero is
    used only where the corresponding rule is not active. */
type protocol_deployed_code_size_limit = {24576, 65536}
type protocol_initcode_size_limit = {0, 49152, 131072}
type protocol_transaction_gas_limit = {0, 16777216}
type protocol_transaction_execution_gas_limit = {0, 16777216}

/*! Largest chain identifier admitted by the typed-transaction wire decoder. */
type chain_identifier_bound : Int = 2 ^ 64 - 1

/*! A chain identifier. Typed-transaction chain identifiers and the stateless
    chain configuration are decoded as unsigned 64-bit integers. */
type chain_identifier = range(0, chain_identifier_bound)

/*! A beacon-chain slot number, explicitly a `uint64` in EIP-7843. */
type slot_number = range(0, 2 ^ 64 - 1)
/*! An EIP-4895 withdrawal index, encoded as SSZ `uint64`. */
type withdrawal_index = range(0, 2 ^ 64 - 1)
/*! An EIP-4895 validator index, encoded as SSZ `uint64`. */
type validator_index = range(0, 2 ^ 64 - 1)
/*! An EIP-4895 withdrawal amount in gwei, encoded as SSZ `uint64`. */
type withdrawal_amount = range(0, 2 ^ 64 - 1)

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

/*! The number of entries in a host-backed collection without a tighter
    schema-specific bound. Host tables are addressed by 64-bit ordinals in
    every executable build; protocol and structural counts use their own
    tighter singleton/range types instead. */
type item_count = range(0, 2 ^ 64 - 1)
/*! An index into a host-backed collection without a tighter schema bound. */
type item_index = range(0, 2 ^ 64 - 1)

/* Small structural domains carry their protocol bounds directly. */
/*! The nesting depth of an execution frame. */
type frame_depth = range(0, 1024)
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
/*! The largest one-based precompile identifier admitted by the model. */
type precompile_id_bound : Int = 256
/*! The one-based identifier of a precompiled contract. */
type precompile_id = range(1, precompile_id_bound)
/*! An active precompile identifier, or zero for an ordinary address. */
type precompile_selector = range(0, precompile_id_bound)
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
/*! An index into the 256 most recent ancestor block hashes. */
type ancestor_index = range(0, 255)

/*! Converts a word to an account nonce when it fits the protocol field. */
function word_to_account_nonce(value : word) -> option(account_nonce) = {
    if value <= sizeof(account_nonce_bound) then {
        Some(value)
    } else {
        None()
    }
}

/*! Embeds an EIP-2681 account nonce in the EVM word domain. */
function word_of_account_nonce(value : account_nonce) -> account_nonce = value

/*! Embeds an EIP-4895 withdrawal amount in the EVM word domain. */
function word_of_withdrawal_amount(value : withdrawal_amount) -> withdrawal_amount = value

/*! Embeds an EIP-7843 slot number in the EVM word domain. */
function word_of_slot_number(value : slot_number) -> slot_number = value

/*! Converts a semantic block number to the 256-bit value exposed by NUMBER. */
val word_of_block_number : forall 'value, 0 <= 'value. int('value) -> {'result,
                                                                          0
                                                                       <= 'result
                                                                       &  'result
                                                                       <= 'value
                                                                       &  'result
                                                                       <  2 ^ 256.
                                                                       int('result)}
function word_of_block_number(value) =
    if value < sizeof(word_modulus) then {
        value
    } else {
        assert(false);
        WORD_ZERO
    }

/*! Converts a semantic timestamp to the 256-bit value exposed by TIMESTAMP. */
val word_of_block_timestamp : forall 'value, 0 <= 'value. int('value) -> {'result,
                                                                             0
                                                                          <= 'result
                                                                          &  'result
                                                                          <= 'value
                                                                          &  'result
                                                                          <  2 ^ 256.
                                                                          int('result)}
function word_of_block_timestamp(value) =
    if value < sizeof(word_modulus) then {
        value
    } else {
        assert(false);
        WORD_ZERO
    }

/*! Converts a chain identifier to the value exposed by CHAINID. */
function word_of_chain_identifier(value : chain_identifier) -> chain_identifier = value

/*! Encodes a precompiled-contract identifier directly as its 20-byte
    address.  Identifiers currently occupy only the two least-significant
    bytes, including Osaka's `0x0100` P256VERIFY address. */
function precompile_id_to_address(value : precompile_id) -> address = {
    var result : address = ZERO_ADDRESS;
    result[19] = get_slice_int(8, value, 0);
    result[18] = get_slice_int(8, value, 8);
    result
}

/*! Increments an account nonce after establishing that it is not maximal. */
function account_nonce_increment(value : account_nonce) -> account_nonce = {
    assert(value < sizeof(account_nonce_bound));
    value + 1
}

/*! Advances the call depth and rejects an attempt to exceed its bound. */
function frame_depth_increment(value : frame_depth) -> frame_depth =
    if value < 1024 then {
        value + 1
    } else {
        assert(false);
        0
    }

/*! Consumes one validated EIP-4844 transaction blob hash. */
function transaction_blob_count_decrement(value : transaction_blob_count) -> transaction_blob_count =
    if value != 0 then {
        value - 1
    } else {
        0
    }

/*! Descends by one level in a bounded Merkle tree. */
function merkle_depth_increment(value : merkle_depth) -> merkle_depth = {
    assert(value < 64);
    value + 1
}

/*! Ascends by one level in a bounded Merkle tree. */
function merkle_depth_decrement(value : merkle_depth) -> merkle_depth = {
    assert(0 < value);
    value - 1
}

/* Source, memory, and code positions are transparent mathematical naturals in
 * the canonical specification. Their names document the byte domain expected
 * by each interface. The optimized splice supplies finite bounds from which
 * the C backend selects native representations. Raw EVM operands remain words
 * until the opcode has established its specified overflow behaviour. */
/*! Whether an offset/length pair is representable by the host. This is
    vacuous for the canonical natural-number model; optimized builds strengthen
    it with their concrete address-space bound. */
type host_valid_range('off : Int, 'len : Int) -> Bool =
    0 <= 'off & 0 <= 'len
/*! Whether a source-backed byte range is representable by the selected
    model. This is a semantic representation invariant, not an FFI type. */
type source_valid_range('off : Int, 'len : Int) -> Bool =
    0 <= 'off & 0 <= 'len
/*! Whether one host byte quantity is representable. Unlike
    `host_valid_range`, this does not assert that two quantities can be added
    without overflow. */
type host_valid_access('value : Int) -> Bool =
    0 <= 'value
/*! Whether a semantic source length is representable by the selected model.
    The canonical specification is unbounded; optimized builds strengthen
    this representation invariant independently of protocol limits. */
type source_valid_length('value : Int) -> Bool =
    0 <= 'value
/*! A position representable by the host byte-store interface. The canonical
    model is unbounded; production builds refine this representation. */
type host_access = nat
/*! An absolute byte position in a named source region. */
type source_pointer = nat
/*! A byte length or regular-layout count derived from a source region. */
type source_length = nat
/*! An opaque ordinal issued by the host rollback journal. The model may store
    and return this complete transaction-state snapshot cursor, but cannot
    inspect subsystem journals or replay the host's rollback mechanism. */
type journal_checkpoint = nat

/*! An absolute byte position in the current EVM memory frame. */
type memory_pointer = nat
/*! A length or allocated size in EVM memory. */
type memory_length = nat

/* A range describes the mathematical EVM address selected by an opcode.  Its
 * materializability is established separately by the memory-expansion cost:
 * an unaffordable range halts before it reaches the host byte store. */
/*! The invariant carried by every canonical EVM memory range. */
type memory_valid_range('off : Int, 'len : Int) -> Bool =
    0 <= 'off & 0 <= 'len

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

/*! Constructs a memory range whose validity is established by its caller. */
val memory_range : forall 'off 'len, memory_valid_range('off, 'len). (int('off), int('len)) -> MemoryRangeFields(
        'off,
        'len,
    )
function memory_range(off, len) = struct { off = off, len = len }

/*! The canonical inactive range used for a zero-sized operand or halt. */
let EMPTY_MEMORY_RANGE : MemoryRangeFields(0, 0) = memory_range(0, 0)

/*! An absolute byte position in the code arena. */
type code_pointer = nat
/*! A contract-code length. */
type code_length = nat
/*! The representation invariant required of executable code. The canonical
    model is mathematically unbounded; optimized builds strengthen this with
    enough cursor headroom for a complete PUSH32 immediate. */
type code_valid_length('len : Int) -> Bool =
    0 <= 'len
/*! An index into the 256-byte chunks of a JUMPDEST bitmap. */
type code_chunk_index = nat

/*! Embeds an arbitrary mathematical byte count in an EVM word after checking
    the EVM-word boundary. */
val word_of_nat_byte_count : forall 'value, 0 <= 'value. int('value) -> word
function word_of_nat_byte_count(value) =
    if value < 2 ^ 256 then {
        U256(value)
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
