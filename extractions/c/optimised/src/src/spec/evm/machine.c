#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

// register zpc
uint32_t pc;

// register zgas_remaining
uint64_t gas_remaining;

// register zstack_top
uint64_t stack_top;

// register zstate_gas_remaining
uint64_t state_gas_remaining;

// register zstate_gas_spilled
uint32_t state_gas_spilled;

// register zframe_refund
__int128 frame_refund;

// register zframe_status
struct FrameStatus frame_status;

// register zmessage
struct Message message;

// register zcall_depth
uint16_t call_depth;

// register zframe_code
struct CodeFields frame_code;

const uint16_t STACK_LIMIT = UINT16_C(1024);


// register zcalldata
struct CalldataSlice calldata;

// register zreturndata
Bytes returndata;

// register zevm_memory
Bytes evm_memory;

__int128 validated_refund_add(__int128 left, __int128 right)
{
  if ((!((left + right) < (-((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551416))) - 1))) && (!(((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551417))) < (left + right)))) {
    return (left + right);
  }
  fatal_error(ExecutionInvalid);
}

void record_refund(__int128 delta)
{
  frame_refund = validated_refund_add(frame_refund, delta);
}

uint32_t frame_code_len(void)
{
  struct CodeFields code = frame_code;
  return code.len;
}

bool frame_jumpdest_valid(uint32_t dest)
{
  struct CodeFields code = frame_code;
  return jumpdest_ref_contains(code.jumpdests, code.len, dest);
}

uint64_t conserved_gas_add(uint64_t left, uint64_t right)
{
  if (right <= (UINT64_C(18446744073709551615) - left)) {
    return (right + left);
  }
  fatal_error(ExecutionInvalid);
}

uint64_t refill_frame_state_gas(uint64_t g)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool result_2_1827 = (bool)(execution_profile.protocol.fork >= Amsterdam);
  if (result_2_1827) {
    uint64_t refilled = conserved_gas_add_uint64_t_uint32_t_to_uint64_t(g, state_gas_spilled);
    state_gas_remaining = message.state_gas_reservoir;
    state_gas_spilled = STATE_GAS_SPILL_ZERO;
    return refilled;
  }
  return g;
}

__int128 frame_state_gas_used(void)
{
  uint64_t entry = message.state_gas_reservoir;
  uint64_t remaining = state_gas_remaining;
  uint32_t spilled = state_gas_spilled;
  return (((__int128)entry - (__int128)remaining) + (__int128)spilled);
}

uint64_t exc_halt(uint64_t g, enum ExceptionKind k)
{
  refill_frame_state_gas(g);
  frame_status = Exceptional(k);
  return GAS_ZERO;
}

uint16_t stack_height(uint64_t top)
{
  return stack_top_height(top);
}

u256 peek(uint64_t top, uint16_t n)
{
  return stack_slot_read(top, n);
}

uint64_t push_word(uint64_t top, u256 w)
{
  uint64_t pushed = stack_top_advance(top, UINT16_C(1));
  stack_slot_write(pushed, UINT16_C(0), w);
  return pushed;
}

uint64_t push_gas(uint64_t top, uint64_t value)
{
  u256 u256_result_2_1817 = u256_uint64_t_to_u256(value);
  return push_word(top, u256_result_2_1817);
}

struct tuple_u256_bits_64 pop(uint64_t top)
{
  u256 value = stack_slot_read(top, UINT16_C(0));
  uint64_t stack_top_retreat_result_2_1816 = stack_top_retreat(top, UINT16_C(1));
  return ((struct tuple_u256_bits_64){.tup0 = value, .tup1 = stack_top_retreat_result_2_1816});
}

void stack_set(uint64_t top, uint16_t n, u256 w)
{
  stack_slot_write(top, n, w);
}

void calldata_install(struct CalldataSlice data)
{
  calldata = data;
}

void returndata_clear(void)
{
  returndata = EMPTY_OUTPUT_SLICE;
}

uint32_t returndata_size(void)
{
  Bytes data = returndata;
  return data.len;
}

void returndata_copy(uint32_t dst, uint32_t off, uint32_t len)
{
  output_slice_copy(returndata, dst, off, len);
}

void returndata_copy_prefix(uint32_t dst, uint32_t want)
{
  uint32_t available = returndata_size();
  uint32_t copy_length = want < available ? want : available;
  output_slice_copy_Bytes_uint32_t_uint8_t_uint32_t_to_unit(returndata, dst, UINT8_C(0), copy_length);
}

