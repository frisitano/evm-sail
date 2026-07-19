/* Generated-C representation overrides. This is a build input, not a second
 * specification: Sail replaces these definitions after loading the canonical
 * model. Proof backends continue to see the exact mathematical definitions. */

$[c_repr u256]
newtype word = U256 : bits(256)

$[c_repr fixed_bytes]
newtype address = Address : vector(20, dec, byte)

$[c_repr fixed_bytes]
newtype b256 = B256 : vector(32, dec, byte)

/* byte_quantity is represented as uint64_t in this build, so extracting a
 * host limb is an identity at the C level.  Keeping this override separate
 * from nat_to_limb avoids routing an already-bounded value through the
 * canonical arbitrary-precision nat bridge. */
function byte_quantity_to_limb(ByteQuantity(value) : byte_quantity) -> limb =
    get_slice_int(64, value, 0)

/* SSZ basic values are serialized little-endian into the first bytes of a
 * 32-byte chunk.  Build the nominal B256 value directly: converting through
 * bits(256) would throw away both native representations and rebuild them via
 * the generic lbits/vector runtimes. */
function htr_uint(v : protocol_quantity) -> hash = {
    let value : limb = get_slice_int(64, v, 0);
    let B256(initial) = b256_zero();
    var bytes = initial;
    bytes[31] = value[7 .. 0];
    bytes[30] = value[15 .. 8];
    bytes[29] = value[23 .. 16];
    bytes[28] = value[31 .. 24];
    bytes[27] = value[39 .. 32];
    bytes[26] = value[47 .. 40];
    bytes[25] = value[55 .. 48];
    bytes[24] = value[63 .. 56];
    B256(bytes)
}

function htr_u256(U256(value) : word) -> hash = {
    let B256(initial) = b256_zero();
    var bytes = initial;
    bytes[31] = value[7 .. 0];
    bytes[30] = value[15 .. 8];
    bytes[29] = value[23 .. 16];
    bytes[28] = value[31 .. 24];
    bytes[27] = value[39 .. 32];
    bytes[26] = value[47 .. 40];
    bytes[25] = value[55 .. 48];
    bytes[24] = value[63 .. 56];
    bytes[23] = value[71 .. 64];
    bytes[22] = value[79 .. 72];
    bytes[21] = value[87 .. 80];
    bytes[20] = value[95 .. 88];
    bytes[19] = value[103 .. 96];
    bytes[18] = value[111 .. 104];
    bytes[17] = value[119 .. 112];
    bytes[16] = value[127 .. 120];
    bytes[15] = value[135 .. 128];
    bytes[14] = value[143 .. 136];
    bytes[13] = value[151 .. 144];
    bytes[12] = value[159 .. 152];
    bytes[11] = value[167 .. 160];
    bytes[10] = value[175 .. 168];
    bytes[9] = value[183 .. 176];
    bytes[8] = value[191 .. 184];
    bytes[7] = value[199 .. 192];
    bytes[6] = value[207 .. 200];
    bytes[5] = value[215 .. 208];
    bytes[4] = value[223 .. 216];
    bytes[3] = value[231 .. 224];
    bytes[2] = value[239 .. 232];
    bytes[1] = value[247 .. 240];
    bytes[0] = value[255 .. 248];
    B256(bytes)
}

/* Keep the nominal hash/word boundary native in both directions.  The source
 * signatures remain bits(256), but --c-specialize represents that fixed width
 * with the same four limbs as word, so neither bridge needs dynamic lbits. */
function b256_from_bits(value : bits(256)) -> b256 =
    word_to_hash(U256(value))

function b256_to_bits(value : b256) -> bits(256) = {
    let U256(bits) = hash_to_word(value);
    bits
}

/* The canonical SSZ definitions express these layouts through bits(256).
 * Fill the fixed-byte result directly in the optimized extraction. */
function htr_addr(Address(address_bytes) : address) -> hash = {
    let B256(initial) = b256_zero();
    var bytes = initial;
    bytes[12] = address_bytes[0];
    bytes[13] = address_bytes[1];
    bytes[14] = address_bytes[2];
    bytes[15] = address_bytes[3];
    bytes[16] = address_bytes[4];
    bytes[17] = address_bytes[5];
    bytes[18] = address_bytes[6];
    bytes[19] = address_bytes[7];
    bytes[20] = address_bytes[8];
    bytes[21] = address_bytes[9];
    bytes[22] = address_bytes[10];
    bytes[23] = address_bytes[11];
    bytes[24] = address_bytes[12];
    bytes[25] = address_bytes[13];
    bytes[26] = address_bytes[14];
    bytes[27] = address_bytes[15];
    bytes[28] = address_bytes[16];
    bytes[29] = address_bytes[17];
    bytes[30] = address_bytes[18];
    bytes[31] = address_bytes[19];
    B256(bytes)
}

function optimized_htr_chunk_byte(bytes : ByteSlice, start : source_pointer, offset : byte_length) -> byte = {
    let cursor = start + offset;
    if cursor < bytes.len then {
        slice_byte(bytes, cursor)
    } else {
        0x00
    }
}

