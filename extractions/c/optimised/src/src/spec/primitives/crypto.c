#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const bytes32 KECCAK_EMPTY = {
  .lanes = {
      UINT64_C(4333579421379646149),
      UINT64_C(13836122230913597074),
      UINT64_C(4262519377828905189),
      UINT64_C(8116759062988257915)
  },
};


const bytes32 EMPTY_TRIE_ROOT = {
  .lanes = {
      UINT64_C(11985710400040593494),
      UINT64_C(7996352875557389311),
      UINT64_C(13883872631106586715),
      UINT64_C(2428675928708047361)
  },
};


const u256 SECP_N_HALF = (u256){{UINT64_C(16134479119472337056), UINT64_C(6725966010171805725), UINT64_C(18446744073709551615), UINT64_C(9223372036854775807)}};


bytes32 calldata_sha256(struct CalldataSlice input)
{
  switch (input.kind) {
  case Kind_InputCalldata:
    return host_sha256_stateless_input(input.variants.InputCalldata);
  case Kind_MemoryCalldata:
    return host_sha256_memory(input.variants.MemoryCalldata);
  }
}

struct tuple_bool_bytes20 ecrecover_addr(bytes32 h, uint8_t yparity, u256 r, u256 s)
{
  struct AddressResult recovered = precompile_ecrecover_hash_sig(h, yparity, r, s);
  return ((struct tuple_bool_bytes20){.tup0 = recovered.success, .tup1 = recovered.address});
}

