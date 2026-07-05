#include "precompiles.h"
#include "host_crypto.h"
#include "lbits_convert.h"
#include "returndata.h"
#include "zkvm_accelerators.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PRE_INMAX  (1u << 21)   /* EEST drives >1.2MB pairing inputs (multi-inf) */
#define PRE_OUTMAX (1u << 17)

/* Staging buffers are lazily-grown heap allocations, NOT statics: the
 * zkvm-standards memory contract requires the whole ELF .bss zeroed, so a
 * multi-MB static array is memory-image the VM must commit to even when no
 * precompile ever runs. Untouched heap pages are free. PRE_INMAX/PRE_OUTMAX
 * remain the hard caps. */
static uint8_t *PRE_in;
static uint32_t PRE_in_cap;
static uint32_t PRE_inlen;
static uint8_t *PRE_scratch;
static uint8_t *PRE_out;
static uint32_t PRE_outcap;
static uint32_t PRE_outlen;
static int      PRE_id;
static int      PRE_ok;
static const uint8_t PRE_empty[1] = {0};
static const uint8_t *PRE_src = PRE_empty;

/* grow *buf to at least need bytes (cap `max`); returns NULL past the cap */
static uint8_t *pre_grow(uint8_t **buf, uint32_t *cap, uint64_t need, uint32_t max) {
  if (need > max) return NULL;
  if (*cap >= need && *buf) return *buf;
  uint32_t nc = *cap ? *cap : 1024;
  while (nc < need) nc *= 2;
  uint8_t *p = (uint8_t *)realloc(*buf, nc);
  if (!p) return NULL;
  *buf = p;
  *cap = nc;
  return p;
}

static void precompile_output_scratch(void) {
  if (!PRE_scratch) PRE_scratch = (uint8_t *)malloc(PRE_OUTMAX);
  PRE_out = PRE_scratch;
  PRE_outcap = PRE_scratch ? PRE_OUTMAX : 0;
}

static uint8_t precompile_byte(uint32_t i) {
  return (i < PRE_inlen) ? PRE_src[i] : 0;
}

static void precompile_copy(uint8_t *dst, uint32_t off, uint32_t n) {
  for (uint32_t i = 0; i < n; i++) dst[i] = precompile_byte(off + i);
}

static uint32_t precompile_input_u32_be(uint32_t off) {
  uint32_t out = 0;
  for (uint32_t i = 0; i < 4; i++) out = (out << 8) | precompile_byte(off + i);
  return out;
}

static uint32_t precompile_pending_output_cap(void) {
  switch (PRE_id) {
    case 4: return PRE_inlen;
    case 5: return precompile_input_u32_be(92);
    case 6: case 7: case 9: case 10: return 64;
    case 11: case 12: case 16: return 128;
    case 13: case 14: case 17: return 256;
    case 1: case 2: case 3: case 8: case 15: case 256: case 257: return 32;
    default: return 0;
  }
}

static void precompile_output_pending(void) {
  uint32_t cap = precompile_pending_output_cap();
  if (cap > PRE_INMAX) cap = PRE_INMAX;
  PRE_out = returndata_prepare_pending(cap);
  PRE_outcap = cap;
}

static void precompile_begin(uint64_t id) {
  PRE_id = (int)id;
  PRE_inlen = 0;
  PRE_outlen = 0;
  PRE_ok = 1;
  PRE_src = PRE_empty;
  precompile_output_scratch();
}

static void precompile_set_input(uint64_t id, const uint8_t *src, uint64_t len) {
  precompile_begin(id);
  if (len > UINT32_MAX || (len && !src)) {
    PRE_ok = 0;
    return;
  }
  PRE_src = len ? src : PRE_empty;
  PRE_inlen = (uint32_t)len;
}

