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
  return ((struct ExecutionProfileFields){.gas = (gas_limits_for(protocol, block_limit)), .protocol = protocol});
}

struct ProtocolProfileFields schema_protocol_profile(uint64_t schema_fork)
{
  if (schema_fork == UINT64_C(0x0A)) {
    return pack_protocol_profile(((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = (uint32_t)(compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(Berlin, UINT8_C(0), UINT8_C(0), UINT8_C(1))), .fork = Berlin, .initcode_size_limit = UINT32_C(0), .refund_divisor = UINT8_C(2), .transaction_blob_limit = UINT8_C(0), .transaction_regular_gas_limit = UINT64_C(18446744073709551615), .transaction_total_gas_limit = UINT64_C(18446744073709551615)}));
  }
  if (schema_fork == UINT64_C(0x0B)) {
    return pack_protocol_profile(((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = (uint32_t)(compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(London, UINT8_C(0), UINT8_C(0), UINT8_C(1))), .fork = London, .initcode_size_limit = UINT32_C(0), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(0), .transaction_regular_gas_limit = UINT64_C(18446744073709551615), .transaction_total_gas_limit = UINT64_C(18446744073709551615)}));
  }
  if (schema_fork == UINT64_C(0x0C)) {
    return pack_protocol_profile(((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = (uint32_t)(compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(ArrowGlacier, UINT8_C(0), UINT8_C(0), UINT8_C(1))), .fork = ArrowGlacier, .initcode_size_limit = UINT32_C(0), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(0), .transaction_regular_gas_limit = UINT64_C(18446744073709551615), .transaction_total_gas_limit = UINT64_C(18446744073709551615)}));
  }
  if (schema_fork == UINT64_C(0x0D)) {
    return pack_protocol_profile(((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = (uint32_t)(compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(GrayGlacier, UINT8_C(0), UINT8_C(0), UINT8_C(1))), .fork = GrayGlacier, .initcode_size_limit = UINT32_C(0), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(0), .transaction_regular_gas_limit = UINT64_C(18446744073709551615), .transaction_total_gas_limit = UINT64_C(18446744073709551615)}));
  }
  if (schema_fork == UINT64_C(0x0E)) {
    return pack_protocol_profile(((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = (uint32_t)(compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(Paris, UINT8_C(0), UINT8_C(0), UINT8_C(1))), .fork = Paris, .initcode_size_limit = UINT32_C(0), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(0), .transaction_regular_gas_limit = UINT64_C(18446744073709551615), .transaction_total_gas_limit = UINT64_C(18446744073709551615)}));
  }
  if (schema_fork == UINT64_C(0x0F)) {
    return pack_protocol_profile(((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(1), .max = UINT8_C(0), .target = UINT8_C(0)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = (uint32_t)(compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(Shanghai, UINT8_C(0), UINT8_C(0), UINT8_C(1))), .fork = Shanghai, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(0), .transaction_regular_gas_limit = UINT64_C(18446744073709551615), .transaction_total_gas_limit = UINT64_C(18446744073709551615)}));
  }
  if (schema_fork == UINT64_C(0x10)) {
    return pack_protocol_profile(((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(3338477), .max = UINT8_C(6), .target = UINT8_C(3)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = (compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(Cancun, UINT8_C(3), UINT8_C(6), UINT32_C(3338477))), .fork = Cancun, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(18446744073709551615), .transaction_total_gas_limit = UINT64_C(18446744073709551615)}));
  }
  if (schema_fork == UINT64_C(0x11)) {
    return pack_protocol_profile(((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(5007716), .max = UINT8_C(9), .target = UINT8_C(6)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = (compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(Prague, UINT8_C(6), UINT8_C(9), UINT32_C(5007716))), .fork = Prague, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(9), .transaction_regular_gas_limit = UINT64_C(18446744073709551615), .transaction_total_gas_limit = UINT64_C(18446744073709551615)}));
  }
  if (schema_fork == UINT64_C(0x12)) {
    return pack_protocol_profile(((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(5007716), .max = UINT8_C(9), .target = UINT8_C(6)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = (compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(Osaka, UINT8_C(6), UINT8_C(9), UINT32_C(5007716))), .fork = Osaka, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(16777216), .transaction_total_gas_limit = UINT64_C(16777216)}));
  }
  if (schema_fork == UINT64_C(0x13)) {
    return pack_protocol_profile(((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(8346193), .max = UINT8_C(15), .target = UINT8_C(10)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = (compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(BPO1, UINT8_C(10), UINT8_C(15), UINT32_C(8346193))), .fork = BPO1, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(16777216), .transaction_total_gas_limit = UINT64_C(16777216)}));
  }
  if (schema_fork == UINT64_C(0x14)) {
    return pack_protocol_profile(((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(11684671), .max = UINT8_C(21), .target = UINT8_C(14)}), .deployed_code_size_limit = UINT32_C(24576), .excess_blob_gas_limit = (compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(BPO2, UINT8_C(14), UINT8_C(21), UINT32_C(11684671))), .fork = BPO2, .initcode_size_limit = UINT32_C(49152), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(16777216), .transaction_total_gas_limit = UINT64_C(16777216)}));
  }
  return pack_protocol_profile(((struct ProtocolProfileFields){.blob_schedule = ((struct BlobScheduleFields){.base_fee_update_fraction = UINT32_C(11684671), .max = UINT8_C(21), .target = UINT8_C(14)}), .deployed_code_size_limit = UINT32_C(65536), .excess_blob_gas_limit = (compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(Amsterdam, UINT8_C(14), UINT8_C(21), UINT32_C(11684671))), .fork = Amsterdam, .initcode_size_limit = UINT32_C(131072), .refund_divisor = UINT8_C(5), .transaction_blob_limit = UINT8_C(6), .transaction_regular_gas_limit = UINT64_C(16777216), .transaction_total_gas_limit = UINT64_C(18446744073709551615)}));
}

