/* Spike --extlib MMIO accelerator serving the zkvm_accelerators.h API.
 *
 * The guest's zkvm_* functions (zkvm/runtime/accel_guest.c) marshal their
 * arguments per ffi/zkvm_accel_mmio.h and trigger GO; this device -- running
 * on the HOST, not as guest instructions -- reads the guest's input straight
 * from simulator memory (addr_to_mem), dispatches 1:1 into the SAME native
 * zkvm_* implementations (the Rust accel-host) that native builds link
 * directly, and writes the result back to guest memory. So the crypto never
 * enters the guest's retired-instruction (instret) count, exactly as a native
 * zkVM accelerator would behave. */
#include "riscv/abstract_device.h"
#include "riscv/simif.h"
#include "riscv/sim.h"
#include "riscv/dts.h"
#include "fdt/libfdt.h"
extern "C" {
#include "zkvm_accelerators.h"
}
#include "zkvm_accel_mmio.h"
#include <cstring>
#include <cstdint>
#include <sstream>

/* register file (byte offsets within the device's MMIO window; the index
 * layout is fixed by ffi/zkvm_accel_mmio.h) */
enum { R_OP = ZKVM_ACC_R_OP * 8, R_IN = ZKVM_ACC_R_IN * 8,
       R_INLEN = ZKVM_ACC_R_INLEN * 8, R_OUT = ZKVM_ACC_R_OUT * 8,
       R_GO = ZKVM_ACC_R_GO * 8, R_OUTLEN = ZKVM_ACC_R_OUTLEN * 8,
       R_OK = ZKVM_ACC_R_OK * 8, R_NREG = 8 };

class accel_t : public abstract_device_t {
 public:
  accel_t(simif_t* sim) : sim(sim) { std::memset(reg, 0, sizeof(reg)); }

  bool load(reg_t addr, size_t len, uint8_t* bytes) override {
    if (addr + len > sizeof(reg)) return false;
    std::memcpy(bytes, (uint8_t*)reg + addr, len);
    return true;
  }
  bool store(reg_t addr, size_t len, const uint8_t* bytes) override {
    if (addr + len > sizeof(reg)) return false;
    std::memcpy((uint8_t*)reg + addr, bytes, len);
    if (addr == R_GO) compute();
    return true;
  }

 private:
  static uint64_t get_u64(const uint8_t* p) {
    uint64_t w;
    std::memcpy(&w, p, 8);
    return w;
  }

