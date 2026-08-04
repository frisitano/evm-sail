/* Optimized bindings for the conversion equations in prelude.sail. */
#include "evmsail/prelude.h"

#include "evmsail/primitives/word.h"

const char *exception_location;

U256 hash_to_word(Hash32 bytes) {
  return word_from_hash(bytes);
}

Hash32 word_to_hash(U256 value) {
  return hash_from_word(value);
}

uint8_t u64_bit_length(uint64_t value) {
  return (uint8_t)bit_length_u64(value);
}

Address word_to_address(U256 value) {
  return address_from_word(value);
}

U256 address_to_word(Address bytes) {
  return word_from_address(bytes);
}
