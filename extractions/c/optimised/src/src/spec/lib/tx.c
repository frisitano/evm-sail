#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

uint8_t tx_signature_parity(uint64_t chain_id, enum TxSignatureScheme scheme, u256 v)
{
  switch (scheme) {
  case LegacySignature:
  {
    bool eip155_v = word_ule_uint8_t_u256_to_bool(UINT8_C(35), v);
    u256 signature_chain_id = legacy_sig_chain_id(v);
    uint64_t expected_chain_id = word_of_chain_identifier(chain_id);
    if (u256_eq_u64(v, UINT8_C(27)) || (u256_eq_u64(v, UINT8_C(28)) || (eip155_v && u256_eq_u64(signature_chain_id, expected_chain_id)))) {
      u256 parity_bit = word_and(v, WORD_ONE);
      if (eq_u256(parity_bit, WORD_ONE)) {
        return UINT8_C(0);
      }
      return UINT8_C(1);
    }
    fatal_error(InvalidSignature);
  }
  case TypedSignature:
  {
    if (eq_u256(v, WORD_ZERO)) {
      return UINT8_C(0);
    }
    if (eq_u256(v, WORD_ONE)) {
      return UINT8_C(1);
    }
    fatal_error(InvalidSignature);
  }
  }
}

bool tx_auth_valid(bytes20 sender, bytes32 h, uint8_t parity, u256 r, u256 s)
{
  bool high_s = word_ult(SECP_N_HALF, s);
  if (high_s) {
    return false;
  }
  struct AddressResult recovered = ecrecover_addr(h, parity, r, s);
  if (recovered.success) {
    return eq_bytes20(recovered.address, sender);
  }
  return false;
}

