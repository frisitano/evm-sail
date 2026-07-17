# ===========================================================================
# evm-sail — specification validation entry point
#
#   make check          type-check the specification project
#   make lint           sail --all-warnings on the program roots
#   make fmt            format every *.sail in place with `sail --fmt`
#   make fmt-check      verify every *.sail matches `sail --fmt`
#   make eest-smoke     run a tiny EEST state fixture (root-checked)
#   make all            check + lint + fmt-check
#   make clean          remove build artifacts
#
# Requires the rems-project Sail toolchain (`sail`) on PATH, installed via
# opam (NOT `brew install sail`, which is an unrelated tool). See README.md.
#
# Block EXECUTION (compile to C and run) is validated by the EEST harness
# and the zkVM guest (harness/run.py over the zkvm/native-runner builds).
# ===========================================================================

SAIL ?= $(shell bash zkvm/resolve_optimized_sail.sh)
LAKE ?= lake
COQC ?= coqc
PYTHON ?= python3

PROJECT             := sail/evm.sail_project
MODEL               := $(PROJECT) evm
EEST_SMOKE          := harness/fixtures/smoke/state_root_transfer.json
CONTRACTS_DIR       := extractions/contracts
COQ_DIR             := extractions/coq
COQ_CONTRACTS_DIR   := $(COQ_DIR)/contracts
COQ_MODEL_DIR       := $(COQ_DIR)/model
LEAN_DIR            := extractions/lean
LEAN_MODEL_DIR      := $(LEAN_DIR)/evm
LEAN_HOST_AXIOMS    := $(CONTRACTS_DIR)/HostAxioms.lean
COQ_SEMANTIC_FLAGS  := --coq-semantic-range-types --coq-undef-axioms
LEAN_SEMANTIC_FLAGS := --lean-semantic-range-types
SAIL_CONTRACTS      := $(CONTRACTS_DIR)/schema_prefix.sail $(CONTRACTS_DIR)/io_contracts.sail
EXTERN_CONTRACT     := $(CONTRACTS_DIR)/ExternBoundary.v
# Every Sail source owned by this repository, discovered rather than listed by
# hand.  Keep workspace-local worktrees and generated trees out of formatting.
SAIL_FILES := $(shell find sail extractions/contracts -name '*.sail' | sort)

.PHONY: all check check-contracts clean eest-smoke extract extract-coq extract-lean fmt fmt-check help lint runtime-test

help:
	@echo "evm-sail targets:"
	@echo "  make check          - type-check the model ($(MODEL))"
	@echo "  make lint           - sail --all-warnings on the program roots"
	@echo "  make fmt            - format every *.sail with sail --fmt"
	@echo "  make fmt-check      - verify *.sail match sail --fmt"
	@echo "  make runtime-test   - differential-test the bounded Sail C runtime"
	@echo "  make eest-smoke     - run a tiny EEST fixture (root-checked)"
	@echo "  make extract-coq    - generate and validate the complete Coq model"
	@echo "  make extract-lean   - generate and compile the complete Lean model"
	@echo "  make extract        - run both theorem-prover extractions"
	@echo "  make all            - check + lint + fmt-check"

check:
	$(SAIL) $(MODEL)

# Two checks (each recipe is a single-line shell command; Make 3.81 has no
# .ONESHELL). (1) sail --all-warnings on the project entries type-checks every
# reachable definition. (2) banner-box alignment: inside a /* ===...=== */ box,
# every comment line must be the same width as the divider, so the closing */
# columns line up.
lint:
	@o=$$($(SAIL) --all-warnings $(MODEL) 2>&1); if printf '%s\n' "$$o" | grep -qiE "warning|error"; then printf '%s\n' "$$o" | grep -iE "warning|error" | head -20; echo "lint: FAILED (sail warnings)"; exit 1; fi; \
	awk 'function ck(){if(n&&d)for(i=1;i<=n;i++)if(length(b[i])!=w){print f[i]":"l[i]": comment box width "length(b[i])" != "w;bad=1}} FNR==1{ck();n=0;d=0} /^\/\*.*\*\/$$/{b[++n]=$$0;l[n]=FNR;f[n]=FILENAME;if($$0~/^\/\* =+ \*\/$$/){d=1;w=length($$0)};next} {ck();n=0;d=0} END{ck();exit bad}' $(SAIL_FILES) || { echo "lint: FAILED (misaligned comment boxes)"; exit 1; }; \
	echo "lint: clean"

