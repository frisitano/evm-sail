#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint16_t STACK_LIMIT = UINT16_C(1024);


__int128 validated_refund_add(__int128 left, __int128 right)
{
  if ((!((left + right) < (-((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551416))) - 1))) && (!(((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551417))) < (left + right)))) {
    return (left + right);
  }
  fatal_error(ExecutionInvalid);
}

__int128 record_refund(__int128 refund, __int128 delta)
{
  return validated_refund_add(refund, delta);
}

uint32_t frame_code_len(struct CodeFields frame_code)
{
  return frame_code.len;
}

bool frame_jumpdest_valid(struct CodeFields frame_code, uint32_t dest)
{
  return jumpdest_ref_contains(frame_code.jumpdests, frame_code.len, dest);
}

uint64_t conserved_gas_add(uint64_t available, uint64_t credit)
{
  if (credit <= (UINT64_C(18446744073709551615) - available)) {
    return (credit + available);
  }
  fatal_error(ExecutionInvalid);
}

struct tuple_uint_64_uint_64_uint_32 refill_frame_state_gas(uint64_t g, uint64_t state_gas_remaining, uint32_t state_gas_spilled, uint64_t state_gas_reservoir)
{
  struct tuple_uint_64_uint_64_uint_32 result_8_731;
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool result_2_2044 = (bool)(execution_profile.protocol.fork >= Amsterdam);
  if (result_2_2044) {
    uint64_t refilled = conserved_gas_add_uint64_t_uint32_t_to_uint64_t(g, state_gas_spilled);
    struct tuple_uint_64_uint_64_uint_8 tmp_3_3355 = ((struct tuple_uint_64_uint_64_uint_8){.tup0 = refilled, .tup1 = state_gas_reservoir, .tup2 = STATE_GAS_SPILL_ZERO});
    /* conversions */
    result_8_731.tup0 = tmp_3_3355.tup0;
    result_8_731.tup1 = tmp_3_3355.tup1;
    result_8_731.tup2 = (uint32_t)tmp_3_3355.tup2;
    /* end conversions */
  } else {
    result_8_731 = ((struct tuple_uint_64_uint_64_uint_32){.tup0 = g, .tup1 = state_gas_remaining, .tup2 = state_gas_spilled});
  }
  return result_8_731;
}

__int128 frame_state_gas_used(uint64_t state_gas_reservoir, uint64_t state_gas_remaining, uint32_t state_gas_spilled)
{
  return (((__int128)state_gas_reservoir - (__int128)state_gas_remaining) + (__int128)state_gas_spilled);
}

__attribute__((__always_inline__)) struct FrameStatus exceptional_state(uint64_t *restrict state_gas_remaining, uint32_t *restrict state_gas_spilled, uint64_t state_gas_reservoir, enum ExceptionKind k)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool result_2_2039 = (bool)(execution_profile.protocol.fork >= Amsterdam);
  if (result_2_2039) {
    struct FrameStatus Exceptional_result_2_2040 = Exceptional(k);
    (*state_gas_remaining) = state_gas_reservoir;
    (*state_gas_spilled) = (uint32_t)STATE_GAS_SPILL_ZERO;
    return Exceptional_result_2_2040;
  } else {
    struct FrameStatus Exceptional_result_2_2041 = Exceptional(k);
    return Exceptional_result_2_2041;
  }
}

__attribute__((__always_inline__)) uint16_t stack_height(StackPointer top)
{
  return stack_top_height_(top);
}

__attribute__((__always_inline__)) u256 read_stack_word(StackPointer sp)
{
  return stack_slot_read_StackPointer_uint8_t_to_u256(sp, UINT8_C(0));
}

__attribute__((__always_inline__)) void write_stack_word(StackPointer sp, u256 value)
{
  stack_slot_write_StackPointer_uint8_t_u256_to_unit(sp, UINT8_C(0), value);
}

__attribute__((__always_inline__)) void stack_set(StackPointer top, uint16_t n, u256 w)
{
  stack_slot_write_(top, n, w);
}

Bytes returndata_clear(void)
{
  return EMPTY_OUTPUT_SLICE;
}

uint32_t returndata_size(Bytes returndata)
{
  return returndata.len;
}

void returndata_copy(Bytes returndata, uint32_t dst, uint32_t off, uint32_t len)
{
  output_slice_copy(returndata, dst, off, len);
}

void returndata_copy_prefix(Bytes returndata, uint32_t dst, uint32_t want)
{
  uint32_t available = returndata_size(returndata);
  uint32_t copy_length = want < available ? want : available;
  output_slice_copy_Bytes_uint32_t_uint8_t_uint32_t_to_unit(returndata, dst, UINT8_C(0), copy_length);
}