static int precompile_materialize_input(uint64_t padded_len) {
  uint64_t need = padded_len > PRE_inlen ? padded_len : PRE_inlen;
  if (!pre_grow(&PRE_in, &PRE_in_cap, need ? need : 1, PRE_INMAX)) {
    PRE_ok = 0;
    PRE_outlen = 0;
    return 0;
  }
  if (PRE_src != PRE_in) {
    if (PRE_inlen) memcpy(PRE_in, PRE_src, PRE_inlen);
    PRE_src = PRE_in;
  }
  for (uint64_t i = PRE_inlen; i < padded_len; i++) PRE_in[i] = 0;
  return 1;
}

/* ---- EIP-2537 BLS12-381 marshalling: EVM 64-byte-padded field elems <-> blst
 * native compact (Fp 48B). G1 point = x||y (96B). For a G2 point blst serializes
 * each Fp2 coordinate imaginary-part-first (c1||c0), while the EVM layout is
 * c0||c1; the helpers swap accordingly. A raw Fp2 (MAP_FP2 input) stays c0||c1. */
static uint8_t *BLS_scratch;
static uint32_t BLS_scratch_cap;

/* reserve k * pair_size bytes of BLS marshalling space (NULL past the cap) */
static uint8_t *bls_reserve(uint64_t k, uint64_t pair_size) {
  return pre_grow(&BLS_scratch, &BLS_scratch_cap, k * pair_size, PRE_INMAX);
}

static int bls_strip_fp(uint8_t *dst48, uint32_t off) {
  for (uint32_t i = 0; i < 16; i++) if (precompile_byte(off + i) != 0) return 0;
  precompile_copy(dst48, off + 16, 48);
  return 1;
}

static void bls_pad_fp(uint32_t off, const uint8_t *src48) {
  memset(PRE_out + off, 0, 16);
  memcpy(PRE_out + off + 16, src48, 48);
}

static int bls_in_g1(uint8_t *out96, uint32_t off) {
  return bls_strip_fp(out96, off) && bls_strip_fp(out96 + 48, off + 64);
}

static void bls_out_g1(uint32_t off, const uint8_t *c96) {
  bls_pad_fp(off, c96);
  bls_pad_fp(off + 64, c96 + 48);
}

static int bls_in_g2(uint8_t *out192, uint32_t off) {
  return bls_strip_fp(out192 + 48,  off)
      && bls_strip_fp(out192 + 0,   off + 64)
      && bls_strip_fp(out192 + 144, off + 128)
      && bls_strip_fp(out192 + 96,  off + 192);
}

static void bls_out_g2(uint32_t off, const uint8_t *b192) {
  bls_pad_fp(off,       b192 + 48);
  bls_pad_fp(off + 64,  b192 + 0);
  bls_pad_fp(off + 128, b192 + 144);
  bls_pad_fp(off + 192, b192 + 96);
}

