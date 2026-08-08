#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

bytes32 block_header_hash_(struct BlockHeader header, bytes32 transactions_root, bytes32 withdrawals_root, bytes32 requests_hash, bytes32 block_access_list_hash)
{
  return block_header_hash(header, transactions_root, withdrawals_root, requests_hash, block_access_list_hash);
}

