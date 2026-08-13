/* Optimized raw-byte threaded interpreter.
 *
 * The dispatch table owns only control flow. Opcode semantics, calls,
 * creates, exceptional-state accounting, and parent-frame resumption remain
 * generated from Sail. Hot frame state stays in locals until a child is
 * entered or a suspended parent is resumed. */
#include "evmsail/prelude.h"

#include "evmsail/host/frame_stack.h"
#include "evmsail/host/stack.h"
#include "evmsail/spec.h"

#include <stdbool.h>
#include <stdint.h>

/* Compile-time refinements of the canonical EVM constants used by the in-TU
 * stack family.  Their Sail definitions remain the semantic source of truth;
 * spelling the fixed protocol values here prevents loads in the hot loop. */
enum {
  EVM_STACK_LIMIT = 1024,
  EVM_GAS_BASE = 2,
  EVM_GAS_VERYLOW = 3,
};

/* Frame-environment values are cold relative to arithmetic, stack, and memory
 * execution. Keep one address-taken context instead of extending their live
 * ranges through the threaded loop as independent scalar locals. */
struct InterpreterFrameContext {
  bytes20 caller;
  bytes20 address;
  bytes20 code_address;
  u256 value;
  uint64_t state_gas_reservoir;
  bool is_static;
  uint16_t depth;
  struct CodeFields code;
  struct CalldataSlice calldata;
  Bytes returndata;
  struct AccountExecutionContext account;
  uint32_t next_pc;
};

static const uint8_t empty_code_byte = 0;

static inline const uint8_t *interpreter_code_bytes(
    const struct InterpreterFrameContext *frame)
{
  return frame->code.len == 0 ? &empty_code_byte : frame->code.bytes;
}

static inline uint32_t interpreter_cursor_pc(
    const struct InterpreterFrameContext *frame, const uint8_t *ip)
{
  return (uint32_t)(ip - interpreter_code_bytes(frame));
}

static inline void interpreter_load_cursor(
    const struct InterpreterFrameContext *frame, const uint8_t **ip,
    const uint8_t **code_end)
{
  const uint8_t *code_bytes = interpreter_code_bytes(frame);
  const uint32_t pc = frame->next_pc < frame->code.len
                          ? frame->next_pc
                          : frame->code.len;
  *ip = code_bytes + pc;
  *code_end = code_bytes + frame->code.len;
}

typedef struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes
    FrameTransitionResult;

static inline void apply_frame_transition_result(
    struct InterpreterFrameContext *frame, uint64_t *gas,
    uint64_t *state_gas_remaining, uint32_t *state_gas_spilled,
    __int128 *refund, StackPointer *sp, Bytes *memory,
    struct FrameStatus *status, FrameTransitionResult result)
{
  frame->next_pc = result.tup0;
  *gas = result.tup1;
  *state_gas_remaining = result.tup2;
  *state_gas_spilled = result.tup3;
  *refund = result.tup4;
  *status = result.tup5;
  *sp = result.tup6;
  *memory = result.tup7;
  frame->caller = result.tup8;
  frame->address = result.tup9;
  frame->code_address = result.tup10;
  frame->value = result.tup11;
  frame->state_gas_reservoir = result.tup12;
  frame->is_static = result.tup13;
  frame->depth = result.tup14;
  frame->code = result.tup15;
  frame->calldata = result.tup16;
  frame->returndata = result.tup17;
}

__attribute__((noinline)) static void run_frame_entry_transition(
    struct InterpreterFrameContext *frame, uint64_t *gas,
    uint64_t *state_gas_remaining, uint32_t *state_gas_spilled,
    __int128 *refund, StackPointer *sp, Bytes *memory, uint8_t opcode,
    struct FrameStatus *status)
{
  const bytes20 previous_address = frame->address;
  FrameTransitionResult result = run_frame_entry_encoded(
      frame->next_pc, *gas, *state_gas_remaining, *state_gas_spilled, *refund,
      *sp, *memory, frame->caller, frame->address, frame->code_address,
      frame->value, frame->state_gas_reservoir, frame->is_static,
      frame->depth, frame->code, frame->calldata, frame->returndata, opcode);
  apply_frame_transition_result(frame, gas, state_gas_remaining,
                                state_gas_spilled, refund, sp, memory, status,
                                result);
  frame->account = refresh_account_execution_context(
      frame->account, previous_address, frame->address);
}

