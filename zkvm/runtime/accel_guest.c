/* Guest-side implementation of the zkvm_accelerators.h API.
 *
 * Every function marshals its arguments per the wire layout in
 * zkvm/zkvm_accel_mmio.h and issues one MMIO call to the accel device, which
 * dispatches into the native Rust accel-host implementation. The crypto
 * itself never executes as guest instructions -- this file is pure argument
 * marshalling, so extractions/c/ code can call the accelerator API unconditionally on
 * both native and guest builds. */
#include "zkvm_accelerators.h"
#include "zkvm_accel_mmio.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* All device I/O is bounced through MAIN-region heap buffers: the guest
 * stack lives in the second spike memory region (see runtime/link.ld), and
 * device access to that region through addr_to_mem returns wrong data on
 * the validation simulator -- hash inputs built in stack frames read back
 * corrupt, and result writes to stack addresses go astray. Keeping every
 * pointer the device sees inside the MAIN region makes the transport exact.
 * The copies are small (hash inputs / fixed-size structs) and never enter
 * the crypto itself. */
static uint8_t *acc_in_bounce;  static uint64_t acc_in_cap;
static uint8_t *acc_out_bounce; static uint64_t acc_out_cap;

static uint8_t *acc_bounce(uint8_t **buf, uint64_t *cap, uint64_t need) {
  if (need <= *cap && *buf) return *buf;
  uint64_t c = *cap ? *cap : 4096;
  while (c < need) c *= 2;
  uint8_t *q = (uint8_t *)realloc(*buf, (size_t)c);
  if (!q) return NULL;
  *buf = q; *cap = c;
  return q;
}

static uint64_t acc_call(uint64_t op, const uint8_t *in, uint64_t inlen,
                         uint8_t *out, uint64_t outcap, int *ok) {
  volatile uint64_t *d = (volatile uint64_t *)ZKVM_ACC_MMIO_BASE;
  uint8_t *bin = acc_bounce(&acc_in_bounce, &acc_in_cap, inlen ? inlen : 1);
  uint8_t *bout = acc_bounce(&acc_out_bounce, &acc_out_cap, outcap ? outcap : 1);
  if (!bin || !bout) { *ok = 0; return 0; }
  memcpy(bin, in, inlen);
  d[ZKVM_ACC_R_OP] = op;
  d[ZKVM_ACC_R_IN] = (uint64_t)(uintptr_t)bin;
  d[ZKVM_ACC_R_INLEN] = inlen;
  d[ZKVM_ACC_R_OUT] = (uint64_t)(uintptr_t)bout;
  d[ZKVM_ACC_R_GO] = 1;
  *ok = (int)d[ZKVM_ACC_R_OK];
  uint64_t outlen = d[ZKVM_ACC_R_OUTLEN];
  if (outlen > outcap) { *ok = 0; return 0; }
  memcpy(out, bout, outlen);
  return outlen;
}

/* 8-aligned staging buffer for multi-argument marshalling (single-threaded). */
static uint8_t *acc_buf;
static uint64_t acc_cap;

static uint8_t *acc_reserve(uint64_t need) {
  if (need <= acc_cap) return acc_buf;
  uint64_t cap = acc_cap ? acc_cap : 512;
  while (cap < need) cap *= 2;
  uint8_t *p = (uint8_t *)realloc(acc_buf, (size_t)cap);
  if (!p) return NULL;
  acc_buf = p;
  acc_cap = cap;
  return acc_buf;
}

static void acc_put_u64(uint8_t *p, uint64_t w) { memcpy(p, &w, 8); }

/* fixed-size args concatenated -> fixed-size out */
static zkvm_status acc_fixed(uint64_t op, const void *const *args,
                             const uint64_t *lens, int nargs,
                             void *out, uint64_t want_outlen) {
  uint64_t total = 0;
  for (int i = 0; i < nargs; i++) total += lens[i];
  uint8_t *buf = acc_reserve(total);
  if (!buf) return ZKVM_EFAIL;
  uint64_t off = 0;
  for (int i = 0; i < nargs; i++) {
    memcpy(buf + off, args[i], lens[i]);
    off += lens[i];
  }
  int ok = 0;
  uint64_t outlen = acc_call(op, buf, total, (uint8_t *)out, want_outlen, &ok);
  return (ok && outlen == want_outlen) ? ZKVM_EOK : ZKVM_EFAIL;
}

static zkvm_status acc_verify(uint64_t op, const void *const *args,
                              const uint64_t *lens, int nargs, bool *verified) {
  uint8_t v = 0;
  zkvm_status st = acc_fixed(op, args, lens, nargs, &v, 1);
  *verified = (st == ZKVM_EOK) && v == 1;
  return st;
}