# one sail call per file: the files $include each other, so a single multi-file
# invocation double-loads them and errors.
fmt:
	@for f in $(SAIL_FILES); do $(SAIL) --fmt --fmt-emit file "$$f"; done; echo "formatted $$(echo $(SAIL_FILES) | wc -w | tr -d ' ') file(s) with sail --fmt"

fmt-check:
	@rc=0; for f in $(SAIL_FILES); do $(SAIL) --fmt --fmt-emit stdout "$$f" 2>/dev/null | diff -q "$$f" - >/dev/null 2>&1 || { echo "  needs formatting: $$f"; rc=1; }; done; [ "$$rc" -eq 0 ] && echo "fmt-check: clean" || exit 1

runtime-test:
	$(PYTHON) zkvm/runtime/sail256/test_runtime.py

eest-smoke:
	@cd harness && $(PYTHON) run.py fixtures/smoke/state_root_transfer.json --fork Cancun --limit 1 --quiet

check-contracts:
	@for f in $(SAIL_CONTRACTS); do $(SAIL) "$$f"; done
	test -s $(EXTERN_CONTRACT)
	grep -q "Record InputOracle" $(EXTERN_CONTRACT)
	grep -q "Record OutputTraceContract" $(EXTERN_CONTRACT)
	grep -q "Record CryptoContract" $(EXTERN_CONTRACT)
	grep -q "Record MemoryStackContract" $(EXTERN_CONTRACT)
	grep -q "Record WorldStateContract" $(EXTERN_CONTRACT)
	grep -q "Record WitnessDbContract" $(EXTERN_CONTRACT)
	grep -q "Record GuestExternContract" $(EXTERN_CONTRACT)

extract-coq: check-contracts
	mkdir -p $(COQ_CONTRACTS_DIR) $(COQ_MODEL_DIR)
	$(SAIL) --coq --coq-output-dir $(COQ_CONTRACTS_DIR) -o schema_prefix $(CONTRACTS_DIR)/schema_prefix.sail
	$(SAIL) --coq --coq-output-dir $(COQ_CONTRACTS_DIR) -o io_contracts $(CONTRACTS_DIR)/io_contracts.sail
	$(SAIL) --coq $(COQ_SEMANTIC_FLAGS) --coq-output-dir $(COQ_MODEL_DIR) -o evm $(MODEL)
	test -s $(COQ_CONTRACTS_DIR)/schema_prefix.v
	test -s $(COQ_CONTRACTS_DIR)/schema_prefix_types.v
	test -s $(COQ_CONTRACTS_DIR)/io_contracts.v
	test -s $(COQ_CONTRACTS_DIR)/io_contracts_types.v
	grep -q "Definition schema_prefix_ok" $(COQ_CONTRACTS_DIR)/schema_prefix.v
	grep -q "Definition input_header_well_formed" $(COQ_CONTRACTS_DIR)/io_contracts.v
	test -s $(COQ_MODEL_DIR)/evm.v
	test -s $(COQ_MODEL_DIR)/evm_types.v
	grep -q "Definition process_transaction" $(COQ_MODEL_DIR)/evm.v
	grep -q "Definition compute_state_root " $(COQ_MODEL_DIR)/evm.v
	grep -q "Definition trie_root " $(COQ_MODEL_DIR)/evm.v
	grep -q "Definition decode_stateless_input_ref" $(COQ_MODEL_DIR)/evm.v
	grep -q "Definition main" $(COQ_MODEL_DIR)/evm.v
	cd $(COQ_CONTRACTS_DIR) && $(COQC) schema_prefix_types.v
	cd $(COQ_CONTRACTS_DIR) && $(COQC) schema_prefix.v
	cd $(COQ_CONTRACTS_DIR) && $(COQC) io_contracts_types.v
	cd $(COQ_CONTRACTS_DIR) && $(COQC) io_contracts.v
	cd $(COQ_MODEL_DIR) && $(COQC) evm_types.v
	cd $(COQ_MODEL_DIR) && $(COQC) evm.v

