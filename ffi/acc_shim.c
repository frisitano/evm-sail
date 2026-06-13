/* See acc_shim.h. Buffers Sail-pushed input and dispatches to the standard
 * zkvm_accelerators.h functions. Self-contained (only memcpy); links into both
 * the native runner and the freestanding riscv guest. */
#include "acc_shim.h"
#include "zkvm_accelerators.h"
#include <string.h>

#define ACC_INMAX  (1u << 21)   /* EEST drives >1.2MB pairing inputs (multi-inf) */
#define ACC_OUTMAX (1u << 17)
static uint8_t  ACC_in[ACC_INMAX];
static uint32_t ACC_inlen;
static uint8_t  ACC_out[ACC_OUTMAX];
static uint32_t ACC_outlen;
static int      ACC_id;
static int      ACC_ok;

/* EVM precompiles zero-pad their input to the field layout; read with padding. */
static uint8_t in_byte(uint32_t i) { return (i < ACC_inlen) ? ACC_in[i] : 0; }
static void in_copy(uint8_t *dst, uint32_t off, uint32_t n) { for (uint32_t i = 0; i < n; i++) dst[i] = in_byte(off + i); }

/* ---- EIP-2537 BLS12-381 marshalling: EVM 64-byte-padded field elems <-> blst
 * native compact (Fp 48B). G1 point = x||y (96B). For a G2 point blst serializes
 * each Fp2 coordinate imaginary-part-first (c1||c0), while the EVM layout is
 * c0||c1; the helpers swap accordingly. A raw Fp2 (MAP_FP2 input) stays c0||c1. */
static uint8_t BLS_scratch[ACC_INMAX];
static int bls_strip_fp(uint8_t *dst48, uint32_t off) {        /* 64B EVM -> 48B; top 16 must be 0 */
  for (uint32_t i = 0; i < 16; i++) if (in_byte(off + i) != 0) return 0;
  in_copy(dst48, off + 16, 48); return 1;
}
static void bls_pad_fp(uint32_t off, const uint8_t *src48) {   /* 48B -> 64B EVM at ACC_out+off */
  memset(ACC_out + off, 0, 16); memcpy(ACC_out + off + 16, src48, 48);
}
static int bls_in_g1(uint8_t *out96, uint32_t off) {           /* EVM G1 128B -> compact 96B */
  return bls_strip_fp(out96, off) && bls_strip_fp(out96 + 48, off + 64);
}
static void bls_out_g1(uint32_t off, const uint8_t *c96) {     /* compact 96B -> EVM 128B */
  bls_pad_fp(off, c96); bls_pad_fp(off + 64, c96 + 48);
}
static int bls_in_g2(uint8_t *out192, uint32_t off) {          /* EVM G2 256B (c0,c1) -> blst 192B (c1,c0) */
  return bls_strip_fp(out192 + 48,  off)         /* x_c0 -> 2nd half of x */
      && bls_strip_fp(out192 + 0,   off + 64)    /* x_c1 -> 1st half of x */
      && bls_strip_fp(out192 + 144, off + 128)   /* y_c0 -> 2nd half of y */
      && bls_strip_fp(out192 + 96,  off + 192);  /* y_c1 -> 1st half of y */
}
static void bls_out_g2(uint32_t off, const uint8_t *b192) {    /* blst 192B (c1,c0) -> EVM 256B (c0,c1) */
  bls_pad_fp(off,       b192 + 48); bls_pad_fp(off + 64,  b192 + 0);
  bls_pad_fp(off + 128, b192 + 144); bls_pad_fp(off + 192, b192 + 96);
}

unit acc_begin(uint64_t id) { ACC_id = (int)id; ACC_inlen = 0; ACC_outlen = 0; ACC_ok = 1; return UNIT; }
/* begin + bulk-load the input from EVM memory [off, off+len) -- one memcpy,
 * replacing a per-byte stream of an intermediate Sail list */
