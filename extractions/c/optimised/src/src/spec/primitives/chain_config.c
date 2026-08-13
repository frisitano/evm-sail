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
  struct GasLimitsFields gas_limits_for_result_2_2485 = gas_limits_for(protocol, block_limit);
  return ((struct ExecutionProfileFields){.gas = gas_limits_for_result_2_2485, .protocol = protocol});
}

struct ProtocolProfileFields schema_protocol_profile(uint64_t schema_fork)
{
  if (schema_fork == UINT64_C(0x0A)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2473 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(Berlin, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3688;
    tmp_3_3688.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3688.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3688.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2473;
    tmp_3_3688.fork = Berlin;
    tmp_3_3688.initcode_size_limit = UINT32_C(0);
    tmp_3_3688.refund_divisor = UINT8_C(2);
    tmp_3_3688.transaction_blob_limit = UINT8_C(0);
    tmp_3_3688.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3688.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3688);
  }
  if (schema_fork == UINT64_C(0x0B)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2474 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(London, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3685;
    tmp_3_3685.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3685.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3685.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2474;
    tmp_3_3685.fork = London;
    tmp_3_3685.initcode_size_limit = UINT32_C(0);
    tmp_3_3685.refund_divisor = UINT8_C(5);
    tmp_3_3685.transaction_blob_limit = UINT8_C(0);
    tmp_3_3685.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3685.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3685);
  }
  if (schema_fork == UINT64_C(0x0C)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2475 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(ArrowGlacier, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3682;
    tmp_3_3682.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3682.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3682.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2475;
    tmp_3_3682.fork = ArrowGlacier;
    tmp_3_3682.initcode_size_limit = UINT32_C(0);
    tmp_3_3682.refund_divisor = UINT8_C(5);
    tmp_3_3682.transaction_blob_limit = UINT8_C(0);
    tmp_3_3682.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3682.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3682);
  }
  if (schema_fork == UINT64_C(0x0D)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2476 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(GrayGlacier, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3679;
    tmp_3_3679.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3679.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3679.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2476;
    tmp_3_3679.fork = GrayGlacier;
    tmp_3_3679.initcode_size_limit = UINT32_C(0);
    tmp_3_3679.refund_divisor = UINT8_C(5);
    tmp_3_3679.transaction_blob_limit = UINT8_C(0);
    tmp_3_3679.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3679.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3679);
  }
  if (schema_fork == UINT64_C(0x0E)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2477 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(Paris, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3676;
    tmp_3_3676.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3676.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3676.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2477;
    tmp_3_3676.fork = Paris;
    tmp_3_3676.initcode_size_limit = UINT32_C(0);
    tmp_3_3676.refund_divisor = UINT8_C(5);
    tmp_3_3676.transaction_blob_limit = UINT8_C(0);
    tmp_3_3676.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3676.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3676);
  }
  if (schema_fork == UINT64_C(0x0F)) {
    uint8_t compute_profile_excess_blob_gas_limit_result_2_2478 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(Shanghai, UINT8_C(0), UINT8_C(0), UINT8_C(1));
    struct ProtocolProfileFields tmp_3_3673;
    tmp_3_3673.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)});
    tmp_3_3673.deployed_code_size_limit = UINT32_C(24576);
    tmp_3_3673.excess_blob_gas_limit = (uint32_t)compute_profile_excess_blob_gas_limit_result_2_2478;
    tmp_3_3673.fork = Shanghai;
    tmp_3_3673.initcode_size_limit = UINT32_C(49152);
    tmp_3_3673.refund_divisor = UINT8_C(5);
    tmp_3_3673.transaction_blob_limit = UINT8_C(0);
    tmp_3_3673.transaction_regular_gas_limit = UINT64_C(18446744073709551615);
    tmp_3_3673.transaction_total_gas_limit = UINT64_C(18446744073709551615);
    return pack_protocol_profile(tmp_3_3673);
  }
  if (schema_fork == UINT64_C(0x10)) {
    uint32_t compute_profile_excess_blob_gas_limit_result_2_2479 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(Cancun, UINT8_C(3), UINT8_C(6), UINT32_C(3338477));
    struct ProtocolProfileFields tmp_3_3670 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(3338477), .max = UINT8_C(6), .target = UINT8_C(3)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2479, .fork = Cancun, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(18446744073709551615), .transaction_total_gas_limit = UINT64_C(18446744073709551615)});
    return pack_protocol_profile(tmp_3_3670);
  }
  if (schema_fork == UINT64_C(0x11)) {
    uint32_t compute_profile_excess_blob_gas_limit_result_2_2480 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(Prague, UINT8_C(6), UINT8_C(9), UINT32_C(5007716));
    struct ProtocolProfileFields tmp_3_3667 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(5007716), .max = UINT8_C(9), .target = UINT8_C(6)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2480, .fork = Prague, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(9), .transaction_regular_gas_limit = UINT64_C(18446744073709551615), .transaction_total_gas_limit = UINT64_C(18446744073709551615)});
    return pack_protocol_profile(tmp_3_3667);
  }
  if (schema_fork == UINT64_C(0x12)) {
    uint32_t compute_profile_excess_blob_gas_limit_result_2_2481 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(Osaka, UINT8_C(6), UINT8_C(9), UINT32_C(5007716));
    struct ProtocolProfileFields tmp_3_3664 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(5007716), .max = UINT8_C(9), .target = UINT8_C(6)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2481, .fork = Osaka, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(16777216), .transaction_total_gas_limit = UINT64_C(16777216)});
    return pack_protocol_profile(tmp_3_3664);
  }
  if (schema_fork == UINT64_C(0x13)) {
    uint32_t compute_profile_excess_blob_gas_limit_result_2_2482 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(BPO1, UINT8_C(10), UINT8_C(15), UINT32_C(8346193));
    struct ProtocolProfileFields tmp_3_3661 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(8346193), .max = UINT8_C(15), .target = UINT8_C(10)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2482, .fork = BPO1, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(16777216), .transaction_total_gas_limit = UINT64_C(16777216)});
    return pack_protocol_profile(tmp_3_3661);
  }
  if (schema_fork == UINT64_C(0x14)) {
    uint32_t compute_profile_excess_blob_gas_limit_result_2_2483 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(BPO2, UINT8_C(14), UINT8_C(21), UINT32_C(11684671));
    struct ProtocolProfileFields tmp_3_3658 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(11684671), .max = UINT8_C(21), .target = UINT8_C(14)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2483, .fork = BPO2, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(16777216), .transaction_total_gas_limit = UINT64_C(16777216)});
    return pack_protocol_profile(tmp_3_3658);
  }
  uint32_t compute_profile_excess_blob_gas_limit_result_2_2484 = compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(Amsterdam, UINT8_C(14), UINT8_C(21), UINT32_C(11684671));
  struct ProtocolProfileFields tmp_3_3655 = ((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(11684671), .max = UINT8_C(21), .target = UINT8_C(14)}), .deployed_code_size_limit = UINT32_C(65536), .excess_blob_gas_limit = compute_profile_excess_blob_gas_limit_result_2_2484, .fork = Amsterdam, .initcode_size_limit = UINT32_C(131072), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(16777216), .transaction_total_gas_limit = UINT64_C(18446744073709551615)});
  return pack_protocol_profile(tmp_3_3655);
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
  bool lt_int_result_2_2490 = (bool)(fork < Cancun);
  if (lt_int_result_2_2490) {
    return UINT32_C(0);
  }
  return ((UINT32_C(131072) * ((uint32_t)maximum - (uint32_t)target)) + (denominator * UINT32_C(256)));
}