function htr_chunk(bytes : ByteSlice, chunk_index : item_index) -> hash = {
    let start : source_pointer = ByteQuantity(chunk_index) * WORD_BYTE_LENGTH;
    let B256(initial) = b256_zero();
    var chunk = initial;
    chunk[31] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(0));
    chunk[30] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(1));
    chunk[29] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(2));
    chunk[28] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(3));
    chunk[27] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(4));
    chunk[26] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(5));
    chunk[25] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(6));
    chunk[24] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(7));
    chunk[23] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(8));
    chunk[22] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(9));
    chunk[21] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(10));
    chunk[20] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(11));
    chunk[19] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(12));
    chunk[18] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(13));
    chunk[17] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(14));
    chunk[16] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(15));
    chunk[15] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(16));
    chunk[14] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(17));
    chunk[13] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(18));
    chunk[12] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(19));
    chunk[11] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(20));
    chunk[10] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(21));
    chunk[9] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(22));
    chunk[8] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(23));
    chunk[7] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(24));
    chunk[6] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(25));
    chunk[5] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(26));
    chunk[4] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(27));
    chunk[3] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(28));
    chunk[2] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(29));
    chunk[1] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(30));
    chunk[0] = optimized_htr_chunk_byte(bytes, start, ByteQuantity(31));
    B256(chunk)
}

function htr_chunk_count(ByteQuantity(byte_len) : byte_length) -> item_count = {
    let value = ByteQuantity(byte_len);
    let quotient = value / ByteQuantity(32);
    let remainder = value - quotient * ByteQuantity(32);
    let count_quantity = if remainder == BYTE_ZERO then {
            quotient
        } else {
            quotient + BYTE_ONE
        };
    let ByteQuantity(count) = count_quantity;
    if count <= BYTE_QUANTITY_MAX then {
        count
    } else {
        throw InvalidBlock(InvalidConfig)
    }
}

function htr_fixed_count(ByteQuantity(byte_len) : byte_length, ByteQuantity(item_len) : byte_length) -> item_count = {
    if item_len == 0 then {
        throw InvalidBlock(InvalidConfig)
    } else {
        let ByteQuantity(count) = ByteQuantity(byte_len) / ByteQuantity(item_len);
        if count <= BYTE_QUANTITY_MAX then {
            count
        } else {
            throw InvalidBlock(InvalidConfig)
        }
    }
}

function inline_node_from_slice(bytes : ByteSlice) -> InlineNode = {
    let length = bytes.len;
    if MPT_HASH_LENGTH <= length then {
        throw InvalidBlock(WitnessDeficient)
    };
    var data : bits(256) = word_to_bits(ZERO_WORD);
    foreach (offset from 0 to 30) {
        let byte_offset = ByteQuantity(offset);
        if byte_offset < length then {
            data = or_vec(sail_shiftleft(data, 8), sail_zero_extend(slice_byte(bytes, byte_offset), 256))
        }
    };
    struct { data = data, len = length }
}

/* Direct native bridges between the four-limb word representation and the
 * fixed byte containers.  The canonical model retains its bits(160/256)
 * definitions; the optimized extraction never materialises those generic
 * bitvectors.  Byte index zero is the least-significant byte. */
function optimized_bytes8_to_limb(
    b0 : byte,
    b1 : byte,
    b2 : byte,
    b3 : byte,
    b4 : byte,
    b5 : byte,
    b6 : byte,
    b7 : byte,
) -> (
    limb
) =
    append(b7, append(b6, append(b5, append(b4, append(b3, append(b2, append(b1, b0)))))))

function optimized_bytes4_to_limb(b0 : byte, b1 : byte, b2 : byte, b3 : byte) -> limb =
    sail_zero_extend(append(b3, append(b2, append(b1, b0))), 64)

function word_to_address(U256(value) : word) -> address = {
    var bytes : vector(20, dec, byte) = [
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
        ];
    bytes[0] = value[7 .. 0];
    bytes[1] = value[15 .. 8];
    bytes[2] = value[23 .. 16];
    bytes[3] = value[31 .. 24];
    bytes[4] = value[39 .. 32];
    bytes[5] = value[47 .. 40];
    bytes[6] = value[55 .. 48];
    bytes[7] = value[63 .. 56];
    bytes[8] = value[71 .. 64];
    bytes[9] = value[79 .. 72];
    bytes[10] = value[87 .. 80];
    bytes[11] = value[95 .. 88];
    bytes[12] = value[103 .. 96];
    bytes[13] = value[111 .. 104];
    bytes[14] = value[119 .. 112];
    bytes[15] = value[127 .. 120];
    bytes[16] = value[135 .. 128];
    bytes[17] = value[143 .. 136];
    bytes[18] = value[151 .. 144];
    bytes[19] = value[159 .. 152];
    Address(bytes)
}

function address_to_word(Address(bytes) : address) -> word =
    word_from_limbs(
        optimized_bytes8_to_limb(bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7]),
        optimized_bytes8_to_limb(bytes[8], bytes[9], bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]),
        optimized_bytes4_to_limb(bytes[16], bytes[17], bytes[18], bytes[19]),
        LIMB_ZERO,
    )

