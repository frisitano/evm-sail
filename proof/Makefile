SAIL ?= sail
Z3 ?= z3
CARGO ?= cargo
PYTHON3 ?= python3
OBJDUMP ?= riscv64-unknown-elf-objdump
ISLA_REPO ?= /Users/f/dev/ethereum/isla
ISLARIS_REPO ?= /Users/f/dev/ethereum/islaris
ERE_GUESTS_REPO ?= /Users/f/dev/ethereum/ere-guests
ISLA_SAIL ?= $(ISLA_REPO)/isla-sail/isla-sail
ZISK_TOOLCHAIN ?= zisk
RUST_TARGET ?= riscv64ima-zisk-zkvm-elf
RISCV_TARGET_DIR ?= /tmp/evm-sail-proof-riscv-target
PROOF_BUILD_DIR ?= /tmp/evm-sail-proof-lowering
ISLA_CARGO_TARGET_DIR ?= /tmp/evm-sail-isla-target
ISLA_IR_DIR ?= $(PROOF_BUILD_DIR)/isla-ir
ISLARIS_DUMP_DIR ?= $(PROOF_BUILD_DIR)/islaris-dump
ISLARIS_COQ_DIR ?= $(PROOF_BUILD_DIR)/islaris-coq
ISLARIS_STACK_SMOKE_COQ_DIR ?= $(PROOF_BUILD_DIR)/stack-store-smoke-coq
SAIL_COQ_DIR ?= $(PROOF_BUILD_DIR)/sail-coq
EVM_SAIL_COQ_CORE_DIR ?= $(PROOF_BUILD_DIR)/evm-sail-coq-core
EVM_SAIL_COQ_GUEST_DIR ?= $(PROOF_BUILD_DIR)/evm-sail-coq-guest
EVM_SAIL_ZKVM_DIR ?= $(CURDIR)/../zkvm
EVM_SAIL_ZKVM_ELF ?= $(EVM_SAIL_ZKVM_DIR)/build/zkvm_guest.elf
EVM_SAIL_ZKVM_ISLARIS_DIR ?= $(PROOF_BUILD_DIR)/evm-sail-zkvm-islaris
EVM_SAIL_ZKVM_DUMP := $(EVM_SAIL_ZKVM_ISLARIS_DIR)/elf.dump
EVM_SAIL_ZKVM_FULL_COQ_DIR ?= $(PROOF_BUILD_DIR)/evm-sail-zkvm-full-coq
EVM_SAIL_ZKVM_COQDIR ?= isla.instructions.evm_sail_zkvm_guest
EVM_SAIL_ZKVM_ISLARIS_CONSTRAINTS ?= zisk-local
EVM_SAIL_ZKVM_PROGRESS_REPORT ?= $(PROOF_BUILD_DIR)/evm-sail-zkvm-full-coq-progress.md
EVM_SAIL_ZKVM_SHARD_INDEX ?= 0
EVM_SAIL_ZKVM_SHARD_SIZE ?= 1024
EVM_SAIL_ZKVM_SHARD_ROOT ?= $(PROOF_BUILD_DIR)/evm-sail-zkvm-shards
EVM_SAIL_ZKVM_SHARD_DUMP ?= $(EVM_SAIL_ZKVM_SHARD_ROOT)/shard-$(EVM_SAIL_ZKVM_SHARD_INDEX)-size-$(EVM_SAIL_ZKVM_SHARD_SIZE).dump
EVM_SAIL_ZKVM_SHARD_COQ_DIR ?= $(EVM_SAIL_ZKVM_SHARD_ROOT)/shard-$(EVM_SAIL_ZKVM_SHARD_INDEX)-size-$(EVM_SAIL_ZKVM_SHARD_SIZE)-coq
EXTERN_BOUNDARY_COQ := coq/ExternBoundary.v
EXTERN_BOUNDARY_DOC := extern-boundary.md
ISLARIS_COQ_STAGE_DIR ?= $(PROOF_BUILD_DIR)/coq-stage
ISLARIS_CONSTRAINTS ?= baseline
ISLARIS_MEMORY_ALIGNMENT ?= strict
ISLARIS_CONFIG ?= riscv64_zisk_isla_coq.toml
ISLARIS_BASE_ADDRESS ?= 0x0000000080000000
ISLARIS_JOBS ?= 4
ISLARIS_COQDIR ?= isla.instructions.evm_sail_schema_prefix
ISLARIS_STACK_SMOKE_COQDIR ?= isla.instructions.evm_sail_stack_store_smoke
REQUIRE_COQ_COMPILE ?= 0