static uint64_t precompile_run_current(void) {
  switch (PRE_id) {
    case 0: {
      zkvm_keccak256_hash h;
      if (zkvm_keccak256(PRE_src, PRE_inlen, &h) == ZKVM_EOK) {
        memcpy(PRE_out, h.data, 32);
        PRE_outlen = 32;
      } else PRE_ok = 0;
      break;
    }
    case 2: {
      zkvm_sha256_hash h;
      if (zkvm_sha256(PRE_src, PRE_inlen, &h) == ZKVM_EOK) {
        memcpy(PRE_out, h.data, 32);
        PRE_outlen = 32;
      } else PRE_ok = 0;
      break;
    }
    case 3: {
      zkvm_ripemd160_hash h;
      if (zkvm_ripemd160(PRE_src, PRE_inlen, &h) == ZKVM_EOK) {
        memcpy(PRE_out, h.data, 32);
        PRE_outlen = 32;
      } else PRE_ok = 0;
      break;
    }
    case 4:
      if (PRE_inlen > PRE_outcap) { PRE_ok = 0; PRE_outlen = 0; break; }
      memcpy(PRE_out, PRE_src, PRE_inlen);
      PRE_outlen = PRE_inlen;
      break;
    case 1: {
      uint8_t hash[32], sig[64], pub[64];
      precompile_copy(hash, 0, 32);
      precompile_copy(sig, 64, 64);
      uint8_t v = precompile_byte(63);
      int v_ok = (v == 27 || v == 28);
      for (uint32_t i = 32; i < 63; i++) if (precompile_byte(i) != 0) v_ok = 0;
      PRE_ok = 1;
      PRE_outlen = 0;
      if (v_ok && zkvm_secp256k1_ecrecover((const zkvm_secp256k1_hash*)hash,
              (const zkvm_secp256k1_signature*)sig, (uint8_t)(v - 27), (zkvm_secp256k1_pubkey*)pub) == ZKVM_EOK) {
        zkvm_keccak256_hash a;
        zkvm_keccak256(pub, 64, &a);
        memset(PRE_out, 0, 12);
        memcpy(PRE_out + 12, a.data + 12, 20);
        PRE_outlen = 32;
      }
      break;
    }
    case 6: {
      uint8_t in[128];
      precompile_copy(in, 0, 128);
      if (zkvm_bn254_g1_add((const zkvm_bn254_g1_point*)in, (const zkvm_bn254_g1_point*)(in + 64),
              (zkvm_bn254_g1_point*)PRE_out) == ZKVM_EOK) PRE_outlen = 64;
      else { PRE_ok = 0; PRE_outlen = 0; }
      break;
    }
    case 7: {
      uint8_t in[96];
      precompile_copy(in, 0, 96);
      if (zkvm_bn254_g1_mul((const zkvm_bn254_g1_point*)in, (const zkvm_bn254_scalar*)(in + 64),
              (zkvm_bn254_g1_point*)PRE_out) == ZKVM_EOK) PRE_outlen = 64;
      else { PRE_ok = 0; PRE_outlen = 0; }
      break;
    }
    case 5: {
      uint32_t bl = 0, el = 0, ml = 0;
      for (int i = 0; i < 4; i++) {
        bl = (bl << 8) | precompile_byte(28 + i);
        el = (el << 8) | precompile_byte(60 + i);
        ml = (ml << 8) | precompile_byte(92 + i);
      }
      if (ml == 0) { PRE_ok = 1; PRE_outlen = 0; break; }
      uint64_t need = (uint64_t)96 + bl + el + ml;
      if (need > PRE_INMAX || ml > PRE_outcap) { PRE_ok = 0; PRE_outlen = 0; break; }
      if (!precompile_materialize_input(need)) break;
      if (zkvm_modexp(PRE_in + 96, bl, PRE_in + 96 + bl, el, PRE_in + 96 + bl + el, ml, PRE_out) == ZKVM_EOK)
        PRE_outlen = ml;
      else { PRE_ok = 0; PRE_outlen = 0; }
      break;
    }
    case 8: {
      if (PRE_inlen % 192 != 0) { PRE_ok = 0; PRE_outlen = 0; break; }
      bool verified = false;
      if (!precompile_materialize_input(PRE_inlen)) break;
      if (zkvm_bn254_pairing((const zkvm_bn254_pairing_pair*)PRE_in, (size_t)(PRE_inlen / 192), &verified) == ZKVM_EOK) {
        memset(PRE_out, 0, 32);
        PRE_out[31] = verified ? 1 : 0;
        PRE_outlen = 32;
      } else { PRE_ok = 0; PRE_outlen = 0; }
      break;
    }
    case 9: {
      if (PRE_inlen != 213) { PRE_ok = 0; PRE_outlen = 0; break; }
      uint8_t f = precompile_byte(212);
      if (f != 0 && f != 1) { PRE_ok = 0; PRE_outlen = 0; break; }
      uint32_t rounds = ((uint32_t)precompile_byte(0) << 24) | ((uint32_t)precompile_byte(1) << 16)
          | ((uint32_t)precompile_byte(2) << 8) | precompile_byte(3);
      uint8_t hbuf[64], m[128], t[16];
      precompile_copy(hbuf, 4, 64);
      precompile_copy(m, 68, 128);
      precompile_copy(t, 196, 16);
      if (zkvm_blake2f(rounds, (zkvm_blake2f_state*)hbuf, (const zkvm_blake2f_message*)m,
              (const zkvm_blake2f_offset*)t, f) == ZKVM_EOK) {
        memcpy(PRE_out, hbuf, 64);
        PRE_outlen = 64;
      } else { PRE_ok = 0; PRE_outlen = 0; }
      break;
    }
    case 10: {
      if (PRE_inlen != 192) { PRE_ok = 0; PRE_outlen = 0; break; }
      uint8_t comm[48], z[32], y[32], proof[48], vh[32];
      precompile_copy(vh, 0, 32);
      precompile_copy(z, 32, 32);
      precompile_copy(y, 64, 32);
      precompile_copy(comm, 96, 48);
      precompile_copy(proof, 144, 48);
      zkvm_sha256_hash sh;
      zkvm_sha256(comm, 48, &sh);
      int vh_ok = (vh[0] == 0x01);
      for (int i = 1; i < 32; i++) if (vh[i] != sh.data[i]) vh_ok = 0;
      bool verified = false;
      if (vh_ok && zkvm_kzg_point_eval((const zkvm_kzg_commitment*)comm, (const zkvm_kzg_field_element*)z,
              (const zkvm_kzg_field_element*)y, (const zkvm_kzg_proof*)proof, &verified) == ZKVM_EOK && verified) {
        static const uint8_t BLS_MOD[32] = {
          0x73,0xed,0xa7,0x53,0x29,0x9d,0x7d,0x48,0x33,0x39,0xd8,0x08,0x09,0xa1,0xd8,0x05,
          0x53,0xbd,0xa4,0x02,0xff,0xfe,0x5b,0xfe,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x01 };
        memset(PRE_out, 0, 64);
        PRE_out[30] = 0x10;
        memcpy(PRE_out + 32, BLS_MOD, 32);
        PRE_outlen = 64;
      } else { PRE_ok = 0; PRE_outlen = 0; }
      break;
    }
    case 11: {
      if (PRE_inlen != 256) { PRE_ok = 0; PRE_outlen = 0; break; }
      uint8_t c[192], out[96];
      if (!bls_in_g1(c, 0) || !bls_in_g1(c + 96, 128)) { PRE_ok = 0; PRE_outlen = 0; break; }
      if (zkvm_bls12_g1_add((const zkvm_bls12_381_g1_point*)c, (const zkvm_bls12_381_g1_point*)(c + 96),
              (zkvm_bls12_381_g1_point*)out) == ZKVM_EOK) { bls_out_g1(0, out); PRE_outlen = 128; }
      else { PRE_ok = 0; PRE_outlen = 0; }
      break;
    }
    case 12: {
      if (PRE_inlen == 0 || PRE_inlen % 160 != 0) { PRE_ok = 0; PRE_outlen = 0; break; }
      uint32_t k = PRE_inlen / 160;
      uint8_t *pairs = bls_reserve(k, 128);
      int ok = pairs != NULL;
      for (uint32_t i = 0; ok && i < k; i++) {
        if (!bls_in_g1(pairs + i * 128, i * 160)) { ok = 0; break; }
        precompile_copy(pairs + i * 128 + 96, i * 160 + 128, 32);
      }
      uint8_t out[96];
      if (ok && zkvm_bls12_g1_msm((const zkvm_bls12_381_g1_msm_pair*)pairs, k,
              (zkvm_bls12_381_g1_point*)out) == ZKVM_EOK) { bls_out_g1(0, out); PRE_outlen = 128; }
      else { PRE_ok = 0; PRE_outlen = 0; }
      break;
    }
    case 13: {
      if (PRE_inlen != 512) { PRE_ok = 0; PRE_outlen = 0; break; }
      uint8_t c[384], out[192];
      if (!bls_in_g2(c, 0) || !bls_in_g2(c + 192, 256)) { PRE_ok = 0; PRE_outlen = 0; break; }
      if (zkvm_bls12_g2_add((const zkvm_bls12_381_g2_point*)c, (const zkvm_bls12_381_g2_point*)(c + 192),
              (zkvm_bls12_381_g2_point*)out) == ZKVM_EOK) { bls_out_g2(0, out); PRE_outlen = 256; }
      else { PRE_ok = 0; PRE_outlen = 0; }
      break;
    }
    case 14: {
      if (PRE_inlen == 0 || PRE_inlen % 288 != 0) { PRE_ok = 0; PRE_outlen = 0; break; }
      uint32_t k = PRE_inlen / 288;
      uint8_t *pairs = bls_reserve(k, 224);
      int ok = pairs != NULL;
      for (uint32_t i = 0; ok && i < k; i++) {
        if (!bls_in_g2(pairs + i * 224, i * 288)) { ok = 0; break; }
        precompile_copy(pairs + i * 224 + 192, i * 288 + 256, 32);
      }
      uint8_t out[192];
      if (ok && zkvm_bls12_g2_msm((const zkvm_bls12_381_g2_msm_pair*)pairs, k,
              (zkvm_bls12_381_g2_point*)out) == ZKVM_EOK) { bls_out_g2(0, out); PRE_outlen = 256; }
      else { PRE_ok = 0; PRE_outlen = 0; }
      break;
    }
    case 15: {
      if (PRE_inlen == 0 || PRE_inlen % 384 != 0) { PRE_ok = 0; PRE_outlen = 0; break; }
      uint32_t k = PRE_inlen / 384;
      uint8_t *pairs = bls_reserve(k, 288);
      int ok = pairs != NULL;
      for (uint32_t i = 0; ok && i < k; i++) {
        if (!bls_in_g1(pairs + i * 288, i * 384) ||
            !bls_in_g2(pairs + i * 288 + 96, i * 384 + 128)) { ok = 0; break; }
      }
      bool verified = false;
      if (ok && zkvm_bls12_pairing((const zkvm_bls12_381_pairing_pair*)pairs, k, &verified) == ZKVM_EOK) {
        memset(PRE_out, 0, 32);
        PRE_out[31] = verified ? 1 : 0;
        PRE_outlen = 32;
      } else { PRE_ok = 0; PRE_outlen = 0; }
      break;
    }
    case 16: {
      if (PRE_inlen != 64) { PRE_ok = 0; PRE_outlen = 0; break; }
      uint8_t fp[48], out[96];
      if (!bls_strip_fp(fp, 0)) { PRE_ok = 0; PRE_outlen = 0; break; }
      if (zkvm_bls12_map_fp_to_g1((const zkvm_bls12_381_fp*)fp, (zkvm_bls12_381_g1_point*)out) == ZKVM_EOK) {
        bls_out_g1(0, out);
        PRE_outlen = 128;
      } else { PRE_ok = 0; PRE_outlen = 0; }
      break;
    }
    case 17: {
      if (PRE_inlen != 128) { PRE_ok = 0; PRE_outlen = 0; break; }
      uint8_t fp2[96], out[192];
      if (!bls_strip_fp(fp2, 0) || !bls_strip_fp(fp2 + 48, 64)) { PRE_ok = 0; PRE_outlen = 0; break; }
      if (zkvm_bls12_map_fp2_to_g2((const zkvm_bls12_381_fp2*)fp2, (zkvm_bls12_381_g2_point*)out) == ZKVM_EOK) {
        bls_out_g2(0, out);
        PRE_outlen = 256;
      } else { PRE_ok = 0; PRE_outlen = 0; }
      break;
    }
    case 256: {
      PRE_ok = 1;
      PRE_outlen = 0;
      if (PRE_inlen == 160) {
        uint8_t h[32], sig[64], pk[64];
        bool verified = false;
        precompile_copy(h, 0, 32);
        precompile_copy(sig, 32, 64);
        precompile_copy(pk, 96, 64);
        if (zkvm_secp256r1_verify((const zkvm_secp256r1_hash*)h, (const zkvm_secp256r1_signature*)sig,
                (const zkvm_secp256r1_pubkey*)pk, &verified) == ZKVM_EOK && verified) {
          memset(PRE_out, 0, 32);
          PRE_out[31] = 1;
          PRE_outlen = 32;
        }
      }
      break;
    }
    case 257: {
      PRE_ok = 1;
      PRE_outlen = 0;
      if (PRE_inlen == 160) {
        uint8_t h[32], sig[64], pk[64];
        bool verified = false;
        precompile_copy(h, 0, 32);
        precompile_copy(sig, 32, 64);
        precompile_copy(pk, 96, 64);
        if (zkvm_secp256k1_verify((const zkvm_secp256k1_hash*)h, (const zkvm_secp256k1_signature*)sig,
                (const zkvm_secp256k1_pubkey*)pk, &verified) == ZKVM_EOK && verified) {
          memset(PRE_out, 0, 32);
          PRE_out[31] = 1;
          PRE_outlen = 32;
        }
      }
      break;
    }
    default:
      PRE_ok = 0;
      PRE_outlen = 0;
      break;
  }
  return PRE_outlen;
}

