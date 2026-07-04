#include "precompiles.h"
#include "host_crypto.h"
#include "memory.h"
#include "returndata.h"
#include "zkvm_accelerators.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define PRE_INMAX  (1u << 21)   /* EEST drives >1.2MB pairing inputs (multi-inf) */
#define PRE_OUTMAX (1u << 17)

static uint8_t  PRE_in[PRE_INMAX];
static uint32_t PRE_inlen;
static uint8_t  PRE_scratch[PRE_OUTMAX];
static uint8_t *PRE_out = PRE_scratch;
static uint32_t PRE_outcap = PRE_OUTMAX;
static uint32_t PRE_outlen;
static int      PRE_id;
static int      PRE_ok;
static const uint8_t *PRE_src = PRE_in;

static void precompile_output_scratch(void) {
  PRE_out = PRE_scratch;
  PRE_outcap = PRE_OUTMAX;
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
  PRE_src = PRE_in;
  precompile_output_scratch();
}

static void precompile_set_input(uint64_t id, const uint8_t *src, uint64_t len) {
  precompile_begin(id);
  if (len > PRE_INMAX) len = PRE_INMAX;
  if (len && src) {
    memcpy(PRE_in, src, (size_t)len);
    PRE_inlen = (uint32_t)len;
  } else {
    PRE_inlen = 0;
  }
}

#ifdef ACCEL_MMIO
static const uintptr_t PRECOMPILE_MMIO_BASE = 0x40000000UL;
enum {
  PRE_R_OP = 0,
  PRE_R_IN = 1,
  PRE_R_INLEN = 2,
  PRE_R_OUT = 3,
  PRE_R_GO = 4,
  PRE_R_OUTLEN = 5,
  PRE_R_OK = 6,
};

static uint64_t precompile_device_call(uint64_t op, const uint8_t *in, uint32_t inlen, uint8_t *out) {
  volatile uint64_t *d = (volatile uint64_t *)PRECOMPILE_MMIO_BASE;
  d[PRE_R_OP] = op;
  d[PRE_R_IN] = (uint64_t)(uintptr_t)in;
  d[PRE_R_INLEN] = inlen;
  d[PRE_R_OUT] = (uint64_t)(uintptr_t)out;
  d[PRE_R_GO] = 1;
  PRE_ok = (int)d[PRE_R_OK];
  PRE_outlen = (uint32_t)d[PRE_R_OUTLEN];
  return PRE_outlen;
}
#endif

/* ---- EIP-2537 BLS12-381 marshalling: EVM 64-byte-padded field elems <-> blst
 * native compact (Fp 48B). G1 point = x||y (96B). For a G2 point blst serializes
 * each Fp2 coordinate imaginary-part-first (c1||c0), while the EVM layout is
 * c0||c1; the helpers swap accordingly. A raw Fp2 (MAP_FP2 input) stays c0||c1. */
static uint8_t BLS_scratch[PRE_INMAX];

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

unit precompile_stage_memory(uint64_t id, uint64_t off, uint64_t len) {
  precompile_set_input(id, len ? hm_rd(off, len) : NULL, len);
  return UNIT;
}

unit precompile_stage_tx(uint64_t id) {
  precompile_begin(id);
  uint64_t len = txd_length(UNIT);
  if (len > PRE_INMAX) len = PRE_INMAX;
  if (len) txd_copy(PRE_in, len);
  PRE_inlen = (uint32_t)len;
  return UNIT;
}

uint64_t precompile_input_byte(uint64_t i) {
  return (i < PRE_inlen) ? PRE_src[i] : 0;
}

static uint64_t precompile_run_current(void) {
#ifdef ACCEL_MMIO
  return precompile_device_call(PRE_id, PRE_src, PRE_inlen, PRE_out);
#else
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
      for (uint32_t i = PRE_inlen; i < need; i++) PRE_in[i] = 0;
      if (zkvm_modexp(PRE_in + 96, bl, PRE_in + 96 + bl, el, PRE_in + 96 + bl + el, ml, PRE_out) == ZKVM_EOK)
        PRE_outlen = ml;
      else { PRE_ok = 0; PRE_outlen = 0; }
      break;
    }
    case 8: {
      if (PRE_inlen % 192 != 0) { PRE_ok = 0; PRE_outlen = 0; break; }
      bool verified = false;
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
      int ok = 1;
      for (uint32_t i = 0; i < k; i++) {
        if (!bls_in_g1(BLS_scratch + i * 128, i * 160)) { ok = 0; break; }
        precompile_copy(BLS_scratch + i * 128 + 96, i * 160 + 128, 32);
      }
      uint8_t out[96];
      if (ok && zkvm_bls12_g1_msm((const zkvm_bls12_381_g1_msm_pair*)BLS_scratch, k,
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
      int ok = 1;
      for (uint32_t i = 0; i < k; i++) {
        if (!bls_in_g2(BLS_scratch + i * 224, i * 288)) { ok = 0; break; }
        precompile_copy(BLS_scratch + i * 224 + 192, i * 288 + 256, 32);
      }
      uint8_t out[192];
      if (ok && zkvm_bls12_g2_msm((const zkvm_bls12_381_g2_msm_pair*)BLS_scratch, k,
              (zkvm_bls12_381_g2_point*)out) == ZKVM_EOK) { bls_out_g2(0, out); PRE_outlen = 256; }
      else { PRE_ok = 0; PRE_outlen = 0; }
      break;
    }
    case 15: {
      if (PRE_inlen == 0 || PRE_inlen % 384 != 0) { PRE_ok = 0; PRE_outlen = 0; break; }
      uint32_t k = PRE_inlen / 384;
      int ok = 1;
      for (uint32_t i = 0; i < k; i++) {
        if (!bls_in_g1(BLS_scratch + i * 288, i * 384) ||
            !bls_in_g2(BLS_scratch + i * 288 + 96, i * 384 + 128)) { ok = 0; break; }
      }
      bool verified = false;
      if (ok && zkvm_bls12_pairing((const zkvm_bls12_381_pairing_pair*)BLS_scratch, k, &verified) == ZKVM_EOK) {
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
#endif
}

uint64_t precompile_run_staged_to_returndata(const unit u) {
  (void)u;
  precompile_output_pending();
  uint64_t n = precompile_run_current();
  returndata_set_pending_len(PRE_ok ? PRE_outlen : 0);
  precompile_output_scratch();
  return n;
}

uint64_t precompile_run_host_input(uint64_t id) {
  uint64_t len = 0;
  const uint8_t *p = host_bytes_data(&len);
  precompile_set_input(id, p, len);
  precompile_output_scratch();
  return precompile_run_current();
}

uint64_t precompile_ok(const unit u) {
  (void)u;
  return (uint64_t)PRE_ok;
}

uint64_t precompile_out(uint64_t i) {
  return (i < PRE_outlen) ? PRE_out[i] : 0;
}