  void compute() {
    uint64_t op = reg[R_OP/8], in = reg[R_IN/8], inlen = reg[R_INLEN/8], out = reg[R_OUT/8];
    char* ip = sim->addr_to_mem(in);
    char* op_ = sim->addr_to_mem(out);
    if (!ip || !op_) { reg[R_OK/8] = 0; reg[R_OUTLEN/8] = 0; return; }
    /* The crypto runs HERE, on the host -- never as guest instructions.
     * Wire layouts per ffi/zkvm_accel_mmio.h; every op is a 1:1 dispatch
     * into the native zkvm_accelerators.h implementation. */
    zkvm_status st = ZKVM_EFAIL;
    uint64_t outlen = 0;
    const uint8_t* ibuf = (const uint8_t*)ip;
    uint8_t* obuf = (uint8_t*)op_;
    bool verified = false;
    switch (op) {
      case ZKVM_ACC_OP_KECCAK256:
        st = zkvm_keccak256(ibuf, inlen, (zkvm_keccak256_hash*)obuf);
        outlen = 32;
        break;
      case ZKVM_ACC_OP_SHA256:
        st = zkvm_sha256(ibuf, inlen, (zkvm_sha256_hash*)obuf);
        outlen = 32;
        break;
      case ZKVM_ACC_OP_RIPEMD160:
        st = zkvm_ripemd160(ibuf, inlen, (zkvm_ripemd160_hash*)obuf);
        outlen = 32;
        break;
      case ZKVM_ACC_OP_SECP256K1_VERIFY:
        if (inlen == 160) {
          st = zkvm_secp256k1_verify((const zkvm_secp256k1_hash*)ibuf,
                                     (const zkvm_secp256k1_signature*)(ibuf + 32),
                                     (const zkvm_secp256k1_pubkey*)(ibuf + 96),
                                     &verified);
          obuf[0] = verified ? 1 : 0;
          outlen = 1;
        }
        break;
      case ZKVM_ACC_OP_SECP256K1_ECRECOVER:
        if (inlen == 97) {
          st = zkvm_secp256k1_ecrecover((const zkvm_secp256k1_hash*)ibuf,
                                        (const zkvm_secp256k1_signature*)(ibuf + 32),
                                        ibuf[96], (zkvm_secp256k1_pubkey*)obuf);
          outlen = 64;
        }
        break;
      case ZKVM_ACC_OP_SECP256R1_VERIFY:
        if (inlen == 160) {
          st = zkvm_secp256r1_verify((const zkvm_secp256r1_hash*)ibuf,
                                     (const zkvm_secp256r1_signature*)(ibuf + 32),
                                     (const zkvm_secp256r1_pubkey*)(ibuf + 96),
                                     &verified);
          obuf[0] = verified ? 1 : 0;
          outlen = 1;
        }
        break;
      case ZKVM_ACC_OP_MODEXP:
        if (inlen >= 24) {
          uint64_t bl = get_u64(ibuf), el = get_u64(ibuf + 8), ml = get_u64(ibuf + 16);
          if (24 + bl + el + ml == inlen) {
            st = zkvm_modexp(ibuf + 24, bl, ibuf + 24 + bl, el,
                             ibuf + 24 + bl + el, ml, obuf);
            outlen = ml;
          }
        }
        break;
      case ZKVM_ACC_OP_BN254_G1_ADD:
        if (inlen == 128) {
          st = zkvm_bn254_g1_add((const zkvm_bn254_g1_point*)ibuf,
                                 (const zkvm_bn254_g1_point*)(ibuf + 64),
                                 (zkvm_bn254_g1_point*)obuf);
          outlen = 64;
        }
        break;
      case ZKVM_ACC_OP_BN254_G1_MUL:
        if (inlen == 96) {
          st = zkvm_bn254_g1_mul((const zkvm_bn254_g1_point*)ibuf,
                                 (const zkvm_bn254_scalar*)(ibuf + 64),
                                 (zkvm_bn254_g1_point*)obuf);
          outlen = 64;
        }
        break;
      case ZKVM_ACC_OP_BN254_PAIRING:
        if (inlen >= 8) {
          uint64_t n = get_u64(ibuf);
          if (8 + n * sizeof(zkvm_bn254_pairing_pair) == inlen) {
            st = zkvm_bn254_pairing((const zkvm_bn254_pairing_pair*)(ibuf + 8), n,
                                    &verified);
            obuf[0] = verified ? 1 : 0;
            outlen = 1;
          }
        }
        break;
      case ZKVM_ACC_OP_BLAKE2F:
        if (inlen == 217) {
          zkvm_blake2f_state h;
          std::memcpy(h.data, ibuf + 8, 64);
          st = zkvm_blake2f((uint32_t)get_u64(ibuf), &h,
                            (const zkvm_blake2f_message*)(ibuf + 72),
                            (const zkvm_blake2f_offset*)(ibuf + 200), ibuf[216]);
          std::memcpy(obuf, h.data, 64);
          outlen = 64;
        }
        break;
      case ZKVM_ACC_OP_KZG_POINT_EVAL:
        if (inlen == 160) {
          st = zkvm_kzg_point_eval((const zkvm_kzg_commitment*)ibuf,
                                   (const zkvm_kzg_field_element*)(ibuf + 48),
                                   (const zkvm_kzg_field_element*)(ibuf + 80),
                                   (const zkvm_kzg_proof*)(ibuf + 112), &verified);
          obuf[0] = verified ? 1 : 0;
          outlen = 1;
        }
        break;
      case ZKVM_ACC_OP_BLS12_G1_ADD:
        if (inlen == 192) {
          st = zkvm_bls12_g1_add((const zkvm_bls12_381_g1_point*)ibuf,
                                 (const zkvm_bls12_381_g1_point*)(ibuf + 96),
                                 (zkvm_bls12_381_g1_point*)obuf);
          outlen = 96;
        }
        break;
      case ZKVM_ACC_OP_BLS12_G1_MSM:
        if (inlen >= 8) {
          uint64_t n = get_u64(ibuf);
          if (8 + n * sizeof(zkvm_bls12_381_g1_msm_pair) == inlen) {
            st = zkvm_bls12_g1_msm((const zkvm_bls12_381_g1_msm_pair*)(ibuf + 8), n,
                                   (zkvm_bls12_381_g1_point*)obuf);
            outlen = 96;
          }
        }
        break;
      case ZKVM_ACC_OP_BLS12_G2_ADD:
        if (inlen == 384) {
          st = zkvm_bls12_g2_add((const zkvm_bls12_381_g2_point*)ibuf,
                                 (const zkvm_bls12_381_g2_point*)(ibuf + 192),
                                 (zkvm_bls12_381_g2_point*)obuf);
          outlen = 192;
        }
        break;
      case ZKVM_ACC_OP_BLS12_G2_MSM:
        if (inlen >= 8) {
          uint64_t n = get_u64(ibuf);
          if (8 + n * sizeof(zkvm_bls12_381_g2_msm_pair) == inlen) {
            st = zkvm_bls12_g2_msm((const zkvm_bls12_381_g2_msm_pair*)(ibuf + 8), n,
                                   (zkvm_bls12_381_g2_point*)obuf);
            outlen = 192;
          }
        }
        break;
      case ZKVM_ACC_OP_BLS12_PAIRING:
        if (inlen >= 8) {
          uint64_t n = get_u64(ibuf);
          if (8 + n * sizeof(zkvm_bls12_381_pairing_pair) == inlen) {
            st = zkvm_bls12_pairing((const zkvm_bls12_381_pairing_pair*)(ibuf + 8), n,
                                    &verified);
            obuf[0] = verified ? 1 : 0;
            outlen = 1;
          }
        }
        break;
      case ZKVM_ACC_OP_BLS12_MAP_FP_TO_G1:
        if (inlen == 48) {
          st = zkvm_bls12_map_fp_to_g1((const zkvm_bls12_381_fp*)ibuf,
                                       (zkvm_bls12_381_g1_point*)obuf);
          outlen = 96;
        }
        break;
      case ZKVM_ACC_OP_BLS12_MAP_FP2_TO_G2:
        if (inlen == 96) {
          st = zkvm_bls12_map_fp2_to_g2((const zkvm_bls12_381_fp2*)ibuf,
                                        (zkvm_bls12_381_g2_point*)obuf);
          outlen = 192;
        }
        break;
      default:
        break;
    }
    reg[R_OUTLEN/8] = (st == ZKVM_EOK) ? outlen : 0;
    reg[R_OK/8] = (st == ZKVM_EOK) ? 1 : 0;
  }
  simif_t* sim;
  uint64_t reg[R_NREG];
};

static accel_t* accel_parse(const void* fdt, const sim_t* sim,
                                      reg_t* base, const std::vector<std::string>& sargs) {
  int node = fdt_node_offset_by_compatible(fdt, -1, "evmsail,accel");
  if (node < 0) return nullptr;
  reg_t b; unsigned long size;
  if (fdt_get_node_addr_size(fdt, node, &b, &size, "reg") != 0) return nullptr;
  *base = b;
  return new accel_t(const_cast<sim_t*>(sim));
}

static std::string accel_generate_dts(const sim_t* sim, const std::vector<std::string>& sargs) {
  reg_t base = 0x40000000;
  std::stringstream s;
  s << std::hex
    << "    accel@" << base << " {\n"
       "      compatible = \"evmsail,accel\";\n"
       "      reg = <0x" << (base >> 32) << " 0x" << (base & 0xffffffffULL)
    << " 0x0 0x1000>;\n"
       "    };\n";
  return s.str();
}

REGISTER_DEVICE(accel, accel_parse, accel_generate_dts)
