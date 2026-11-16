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
      if (eq_u256((word_and(v, WORD_ONE)), WORD_ONE)) {
        return UINT8_C(0);
      }
      return UINT8_C(1);
    }
    fatal_error(InvalidSignature);
  }
  case TypedSignature:
    if (eq_u256(v, WORD_ZERO)) {
      return UINT8_C(0);
    }
    if (eq_u256(v, WORD_ONE)) {
      return UINT8_C(1);
    }
    fatal_error(InvalidSignature);
  }
}

bytes20 tx_recover_sender(bytes32 h, uint8_t parity, u256 r, u256 s)
{
  bool word_ult_result_2_2444 = word_ult(ZERO_WORD, r);
  bool tmp_3_3461;
  if (word_ult_result_2_2444) {
    bool word_ult_result_2_2443 = word_ult(r, SECP_N_FULL);
    bool tmp_3_3460;
    if (word_ult_result_2_2443) {
      bool word_ult_result_2_2442 = word_ult(ZERO_WORD, s);
      bool tmp_3_3459;
      if (word_ult_result_2_2442) {
        tmp_3_3459 = word_ule_u256_u256_to_bool(s, SECP_N_HALF);
      } else {
        tmp_3_3459 = false;
      }
      tmp_3_3460 = tmp_3_3459;
    } else {
      tmp_3_3460 = false;
    }
    tmp_3_3461 = tmp_3_3460;
  } else {
    tmp_3_3461 = false;
  }
  if (!tmp_3_3461) {
    fatal_error(InvalidSignature);
  }
  struct AddressResult recovered = ecrecover_addr(h, parity, r, s);
  if (recovered.success) {
    return recovered.address;
  }
  fatal_error(InvalidSignature);
}