SAIL_SLICES := sail/schema_prefix.sail sail/io_contracts.sail
SMT_SLICES := smt/schema-prefix-refinement.smt2
RISCV_MANIFEST := riscv/schema-prefix-slice/Cargo.toml
RISCV_ELF := $(RISCV_TARGET_DIR)/$(RUST_TARGET)/debug/proof-schema-prefix-slice
ISLA_SAIL_PLUGIN := $(ISLA_REPO)/isla-sail/_build/default/sail_plugin_isla.cmxs
ISLARIS_STACK_SMOKE := $(CURDIR)/islaris/stack-store-smoke.dump
ISLARIS_COQ_STAGE_THEORY_DIR := $(ISLARIS_COQ_STAGE_DIR)/instructions/evm_sail_schema_prefix
ERE_ZISK_ARTIFACT_DIR ?= $(ERE_GUESTS_REPO)/proof-artifacts/reth-zisk
ERE_ZISK_ELF ?= $(ERE_ZISK_ARTIFACT_DIR)/stateless-validator-reth-zisk.proof.elf
ERE_ZISK_ISLARIS_DIR ?= $(ERE_ZISK_ARTIFACT_DIR)/islaris
ERE_ZISK_FULL_DUMP := $(ERE_ZISK_ISLARIS_DIR)/elf.dump
ERE_ZISK_FULL_PROVENANCE := $(ERE_ZISK_ISLARIS_DIR)/provenance.txt
ERE_ZISK_PROOF_ALIGNMENT ?= strict
ERE_ZISK_PROOF_ISLARIS_DIR ?= $(ERE_ZISK_ARTIFACT_DIR)/islaris-$(ERE_ZISK_PROOF_ALIGNMENT)
ERE_ZISK_PROOF_DUMP := $(ERE_ZISK_PROOF_ISLARIS_DIR)/elf.dump
ERE_ZISK_PROOF_PROVENANCE := $(ERE_ZISK_PROOF_ISLARIS_DIR)/provenance.txt
ERE_ZISK_FULL_COQ_DIR ?= $(PROOF_BUILD_DIR)/reth-zisk-full-$(ERE_ZISK_PROOF_ALIGNMENT)-coq
ERE_ZISK_FULL_COQDIR ?= isla.instructions.reth_zisk_full_$(ERE_ZISK_PROOF_ALIGNMENT)
ERE_ZISK_FULL_PROGRESS_REPORT ?= $(PROOF_BUILD_DIR)/reth-zisk-full-$(ERE_ZISK_PROOF_ALIGNMENT)-coq-progress.md
ERE_ZISK_FULL_SHARD_INDEX ?= 0
ERE_ZISK_FULL_SHARD_SIZE ?= 1024
ERE_ZISK_FULL_SHARD_ROOT ?= $(PROOF_BUILD_DIR)/reth-zisk-full-$(ERE_ZISK_PROOF_ALIGNMENT)-shards
ERE_ZISK_FULL_SHARD_DUMP ?= $(ERE_ZISK_FULL_SHARD_ROOT)/shard-$(ERE_ZISK_FULL_SHARD_INDEX)-size-$(ERE_ZISK_FULL_SHARD_SIZE).dump
ERE_ZISK_FULL_SHARD_COQ_DIR ?= $(ERE_ZISK_FULL_SHARD_ROOT)/shard-$(ERE_ZISK_FULL_SHARD_INDEX)-size-$(ERE_ZISK_FULL_SHARD_SIZE)-coq

ifeq ($(REQUIRE_COQ_COMPILE),1)
COQ_COMPILE_TARGETS := check-islaris-coq-compile
endif