bool precompile_run_source_to_returndata(uint64_t id, uint64_t source_kind,
                                            uint64_t off, uint64_t len) {
  const uint8_t *src = NULL;
  uint64_t resolved_len = 0;
  if (!evmsail_resolve_byte_source(source_kind, off, len, &src, &resolved_len) ||
      resolved_len != len) {
    precompile_begin(id);
    PRE_ok = 0;
    returndata_set_pending_len(0);
    return 0;
  }
  precompile_set_input(id, src, len);
  precompile_output_pending();
  (void)precompile_run_current();
  returndata_set_pending_len(PRE_ok ? PRE_outlen : 0);
  precompile_output_scratch();
  return (uint64_t)PRE_ok;
}

static void precompile_put_be64(uint8_t *p, uint64_t w) {
  for (int i = 7; i >= 0; i--) {
    p[7 - i] = (uint8_t)(w >> (8 * i));
  }
}

bool precompile_secp256k1_verify_hash_sig_pub(const lbits hw, const lbits r,
                                              const lbits s, const lbits x,
                                              const lbits y) {
  uint8_t h[32], sig[64], pk[64];
  bool verified = false;
  lbits_to_be_bytes(h, 32, hw);
  lbits_to_be_bytes(sig, 32, r);
  lbits_to_be_bytes(sig + 32, 32, s);
  lbits_to_be_bytes(pk, 32, x);
  lbits_to_be_bytes(pk + 32, 32, y);
  return zkvm_secp256k1_verify((const zkvm_secp256k1_hash*)h,
                               (const zkvm_secp256k1_signature*)sig,
                               (const zkvm_secp256k1_pubkey*)pk,
                               &verified) == ZKVM_EOK && verified;
}