uint32_t returndata_remaining(uint32_t available, uint32_t offset)
{
  return (available - offset);
}

uint32_t memory_high_water(Bytes mem)
{
  return mem.len;
}

Bytes memory_reset(void)
{
  mem_clear();
  return EMPTY_EVM_MEMORY_SLICE;
}

struct tuple_Bytes_Bytes memory_expand_to(Bytes mem, uint32_t new_size)
{
  if (mem.len < new_size) {
    Bytes expanded = mem_expand(new_size);
    return ((struct tuple_Bytes_Bytes){.tup0 = expanded, .tup1 = expanded});
  }
  Bytes memory_sub_slice_result_2_2032 = memory_sub_slice_Bytes_uint8_t_uint32_t_to_Bytes(mem, UINT8_C(0), new_size);
  return ((struct tuple_Bytes_Bytes){.tup0 = memory_sub_slice_result_2_2032, .tup1 = mem});
}

struct tuple_Bytes_Bytes active_memory_slice(Bytes mem, uint32_t off, uint32_t len)
{
  if (len == UINT8_C(0)) {
    return ((struct tuple_Bytes_Bytes){.tup0 = EMPTY_EVM_MEMORY_SLICE, .tup1 = mem});
  }
  struct tuple_Bytes_Bytes result_2_2028 = memory_expand_to(mem, (len + off));
  Bytes memory_sub_slice_result_2_2029 = memory_sub_slice(result_2_2028.tup0, off, len);
  return ((struct tuple_Bytes_Bytes){.tup0 = memory_sub_slice_result_2_2029, .tup1 = result_2_2028.tup1});
}

struct tuple_Bytes_Bytes_1 memory_code_slice(Bytes mem, uint32_t off, uint32_t len)
{
  if (len == UINT8_C(0)) {
    return ((struct tuple_Bytes_Bytes_1){.tup0 = EMPTY_CODE_SLICE, .tup1 = mem});
  }
  struct tuple_Bytes_Bytes result_2_2024 = memory_expand_to(mem, (len + off));
  Bytes initcode = memory_sub_slice(result_2_2024.tup0, off, len);
  Bytes code_db_intern_memory_result_2_2025 = code_db_intern_memory(initcode);
  return ((struct tuple_Bytes_Bytes_1){.tup0 = code_db_intern_memory_result_2_2025, .tup1 = result_2_2024.tup1});
}

Bytes memory_frame_enter(void)
{
  return mem_frame_enter_slice();
}

Bytes memory_frame_leave(Bytes parent)
{
  mem_frame_leave();
  return parent;
}

struct tuple_uint_32_uint_64_StackPointer_Bytes_uint_64_uint_32_int_128_FrameStatus_Message_CodeFields_CalldataSlice restore_frame(struct FrameCheckpoint checkpoint)
{
  operand_stack_pop_frame();
  Bytes memory = memory_frame_leave(checkpoint.memory);
  return ((struct tuple_uint_32_uint_64_StackPointer_Bytes_uint_64_uint_32_int_128_FrameStatus_Message_CodeFields_CalldataSlice){.tup0 = checkpoint.pc, .tup1 = checkpoint.gas_remaining, .tup2 = checkpoint.stack_top, .tup3 = memory, .tup4 = checkpoint.state_gas_remaining, .tup5 = checkpoint.state_gas_spilled, .tup6 = checkpoint.refund, .tup7 = checkpoint.status, .tup8 = checkpoint.message, .tup9 = checkpoint.code, .tup10 = checkpoint.calldata});
}

void mem_set_byte(uint32_t off, uint64_t v)
{
  mem_write_byte(off, v);
}

u256 mem_load(uint32_t off)
{
  return mem_load_word(off);
}

void mem_store(uint32_t off, u256 w)
{
  mem_store_word(off, w);
}

void mem_store_byte(uint32_t off, u256 w)
{
  uint64_t value = word_low_byte(w);
  mem_set_byte(off, value);
}

void mem_mcopy(uint32_t dst, uint32_t src, uint32_t len)
{
  if (len != UINT8_C(0)) {
    mem_move(dst, src, len);
    return;
  }
}

struct tuple_u256_Bytes mem_keccak(Bytes mem, struct MemoryRangeFields range)
{
  struct tuple_Bytes_Bytes result_2_2008 = active_memory_slice(mem, range.off, range.len);
  bytes32 digest = host_keccak_memory(result_2_2008.tup0);
  u256 hash_to_word_result_2_2009 = hash_to_word(digest);
  return ((struct tuple_u256_Bytes){.tup0 = hash_to_word_result_2_2009, .tup1 = result_2_2008.tup1});
}

uint64_t conserved_gas_add_uint64_t_uint32_t_to_uint64_t(uint64_t available, uint32_t credit)
{
  if (credit <= (UINT64_C(18446744073709551615) - available)) {
    return (available + (uint64_t)credit);
  }
  fatal_error(ExecutionInvalid);
}

