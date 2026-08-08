#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint16_t G_zero = UINT16_C(0);


const uint16_t G_jumpdest = UINT16_C(1);


const uint16_t G_base = UINT16_C(2);


const uint16_t G_verylow = UINT16_C(3);


const uint16_t G_low = UINT16_C(5);


const uint16_t G_mid = UINT16_C(8);


const uint16_t G_high = UINT16_C(10);


const uint16_t G_warm_access = UINT16_C(100);


const uint16_t G_cold_sload = UINT16_C(2100);


const uint16_t G_cold_account = UINT16_C(2600);


const uint16_t G_keccak = UINT16_C(30);


const uint8_t G_keccak_word = UINT8_C(6);


const uint16_t G_copy_word = UINT16_C(3);


const uint8_t G_memory = UINT8_C(3);


const uint16_t G_log = UINT16_C(375);


const uint16_t G_logtopic = UINT16_C(375);


const uint16_t G_logdata = UINT16_C(8);


const uint16_t G_exp = UINT16_C(10);


const uint16_t G_expbyte = UINT16_C(50);


const uint16_t G_sset = UINT16_C(20000);


const uint16_t G_sreset = UINT16_C(2900);


const uint16_t R_sclear_pre_london = UINT16_C(15000);


const uint16_t R_sclear_london = UINT16_C(4800);


const uint16_t R_selfdestruct_pre_london = UINT16_C(24000);


const uint16_t G_create = UINT16_C(32000);


const uint8_t G_codedeposit = UINT8_C(200);


const uint16_t G_callvalue = UINT16_C(9000);


const uint64_t G_callstipend = UINT64_C(2300);


const uint16_t G_newaccount = UINT16_C(25000);


const uint16_t G_selfdestruct = UINT16_C(5000);


const uint8_t G_initcode_word = UINT8_C(2);


const uint16_t G_amsterdam_cold_account_access = UINT16_C(3000);


const uint16_t G_amsterdam_cold_storage_access = UINT16_C(3000);


const uint16_t G_amsterdam_storage_write = UINT16_C(10000);


const uint16_t G_amsterdam_account_write = UINT16_C(8000);


const uint16_t G_amsterdam_call_value = UINT16_C(10300);


const uint16_t G_amsterdam_create_access = UINT16_C(11000);


const uint16_t G_amsterdam_state_byte = UINT16_C(1530);


const uint32_t G_amsterdam_state_storage_set = UINT32_C(97920);


const uint32_t G_amsterdam_state_new_account = UINT32_C(183600);


const uint32_t G_amsterdam_state_auth_base = UINT32_C(35190);


const uint16_t R_amsterdam_storage_clear = UINT16_C(12480);


const uint64_t G_sstore_sentry = UINT64_C(2301);


const vector_128_uint_16 BLS_G1_DISCOUNT = {
  .len = 128,
  .data = {
      UINT16_C(519), UINT16_C(520), UINT16_C(520), UINT16_C(521), UINT16_C(522), UINT16_C(522), UINT16_C(523), UINT16_C(524),
      UINT16_C(525), UINT16_C(525), UINT16_C(526), UINT16_C(527), UINT16_C(528), UINT16_C(528), UINT16_C(529), UINT16_C(530),
      UINT16_C(531), UINT16_C(532), UINT16_C(532), UINT16_C(533), UINT16_C(534), UINT16_C(535), UINT16_C(536), UINT16_C(536),
      UINT16_C(537), UINT16_C(538), UINT16_C(539), UINT16_C(540), UINT16_C(540), UINT16_C(541), UINT16_C(542), UINT16_C(543),
      UINT16_C(544), UINT16_C(545), UINT16_C(546), UINT16_C(547), UINT16_C(547), UINT16_C(548), UINT16_C(549), UINT16_C(550),
      UINT16_C(551), UINT16_C(552), UINT16_C(553), UINT16_C(554), UINT16_C(555), UINT16_C(556), UINT16_C(557), UINT16_C(558),
      UINT16_C(559), UINT16_C(560), UINT16_C(561), UINT16_C(562), UINT16_C(563), UINT16_C(564), UINT16_C(565), UINT16_C(566),
      UINT16_C(567), UINT16_C(568), UINT16_C(569), UINT16_C(570), UINT16_C(572), UINT16_C(573), UINT16_C(574), UINT16_C(575),
      UINT16_C(576), UINT16_C(577), UINT16_C(579), UINT16_C(580), UINT16_C(581), UINT16_C(582), UINT16_C(584), UINT16_C(585),
      UINT16_C(586), UINT16_C(588), UINT16_C(589), UINT16_C(591), UINT16_C(592), UINT16_C(593), UINT16_C(595), UINT16_C(596),
      UINT16_C(598), UINT16_C(599), UINT16_C(601), UINT16_C(603), UINT16_C(604), UINT16_C(606), UINT16_C(608), UINT16_C(609),
      UINT16_C(611), UINT16_C(613), UINT16_C(615), UINT16_C(617), UINT16_C(619), UINT16_C(621), UINT16_C(623), UINT16_C(625),
      UINT16_C(627), UINT16_C(630), UINT16_C(632), UINT16_C(635), UINT16_C(637), UINT16_C(640), UINT16_C(642), UINT16_C(645),
      UINT16_C(648), UINT16_C(651), UINT16_C(654), UINT16_C(658), UINT16_C(661), UINT16_C(665), UINT16_C(669), UINT16_C(673),
      UINT16_C(677), UINT16_C(682), UINT16_C(687), UINT16_C(692), UINT16_C(698), UINT16_C(705), UINT16_C(712), UINT16_C(719),
      UINT16_C(728), UINT16_C(738), UINT16_C(750), UINT16_C(764), UINT16_C(797), UINT16_C(848), UINT16_C(949), UINT16_C(1000)
  },
};


const vector_128_uint_16 BLS_G2_DISCOUNT = {
  .len = 128,
  .data = {
      UINT16_C(524), UINT16_C(524), UINT16_C(525), UINT16_C(526), UINT16_C(526), UINT16_C(527), UINT16_C(528), UINT16_C(528),
      UINT16_C(529), UINT16_C(530), UINT16_C(530), UINT16_C(531), UINT16_C(532), UINT16_C(532), UINT16_C(533), UINT16_C(534),
      UINT16_C(535), UINT16_C(535), UINT16_C(536), UINT16_C(537), UINT16_C(537), UINT16_C(538), UINT16_C(539), UINT16_C(540),
      UINT16_C(541), UINT16_C(541), UINT16_C(542), UINT16_C(543), UINT16_C(544), UINT16_C(545), UINT16_C(545), UINT16_C(546),
      UINT16_C(547), UINT16_C(548), UINT16_C(549), UINT16_C(550), UINT16_C(551), UINT16_C(552), UINT16_C(552), UINT16_C(553),
      UINT16_C(554), UINT16_C(555), UINT16_C(556), UINT16_C(557), UINT16_C(558), UINT16_C(559), UINT16_C(560), UINT16_C(561),
      UINT16_C(562), UINT16_C(563), UINT16_C(565), UINT16_C(566), UINT16_C(567), UINT16_C(568), UINT16_C(569), UINT16_C(570),
      UINT16_C(571), UINT16_C(573), UINT16_C(574), UINT16_C(575), UINT16_C(576), UINT16_C(578), UINT16_C(579), UINT16_C(580),
      UINT16_C(582), UINT16_C(583), UINT16_C(584), UINT16_C(586), UINT16_C(587), UINT16_C(589), UINT16_C(590), UINT16_C(592),
      UINT16_C(593), UINT16_C(595), UINT16_C(597), UINT16_C(598), UINT16_C(600), UINT16_C(602), UINT16_C(604), UINT16_C(606),
      UINT16_C(607), UINT16_C(609), UINT16_C(611), UINT16_C(613), UINT16_C(615), UINT16_C(618), UINT16_C(620), UINT16_C(622),
      UINT16_C(624), UINT16_C(627), UINT16_C(629), UINT16_C(632), UINT16_C(634), UINT16_C(637), UINT16_C(640), UINT16_C(643),
      UINT16_C(646), UINT16_C(649), UINT16_C(652), UINT16_C(655), UINT16_C(659), UINT16_C(663), UINT16_C(666), UINT16_C(670),
      UINT16_C(674), UINT16_C(679), UINT16_C(683), UINT16_C(688), UINT16_C(693), UINT16_C(699), UINT16_C(704), UINT16_C(711),
      UINT16_C(717), UINT16_C(724), UINT16_C(732), UINT16_C(740), UINT16_C(749), UINT16_C(759), UINT16_C(770), UINT16_C(782),
      UINT16_C(796), UINT16_C(812), UINT16_C(832), UINT16_C(855), UINT16_C(884), UINT16_C(923), UINT16_C(1000), UINT16_C(1000)
  },
};


