#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint8_t ADDRESS_BYTE_LENGTH = UINT8_C(20);


const uint8_t WORD_BYTE_LENGTH = UINT8_C(32);


const uint8_t EIGHT_BYTE_LENGTH = UINT8_C(8);


const uint8_t DOUBLE_WORD_BYTE_LENGTH = UINT8_C(64);


Bytes EMPTY_STATELESS_INPUT_SLICE;


Bytes EMPTY_SCRATCH_SLICE;


Bytes EMPTY_EVM_MEMORY_SLICE;


Bytes EMPTY_CODE_REGION_SLICE;


Bytes EMPTY_OUTPUT_SLICE;


struct CalldataSlice EMPTY_CALLDATA;


uint32_t calldata_slice_length(struct CalldataSlice s)
{
  switch (s.kind) {
  case Kind_InputCalldata:
    return s.variants.InputCalldata.len;
  case Kind_MemoryCalldata:
    return s.variants.MemoryCalldata.len;
  }
}

uint32_t stateless_input_slice_length(Bytes s)
{
  return s.len;
}

void create_letbind_25(void) {

  Bytes let_value_3_31;
  Bytes tmp_3_30;
  tmp_3_30.bytes = NULL;
  tmp_3_30.len = UINT32_C(0);
  let_value_3_31 = tmp_3_30;
  EMPTY_STATELESS_INPUT_SLICE = let_value_3_31;
}
void kill_letbind_25(void) {
}

void create_letbind_26(void) {

  Bytes let_value_3_33;
  Bytes tmp_3_32;
  tmp_3_32.bytes = NULL;
  tmp_3_32.len = UINT32_C(0);
  let_value_3_33 = tmp_3_32;
  EMPTY_SCRATCH_SLICE = let_value_3_33;
}
void kill_letbind_26(void) {
}

void create_letbind_27(void) {

  Bytes let_value_3_35;
  Bytes tmp_3_34;
  tmp_3_34.bytes = NULL;
  tmp_3_34.len = UINT32_C(0);
  let_value_3_35 = tmp_3_34;
  EMPTY_EVM_MEMORY_SLICE = let_value_3_35;
}
void kill_letbind_27(void) {
}

void create_letbind_28(void) {

  Bytes let_value_3_37;
  Bytes tmp_3_36;
  tmp_3_36.bytes = NULL;
  tmp_3_36.len = UINT32_C(0);
  let_value_3_37 = tmp_3_36;
  EMPTY_CODE_REGION_SLICE = let_value_3_37;
}
void kill_letbind_28(void) {
}

void create_letbind_30(void) {

  Bytes let_value_3_41;
  Bytes tmp_3_40;
  tmp_3_40.bytes = NULL;
  tmp_3_40.len = UINT32_C(0);
  let_value_3_41 = tmp_3_40;
  EMPTY_OUTPUT_SLICE = let_value_3_41;
}
void kill_letbind_30(void) {
}

void create_letbind_31(void) {

  struct CalldataSlice let_value_3_42 = InputCalldata(EMPTY_STATELESS_INPUT_SLICE);
  EMPTY_CALLDATA = let_value_3_42;
}
void kill_letbind_31(void) {
}

Bytes stateless_input_sub_slice(Bytes s, uint32_t off, uint32_t len)
{
  return ((Bytes){.bytes = (s.bytes + off), .len = len});
}

Bytes scratch_sub_slice(Bytes s, uint32_t off, uint32_t len)
{
  return ((Bytes){.bytes = (s.bytes + off), .len = len});
}

Bytes memory_sub_slice(Bytes s, uint32_t off, uint32_t len)
{
  return ((Bytes){.bytes = (s.bytes + off), .len = len});
}

Bytes stateless_input_slice_suffix(Bytes s, uint32_t off)
{
  return ((Bytes){.bytes = (s.bytes + off), .len = (s.len - off)});
}