static zkvm_status acc_hash(uint64_t op, const uint8_t *data, size_t len,
                            void *out) {
  static const uint8_t empty = 0;
  int ok = 0;
  uint64_t outlen = acc_call(op, len ? data : &empty, len, (uint8_t *)out, 32, &ok);
  return (ok && outlen == 32) ? ZKVM_EOK : ZKVM_EFAIL;
}

zkvm_status zkvm_keccak256(const uint8_t *data, size_t len,
                           zkvm_keccak256_hash *output) {
  return acc_hash(ZKVM_ACC_OP_KECCAK256, data, len, output->data);
}

zkvm_status zkvm_sha256(const uint8_t *data, size_t len,
                        zkvm_sha256_hash *output) {
  return acc_hash(ZKVM_ACC_OP_SHA256, data, len, output->data);
}

zkvm_status zkvm_ripemd160(const uint8_t *data, size_t len,
                           zkvm_ripemd160_hash *output) {
  return acc_hash(ZKVM_ACC_OP_RIPEMD160, data, len, output->data);
}

zkvm_status zkvm_secp256k1_verify(const zkvm_secp256k1_hash *msg,
                                  const zkvm_secp256k1_signature *sig,
                                  const zkvm_secp256k1_pubkey *pubkey,
                                  bool *verified) {
  const void *args[] = {msg->data, sig->data, pubkey->data};
  const uint64_t lens[] = {32, 64, 64};
  return acc_verify(ZKVM_ACC_OP_SECP256K1_VERIFY, args, lens, 3, verified);
}

zkvm_status zkvm_secp256k1_ecrecover(const zkvm_secp256k1_hash *msg,
                                     const zkvm_secp256k1_signature *sig,
                                     uint8_t recid,
                                     zkvm_secp256k1_pubkey *output) {
  const void *args[] = {msg->data, sig->data, &recid};
  const uint64_t lens[] = {32, 64, 1};
  return acc_fixed(ZKVM_ACC_OP_SECP256K1_ECRECOVER, args, lens, 3,
                   output->data, 64);
}

zkvm_status zkvm_secp256r1_verify(const zkvm_secp256r1_hash *msg,
                                  const zkvm_secp256r1_signature *sig,
                                  const zkvm_secp256r1_pubkey *pubkey,
                                  bool *verified) {
  const void *args[] = {msg->data, sig->data, pubkey->data};
  const uint64_t lens[] = {32, 64, 64};
  return acc_verify(ZKVM_ACC_OP_SECP256R1_VERIFY, args, lens, 3, verified);
}

zkvm_status zkvm_modexp(const uint8_t *base, size_t base_len,
                        const uint8_t *exp, size_t exp_len,
                        const uint8_t *modulus, size_t mod_len,
                        uint8_t *output) {
  uint64_t total = 24 + (uint64_t)base_len + exp_len + mod_len;
  uint8_t *buf = acc_reserve(total);
  if (!buf) return ZKVM_EFAIL;
  acc_put_u64(buf, base_len);
  acc_put_u64(buf + 8, exp_len);
  acc_put_u64(buf + 16, mod_len);
  memcpy(buf + 24, base, base_len);
  memcpy(buf + 24 + base_len, exp, exp_len);
  memcpy(buf + 24 + base_len + exp_len, modulus, mod_len);
  int ok = 0;
  uint64_t outlen = acc_call(ZKVM_ACC_OP_MODEXP, buf, total, output, mod_len, &ok);
  return (ok && outlen == mod_len) ? ZKVM_EOK : ZKVM_EFAIL;
}

zkvm_status zkvm_bn254_g1_add(const zkvm_bn254_g1_point *p1,
                              const zkvm_bn254_g1_point *p2,
                              zkvm_bn254_g1_point *result) {
  const void *args[] = {p1->data, p2->data};
  const uint64_t lens[] = {64, 64};
  return acc_fixed(ZKVM_ACC_OP_BN254_G1_ADD, args, lens, 2, result->data, 64);
}

zkvm_status zkvm_bn254_g1_mul(const zkvm_bn254_g1_point *point,
                              const zkvm_bn254_scalar *scalar,
                              zkvm_bn254_g1_point *result) {
  const void *args[] = {point->data, scalar->data};
  const uint64_t lens[] = {64, 32};
  return acc_fixed(ZKVM_ACC_OP_BN254_G1_MUL, args, lens, 2, result->data, 64);
}

/* u64 count prefix + raw pair array -> verify byte / point */
static zkvm_status acc_pairs(uint64_t op, const void *pairs, uint64_t num,
                             uint64_t pair_size, void *out,
                             uint64_t want_outlen) {
  uint64_t total = 8 + num * pair_size;
  uint8_t *buf = acc_reserve(total);
  if (!buf) return ZKVM_EFAIL;
  acc_put_u64(buf, num);
  memcpy(buf + 8, pairs, num * pair_size);
  int ok = 0;
  uint64_t outlen = acc_call(op, buf, total, (uint8_t *)out, want_outlen, &ok);
  return (ok && outlen == want_outlen) ? ZKVM_EOK : ZKVM_EFAIL;
}