u256 protocol_word(u256 value)
{
  return as_u256(value);
}

u256 fake_exponential_word(struct BlobScheduleFields schedule, uint32_t numerator)
{
  uint32_t denominator = schedule.base_fee_update_fraction;
  u320 scaled_limit = u320_mul_widen(denominator, ((u320){{UINT64_C(0), UINT64_C(0), UINT64_C(0), UINT64_C(0), UINT64_C(1)}}));
  u320 term_index = u320_of_u64(UINT8_C(1));
  u320 output = u320_of_u64(UINT8_C(0));
  u320 numerator_accum = u320_of_u64(denominator);
  while (u320_lt(u320_of_u64(UINT8_C(0)), numerator_accum)) {
    u320 current_accum = numerator_accum;
    if (!u320_lt(current_accum, scaled_limit)) {
      fatal_error(NumericOverflow);
    } else {
      u320 next_output = u320_add_widen(output, current_accum);
      if (!u320_lt(next_output, scaled_limit)) {
        fatal_error(NumericOverflow);
      } else {
        output = next_output;
        u320 mult_atom_result_2_1792 = u320_mul_widen(denominator, term_index);
        numerator_accum = u320_div(u320_mul_widen(current_accum, numerator), mult_atom_result_2_1792);
        u320 current_index = term_index;
        if (u320_lt(current_index, scaled_limit)) {
          term_index = u320_add_widen(current_index, UINT8_C(1));
        } else {
          fatal_error(NumericOverflow);
        }
      }
    }
  }
  u256 tmp_3_3118 = u256_of_u320(u320_div_u64(output, denominator));
  return protocol_word(tmp_3_3118);
}

u256 blob_base_fee(struct ProtocolProfileFields profile, uint32_t excess_blob_gas)
{
  bool result_2_1786 = (bool)(profile.fork >= Cancun);
  if (result_2_1786 && (excess_blob_gas <= profile.excess_blob_gas_limit)) {
    return fake_exponential_word(profile.blob_schedule, excess_blob_gas);
  }
  fatal_error(InvalidConfig);
}

uint32_t block_blob_gas_add(uint8_t maximum_count, uint32_t accumulated, uint32_t transaction)
{
  bool tmp_3_3111 = (bool)((accumulated <= (UINT32_C(131072) * (uint32_t)maximum_count)) && ((int64_t)transaction <= ((int64_t)(int32_t)(UINT32_C(131072) * (uint32_t)maximum_count) - (int64_t)(int32_t)accumulated)));
  if (tmp_3_3111) {
    return (transaction + accumulated);
  }
  fatal_error(BlobGasLimitExceeded);
}

uint32_t next_excess_blob_gas(struct ProtocolProfileFields profile, uint32_t parent_excess_blob_gas, uint32_t parent_blob_gas_used, u256 parent_base_fee_per_gas)
{
  if ((parent_blob_gas_used + parent_excess_blob_gas) < (UINT32_C(131072) * (uint32_t)profile.blob_schedule.target)) {
    return UINT32_C(0);
  }
  u256 parent_blob_base_fee = blob_base_fee(profile, parent_excess_blob_gas);
  bool result_2_1770 = (bool)(profile.fork >= Osaka);
  bool tmp_3_3107 = (bool)(result_2_1770 && u320_lt(u320_mul_widen(UINT8_C(16), parent_blob_base_fee), u320_of_u256(parent_base_fee_per_gas)));
  if (tmp_3_3107) {
    if ((parent_excess_blob_gas + ((uint32_t)((int64_t)parent_blob_gas_used * ((int64_t)profile.blob_schedule.max - (int64_t)profile.blob_schedule.target)) / (uint32_t)profile.blob_schedule.max)) <= profile.excess_blob_gas_limit) {
      return (parent_excess_blob_gas + ((uint32_t)((int64_t)parent_blob_gas_used * ((int64_t)profile.blob_schedule.max - (int64_t)profile.blob_schedule.target)) / (uint32_t)profile.blob_schedule.max));
    }
    fatal_error(InvalidConfig);
  }
  if ((uint32_t)(((uint64_t)parent_blob_gas_used + (uint64_t)parent_excess_blob_gas) - ((uint64_t)UINT32_C(131072) * (uint64_t)(uint32_t)profile.blob_schedule.target)) <= profile.excess_blob_gas_limit) {
    return (uint32_t)(((uint64_t)parent_blob_gas_used + (uint64_t)parent_excess_blob_gas) - ((uint64_t)UINT32_C(131072) * (uint64_t)(uint32_t)profile.blob_schedule.target));
  }
  fatal_error(InvalidConfig);
}

uint16_t sstore_clear_refund(void)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool result_2_1764 = (bool)(execution_profile.protocol.fork >= London);
  if (result_2_1764) {
    return R_sclear_london;
  }
  return R_sclear_pre_london;
}

uint32_t state_gas_spill_room(uint32_t left)
{
  return (UINT32_C(16777216) - left);
}

uint64_t credit_state_gas_refund(uint64_t g, uint32_t amount)
{
  uint32_t spilled = state_gas_spilled;
  if (amount <= spilled) {
    if (amount != UINT8_C(0)) {
      state_gas_spilled = (spilled - amount);
      return conserved_gas_add_uint64_t_uint32_t_to_uint64_t(g, amount);
    }
    return g;
  }
  uint64_t credited;
  if (spilled != UINT8_C(0)) {
    state_gas_spilled = STATE_GAS_SPILL_ZERO;
    credited = conserved_gas_add_uint64_t_uint32_t_to_uint64_t(g, spilled);
  } else {
    credited = g;
  }
  state_gas_remaining = conserved_gas_add_uint64_t_uint32_t_to_uint64_t(state_gas_remaining, (amount - spilled));
  return credited;
}

void return_child_state_gas(uint64_t child_remaining, uint32_t child_spilled)
{
  state_gas_remaining = conserved_gas_add(state_gas_remaining, child_remaining);
  state_gas_spilled = state_gas_spill_add_uint32_t_uint32_t_to_uint32_t(state_gas_spilled, child_spilled);
}

uint64_t refund_gas(uint64_t g, uint64_t amount)
{
  return conserved_gas_add(g, amount);
}

uint64_t gas_sub(uint64_t left, uint64_t right)
{
  if (right <= left) {
    return (left - right);
  }
  return GAS_ZERO;
}