uint32_t returndata_remaining(uint32_t available, uint32_t offset)
{
  return (available - offset);
}

uint64_t validated_returndata_copy(uint64_t g, uint32_t dst, u256 source_offset, u256 length_)
{
  uint32_t returndata_size_result_2_1812 = returndata_size();
  if (!u64_lt_u256(returndata_size_result_2_1812, source_offset)) {
    uint32_t remaining;
    uint32_t tmp_3_3817 = (uint32_t)u256_to_u64(source_offset);
    remaining = returndata_remaining(returndata_size_result_2_1812, tmp_3_3817);
    if (!u64_lt_u256(remaining, length_)) {
      returndata_copy(dst, (uint32_t)u256_to_u64_unchecked(source_offset), (uint32_t)u256_to_u64_unchecked(length_));
      return g;
    }
    return exc_halt(g, InvalidOpcode);
  }
  return exc_halt(g, InvalidOpcode);
}

uint64_t returndata_copy_words(uint64_t g, uint32_t dst, u256 source_offset, u256 length_)
{
  return validated_returndata_copy(g, dst, source_offset, length_);
}

uint32_t memory_high_water(Bytes mem)
{
  return mem.len;
}

void memory_reset(void)
{
  mem_clear();
  evm_memory = EMPTY_EVM_MEMORY_SLICE;
}

struct tuple_Bytes_Bytes memory_expand_to(Bytes mem, uint32_t new_size)
{
  if (mem.len < new_size) {
    Bytes expanded = mem_expand(new_size);
    return ((struct tuple_Bytes_Bytes){.tup0 = expanded, .tup1 = expanded});
  }
  Bytes memory_sub_slice_result_2_1811 = memory_sub_slice_Bytes_uint8_t_uint32_t_to_Bytes(mem, UINT8_C(0), new_size);
  return ((struct tuple_Bytes_Bytes){.tup0 = memory_sub_slice_result_2_1811, .tup1 = mem});
}

struct tuple_Bytes_Bytes active_memory_slice(Bytes mem, uint32_t off, uint32_t len)
{
  if (len == UINT8_C(0)) {
    return ((struct tuple_Bytes_Bytes){.tup0 = EMPTY_EVM_MEMORY_SLICE, .tup1 = mem});
  }
  struct tuple_Bytes_Bytes result_2_1807 = memory_expand_to(mem, (len + off));
  Bytes memory_sub_slice_result_2_1808 = memory_sub_slice(result_2_1807.tup0, off, len);
  return ((struct tuple_Bytes_Bytes){.tup0 = memory_sub_slice_result_2_1808, .tup1 = result_2_1807.tup1});
}

struct tuple_Bytes_Bytes_1 memory_code_slice(Bytes mem, uint32_t off, uint32_t len)
{
  if (len == UINT8_C(0)) {
    return ((struct tuple_Bytes_Bytes_1){.tup0 = EMPTY_CODE_SLICE, .tup1 = mem});
  }
  struct tuple_Bytes_Bytes result_2_1803 = memory_expand_to(mem, (len + off));
  Bytes initcode = memory_sub_slice(result_2_1803.tup0, off, len);
  Bytes code_db_intern_memory_result_2_1804 = code_db_intern_memory(initcode);
  return ((struct tuple_Bytes_Bytes_1){.tup0 = code_db_intern_memory_result_2_1804, .tup1 = result_2_1803.tup1});
}

Bytes memory_frame_enter(void)
{
  Bytes parent = evm_memory;
  evm_memory = mem_frame_enter_slice();
  return parent;
}

void memory_frame_leave(Bytes parent)
{
  mem_frame_leave();
  evm_memory = parent;
}

struct FrameCheckpoint suspend_frame(void)
{
  k_journal_checkpoint();
  uint32_t saved_pc = pc;
  uint64_t saved_gas = gas_remaining;
  uint64_t saved_stack = stack_top;
  uint64_t saved_state_gas = state_gas_remaining;
  uint32_t saved_state_spill = state_gas_spilled;
  __int128 saved_refund = frame_refund;
  struct FrameStatus saved_status = frame_status;
  struct Message saved_message = message;
  uint16_t saved_depth = call_depth;
  struct CodeFields saved_code = frame_code;
  struct CalldataSlice saved_calldata = calldata;
  stack_top = operand_stack_push_empty_frame();
  Bytes saved_memory = memory_frame_enter();
  return ((struct FrameCheckpoint){.call_depth = saved_depth, .calldata = saved_calldata, .code = saved_code, .gas_remaining = saved_gas, .memory = saved_memory, .message = saved_message, .pc = saved_pc, .refund = saved_refund, .stack_top = saved_stack, .state_gas_remaining = saved_state_gas, .state_gas_spilled = saved_state_spill, .status = saved_status});
}