.PHONY: check check-ere-full-dump check-ere-full-islaris-coq check-ere-full-islaris-progress check-ere-full-proof-dump check-evm-sail-coq check-evm-sail-core-coq check-evm-sail-guest-coq check-evm-sail-zkvm-dump check-evm-sail-zkvm-elf check-evm-sail-zkvm-islaris-coq check-evm-sail-zkvm-islaris-progress check-extern-boundary check-isla-ir check-islaris-coq check-islaris-coq-compile check-islaris-coq-stage check-islaris-dump check-islaris-stack-store-smoke check-proof-pipeline check-riscv check-riscv-no-rvc check-rustfmt check-sail check-sail-coq check-smt finalize-ere-full-islaris-coq finalize-evm-sail-zkvm-islaris-coq report-ere-full-islaris-coq report-evm-sail-zkvm-islaris-coq run-ere-full-dump run-ere-full-islaris-coq run-ere-full-islaris-coq-shard run-ere-full-proof-dump run-evm-sail-coq run-evm-sail-core-coq run-evm-sail-guest-coq run-evm-sail-zkvm-dump run-evm-sail-zkvm-elf run-evm-sail-zkvm-islaris-coq run-evm-sail-zkvm-islaris-coq-shard run-islaris-coq run-sail-coq stage-islaris-coq

check: check-sail check-smt check-extern-boundary

check-sail:
	@for f in $(SAIL_SLICES); do $(SAIL) "$$f"; done

check-smt:
	@for f in $(SMT_SLICES); do $(Z3) "$$f"; done

check-extern-boundary:
	test -s $(EXTERN_BOUNDARY_COQ)
	test -s $(EXTERN_BOUNDARY_DOC)
	grep -q "Record InputOracle" $(EXTERN_BOUNDARY_COQ)
	grep -q "Record OutputTraceContract" $(EXTERN_BOUNDARY_COQ)
	grep -q "Record HostCryptoContract" $(EXTERN_BOUNDARY_COQ)
	grep -q "Record MemoryStackContract" $(EXTERN_BOUNDARY_COQ)
	grep -q "Record WorldStateContract" $(EXTERN_BOUNDARY_COQ)
	grep -q "Record WitnessDbContract" $(EXTERN_BOUNDARY_COQ)
	grep -q "Record GuestExternContract" $(EXTERN_BOUNDARY_COQ)

check-riscv:
	env -u RUSTC_WRAPPER RUSTUP_TOOLCHAIN=$(ZISK_TOOLCHAIN) $(CARGO) build --manifest-path $(RISCV_MANIFEST) --target $(RUST_TARGET) --target-dir $(RISCV_TARGET_DIR)

check-riscv-no-rvc: check-riscv
	$(OBJDUMP) -d $(RISCV_ELF) | awk '/^[[:space:]]+[[:xdigit:]]+:/ { if ($$2 !~ /^[[:xdigit:]]{8}$$/) { print "compressed or malformed instruction: " $$0; bad=1 } } END { exit bad }'
	$(OBJDUMP) -h $(RISCV_ELF) | awk '$$2 == ".text" && $$7 != "2**2" { print ".text alignment is " $$7 ", want 2**2"; bad=1 } END { exit bad }'

check-rustfmt:
	env -u RUSTC_WRAPPER $(CARGO) fmt --manifest-path $(RISCV_MANIFEST) -- --check

$(ISLA_SAIL_PLUGIN):
	$(MAKE) -C $(ISLA_REPO)/isla-sail

check-isla-ir: $(ISLA_SAIL_PLUGIN)
	mkdir -p $(ISLA_IR_DIR)
	$(ISLA_SAIL) sail/schema_prefix.sail -o $(ISLA_IR_DIR)/schema_prefix
	$(ISLA_SAIL) sail/io_contracts.sail -o $(ISLA_IR_DIR)/io_contracts
	test -s $(ISLA_IR_DIR)/schema_prefix.ir
	test -s $(ISLA_IR_DIR)/io_contracts.ir

run-sail-coq:
	mkdir -p $(SAIL_COQ_DIR)
	$(SAIL) --coq --coq-output-dir $(SAIL_COQ_DIR) -o schema_prefix sail/schema_prefix.sail
	$(SAIL) --coq --coq-output-dir $(SAIL_COQ_DIR) -o io_contracts sail/io_contracts.sail

check-sail-coq: run-sail-coq
	test -s $(SAIL_COQ_DIR)/schema_prefix.v
	test -s $(SAIL_COQ_DIR)/schema_prefix_types.v
	test -s $(SAIL_COQ_DIR)/io_contracts.v
	test -s $(SAIL_COQ_DIR)/io_contracts_types.v
	grep -q "Definition schema_prefix_ok" $(SAIL_COQ_DIR)/schema_prefix.v
	grep -q "Definition input_header_well_formed" $(SAIL_COQ_DIR)/io_contracts.v

