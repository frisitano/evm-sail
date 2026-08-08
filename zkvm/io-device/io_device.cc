/* Spike validation device for extractions/c/zkvm_io.h. Private input remains on the host
 * until read_input requests it; complete write_output spans are copied back as
 * the public result. Neither direction is embedded in the guest ELF. */
#include "riscv/abstract_device.h"
#include "riscv/simif.h"
#include "riscv/sim.h"
#include "riscv/dts.h"
#include "fdt/libfdt.h"
#include "io_mmio.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

enum {
  R_INPUT_SIZE = ZKVM_IO_R_INPUT_SIZE * 8,
  R_INPUT_DST = ZKVM_IO_R_INPUT_DST * 8,
  R_INPUT_GO = ZKVM_IO_R_INPUT_GO * 8,
  R_INPUT_OK = ZKVM_IO_R_INPUT_OK * 8,
  R_OUTPUT_SRC = ZKVM_IO_R_OUTPUT_SRC * 8,
  R_OUTPUT_SIZE = ZKVM_IO_R_OUTPUT_SIZE * 8,
  R_OUTPUT_GO = ZKVM_IO_R_OUTPUT_GO * 8,
  R_OUTPUT_OK = ZKVM_IO_R_OUTPUT_OK * 8,
  R_NREG = 8,
};

class evmsail_input_t : public abstract_device_t {
 public:
  evmsail_input_t(simif_t *sim, const std::string& path) : sim(sim) {
    std::memset(reg, 0, sizeof(reg));
    if (!path.empty()) {
      std::ifstream file(path, std::ios::binary);
      if (!file) throw std::runtime_error("cannot open evm-sail input: " + path);
      input.assign(std::istreambuf_iterator<char>(file),
                   std::istreambuf_iterator<char>());
    }
    reg[R_INPUT_SIZE / 8] = input.size();
  }

  bool load(reg_t addr, size_t len, uint8_t *bytes) override {
    if (addr + len > sizeof(reg)) return false;
    std::memcpy(bytes, reinterpret_cast<uint8_t *>(reg) + addr, len);
    return true;
  }

  bool store(reg_t addr, size_t len, const uint8_t *bytes) override {
    if (addr + len > sizeof(reg)) return false;
    std::memcpy(reinterpret_cast<uint8_t *>(reg) + addr, bytes, len);
    if (addr == R_INPUT_GO) copy_input();
    if (addr == R_OUTPUT_GO) publish_output();
    return true;
  }

 private:
  void copy_input() {
    reg[R_INPUT_OK / 8] = 0;
    if (input.empty()) {
      reg[R_INPUT_OK / 8] = 1;
      return;
    }

    uint64_t dst = reg[R_INPUT_DST / 8];
    uint64_t last = dst + input.size() - 1;
    if (last < dst) return;

    /* addr_to_mem only guarantees a contiguous host pointer within the
     * current guest page. Translate each page separately so an input buffer
     * spanning a page boundary is copied into the actual guest pages rather
     * than into unrelated host memory past the first mapping. */
    constexpr uint64_t page_size = 4096;
    size_t copied = 0;
    while (copied < input.size()) {
      uint64_t guest_addr = dst + copied;
      uint64_t page_left = page_size - (guest_addr & (page_size - 1));
      size_t remaining = input.size() - copied;
      size_t chunk = remaining < page_left ? remaining : (size_t)page_left;
      char *guest_ptr = sim->addr_to_mem(guest_addr);
      if (!guest_ptr) return;
      std::memcpy(guest_ptr, input.data() + copied, chunk);
      copied += chunk;
    }
    reg[R_INPUT_OK / 8] = 1;
  }

  bool append_guest_output(uint64_t src, uint64_t size) {
    if (size > std::numeric_limits<size_t>::max() - output.size()) return false;
    if (size != 0 && src > UINT64_MAX - (size - 1)) return false;

    size_t old_size = output.size();
    output.resize(old_size + (size_t)size);
    constexpr uint64_t page_size = 4096;
    uint64_t copied = 0;
    while (copied < size) {
      uint64_t guest_addr = src + copied;
      uint64_t page_left = page_size - (guest_addr & (page_size - 1));
      uint64_t remaining = size - copied;
      size_t chunk = (size_t)(remaining < page_left ? remaining : page_left);
      char *guest_ptr = sim->addr_to_mem(guest_addr);
      if (!guest_ptr) {
        output.resize(old_size);
        return false;
      }
      std::memcpy(output.data() + old_size + (size_t)copied, guest_ptr, chunk);
      copied += chunk;
    }
    return true;
  }

  void publish_output() {
    reg[R_OUTPUT_OK / 8] = 0;
    if (!append_guest_output(reg[R_OUTPUT_SRC / 8], reg[R_OUTPUT_SIZE / 8]))
      return;

    static const char hex[] = "0123456789abcdef";
    std::cout << "output_hex=";
    for (uint8_t byte : output)
      std::cout << hex[byte >> 4] << hex[byte & 0x0f];
    std::cout << std::endl;
    reg[R_OUTPUT_OK / 8] = 1;
  }

  simif_t *sim;
  uint64_t reg[R_NREG];
  std::vector<uint8_t> input;
  std::vector<uint8_t> output;
};

static evmsail_input_t *evmsail_input_parse(
    const void *fdt, const sim_t *sim, reg_t *base,
    const std::vector<std::string>& sargs) {
  int node = fdt_node_offset_by_compatible(fdt, -1, "evmsail,input");
  if (node < 0) return nullptr;
  reg_t device_base;
  unsigned long size;
  if (fdt_get_node_addr_size(fdt, node, &device_base, &size, "reg") != 0)
    return nullptr;
  *base = device_base;
  const std::string path = sargs.empty() ? std::string() : sargs.front();
  return new evmsail_input_t(const_cast<sim_t *>(sim), path);
}

static std::string evmsail_input_generate_dts(
    const sim_t *sim, const std::vector<std::string>& sargs) {
  (void)sim;
  (void)sargs;
  reg_t base = ZKVM_IO_MMIO_BASE;
  std::stringstream dts;
  dts << std::hex
      << "    input@" << base << " {\n"
         "      compatible = \"evmsail,input\";\n"
         "      reg = <0x" << (base >> 32) << " 0x"
      << (base & 0xffffffffULL) << " 0x0 0x1000>;\n"
         "    };\n";
  return dts.str();
}

REGISTER_DEVICE(evmsail_input, evmsail_input_parse, evmsail_input_generate_dts)