u256 memory_word_count_word(u256 byte_len)
{
  u256 quotient = word_div_word_u256_u256_to_u256(byte_len, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  u256 remainder = word_mod_word(byte_len, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  bool eq_int_result_2_1743 = eq_u256(remainder, WORD_ZERO);
  if (!eq_int_result_2_1743) {
    quotient = word_add_word(quotient, WORD_ONE);
  }
  return quotient;
}

uint64_t memory_required_size(u256 start, u256 size)
{
  if (u256_eq_u64(size, UINT8_C(0))) {
    return UINT64_C(0);
  }
  if (!u64_lt_u256(UINT32_C(4294967295), start)) {
    if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(start)), size)) {
      uint64_t tmp_3_3079;
      uint32_t bounded_size = (uint32_t)u256_to_u64(size);
      tmp_3_3079 = u256_to_u64_unchecked(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(start)), bounded_size));
      return tmp_3_3079;
    }
    return UINT64_C(4294967296);
  }
  return UINT64_C(4294967296);
}

struct MemoryAccessFields memory_access(u256 start, u256 size)
{
  if (u256_eq_u64(size, UINT8_C(0))) {
    return EMPTY_MEMORY_ACCESS;
  }
  if (!u64_lt_u256(UINT32_C(4294967295), start)) {
    if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(start)), size)) {
      uint32_t bounded_size = (uint32_t)u256_to_u64(size);
      struct MemoryRangeFields memory_range_result_2_1737 = memory_range((uint32_t)u256_to_u64_unchecked(start), bounded_size);
      uint32_t add_atom_result_2_1738 = (uint32_t)u256_to_u64(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(start)), bounded_size));
      return ((struct MemoryAccessFields){.range = memory_range_result_2_1737, .required_size = add_atom_result_2_1738});
    }
    fatal_error(ExecutionInvalid);
  }
  fatal_error(ExecutionInvalid);
}

Bytes expand_memory(Bytes mem, uint32_t required_size)
{
  struct tuple_Bytes_Bytes memory_expand_to_result_2_1728 = memory_expand_to(mem, required_size);
  return memory_expand_to_result_2_1728.tup1;
}

uint16_t account_cost(bool warm)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  if (warm) {
    return G_warm_access;
  }
  bool result_2_1727 = (bool)(execution_profile.protocol.fork >= Amsterdam);
  if (result_2_1727) {
    return G_amsterdam_cold_account_access;
  }
  return G_cold_account;
}

uint16_t external_code_read_cost(void)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool result_2_1725 = (bool)(execution_profile.protocol.fork >= Amsterdam);
  if (result_2_1725) {
    return G_warm_access;
  }
  return G_zero;
}

uint16_t sload_cost(bool warm)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  if (warm) {
    return G_warm_access;
  }
  bool result_2_1723 = (bool)(execution_profile.protocol.fork >= Amsterdam);
  if (result_2_1723) {
    return G_amsterdam_cold_storage_access;
  }
  return G_cold_sload;
}

uint16_t call_value_cost(void)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool result_2_1721 = (bool)(execution_profile.protocol.fork >= Amsterdam);
  if (result_2_1721) {
    return G_amsterdam_call_value;
  }
  return G_callvalue;
}

uint16_t create_access_cost(void)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool result_2_1719 = (bool)(execution_profile.protocol.fork >= Amsterdam);
  if (result_2_1719) {
    return G_amsterdam_create_access;
  }
  return G_create;
}

struct GasCharge code_deployment_execution_cost(uint32_t byte_len, uint64_t available)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  bool result_2_1711 = (bool)(profile.fork >= Amsterdam);
  if (result_2_1711) {
    if (byte_len <= profile.deployed_code_size_limit) {
      uint16_t words = memory_word_count_uint32_t_to_uint16_t(byte_len);
      uint64_t ediv_int_result_2_1714 = (available / (uint64_t)G_keccak_word);
      if (words <= ediv_int_result_2_1714) {
        uint32_t integer_result_3_3661 = ((uint32_t)G_keccak_word * (uint32_t)words);
        return gas_charge((uint64_t)integer_result_3_3661);
      }
      return GAS_CHARGE_UNAFFORDABLE;
    }
    return GAS_CHARGE_UNAFFORDABLE;
  }
  uint64_t ediv_int_result_2_1716 = (available / (uint64_t)G_codedeposit);
  if (byte_len <= ediv_int_result_2_1716) {
    uint64_t cost_3_3066 = ((uint64_t)G_codedeposit * (uint64_t)byte_len);
    return gas_charge(cost_3_3066);
  }
  return GAS_CHARGE_UNAFFORDABLE;
}

uint64_t code_deployment_state_cost(uint32_t byte_len)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  bool result_2_1707 = (bool)(profile.fork >= Amsterdam);
  if (result_2_1707) {
    if (byte_len <= profile.deployed_code_size_limit) {
      return ((uint64_t)G_amsterdam_state_byte * (uint64_t)byte_len);
    }
    fatal_error(ExecutionInvalid);
  }
  return GAS_COST_ZERO;
}

u256 pc_word(struct CalldataSlice input, uint32_t start, uint8_t byte_count)
{
  u256 value = ZERO_WORD;
  uint32_t start_offset = start;
  uint8_t count = byte_count;
  uint32_t input_length = calldata_slice_length(input);
  int64_t tmp_3_3061 = (int64_t)UINT8_C(31);
  int64_t tmp_3_3062 = (int64_t)UINT8_C(1);
  int64_t byte_index = (int64_t)UINT8_C(0);
  while (byte_index <= tmp_3_3061) {
    uint8_t offset = (uint8_t)byte_index;
    if (offset < count) {
      uint64_t next_byte;
      if ((start_offset < input_length) && (offset < (input_length - start_offset))) {
        next_byte = calldata_slice_byte(input, (start_offset + (uint32_t)offset));
      } else {
        next_byte = UINT64_C(0x00);
      }
      u256 shifted = word_shift_left_u256_uint8_t_to_u256(value, UINT8_C(8));
      uint8_t tmp_3_3064 = (uint8_t)next_byte;
      value = word_add_word_u256_uint8_t_to_u256(shifted, tmp_3_3064);
    }
    byte_index = (byte_index + tmp_3_3062);
  }
  return value;
}

u256 pc_word_after_declared_field(struct CalldataSlice input, uint32_t prefix, u256 declared_length, uint8_t byte_count)
{
  uint32_t input_length = calldata_slice_length(input);
  if (prefix < input_length) {
    if (u256_lt_u64(declared_length, (input_length - prefix))) {
      return pc_word(input, ((uint32_t)u256_to_u64_unchecked(declared_length) + prefix), byte_count);
    }
    return ZERO_WORD;
  }
  return ZERO_WORD;
}

uint32_t pc_blake2_rounds(struct CalldataSlice input)
{
  u256 rounds_word = pc_word_struct_CalldataSlice_uint8_t_uint8_t_to_u256(input, UINT8_C(0), UINT8_C(4));
  return (uint32_t)u256_to_u64_unchecked(u256_mod_u64(rounds_word, UINT64_C(4294967296)));
}

