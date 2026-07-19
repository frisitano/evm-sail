/* AddressResult crosses the extern boundary as a generated aggregate.  This
 * glue is compiled against each build's model header, while the underlying
 * host implementations remain independent of generated C layouts. */
#include EVMSAIL_MODEL_H
#include "code_db.h"
#include "precompiles.h"

EVMSAIL_ADDRESS_RESULT_RECORD_RETURN precompile_ecrecover_hash_sig(
    EVMSAIL_ADDRESS_RESULT_RECORD_RESULT(result) sail_hash hash,
    uint64_t yparity, sail_word r, sail_word s) {
#ifndef EVMSAIL_STANDARD_ABI
  struct zAddressResult value;
  struct zAddressResult *result = &value;
#endif
  uint8_t address[20];
  result->zsuccess = precompile_ecrecover_hash_sig_address(
      address, hash, yparity, r, s);
  evmsail_address_set_be_bytes(&result->zaddress, address);
#ifdef EVMSAIL_STANDARD_ABI
  return;
#else
  return value;
#endif
}

EVMSAIL_ADDRESS_RESULT_RECORD_RETURN code_db_read_delegation(
    EVMSAIL_ADDRESS_RESULT_RECORD_RESULT(result) sail_hash hash) {
#ifndef EVMSAIL_STANDARD_ABI
  struct zAddressResult value;
  struct zAddressResult *result = &value;
#endif
  uint8_t address[20];
  result->zsuccess = code_db_read_delegation_address(address, hash);
  evmsail_address_set_be_bytes(&result->zaddress, address);
#ifdef EVMSAIL_STANDARD_ABI
  return;
#else
  return value;
#endif
}
