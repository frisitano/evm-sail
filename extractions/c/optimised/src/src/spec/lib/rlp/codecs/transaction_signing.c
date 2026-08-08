#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

u256 legacy_sig_chain_id(u256 v)
{
  u256 adjusted_v = word_sub_word_u256_uint8_t_to_u256(v, UINT8_C(35));
  return word_div_word_u256_uint8_t_to_u256(adjusted_v, UINT8_C(2));
}