function word_to_hash(U256(value) : word) -> hash = {
    var bytes : vector(32, dec, byte) = [
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
        ];
    bytes[0] = value[7 .. 0];
    bytes[1] = value[15 .. 8];
    bytes[2] = value[23 .. 16];
    bytes[3] = value[31 .. 24];
    bytes[4] = value[39 .. 32];
    bytes[5] = value[47 .. 40];
    bytes[6] = value[55 .. 48];
    bytes[7] = value[63 .. 56];
    bytes[8] = value[71 .. 64];
    bytes[9] = value[79 .. 72];
    bytes[10] = value[87 .. 80];
    bytes[11] = value[95 .. 88];
    bytes[12] = value[103 .. 96];
    bytes[13] = value[111 .. 104];
    bytes[14] = value[119 .. 112];
    bytes[15] = value[127 .. 120];
    bytes[16] = value[135 .. 128];
    bytes[17] = value[143 .. 136];
    bytes[18] = value[151 .. 144];
    bytes[19] = value[159 .. 152];
    bytes[20] = value[167 .. 160];
    bytes[21] = value[175 .. 168];
    bytes[22] = value[183 .. 176];
    bytes[23] = value[191 .. 184];
    bytes[24] = value[199 .. 192];
    bytes[25] = value[207 .. 200];
    bytes[26] = value[215 .. 208];
    bytes[27] = value[223 .. 216];
    bytes[28] = value[231 .. 224];
    bytes[29] = value[239 .. 232];
    bytes[30] = value[247 .. 240];
    bytes[31] = value[255 .. 248];
    B256(bytes)
}

function hash_to_word(B256(bytes) : hash) -> word =
    word_from_limbs(
        optimized_bytes8_to_limb(bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7]),
        optimized_bytes8_to_limb(bytes[8], bytes[9], bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]),
        optimized_bytes8_to_limb(bytes[16], bytes[17], bytes[18], bytes[19], bytes[20], bytes[21], bytes[22], bytes[23]),
        optimized_bytes8_to_limb(bytes[24], bytes[25], bytes[26], bytes[27], bytes[28], bytes[29], bytes[30], bytes[31]),
    )

/* The canonical definition is the exact bits(256) concatenation.  This
 * equivalent spelling stays wholly inside the native four-limb word algebra
 * in the optimized C extraction. */
function word_from_limbs(limb_0 : limb, limb_1 : limb, limb_2 : limb, limb_3 : limb) -> word =
    word_or(
        limb_to_word(limb_0),
        word_or(
            word_shift_left_limb(limb_to_word(limb_1), 0x0000000000000040),
            word_or(
                word_shift_left_limb(limb_to_word(limb_2), 0x0000000000000080),
                word_shift_left_limb(limb_to_word(limb_3), 0x00000000000000C0),
            ),
        ),
    )

/* The canonical definition proves that a word fits one limb by slicing the
 * underlying bits(256).  The optimized representation already exposes the
 * same four limbs, so perform the proof directly and avoid a round-trip
 * through lbits. */
function word_to_limb(value : word) -> option(limb) =
    if (word_limb_1(value) == LIMB_ZERO) & (word_limb_2(value) == LIMB_ZERO) & (word_limb_3(value) == LIMB_ZERO) then {
        Some(word_limb_0(value))
    } else {
        None()
    }

/* Convert a protocol u64 into the low limb of a native EVM word without
 * passing through the canonical nat -> bits conversion.  The source type
 * still has its exact range semantics; this spelling is only an optimized-C
 * representation bridge. */
function optimized_protocol_quantity_to_word(value : protocol_quantity) -> word =
    limb_to_word(get_slice_int(64, value, 0))

function word_of_protocol_quantity(value : protocol_quantity) -> word =
    optimized_protocol_quantity_to_word(value)

/* k_blob_schedule.max is the active block limit.  Every schedule currently
 * accepted by protocol_profile has max <= 21 (Osaka/BPO2), which is the
 * separate compile-time ceiling that proves this multiplication fits the
 * optimized uint64_t representation.  A future larger accepted schedule must
 * deliberately raise this ceiling rather than silently changing semantics. */
function blob_gas_for_count(count : blob_count) -> blob_gas = {
    let active_maximum = blob_schedule_max();
    if (active_maximum <= 21) & (count <= active_maximum) then {
        131072 * count
    } else {
        throw InvalidBlock(InvalidConfig)
    }
}

/* The canonical Osaka calculation uses exact naturals for
 * parent_blob_gas_used * (maximum - target), then divides before narrowing.
 * A uint64_t product would reject valid configurations when the intermediate
 * exceeds 64 bits.  Widen both factors to the native four-limb word, where the
 * at-most-128-bit product and following addition are exact, and narrow only
 * after word_to_limb proves that the result fits the protocol quantity. */
