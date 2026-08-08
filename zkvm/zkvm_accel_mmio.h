/* MMIO wire protocol for the zkvm_accelerators.h API.
 *
 * The guest implements every zkvm_* accelerator function by marshalling its
 * arguments into a contiguous staging buffer and issuing one call to the
 * accel device (spike --extlib accel_device.so); the device unmarshals and
 * dispatches 1:1 into the SAME native zkvm_* implementations (the Rust
 * accel-host) that native builds link directly. extractions/c/ code therefore calls
 * the zkvm_accelerators.h API unconditionally -- the MMIO plumbing lives
 * entirely behind it (zkvm/runtime/accel_guest.c on the guest side).
 *
 * Register file (uint64_t each, byte offsets = index * 8):
 *   OP     in : operation code (ZKVM_ACC_OP_*)
 *   IN     in : guest pointer to the marshalled input buffer
 *   INLEN  in : input buffer length in bytes
 *   OUT    in : guest pointer to the output buffer
 *   GO     in : write 1 to execute
 *   OUTLEN out: bytes written to OUT
 *   OK     out: 1 if the native call returned ZKVM_EOK, else 0
 *
 * Input layouts are the raw zkvm_accelerators.h struct bytes concatenated in
 * argument order. Variable-length data (hash inputs, modexp operands, MSM /
 * pairing arrays) is length-prefixed with uint64 fields where noted. bool
 * out-parameters are returned as one output byte (0/1).
 *
 *   KECCAK256 / SHA256 / RIPEMD160 : in = data[INLEN]            out = hash struct (32)
 *   SECP256K1_VERIFY               : in = msg32 sig64 pub64      out = verified[1]
 *   SECP256K1_ECRECOVER            : in = msg32 sig64 recid[1]   out = pubkey[64]
 *   SECP256R1_VERIFY               : in = msg32 sig64 pub64      out = verified[1]
 *   MODEXP    : in = u64 base_len, u64 exp_len, u64 mod_len, base, exp, mod
 *                                                                out = mod_len bytes
 *   BN254_G1_ADD                   : in = p1[64] p2[64]          out = point[64]
 *   BN254_G1_MUL                   : in = point[64] scalar[32]   out = point[64]
 *   BN254_PAIRING                  : in = u64 n, pairs[n*192]    out = verified[1]
 *   BLAKE2F                        : in = u64 rounds, h[64], m[128], t[16], f[1]
 *                                                                out = h[64]
 *   KZG_POINT_EVAL                 : in = c[48] z[32] y[32] proof[48]
 *                                                                out = verified[1]
 *   BLS12_G1_ADD                   : in = p1[96] p2[96]          out = point[96]
 *   BLS12_G1_MSM                   : in = u64 n, pairs[n*128]    out = point[96]
 *   BLS12_G2_ADD                   : in = p1[192] p2[192]        out = point[192]
 *   BLS12_G2_MSM                   : in = u64 n, pairs[n*224]    out = point[192]
 *   BLS12_PAIRING                  : in = u64 n, pairs[n*288]    out = verified[1]
 *   BLS12_MAP_FP_TO_G1             : in = fp[48]                 out = point[96]
 *   BLS12_MAP_FP2_TO_G2            : in = fp2[96]                out = point[192]
 */
#ifndef ZKVM_ACCEL_MMIO_H
#define ZKVM_ACCEL_MMIO_H

#define ZKVM_ACC_MMIO_BASE 0x40000000UL

enum {
  ZKVM_ACC_R_OP = 0,
  ZKVM_ACC_R_IN = 1,
  ZKVM_ACC_R_INLEN = 2,
  ZKVM_ACC_R_OUT = 3,
  ZKVM_ACC_R_GO = 4,
  ZKVM_ACC_R_OUTLEN = 5,
  ZKVM_ACC_R_OK = 6,
};

enum {
  ZKVM_ACC_OP_KECCAK256 = 1,
  ZKVM_ACC_OP_SHA256 = 2,
  ZKVM_ACC_OP_RIPEMD160 = 3,
  ZKVM_ACC_OP_SECP256K1_VERIFY = 4,
  ZKVM_ACC_OP_SECP256K1_ECRECOVER = 5,
  ZKVM_ACC_OP_MODEXP = 6,
  ZKVM_ACC_OP_BN254_G1_ADD = 7,
  ZKVM_ACC_OP_BN254_G1_MUL = 8,
  ZKVM_ACC_OP_BN254_PAIRING = 9,
  ZKVM_ACC_OP_BLAKE2F = 10,
  ZKVM_ACC_OP_KZG_POINT_EVAL = 11,
  ZKVM_ACC_OP_BLS12_G1_ADD = 12,
  ZKVM_ACC_OP_BLS12_G1_MSM = 13,
  ZKVM_ACC_OP_BLS12_G2_ADD = 14,
  ZKVM_ACC_OP_BLS12_G2_MSM = 15,
  ZKVM_ACC_OP_BLS12_PAIRING = 16,
  ZKVM_ACC_OP_BLS12_MAP_FP_TO_G1 = 17,
  ZKVM_ACC_OP_BLS12_MAP_FP2_TO_G2 = 18,
  ZKVM_ACC_OP_SECP256R1_VERIFY = 19,
};

#endif