unit acc_begin_mem(uint64_t id, uint64_t off, uint64_t len) {
  acc_begin(id);
  if (len > ACC_INMAX) len = ACC_INMAX;
  if (len) {
    const uint8_t *p = hm_rd(off, len);
    memcpy(ACC_in, p, len);
  }
  ACC_inlen = (uint32_t)len;
  return UNIT;
}
/* begin + load the input from the code store (EXTCODEHASH keccak input) */
extern const uint8_t *cs_bytes(uint64_t a2, uint64_t a1, uint64_t a0, uint64_t *len);
unit acc_begin_cs(uint64_t id, uint64_t a2, uint64_t a1, uint64_t a0) {
  acc_begin(id);
  uint64_t len; const uint8_t *p = cs_bytes(a2, a1, a0, &len);
  if (len > ACC_INMAX) len = ACC_INMAX;
  if (len) memcpy(ACC_in, p, len);
  ACC_inlen = (uint32_t)len;
  return UNIT;
}
unit acc_push(uint64_t b)   { if (ACC_inlen < ACC_INMAX) ACC_in[ACC_inlen++] = (uint8_t)(b & 0xff); return UNIT; }
/* staged-input byte (zero past the end): precompile gas inspection (modexp
 * length header / exponent head, blake2f rounds) without a Sail byte list */
uint64_t acc_in_byte(uint64_t i) { return (i < ACC_inlen) ? ACC_in[i] : 0; }
unit acc_push8(uint64_t w)  {   /* 8 input bytes (big-endian) in one store when room */
  if (ACC_inlen + 8 <= ACC_INMAX) { for (int i = 0; i < 8; i++) ACC_in[ACC_inlen + i] = (uint8_t)(w >> (8 * (7 - i))); ACC_inlen += 8; }
  else { for (int i = 0; i < 8; i++) acc_push((w >> (8 * (7 - i))) & 0xff); }
  return UNIT;
}