function next_excess_blob_gas(
    parent_excess_blob_gas : blob_gas,
    parent_blob_gas_used : blob_gas,
    parent_base_fee_per_gas : word,
) -> (
    blob_gas
) = {
    let protocol_maximum : protocol_quantity = 18446744073709551615;
    if protocol_maximum - parent_excess_blob_gas < parent_blob_gas_used then {
        throw InvalidBlock(InvalidConfig)
    };
    let parent_blob_gas = parent_excess_blob_gas + parent_blob_gas_used;
    let target_blob_gas = blob_target_gas_per_block();
    if parent_blob_gas < target_blob_gas then {
        0
    } else {
        let price = blob_base_fee(parent_excess_blob_gas);
        if (k_fork >= Osaka) & blob_reserve_price_active(parent_base_fee_per_gas, price) then {
            let target = blob_schedule_target();
            let maximum = blob_schedule_max();
            if (maximum == 0) | (maximum < target) then {
                throw InvalidBlock(InvalidConfig)
            };
            let schedule_delta = maximum - target;
            let scaled = word_mul(
                optimized_protocol_quantity_to_word(parent_blob_gas_used),
                optimized_protocol_quantity_to_word(schedule_delta),
            );
            let share = word_divmod(scaled, optimized_protocol_quantity_to_word(maximum)).quotient;
            let next = word_add(optimized_protocol_quantity_to_word(parent_excess_blob_gas), share);
            if (word_limb_1(next) == LIMB_ZERO) & (word_limb_2(next) == LIMB_ZERO) & (word_limb_3(next) == LIMB_ZERO) then {
                unsigned(word_limb_0(next))
            } else {
                throw InvalidBlock(InvalidConfig)
            }
        } else {
            parent_blob_gas - target_blob_gas
        }
    }
}

/* The optimized extraction preserves the canonical live-gas bound while
 * representing the nominal type directly as uint64_t. The spare value 2^63
 * in the optimized cost domain means "at least as large as every admitted gas
 * limit". Calculators clamp only after their final monotone expression; a
 * saturated value must never be fed through a later subtraction or division. */
newtype gas = Gas : range(0, 2 ^ 63 - 1)

newtype gas_cost = GasCost : range(0, 2 ^ 63)

$[c_repr int64]
newtype gas_refund = GasRefund : int

$[c_repr int64]
newtype state_gas_delta = StateGasDelta : int

/* The canonical implementation follows EELS and widens the three unsigned
 * pools to mathematical integers in one helper.  The optimized implementation
 * instead crosses the represented-newtype boundary first, then uses its native
 * signed operators.  Each conversion is exact, and frame gas conservation
 * keeps the net delta within the signed live-gas range. */
function frame_state_gas_used() -> state_gas_delta = {
    let Gas(entry) = message.state_gas_reservoir;
    let Gas(remaining) = state_gas_remaining;
    let Gas(spilled) = state_gas_spilled;
    let signed_entry : state_gas_delta = StateGasDelta(entry);
    let signed_remaining : state_gas_delta = StateGasDelta(remaining);
    let signed_spilled : state_gas_delta = StateGasDelta(spilled);
    signed_entry - signed_remaining + signed_spilled
}

function gas_value_supported(value : protocol_quantity) -> bool =
    value <= 9223372036854775807

function nat_to_gas(value : protocol_quantity) -> gas = {
    assert(value <= 9223372036854775807);
    Gas(value)
}

function word_to_gas(value : word) -> option(gas) =
    match word_to_limb(value) {
        Some(limb_value) if limb_value[63] == bitzero => Some(Gas(unsigned(limb_value[62 .. 0]))),
        _ => None(),
    }

function gas_sum_supported(Gas(left) : gas, Gas(right) : gas) -> bool =
    if left <= 9223372036854775807 then {
        right <= 9223372036854775807 - left
    } else {
        false
    }

function gas_to_cost(Gas(amount) : gas) -> gas_cost = {
    assert(amount <= 9223372036854775807);
    GasCost(amount)
}

function word_checked_mul_gas(value : word, Gas(factor) : gas) -> option(word) = {
    assert(factor <= 9223372036854775807);
    word_checked_mul_limb(value, get_slice_int(64, factor, 0))
}

function protocol_quantity_quotient(value : protocol_quantity, divisor : protocol_divisor) -> protocol_quantity = {
    let quotient = tdiv_int(value, divisor);
    assert(0 <= quotient);
    assert(quotient <= value);
    quotient
}

function optimized_gas_cost_sentinel() -> limb =
    0x8000000000000000

function optimized_unaffordable_gas_cost() -> gas_cost =
    GasCost(9223372036854775808)

function optimized_gas_cost_from_limb(value : limb) -> gas_cost =
    if value[63] == bitzero then {
        GasCost(unsigned(value[62 .. 0]))
    } else {
        optimized_unaffordable_gas_cost()
    }

function optimized_gas_cost_multiply(GasCost(left) : gas_cost, GasCost(right) : gas_cost) -> gas_cost = {
    if (left == 0) | (right == 0) then {
        GAS_COST_ZERO
    } else {
        let GasCost(limit) = optimized_unaffordable_gas_cost();
        if tdiv_int(limit, left) < right then {
            optimized_unaffordable_gas_cost()
        } else {
            let product = left * right;
            assert(product <= limit);
            GasCost(product)
        }
    }
}

function gas_cost_add(GasCost(left) : gas_cost, GasCost(right) : gas_cost) -> gas_cost = {
    let GasCost(limit) = optimized_unaffordable_gas_cost();
    if right <= limit - left then {
        GasCost(left + right)
    } else {
        optimized_unaffordable_gas_cost()
    }
}

