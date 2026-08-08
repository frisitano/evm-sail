#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

struct ProtocolProfileFields DEFAULT_PROTOCOL_PROFILE;


struct ExecutionProfileFields DEFAULT_EXECUTION_PROFILE;


struct ProtocolProfileFields pack_protocol_profile(struct ProtocolProfileFields profile)
{
  return profile;
}

struct GasLimitsFields gas_limits_for(struct ProtocolProfileFields profile, uint64_t block_limit)
{
  uint64_t transaction_total_limit = block_limit < profile.transaction_total_gas_limit ? block_limit : profile.transaction_total_gas_limit;
  uint64_t transaction_regular_limit = transaction_total_limit < profile.transaction_regular_gas_limit ? transaction_total_limit : profile.transaction_regular_gas_limit;
  return ((struct GasLimitsFields){.block_limit = block_limit, .system_regular_limit = SYSTEM_CALL_GAS_LIMIT, .system_state_limit = UINT8_C(0), .transaction_regular_limit = transaction_regular_limit, .transaction_total_limit = transaction_total_limit});
}

struct ExecutionProfileFields execution_profile_for(struct ProtocolProfileFields protocol, uint64_t block_limit)
{
  struct GasLimitsFields gas_limits_for_result_2_2267 = gas_limits_for(protocol, block_limit);
  return ((struct ExecutionProfileFields){.gas = gas_limits_for_result_2_2267, .protocol = protocol});
}

struct ProtocolProfileFields schema_protocol_profile(uint64_t schema_fork)
{
  if (schema_fork == UINT64_C(0x0A)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2255 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(Berlin, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3502;
    tmp_3_3502.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3502.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3502.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2255;
    tmp_3_3502.fork = Berlin;
    tmp_3_3502.initcode_size_limit = UINT32_C(0);
    tmp_3_3502.refund_divisor = UINT8_C(2);
    tmp_3_3502.transaction_blob_limit = UINT8_C(0);
    tmp_3_3502.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3502.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3502);
  }
  if (schema_fork == UINT64_C(0x0B)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2256 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(London, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3499;
    tmp_3_3499.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3499.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3499.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2256;
    tmp_3_3499.fork = London;
    tmp_3_3499.initcode_size_limit = UINT32_C(0);
    tmp_3_3499.refund_divisor = UINT8_C(5);
    tmp_3_3499.transaction_blob_limit = UINT8_C(0);
    tmp_3_3499.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3499.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3499);
  }
  if (schema_fork == UINT64_C(0x0C)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2257 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(ArrowGlacier, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3496;
    tmp_3_3496.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3496.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3496.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2257;
    tmp_3_3496.fork = ArrowGlacier;
    tmp_3_3496.initcode_size_limit = UINT32_C(0);
    tmp_3_3496.refund_divisor = UINT8_C(5);
    tmp_3_3496.transaction_blob_limit = UINT8_C(0);
    tmp_3_3496.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3496.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3496);
  }
  if (schema_fork == UINT64_C(0x0D)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2258 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(GrayGlacier, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3493;
    tmp_3_3493.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3493.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3493.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2258;
    tmp_3_3493.fork = GrayGlacier;
    tmp_3_3493.initcode_size_limit = UINT32_C(0);
    tmp_3_3493.refund_divisor = UINT8_C(5);
    tmp_3_3493.transaction_blob_limit = UINT8_C(0);
    tmp_3_3493.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3493.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3493);
  }
  if (schema_fork == UINT64_C(0x0E)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2259 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(Paris, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3490;
    tmp_3_3490.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3490.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3490.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2259;
    tmp_3_3490.fork = Paris;
    tmp_3_3490.initcode_size_limit = UINT32_C(0);
    tmp_3_3490.refund_divisor = UINT8_C(5);
    tmp_3_3490.transaction_blob_limit = UINT8_C(0);
    tmp_3_3490.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3490.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3490);
  }
  if (schema_fork == UINT64_C(0x0F)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2260 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(Shanghai, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3487;
    tmp_3_3487.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3487.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3487.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2260;
    tmp_3_3487.fork = Shanghai;
    tmp_3_3487.initcode_size_limit = UINT32_C(49152);
    tmp_3_3487.refund_divisor = UINT8_C(5);
    tmp_3_3487.transaction_blob_limit = UINT8_C(0);
    tmp_3_3487.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3487.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3487);
  }
  if (schema_fork == UINT64_C(0x10)) {
    uint32_t compute_profile_excess_blob_gas_limit_result_2_2261 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(Cancun, UINT8_C(3), UINT8_C(6), UINT32_C(3338477));
    struct ProtocolProfileFields tmp_3_3484 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(3338477), .max = UINT8_C(6), .target = UINT8_C(3)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2261, .fork = Cancun, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(18446744073709551615), .transaction_total_gas_limit = UINT64_C(18446744073709551615)});
    return pack_protocol_profile(tmp_3_3484);
  }
  if (schema_fork == UINT64_C(0x11)) {
    uint32_t compute_profile_excess_blob_gas_limit_result_2_2262 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(Prague, UINT8_C(6), UINT8_C(9), UINT32_C(5007716));
    struct ProtocolProfileFields tmp_3_3481 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(5007716), .max = UINT8_C(9), .target = UINT8_C(6)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2262, .fork = Prague, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(9), .transaction_regular_gas_limit = UINT64_C(18446744073709551615), .transaction_total_gas_limit = UINT64_C(18446744073709551615)});
    return pack_protocol_profile(tmp_3_3481);
  }
  if (schema_fork == UINT64_C(0x12)) {
    uint32_t compute_profile_excess_blob_gas_limit_result_2_2263 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(Osaka, UINT8_C(6), UINT8_C(9), UINT32_C(5007716));
    struct ProtocolProfileFields tmp_3_3478 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(5007716), .max = UINT8_C(9), .target = UINT8_C(6)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2263, .fork = Osaka, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(16777216), .transaction_total_gas_limit = UINT64_C(16777216)});
    return pack_protocol_profile(tmp_3_3478);
  }
  if (schema_fork == UINT64_C(0x13)) {
    uint32_t compute_profile_excess_blob_gas_limit_result_2_2264 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(BPO1, UINT8_C(10), UINT8_C(15), UINT32_C(8346193));
    struct ProtocolProfileFields tmp_3_3475 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(8346193), .max = UINT8_C(15), .target = UINT8_C(10)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2264, .fork = BPO1, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(16777216), .transaction_total_gas_limit = UINT64_C(16777216)});
    return pack_protocol_profile(tmp_3_3475);
  }
  if (schema_fork == UINT64_C(0x14)) {
    uint32_t compute_profile_excess_blob_gas_limit_result_2_2265 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(BPO2, UINT8_C(14), UINT8_C(21), UINT32_C(11684671));
    struct ProtocolProfileFields tmp_3_3472 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(11684671), .max = UINT8_C(21), .target = UINT8_C(14)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2265, .fork = BPO2, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(16777216), .transaction_total_gas_limit = UINT64_C(16777216)});
    return pack_protocol_profile(tmp_3_3472);
  }
  uint32_t compute_profile_excess_blob_gas_limit_result_2_2266 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(Amsterdam, UINT8_C(14), UINT8_C(21), UINT32_C(11684671));
  struct ProtocolProfileFields tmp_3_3469 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(11684671), .max = UINT8_C(21), .target = UINT8_C(14)}), .deployed_code_size_limit = UINT32_C(65536), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2266, .fork = Amsterdam, .initcode_size_limit = UINT32_C(131072), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(16777216), .transaction_total_gas_limit = UINT64_C(18446744073709551615)});
  return pack_protocol_profile(tmp_3_3469);
}