__int128 record_refund___int128_uint16_t_to___int128(__int128 refund, uint16_t delta)
{
  return validated_refund_add___int128_uint16_t_to___int128(refund, delta);
}

__attribute__((__always_inline__)) void stack_set_StackPointer_uint8_t_u256_to_unit(StackPointer top, uint8_t n, u256 w)
{
  stack_slot_write_StackPointer_uint8_t_u256_to_unit(top, n, w);
}

struct tuple_FrameCheckpoint_StackPointer_Bytes suspend_frame(uint32_t pc, uint64_t gas_remaining, StackPointer stack_top, Bytes evm_memory, uint8_t state_gas_remaining, uint32_t state_gas_spilled, __int128 frame_refund, struct FrameStatus frame_status, struct Message message, struct CodeFields frame_code, struct CalldataSlice calldata)
{
  k_journal_checkpoint();
  StackPointer child_stack = operand_stack_push_empty_frame();
  Bytes child_memory = memory_frame_enter();
  return ((struct tuple_FrameCheckpoint_StackPointer_Bytes){.tup0 = ((struct FrameCheckpoint){.calldata = calldata, .code = frame_code, .gas_remaining = gas_remaining, .memory = evm_memory, .message = message, .pc = pc, .refund = frame_refund, .stack_top = stack_top, .state_gas_remaining = (uint64_t)state_gas_remaining, .state_gas_spilled = state_gas_spilled, .status = frame_status}), .tup1 = child_stack, .tup2 = child_memory});
}

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint16_t_uint16_t_to_enum_StackValidation(StackPointer top, uint16_t inputs, uint16_t outputs)
{
  uint16_t height = stack_height(top);
  if (height < inputs) {
    return StackUnderflowFailure;
  }
  bool result_2_2037 = (bool)(STACK_LIMIT < (uint16_t)((int16_t)((int32_t)(int16_t)((uint16_t)((uint32_t)height - (uint32_t)inputs)) + (int32_t)(int16_t)outputs)));
  if (result_2_2037) {
    return StackOverflowFailure;
  }
  return StackValid;
}

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint16_t_uint8_t_to_enum_StackValidation(StackPointer top, uint16_t inputs, uint8_t outputs)
{
  uint16_t height = stack_height(top);
  if (height < inputs) {
    return StackUnderflowFailure;
  }
  bool result_2_2037 = (bool)(STACK_LIMIT < (uint16_t)((int16_t)((int32_t)(int16_t)((uint16_t)((uint32_t)height - (uint32_t)inputs)) + (int32_t)(int16_t)outputs)));
  if (result_2_2037) {
    return StackOverflowFailure;
  }
  return StackValid;
}

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint8_t_uint8_t_to_enum_StackValidation(StackPointer top, uint8_t inputs, uint8_t outputs)
{
  uint16_t height = stack_height(top);
  if (height < inputs) {
    return StackUnderflowFailure;
  }
  bool result_2_2037 = (bool)(STACK_LIMIT < ((uint16_t)(((uint32_t)height - (uint32_t)(uint16_t)inputs) + (uint32_t)(uint16_t)outputs)));
  if (result_2_2037) {
    return StackOverflowFailure;
  }
  return StackValid;
}

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint8_t_uint8_t_to_enum_StackValidation_variant_2(StackPointer top, uint8_t inputs, uint8_t outputs)
{
  uint16_t height = stack_height(top);
  if (height < inputs) {
    return StackUnderflowFailure;
  }
  bool result_2_2037 = (bool)(STACK_LIMIT < (uint16_t)((int16_t)((int32_t)(int16_t)((uint16_t)((uint32_t)height - (uint32_t)(uint16_t)inputs)) + (int32_t)(int16_t)outputs)));
  if (result_2_2037) {
    return StackOverflowFailure;
  }
  return StackValid;
}

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint8_t_uint8_t_to_enum_StackValidation_variant_3(StackPointer top, uint8_t inputs, uint8_t outputs)
{
  uint16_t height = stack_height(top);
  bool result_2_2037 = (bool)(STACK_LIMIT < ((uint16_t)(((uint32_t)height - (uint32_t)(uint16_t)inputs) + (uint32_t)(uint16_t)outputs)));
  if (result_2_2037) {
    return StackOverflowFailure;
  }
  return StackValid;
}

__int128 validated_refund_add___int128_uint16_t_to___int128(__int128 left, uint16_t right)
{
  if ((!((left + (__int128)right) < (-((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551416))) - 1))) && (!(((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551417))) < (left + (__int128)right)))) {
    return (left + (__int128)right);
  }
  fatal_error(ExecutionInvalid);
}