function gas_cost_add_constant(left : gas_cost, GasConstant(right) : gas_constant) -> gas_cost =
    gas_cost_add(left, GasCost(right))

function gas_constant_add(GasConstant(left) : gas_constant, GasConstant(right) : gas_constant) -> gas_cost =
    gas_cost_add(GasCost(left), GasCost(right))

function gas_cost_scale(cost : gas_cost, factor : nat) -> gas_cost =
    if (cost == GAS_COST_ZERO) | (factor == 0) then {
        GAS_COST_ZERO
    } else {
        let GasCost(limit) = optimized_unaffordable_gas_cost();
        if factor <= limit then {
            optimized_gas_cost_multiply(cost, GasCost(factor))
        } else {
            optimized_unaffordable_gas_cost()
        }
    }

function gas_cost_scale_byte_quantity(cost : gas_cost, ByteQuantity(factor) : byte_quantity) -> gas_cost =
    if (cost == GAS_COST_ZERO) | (factor == 0) then {
        GAS_COST_ZERO
    } else {
        let GasCost(limit) = optimized_unaffordable_gas_cost();
        if factor <= limit then {
            optimized_gas_cost_multiply(cost, GasCost(factor))
        } else {
            optimized_unaffordable_gas_cost()
        }
    }

function gas_cost_scale_protocol_quantity(cost : gas_cost, factor : protocol_quantity) -> gas_cost =
    if (cost == GAS_COST_ZERO) | (factor == 0) then {
        GAS_COST_ZERO
    } else {
        let GasCost(limit) = optimized_unaffordable_gas_cost();
        if factor <= limit then {
            optimized_gas_cost_multiply(cost, GasCost(factor))
        } else {
            optimized_unaffordable_gas_cost()
        }
    }

function gas_constant_scale(GasConstant(schedule_cost) : gas_constant, factor : nat) -> gas_cost =
    gas_cost_scale(GasCost(schedule_cost), factor)

function gas_constant_scale_byte_quantity(
    GasConstant(schedule_cost) : gas_constant,
    factor : byte_quantity,
) -> (
    gas_cost
) =
    gas_cost_scale_byte_quantity(GasCost(schedule_cost), factor)

function gas_constant_scale_protocol_quantity(
    GasConstant(schedule_cost) : gas_constant,
    factor : protocol_quantity,
) -> (
    gas_cost
) =
    gas_cost_scale_protocol_quantity(GasCost(schedule_cost), factor)

/* Every ScaledBlobValue remainder is strictly below denominator. Expressing
 * the carry through the remaining gap preserves exact addition without ever
 * constructing the potentially 65-bit sum of two uint64_t remainders. */
function scaled_blob_add(
    left : ScaledBlobValue,
    right : ScaledBlobValue,
    denominator : blob_fee_update_fraction,
) -> (
    option(ScaledBlobValue)
) = {
    if (denominator == 0) | (denominator <= left.remainder) | (denominator <= right.remainder) then {
        return None()
    };
    let gap = protocol_quantity_sub(denominator, left.remainder);
    var carry : bool = false;
    var remainder : protocol_quantity = 0;
    if gap <= right.remainder then {
        carry = true;
        remainder = protocol_quantity_sub(right.remainder, gap)
    } else {
        remainder = protocol_quantity_add(left.remainder, right.remainder)
    };
    let whole : word = match word_checked_add(left.whole, right.whole) {
        Some(value) => value,
        None() => return None(),
    };
    if carry then {
        match word_checked_add(whole, WORD_ONE) {
            Some(value) => Some(struct { whole = value, remainder = remainder }),
            None() => None(),
        }
    } else {
        Some(struct { whole = whole, remainder = remainder })
    }
}

function gas_quotient(Gas(value) : gas, divisor : gas_divisor) -> gas = {
    let quotient = tdiv_int(value, divisor);
    assert(0 <= quotient);
    assert(quotient <= value);
    Gas(quotient)
}

function gas_cost_quotient(GasCost(cost) : gas_cost, divisor : gas_divisor) -> gas_cost = {
    let quotient = tdiv_int(cost, divisor);
    assert(0 <= quotient);
    assert(quotient <= cost);
    GasCost(quotient)
}

function capped_gas_refund(GasRefund(refund) : gas_refund, limit : gas) -> gas = {
    let GasRefund(signed_limit) = gas_to_refund(limit);
    if refund <= 0 then {
        GAS_ZERO
    } else {
        if refund <= signed_limit then {
            assert(not_bool(refund <= 0));
            assert(refund <= 9223372036854775807);
            Gas(refund)
        } else {
            limit
        }
    }
}

function bloom_bit_mask(bit_to_set : bloom_limb_bit) -> limb =
    sail_shiftleft(LIMB_ONE, bit_to_set)