run-evm-sail-core-coq:
	mkdir -p $(EVM_SAIL_COQ_CORE_DIR)
	$(SAIL) --coq --coq-output-dir $(EVM_SAIL_COQ_CORE_DIR) -o evm ../sail/evm.sail

check-evm-sail-core-coq: run-evm-sail-core-coq
	test -s $(EVM_SAIL_COQ_CORE_DIR)/evm.v
	test -s $(EVM_SAIL_COQ_CORE_DIR)/evm_types.v
	grep -q "Definition process_transaction" $(EVM_SAIL_COQ_CORE_DIR)/evm.v
	grep -q "Definition compute_state_root_overlay" $(EVM_SAIL_COQ_CORE_DIR)/evm.v

run-evm-sail-guest-coq:
	mkdir -p $(EVM_SAIL_COQ_GUEST_DIR)
	$(SAIL) --coq --coq-output-dir $(EVM_SAIL_COQ_GUEST_DIR) -o evm_guest ../sail/main.sail

check-evm-sail-guest-coq: run-evm-sail-guest-coq
	test -s $(EVM_SAIL_COQ_GUEST_DIR)/evm_guest.v
	test -s $(EVM_SAIL_COQ_GUEST_DIR)/evm_guest_types.v
	grep -q "Definition input_well_formed" $(EVM_SAIL_COQ_GUEST_DIR)/evm_guest.v
	grep -q "Definition main" $(EVM_SAIL_COQ_GUEST_DIR)/evm_guest.v

run-evm-sail-coq: check-extern-boundary run-evm-sail-core-coq run-evm-sail-guest-coq

check-evm-sail-coq: check-extern-boundary check-evm-sail-core-coq check-evm-sail-guest-coq

run-evm-sail-zkvm-elf:
	cd $(EVM_SAIL_ZKVM_DIR) && ./build.sh guest

check-evm-sail-zkvm-elf: run-evm-sail-zkvm-elf
	test -s $(EVM_SAIL_ZKVM_ELF)
	$(OBJDUMP) -d $(EVM_SAIL_ZKVM_ELF) | awk '/^[[:space:]]+[[:xdigit:]]+:/ { c++; if ($$2 !~ /^[[:xdigit:]]{8}$$/) { print "compressed or malformed instruction: " $$0; bad=1 } last=$$1 } END { gsub(":", "", last); if (c < 100000) { print "too few full evm-sail zkVM guest instructions:", c; bad=1 } print "full evm-sail zkVM guest instructions:", c, "last:", last; exit bad }'
	$(OBJDUMP) -h $(EVM_SAIL_ZKVM_ELF) | awk '$$2 == ".text" { found=1; split($$7, a, "\\*\\*"); if (a[2] < 2) { print ".text alignment is " $$7 ", want at least 2**2"; bad=1 } } END { if (!found) { print "missing .text section"; bad=1 } exit bad }'
	$(OBJDUMP) -d $(EVM_SAIL_ZKVM_ELF) | awk '/^[[:space:]]+[[:xdigit:]]+:/ && $$3 ~ /^(ecall|ebreak|mret|sret)$$/ { print "environment/trap-return instruction in text: " $$0; bad=1 } END { exit bad }'

run-evm-sail-zkvm-dump: check-evm-sail-zkvm-elf
	mkdir -p $(EVM_SAIL_ZKVM_ISLARIS_DIR)
	env -u RUSTC_WRAPPER CARGO_TARGET_DIR=$(ISLA_CARGO_TARGET_DIR) $(CARGO) run --manifest-path $(ISLA_REPO)/Cargo.toml --bin isla-riscv-dump -- --elf $(EVM_SAIL_ZKVM_ELF) --out $(EVM_SAIL_ZKVM_ISLARIS_DIR) --constraints $(EVM_SAIL_ZKVM_ISLARIS_CONSTRAINTS) --memory-alignment $(ISLARIS_MEMORY_ALIGNMENT) --isla-config $(ISLARIS_CONFIG) --base-address $(ISLARIS_BASE_ADDRESS)

