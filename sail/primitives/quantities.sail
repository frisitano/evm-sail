/*md
# Protocol quantities

Semantic numeric domains used outside the EVM's 256-bit word algebra.
Protocol fields retain their wire bounds, structural counters carry the
limits imposed by their data structures, and byte positions remain exact
non-negative quantities after operand validation.

## Types

The aliases below prevent unrelated protocol and structural quantities from
being interchanged accidentally while preserving their mathematical values.

## Constants

`BYTE_ZERO`, `BYTE_ONE`, and `MAX_BYTE_QUANTITY` delimit the byte-addressable
domain used by source-backed slices and memory regions.
*/

/* Protocol quantities are mathematical naturals with semantic bounds. The
 * canonical model never uses a fixed-width bitvector merely because a wire
 * encoding or host implementation happens to use 64 bits. */

/*! An unsigned quantity carried by an Ethereum protocol field. */
type protocol_quantity = range(0, 2 ^ 64 - 1)

/*! An account transaction-count nonce. */
type account_nonce = protocol_quantity
/*! An execution block number. */
type block_number = protocol_quantity
/*! An execution block timestamp in seconds. */
type block_timestamp = protocol_quantity
/*! A quantity of blob gas. */
type blob_gas = protocol_quantity
/*! A number of blobs. */
type blob_count = protocol_quantity
/*! The denominator governing excess-blob-gas fee adjustment. */
type blob_fee_update_fraction = protocol_quantity
/*! An EIP-155 chain identifier. */
type chain_identifier = protocol_quantity
/*! The ordinal assigned to a supported protocol fork. */
type protocol_fork_index = protocol_quantity
/*! A beacon-chain slot number. */
type slot_number = protocol_quantity
/*! An index in the withdrawal sequence. */
type withdrawal_index = protocol_quantity
/*! A consensus-layer validator index. */
type validator_index = protocol_quantity
/*! A withdrawal amount denominated in gwei. */
type withdrawal_amount = protocol_quantity

/* Source-backed collections cannot contain more entries than their encoded
 * byte source. Keeping their counters bounded gives optimized backends a
 * native representation without leaking that representation into the model. */
/*! The number of entries in a source-backed collection. */
type item_count = protocol_quantity
/*! An index into a source-backed collection. */
type item_index = protocol_quantity

/* Small structural domains carry their protocol bounds directly. */
/*! The nesting depth of an execution frame. */
type frame_depth = range(0, 1024)
/*! The immediate-byte width of a PUSH instruction. */
type push_width = range(0, 32)
/*! The number of words on an operand stack. */
type operand_stack_height = range(0, 1024)
/*! An index into the top sixteen operand-stack entries. */
type stack_index = range(0, 16)
/*! A nonzero operand-stack position used by DUP and SWAP. */
type stack_operation_index = range(1, 16)
/*! The number of indexed topics attached to one log. */
type log_topic_count = range(0, 4)
/*! The parity bit used by transaction signatures. */
type y_parity = range(0, 1)
/*! A depth in a 64-level binary Merkle tree. */
type merkle_depth = range(0, 64)
/*! An EVM instruction byte. */
type opcode = range(0, 255)
/*! The one-based identifier of a precompiled contract. */
type precompile_id = range(1, 256)
/*! The round count supplied to the BLAKE2 compression precompile. */
type blake2_rounds = range(0, 2 ^ 32 - 1)
/*! A fixed-point discount factor used by BLS precompile pricing. */
type bls_discount = range(0, 2 ^ 16 - 1)
/*! A bit position in the 2048-bit log bloom. */
type bloom_bit_index = range(0, 2047)
/*! A 64-bit limb position in the log bloom. */
type bloom_limb_index = range(0, 31)
/*! A bit position within a log-bloom limb. */
type bloom_limb_bit = range(0, 63)
/*! An index into the 256 most recent ancestor block hashes. */
type ancestor_index = range(0, 255)

/* Exact division of bounded protocol quantities is only used with small,
 * positive structural divisors. */
/*! A small positive divisor used for exact protocol arithmetic. */
type protocol_divisor = range(1, 256)

/*! Converts a word to an account nonce when it fits the protocol field. */
function word_to_account_nonce(value : word) -> option(account_nonce) = {
    let amount = unsigned(value);
    if amount <= 2 ^ 64 - 1 then {
        Some(amount)
    } else {
        None()
    }
}

/*! Recognizes a word as a valid precompiled-contract identifier. */
function word_to_precompile_id(value : word) -> option(precompile_id) = {
    let candidate = unsigned(value[8 .. 0]);
    if (value[255 .. 9] == sail_zeros(247)) & (1 <= candidate) & (candidate <= 256) then {
        Some(candidate)
    } else {
        None()
    }
}

/*! Embeds a precompiled-contract identifier in an EVM word. */
function word_of_precompile_id(value : precompile_id) -> word =
    word_of_nat(value)

/*! Increments an account nonce after establishing that it is not maximal. */
function account_nonce_increment(value : account_nonce) -> account_nonce = {
    assert(value < 2 ^ 64 - 1);
    value + 1
}

/*! Increments a bounded protocol quantity. */
function protocol_quantity_increment(value : protocol_quantity) -> protocol_quantity = {
    assert(value < 2 ^ 64 - 1);
    value + 1
}

/*! Decrements a positive protocol quantity. */
function protocol_quantity_decrement(value : protocol_quantity) -> protocol_quantity = {
    assert(0 < value);
    value - 1
}

/*! Advances the call depth and rejects an attempt to exceed its bound. */
function frame_depth_increment(value : frame_depth) -> frame_depth =
    if value < 1024 then {
        value + 1
    } else {
        assert(false);
        0
    }