function bloom_set_bit(bloom : LogsBloom, bit_to_set : bloom_bit_index) -> LogsBloom = {
    var out = bloom;
    let quotient = tdiv_int(bit_to_set, 64);
    assert(0 <= quotient);
    assert(quotient <= 31);
    let natural_limb : bloom_limb_index = quotient;
    let limb_index : bloom_limb_index = 31 - natural_limb;
    let remainder = tmod_int(bit_to_set, 64);
    assert(0 <= remainder);
    assert(remainder <= 63);
    let bit_in_limb : bloom_limb_bit = remainder;
    let mask = bloom_bit_mask(bit_in_limb);
    out[limb_index] = or_vec(out[limb_index], mask);
    out
}

/* This generic helper is retained for the canonical API. Hot calculators
 * whose expression decreases after a wide intermediate are replaced below. */
function gas_product_quotient(left : nat, right : nat, divisor : gas_divisor) -> gas_cost = {
    let cost = exact_quotient(left * right, divisor);
    if (2 ^ 63) <= cost then {
        optimized_unaffordable_gas_cost()
    } else {
        GasCost(cost)
    }
}

/* Exact floor(left * right / divisor), saturated only once it cannot be paid
 * by any admitted frame.  Divide each operand first so every native product
 * below is bounded by one of the original uint64_t operands. */
function gas_product_quotient_bytes(
    ByteQuantity(left) : byte_quantity,
    ByteQuantity(right) : byte_quantity,
    divisor : gas_divisor,
) -> (
    gas_cost
) = {
    if (left == 0) | (right == 0) then {
        return GAS_COST_ZERO
    };
    let left_quotient = tdiv_int(left, divisor);
    let left_remainder = tmod_int(left, divisor);
    let right_quotient = tdiv_int(right, divisor);
    let right_remainder = tmod_int(right, divisor);
    let GasCost(limit) = optimized_unaffordable_gas_cost();
    if (left_quotient != 0) & (tdiv_int(limit, left_quotient) < right) then {
        return optimized_unaffordable_gas_cost()
    };
    let whole = left_quotient * right;
    let residual_whole = left_remainder * right_quotient;
    let residual_fraction = tdiv_int(left_remainder * right_remainder, divisor);
    let residual = residual_whole + residual_fraction;
    assert(whole <= limit);
    if limit - whole < residual then {
        optimized_unaffordable_gas_cost()
    } else {
        let cost = whole + residual;
        assert(0 <= cost);
        assert(cost <= limit);
        GasCost(cost)
    }
}

function optimized_memory_words(size : limb) -> limb = {
    let quotient = sail_shiftright(size, 5);
    if and_vec(size, 0x000000000000001F) == LIMB_ZERO then {
        quotient
    } else {
        add_bits(quotient, LIMB_ONE)
    }
}

function memory_word_count_bytes(size : byte_quantity) -> byte_quantity =
    ByteQuantity(unsigned(optimized_memory_words(to_limb(size))))

/* Saturate a non-empty word range whose exact endpoint cannot be represented
 * by the host ABI.  Under the optimized build's gas-limit assumption that
 * sentinel endpoint is necessarily unaffordable, while the canonical build
 * continues to compute the full mathematical natural. */
function memory_required_size(start : word, size : word) -> byte_quantity =
    if word_is_zero(size) then {
        BYTE_ZERO
    } else {
        match word_to_limb(start) {
            Some(start_limb) => match word_to_limb(size) {
                Some(size_limb) => {
                    let endpoint = add_bits(start_limb, size_limb);
                    if limb_ult(endpoint, start_limb) then {
                        ByteQuantity(18446744073709551615)
                    } else {
                        ByteQuantity(unsigned(endpoint))
                    }
                },
                None() => ByteQuantity(18446744073709551615),
            },
            None() => ByteQuantity(18446744073709551615),
        }
    }

/* C_mem(new) - C_mem(old) is calculated as one fixed-width expression.
 * Saturating the two absolute C_mem values first would make sentinel -
 * sentinel incorrectly look free. A one-limb byte length has at most 2^59 words,
 * so both squares fit exactly in the existing 256-bit word algebra. */
function memory_expansion_gas(required_size : byte_quantity) -> gas_cost = {
    let required = to_limb(required_size);
    let old_size = evm_memory_size();
    let new_words = optimized_memory_words(required);
    let old_words = optimized_memory_words(to_limb(old_size));
    if limb_ule(new_words, old_words) then {
        GAS_COST_ZERO
    } else {
        let new_word = limb_to_word(new_words);
        let old_word = limb_to_word(old_words);
        let linear = word_mul(limb_to_word(0x0000000000000003), word_sub(new_word, old_word));
        let new_quadratic = word_shift_right_limb(word_mul(new_word, new_word), 0x0000000000000009);
        let old_quadratic = word_shift_right_limb(word_mul(old_word, old_word), 0x0000000000000009);
        let cost = word_add(linear, word_sub(new_quadratic, old_quadratic));
        if word_ule(limb_to_word(optimized_gas_cost_sentinel()), cost) then {
            optimized_unaffordable_gas_cost()
        } else {
            match word_to_limb(cost) {
                Some(value) => optimized_gas_cost_from_limb(value),
                None() => optimized_unaffordable_gas_cost(),
            }
        }
    }
}

/* MODEXP deliberately widens through a 256-bit integer. Once its three
 * lengths fit one limb, the complete EIP-2565/7883 expression is below 2^190, so
 * the EVM word algebra is exact rather than wrapping. */