check-evm-sail-zkvm-dump: run-evm-sail-zkvm-dump
	test -s $(EVM_SAIL_ZKVM_DUMP)
	grep -q "Full executable ELF" $(EVM_SAIL_ZKVM_DUMP)
	grep -q "Max instructions: full target" $(EVM_SAIL_ZKVM_DUMP)
	grep -q "Constraint mode: ZiskLocal" $(EVM_SAIL_ZKVM_DUMP)
	grep -q "Memory alignment: Strict" $(EVM_SAIL_ZKVM_DUMP)
	grep -q "//@isla-config: $(ISLARIS_CONFIG)" $(EVM_SAIL_ZKVM_DUMP)
	awk '/^[[:space:]]+[[:xdigit:]]+:/ { c++; last=$$1 } END { gsub(":", "", last); if (c < 100000) { print "too few full evm-sail zkVM dump instructions:", c; exit 1 } print "full evm-sail zkVM dump instructions:", c, "last:", last }' $(EVM_SAIL_ZKVM_DUMP)

run-evm-sail-zkvm-islaris-coq: check-evm-sail-zkvm-dump
	mkdir -p $(EVM_SAIL_ZKVM_FULL_COQ_DIR)
	cd $(ISLARIS_REPO) && env -u RUSTC_WRAPPER PATH="$(ISLARIS_REPO)/bin:$(PATH)" opam exec -- dune exec -- islaris $(EVM_SAIL_ZKVM_DUMP) -j $(ISLARIS_JOBS) -o $(EVM_SAIL_ZKVM_FULL_COQ_DIR) --coqdir=$(EVM_SAIL_ZKVM_COQDIR) --arch=riscv64

run-evm-sail-zkvm-islaris-coq-shard: check-evm-sail-zkvm-dump
	mkdir -p $(EVM_SAIL_ZKVM_SHARD_ROOT) $(EVM_SAIL_ZKVM_FULL_COQ_DIR)
	$(PYTHON3) scripts/report_islaris_lowering.py --dump $(EVM_SAIL_ZKVM_DUMP) --coq-dir $(EVM_SAIL_ZKVM_FULL_COQ_DIR) --write-shard $(EVM_SAIL_ZKVM_SHARD_DUMP) --shard-index $(EVM_SAIL_ZKVM_SHARD_INDEX) --shard-size $(EVM_SAIL_ZKVM_SHARD_SIZE)
	mkdir -p $(EVM_SAIL_ZKVM_SHARD_COQ_DIR)
	@status=0; \
	( cd $(ISLARIS_REPO) && env -u RUSTC_WRAPPER PATH="$(ISLARIS_REPO)/bin:$(PATH)" opam exec -- dune exec -- islaris $(EVM_SAIL_ZKVM_SHARD_DUMP) -j $(ISLARIS_JOBS) -o $(EVM_SAIL_ZKVM_SHARD_COQ_DIR) --coqdir=$(EVM_SAIL_ZKVM_COQDIR) --arch=riscv64 ) || status=$$?; \
	find $(EVM_SAIL_ZKVM_SHARD_COQ_DIR) -maxdepth 1 -type f -name 'a*.v' -exec cp {} $(EVM_SAIL_ZKVM_FULL_COQ_DIR)/ \; ; \
	find $(EVM_SAIL_ZKVM_SHARD_COQ_DIR) -maxdepth 1 -type f -name 'a*.isla' -exec cp {} $(EVM_SAIL_ZKVM_FULL_COQ_DIR)/ \; ; \
	$(PYTHON3) scripts/report_islaris_lowering.py --dump $(EVM_SAIL_ZKVM_DUMP) --coq-dir $(EVM_SAIL_ZKVM_FULL_COQ_DIR) --report $(EVM_SAIL_ZKVM_PROGRESS_REPORT) --require-any; \
	exit $$status

report-evm-sail-zkvm-islaris-coq: check-evm-sail-zkvm-dump
	$(PYTHON3) scripts/report_islaris_lowering.py --dump $(EVM_SAIL_ZKVM_DUMP) --coq-dir $(EVM_SAIL_ZKVM_FULL_COQ_DIR) --report $(EVM_SAIL_ZKVM_PROGRESS_REPORT)

check-evm-sail-zkvm-islaris-progress: check-evm-sail-zkvm-dump
	$(PYTHON3) scripts/report_islaris_lowering.py --dump $(EVM_SAIL_ZKVM_DUMP) --coq-dir $(EVM_SAIL_ZKVM_FULL_COQ_DIR) --require-any