void create_letbind_56(void) {

  struct ProtocolProfileFields let_value_3_69 = schema_protocol_profile(UINT64_C(0x15));
  DEFAULT_PROTOCOL_PROFILE = let_value_3_69;
}
void kill_letbind_56(void) {
}

void create_letbind_57(void) {

  struct ExecutionProfileFields let_value_3_70 = execution_profile_for_struct_ProtocolProfileFields_uint8_t_to_struct_ExecutionProfileFields(DEFAULT_PROTOCOL_PROFILE, UINT8_C(0));
  DEFAULT_EXECUTION_PROFILE = let_value_3_70;
}
void kill_letbind_57(void) {
}

uint32_t compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(uint8_t fork, uint8_t target, uint8_t maximum, uint32_t denominator)
{
  bool lt_int_result_2_2272 = (bool)(fork < Cancun);
  if (lt_int_result_2_2272) {
    return UINT32_C(0);
  }
  return ((UINT32_C(131072) * ((uint32_t)maximum - (uint32_t)target)) + (denominator * UINT32_C(256)));
}

uint8_t compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(uint8_t fork, uint8_t target, uint8_t maximum, uint8_t denominator)
{
  uint8_t result_8_717;
  bool lt_int_result_2_2272 = (bool)(fork < Cancun);
  if (lt_int_result_2_2272) {
    result_8_717 = UINT8_C(0);
  } else {
    result_8_717 = (uint8_t)((uint16_t)((uint32_t)(uint16_t)((uint32_t)UINT16_C(256) * (uint32_t)(uint16_t)denominator) + (uint32_t)(uint16_t)(UINT32_C(131072) * ((uint32_t)maximum - (uint32_t)target))));
  }
  return result_8_717;
}

struct ExecutionProfileFields execution_profile_for_struct_ProtocolProfileFields_uint8_t_to_struct_ExecutionProfileFields(struct ProtocolProfileFields protocol, uint8_t block_limit)
{
  struct GasLimitsFields gas_limits_for_result_2_2267 = gas_limits_for_struct_ProtocolProfileFields_uint8_t_to_struct_GasLimitsFields(protocol, block_limit);
  return ((struct ExecutionProfileFields){.gas = gas_limits_for_result_2_2267, .protocol = protocol});
}

struct GasLimitsFields gas_limits_for_struct_ProtocolProfileFields_uint8_t_to_struct_GasLimitsFields(struct ProtocolProfileFields profile, uint8_t block_limit)
{
  uint64_t transaction_total_limit = block_limit < profile.transaction_total_gas_limit ? (uint64_t)block_limit : profile.transaction_total_gas_limit;
  uint64_t transaction_regular_limit = transaction_total_limit < profile.transaction_regular_gas_limit ? transaction_total_limit : profile.transaction_regular_gas_limit;
  return ((struct GasLimitsFields){.block_limit = (uint64_t)block_limit, .system_regular_limit = SYSTEM_CALL_GAS_LIMIT, .system_state_limit = UINT8_C(0), .transaction_regular_limit = transaction_regular_limit, .transaction_total_limit = transaction_total_limit});
}