void create_letbind_57(void) {
    DEFAULT_PROTOCOL_PROFILE = (schema_protocol_profile(UINT64_C(0x15)));
}
void kill_letbind_57(void) {
}

void create_letbind_58(void) {

  DEFAULT_EXECUTION_PROFILE = (execution_profile_for_struct_ProtocolProfileFields_uint8_t_to_struct_ExecutionProfileFields(DEFAULT_PROTOCOL_PROFILE, UINT8_C(0)));
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
  if (fork < Cancun) {
    return UINT8_C(0);
  }
  return (uint8_t)((uint16_t)((uint32_t)(uint16_t)((uint32_t)UINT16_C(256) * (uint32_t)(uint16_t)denominator) + (uint32_t)(uint16_t)(UINT32_C(131072) * ((uint32_t)maximum - (uint32_t)target))));
}

struct ExecutionProfileFields execution_profile_for_struct_ProtocolProfileFields_uint8_t_to_struct_ExecutionProfileFields(struct ProtocolProfileFields protocol, uint8_t block_limit)
{
  return ((struct ExecutionProfileFields){.gas = (gas_limits_for_struct_ProtocolProfileFields_uint8_t_to_struct_GasLimitsFields(protocol, block_limit)), .protocol = protocol});
}

struct GasLimitsFields gas_limits_for_struct_ProtocolProfileFields_uint8_t_to_struct_GasLimitsFields(struct ProtocolProfileFields profile, uint8_t block_limit)
{
  uint64_t transaction_total_limit = block_limit < profile.transaction_total_gas_limit ? (uint64_t)block_limit : profile.transaction_total_gas_limit;
  uint64_t transaction_regular_limit = transaction_total_limit < profile.transaction_regular_gas_limit ? transaction_total_limit : profile.transaction_regular_gas_limit;
  return ((struct GasLimitsFields){.block_limit = (uint64_t)block_limit, .system_regular_limit = SYSTEM_CALL_GAS_LIMIT, .system_state_limit = UINT8_C(0), .transaction_regular_limit = transaction_regular_limit, .transaction_total_limit = transaction_total_limit});
}

