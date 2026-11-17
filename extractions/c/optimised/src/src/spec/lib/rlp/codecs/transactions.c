#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

PreparedAuthorizationList prepare_authorizations_(struct AuthorizationListRefFields authorizations)
{
  return prepare_authorizations(authorizations);
}

struct Authorization prepared_authorization_head_(PreparedAuthorizationList authorizations)
{
  return prepared_authorization_head(authorizations);
}

PreparedAuthorizationList prepared_authorization_tail_(PreparedAuthorizationList authorizations,
                                                       uint32_t count)
{
  return prepared_authorization_tail(authorizations, count);
}

struct DecodedTransactionFields rlp_decode_tx(Bytes tx, uint8_t blob_limit)
{
  return decode_transaction(tx, blob_limit);
}