zkvm_status zkvm_bn254_pairing(const zkvm_bn254_pairing_pair *pairs,
                               size_t num_pairs, bool *verified) {
  uint8_t v = 0;
  zkvm_status st = acc_pairs(ZKVM_ACC_OP_BN254_PAIRING, pairs, num_pairs,
                             sizeof(zkvm_bn254_pairing_pair), &v, 1);
  *verified = (st == ZKVM_EOK) && v == 1;
  return st;
}

zkvm_status zkvm_blake2f(uint32_t rounds, zkvm_blake2f_state *h,
                         const zkvm_blake2f_message *m,
                         const zkvm_blake2f_offset *t, uint8_t f) {
  uint64_t r64 = rounds;
  const void *args[] = {&r64, h->data, m->data, t->data, &f};
  const uint64_t lens[] = {8, 64, 128, 16, 1};
  return acc_fixed(ZKVM_ACC_OP_BLAKE2F, args, lens, 5, h->data, 64);
}

zkvm_status zkvm_kzg_point_eval(const zkvm_kzg_commitment *commitment,
                                const zkvm_kzg_field_element *z,
                                const zkvm_kzg_field_element *y,
                                const zkvm_kzg_proof *proof, bool *verified) {
  const void *args[] = {commitment->data, z->data, y->data, proof->data};
  const uint64_t lens[] = {48, 32, 32, 48};
  return acc_verify(ZKVM_ACC_OP_KZG_POINT_EVAL, args, lens, 4, verified);
}

zkvm_status zkvm_bls12_g1_add(const zkvm_bls12_381_g1_point *p1,
                              const zkvm_bls12_381_g1_point *p2,
                              zkvm_bls12_381_g1_point *result) {
  const void *args[] = {p1->data, p2->data};
  const uint64_t lens[] = {96, 96};
  return acc_fixed(ZKVM_ACC_OP_BLS12_G1_ADD, args, lens, 2, result->data, 96);
}

zkvm_status zkvm_bls12_g1_msm(const zkvm_bls12_381_g1_msm_pair *pairs,
                              size_t num_pairs,
                              zkvm_bls12_381_g1_point *result) {
  return acc_pairs(ZKVM_ACC_OP_BLS12_G1_MSM, pairs, num_pairs,
                   sizeof(zkvm_bls12_381_g1_msm_pair), result->data, 96);
}

zkvm_status zkvm_bls12_g2_add(const zkvm_bls12_381_g2_point *p1,
                              const zkvm_bls12_381_g2_point *p2,
                              zkvm_bls12_381_g2_point *result) {
  const void *args[] = {p1->data, p2->data};
  const uint64_t lens[] = {192, 192};
  return acc_fixed(ZKVM_ACC_OP_BLS12_G2_ADD, args, lens, 2, result->data, 192);
}

zkvm_status zkvm_bls12_g2_msm(const zkvm_bls12_381_g2_msm_pair *pairs,
                              size_t num_pairs,
                              zkvm_bls12_381_g2_point *result) {
  return acc_pairs(ZKVM_ACC_OP_BLS12_G2_MSM, pairs, num_pairs,
                   sizeof(zkvm_bls12_381_g2_msm_pair), result->data, 192);
}

zkvm_status zkvm_bls12_pairing(const zkvm_bls12_381_pairing_pair *pairs,
                               size_t num_pairs, bool *verified) {
  uint8_t v = 0;
  zkvm_status st = acc_pairs(ZKVM_ACC_OP_BLS12_PAIRING, pairs, num_pairs,
                             sizeof(zkvm_bls12_381_pairing_pair), &v, 1);
  *verified = (st == ZKVM_EOK) && v == 1;
  return st;
}

zkvm_status zkvm_bls12_map_fp_to_g1(const zkvm_bls12_381_fp *field_element,
                                    zkvm_bls12_381_g1_point *result) {
  const void *args[] = {field_element->data};
  const uint64_t lens[] = {48};
  return acc_fixed(ZKVM_ACC_OP_BLS12_MAP_FP_TO_G1, args, lens, 1,
                   result->data, 96);
}

zkvm_status zkvm_bls12_map_fp2_to_g2(const zkvm_bls12_381_fp2 *field_element,
                                     zkvm_bls12_381_g2_point *result) {
  const void *args[] = {field_element->data};
  const uint64_t lens[] = {96};
  return acc_fixed(ZKVM_ACC_OP_BLS12_MAP_FP2_TO_G2, args, lens, 1,
                   result->data, 192);
}