struct GasCharge modexp_gas(struct CalldataSlice input, uint64_t available)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  u256 bl_word = pc_word_struct_CalldataSlice_uint8_t_uint8_t_to_u256(input, UINT8_C(0), UINT8_C(32));
  u256 el_word = pc_word_struct_CalldataSlice_uint8_t_uint8_t_to_u256(input, UINT8_C(32), UINT8_C(32));
  u256 ml_word = pc_word_struct_CalldataSlice_uint8_t_uint8_t_to_u256(input, UINT8_C(64), UINT8_C(32));
  bool osaka = (bool)(execution_profile.protocol.fork >= Osaka);
  bool tmp_3_3024;
  if (osaka) {
    bool tmp_3_3023 = (bool)(u64_lt_u256(UINT16_C(1024), bl_word) || (u64_lt_u256(UINT16_C(1024), el_word) || u64_lt_u256(UINT16_C(1024), ml_word)));
    tmp_3_3024 = tmp_3_3023;
  } else {
    tmp_3_3024 = false;
  }
  if (tmp_3_3024) {
    return GAS_CHARGE_UNAFFORDABLE;
  }
  uint16_t minimum = osaka ? UINT16_C(500) : UINT16_C(200);
  if (available < minimum) {
    return GAS_CHARGE_UNAFFORDABLE;
  }
  bool base_is_zero = word_is_zero(bl_word);
  bool modulus_is_zero = word_is_zero(ml_word);
  if (!osaka && (base_is_zero && modulus_is_zero)) {
    return ((struct GasCharge){.affordable = true, .cost = UINT64_C(200)});
  }
  u256 maxlen = u256_lt(ml_word, bl_word) ? bl_word : ml_word;
  uint8_t tmp_3_3031 = (uint8_t)u256_to_u64_unchecked(u256_mod_u64(maxlen, UINT8_C(8)));
  u256 words;
  uint8_t result_2_1697 = tmp_3_3031 == UINT8_C(0) ? UINT8_C(0) : UINT8_C(1);
  words = u256_add_u64(u256_div_u64(maxlen, UINT8_C(8)), result_2_1697);
  u128 product_limit_value = osaka ? u128_of_u64(available) : u128_add_u64(u128_mul_u64_u64(UINT8_C(3), available), UINT8_C(2));
  bool words_unaffordable = u128_lt_u256(product_limit_value, words);
  u128 bounded_words = words_unaffordable ? product_limit_value : u128_of_u256_unchecked(words);
  if (words_unaffordable) {
    return GAS_CHARGE_UNAFFORDABLE;
  }
  u256 words_squared = u256_mul_u128_u128(bounded_words, bounded_words);
  bool square_unaffordable = u128_lt_u256(product_limit_value, words_squared);
  u128 affordable_square = square_unaffordable ? product_limit_value : u128_of_u256_unchecked(words_squared);
  if (square_unaffordable) {
    return GAS_CHARGE_UNAFFORDABLE;
  }
  u128 mult_value;
  if (osaka && (!u64_lt_u256(UINT8_C(32), maxlen))) {
    mult_value = u128_of_u64(UINT8_C(16));
  } else if (osaka) {
    bool doubled_unaffordable = u64_lt_u256((available >> 1), words_squared);
    uint64_t bounded_square = doubled_unaffordable ? UINT64_C(0) : (uint64_t)u256_to_u64_unchecked(words_squared);
    if (doubled_unaffordable) {
      return GAS_CHARGE_UNAFFORDABLE;
    }
    mult_value = u128_of_u64((UINT64_C(2) * bounded_square));
  } else {
    mult_value = affordable_square;
  }
  if (u128_lt(product_limit_value, mult_value)) {
    return GAS_CHARGE_UNAFFORDABLE;
  }
  u128 iterations;
  if (!u64_lt_u256(UINT8_C(32), el_word)) {
    u256 exponent_head = pc_word_after_declared_field(input, UINT32_C(96), bl_word, (uint8_t)u256_to_u64_unchecked(el_word));
    uint16_t exponent_bits = word_bit_length(exponent_head);
    if (exponent_bits == UINT8_C(0)) {
      iterations = u128_of_u64(UINT8_C(1));
    } else {
      uint8_t tmp_3_3045 = (uint8_t)((uint16_t)((uint32_t)exponent_bits - (uint32_t)UINT16_C(1)));
      iterations = tmp_3_3045 == UINT8_C(0) ? u128_of_u64(UINT8_C(1)) : u128_of_u64(tmp_3_3045);
    }
  } else {
    u256 exponent_head_3_3048 = pc_word_after_declared_field_struct_CalldataSlice_uint8_t_u256_uint8_t_to_u256(input, UINT8_C(96), bl_word, UINT8_C(32));
    uint16_t head_bits = word_bit_length(exponent_head_3_3048);
    uint8_t high_bits;
    if (head_bits != UINT8_C(0)) {
      uint8_t tmp_3_3041 = (uint8_t)((uint16_t)((uint32_t)head_bits - (uint32_t)UINT16_C(1)));
      high_bits = tmp_3_3041;
    } else {
      high_bits = UINT8_C(0);
    }
    u128 count_3_3049;
    if (osaka) {
      uint64_t extra_limit = (available >> 4);
      bool exceeds_maximum = word_greater_than_word(u256_sub_u64(el_word, UINT8_C(32)), (u256){{UINT64_C(992), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
      uint16_t osaka_extra = exceeds_maximum ? UINT16_C(992) : (uint16_t)u256_to_u64_unchecked(u256_sub_u64(el_word, UINT8_C(32)));
      bool extra_unaffordable = (bool)(osaka_extra > extra_limit);
      uint64_t bounded_extra = extra_unaffordable ? extra_limit : (uint64_t)osaka_extra;
      if (extra_unaffordable) {
        return GAS_CHARGE_UNAFFORDABLE;
      }
      count_3_3049 = u128_of_u64(((uint16_t)((uint32_t)(uint16_t)((uint64_t)UINT16_C(16) * (uint64_t)(uint16_t)bounded_extra) + (uint32_t)(uint16_t)high_bits)));
    } else {
      uint64_t tmp_3_3042 = (uint64_t)u128_to_u64_unchecked(u128_div_u64(u128_add_u64(u128_mul_u64_u64(UINT8_C(3), available), UINT8_C(2)), UINT8_C(8)));
      bool extra_unaffordable_3_3051 = u64_lt_u256(tmp_3_3042, u256_sub_u64(el_word, UINT8_C(32)));
      uint64_t bounded_extra_3_3052 = extra_unaffordable_3_3051 ? tmp_3_3042 : (uint64_t)u256_to_u64_unchecked(u256_sub_u64(el_word, UINT8_C(32)));
      if (extra_unaffordable_3_3051) {
        return GAS_CHARGE_UNAFFORDABLE;
      }
      count_3_3049 = u128_add_u64(u128_mul_u64_u64(UINT8_C(8), bounded_extra_3_3052), high_bits);
    }
    iterations = count_3_3049;
  }
  if (u128_eq_u64(iterations, UINT8_C(0))) {
    return GAS_CHARGE_UNAFFORDABLE;
  }
  u256 product = u256_mul_u128_u128(mult_value, iterations);
  bool product_unaffordable = u128_lt_u256(product_limit_value, product);
  u128 affordable_product = product_unaffordable ? product_limit_value : u128_of_u256(product);
  if (product_unaffordable) {
    return GAS_CHARGE_UNAFFORDABLE;
  }
  u128 calculated = osaka ? affordable_product : u128_div_u64(affordable_product, UINT8_C(3));
  u128 cost = u128_lt_u64(calculated, minimum) ? u128_of_u64(minimum) : calculated;
  if (!u64_lt_u128(available, cost)) {
    return gas_charge((uint64_t)u128_to_u64_unchecked(cost));
  }
  return GAS_CHARGE_UNAFFORDABLE;
}

struct GasCharge precompile_gas(enum PrecompileId num, struct CalldataSlice input, uint64_t available)
{
  uint32_t input_len = calldata_slice_length(input);
  uint32_t words = memory_word_count_uint32_t_to_uint32_t(input_len);
  switch (num) {
  case NotPrecompile:
    return GAS_CHARGE_UNAFFORDABLE;
  case Ecrecover:
    return fixed_precompile_gas_uint16_t_uint64_t_to_struct_GasCharge(UINT16_C(3000), available);
  case Sha256:
    return linear_gas_uint8_t_uint8_t_uint32_t_uint64_t_to_struct_GasCharge(UINT8_C(60), UINT8_C(12), words, available);
  case Ripemd160:
    return linear_gas_uint16_t_uint8_t_uint32_t_uint64_t_to_struct_GasCharge(UINT16_C(600), UINT8_C(120), words, available);
  case Identity:
    return linear_gas_uint8_t_uint8_t_uint32_t_uint64_t_to_struct_GasCharge(UINT8_C(15), UINT8_C(3), words, available);
  case Modexp:
    return modexp_gas(input, available);
  case Bn254Add:
    return fixed_precompile_gas_uint8_t_uint64_t_to_struct_GasCharge(UINT8_C(150), available);
  case Bn254Mul:
    return fixed_precompile_gas_uint16_t_uint64_t_to_struct_GasCharge(UINT16_C(6000), available);
  case Bn254Pairing:
    return linear_gas_uint16_t_uint16_t_uint32_t_uint64_t_to_struct_GasCharge(UINT16_C(45000), UINT16_C(34000), (input_len / (uint32_t)UINT8_C(192)), available);
  case Blake2f:
  {
    uint32_t rounds = pc_blake2_rounds(input);
    return fixed_precompile_gas_uint32_t_uint64_t_to_struct_GasCharge(rounds, available);
  }
  case KzgPointEvaluation:
    return fixed_precompile_gas_uint16_t_uint64_t_to_struct_GasCharge(UINT16_C(50000), available);
  case BlsG1Add:
    return fixed_precompile_gas_uint16_t_uint64_t_to_struct_GasCharge(UINT16_C(375), available);
  case BlsG1Msm:
    return bls_msm_gas(BLS_G1_DISCOUNT, UINT16_C(12000), UINT16_C(519), (input_len / (uint32_t)UINT8_C(160)), available);
  case BlsG2Add:
    return fixed_precompile_gas_uint16_t_uint64_t_to_struct_GasCharge(UINT16_C(600), available);
  case BlsG2Msm:
    return bls_msm_gas(BLS_G2_DISCOUNT, UINT16_C(22500), UINT16_C(524), (input_len / (uint32_t)UINT16_C(288)), available);
  case BlsPairing:
    return linear_gas_uint16_t_uint16_t_uint32_t_uint64_t_to_struct_GasCharge(UINT16_C(37700), UINT16_C(32600), (input_len / (uint32_t)UINT16_C(384)), available);
  case BlsMapFpToG1:
    return fixed_precompile_gas_uint16_t_uint64_t_to_struct_GasCharge(UINT16_C(5500), available);
  case BlsMapFp2ToG2:
    return fixed_precompile_gas_uint16_t_uint64_t_to_struct_GasCharge(UINT16_C(23800), available);
  case P256Verify:
    return fixed_precompile_gas_uint16_t_uint64_t_to_struct_GasCharge(UINT16_C(6900), available);
  }
}

uint16_t amsterdam_storage_access_cost(bool cold)
{
  if (cold) {
    return G_amsterdam_cold_storage_access;
  }
  return G_warm_access;
}

uint64_t sstore_sentry_cost(bool cold)
{
  uint16_t access_cost = amsterdam_storage_access_cost(cold);
  bool lt_int_result_2_1666 = (bool)(access_cost < G_sstore_sentry);
  if (lt_int_result_2_1666) {
    return G_sstore_sentry;
  }
  return (uint64_t)access_cost;
}

struct SstoreCosts legacy_sstore_costs(u256 original, u256 current, u256 new_value, bool cold)
{
  uint16_t cold_cost = cold ? G_cold_sload : GAS_CONSTANT_ZERO;
  uint16_t clear_refund = sstore_clear_refund();
  bool original_is_zero = word_is_zero(original);
  bool current_is_zero = word_is_zero(current);
  bool new_value_is_zero = word_is_zero(new_value);
  uint16_t base;
  if ((!eq_u256(current, new_value)) && eq_u256(original, current)) {
    base = original_is_zero ? G_sset : G_sreset;
  } else {
    base = G_warm_access;
  }
  __int128 refund;
  if (eq_u256(current, new_value)) {
    refund = (__int128)UINT8_C(0);
  } else if (eq_u256(original, current)) {
    refund = !original_is_zero && new_value_is_zero ? (__int128)clear_refund : (__int128)UINT8_C(0);
  } else {
    int32_t clear_delta;
    if (original_is_zero) {
      clear_delta = (int32_t)UINT8_C(0);
    } else {
      int32_t result_2_1663 = current_is_zero ? ((int32_t)UINT8_C(0) - (int32_t)clear_refund) : (int32_t)UINT8_C(0);
      uint16_t result_2_1664 = new_value_is_zero ? clear_refund : UINT16_C(0);
      clear_delta = (result_2_1663 + (int32_t)result_2_1664);
    }
    int32_t reset_delta;
    if (eq_u256(original, new_value)) {
      reset_delta = original_is_zero ? ((int32_t)G_sset - (int32_t)G_warm_access) : ((int32_t)G_sreset - (int32_t)G_warm_access);
    } else {
      reset_delta = (int32_t)UINT8_C(0);
    }
    refund = ((__int128)clear_delta + (__int128)reset_delta);
  }
  return ((struct SstoreCosts){.execution = ((uint64_t)(uint32_t)base + (uint64_t)(uint32_t)cold_cost), .refund = refund, .state_charge = UINT64_C(0), .state_credit = UINT32_C(0)});
}

struct SstoreCosts amsterdam_sstore_costs(u256 original, u256 current, u256 new_value, bool cold)
{
  bool changed = (bool)(!eq_u256(current, new_value));
  bool clean_change = (bool)(eq_u256(original, current) && changed);
  uint16_t access = amsterdam_storage_access_cost(cold);
  bool original_is_zero = word_is_zero(original);
  bool current_is_zero = word_is_zero(current);
  bool new_value_is_zero = word_is_zero(new_value);
  uint64_t execution;
  if (clean_change) {
    uint32_t integer_result_3_3670 = ((uint32_t)access + (uint32_t)G_amsterdam_storage_write);
    execution = (uint64_t)integer_result_3_3670;
  } else {
    execution = (uint64_t)access;
  }
  int32_t clear_delta;
  if (changed && (!original_is_zero && (!current_is_zero && new_value_is_zero))) {
    clear_delta = (int32_t)R_amsterdam_storage_clear;
  } else if (changed && (!original_is_zero && current_is_zero)) {
    clear_delta = ((int32_t)UINT8_C(0) - (int32_t)R_amsterdam_storage_clear);
  } else {
    clear_delta = (int32_t)UINT8_C(0);
  }
  uint16_t restore_delta = changed && eq_u256(original, new_value) ? G_amsterdam_storage_write : UINT16_C(0);
  uint64_t state_charge = clean_change && original_is_zero ? (uint64_t)G_amsterdam_state_storage_set : GAS_COST_ZERO;
  uint32_t state_credit = UINT32_C(0);
  bool tmp_3_2993 = (bool)(changed && (eq_u256(original, new_value) && original_is_zero));
  if (tmp_3_2993) {
    state_credit = G_amsterdam_state_storage_set;
  }
  return ((struct SstoreCosts){.execution = execution, .refund = ((__int128)clear_delta + (__int128)(int32_t)restore_delta), .state_charge = state_charge, .state_credit = state_credit});
}

struct SstoreCosts sstore_costs(u256 original, u256 current, u256 new_value, bool cold)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool result_2_1650 = (bool)(execution_profile.protocol.fork >= Amsterdam);
  if (result_2_1650) {
    return amsterdam_sstore_costs(original, current, new_value, cold);
  }
  return legacy_sstore_costs(original, current, new_value, cold);
}

struct tuple_bool_uint_64 charge_word_scaled_gas(uint64_t g, uint16_t per_unit, u256 units)
{
  if ((per_unit == UINT8_C(0)) || u256_eq_u64(units, UINT8_C(0))) {
    return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = g});
  }
  if (!u64_lt_u256(g, units)) {
    if (!u64_lt_u128(g, u128_mul_u64_u64(per_unit, (uint64_t)u256_to_u64_unchecked(units)))) {
      return charge_uint64_t_uint64_t_to_struct_tuple_bool_uint_64(g, (uint64_t)u128_to_u64_unchecked(u128_mul_u64_u64(per_unit, (uint64_t)u256_to_u64_unchecked(units))));
    }
    uint64_t exc_halt_result_2_1647 = exc_halt(g, OutOfGas);
    return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = exc_halt_result_2_1647});
  }
  uint64_t exc_halt_result_2_1648 = exc_halt(g, OutOfGas);
  return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = exc_halt_result_2_1648});
}