uint64_t acc_exec(unit u) {
  (void)u;
  switch (ACC_id) {
    case 0: { zkvm_keccak256_hash h;
      if (zkvm_keccak256(ACC_in, ACC_inlen, &h) == ZKVM_EOK) { memcpy(ACC_out, h.data, 32); ACC_outlen = 32; }
      else ACC_ok = 0; break; }
    case 2: { zkvm_sha256_hash h;
      if (zkvm_sha256(ACC_in, ACC_inlen, &h) == ZKVM_EOK) { memcpy(ACC_out, h.data, 32); ACC_outlen = 32; }
      else ACC_ok = 0; break; }
    case 3: { zkvm_ripemd160_hash h;
      if (zkvm_ripemd160(ACC_in, ACC_inlen, &h) == ZKVM_EOK) { memcpy(ACC_out, h.data, 32); ACC_outlen = 32; }
      else ACC_ok = 0; break; }
    case 4: memcpy(ACC_out, ACC_in, ACC_inlen); ACC_outlen = ACC_inlen; break;  /* identity (guest-side) */
    case 1: {  /* ecrecover (0x01): input hash[32] v[32] r[32] s[32]; output = keccak(pubkey)[12:] */
      uint8_t hash[32], sig[64], pub[64];
      in_copy(hash, 0, 32); in_copy(sig, 64, 64);
      uint8_t v = in_byte(63); int v_ok = (v == 27 || v == 28);
      for (uint32_t i = 32; i < 63; i++) if (in_byte(i) != 0) v_ok = 0;   /* upper v bytes must be zero */
      ACC_ok = 1; ACC_outlen = 0;   /* EVM ecrecover always "succeeds"; empty output on bad recovery */
      if (v_ok && zkvm_secp256k1_ecrecover((const zkvm_secp256k1_hash*)hash,
              (const zkvm_secp256k1_signature*)sig, (uint8_t)(v - 27), (zkvm_secp256k1_pubkey*)pub) == ZKVM_EOK) {
        zkvm_keccak256_hash a; zkvm_keccak256(pub, 64, &a);
        memset(ACC_out, 0, 12); memcpy(ACC_out + 12, a.data + 12, 20); ACC_outlen = 32;
      }
      break;
    }
    case 6: {  /* bn254 G1 add (0x06): input p1[64] p2[64] -> p[64] */
      uint8_t in[128]; in_copy(in, 0, 128);
      if (zkvm_bn254_g1_add((const zkvm_bn254_g1_point*)in, (const zkvm_bn254_g1_point*)(in + 64),
              (zkvm_bn254_g1_point*)ACC_out) == ZKVM_EOK) ACC_outlen = 64;
      else { ACC_ok = 0; ACC_outlen = 0; }
      break;
    }
    case 7: {  /* bn254 G1 mul (0x07): input point[64] scalar[32] -> p[64] */
      uint8_t in[96]; in_copy(in, 0, 96);
      if (zkvm_bn254_g1_mul((const zkvm_bn254_g1_point*)in, (const zkvm_bn254_scalar*)(in + 64),
              (zkvm_bn254_g1_point*)ACC_out) == ZKVM_EOK) ACC_outlen = 64;
      else { ACC_ok = 0; ACC_outlen = 0; }
      break;
    }
    case 5: {  /* modexp (0x05): bsize[32] esize[32] msize[32] base exp mod -> mod_len bytes */
      uint32_t bl = 0, el = 0, ml = 0;
      for (int i = 0; i < 4; i++) { bl = (bl << 8) | in_byte(28 + i); el = (el << 8) | in_byte(60 + i); ml = (ml << 8) | in_byte(92 + i); }
      if (ml == 0) { ACC_ok = 1; ACC_outlen = 0; break; }
      uint64_t need = (uint64_t)96 + bl + el + ml;
      if (need > ACC_INMAX || ml > ACC_OUTMAX) { ACC_ok = 0; ACC_outlen = 0; break; }
      for (uint32_t i = ACC_inlen; i < need; i++) ACC_in[i] = 0;   /* zero-pad to layout length */
      if (zkvm_modexp(ACC_in + 96, bl, ACC_in + 96 + bl, el, ACC_in + 96 + bl + el, ml, ACC_out) == ZKVM_EOK)
        ACC_outlen = ml;
      else { ACC_ok = 0; ACC_outlen = 0; }
      break;
    }
    case 8: {  /* bn254 pairing (0x08): k*(g1[64] g2[128]) -> 32B (0..0||verified) */
      if (ACC_inlen % 192 != 0) { ACC_ok = 0; ACC_outlen = 0; break; }
      bool verified = false;
      if (zkvm_bn254_pairing((const zkvm_bn254_pairing_pair*)ACC_in, (size_t)(ACC_inlen / 192), &verified) == ZKVM_EOK) {
        memset(ACC_out, 0, 32); ACC_out[31] = verified ? 1 : 0; ACC_outlen = 32;
      } else { ACC_ok = 0; ACC_outlen = 0; }
      break;
    }
    case 9: {  /* blake2f (0x09): rounds[4] h[64] m[128] t[16] f[1] (exactly 213B) -> h[64] */
      if (ACC_inlen != 213) { ACC_ok = 0; ACC_outlen = 0; break; }
      uint8_t f = in_byte(212);
      if (f != 0 && f != 1) { ACC_ok = 0; ACC_outlen = 0; break; }
      uint32_t rounds = ((uint32_t)in_byte(0) << 24) | ((uint32_t)in_byte(1) << 16) | ((uint32_t)in_byte(2) << 8) | in_byte(3);
      uint8_t hbuf[64], m[128], t[16];
      in_copy(hbuf, 4, 64); in_copy(m, 68, 128); in_copy(t, 196, 16);
      if (zkvm_blake2f(rounds, (zkvm_blake2f_state*)hbuf, (const zkvm_blake2f_message*)m, (const zkvm_blake2f_offset*)t, f) == ZKVM_EOK) {
        memcpy(ACC_out, hbuf, 64); ACC_outlen = 64;
      } else { ACC_ok = 0; ACC_outlen = 0; }
      break;
    }
    case 10: {  /* KZG point eval (0x0a): vhash[32] z[32] y[32] commitment[48] proof[48] (192B) */
      if (ACC_inlen != 192) { ACC_ok = 0; ACC_outlen = 0; break; }
      uint8_t comm[48], z[32], y[32], proof[48], vh[32];
      in_copy(vh, 0, 32); in_copy(z, 32, 32); in_copy(y, 64, 32); in_copy(comm, 96, 48); in_copy(proof, 144, 48);
      zkvm_sha256_hash sh; zkvm_sha256(comm, 48, &sh);   /* versioned_hash = 0x01 || sha256(commitment)[1:] */
      int vh_ok = (vh[0] == 0x01);
      for (int i = 1; i < 32; i++) if (vh[i] != sh.data[i]) vh_ok = 0;
      bool verified = false;
      if (vh_ok && zkvm_kzg_point_eval((const zkvm_kzg_commitment*)comm, (const zkvm_kzg_field_element*)z,
              (const zkvm_kzg_field_element*)y, (const zkvm_kzg_proof*)proof, &verified) == ZKVM_EOK && verified) {
        static const uint8_t BLS_MOD[32] = {  /* BLS_MODULUS */
          0x73,0xed,0xa7,0x53,0x29,0x9d,0x7d,0x48,0x33,0x39,0xd8,0x08,0x09,0xa1,0xd8,0x05,
          0x53,0xbd,0xa4,0x02,0xff,0xfe,0x5b,0xfe,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x01 };
        memset(ACC_out, 0, 64);
        ACC_out[30] = 0x10;                 /* FIELD_ELEMENTS_PER_BLOB = 4096 = 0x1000 */
        memcpy(ACC_out + 32, BLS_MOD, 32);
        ACC_outlen = 64;
      } else { ACC_ok = 0; ACC_outlen = 0; }
      break;
    }
    case 11: {  /* BLS12_G1ADD probe */
      if (ACC_inlen != 256) { ACC_ok = 0; ACC_outlen = 0; break; }
      uint8_t c[192], out[96];
      if (!bls_in_g1(c, 0) || !bls_in_g1(c + 96, 128)) { ACC_ok = 0; ACC_outlen = 0; break; }
      if (zkvm_bls12_g1_add((const zkvm_bls12_381_g1_point*)c, (const zkvm_bls12_381_g1_point*)(c + 96), (zkvm_bls12_381_g1_point*)out) == ZKVM_EOK) { bls_out_g1(0, out); ACC_outlen = 128; }
      else { ACC_ok = 0; ACC_outlen = 0; }
      break;
    }
    case 12: {  /* BLS12_G1MSM (0x0c): k*(G1[128] scalar[32]) -> G1[128] */
      if (ACC_inlen == 0 || ACC_inlen % 160 != 0) { ACC_ok = 0; ACC_outlen = 0; break; }
      uint32_t k = ACC_inlen / 160; int ok = 1;
      for (uint32_t i = 0; i < k; i++) {
        if (!bls_in_g1(BLS_scratch + i * 128, i * 160)) { ok = 0; break; }
        in_copy(BLS_scratch + i * 128 + 96, i * 160 + 128, 32);
      }
      uint8_t out[96];
      if (ok && zkvm_bls12_g1_msm((const zkvm_bls12_381_g1_msm_pair*)BLS_scratch, k, (zkvm_bls12_381_g1_point*)out) == ZKVM_EOK) { bls_out_g1(0, out); ACC_outlen = 128; }
      else { ACC_ok = 0; ACC_outlen = 0; }
      break;
    }
    case 13: {  /* BLS12_G2ADD (0x0d): G2[256] G2[256] -> G2[256] */
      if (ACC_inlen != 512) { ACC_ok = 0; ACC_outlen = 0; break; }
      uint8_t c[384], out[192];
      if (!bls_in_g2(c, 0) || !bls_in_g2(c + 192, 256)) { ACC_ok = 0; ACC_outlen = 0; break; }
      if (zkvm_bls12_g2_add((const zkvm_bls12_381_g2_point*)c, (const zkvm_bls12_381_g2_point*)(c + 192), (zkvm_bls12_381_g2_point*)out) == ZKVM_EOK) { bls_out_g2(0, out); ACC_outlen = 256; }
      else { ACC_ok = 0; ACC_outlen = 0; }
      break;
    }
    case 14: {  /* BLS12_G2MSM (0x0e): k*(G2[256] scalar[32]) -> G2[256] */
      if (ACC_inlen == 0 || ACC_inlen % 288 != 0) { ACC_ok = 0; ACC_outlen = 0; break; }
      uint32_t k = ACC_inlen / 288; int ok = 1;
      for (uint32_t i = 0; i < k; i++) {
        if (!bls_in_g2(BLS_scratch + i * 224, i * 288)) { ok = 0; break; }
        in_copy(BLS_scratch + i * 224 + 192, i * 288 + 256, 32);
      }
      uint8_t out[192];
      if (ok && zkvm_bls12_g2_msm((const zkvm_bls12_381_g2_msm_pair*)BLS_scratch, k, (zkvm_bls12_381_g2_point*)out) == ZKVM_EOK) { bls_out_g2(0, out); ACC_outlen = 256; }
      else { ACC_ok = 0; ACC_outlen = 0; }
      break;
    }
    case 15: {  /* BLS12_PAIRING_CHECK (0x0f): k*(G1[128] G2[256]) -> 32B (0..0||result) */
      if (ACC_inlen == 0 || ACC_inlen % 384 != 0) { ACC_ok = 0; ACC_outlen = 0; break; }
      uint32_t k = ACC_inlen / 384; int ok = 1;
      for (uint32_t i = 0; i < k; i++) {
        if (!bls_in_g1(BLS_scratch + i * 288, i * 384) ||
            !bls_in_g2(BLS_scratch + i * 288 + 96, i * 384 + 128)) { ok = 0; break; }
      }
      bool verified = false;
      if (ok && zkvm_bls12_pairing((const zkvm_bls12_381_pairing_pair*)BLS_scratch, k, &verified) == ZKVM_EOK) {
        memset(ACC_out, 0, 32); ACC_out[31] = verified ? 1 : 0; ACC_outlen = 32;
      } else { ACC_ok = 0; ACC_outlen = 0; }
      break;
    }
    case 16: {  /* BLS12_MAP_FP_TO_G1 (0x10): Fp[64] -> G1[128] */
      if (ACC_inlen != 64) { ACC_ok = 0; ACC_outlen = 0; break; }
      uint8_t fp[48], out[96];
      if (!bls_strip_fp(fp, 0)) { ACC_ok = 0; ACC_outlen = 0; break; }
      if (zkvm_bls12_map_fp_to_g1((const zkvm_bls12_381_fp*)fp, (zkvm_bls12_381_g1_point*)out) == ZKVM_EOK) { bls_out_g1(0, out); ACC_outlen = 128; }
      else { ACC_ok = 0; ACC_outlen = 0; }
      break;
    }
    case 17: {  /* BLS12_MAP_FP2_TO_G2 (0x11): Fp2[128] (c0,c1) -> G2[256] */
      if (ACC_inlen != 128) { ACC_ok = 0; ACC_outlen = 0; break; }
      uint8_t fp2[96], out[192];
      if (!bls_strip_fp(fp2, 0) || !bls_strip_fp(fp2 + 48, 64)) { ACC_ok = 0; ACC_outlen = 0; break; }
      if (zkvm_bls12_map_fp2_to_g2((const zkvm_bls12_381_fp2*)fp2, (zkvm_bls12_381_g2_point*)out) == ZKVM_EOK) { bls_out_g2(0, out); ACC_outlen = 256; }
      else { ACC_ok = 0; ACC_outlen = 0; }
      break;
    }
    case 256: {  /* P256VERIFY (EIP-7951, 0x100): hash[32] r[32] s[32] x[32] y[32] (160B)
                  * -> 32B 0..01 if the secp256r1 signature verifies, else EMPTY. The CALL
                  * itself always succeeds (like ecrecover); failure = empty returndata. */
      ACC_ok = 1; ACC_outlen = 0;
      if (ACC_inlen == 160) {
        uint8_t h[32], sig[64], pk[64]; bool verified = false;
        in_copy(h, 0, 32); in_copy(sig, 32, 64); in_copy(pk, 96, 64);
        if (zkvm_secp256r1_verify((const zkvm_secp256r1_hash*)h, (const zkvm_secp256r1_signature*)sig,
                (const zkvm_secp256r1_pubkey*)pk, &verified) == ZKVM_EOK && verified) {
          memset(ACC_out, 0, 32); ACC_out[31] = 1; ACC_outlen = 32;
        }
      }
      break;
    }
    default: ACC_ok = 0; ACC_outlen = 0; break;
  }
  return ACC_outlen;
}
uint64_t acc_ok(unit u)  { (void)u; return (uint64_t)ACC_ok; }
uint64_t acc_out(uint64_t i) { return (i < ACC_outlen) ? ACC_out[i] : 0; }
uint64_t acc_word(uint64_t i) {   /* big-endian 64-bit word i of the output (4 reads per 32-byte digest) */
  uint64_t w = 0;
  for (int k = 0; k < 8; k++) { uint64_t j = i * 8 + k; w = (w << 8) | ((j < ACC_outlen) ? ACC_out[j] : 0); }
  return w;
}