/*! Advances a source-backed collection count. */
function item_count_increment(value : item_count) -> item_count = {
    protocol_quantity_increment(value)
}

/*! Advances a source-backed collection index. */
function item_index_increment(value : item_index) -> item_index = {
    protocol_quantity_increment(value)
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

/* Host-backed byte arenas are mathematical naturals in the canonical
 * specification. The constructor marks the point at which an exact protocol
 * quantity has been shown to fit the host byte domain; the optimized C build
 * gives this nominal type a fixed-width representation. */

/*! An exact byte offset, length, or size after representability checks. */
newtype byte_quantity = ByteQuantity : nat

let BYTE_ZERO : byte_quantity = ByteQuantity(0)
let BYTE_ONE : byte_quantity = ByteQuantity(1)
let BYTE_QUANTITY_MAX : nat = 2 ^ 64 - 1
let MAX_BYTE_QUANTITY : byte_quantity = ByteQuantity(BYTE_QUANTITY_MAX)

/* Source, memory, and code positions are transparent mathematical naturals in
 * the specification. The optimized backend gives byte_quantity a native
 * fixed-width representation; raw EVM operands stay words or plain nat until the opcode has
 * established its specified overflow behaviour. */
/*! An absolute byte position in a named source region. */
type source_pointer = byte_quantity
/*! A byte-sequence length. */
type byte_length = byte_quantity

/*! An absolute byte position in the current EVM memory frame. */
type memory_pointer = byte_quantity
/*! A length or allocated size in EVM memory. */
type memory_length = byte_quantity

/*! An absolute byte position in the code arena. */
type code_pointer = byte_quantity
/*! A contract-code length. */
type code_length = byte_quantity

/*! Tests whether a natural number fits the byte-addressable domain. */
function nat_fits_limb(value : nat) -> bool = value <= BYTE_QUANTITY_MAX

/*! Tests whether a byte quantity fits the byte-addressable domain. */
function byte_quantity_fits_limb(ByteQuantity(value) : byte_quantity) -> bool =
    nat_fits_limb(value)

overload fits_limb = {
    nat_fits_limb,
    byte_quantity_fits_limb
}

/* Fixed-width host ABIs use this after the semantic caller has established
 * that its exact quantity is representable. */
/*! Converts a representable natural number to a 64-bit limb. */
function nat_to_limb(value : nat) -> limb = {
    assert(nat_fits_limb(value));
    get_slice_int(64, value, 0)
}

/*! Converts a representable byte quantity to a 64-bit limb. */
function byte_quantity_to_limb(ByteQuantity(value) : byte_quantity) -> limb =
    nat_to_limb(value)

overload to_limb = {
    nat_to_limb,
    byte_quantity_to_limb
}

/*! Embeds a byte quantity in an EVM word. */
function word_of_byte_quantity(ByteQuantity(value) : byte_quantity) -> word =
    word_of_nat(value)

overload word_of_quantity = {
    word_of_nat,
    word_of_byte_quantity
}

/*! Computes the non-negative integer quotient for a nonzero divisor. */
function exact_quotient(dividend : nat, divisor : nat) -> nat = {
    assert(divisor != 0);
    let quotient = tdiv_int(dividend, divisor);
    assert(quotient >= 0);
    quotient
}

/*! Divides a protocol quantity by a small positive divisor. */
function protocol_quantity_quotient(value : protocol_quantity, divisor : protocol_divisor) -> protocol_quantity = {
    let quotient = exact_quotient(value, divisor);
    assert(quotient <= value);
    quotient
}

function byte_quantity_equal(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> bool =
    left == right

function byte_quantity_not_equal(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> bool =
    left != right

function byte_quantity_lt(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> bool =
    left < right

function byte_quantity_le(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> bool =
    left <= right

function byte_quantity_gt(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> bool =
    left > right

function byte_quantity_ge(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> bool =
    left >= right

/*! Adds two byte quantities after checking the addressable bound. */
function byte_quantity_add(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> byte_quantity = {
    assert(left <= BYTE_QUANTITY_MAX);
    assert(right <= BYTE_QUANTITY_MAX - left);
    ByteQuantity(left + right)
}

/*! Subtracts a byte quantity without permitting underflow. */
function byte_quantity_sub(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> byte_quantity = {
    assert(left <= BYTE_QUANTITY_MAX);
    assert(right <= left);
    ByteQuantity(left - right)
}

/*! Multiplies byte quantities after checking the addressable bound. */
function byte_quantity_mul(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> byte_quantity = {
    assert(left <= BYTE_QUANTITY_MAX);
    if left == 0 then {
        BYTE_ZERO
    } else {
        assert(right <= exact_quotient(BYTE_QUANTITY_MAX, left));
        ByteQuantity(left * right)
    }
}

function nat_scale_byte_quantity(left : nat, ByteQuantity(right) : byte_quantity) -> nat =
    left * right

/*! Divides a byte quantity by a nonzero byte quantity. */
function byte_quantity_quotient(
    ByteQuantity(dividend) : byte_quantity,
    ByteQuantity(divisor) : byte_quantity,
) -> (
    byte_quantity
) = {
    assert(dividend <= BYTE_QUANTITY_MAX);
    ByteQuantity(exact_quotient(dividend, divisor))
}

overload operator == = {
    byte_quantity_equal
}

overload operator != = {
    byte_quantity_not_equal
}

overload operator < = {
    byte_quantity_lt
}

overload operator <= = {
    byte_quantity_le
}

overload operator > = {
    byte_quantity_gt
}

overload operator >= = {
    byte_quantity_ge
}

overload operator + = {
    byte_quantity_add
}

overload operator - = {
    byte_quantity_sub
}

overload operator * = {
    byte_quantity_mul,
    nat_scale_byte_quantity
}

overload operator / = {
    byte_quantity_quotient
}