struct tuple_bool_uint_64 charge_memory_word_gas(uint64_t g, uint16_t base, uint16_t per_word, u256 size)
{
  struct tuple_bool_uint_64 charge_result_2_1642 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, base);
  if (charge_result_2_1642.tup0) {
    u256 words = memory_word_count_word(size);
    return charge_word_scaled_gas(charge_result_2_1642.tup1, per_word, words);
  }
  return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = charge_result_2_1642.tup1});
}

struct tuple_bool_uint_64 charge_keccak_gas(uint64_t g, u256 size)
{
  return charge_memory_word_gas_uint64_t_uint16_t_uint8_t_u256_to_struct_tuple_bool_uint_64(g, G_keccak, G_keccak_word, size);
}

struct tuple_bool_uint_64 charge_copy_gas(uint64_t g, u256 size)
{
  return charge_memory_word_gas(g, GAS_CONSTANT_ZERO, G_copy_word, size);
}

uint64_t exp_gas(u256 exponent)
{
  uint32_t result_2_1639;
  uint8_t word_byte_length_result_2_1638 = word_byte_length(exponent);
  result_2_1639 = ((uint32_t)G_expbyte * (uint32_t)word_byte_length_result_2_1638);
  uint32_t integer_result_3_3673 = (result_2_1639 + (uint32_t)G_exp);
  return (uint64_t)integer_result_3_3673;
}