extract-lean:
	mkdir -p $(LEAN_MODEL_DIR)
	mkdir -p $(LEAN_MODEL_DIR)/.lake
	@test ! -f $(LEAN_MODEL_DIR)/lake-manifest.json || cp $(LEAN_MODEL_DIR)/lake-manifest.json $(LEAN_MODEL_DIR)/.lake/lake-manifest.saved.json
	rm -rf $(LEAN_MODEL_DIR)/Evm
	rm -f $(LEAN_MODEL_DIR)/Evm.lean $(LEAN_MODEL_DIR)/lakefile.toml $(LEAN_MODEL_DIR)/lean-toolchain $(LEAN_MODEL_DIR)/.gitignore
	$(SAIL) --lean --lean-force-output --lean-source-root sail $(LEAN_SEMANTIC_FLAGS) --lean-import-file $(LEAN_HOST_AXIOMS) --lean-output-dir $(LEAN_DIR) -o evm $(MODEL)
	@test ! -f $(LEAN_MODEL_DIR)/.lake/lake-manifest.saved.json || mv $(LEAN_MODEL_DIR)/.lake/lake-manifest.saved.json $(LEAN_MODEL_DIR)/lake-manifest.json
	find $(LEAN_MODEL_DIR) -name '*.lean' -exec sed -i.bak 's/ByteSlice/EvmByteSlice/g' {} +
	find $(LEAN_MODEL_DIR)/Evm -name '*.lean' -exec sed -E -i.bak 's/(^|[^[:alnum:]_])prefix([^[:alnum:]_]|$$)/\1evm_prefix\2/g' {} +
	find $(LEAN_MODEL_DIR) -name '*.bak' -exec rm -f {} +
	mv $(LEAN_MODEL_DIR)/Evm/Host/ByteSlice.lean $(LEAN_MODEL_DIR)/Evm/Host/EvmByteSlice.lean
	rm -f $(LEAN_MODEL_DIR)/.gitignore
	test -s $(LEAN_MODEL_DIR)/lakefile.toml
	test -s $(LEAN_MODEL_DIR)/lean-toolchain
	test -s $(LEAN_MODEL_DIR)/Evm.lean
	grep -R -q "^def process_transaction " $(LEAN_MODEL_DIR)/Evm
	grep -R -q "^def compute_state_root " $(LEAN_MODEL_DIR)/Evm
	grep -R -q "^def trie_root " $(LEAN_MODEL_DIR)/Evm
	grep -R -q "^def decode_stateless_input_ref " $(LEAN_MODEL_DIR)/Evm
	grep -q "^def main " $(LEAN_MODEL_DIR)/Evm.lean
	! grep -R -E -q "noncomputable (section|def)|^[[:space:]]*partial def" $(LEAN_MODEL_DIR)/Evm $(LEAN_MODEL_DIR)/Evm.lean
	cd $(LEAN_MODEL_DIR) && { test -f lake-manifest.json || $(LAKE) update; }
	cd $(LEAN_MODEL_DIR) && $(LAKE) build

extract: extract-coq extract-lean

all: check lint fmt-check

clean:
	rm -rf sail_smt_cache sail/sail_smt_cache $(LEAN_MODEL_DIR)/.lake/build
