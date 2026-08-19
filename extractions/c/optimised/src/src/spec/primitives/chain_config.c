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
  struct GasLimitsFields gas_limits_for_result_2_2464 = gas_limits_for(protocol, block_limit);
  return ((struct ExecutionProfileFields){.gas = gas_limits_for_result_2_2464, .protocol = protocol});
}

struct ProtocolProfileFields schema_protocol_profile(uint64_t schema_fork)
{
  if (schema_fork == UINT64_C(0x0A)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2452 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(Berlin, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3429;
    tmp_3_3429.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3429.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3429.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2452;
    tmp_3_3429.fork = Berlin;
    tmp_3_3429.initcode_size_limit = UINT32_C(0);
    tmp_3_3429.refund_divisor = UINT8_C(2);
    tmp_3_3429.transaction_blob_limit = UINT8_C(0);
    tmp_3_3429.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3429.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3429);
  }
  if (schema_fork == UINT64_C(0x0B)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2453 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(London, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3426;
    tmp_3_3426.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3426.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3426.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2453;
    tmp_3_3426.fork = London;
    tmp_3_3426.initcode_size_limit = UINT32_C(0);
    tmp_3_3426.refund_divisor = UINT8_C(5);
    tmp_3_3426.transaction_blob_limit = UINT8_C(0);
    tmp_3_3426.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3426.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3426);
  }
  if (schema_fork == UINT64_C(0x0C)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2454 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(ArrowGlacier, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3423;
    tmp_3_3423.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3423.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3423.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2454;
    tmp_3_3423.fork = ArrowGlacier;
    tmp_3_3423.initcode_size_limit = UINT32_C(0);
    tmp_3_3423.refund_divisor = UINT8_C(5);
    tmp_3_3423.transaction_blob_limit = UINT8_C(0);
    tmp_3_3423.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3423.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3423);
  }
  if (schema_fork == UINT64_C(0x0D)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2455 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(GrayGlacier, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3420;
    tmp_3_3420.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3420.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3420.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2455;
    tmp_3_3420.fork = GrayGlacier;
    tmp_3_3420.initcode_size_limit = UINT32_C(0);
    tmp_3_3420.refund_divisor = UINT8_C(5);
    tmp_3_3420.transaction_blob_limit = UINT8_C(0);
    tmp_3_3420.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3420.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3420);
  }
  if (schema_fork == UINT64_C(0x0E)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2456 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(Paris, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3417;
    tmp_3_3417.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3417.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3417.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2456;
    tmp_3_3417.fork = Paris;
    tmp_3_3417.initcode_size_limit = UINT32_C(0);
    tmp_3_3417.refund_divisor = UINT8_C(5);
    tmp_3_3417.transaction_blob_limit = UINT8_C(0);
    tmp_3_3417.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3417.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3417);
  }
  if (schema_fork == UINT64_C(0x0F)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2457 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(Shanghai, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3414;
    tmp_3_3414.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3414.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3414.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2457;
    tmp_3_3414.fork = Shanghai;
    tmp_3_3414.initcode_size_limit = UINT32_C(49152);
    tmp_3_3414.refund_divisor = UINT8_C(5);
    tmp_3_3414.transaction_blob_limit = UINT8_C(0);
    tmp_3_3414.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3414.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3414);
  }
  if (schema_fork == UINT64_C(0x10)) {
    uint32_t compute_profile_excess_blob_gas_limit_result_2_2458 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(Cancun, UINT8_C(3), UINT8_C(6), UINT32_C(3338477));
    struct ProtocolProfileFields tmp_3_3411 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(3338477), .max = UINT8_C(6), .target = UINT8_C(3)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2458, .fork = Cancun, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(18446744073709551615), .transaction_total_gas_limit = UINT64_C(18446744073709551615)});
    return pack_protocol_profile(tmp_3_3411);
  }
  if (schema_fork == UINT64_C(0x11)) {
    uint32_t compute_profile_excess_blob_gas_limit_result_2_2459 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(Prague, UINT8_C(6), UINT8_C(9), UINT32_C(5007716));
    struct ProtocolProfileFields tmp_3_3408 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(5007716), .max = UINT8_C(9), .target = UINT8_C(6)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2459, .fork = Prague, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(9), .transaction_regular_gas_limit = UINT64_C(18446744073709551615), .transaction_total_gas_limit = UINT64_C(18446744073709551615)});
    return pack_protocol_profile(tmp_3_3408);
  }
  if (schema_fork == UINT64_C(0x12)) {
    uint32_t compute_profile_excess_blob_gas_limit_result_2_2460 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(Osaka, UINT8_C(6), UINT8_C(9), UINT32_C(5007716));
    struct ProtocolProfileFields tmp_3_3405 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(5007716), .max = UINT8_C(9), .target = UINT8_C(6)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2460, .fork = Osaka, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(16777216), .transaction_total_gas_limit = UINT64_C(16777216)});
    return pack_protocol_profile(tmp_3_3405);
  }
  if (schema_fork == UINT64_C(0x13)) {
    uint32_t compute_profile_excess_blob_gas_limit_result_2_2461 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(BPO1, UINT8_C(10), UINT8_C(15), UINT32_C(8346193));
    struct ProtocolProfileFields tmp_3_3402 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(8346193), .max = UINT8_C(15), .target = UINT8_C(10)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2461, .fork = BPO1, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(16777216), .transaction_total_gas_limit = UINT64_C(16777216)});
    return pack_protocol_profile(tmp_3_3402);
  }
  if (schema_fork == UINT64_C(0x14)) {
    uint32_t compute_profile_excess_blob_gas_limit_result_2_2462 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(BPO2, UINT8_C(14), UINT8_C(21), UINT32_C(11684671));
    struct ProtocolProfileFields tmp_3_3399 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(11684671), .max = UINT8_C(21), .target = UINT8_C(14)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2462, .fork = BPO2, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(16777216), .transaction_total_gas_limit = UINT64_C(16777216)});
    return pack_protocol_profile(tmp_3_3399);
  }
  uint32_t compute_profile_excess_blob_gas_limit_result_2_2463 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(Amsterdam, UINT8_C(14), UINT8_C(21), UINT32_C(11684671));
  struct ProtocolProfileFields tmp_3_3396 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(11684671), .max = UINT8_C(21), .target = UINT8_C(14)}), .deployed_code_size_limit = UINT32_C(65536), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2463, .fork = Amsterdam, .initcode_size_limit = UINT32_C(131072), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(16777216), .transaction_total_gas_limit = UINT64_C(18446744073709551615)});
  return pack_protocol_profile(tmp_3_3396);
}