uint32_t transaction_initcode_gas(uint32_t byte_len)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool result_2_1637 = (bool)(execution_profile.protocol.fork >= Shanghai);
  if (result_2_1637) {
    uint32_t words = memory_word_count_uint32_t_to_uint32_t(byte_len);
    return (words + words);
  }
  return UINT32_C(0);
}

uint64_t call_gas_cap_word(uint64_t available, u256 requested)
{
  uint64_t all_but_64th = gas_sub(available, (available >> 6));
  if (u256_lt_u64(requested, all_but_64th)) {
    return (uint64_t)u256_to_u64_unchecked(requested);
  }
  return all_but_64th;
}

uint32_t block_blob_gas_add_uint8_t_uint32_t_uint32_t_to_uint32_t(uint8_t maximum_count, uint32_t accumulated, uint32_t transaction)
{
  bool tmp_3_3111 = (bool)((accumulated <= (UINT32_C(131072) * (uint32_t)maximum_count)) && ((int32_t)((UINT32_C(131072) * (uint32_t)maximum_count) - accumulated) >= INT32_C(0) && transaction <= ((uint32_t)(int32_t)((UINT32_C(131072) * (uint32_t)maximum_count) - accumulated))));
  if (tmp_3_3111) {
    return (transaction + accumulated);
  }
  fatal_error(BlobGasLimitExceeded);
}

struct GasCharge bls_msm_gas(vector_128_uint_16 table, uint16_t base, uint16_t maxd, uint32_t k, uint64_t available)
{
  if (k == UINT8_C(0)) {
    return gas_charge(GAS_COST_ZERO);
  }
  uint16_t discount = maxd;
  if (k < UINT8_C(128)) {
    discount = table.data[(size_t)((uint8_t)((uint32_t)UINT8_C(128) - (uint32_t)(uint8_t)k))];
  }
  uint16_t tmp_3_3020 = (uint16_t)(k % UINT32_C(1000));
  if (u64_lt_u128(available, u128_of_u64((((uint64_t)(uint32_t)base * (uint64_t)(uint32_t)discount) * (uint64_t)(uint32_t)((uint16_t)(k / (uint32_t)UINT16_C(1000))))))) {
    return GAS_CHARGE_UNAFFORDABLE;
  }
  uint32_t tmp_3_3021 = (uint32_t)((((uint64_t)(uint32_t)base * (uint64_t)(uint32_t)discount) * (uint64_t)tmp_3_3020) / UINT64_C(1000));
  if (!u64_lt_u128(available, u128_of_u64(((uint64_t)u128_to_u64_unchecked(u128_of_u64((((uint64_t)(uint32_t)base * (uint64_t)(uint32_t)discount) * (uint64_t)(uint32_t)((uint16_t)(k / (uint32_t)UINT16_C(1000)))))) + (uint64_t)tmp_3_3021)))) {
    return gas_charge((uint64_t)u128_to_u64_unchecked(u128_of_u64(((uint64_t)u128_to_u64_unchecked(u128_of_u64((((uint64_t)(uint32_t)base * (uint64_t)(uint32_t)discount) * (uint64_t)(uint32_t)((uint16_t)(k / (uint32_t)UINT16_C(1000)))))) + (uint64_t)tmp_3_3021))));
  }
  return GAS_CHARGE_UNAFFORDABLE;
}

struct tuple_bool_uint_64 charge_uint64_t_u128_to_struct_tuple_bool_uint_64(uint64_t g, u128 amount)
{
  if (!u64_lt_u128(g, amount)) {
    return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = (uint64_t)((__int128)g - (__int128)u128_to_u64_unchecked(amount))});
  }
  uint64_t exc_halt_result_2_1762 = exc_halt(g, OutOfGas);
  return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = exc_halt_result_2_1762});
}

struct tuple_bool_uint_64 charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(uint64_t g, uint16_t amount)
{
  if (amount <= g) {
    return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = (g - (uint64_t)amount)});
  }
  uint64_t exc_halt_result_2_1762 = exc_halt_uint64_t_enum_ExceptionKind_to_uint64_t(g, OutOfGas);
  return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = exc_halt_result_2_1762});
}

struct tuple_bool_uint_64 charge_uint64_t_uint32_t_to_struct_tuple_bool_uint_64(uint64_t g, uint32_t amount)
{
  if (amount <= g) {
    return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = (g - (uint64_t)amount)});
  }
  uint64_t exc_halt_result_2_1762 = exc_halt_uint64_t_enum_ExceptionKind_to_uint64_t(g, OutOfGas);
  return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = exc_halt_result_2_1762});
}

struct tuple_bool_uint_64 charge_uint64_t_uint64_t_to_struct_tuple_bool_uint_64(uint64_t g, uint64_t amount)
{
  if (amount <= g) {
    return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = (g - amount)});
  }
  uint64_t exc_halt_result_2_1762 = exc_halt_uint64_t_enum_ExceptionKind_to_uint64_t(g, OutOfGas);
  return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = exc_halt_result_2_1762});
}

struct tuple_bool_uint_64 charge_uint64_t_uint8_t_to_struct_tuple_bool_uint_64(uint64_t g, uint8_t amount)
{
  if (amount <= g) {
    return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = (g - (uint64_t)amount)});
  }
  uint64_t exc_halt_result_2_1762 = exc_halt_uint64_t_enum_ExceptionKind_to_uint64_t(g, OutOfGas);
  return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = exc_halt_result_2_1762});
}