uint8_t compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(uint8_t fork, uint8_t target, uint8_t maximum, uint8_t denominator)
{
  uint8_t result_8_1143;
  bool lt_int_result_2_2490 = (bool)(fork < Cancun);
  result_8_1143 = lt_int_result_2_2490 ? UINT8_C(0) : (uint8_t)((uint16_t)((uint32_t)(uint16_t)((uint32_t)UINT16_C(256) * (uint32_t)(uint16_t)denominator) + (uint32_t)(uint16_t)(UINT32_C(131072) * ((uint32_t)maximum - (uint32_t)target))));
  return result_8_1143;
}

struct ExecutionProfileFields execution_profile_for_struct_ProtocolProfileFields_uint8_t_to_struct_ExecutionProfileFields(struct ProtocolProfileFields protocol, uint8_t block_limit)
{
  struct GasLimitsFields gas_limits_for_result_2_2485 = gas_limits_for_struct_ProtocolProfileFields_uint8_t_to_struct_GasLimitsFields(protocol, block_limit);
  return ((struct ExecutionProfileFields){.gas = gas_limits_for_result_2_2485, .protocol = protocol});
}

struct GasLimitsFields gas_limits_for_struct_ProtocolProfileFields_uint8_t_to_struct_GasLimitsFields(struct ProtocolProfileFields profile, uint8_t block_limit)
{
  uint64_t transaction_total_limit = block_limit < profile.transaction_total_gas_limit ? (uint64_t)block_limit : profile.transaction_total_gas_limit;
  uint64_t transaction_regular_limit = transaction_total_limit < profile.transaction_regular_gas_limit ? transaction_total_limit : profile.transaction_regular_gas_limit;
  return ((struct GasLimitsFields){.block_limit = (uint64_t)block_limit, .system_regular_limit = SYSTEM_CALL_GAS_LIMIT, .system_state_limit = UINT8_C(0), .transaction_regular_limit = transaction_regular_limit, .transaction_total_limit = transaction_total_limit});
}

