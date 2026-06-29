/* Spike --extlib MMIO crypto accelerator.
 *
 * Models a zkVM crypto precompile: the guest writes a request (op, input addr,
 * input len, output addr) to MMIO registers and triggers GO; this device --
 * running on the HOST, not as guest instructions -- reads the guest's input
 * straight from simulator memory (addr_to_mem), computes the hash/precompile,
 * and writes the result back to guest memory. So the crypto never enters the
 * guest's retired-instruction (instret) count, exactly as a native zkVM
 * accelerator would behave. (Smoke-test build: the compute is an echo; the
 * crypto dispatch is wired in the next step.) */
#include "riscv/abstract_device.h"
#include "riscv/simif.h"
#include "riscv/sim.h"
#include "riscv/dts.h"
#include "fdt/libfdt.h"
extern "C" {
#include "zkvm_accelerators.h"
}
#include <cstring>
#include <cstdint>
#include <sstream>

/* register file (byte offsets within the device's MMIO window) */
enum { R_OP = 0x00, R_IN = 0x08, R_INLEN = 0x10, R_OUT = 0x18,
       R_GO = 0x20, R_OUTLEN = 0x28, R_OK = 0x30, R_NREG = 8 };

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
  void compute() {
    uint64_t op = reg[R_OP/8], in = reg[R_IN/8], inlen = reg[R_INLEN/8], out = reg[R_OUT/8];
    char* ip = sim->addr_to_mem(in);
    char* op_ = sim->addr_to_mem(out);
    if (!ip || !op_) { reg[R_OK/8] = 0; reg[R_OUTLEN/8] = 0; return; }
    /* The crypto runs HERE, on the host -- never as guest instructions. */
    int ok = 0; uint32_t outlen = 0;
    const uint8_t* ibuf = (const uint8_t*)ip;
    uint8_t* obuf = (uint8_t*)op_;
    if (op == 0) {        /* keccak256 */
      zkvm_keccak256_hash h;
      if (zkvm_keccak256(ibuf, inlen, &h) == ZKVM_EOK) { std::memcpy(obuf, h.data, 32); outlen = 32; ok = 1; }
    } else if (op == 1) { /* ecrecover: hash[32] v[32] r[32] s[32] -> keccak(pubkey)[12:] */
      ok = 1;             /* "call" always succeeds; empty output on bad recovery */
      if (inlen == 128) {
        uint8_t v = ibuf[63]; int v_ok = (v == 27 || v == 28);
        for (int i = 32; i < 63; i++) if (ibuf[i] != 0) v_ok = 0;
        uint8_t pub[64];
        if (v_ok && zkvm_secp256k1_ecrecover((const zkvm_secp256k1_hash*)ibuf,
                (const zkvm_secp256k1_signature*)(ibuf + 64), (uint8_t)(v - 27),
                (zkvm_secp256k1_pubkey*)pub) == ZKVM_EOK) {
          zkvm_keccak256_hash a; zkvm_keccak256(pub, 64, &a);
          std::memset(obuf, 0, 12); std::memcpy(obuf + 12, a.data + 12, 20); outlen = 32;
        }
      }
    } else if (op == 2) { /* sha256 */
      zkvm_sha256_hash h;
      if (zkvm_sha256(ibuf, inlen, &h) == ZKVM_EOK) { std::memcpy(obuf, h.data, 32); outlen = 32; ok = 1; }
    } else if (op == 257) { /* secp256k1 verify (tx-sender auth): input hash[32] r[32] s[32]
                             * x[32] y[32] (160B) -> 32B 0..01 if it verifies, else empty */
      ok = 1;
      bool verified = false;
      if (inlen == 160 &&
          zkvm_secp256k1_verify((const zkvm_secp256k1_hash*)ibuf,
              (const zkvm_secp256k1_signature*)(ibuf + 32),
              (const zkvm_secp256k1_pubkey*)(ibuf + 96), &verified) == ZKVM_EOK && verified) {
        std::memset(obuf, 0, 32); obuf[31] = 1; outlen = 32;
      }
    }
    reg[R_OUTLEN/8] = outlen;
    reg[R_OK/8] = ok;
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