struct tuple_bool_uint_64 charge_deployment_state_gas(uint64_t g, uint64_t amount)
{
  struct tuple_bool_uint_64 debit_state_gas_result_2_1749 = debit_state_gas_uint64_t_uint64_t_to_struct_tuple_bool_uint_64(g, amount);
  if (debit_state_gas_result_2_1749.tup0) {
    return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = debit_state_gas_result_2_1749.tup1});
  }
  uint64_t exc_halt_result_2_1750 = exc_halt(debit_state_gas_result_2_1749.tup1, OutOfGas);
  return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = exc_halt_result_2_1750});
}

struct tuple_bool_uint_64 charge_log_gas(uint64_t g, uint8_t num_topics, u256 size)
{
  struct tuple_bool_uint_64 charge_result_2_1640 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_log);
  if (charge_result_2_1640.tup0) {
    uint32_t integer_result_3_3743 = ((uint32_t)G_logtopic * (uint32_t)num_topics);
    struct tuple_bool_uint_64 charge_result_2_1641 = charge_uint64_t_uint64_t_to_struct_tuple_bool_uint_64(charge_result_2_1640.tup1, (uint64_t)integer_result_3_3743);
    if (charge_result_2_1641.tup0) {
      return charge_word_scaled_gas(charge_result_2_1641.tup1, G_logdata, size);
    }
    return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = charge_result_2_1641.tup1});
  }
  return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = charge_result_2_1640.tup1});
}

struct tuple_bool_uint_64 charge_memory_word_gas_uint64_t_uint16_t_uint8_t_u256_to_struct_tuple_bool_uint_64(uint64_t g, uint16_t base, uint8_t per_word, u256 size)
{
  struct tuple_bool_uint_64 charge_result_2_1642 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, base);
  if (charge_result_2_1642.tup0) {
    u256 words = memory_word_count_word(size);
    return charge_word_scaled_gas_uint64_t_uint8_t_u256_to_struct_tuple_bool_uint_64(charge_result_2_1642.tup1, per_word, words);
  }
  return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = charge_result_2_1642.tup1});
}

struct tuple_bool_uint_64 charge_state_gas(uint64_t g, uint32_t amount)
{
  struct tuple_bool_uint_64 debit_state_gas_result_2_1751 = debit_state_gas_uint64_t_uint32_t_to_struct_tuple_bool_uint_64(g, amount);
  if (debit_state_gas_result_2_1751.tup0) {
    return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = debit_state_gas_result_2_1751.tup1});
  }
  uint64_t exc_halt_result_2_1752 = exc_halt(debit_state_gas_result_2_1751.tup1, OutOfGas);
  return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = exc_halt_result_2_1752});
}

struct tuple_bool_uint_64 charge_word_scaled_gas_uint64_t_uint8_t_u256_to_struct_tuple_bool_uint_64(uint64_t g, uint8_t per_unit, u256 units)
{
  if (u256_eq_u64(units, UINT8_C(0))) {
    return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = g});
  }
  if (!u64_lt_u256(g, units)) {
    if (!u64_lt_u128(g, u128_mul_u64_u64(per_unit, (uint64_t)u256_to_u64_unchecked(units)))) {
      return charge_uint64_t_uint64_t_to_struct_tuple_bool_uint_64(g, (uint64_t)u128_to_u64_unchecked(u128_mul_u64_u64(per_unit, (uint64_t)u256_to_u64_unchecked(units))));
    }
    uint64_t exc_halt_result_2_1647 = exc_halt_uint64_t_enum_ExceptionKind_to_uint64_t(g, OutOfGas);
    return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = exc_halt_result_2_1647});
  }
  uint64_t exc_halt_result_2_1648 = exc_halt(g, OutOfGas);
  return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = exc_halt_result_2_1648});
}

struct tuple_bool_uint_64 check_execution_gas(uint64_t g, uint32_t amount)
{
  if (g < amount) {
    uint64_t exc_halt_result_2_1759 = exc_halt_uint64_t_enum_ExceptionKind_to_uint64_t(g, OutOfGas);
    return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = exc_halt_result_2_1759});
  }
  return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = g});
}

struct tuple_bool_uint_64 debit_state_gas_uint64_t_uint32_t_to_struct_tuple_bool_uint_64(uint64_t g, uint32_t amount)
{
  if (amount == UINT8_C(0)) {
    return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = g});
  }
  uint64_t state_left = state_gas_remaining;
  if (amount <= state_left) {
    state_gas_remaining = (state_left - (uint64_t)amount);
    return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = g});
  }
  if (!((__int128)g < (__int128)((int64_t)amount - (int64_t)(uint32_t)state_left))) {
    uint32_t spilled = state_gas_spilled;
    state_gas_remaining = GAS_ZERO;
    state_gas_spilled = state_gas_spill_add_uint32_t_int64_t_to_uint32_t(spilled, ((int64_t)amount - (int64_t)(uint32_t)state_left));
    return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = (uint64_t)((__int128)g - (__int128)((int64_t)amount - (int64_t)(uint32_t)state_left))});
  }
  return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = g});
}

struct tuple_bool_uint_64 debit_state_gas_uint64_t_uint64_t_to_struct_tuple_bool_uint_64(uint64_t g, uint64_t amount)
{
  if (amount == UINT8_C(0)) {
    return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = g});
  }
  uint64_t state_left = state_gas_remaining;
  if (amount <= state_left) {
    state_gas_remaining = (state_left - amount);
    return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = g});
  }
  if (!(g < ((__int128)amount - (__int128)state_left))) {
    uint32_t spilled = state_gas_spilled;
    state_gas_remaining = GAS_ZERO;
    state_gas_spilled = state_gas_spill_add_uint32_t___int128_to_uint32_t(spilled, ((__int128)amount - (__int128)state_left));
    return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = (uint64_t)((__int128)g - ((__int128)amount - (__int128)state_left))});
  }
  return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = g});
}

bool deployed_code_size_allowed(uint32_t size)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  return (bool)(size <= execution_profile.protocol.deployed_code_size_limit);
}

struct GasCharge fixed_precompile_gas_uint16_t_uint64_t_to_struct_GasCharge(uint16_t cost, uint64_t available)
{
  if (cost <= available) {
    return gas_charge((uint64_t)cost);
  }
  return GAS_CHARGE_UNAFFORDABLE;
}

struct GasCharge fixed_precompile_gas_uint32_t_uint64_t_to_struct_GasCharge(uint32_t cost, uint64_t available)
{
  if (cost <= available) {
    return gas_charge((uint64_t)cost);
  }
  return GAS_CHARGE_UNAFFORDABLE;
}

struct GasCharge fixed_precompile_gas_uint8_t_uint64_t_to_struct_GasCharge(uint8_t cost, uint64_t available)
{
  if (cost <= available) {
    return gas_charge((uint64_t)cost);
  }
  return GAS_CHARGE_UNAFFORDABLE;
}

bool initcode_size_allowed(uint32_t size)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  return (bool)((execution_profile.protocol.initcode_size_limit == UINT8_C(0)) || (size <= execution_profile.protocol.initcode_size_limit));
}

struct GasCharge linear_gas_uint16_t_uint16_t_uint32_t_uint64_t_to_struct_GasCharge(uint16_t base, uint16_t per_unit, uint32_t units, uint64_t available)
{
  if (u64_lt_u128(available, u128_of_u64(((uint64_t)u128_to_u64_unchecked(u128_of_u64(((uint64_t)per_unit * (uint64_t)units))) + (uint64_t)base)))) {
    return GAS_CHARGE_UNAFFORDABLE;
  }
  return gas_charge((uint64_t)u128_to_u64_unchecked(u128_of_u64(((uint64_t)u128_to_u64_unchecked(u128_of_u64(((uint64_t)per_unit * (uint64_t)units))) + (uint64_t)base))));
}

