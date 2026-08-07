/* AddressResult crosses the extern boundary as a generated aggregate.  This
 * binding is compiled against this backend's generated model header, while the underlying
 * host implementations remain independent of generated C layouts. */
#include EVMSAIL_MODEL_H
#include "code_db.h"
#include "precompiles.h"
#include <string.h>

struct zAddressResult precompile_ecrecover_hash_sig(
     fixed_bytes_32 hash,
    uint64_t yparity, const u256 r, const u256 s) {
  struct zAddressResult value;
  struct zAddressResult *result = &value;
  uint8_t address[20];
  result->zsuccess = precompile_ecrecover_hash_sig_address(
      address, hash, yparity, (r), (s));
  memcpy(result->zaddress.bytes, address, sizeof(address));
  return value;
}

struct zAddressResult code_db_read_delegation(
     fixed_bytes_32 hash) {
  struct zAddressResult value;
  struct zAddressResult *result = &value;
  uint8_t address[20];
  result->zsuccess = code_db_read_delegation_address(address, hash);
  memcpy(result->zaddress.bytes, address, sizeof(address));
  return value;
}