void create_letbind_57(void) {

  struct ProtocolProfileFields let_value_3_70 = schema_protocol_profile(UINT64_C(0x15));
  DEFAULT_PROTOCOL_PROFILE = let_value_3_70;
}
void kill_letbind_57(void) {
}

void create_letbind_58(void) {

  struct ExecutionProfileFields let_value_3_71 = execution_profile_for_struct_ProtocolProfileFields_uint8_t_to_struct_ExecutionProfileFields(DEFAULT_PROTOCOL_PROFILE, UINT8_C(0));
  DEFAULT_EXECUTION_PROFILE = let_value_3_71;
}
void kill_letbind_58(void) {
}

uint32_t compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(uint8_t fork, uint8_t target, uint8_t maximum, uint32_t denominator)
{
  if (fork < Cancun) {
    return UINT32_C(0);
  }
  return ((UINT32_C(131072) * ((uint32_t)maximum - (uint32_t)target)) + (denominator * UINT32_C(256)));
}

uint8_t compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(uint8_t fork, uint8_t target, uint8_t maximum, uint8_t denominator)
{
  uint8_t result_8_1075;
  bool lt_int_result_2_2469 = (bool)(fork < Cancun);
  result_8_1075 = lt_int_result_2_2469 ? UINT8_C(0) : (uint8_t)((uint16_t)((uint32_t)(uint16_t)((uint32_t)UINT16_C(256) * (uint32_t)(uint16_t)denominator) + (uint32_t)(uint16_t)(UINT32_C(131072) * ((uint32_t)maximum - (uint32_t)target))));
  return result_8_1075;
}

struct ExecutionProfileFields execution_profile_for_struct_ProtocolProfileFields_uint8_t_to_struct_ExecutionProfileFields(struct ProtocolProfileFields protocol, uint8_t block_limit)
{
  struct GasLimitsFields gas_limits_for_result_2_2464 = gas_limits_for_struct_ProtocolProfileFields_uint8_t_to_struct_GasLimitsFields(protocol, block_limit);
  return ((struct ExecutionProfileFields){.gas = gas_limits_for_result_2_2464, .protocol = protocol});
}

struct GasLimitsFields gas_limits_for_struct_ProtocolProfileFields_uint8_t_to_struct_GasLimitsFields(struct ProtocolProfileFields profile, uint8_t block_limit)
{
  uint64_t transaction_total_limit = block_limit < profile.transaction_total_gas_limit ? (uint64_t)block_limit : profile.transaction_total_gas_limit;
  uint64_t transaction_regular_limit = transaction_total_limit < profile.transaction_regular_gas_limit ? transaction_total_limit : profile.transaction_regular_gas_limit;
  return ((struct GasLimitsFields){.block_limit = (uint64_t)block_limit, .system_regular_limit = SYSTEM_CALL_GAS_LIMIT, .system_state_limit = UINT8_C(0), .transaction_regular_limit = transaction_regular_limit, .transaction_total_limit = transaction_total_limit});
}

