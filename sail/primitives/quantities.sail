/* Protocol quantities are mathematical naturals with semantic bounds. The
 * canonical model never uses a fixed-width bitvector merely because a wire
 * encoding or host implementation happens to use 64 bits. */

type protocol_quantity = range(0, 2 ^ 64 - 1)

type account_nonce = protocol_quantity
type block_number = protocol_quantity
type block_timestamp = protocol_quantity
type blob_gas = protocol_quantity
type blob_count = protocol_quantity
type blob_fee_update_fraction = protocol_quantity
type chain_identifier = protocol_quantity
type protocol_fork_index = protocol_quantity
type slot_number = protocol_quantity
type withdrawal_index = protocol_quantity
type validator_index = protocol_quantity
type withdrawal_amount = protocol_quantity

/* Source-backed collections cannot contain more entries than their encoded
 * byte source. Keeping their counters bounded gives optimized backends a
 * native representation without leaking that representation into the model. */
type item_count = protocol_quantity
type item_index = protocol_quantity

/* Small structural domains carry their protocol bounds directly. */
type frame_depth = range(0, 1024)
type push_width = range(0, 32)
type operand_stack_height = range(0, 1024)
type stack_index = range(0, 16)
type stack_operation_index = range(1, 16)
type log_topic_count = range(0, 4)
type y_parity = range(0, 1)
type merkle_depth = range(0, 64)
type opcode = range(0, 255)
type precompile_id = range(1, 256)
type blake2_rounds = range(0, 2 ^ 32 - 1)
type bls_discount = range(0, 2 ^ 16 - 1)
type bloom_bit_index = range(0, 2047)
type bloom_limb_index = range(0, 31)
type bloom_limb_bit = range(0, 63)
type ancestor_index = range(0, 255)

/* Exact division of bounded protocol quantities is only used with small,
 * positive structural divisors. */
type protocol_divisor = range(1, 256)

function word_to_account_nonce(value : word) -> option(account_nonce) = {
    let amount = unsigned(value);
    if amount <= 2 ^ 64 - 1 then {
        Some(amount)
    } else {
        None()
    }
}

function word_to_precompile_id(value : word) -> option(precompile_id) = {
    let candidate = unsigned(value[8 .. 0]);
    if (value[255 .. 9] == sail_zeros(247)) & (1 <= candidate) & (candidate <= 256) then {
        Some(candidate)
    } else {
        None()
    }
}

function word_of_precompile_id(value : precompile_id) -> word =
    word_of_nat(value)

function account_nonce_increment(value : account_nonce) -> account_nonce = {
    assert(value < 2 ^ 64 - 1);
    value + 1
}

function protocol_quantity_increment(value : protocol_quantity) -> protocol_quantity = {
    assert(value < 2 ^ 64 - 1);
    value + 1
}

function protocol_quantity_decrement(value : protocol_quantity) -> protocol_quantity = {
    assert(0 < value);
    value - 1
}

function frame_depth_increment(value : frame_depth) -> frame_depth =
    if value < 1024 then {
        value + 1
    } else {
        assert(false);
        0
    }

function item_count_increment(value : item_count) -> item_count = {
    protocol_quantity_increment(value)
}

function item_index_increment(value : item_index) -> item_index = {
    protocol_quantity_increment(value)
}

function merkle_depth_increment(value : merkle_depth) -> merkle_depth = {
    assert(value < 64);
    value + 1
}

function merkle_depth_decrement(value : merkle_depth) -> merkle_depth = {
    assert(0 < value);
    value - 1
}

/* Host-backed byte arenas are mathematical naturals in the canonical
 * specification. The constructor marks the point at which an exact protocol
 * quantity has been shown to fit the host byte domain; the optimized C build
 * gives this nominal type a fixed-width representation. */

newtype byte_quantity = ByteQuantity : nat

let BYTE_ZERO : byte_quantity = ByteQuantity(0)
let BYTE_ONE : byte_quantity = ByteQuantity(1)
let BYTE_QUANTITY_MAX : nat = 2 ^ 64 - 1
let MAX_BYTE_QUANTITY : byte_quantity = ByteQuantity(BYTE_QUANTITY_MAX)

/* Source, memory, and code positions are transparent mathematical naturals in
 * the specification. The optimized backend gives byte_quantity a native
 * fixed-width representation; raw EVM operands stay words or plain nat until the opcode has
 * established its specified overflow behaviour. */
type source_pointer = byte_quantity
type byte_length = byte_quantity

type memory_pointer = byte_quantity
type memory_length = byte_quantity

type code_pointer = byte_quantity
type code_length = byte_quantity

function nat_fits_limb(value : nat) -> bool = value <= BYTE_QUANTITY_MAX

function byte_quantity_fits_limb(ByteQuantity(value) : byte_quantity) -> bool =
    nat_fits_limb(value)

overload fits_limb = {
    nat_fits_limb,
    byte_quantity_fits_limb
}

/* Fixed-width host ABIs use this after the semantic caller has established
 * that its exact quantity is representable. */
function nat_to_limb(value : nat) -> limb = {
    assert(nat_fits_limb(value));
    get_slice_int(64, value, 0)
}

function byte_quantity_to_limb(ByteQuantity(value) : byte_quantity) -> limb =
    nat_to_limb(value)

overload to_limb = {
    nat_to_limb,
    byte_quantity_to_limb
}

function word_of_byte_quantity(ByteQuantity(value) : byte_quantity) -> word =
    word_of_nat(value)

overload word_of_quantity = {
    word_of_nat,
    word_of_byte_quantity
}

function exact_quotient(dividend : nat, divisor : nat) -> nat = {
    assert(divisor != 0);
    let quotient = tdiv_int(dividend, divisor);
    assert(quotient >= 0);
    quotient
}

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

function byte_quantity_add(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> byte_quantity = {
    assert(left <= BYTE_QUANTITY_MAX);
    assert(right <= BYTE_QUANTITY_MAX - left);
    ByteQuantity(left + right)
}

function byte_quantity_sub(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> byte_quantity = {
    assert(left <= BYTE_QUANTITY_MAX);
    assert(right <= left);
    ByteQuantity(left - right)
}

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