finalize-evm-sail-zkvm-islaris-coq: check-evm-sail-zkvm-dump
	$(PYTHON3) scripts/report_islaris_lowering.py --dump $(EVM_SAIL_ZKVM_DUMP) --coq-dir $(EVM_SAIL_ZKVM_FULL_COQ_DIR) --coq-namespace $(EVM_SAIL_ZKVM_COQDIR) --require-complete --write-instrs

check-evm-sail-zkvm-islaris-coq: run-evm-sail-zkvm-islaris-coq
	$(PYTHON3) scripts/report_islaris_lowering.py --dump $(EVM_SAIL_ZKVM_DUMP) --coq-dir $(EVM_SAIL_ZKVM_FULL_COQ_DIR) --require-complete
	test -s $(EVM_SAIL_ZKVM_FULL_COQ_DIR)/instrs.v

check-islaris-dump: check-riscv-no-rvc
	mkdir -p $(ISLARIS_DUMP_DIR)
	env -u RUSTC_WRAPPER CARGO_TARGET_DIR=$(ISLA_CARGO_TARGET_DIR) $(CARGO) run --manifest-path $(ISLA_REPO)/Cargo.toml --bin isla-riscv-dump -- --elf $(RISCV_ELF) --out $(ISLARIS_DUMP_DIR) --constraints $(ISLARIS_CONSTRAINTS) --memory-alignment $(ISLARIS_MEMORY_ALIGNMENT) --isla-config $(ISLARIS_CONFIG) --base-address $(ISLARIS_BASE_ADDRESS)
	test -s $(ISLARIS_DUMP_DIR)/elf.dump
	grep -q "Full executable ELF" $(ISLARIS_DUMP_DIR)/elf.dump
	grep -q "Max instructions: full target" $(ISLARIS_DUMP_DIR)/elf.dump
	grep -q "Memory alignment:" $(ISLARIS_DUMP_DIR)/elf.dump

check-islaris-coq: run-islaris-coq
	test -s $(ISLARIS_COQ_DIR)/instrs.v
	grep -q "0x80000000%Z" $(ISLARIS_COQ_DIR)/instrs.v
	grep -q "0x80000190%Z" $(ISLARIS_COQ_DIR)/instrs.v

check-islaris-stack-store-smoke:
	mkdir -p $(ISLARIS_STACK_SMOKE_COQ_DIR)
	cd $(ISLARIS_REPO) && env -u RUSTC_WRAPPER PATH="$(ISLARIS_REPO)/bin:$(PATH)" opam exec -- dune exec -- islaris $(ISLARIS_STACK_SMOKE) -j 1 -o $(ISLARIS_STACK_SMOKE_COQ_DIR) --coqdir=$(ISLARIS_STACK_SMOKE_COQDIR) --arch=riscv64
	test -s $(ISLARIS_STACK_SMOKE_COQ_DIR)/a80000004.v

stage-islaris-coq: run-islaris-coq
	mkdir -p $(ISLARIS_COQ_STAGE_DIR)/instructions
	cp $(ISLARIS_REPO)/dune-project $(ISLARIS_COQ_STAGE_DIR)/dune-project
	cp $(ISLARIS_REPO)/islaris.opam $(ISLARIS_COQ_STAGE_DIR)/islaris.opam
	test -e $(ISLARIS_COQ_STAGE_DIR)/theories || ln -s $(ISLARIS_REPO)/theories $(ISLARIS_COQ_STAGE_DIR)/theories
	rm -rf $(ISLARIS_COQ_STAGE_THEORY_DIR)
	cp -R $(ISLARIS_COQ_DIR) $(ISLARIS_COQ_STAGE_THEORY_DIR)

check-islaris-coq-stage: stage-islaris-coq
	test -s $(ISLARIS_COQ_STAGE_DIR)/dune-project
	test -s $(ISLARIS_COQ_STAGE_DIR)/islaris.opam
	test -e $(ISLARIS_COQ_STAGE_DIR)/theories/dune
	test -s $(ISLARIS_COQ_STAGE_THEORY_DIR)/dune
	test -s $(ISLARIS_COQ_STAGE_THEORY_DIR)/instrs.v
	grep -q "(name isla.instructions.evm_sail_schema_prefix)" $(ISLARIS_COQ_STAGE_THEORY_DIR)/dune
	grep -q "Require Export isla.instructions.evm_sail_schema_prefix.a80000000" $(ISLARIS_COQ_STAGE_THEORY_DIR)/instrs.v

