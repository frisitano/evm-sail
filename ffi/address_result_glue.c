/* AddressResult crosses the extern boundary as a generated aggregate.  This
 * glue is compiled against each build's model header, while the underlying
 * host implementations remain independent of generated C layouts. */
#include EVMSAIL_MODEL_H
#include "code_db.h"
#include "precompiles.h"

EVMSAIL_ADDRESS_RESULT_RECORD_RETURN precompile_ecrecover_hash_sig(
    EVMSAIL_ADDRESS_RESULT_RECORD_RESULT(result) sail_hash hash,
    uint64_t yparity, EVMSAIL_WORD_PARAM(r), EVMSAIL_WORD_PARAM(s)) {
  struct zAddressResult value;
  struct zAddressResult *result = &value;
  uint8_t address[20];
  result->zsuccess = precompile_ecrecover_hash_sig_address(
      address, hash, yparity, EVMSAIL_WORD_VALUE(r), EVMSAIL_WORD_VALUE(s));
  evmsail_address_set_be_bytes(&result->zaddress, address);
  return value;
}

EVMSAIL_ADDRESS_RESULT_RECORD_RETURN code_db_read_delegation(
    EVMSAIL_ADDRESS_RESULT_RECORD_RESULT(result) sail_hash hash) {
  struct zAddressResult value;
  struct zAddressResult *result = &value;
  uint8_t address[20];
  result->zsuccess = code_db_read_delegation_address(address, hash);
  evmsail_address_set_be_bytes(&result->zaddress, address);
  return value;
}