__attribute__((noinline)) static void resume_frame_transition(
    struct InterpreterFrameContext *frame,
    const struct FrameContinuation *continuation, Bytes output, uint64_t *gas,
    uint64_t *state_gas_remaining, uint32_t *state_gas_spilled,
    __int128 *refund, StackPointer *sp, Bytes *memory,
    struct FrameStatus *status)
{
  const bytes20 previous_address = frame->address;
  FrameTransitionResult result = resume_frame(
      *continuation, output, *gas, *state_gas_remaining, *state_gas_spilled,
      *refund, *status, frame->state_gas_reservoir);
  apply_frame_transition_result(frame, gas, state_gas_remaining,
                                state_gas_spilled, refund, sp, memory, status,
                                result);
  frame->account = refresh_account_execution_context(
      frame->account, previous_address, frame->address);
}

#if defined(EVMSAIL_OPCODE_PROFILE)
enum {
  EVMSAIL_SCOPE_EVM_ALU = 52,
  EVMSAIL_SCOPE_EVM_STACK = 53,
  EVMSAIL_SCOPE_EVM_MEMORY = 54,
  EVMSAIL_SCOPE_EVM_FLOW = 55,
  EVMSAIL_SCOPE_EVM_ENVIRONMENT = 56,
  EVMSAIL_SCOPE_EVM_STATE = 57,
  EVMSAIL_SCOPE_EVM_FRAME = 58,
  EVMSAIL_SCOPE_EVM_HALT = 59,
};

#define OPCODE_FAMILY_NONE UINT8_MAX
#define OPCODE_FAMILY_SWITCH(scope)                                              \
  do {                                                                            \
    if (active_opcode_family != (scope)) {                                        \
      if (active_opcode_family != OPCODE_FAMILY_NONE) {                           \
        cycle_scope_end(active_opcode_family);                                    \
      }                                                                           \
      active_opcode_family = (scope);                                             \
      cycle_scope_start(active_opcode_family);                                    \
    }                                                                             \
  } while (0)
#define OPCODE_FAMILY_END()                                                       \
  do {                                                                            \
    if (active_opcode_family != OPCODE_FAMILY_NONE) {                             \
      cycle_scope_end(active_opcode_family);                                      \
      active_opcode_family = OPCODE_FAMILY_NONE;                                  \
    }                                                                             \
  } while (0)
#else
#define OPCODE_FAMILY_SWITCH(scope) do { (void)0; } while (0)
#define OPCODE_FAMILY_END() do { (void)0; } while (0)
#endif

#define NEXT_OPCODE()                                                            \
  do {                                                                            \
    if (ip == code_end) {                                                         \
      goto end_of_code;                                                           \
    }                                                                             \
    opcode = *ip++;                                                               \
    OPCODE_FAMILY_SWITCH(opcode_families[opcode]);                                \
    goto *dispatch[opcode];                                                       \
  } while (0)

#define REQUIRE_OPCODE_AVAILABLE(raw_opcode)                                      \
  do {                                                                            \
    if (!opcode_available((raw_opcode), fork)) {                                  \
      goto opcode_invalid;                                                            \
    }                                                                              \
  } while (0)

#define FINISH_OPCODE(outcome)                                                    \
  do {                                                                            \
    struct OpcodeOutcome opcode_outcome = (outcome);                              \
    if (opcode_outcome.kind == Kind_Continue) {                                   \
      NEXT_OPCODE();                                                              \
    }                                                                              \
    status = opcode_frame_status(opcode_outcome);                                 \
    goto opcode_done;                                                             \
  } while (0)

#define EXECUTE_HANDLER(call)                                                     \
  do {                                                                            \
    struct OpcodeOutcome outcome = (call);                                        \
    FINISH_OPCODE(outcome);                                                       \
  } while (0)

struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes
threaded_interpret(uint64_t initial_gas, uint64_t initial_state_gas,
                   uint32_t initial_state_spill, __int128 initial_refund,
                   StackPointer initial_sp, Bytes initial_memory,
                   bytes20 initial_caller, bytes20 initial_address,
                   bytes20 initial_code_address, u256 initial_value,
                   uint64_t initial_state_gas_reservoir,
                   bool initial_is_static, uint16_t initial_depth,
                   struct CodeFields initial_code,
                   struct CalldataSlice initial_calldata, uint8_t fork,
                   u256 blob_fee)
{
  static const void *const dispatch[256] = {
      [0x00] = &&opcode_stop, [0x01] = &&opcode_add, [0x02] = &&opcode_mul,
      [0x03] = &&opcode_sub, [0x04] = &&opcode_div, [0x05] = &&opcode_sdiv,
      [0x06] = &&opcode_mod, [0x07] = &&opcode_smod, [0x08] = &&opcode_addmod,
      [0x09] = &&opcode_mulmod, [0x0a] = &&opcode_exp, [0x0b] = &&opcode_signextend,
      [0x0c ... 0x0f] = &&opcode_invalid,
      [0x10] = &&opcode_lt, [0x11] = &&opcode_gt, [0x12] = &&opcode_slt,
      [0x13] = &&opcode_sgt, [0x14] = &&opcode_eq, [0x15] = &&opcode_iszero,
      [0x16] = &&opcode_and, [0x17] = &&opcode_or, [0x18] = &&opcode_xor,
      [0x19] = &&opcode_not, [0x1a] = &&opcode_byte, [0x1b] = &&opcode_shl,
      [0x1c] = &&opcode_shr, [0x1d] = &&opcode_sar, [0x1e] = &&opcode_clz,
      [0x1f] = &&opcode_invalid, [0x20] = &&opcode_keccak256,
      [0x21 ... 0x2f] = &&opcode_invalid,
      [0x30] = &&opcode_address, [0x31] = &&opcode_balance, [0x32] = &&opcode_origin,
      [0x33] = &&opcode_caller, [0x34] = &&opcode_callvalue, [0x35] = &&opcode_calldataload,
      [0x36] = &&opcode_calldatasize, [0x37] = &&opcode_calldatacopy, [0x38] = &&opcode_codesize,
      [0x39] = &&opcode_codecopy, [0x3a] = &&opcode_gasprice, [0x3b] = &&opcode_extcodesize,
      [0x3c] = &&opcode_extcodecopy, [0x3d] = &&opcode_returndatasize, [0x3e] = &&opcode_returndatacopy,
      [0x3f] = &&opcode_extcodehash, [0x40] = &&opcode_blockhash, [0x41] = &&opcode_coinbase,
      [0x42] = &&opcode_timestamp, [0x43] = &&opcode_number, [0x44] = &&opcode_prevrandao,
      [0x45] = &&opcode_gaslimit, [0x46] = &&opcode_chainid, [0x47] = &&opcode_selfbalance,
      [0x48] = &&opcode_basefee, [0x49] = &&opcode_blobhash, [0x4a] = &&opcode_blobbasefee,
      [0x4b] = &&opcode_slotnum, [0x4c ... 0x4f] = &&opcode_invalid,
      [0x50] = &&opcode_pop, [0x51] = &&opcode_mload, [0x52] = &&opcode_mstore,
      [0x53] = &&opcode_mstore8, [0x54] = &&opcode_sload, [0x55] = &&opcode_sstore,
      [0x56] = &&opcode_jump, [0x57] = &&opcode_jumpi, [0x58] = &&opcode_pc,
      [0x59] = &&opcode_msize, [0x5a] = &&opcode_gas, [0x5b] = &&opcode_jumpdest,
      [0x5c] = &&opcode_tload, [0x5d] = &&opcode_tstore, [0x5e] = &&opcode_mcopy,
      [0x5f ... 0x7f] = &&opcode_push_family,
      [0x80 ... 0x8f] = &&opcode_dup_family,
      [0x90 ... 0x9f] = &&opcode_swap_family,
      [0xa0 ... 0xa4] = &&opcode_log_family,
      [0xa5 ... 0xe5] = &&opcode_invalid,
      [0xe6 ... 0xe8] = &&opcode_deep_stack_family,
      [0xe9 ... 0xef] = &&opcode_invalid,
      [0xf0] = &&opcode_frame_entry, [0xf1] = &&opcode_frame_entry, [0xf2] = &&opcode_frame_entry,
      [0xf3] = &&opcode_return, [0xf4] = &&opcode_frame_entry, [0xf5] = &&opcode_frame_entry,
      [0xf6 ... 0xf9] = &&opcode_invalid,
      [0xfa] = &&opcode_frame_entry, [0xfb ... 0xfc] = &&opcode_invalid,
      [0xfd] = &&opcode_revert, [0xfe] = &&opcode_invalid, [0xff] = &&opcode_selfdestruct,
  };

#if defined(EVMSAIL_OPCODE_PROFILE)
  static const uint8_t opcode_families[256] = {
      [0x00] = EVMSAIL_SCOPE_EVM_HALT,
      [0x01 ... 0x0b] = EVMSAIL_SCOPE_EVM_ALU,
      [0x0c ... 0x0f] = EVMSAIL_SCOPE_EVM_HALT,
      [0x10 ... 0x1e] = EVMSAIL_SCOPE_EVM_ALU,
      [0x1f] = EVMSAIL_SCOPE_EVM_HALT,
      [0x20] = EVMSAIL_SCOPE_EVM_MEMORY,
      [0x21 ... 0x2f] = EVMSAIL_SCOPE_EVM_HALT,
      [0x30] = EVMSAIL_SCOPE_EVM_ENVIRONMENT,
      [0x31] = EVMSAIL_SCOPE_EVM_STATE,
      [0x32 ... 0x36] = EVMSAIL_SCOPE_EVM_ENVIRONMENT,
      [0x37] = EVMSAIL_SCOPE_EVM_MEMORY,
      [0x38] = EVMSAIL_SCOPE_EVM_ENVIRONMENT,
      [0x39] = EVMSAIL_SCOPE_EVM_MEMORY,
      [0x3a] = EVMSAIL_SCOPE_EVM_ENVIRONMENT,
      [0x3b ... 0x3c] = EVMSAIL_SCOPE_EVM_STATE,
      [0x3d] = EVMSAIL_SCOPE_EVM_ENVIRONMENT,
      [0x3e] = EVMSAIL_SCOPE_EVM_MEMORY,
      [0x3f] = EVMSAIL_SCOPE_EVM_STATE,
      [0x40 ... 0x46] = EVMSAIL_SCOPE_EVM_ENVIRONMENT,
      [0x47] = EVMSAIL_SCOPE_EVM_STATE,
      [0x48 ... 0x4b] = EVMSAIL_SCOPE_EVM_ENVIRONMENT,
      [0x4c ... 0x4f] = EVMSAIL_SCOPE_EVM_HALT,
      [0x50] = EVMSAIL_SCOPE_EVM_STACK,
      [0x51 ... 0x53] = EVMSAIL_SCOPE_EVM_MEMORY,
      [0x54 ... 0x55] = EVMSAIL_SCOPE_EVM_STATE,
      [0x56 ... 0x58] = EVMSAIL_SCOPE_EVM_FLOW,
      [0x59] = EVMSAIL_SCOPE_EVM_MEMORY,
      [0x5a ... 0x5b] = EVMSAIL_SCOPE_EVM_FLOW,
      [0x5c ... 0x5d] = EVMSAIL_SCOPE_EVM_STATE,
      [0x5e] = EVMSAIL_SCOPE_EVM_MEMORY,
      [0x5f ... 0x9f] = EVMSAIL_SCOPE_EVM_STACK,
      [0xa0 ... 0xa4] = EVMSAIL_SCOPE_EVM_MEMORY,
      [0xa5 ... 0xe5] = EVMSAIL_SCOPE_EVM_HALT,
      [0xe6 ... 0xe8] = EVMSAIL_SCOPE_EVM_STACK,
      [0xe9 ... 0xef] = EVMSAIL_SCOPE_EVM_HALT,
      [0xf0 ... 0xf2] = EVMSAIL_SCOPE_EVM_FRAME,
      [0xf3] = EVMSAIL_SCOPE_EVM_HALT,
      [0xf4 ... 0xf5] = EVMSAIL_SCOPE_EVM_FRAME,
      [0xf6 ... 0xf9] = EVMSAIL_SCOPE_EVM_HALT,
      [0xfa] = EVMSAIL_SCOPE_EVM_FRAME,
      [0xfb ... 0xff] = EVMSAIL_SCOPE_EVM_HALT,
  };
#endif

  uint64_t gas = initial_gas;
  uint64_t state_gas_remaining = initial_state_gas;
  uint32_t state_gas_spilled = initial_state_spill;
  __int128 refund = initial_refund;
  StackPointer sp = initial_sp;
  Bytes memory = initial_memory;
  struct InterpreterFrameContext frame = {
      .caller = initial_caller,
      .address = initial_address,
      .code_address = initial_code_address,
      .value = initial_value,
      .state_gas_reservoir = initial_state_gas_reservoir,
      .is_static = initial_is_static,
      .depth = initial_depth,
      .code = initial_code,
      .calldata = initial_calldata,
      .returndata = EMPTY_OUTPUT_SLICE,
      .account = account_execution_context(initial_address),
      .next_pc = 0,
  };
  struct FrameStatus status = {.kind = Kind_Running};
  const uint8_t *ip;
  const uint8_t *code_end;
  uint8_t opcode;
#if defined(EVMSAIL_OPCODE_PROFILE)
  uint8_t active_opcode_family = OPCODE_FAMILY_NONE;
#endif

  frame_stack_reset();
  interpreter_load_cursor(&frame, &ip, &code_end);
  NEXT_OPCODE();

end_of_code:
  OPCODE_FAMILY_SWITCH(EVMSAIL_SCOPE_EVM_HALT);
  status = execute_stop();
  goto interpreter_continue;

opcode_stop:
  status = execute_stop();
  goto interpreter_continue;
opcode_add: EXECUTE_HANDLER(execute_add(&gas, &sp));
opcode_mul: EXECUTE_HANDLER(execute_mul(&gas, &sp));
opcode_sub: EXECUTE_HANDLER(execute_sub(&gas, &sp));
opcode_div: EXECUTE_HANDLER(execute_div(&gas, &sp));
opcode_sdiv: EXECUTE_HANDLER(execute_sdiv(&gas, &sp));
opcode_mod: EXECUTE_HANDLER(execute_mod(&gas, &sp));
opcode_smod: EXECUTE_HANDLER(execute_smod(&gas, &sp));
opcode_addmod: EXECUTE_HANDLER(execute_addmod(&gas, &sp));
opcode_mulmod: EXECUTE_HANDLER(execute_mulmod(&gas, &sp));
opcode_exp: EXECUTE_HANDLER(execute_exp(&gas, &sp));
opcode_signextend: EXECUTE_HANDLER(execute_signextend(&gas, &sp));
opcode_lt: EXECUTE_HANDLER(execute_lt(&gas, &sp));
opcode_gt: EXECUTE_HANDLER(execute_gt(&gas, &sp));
opcode_slt: EXECUTE_HANDLER(execute_slt(&gas, &sp));
opcode_sgt: EXECUTE_HANDLER(execute_sgt(&gas, &sp));
opcode_eq: EXECUTE_HANDLER(execute_eq(&gas, &sp));
opcode_iszero: EXECUTE_HANDLER(execute_iszero(&gas, &sp));
opcode_and: EXECUTE_HANDLER(execute_and(&gas, &sp));
opcode_or: EXECUTE_HANDLER(execute_or(&gas, &sp));
opcode_xor: EXECUTE_HANDLER(execute_xor(&gas, &sp));
opcode_not: EXECUTE_HANDLER(execute_not(&gas, &sp));
opcode_byte: EXECUTE_HANDLER(execute_byte(&gas, &sp));
opcode_shl: EXECUTE_HANDLER(execute_shl(&gas, &sp));
opcode_shr: EXECUTE_HANDLER(execute_shr(&gas, &sp));
opcode_sar: EXECUTE_HANDLER(execute_sar(&gas, &sp));
opcode_clz:
  REQUIRE_OPCODE_AVAILABLE(0x1e);
  EXECUTE_HANDLER(execute_clz(&gas, &sp));
opcode_keccak256: EXECUTE_HANDLER(execute_keccak256(&gas, &sp, &memory));
opcode_address: EXECUTE_HANDLER(execute_address(frame.address, &gas, &sp));
opcode_balance: EXECUTE_HANDLER(execute_balance(&gas, &sp));
opcode_origin: EXECUTE_HANDLER(execute_origin(&gas, &sp));
opcode_caller: EXECUTE_HANDLER(execute_caller(frame.caller, &gas, &sp));
opcode_callvalue: EXECUTE_HANDLER(execute_callvalue(frame.value, &gas, &sp));
opcode_calldataload: EXECUTE_HANDLER(execute_calldataload(frame.calldata, &gas, &sp));
opcode_calldatasize: EXECUTE_HANDLER(execute_calldatasize(frame.calldata, &gas, &sp));
opcode_calldatacopy: EXECUTE_HANDLER(execute_calldatacopy(frame.calldata, &gas, &sp, &memory));
opcode_codesize: EXECUTE_HANDLER(execute_codesize(frame.code, &gas, &sp));
opcode_codecopy: EXECUTE_HANDLER(execute_codecopy(frame.code, &gas, &sp, &memory));
opcode_gasprice: EXECUTE_HANDLER(execute_gasprice(&gas, &sp));
opcode_extcodesize: EXECUTE_HANDLER(execute_extcodesize(&gas, &sp));
opcode_extcodecopy: EXECUTE_HANDLER(execute_extcodecopy(&gas, &sp, &memory));
opcode_returndatasize: EXECUTE_HANDLER(execute_returndatasize(frame.returndata, &gas, &sp));
opcode_returndatacopy: EXECUTE_HANDLER(execute_returndatacopy(frame.returndata, &gas, &sp, &memory));
opcode_extcodehash: EXECUTE_HANDLER(execute_extcodehash(&gas, &sp));
opcode_blockhash: EXECUTE_HANDLER(execute_blockhash(&gas, &sp));
opcode_coinbase: EXECUTE_HANDLER(execute_coinbase(&gas, &sp));
opcode_timestamp: EXECUTE_HANDLER(execute_timestamp(&gas, &sp));
opcode_number: EXECUTE_HANDLER(execute_number(&gas, &sp));
opcode_prevrandao: EXECUTE_HANDLER(execute_prevrandao(&gas, &sp));
opcode_gaslimit: EXECUTE_HANDLER(execute_gaslimit(&gas, &sp));
opcode_chainid: EXECUTE_HANDLER(execute_chainid(&gas, &sp));
opcode_selfbalance: EXECUTE_HANDLER(execute_selfbalance(frame.address, &gas, &sp));
opcode_basefee:
  REQUIRE_OPCODE_AVAILABLE(0x48);
  EXECUTE_HANDLER(execute_basefee(&gas, &sp));
opcode_blobhash:
  REQUIRE_OPCODE_AVAILABLE(0x49);
  EXECUTE_HANDLER(execute_blobhash(&gas, &sp));
opcode_blobbasefee:
  REQUIRE_OPCODE_AVAILABLE(0x4a);
  EXECUTE_HANDLER(execute_blobbasefee(blob_fee, &gas, &sp));
opcode_slotnum:
  REQUIRE_OPCODE_AVAILABLE(0x4b);
  EXECUTE_HANDLER(execute_slotnum(&gas, &sp));
opcode_pop:
  if (sp.height == 0) {
    goto exceptional_stack_underflow;
  }
  if (gas < EVM_GAS_BASE) {
    goto exceptional_out_of_gas;
  }
  gas -= EVM_GAS_BASE;
  sp.storage--;
  sp.height--;
  NEXT_OPCODE();
opcode_mload: EXECUTE_HANDLER(execute_mload(&gas, &sp, &memory));
opcode_mstore: EXECUTE_HANDLER(execute_mstore(&gas, &sp, &memory));
opcode_mstore8: EXECUTE_HANDLER(execute_mstore8(&gas, &sp, &memory));
opcode_sload: EXECUTE_HANDLER(execute_sload(frame.account, &gas, &sp));
opcode_sstore: {
  struct OpcodeOutcome outcome = execute_sstore(
      frame.account, fork, frame.is_static, &gas, &state_gas_remaining,
      &state_gas_spilled, &refund, &sp);
  FINISH_OPCODE(outcome);
}
opcode_jump: {
  frame.next_pc = interpreter_cursor_pc(&frame, ip);
  struct OpcodeOutcome outcome =
      execute_jump(frame.code, &frame.next_pc, &gas, &sp);
  interpreter_load_cursor(&frame, &ip, &code_end);
  FINISH_OPCODE(outcome);
}
opcode_jumpi: {
  frame.next_pc = interpreter_cursor_pc(&frame, ip);
  struct OpcodeOutcome outcome =
      execute_jumpi(frame.code, &frame.next_pc, &gas, &sp);
  interpreter_load_cursor(&frame, &ip, &code_end);
  FINISH_OPCODE(outcome);
}
opcode_pc: {
  frame.next_pc = interpreter_cursor_pc(&frame, ip);
  struct OpcodeOutcome outcome = execute_pc(&frame.next_pc, &gas, &sp);
  interpreter_load_cursor(&frame, &ip, &code_end);
  FINISH_OPCODE(outcome);
}
opcode_msize: EXECUTE_HANDLER(execute_msize(&gas, &sp, &memory));
opcode_gas: EXECUTE_HANDLER(execute_gas(&gas, &sp));
opcode_jumpdest: {
  struct OpcodeOutcome outcome = execute_jumpdest(&gas);
  FINISH_OPCODE(outcome);
}
opcode_tload:
  REQUIRE_OPCODE_AVAILABLE(0x5c);
  EXECUTE_HANDLER(execute_tload(frame.address, &gas, &sp));
opcode_tstore:
  REQUIRE_OPCODE_AVAILABLE(0x5d);
  EXECUTE_HANDLER(execute_tstore(frame.address, frame.is_static, &gas, &sp));
opcode_mcopy:
  REQUIRE_OPCODE_AVAILABLE(0x5e);
  EXECUTE_HANDLER(execute_mcopy(&gas, &sp, &memory));

opcode_push_family:
  if (opcode == 0x5f) {
    REQUIRE_OPCODE_AVAILABLE(0x5f);
  }
  {
    uint8_t width = opcode - 0x5f;
    uint64_t push_cost = width == 0 ? EVM_GAS_BASE : EVM_GAS_VERYLOW;
    const uint8_t *code_bytes = interpreter_code_bytes(&frame);
    const uint32_t immediate_pc = interpreter_cursor_pc(&frame, ip);
    Bytes code_slice = {.len = frame.code.len, .bytes = code_bytes};

    if (sp.height == EVM_STACK_LIMIT) {
      goto exceptional_stack_overflow;
    }
    if (gas < push_cost) {
      goto exceptional_out_of_gas;
    }
    u256 pushed = read_push(code_slice, immediate_pc, width);

    gas -= push_cost;
    const uint32_t remaining = (uint32_t)(code_end - ip);
    ip += width < remaining ? width : remaining;
    sp.storage[0] = pushed;
    sp.storage++;
    sp.height++;
  }
  NEXT_OPCODE();

opcode_dup_family: {
  uint8_t depth_from_top = opcode - 0x7f;

  if (sp.height < depth_from_top) {
    goto exceptional_stack_underflow;
  }
  if (sp.height == EVM_STACK_LIMIT) {
    goto exceptional_stack_overflow;
  }
  if (gas < EVM_GAS_VERYLOW) {
    goto exceptional_out_of_gas;
  }
  gas -= EVM_GAS_VERYLOW;
  sp.storage[0] = sp.storage[-depth_from_top];
  sp.storage++;
  sp.height++;
  NEXT_OPCODE();
}

opcode_swap_family: {
  uint8_t depth_below_top = opcode - 0x8f;
  u256 temporary;

  if (sp.height <= depth_below_top) {
    goto exceptional_stack_underflow;
  }
  if (gas < EVM_GAS_VERYLOW) {
    goto exceptional_out_of_gas;
  }
  gas -= EVM_GAS_VERYLOW;
  temporary = sp.storage[-1];
  sp.storage[-1] = sp.storage[-1 - depth_below_top];
  sp.storage[-1 - depth_below_top] = temporary;
  NEXT_OPCODE();
}

opcode_log_family: {
  struct OpcodeOutcome outcome = execute_log_encoded(
      frame.address, frame.is_static, &gas, &sp, &memory, opcode);
  FINISH_OPCODE(outcome);
}

opcode_deep_stack_family: {
  REQUIRE_OPCODE_AVAILABLE(opcode);
  frame.next_pc = interpreter_cursor_pc(&frame, ip);
  struct OpcodeOutcome outcome = execute_deep_stack_encoded(
      frame.code, opcode, &frame.next_pc, &gas, &sp);
  interpreter_load_cursor(&frame, &ip, &code_end);
  FINISH_OPCODE(outcome);
}

opcode_frame_entry: {
  frame.next_pc = interpreter_cursor_pc(&frame, ip);
  run_frame_entry_transition(
      &frame, &gas, &state_gas_remaining, &state_gas_spilled, &refund, &sp,
      &memory, opcode, &status);
  interpreter_load_cursor(&frame, &ip, &code_end);
  goto interpreter_continue;
}

opcode_return: {
  status = execute_return(&gas, &sp, &memory);
  goto opcode_done;
}

opcode_revert: {
  status = execute_revert(frame.state_gas_reservoir, &gas,
                          &state_gas_remaining, &state_gas_spilled, &sp,
                          &memory);
  goto opcode_done;
}

opcode_selfdestruct: {
  status = execute_selfdestruct(
      frame.address, fork, frame.is_static, &gas, &state_gas_remaining,
      &state_gas_spilled, &refund, &sp);
  goto opcode_done;
}

opcode_invalid:
  {
    struct OpcodeOutcome outcome = execute_invalid(&gas);
    FINISH_OPCODE(outcome);
  }

exceptional_stack_underflow:
  gas = 0;
  status = (struct FrameStatus){
      .kind = Kind_Exceptional,
      .variants.Exceptional = StackUnderflow,
  };
  goto opcode_done;

exceptional_stack_overflow:
  gas = 0;
  status = (struct FrameStatus){
      .kind = Kind_Exceptional,
      .variants.Exceptional = StackOverflow,
  };
  goto opcode_done;

exceptional_out_of_gas:
  gas = 0;
  status = (struct FrameStatus){
      .kind = Kind_Exceptional,
      .variants.Exceptional = OutOfGas,
  };
  goto opcode_done;

opcode_done:
  if (status.kind == Kind_Exceptional) {
    gas = 0;
    status = exceptional_state(&state_gas_remaining, &state_gas_spilled,
                               frame.state_gas_reservoir,
                               status.variants.Exceptional);
  }

interpreter_continue:
  OPCODE_FAMILY_END();
  if (status.kind == Kind_Running) {
    NEXT_OPCODE();
  }

  {
    Bytes output = frame_output(status);
    const struct FrameContinuation *continuation = frame_stack_pop_borrowed();
    if (continuation == NULL) {
      return (struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes){
          .tup0 = gas,
          .tup1 = state_gas_remaining,
          .tup2 = state_gas_spilled,
          .tup3 = refund,
          .tup4 = status,
          .tup5 = output,
      };
    }

    resume_frame_transition(
        &frame, continuation, output, &gas, &state_gas_remaining,
        &state_gas_spilled, &refund, &sp, &memory, &status);
    interpreter_load_cursor(&frame, &ip, &code_end);
    goto interpreter_continue;
  }
}

#undef EXECUTE_HANDLER
#undef FINISH_OPCODE
#undef NEXT_OPCODE
#undef OPCODE_FAMILY_END
#undef OPCODE_FAMILY_SWITCH