function modexp_gas(input : ByteSlice) -> option(gas_cost) = {
    let base_length_word = pc_word(input, ByteQuantity(0), ByteQuantity(32));
    let exponent_length_word = pc_word(input, ByteQuantity(32), ByteQuantity(32));
    let modulus_length_word = pc_word(input, ByteQuantity(64), ByteQuantity(32));
    let osaka = k_fork >= Osaka;
    let osaka_limit = limb_to_word(0x0000000000000400);

    if osaka &
        (  word_ult(osaka_limit, base_length_word)
        | word_ult(osaka_limit, exponent_length_word)
        | word_ult(osaka_limit, modulus_length_word)) then {
        return None()
    };

    if not_bool(osaka) & word_is_zero(base_length_word) & word_is_zero(modulus_length_word) then {
        return Some(GasCost(200))
    };

    let base_length : limb = match word_to_limb(base_length_word) {
        Some(value) => value,
        None() => return Some(optimized_unaffordable_gas_cost()),
    };
    let exponent_length : limb = match word_to_limb(exponent_length_word) {
        Some(value) => value,
        None() => return Some(optimized_unaffordable_gas_cost()),
    };
    let modulus_length : limb = match word_to_limb(modulus_length_word) {
        Some(value) => value,
        None() => return Some(optimized_unaffordable_gas_cost()),
    };

    let maximum_length = if limb_ult(modulus_length, base_length) then { base_length } else { modulus_length };
    let length_words_parts = limb_divmod(maximum_length, 0x0000000000000008);
    let length_words = if length_words_parts.remainder == LIMB_ZERO then {
            length_words_parts.quotient
        } else {
            add_bits(length_words_parts.quotient, LIMB_ONE)
        };
    let length_words_word = limb_to_word(length_words);
    let words_squared = word_mul(length_words_word, length_words_word);
    let multiplication_complexity = if osaka & limb_ule(maximum_length, 0x0000000000000020) then {
            limb_to_word(0x0000000000000010)
        } else {
            if osaka then {
                word_shift_left_one(words_squared)
            } else {
                words_squared
            }
        };

    /* Only the top 96 limb values would overflow the native source-pointer
     * addition.  Their MODEXP multiplication complexity is already far above
     * the optimized unaffordable sentinel, so clamp before forming an offset. */
    if limb_ult(0xffffffffffffff9f, base_length) then {
        return Some(optimized_unaffordable_gas_cost())
    };
    let exponent_offset : source_pointer = ByteQuantity(96) + ByteQuantity(unsigned(base_length));
    let iteration_count : word = if limb_ule(exponent_length, 0x0000000000000020) then {
            let exponent_bits = word_bit_length(
                pc_word(input, exponent_offset, ByteQuantity(unsigned(exponent_length))),
            );
            if exponent_bits <= 1 then {
                WORD_ONE
            } else {
                word_of_protocol_quantity(exponent_bits - 1)
            }
        } else {
            let head_bits = word_bit_length(pc_word(input, exponent_offset, ByteQuantity(32)));
            let high_bits = if head_bits == 0 then {
                    0
                } else {
                    head_bits - 1
                };
            let multiplier = if osaka then { 0x0000000000000010 } else { 0x0000000000000008 };
            word_add(
                word_mul(limb_to_word(multiplier), limb_to_word(sub_bits(exponent_length, 0x0000000000000020))),
                word_of_protocol_quantity(high_bits),
            )
        };

    let product = word_mul(multiplication_complexity, iteration_count);
    let calculated = if osaka then {
            product
        } else {
            word_divmod(product, limb_to_word(0x0000000000000003)).quotient
        };
    let minimum = if osaka then {
            limb_to_word(0x00000000000001F4)
        } else {
            limb_to_word(0x00000000000000C8)
        };
    let cost = if word_ult(calculated, minimum) then { minimum } else { calculated };
    if word_ule(limb_to_word(0x8000000000000000), cost) then {
        Some(optimized_unaffordable_gas_cost())
    } else {
        match word_to_limb(cost) {
            Some(value) => Some(optimized_gas_cost_from_limb(value)),
            None() => Some(optimized_unaffordable_gas_cost()),
        }
    }
}

/* Length words are attacker-controlled 256-bit values.  The canonical path
 * converts them to exact naturals and then rejects values above the accelerator
 * ceiling; the optimized path can reject every multi-limb value immediately
 * because it necessarily exceeds that same ceiling. */
function optimized_modexp_length(value : word) -> option(byte_length) =
    match word_to_limb(value) {
        Some(length) => Some(ByteQuantity(unsigned(length))),
        None() => None(),
    }

