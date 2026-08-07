/* Optimized block primitive boundaries. */
#include "evmsail/host/types.h"
#include "evmsail/prelude.h"

#include "evmsail/primitives/block.h"

#include <stdint.h>
#include <string.h>

bool logs_bloom_matches_ref(bytes256 computed, Bytes reference)
{
  const uint64_t reference_len = reference.len;
  const uint8_t *reference_bytes = reference.bytes;
  if (reference_len != sizeof(computed.bytes) || !reference_bytes) {
    return false;
  }

  for (size_t word = 0; word < sizeof(computed.bytes) / sizeof(uint64_t); ++word) {
    uint64_t computed_word = 0;
    uint64_t reference_word = 0;
    const size_t reference_offset = sizeof(computed.bytes) - ((word + 1) * sizeof(uint64_t));
    memcpy(&computed_word, computed.bytes + (word * sizeof(uint64_t)), sizeof(computed_word));
    memcpy(&reference_word, reference_bytes + reference_offset, sizeof(reference_word));
    if (computed_word != __builtin_bswap64(reference_word)) {
      return false;
    }
  }
  return true;
}