check-islaris-coq-compile: check-islaris-coq-stage
	cd $(ISLARIS_COQ_STAGE_DIR) && opam exec -- dune build instructions/evm_sail_schema_prefix/instrs.vo

check-proof-pipeline: check check-rustfmt check-isla-ir check-sail-coq check-islaris-stack-store-smoke check-islaris-coq check-islaris-coq-stage $(COQ_COMPILE_TARGETS)

run-islaris-coq: check-islaris-dump
	mkdir -p $(ISLARIS_COQ_DIR)
	cd $(ISLARIS_REPO) && env -u RUSTC_WRAPPER PATH="$(ISLARIS_REPO)/bin:$(PATH)" opam exec -- dune exec -- islaris $(ISLARIS_DUMP_DIR)/elf.dump -j 1 -o $(ISLARIS_COQ_DIR) --coqdir=$(ISLARIS_COQDIR) --arch=riscv64

run-ere-full-dump:
	test -s $(ERE_ZISK_ELF)
	cd $(ERE_GUESTS_REPO) && env -u RUSTC_WRAPPER ISLA_REPO=$(ISLA_REPO) scripts/extract-zisk-elf-for-islaris.sh --elf $(ERE_ZISK_ELF) --out $(ERE_ZISK_ISLARIS_DIR)

check-ere-full-dump: run-ere-full-dump
	test -s $(ERE_ZISK_FULL_DUMP)
	test -s $(ERE_ZISK_FULL_PROVENANCE)
	grep -q "Full executable ELF" $(ERE_ZISK_FULL_DUMP)
	grep -q "Max instructions: full target" $(ERE_ZISK_FULL_DUMP)
	grep -q "Memory alignment: Native" $(ERE_ZISK_FULL_DUMP)
	grep -q "^mode=full-executable-elf$$" $(ERE_ZISK_FULL_PROVENANCE)
	grep -q "^max_instructions=full$$" $(ERE_ZISK_FULL_PROVENANCE)
	grep -q "^memory_alignment=native$$" $(ERE_ZISK_FULL_PROVENANCE)
	grep -q "^islaris_config=riscv64_zisk_isla_coq.toml$$" $(ERE_ZISK_FULL_PROVENANCE)
	grep -q "^zisk_target=riscv64ima-zisk-zkvm-elf$$" $(ERE_ZISK_FULL_PROVENANCE)
	grep -q "^zisk_profile=proof$$" $(ERE_ZISK_FULL_PROVENANCE)
	awk '/^[[:space:]]+[[:xdigit:]]+:/ { c++; last=$$1 } END { gsub(":", "", last); if (c < 500000) { print "too few full guest instructions:", c; exit 1 } print "full ERE ZisK instructions:", c, "last:", last }' $(ERE_ZISK_FULL_DUMP)

run-ere-full-proof-dump:
	test -s $(ERE_ZISK_ELF)
	cd $(ERE_GUESTS_REPO) && env -u RUSTC_WRAPPER ISLA_REPO=$(ISLA_REPO) scripts/extract-zisk-elf-for-islaris.sh --elf $(ERE_ZISK_ELF) --out $(ERE_ZISK_PROOF_ISLARIS_DIR) --memory-alignment $(ERE_ZISK_PROOF_ALIGNMENT)

check-ere-full-proof-dump: run-ere-full-proof-dump
	test -s $(ERE_ZISK_PROOF_DUMP)
	test -s $(ERE_ZISK_PROOF_PROVENANCE)
	grep -q "Full executable ELF" $(ERE_ZISK_PROOF_DUMP)
	grep -q "Max instructions: full target" $(ERE_ZISK_PROOF_DUMP)
	grep -q "Memory alignment: Strict" $(ERE_ZISK_PROOF_DUMP)
	grep -q "^mode=full-executable-elf$$" $(ERE_ZISK_PROOF_PROVENANCE)
	grep -q "^max_instructions=full$$" $(ERE_ZISK_PROOF_PROVENANCE)
	grep -q "^memory_alignment=$(ERE_ZISK_PROOF_ALIGNMENT)$$" $(ERE_ZISK_PROOF_PROVENANCE)
	grep -q "^islaris_config=riscv64_zisk_isla_coq.toml$$" $(ERE_ZISK_PROOF_PROVENANCE)
	grep -q "^zisk_target=riscv64ima-zisk-zkvm-elf$$" $(ERE_ZISK_PROOF_PROVENANCE)
	grep -q "^zisk_profile=proof$$" $(ERE_ZISK_PROOF_PROVENANCE)
	awk '/^[[:space:]]+[[:xdigit:]]+:/ { c++; last=$$1 } END { gsub(":", "", last); if (c < 500000) { print "too few full guest instructions:", c; exit 1 } print "full ERE ZisK proof instructions:", c, "last:", last }' $(ERE_ZISK_PROOF_DUMP)