/* ==================== RETURNDATA (C-backed, per frame) ==================== */
/* Each call frame has a returndata slot (what its LAST sub-call returned);
 * a child's RETURN/REVERT captures its payload into one PENDING buffer that
 * the parent ADOPTS (a pointer swap) after the child frame is torn down.
 * Adoption consumes the pending buffer, so an exceptionally-halted child
 * (which never captures) correctly yields empty returndata. */
#define HR_MAXDEPTH 1100
typedef struct { uint8_t *p; uint64_t cap, len; } hr_buf;
static hr_buf hr_rd[HR_MAXDEPTH];
static hr_buf hr_pend;

static void hr_fit(hr_buf *b, uint64_t need) {
  if (b->cap < need) {
    uint64_t n = b->cap ? b->cap : 256;
    while (n < need) n <<= 1;
    b->p = (uint8_t *)realloc(b->p, n);
    b->cap = n;
  }
}

unit hr_reset(const unit u) {           /* per tx: all slots + pending empty */
  (void)u;
  for (int i = 0; i < HR_MAXDEPTH; i++) hr_rd[i].len = 0;
  hr_pend.len = 0;
  return UNIT;
}
unit hr_clear(const unit u) { (void)u; hr_rd[hm_depth(UNIT)].len = 0; return UNIT; }
unit hr_discard(const unit u) { (void)u; hr_pend.len = 0; return UNIT; }