function run_modexp(input : ByteSlice) -> PrecompileResult = {
    let base_len : byte_length = match optimized_modexp_length(pc_word(input, ByteQuantity(0), ByteQuantity(32))) {
        Some(length) => length,
        None() => return precompile_failure(),
    };
    let exponent_len : byte_length = match optimized_modexp_length(pc_word(input, ByteQuantity(32), ByteQuantity(32))) {
        Some(length) => length,
        None() => return precompile_failure(),
    };
    let modulus_len : byte_length = match optimized_modexp_length(pc_word(input, ByteQuantity(64), ByteQuantity(32))) {
        Some(length) => length,
        None() => return precompile_failure(),
    };
    if modulus_len == BYTE_ZERO then {
        precompile_success(EMPTY_SLICE)
    } else {
        let maximum = ByteQuantity(2097152);
        if (maximum < base_len) | (maximum < exponent_len) | (maximum < modulus_len) then {
            precompile_failure()
        } else {
            let input_end = ByteQuantity(96) + base_len + exponent_len + modulus_len;
            if maximum < input_end then {
                precompile_failure()
            } else {
                accelerator_result(accelerator_modexp(input, base_len, exponent_len, modulus_len), modulus_len)
            }
        }
    }
}

function kzg_versioned_hash_matches(input : ByteSlice) -> bool = {
    let commitment_hash = sha256([|BytesSlice(sub_slice(input, KZG_COMMITMENT_OFFSET, KZG_COMMITMENT_LENGTH))|]);
    let payload_mask = U256(0x00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff);
    let version = U256(0x0100000000000000000000000000000000000000000000000000000000000000);
    let expected = word_or(version, word_and(hash_to_word(commitment_hash), payload_mask));
    slice_load(input, BYTE_ZERO) == expected
}

/* Divide the pair count first so the EIP-2537 product never needs a wide
 * temporary: a*k/1000 = a*(k/1000) + floor(a*(k%1000)/1000). */
function bls_msm_gas(
    table : vector(128, inc, bits(16)),
    GasConstant(base) : gas_constant,
    maxd : bls_discount,
    k : byte_quantity,
) -> (
    gas_cost
) = {
    if k == BYTE_ZERO then {
        return GAS_COST_ZERO
    };
    var discount : gas_cost = GasCost(maxd);
    if k < ByteQuantity(128) then {
        let target = ByteQuantity(128) - k;
        var cursor : byte_quantity = BYTE_ZERO;
        foreach (index from 0 to 127) {
            if cursor == target then {
                discount = GasCost(unsigned(table[index]))
            };
            cursor = cursor + BYTE_ONE
        }
    };
    let scaled_base = optimized_gas_cost_multiply(GasCost(base), discount);
    if scaled_base == GAS_COST_ZERO then {
        GAS_COST_ZERO
    } else {
        let quotient = k / ByteQuantity(1000);
        let remainder = k - quotient * ByteQuantity(1000);
        let whole = gas_cost_scale_byte_quantity(scaled_base, quotient);
        let residual_product = gas_cost_scale_byte_quantity(scaled_base, remainder);
        let residual = gas_cost_quotient(residual_product, 1000);
        gas_cost_add(whole, residual)
    }
}

$[c_repr uint64]
newtype byte_quantity = ByteQuantity : nat

type source_pointer = byte_quantity
type byte_length = byte_quantity

type memory_pointer = byte_quantity
type memory_length = byte_quantity

type code_pointer = byte_quantity
type code_length = byte_quantity

/* The optimized host can address at most one uint64_t byte arena.  A nonzero
 * EVM range whose endpoint does not fit that arena is necessarily
 * unaffordable under the admitted gas regime, so refine it to OutOfGas at the
 * conversion boundary. */
function word_to_byte_quantity(value : word) -> byte_quantity =
    match word_to_limb(value) {
        Some(amount) => ByteQuantity(unsigned(amount)),
        None() => BYTE_ZERO,
    }

function word_of_byte_quantity(ByteQuantity(value) : byte_quantity) -> word =
    limb_to_word(get_slice_int(64, value, 0))

function word_of_gas(Gas(value) : gas) -> word =
    limb_to_word(get_slice_int(64, value, 0))

function word_to_byte_quantity_or_oog(value : word) -> byte_quantity =
    match word_to_limb(value) {
        Some(amount) => ByteQuantity(unsigned(amount)),
        None() => {
            exc_halt(OutOfGas);
            BYTE_ZERO
        },
    }

function optimized_max_byte_quantity() -> byte_quantity =
    ByteQuantity(18446744073709551615)

function byte_quantity_fits_limb(_ : byte_quantity) -> bool =
    true

function byte_quantity_not_equal(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> bool =
    not_bool(left == right)

function byte_quantity_add(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> byte_quantity = {
    let ByteQuantity(maximum) = optimized_max_byte_quantity();
    assert(right <= maximum - left);
    ByteQuantity(left + right)
}

function byte_quantity_sub(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> byte_quantity = {
    assert(right <= left);
    ByteQuantity(left - right)
}

function byte_quantity_mul(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> byte_quantity = {
    let ByteQuantity(maximum) = optimized_max_byte_quantity();
    if left == 0 then {
        BYTE_ZERO
    } else {
        assert(right <= tdiv_int(maximum, left));
        ByteQuantity(left * right)
    }
}

function byte_quantity_quotient(
    ByteQuantity(dividend) : byte_quantity,
    ByteQuantity(divisor) : byte_quantity,
) -> (
    byte_quantity
) = {
    assert(not_bool(divisor == 0));
    let quotient = tdiv_int(dividend, divisor);
    assert(0 <= quotient);
    assert(quotient <= dividend);
    ByteQuantity(quotient)
}
