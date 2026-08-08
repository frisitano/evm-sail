#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

uint8_t * analyze_code(Bytes code, uint8_t fork)
{
  bool gteq_int_result_2_2251 = (bool)(fork >= Amsterdam);
  return code_db_analyze_indexed(code, gteq_int_result_2_2251);
}

Bytes code_db_intern_input(Bytes bytes)
{
  Bytes region = code_region_from_input(bytes);
  return validated_code_slice(region);
}

Bytes code_db_intern_memory(Bytes bytes)
{
  Bytes region = code_region_from_memory(bytes);
  return validated_code_slice(region);
}

Bytes code_db_intern_output(Bytes bytes)
{
  Bytes region = code_region_from_output(bytes);
  return validated_code_slice(region);
}

struct CodeFields code_db_resolve(bytes32 code_hash)
{
  return code_db_resolve_indexed(code_hash);
}

bytes32 code_db_insert(Bytes code, uint8_t fork)
{
  uint8_t * jumpdest_table = analyze_code(code, fork);
  struct CodeFields analyzed = analyzed_code(code, jumpdest_table);
  return code_db_store_indexed(analyzed);
}