Bytes scratch_slice_suffix(Bytes s, uint32_t off)
{
  return ((Bytes){.bytes = (s.bytes + off), .len = (s.len - off)});
}

void initialize_block_access_list_state_(Bytes bytes, bytes32 parent_state_root, uint32_t transaction_count)
{
  initialize_block_access_list_state(bytes, parent_state_root, transaction_count);
}

struct CalldataSlice calldata_sub_slice(struct CalldataSlice s, uint8_t off, uint8_t len)
{
  switch (s.kind) {
  case Kind_InputCalldata:
  {
    Bytes bytes = s.variants.InputCalldata;
    if (((uint64_t)len + (uint64_t)off) > bytes.len) {
      __builtin_trap();
    }
    Bytes subslice = stateless_input_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(bytes, off, len);
    return InputCalldata(subslice);
  }
  case Kind_MemoryCalldata:
  {
    Bytes bytes_3_3574 = s.variants.MemoryCalldata;
    if (((uint64_t)len + (uint64_t)off) > bytes_3_3574.len) {
      __builtin_trap();
    }
    Bytes subslice_3_3575 = memory_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(bytes_3_3574, off, len);
    return MemoryCalldata(subslice_3_3575);
  }
  }
}

Bytes evm_memory_slice(const uint8_t * off, uint32_t len)
{
  return ((Bytes){.bytes = off, .len = len});
}

Bytes log_data_sub_slice_Bytes_uint16_t_uint8_t_to_Bytes(Bytes s, uint16_t off, uint8_t len)
{
  return ((Bytes){.bytes = (s.bytes + off), .len = (uint32_t)len});
}

Bytes log_data_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(Bytes s, uint8_t off, uint8_t len)
{
  return ((Bytes){.bytes = (s.bytes + off), .len = (uint32_t)len});
}

Bytes memory_sub_slice_Bytes_uint8_t_uint32_t_to_Bytes(Bytes s, uint8_t off, uint32_t len)
{
  return ((Bytes){.bytes = (s.bytes + off), .len = len});
}

Bytes memory_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(Bytes s, uint8_t off, uint8_t len)
{
  return ((Bytes){.bytes = (s.bytes + off), .len = (uint32_t)len});
}

Bytes scratch_slice_suffix_Bytes_uint8_t_to_Bytes(Bytes s, uint8_t off)
{
  return ((Bytes){.bytes = (s.bytes + off), .len = (s.len - (uint32_t)off)});
}

Bytes scratch_sub_slice_Bytes_uint8_t_uint32_t_to_Bytes(Bytes s, uint8_t off, uint32_t len)
{
  return ((Bytes){.bytes = (s.bytes + off), .len = len});
}

Bytes scratch_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(Bytes s, uint8_t off, uint8_t len)
{
  return ((Bytes){.bytes = (s.bytes + off), .len = (uint32_t)len});
}

Bytes stateless_input_slice(const uint8_t * off, uint32_t len)
{
  return ((Bytes){.bytes = off, .len = len});
}

Bytes stateless_input_slice_suffix_Bytes_uint8_t_to_Bytes(Bytes s, uint8_t off)
{
  return ((Bytes){.bytes = (s.bytes + off), .len = (s.len - (uint32_t)off)});
}

Bytes stateless_input_sub_slice_Bytes_uint8_t_uint16_t_to_Bytes(Bytes s, uint8_t off, uint16_t len)
{
  return ((Bytes){.bytes = (s.bytes + off), .len = (uint32_t)len});
}

Bytes stateless_input_sub_slice_Bytes_uint8_t_uint32_t_to_Bytes(Bytes s, uint8_t off, uint32_t len)
{
  return ((Bytes){.bytes = (s.bytes + off), .len = len});
}

Bytes stateless_input_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(Bytes s, uint8_t off, uint8_t len)
{
  return ((Bytes){.bytes = (s.bytes + off), .len = (uint32_t)len});
}

