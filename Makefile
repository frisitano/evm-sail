# ===========================================================================
# evm-sail — validation + execution entry point
#
#   make check          type-check the specification (evm/evm.sail)
#   make check-example  type-check the runnable block example
#   make run-example    compile the example to C and EXECUTE a block
#   make lint            sail --all-warnings + source hygiene (trailing ws/tabs/newline)
#   make fmt             format *.sail with the official `sail --fmt` (opt-in)
#   make all            check + check-example + lint + fmt-check
#   make clean          remove build artifacts
#
# Requires the rems-project Sail toolchain (`sail`) on PATH, installed via
# opam (NOT `brew install sail`, which is an unrelated tool). See README.md.
# `run-example` additionally needs a C compiler and libgmp.
# ===========================================================================

SAIL ?= sail

MODEL   := evm/evm.sail
EXAMPLE := examples/run_block.sail

# Sail runtime library (ships with the toolchain) and its C sources.
SAIL_LIB := $(shell dirname $(shell command -v $(SAIL)))/../share/sail/lib
SAIL_CSRC := $(SAIL_LIB)/sail.c $(SAIL_LIB)/rts.c $(SAIL_LIB)/sail_failure.c \
             $(SAIL_LIB)/sail_config.c $(SAIL_LIB)/elf.c $(SAIL_LIB)/cJSON.c

# libgmp location (Homebrew on macOS; adjust for Linux: usually already on path).
GMP_PREFIX ?= /opt/homebrew
CFLAGS_GMP := -I$(GMP_PREFIX)/include -L$(GMP_PREFIX)/lib

# Keccak-256 C accelerator (FFI). The model declares keccak_reset/absorb/word as
# externs (evm/keccak.sail); native builds link this fast C implementation, the
# eth-act zkVM c-interface-accelerator boundary. --c-include injects the decls
# into the Sail-generated C so the call sites compile.
# zkvm-standards crypto accelerators: the standard reference impl + the Sail
# marshalling shim. One boundary for keccak/sha256/ripemd/precompiles.
ACC_FFI := ffi/zkvm_accelerators.c ffi/acc_shim.c ffi/host_mem.c ffi/host_map.c ffi/host_stack.c ffi/host_word.c ffi/host_code.c
SAIL_CFLAGS := --c-include acc_shim.h
CFLAGS_FFI  := -Iffi

.PHONY: all check check-example run-example clean help lint fmt fmt-check

help:
	@echo "evm-sail targets:"
	@echo "  make check          - type-check the model ($(MODEL))"
	@echo "  make check-example  - type-check the block example ($(EXAMPLE))"
	@echo "  make run-example    - compile to C and execute a block"
	@echo "  make lint           - sail --all-warnings + source hygiene"
	@echo "  make fmt            - format every *.sail with sail --fmt (opinionated; opt-in)"
	@echo "  make fmt-check      - verify *.sail match sail --fmt"
	@echo "  make all            - check + check-example + lint + fmt-check"

check:
	$(SAIL) $(MODEL)

check-example:
	$(SAIL) $(EXAMPLE)

lint:
	@SAIL=$(SAIL) bash scripts/lint-sail.sh

fmt:
	@SAIL=$(SAIL) bash scripts/fmt-sail.sh

fmt-check:
	@SAIL=$(SAIL) bash scripts/fmt-sail.sh --check

all: check check-example lint fmt-check

# Compile the example to C against the Sail runtime, then run it. Executes a
# one-transaction block and prints the resulting state / gas / receipt.
run-example:
	$(SAIL) -c $(SAIL_CFLAGS) $(EXAMPLE) -o build_run_block
	cc -O1 -I$(SAIL_LIB) $(CFLAGS_FFI) $(CFLAGS_GMP) build_run_block.c $(SAIL_CSRC) $(ACC_FFI) -lgmp -o build_run_block
	./build_run_block

# Compile + run ANY example file:  make run EX=examples/syscall_demo.sail
EX ?= $(EXAMPLE)
run:
	$(SAIL) -c $(SAIL_CFLAGS) $(EX) -o build_ex
	cc -O1 -I$(SAIL_LIB) $(CFLAGS_FFI) $(CFLAGS_GMP) build_ex.c $(SAIL_CSRC) $(ACC_FFI) -lgmp -o build_ex
	@echo "--- output ---"
	@./build_ex
	@rm -f build_ex build_ex.c build_ex.h

# Run the block fixture suite (8 blocks, asserts expected state/gas/receipts).
fixtures:
	@$(MAKE) run EX=examples/fixtures.sail

clean:
	rm -f build_run_block build_run_block.c build_run_block.h build_ex build_ex.c build_ex.h
	rm -rf sail_smt_cache evm/sail_smt_cache examples/sail_smt_cache