run-ere-full-islaris-coq: check-ere-full-proof-dump
	mkdir -p $(ERE_ZISK_FULL_COQ_DIR)
	cd $(ISLARIS_REPO) && env -u RUSTC_WRAPPER PATH="$(ISLARIS_REPO)/bin:$(PATH)" opam exec -- dune exec -- islaris $(ERE_ZISK_PROOF_DUMP) -j $(ISLARIS_JOBS) -o $(ERE_ZISK_FULL_COQ_DIR) --coqdir=$(ERE_ZISK_FULL_COQDIR) --arch=riscv64

run-ere-full-islaris-coq-shard: check-ere-full-proof-dump
	mkdir -p $(ERE_ZISK_FULL_SHARD_ROOT) $(ERE_ZISK_FULL_COQ_DIR)
	$(PYTHON3) scripts/report_islaris_lowering.py --dump $(ERE_ZISK_PROOF_DUMP) --coq-dir $(ERE_ZISK_FULL_COQ_DIR) --write-shard $(ERE_ZISK_FULL_SHARD_DUMP) --shard-index $(ERE_ZISK_FULL_SHARD_INDEX) --shard-size $(ERE_ZISK_FULL_SHARD_SIZE)
	mkdir -p $(ERE_ZISK_FULL_SHARD_COQ_DIR)
	@status=0; \
	( cd $(ISLARIS_REPO) && env -u RUSTC_WRAPPER PATH="$(ISLARIS_REPO)/bin:$(PATH)" opam exec -- dune exec -- islaris $(ERE_ZISK_FULL_SHARD_DUMP) -j $(ISLARIS_JOBS) -o $(ERE_ZISK_FULL_SHARD_COQ_DIR) --coqdir=$(ERE_ZISK_FULL_COQDIR) --arch=riscv64 ) || status=$$?; \
	find $(ERE_ZISK_FULL_SHARD_COQ_DIR) -maxdepth 1 -type f -name 'a*.v' -exec cp {} $(ERE_ZISK_FULL_COQ_DIR)/ \; ; \
	find $(ERE_ZISK_FULL_SHARD_COQ_DIR) -maxdepth 1 -type f -name 'a*.isla' -exec cp {} $(ERE_ZISK_FULL_COQ_DIR)/ \; ; \
	$(PYTHON3) scripts/report_islaris_lowering.py --dump $(ERE_ZISK_PROOF_DUMP) --coq-dir $(ERE_ZISK_FULL_COQ_DIR) --report $(ERE_ZISK_FULL_PROGRESS_REPORT) --require-any; \
	exit $$status

report-ere-full-islaris-coq: check-ere-full-proof-dump
	$(PYTHON3) scripts/report_islaris_lowering.py --dump $(ERE_ZISK_PROOF_DUMP) --coq-dir $(ERE_ZISK_FULL_COQ_DIR) --report $(ERE_ZISK_FULL_PROGRESS_REPORT)

check-ere-full-islaris-progress: check-ere-full-proof-dump
	$(PYTHON3) scripts/report_islaris_lowering.py --dump $(ERE_ZISK_PROOF_DUMP) --coq-dir $(ERE_ZISK_FULL_COQ_DIR) --require-any

finalize-ere-full-islaris-coq: check-ere-full-proof-dump
	$(PYTHON3) scripts/report_islaris_lowering.py --dump $(ERE_ZISK_PROOF_DUMP) --coq-dir $(ERE_ZISK_FULL_COQ_DIR) --coq-namespace $(ERE_ZISK_FULL_COQDIR) --require-complete --write-instrs

check-ere-full-islaris-coq: run-ere-full-islaris-coq
	$(PYTHON3) scripts/report_islaris_lowering.py --dump $(ERE_ZISK_PROOF_DUMP) --coq-dir $(ERE_ZISK_FULL_COQ_DIR) --require-complete
	test -s $(ERE_ZISK_FULL_COQ_DIR)/instrs.v