/* RETURN/REVERT: pending := the child's memory[off, off+len) */
unit hr_capture(uint64_t off, uint64_t len) {
  hr_fit(&hr_pend, len ? len : 1);
  if (len) {
    const uint8_t *p = hm_rd(off, len);
    memcpy(hr_pend.p, p, len);
  }
  hr_pend.len = len;
  return UNIT;
}
/* precompile output: pending := ACC_out */
unit hr_capture_acc(const unit u) {
  (void)u;
  hr_fit(&hr_pend, ACC_outlen ? ACC_outlen : 1);
  memcpy(hr_pend.p, ACC_out, ACC_outlen);
  hr_pend.len = ACC_outlen;
  return UNIT;
}
/* parent adopts the pending output as its returndata (pointer swap; consumes) */
unit hr_adopt(const unit u) {
  (void)u;
  hr_buf *slot = &hr_rd[hm_depth(UNIT)];
  hr_buf tmp = *slot;
  *slot = hr_pend;
  hr_pend = tmp;
  hr_pend.len = 0;
  return UNIT;
}
uint64_t hr_len(const unit u) { (void)u; return hr_rd[hm_depth(UNIT)].len; }
uint64_t hr_pending_len(const unit u) { (void)u; return hr_pend.len; }
uint64_t hr_pending_byte(uint64_t i) { return i < hr_pend.len ? hr_pend.p[i] : 0; }

/* RETURNDATACOPY (the Sail side bounds-checks off+len <= hr_len first) */
unit hr_to_mem(uint64_t dst, uint64_t off, uint64_t len) {
  hr_buf *slot = &hr_rd[hm_depth(UNIT)];
  if (!len || off + len > slot->len) return UNIT;
  uint8_t *d = hm_wr(dst, len);
  if (d) memcpy(d, slot->p + off, len);
  return UNIT;
}
/* post-call output region: memory[dst ..) := returndata[0 .. min(want, len)) */
unit hr_out_region(uint64_t dst, uint64_t want) {
  hr_buf *slot = &hr_rd[hm_depth(UNIT)];
  uint64_t n = want < slot->len ? want : slot->len;
  if (n) {
    uint8_t *d = hm_wr(dst, n);
    if (d) memcpy(d, slot->p, n);
  }
  return UNIT;
}