struct GasCharge linear_gas_uint16_t_uint8_t_uint32_t_uint64_t_to_struct_GasCharge(uint16_t base, uint8_t per_unit, uint32_t units, uint64_t available)
{
  if (u64_lt_u128(available, u128_of_u64(((uint64_t)u128_to_u64_unchecked(u128_of_u64(((uint64_t)per_unit * (uint64_t)units))) + (uint64_t)base)))) {
    return GAS_CHARGE_UNAFFORDABLE;
  }
  return gas_charge((uint64_t)u128_to_u64_unchecked(u128_of_u64(((uint64_t)u128_to_u64_unchecked(u128_of_u64(((uint64_t)per_unit * (uint64_t)units))) + (uint64_t)base))));
}

struct GasCharge linear_gas_uint8_t_uint8_t_uint32_t_uint64_t_to_struct_GasCharge(uint8_t base, uint8_t per_unit, uint32_t units, uint64_t available)
{
  if (u64_lt_u128(available, u128_of_u64(((uint64_t)u128_to_u64_unchecked(u128_of_u64(((uint64_t)per_unit * (uint64_t)units))) + (uint64_t)base)))) {
    return GAS_CHARGE_UNAFFORDABLE;
  }
  return gas_charge((uint64_t)u128_to_u64_unchecked(u128_of_u64(((uint64_t)u128_to_u64_unchecked(u128_of_u64(((uint64_t)per_unit * (uint64_t)units))) + (uint64_t)base))));
}

uint64_t mem_cost(uint32_t words)
{
  uint64_t linear = ((uint64_t)G_memory * (uint64_t)words);
  return ((((uint64_t)words * (uint64_t)words) >> 9) + linear);
}

struct MemoryAccessFields memory_access_u256_u256_to_struct_MemoryAccessFields(u256 start, u256 size)
{
  if (!u64_lt_u256(UINT32_C(4294967295), start)) {
    if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(start)), size)) {
      uint32_t bounded_size = (uint32_t)u256_to_u64_unchecked(size);
      struct MemoryRangeFields memory_range_result_2_1737 = memory_range((uint32_t)u256_to_u64_unchecked(start), bounded_size);
      return ((struct MemoryAccessFields){.range = memory_range_result_2_1737, .required_size = (bounded_size + (uint32_t)u256_to_u64_unchecked(start))});
    }
    fatal_error(ExecutionInvalid);
  }
  fatal_error(ExecutionInvalid);
}

u128 memory_expansion_cost(Bytes mem, uint64_t required_size)
{
  if (required_size <= UINT32_C(4294967295)) {
    uint32_t new_words = memory_word_count_uint32_t_to_uint32_t((uint32_t)required_size);
    uint32_t old_words;
    uint32_t memory_high_water_result_2_1732 = memory_high_water(mem);
    old_words = memory_word_count_uint32_t_to_uint32_t(memory_high_water_result_2_1732);
    if (new_words <= old_words) {
      return u128_of_u64(UINT8_C(0));
    }
    uint64_t old_cost = mem_cost(old_words);
    uint64_t new_cost = mem_cost(new_words);
    return u128_of_u64((new_cost - old_cost));
  }
  return (u128){{UINT64_C(0), UINT64_C(1)}};
}

uint64_t memory_required_size_u256_u256_to_uint64_t(u256 start, u256 size)
{
  if (!u64_lt_u256(UINT32_C(4294967295), start)) {
    if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(start)), size)) {
      return ((uint64_t)(uint32_t)u256_to_u64_unchecked(start) + (uint64_t)(uint32_t)u256_to_u64_unchecked(size));
    }
    return UINT64_C(4294967296);
  }
  return UINT64_C(4294967296);
}

uint16_t memory_word_count_uint32_t_to_uint16_t(uint32_t byte_len)
{
  uint16_t result_8_795;
  if ((uint8_t)(byte_len & UINT32_C(31)) == UINT8_C(0)) {
    result_8_795 = (uint16_t)(byte_len >> 5);
  } else {
    result_8_795 = (uint16_t)((byte_len >> 5) + UINT32_C(1));
  }
  return result_8_795;
}

uint32_t memory_word_count_uint32_t_to_uint32_t(uint32_t byte_len)
{
  if ((uint8_t)(byte_len & UINT32_C(31)) == UINT8_C(0)) {
    return (byte_len >> 5);
  }
  return ((byte_len >> 5) + UINT32_C(1));
}

u256 pc_word_struct_CalldataSlice_uint8_t_uint8_t_to_u256(struct CalldataSlice input, uint8_t start, uint8_t byte_count)
{
  u256 value = ZERO_WORD;
  uint32_t start_offset = (uint32_t)start;
  uint8_t count = byte_count;
  uint32_t input_length = calldata_slice_length(input);
  int64_t tmp_3_3061 = (int64_t)UINT8_C(31);
  int64_t tmp_3_3062 = (int64_t)UINT8_C(1);
  int64_t byte_index = (int64_t)UINT8_C(0);
  while (byte_index <= tmp_3_3061) {
    uint8_t offset = (uint8_t)byte_index;
    if (offset < count) {
      uint64_t next_byte;
      if ((start_offset < input_length) && (offset < (input_length - start_offset))) {
        next_byte = calldata_slice_byte(input, ((uint32_t)offset + (uint32_t)(uint8_t)start_offset));
      } else {
        next_byte = UINT64_C(0x00);
      }
      u256 shifted = word_shift_left_u256_uint8_t_to_u256(value, UINT8_C(8));
      uint8_t tmp_3_3064 = (uint8_t)next_byte;
      value = word_add_word_u256_uint8_t_to_u256(shifted, tmp_3_3064);
    }
    byte_index = (byte_index + tmp_3_3062);
  }
  return value;
}

u256 pc_word_after_declared_field_struct_CalldataSlice_uint8_t_u256_uint8_t_to_u256(struct CalldataSlice input, uint8_t prefix, u256 declared_length, uint8_t byte_count)
{
  uint32_t input_length = calldata_slice_length(input);
  if ((uint32_t)prefix < input_length) {
    if (u256_lt_u64(declared_length, (input_length - (uint32_t)prefix))) {
      return pc_word(input, ((uint32_t)u256_to_u64_unchecked(declared_length) + (uint32_t)prefix), byte_count);
    }
    return ZERO_WORD;
  }
  return ZERO_WORD;
}

uint32_t state_gas_spill_add_uint32_t___int128_to_uint32_t(uint32_t left, __int128 right)
{
  uint32_t result_8_934;
  uint32_t room = state_gas_spill_room(left);
  if (!(room < right)) {
    result_8_934 = (uint32_t)(right + (__int128)left);
  } else {
    fatal_error(ExecutionInvalid);
  }
  return result_8_934;
}

uint32_t state_gas_spill_add_uint32_t_int64_t_to_uint32_t(uint32_t left, int64_t right)
{
  uint32_t result_8_935;
  uint32_t room = state_gas_spill_room(left);
  if (right <= room) {
    result_8_935 = (uint32_t)(right + (int64_t)left);
  } else {
    fatal_error(ExecutionInvalid);
  }
  return result_8_935;
}

uint32_t state_gas_spill_add_uint32_t_uint32_t_to_uint32_t(uint32_t left, uint32_t right)
{
  uint32_t result_8_936;
  uint32_t room = state_gas_spill_room(left);
  if ((int64_t)right <= (int64_t)room) {
    result_8_936 = (uint32_t)((uint64_t)left + (uint64_t)right);
  } else {
    fatal_error(ExecutionInvalid);
  }
  return result_8_936;
}