static void precompile_recovered_address_lbits(lbits *rop, int ok, const uint8_t addr[20]) {
  uint8_t b[21] = {0};
  if (ok) {
    b[0] = 0x01;                 /* success flag: bit 160 */
    memcpy(b + 1, addr, 20);
  }
  be_bytes_to_lbits(rop, 168, b, sizeof b);
}

void precompile_ecrecover_hash_sig(lbits *rop, const lbits hw, uint64_t yparity,
                                   const lbits r, const lbits s) {
  uint8_t h[32], sig[64], pub[64], addr_hash[32], addr[20] = {0};
  uint64_t out[4] = {0, 0, 0, 0};
  int ok = (yparity <= 1);
  lbits_to_be_bytes(h, 32, hw);
  lbits_to_be_bytes(sig, 32, r);
  lbits_to_be_bytes(sig + 32, 32, s);
  if (ok) {
    ok = zkvm_secp256k1_ecrecover((const zkvm_secp256k1_hash*)h,
                                  (const zkvm_secp256k1_signature*)sig,
                                  (uint8_t)yparity,
                                  (zkvm_secp256k1_pubkey*)pub) == ZKVM_EOK;
  }
  if (ok) {
    host_keccak256_bytes(out, pub, sizeof pub);
    for (int i = 0; i < 4; i++) precompile_put_be64(addr_hash + i * 8, out[i]);
    memcpy(addr, addr_hash + 12, 20);
  }
  precompile_recovered_address_lbits(rop, ok, addr);
}