void restore_frame(struct FrameCheckpoint checkpoint)
{
  operand_stack_pop_frame();
  memory_frame_leave(checkpoint.memory);
  pc = checkpoint.pc;
  gas_remaining = checkpoint.gas_remaining;
  stack_top = checkpoint.stack_top;
  state_gas_remaining = checkpoint.state_gas_remaining;
  state_gas_spilled = checkpoint.state_gas_spilled;
  frame_refund = checkpoint.refund;
  frame_status = checkpoint.status;
  message = checkpoint.message;
  call_depth = checkpoint.call_depth;
  frame_code = checkpoint.code;
  calldata = checkpoint.calldata;
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
  struct tuple_Bytes_Bytes result_2_1797 = active_memory_slice(mem, range.off, range.len);
  bytes32 digest = host_keccak_memory(result_2_1797.tup0);
  u256 hash_to_word_result_2_1798 = hash_to_word(digest);
  return ((struct tuple_u256_Bytes){.tup0 = hash_to_word_result_2_1798, .tup1 = result_2_1797.tup1});
}

uint64_t conserved_gas_add_uint64_t_uint32_t_to_uint64_t(uint64_t left, uint32_t right)
{
  if (right <= (UINT64_C(18446744073709551615) - left)) {
    return (left + (uint64_t)right);
  }
  fatal_error(ExecutionInvalid);
}

uint64_t exc_halt_uint64_t_enum_ExceptionKind_to_uint64_t(uint64_t g, enum ExceptionKind k)
{
  refill_frame_state_gas_uint64_t_to_uint64_t(g);
  frame_status = Exceptional(k);
  return GAS_ZERO;
}

u256 peek_uint64_t_uint8_t_to_u256(uint64_t top, uint8_t n)
{
  return stack_slot_read(top, (uint16_t)n);
}

void record_refund_uint16_t_to_unit(uint16_t delta)
{
  frame_refund = validated_refund_add___int128_uint16_t_to___int128(frame_refund, delta);
}

uint64_t refill_frame_state_gas_uint64_t_to_uint64_t(uint64_t g)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool result_2_1827 = (bool)(execution_profile.protocol.fork >= Amsterdam);
  if (result_2_1827) {
    uint64_t refilled = conserved_gas_add_uint64_t_uint32_t_to_uint64_t(g, state_gas_spilled);
    state_gas_remaining = message.state_gas_reservoir;
    state_gas_spilled = STATE_GAS_SPILL_ZERO;
    return refilled;
  }
  return g;
}

void stack_set_uint64_t_uint8_t_u256_to_unit(uint64_t top, uint8_t n, u256 w)
{
  stack_slot_write(top, (uint16_t)n, w);
}

struct tuple_bool_uint_64 validate_stack(uint64_t g, uint64_t top, uint16_t inputs, uint16_t outputs)
{
  uint16_t height = stack_height(top);
  if (height < inputs) {
    uint64_t exc_halt_result_2_1819 = exc_halt(g, StackUnderflow);
    return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = exc_halt_result_2_1819});
  }
  bool result_2_1822 = (bool)(STACK_LIMIT < (uint16_t)((int16_t)((int32_t)(int16_t)((uint16_t)((uint32_t)height - (uint32_t)inputs)) + (int32_t)(int16_t)outputs)));
  if (result_2_1822) {
    uint64_t exc_halt_result_2_1823 = exc_halt(g, StackOverflow);
    return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = exc_halt_result_2_1823});
  }
  return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = g});
}

__int128 validated_refund_add___int128_uint16_t_to___int128(__int128 left, uint16_t right)
{
  if ((!((left + (__int128)right) < (-((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551416))) - 1))) && (!(((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551417))) < (left + (__int128)right)))) {
    return (left + (__int128)right);
  }
  fatal_error(ExecutionInvalid);
}

