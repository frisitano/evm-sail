#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint16_t STACK_LIMIT = UINT16_C(1024);


const uint32_t MEMORY_HEIGHT_ZERO = UINT32_C(0);


const uint32_t MEMORY_BASE_ZERO = UINT32_C(0);


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

__int128 frame_state_gas_used(uint64_t state_gas_reservoir, uint64_t state_gas_remaining, uint32_t state_gas_spilled)
{
  return (((__int128)state_gas_reservoir - (__int128)state_gas_remaining) + (__int128)state_gas_spilled);
}

__attribute__((__always_inline__)) struct ExceptionalStateTransition exceptional_state(uint64_t state_gas_remaining, uint32_t state_gas_spilled, uint64_t state_gas_reservoir, enum ExceptionKind k)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  if (execution_profile.protocol.fork >= Amsterdam) {
    struct FrameStatus Exceptional_result_2_2021 = Exceptional(k);
    return ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_2_2021});
  }
  struct FrameStatus Exceptional_result_2_2022 = Exceptional(k);
  return ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_remaining, .state_gas_spilled = state_gas_spilled, .status = Exceptional_result_2_2022});
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

uint32_t memory_high_water(uint32_t height)
{
  return height;
}

uint32_t memory_absolute(uint32_t base, uint32_t relative)
{
  if (relative <= (UINT32_C(4294967295) - base)) {
    return (relative + base);
  }
  fatal_error(ExecutionInvalid);
}

uint32_t memory_parent_base(uint32_t child_base, uint32_t parent_height)
{
  if (parent_height <= child_base) {
    return (child_base - parent_height);
  }
  fatal_error(ExecutionInvalid);
}

__attribute__((__always_inline__)) uint32_t expand_memory(uint32_t base, uint32_t height, uint32_t requested_height)
{
  if (requested_height <= (UINT32_C(4294967295) - base)) {
    if (height < requested_height) {
      mem_expand(base, height, requested_height);
      return requested_height;
    }
    return height;
  }
  fatal_error(ExecutionInvalid);
}

Bytes active_memory_slice(uint32_t base, uint32_t mem, uint32_t off, uint32_t len)
{
  if (len == UINT8_C(0)) {
    return EMPTY_EVM_MEMORY_SLICE;
  }
  if ((mem <= (UINT32_C(4294967295) - base)) && ((len + off) <= mem)) {
    Bytes window = mem_view(base, mem, (len + off));
    return memory_sub_slice(window, off, len);
  }
  fatal_error(ExecutionInvalid);
}

Bytes memory_code_slice(uint32_t base, uint32_t mem, uint32_t off, uint32_t len)
{
  if (len == UINT8_C(0)) {
    return EMPTY_CODE_SLICE;
  }
  if ((mem <= (UINT32_C(4294967295) - base)) && ((len + off) <= mem)) {
    Bytes window = mem_view(base, mem, (len + off));
    Bytes initcode = memory_sub_slice(window, off, len);
    return code_db_intern_memory(initcode);
  }
  fatal_error(ExecutionInvalid);
}

void mem_set_byte(uint32_t base, uint32_t off, uint64_t v)
{
  uint32_t absolute_offset = memory_absolute(base, off);
  mem_write_byte(absolute_offset, v);
}

u256 mem_load(uint32_t base, uint32_t off)
{
  uint32_t absolute_offset = memory_absolute(base, off);
  return mem_load_word(absolute_offset);
}

void mem_store(uint32_t base, uint32_t off, u256 w)
{
  uint32_t absolute_offset = memory_absolute(base, off);
  mem_store_word(absolute_offset, w);
}

void mem_store_byte(uint32_t base, uint32_t off, u256 w)
{
  uint64_t value = word_low_byte(w);
  mem_set_byte(base, off, value);
}

void mem_mcopy(uint32_t base, uint32_t dst, uint32_t src, uint32_t len)
{
  if (len != UINT8_C(0)) {
    uint32_t absolute_dst = memory_absolute(base, dst);
    uint32_t absolute_src = memory_absolute(base, src);
    mem_move(absolute_dst, absolute_src, len);
    return;
  }
}

u256 mem_keccak(uint32_t base, uint32_t mem, struct MemoryRangeFields range)
{
  Bytes bytes = active_memory_slice(base, mem, range.off, range.len);
  bytes32 digest = host_keccak_memory(bytes);
  return hash_to_word(digest);
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

struct tuple_FrameCheckpoint_StackPointer_uint_32_uint_32 suspend_frame(uint32_t pc, uint64_t gas_remaining, StackPointer stack_top, uint32_t memory_base, uint32_t memory_height, uint8_t state_gas_remaining, uint32_t state_gas_spilled, __int128 frame_refund, struct FrameStatus frame_status, struct Message message, struct CodeFields frame_code, struct CalldataSlice calldata)
{
  k_journal_checkpoint();
  StackPointer child_stack = operand_stack_push_empty_frame();
  uint32_t child_memory_base = memory_absolute(memory_base, memory_height);
  uint32_t child_memory_height = MEMORY_HEIGHT_ZERO;
  return ((struct tuple_FrameCheckpoint_StackPointer_uint_32_uint_32){.tup0 = ((struct FrameCheckpoint){.calldata = calldata, .code = frame_code, .gas_remaining = gas_remaining, .memory_height = memory_height, .message = message, .pc = pc, .refund = frame_refund, .stack_top = stack_top, .state_gas_remaining = (uint64_t)state_gas_remaining, .state_gas_spilled = state_gas_spilled, .status = frame_status}), .tup1 = child_stack, .tup2 = child_memory_base, .tup3 = child_memory_height});
}

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint16_t_uint16_t_to_enum_StackValidation(StackPointer top, uint16_t inputs, uint16_t outputs)
{
  uint16_t height = stack_height(top);
  if (height < inputs) {
    return StackUnderflowFailure;
  }
  if (STACK_LIMIT < (uint16_t)((int16_t)((int32_t)(int16_t)((uint16_t)((uint32_t)height - (uint32_t)inputs)) + (int32_t)(int16_t)outputs))) {
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
  if (STACK_LIMIT < (uint16_t)((int16_t)((int32_t)(int16_t)((uint16_t)((uint32_t)height - (uint32_t)inputs)) + (int32_t)(int16_t)outputs))) {
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
  if (STACK_LIMIT < ((uint16_t)(((uint32_t)height - (uint32_t)(uint16_t)inputs) + (uint32_t)(uint16_t)outputs))) {
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
  if (STACK_LIMIT < (uint16_t)((int16_t)((int32_t)(int16_t)((uint16_t)((uint32_t)height - (uint32_t)(uint16_t)inputs)) + (int32_t)(int16_t)outputs))) {
    return StackOverflowFailure;
  }
  return StackValid;
}

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint8_t_uint8_t_to_enum_StackValidation_variant_3(StackPointer top, uint8_t inputs, uint8_t outputs)
{
  uint16_t height = stack_height(top);
  if (STACK_LIMIT < ((uint16_t)(((uint32_t)height - (uint32_t)(uint16_t)inputs) + (uint32_t)(uint16_t)outputs))) {
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

