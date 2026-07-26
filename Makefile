# ===========================================================================
# evm-sail — specification validation entry point
#
#   make check          type-check the specification project
#   make lint           sail --all-warnings on the program roots
#   make fmt            format every *.sail in place with `sail --fmt`
#   make fmt-check      verify every *.sail matches `sail --fmt`
#   make extract-c      generate and compile-check the optimized C model
#   make eest-smoke     run one embedded v0.6.2 stateless fixture
#   make all            check + lint + fmt-check
#   make clean          remove build artifacts
#
# Requires this repository's custom rems-project Sail compiler. It is resolved
# once through zkvm/resolve_optimized_sail.sh and used for checks plus the C,
# Coq, and Lean backends. See README.md.
#
# Block EXECUTION (compile to C and run) is validated by the EEST harness
# and the zkVM guest (harness/run.py over the zkvm/native-runner builds).
# ===========================================================================

SAIL ?= $(shell bash zkvm/resolve_optimized_sail.sh)
LAKE ?= lake
COQC ?= opam exec -- rocq c
PYTHON ?= python3
UV ?= uv

PROJECT             := sail/evm.sail_project
MODEL               := $(PROJECT) evm
EEST_CORPUS         ?= zkvm/.fixtures/current-v062-full
EEST_SMOKE          := $(EEST_CORPUS)/blockchain_tests/for_amsterdam/shanghai/eip3855_push0/push0/push0_contracts.json
CONTRACTS_DIR       := extractions/contracts
C_DIR               := extractions/c
C_MODEL_DIR         := $(C_DIR)/evm
C_MODEL             := $(C_MODEL_DIR)/evm
C_BUILD_DIR         := build/extract-c
C_EXTRACTOR         := tools/extract_c.py
C_GENERATOR_DIR     := $(C_BUILD_DIR)/generate
C_OPTIMIZED_SPLICE  := sail/splices/c_optimized.sail
COQ_DIR             := extractions/coq
COQ_CONTRACTS_DIR   := $(COQ_DIR)/contracts
COQ_MODEL_DIR       := $(COQ_DIR)/model
LEAN_DIR            := extractions/lean
LEAN_MODEL_DIR      := $(LEAN_DIR)/evm
LEAN_HOST_AXIOMS    := $(CONTRACTS_DIR)/HostAxioms.lean
LEAN_SPECIALIZATION := $(CONTRACTS_DIR)/Specialization.lean
LEAN_SAIL_LIB       ?= $(abspath $(LEAN_MODEL_DIR)/.lake/packages/Sail)
COQ_SEMANTIC_FLAGS  := --coq-semantic-range-types --coq-undef-axioms
C_MODEL_HEADERS     := sail_failure.h byte_slice_glue.h host_crypto.h precompiles.h output.h \
                       scratch.h memory.h transient_storage.h stack.h frame_stack.h \
                       code_db.h kernel_state.h trie_node_db.h state_db.h \
                       cycle_scopes.h
C_MODEL_INCLUDES    := $(foreach header,$(C_MODEL_HEADERS),--c-include $(header))
C_PRESERVE_FLAGS    := --c-preserve main \
                       --c-preserve process_transaction \
                       --c-preserve compute_state_root \
                       --c-preserve trie_root \
                       --c-preserve decode_stateless_input_ref
C_EDITOR_FLAGS      := -w -I$(C_MODEL_DIR) \
                       -Izkvm/runtime/sail256 -Izkvm/runtime -Iffi
C_EDITOR_ARGS       := $(foreach flag,$(C_EDITOR_FLAGS),--compile-flag=$(flag))
SAIL_CONTRACTS      :=
EXTERN_CONTRACT     := $(CONTRACTS_DIR)/ExternBoundary.v
# Every Sail source owned by this repository, discovered rather than listed by
# hand.  Keep workspace-local worktrees and generated trees out of formatting.
SAIL_FILES := $(shell find sail extractions/contracts -name '*.sail' | sort)

.PHONY: all check check-contracts clean docs-site eest-smoke extract extract-c extract-coq extract-lean fmt fmt-check help lean-extract lean-harness lint runtime-test zisk-guest

help:
	@echo "evm-sail targets:"
	@echo "  make check          - type-check the model ($(MODEL))"
	@echo "  make lint           - sail --all-warnings on the program roots"
	@echo "  make fmt            - format every *.sail with sail --fmt"
	@echo "  make fmt-check      - verify *.sail match sail --fmt"
	@echo "  make runtime-test   - differential-test the bounded Sail C runtime"
	@echo "  make eest-smoke     - run one embedded tests-zkevm@v0.6.2 fixture"
	@echo "  make extract-c      - generate source-aligned optimized C and compile-check it"
	@echo "  make extract-coq    - generate and validate the complete Coq model"
	@echo "  make extract-lean   - generate and compile the complete Lean model"
	@echo "  make lean-harness   - build the executable Lean fixture-harness library"
	@echo "  make docs-site      - build the literate specification book"
	@echo "  make zisk-guest     - build the production ZisK guest ELF"
	@echo "  make extract        - run all maintained model extractions"
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
	@$(PYTHON) tools/docs_lint.py . || { echo "lint: FAILED (docs style)"; exit 1; }

# one sail call per file: the files $include each other, so a single multi-file
# invocation double-loads them and errors.
fmt:
	@for f in $(SAIL_FILES); do $(SAIL) --fmt --fmt-emit file "$$f"; done; echo "formatted $$(echo $(SAIL_FILES) | wc -w | tr -d ' ') file(s) with sail --fmt"

fmt-check:
	@rc=0; for f in $(SAIL_FILES); do $(SAIL) --fmt --fmt-emit stdout "$$f" 2>/dev/null | diff -q "$$f" - >/dev/null 2>&1 || { echo "  needs formatting: $$f"; rc=1; }; done; [ "$$rc" -eq 0 ] && echo "fmt-check: clean" || exit 1

runtime-test:
	$(PYTHON) zkvm/runtime/sail256/test_runtime.py

zisk-guest:
	bash zkvm/zisk/build.sh guest

eest-smoke:
	@$(PYTHON) harness/run.py $(EEST_SMOKE) --limit 1 --quiet

check-contracts:
	@for f in $(SAIL_CONTRACTS); do $(SAIL) "$$f"; done
	test -s $(EXTERN_CONTRACT)
	test -s $(LEAN_HOST_AXIOMS)
	test -s $(LEAN_SPECIALIZATION)
	grep -q "Record InputOracle" $(EXTERN_CONTRACT)
	grep -q "Record OutputTraceContract" $(EXTERN_CONTRACT)
	grep -q "Record CryptoContract" $(EXTERN_CONTRACT)
	grep -q "Record MemoryStackContract" $(EXTERN_CONTRACT)
	grep -q "continuation_stack_lifo_contract" $(EXTERN_CONTRACT)
	grep -q "Record PersistentWorld" $(EXTERN_CONTRACT)
	grep -q "Record TransactionSnapshot" $(EXTERN_CONTRACT)
	grep -q "Definition reference_read_storage_original" $(EXTERN_CONTRACT)
	grep -q "Definition world_delta_describes" $(EXTERN_CONTRACT)
	grep -q "Record WorldStateContract" $(EXTERN_CONTRACT)
	grep -q "world_checkpoint_denotes" $(EXTERN_CONTRACT)
	grep -q "Definition reference_world_state_contract" $(EXTERN_CONTRACT)
	grep -q "Definition world_state_boundary" $(EXTERN_CONTRACT)
	grep -q "Record WitnessDbContract" $(EXTERN_CONTRACT)
	grep -q "Record GuestExternContract" $(EXTERN_CONTRACT)
	grep -q "^structure PersistentWorld where" $(LEAN_HOST_AXIOMS)
	grep -q "^structure TransactionSnapshot where" $(LEAN_HOST_AXIOMS)
	grep -q "^def frame_stack_reset " $(LEAN_HOST_AXIOMS)
	grep -q "^def frame_stack_push " $(LEAN_HOST_AXIOMS)
	grep -q "^def frame_stack_pop " $(LEAN_HOST_AXIOMS)
	grep -q "^def referenceReadStorageOriginal" $(LEAN_HOST_AXIOMS)
	grep -q "^def worldDeltaDescribes" $(LEAN_HOST_AXIOMS)
	grep -q "checkpointDenotes" $(LEAN_HOST_AXIOMS)
	grep -q "^def referenceWorldStateContract" $(LEAN_HOST_AXIOMS)
	grep -q "^def worldStateBoundary" $(LEAN_HOST_AXIOMS)

extract-coq: check-contracts
	mkdir -p $(COQ_CONTRACTS_DIR) $(COQ_MODEL_DIR)
	$(COQC) -q -noglob -o $(abspath $(COQ_CONTRACTS_DIR))/ExternBoundary.vo $(EXTERN_CONTRACT)
	$(SAIL) --coq $(COQ_SEMANTIC_FLAGS) --coq-output-dir $(COQ_MODEL_DIR) -o evm $(MODEL)
	test -s $(COQ_MODEL_DIR)/evm.v
	test -s $(COQ_MODEL_DIR)/evm_types.v
	grep -q "Definition process_transaction" $(COQ_MODEL_DIR)/evm.v
	grep -q "Definition compute_state_root " $(COQ_MODEL_DIR)/evm.v
	grep -q "Definition trie_root " $(COQ_MODEL_DIR)/evm.v
	grep -q "Definition decode_stateless_input_ref" $(COQ_MODEL_DIR)/evm.v
	grep -q "Definition main" $(COQ_MODEL_DIR)/evm.v
	grep -q "Axiom frame_stack_reset " $(COQ_MODEL_DIR)/evm.v
	grep -q "Axiom frame_stack_push " $(COQ_MODEL_DIR)/evm.v
	grep -q "Axiom frame_stack_pop " $(COQ_MODEL_DIR)/evm.v
	cd $(COQ_MODEL_DIR) && $(COQC) evm_types.v
	cd $(COQ_MODEL_DIR) && $(COQC) evm.v

# Sail emits one C translation unit. The extraction generator uses temporary
# marker builds to identify source boundaries, mirrors each active sail/ source
# as a unity fragment, and verifies that recombining the fragments reconstructs
# the unmodified optimized C output byte-for-byte. Default Sail name mangling is
# retained. Compile the unity source into the ignored build directory; the
# tracked extraction contains sources only.
extract-c:
	mkdir -p $(C_BUILD_DIR)
	$(PYTHON) $(C_EXTRACTOR) \
		--sail-command "$(SAIL)" \
		--cc "$(CC)" $(C_EDITOR_ARGS) \
		--project $(PROJECT) --module evm --source-root sail \
		--output-dir $(C_MODEL_DIR) --work-dir $(C_GENERATOR_DIR) \
		--variable EVM_PROFILE=off --variable EVM_DEBUG=off -- \
		-c -O --Oconstant-fold --c-no-main --c-no-rts \
		$(C_PRESERVE_FLAGS) $(C_MODEL_INCLUDES) \
		--c-specialize --c-require-bounded-int --splice $(C_OPTIMIZED_SPLICE)
	test -s $(C_MODEL).c
	test -s $(C_MODEL).h
	test -s $(C_MODEL_DIR)/evm_internal.h
	test -s $(C_MODEL_DIR)/compile_commands.json
	test -s $(C_MODEL_DIR)/prelude.c
	test -s $(C_MODEL_DIR)/primitives/quantities.c
	test -s $(C_MODEL_DIR)/main.c
	@source_count="$$($(SAIL) --list-files $(MODEL) --variable EVM_PROFILE=off --variable EVM_DEBUG=off | wc -w | tr -d ' ')"; \
		c_count="$$(find $(C_MODEL_DIR) -name '*.c' | wc -l | tr -d ' ')"; \
		h_count="$$(find $(C_MODEL_DIR) -name '*.h' | wc -l | tr -d ' ')"; \
		expected_count="$$((source_count + 1))"; \
		test "$$c_count" = "$$expected_count" || { echo "extract-c: expected $$expected_count C files, found $$c_count"; exit 1; }; \
		test "$$h_count" = "2" || { echo "extract-c: expected public and internal headers, found $$h_count"; exit 1; }
	grep -Fq "zprocess_transaction(" $(C_MODEL).h
	grep -Fq "zcompute_state_root(" $(C_MODEL).h
	grep -Fq "ztrie_root(" $(C_MODEL).h
	grep -Fq "zdecode_stateless_input_ref(" $(C_MODEL).h
	grep -Fq "zmain(" $(C_MODEL).h
	grep -Fq "typedef struct { uint64_t limbs[4]; } sail_u256;" $(C_MODEL).h
	grep -Fq "typedef struct { uint8_t bytes[20]; } sail_fixed_bytes_20;" $(C_MODEL).h
	grep -Fq "typedef struct { uint8_t bytes[32]; } sail_fixed_bytes_32;" $(C_MODEL).h
	@sail_lib="$$($(SAIL) --dir)/lib"; \
		test -f "$$sail_lib/sail.h" || { echo "missing Sail C runtime headers under $$sail_lib"; exit 1; }; \
		fragment_sources="$$(find $(C_MODEL_DIR) -name '*.c' ! -name 'evm.c' | sort)"; \
		test -n "$$fragment_sources" || { echo "extract-c: no source fragments found"; exit 1; }; \
		$(CC) $(C_EDITOR_FLAGS) -I"$$sail_lib" -fsyntax-only $$fragment_sources; \
		$(CC) -O2 $(C_EDITOR_FLAGS) -I"$$sail_lib" \
			-c $(C_MODEL).c -o $(C_BUILD_DIR)/evm.o
	test -s $(C_BUILD_DIR)/evm.o

extract-lean:
	mkdir -p $(LEAN_MODEL_DIR)
	test -s $(LEAN_SAIL_LIB)/lakefile.toml
	rm -rf $(LEAN_MODEL_DIR)/Evm
	rm -f $(LEAN_MODEL_DIR)/Evm.lean $(LEAN_MODEL_DIR)/lakefile.toml $(LEAN_MODEL_DIR)/lean-toolchain $(LEAN_MODEL_DIR)/.gitignore
	$(SAIL) --lean --lean-executable --lean-explicit-measures --lean-force-output --lean-source-root sail --lean-lib-path $(LEAN_SAIL_LIB) --lean-specialization-file $(LEAN_SPECIALIZATION) --lean-import-file $(LEAN_HOST_AXIOMS) --lean-output-dir $(LEAN_DIR) -o evm $(MODEL)
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
	grep -q "^def frame_stack_reset " $(LEAN_MODEL_DIR)/Evm/HostAxioms.lean
	grep -q "^def frame_stack_push " $(LEAN_MODEL_DIR)/Evm/HostAxioms.lean
	grep -q "^def frame_stack_pop " $(LEAN_MODEL_DIR)/Evm/HostAxioms.lean
	grep -q "^def referenceWorldStateContract" $(LEAN_MODEL_DIR)/Evm/HostAxioms.lean
	grep -q "^def worldStateBoundary" $(LEAN_MODEL_DIR)/Evm/HostAxioms.lean
	! grep -R -E -q "noncomputable (section|def)|^[[:space:]]*partial def" $(LEAN_MODEL_DIR)/Evm $(LEAN_MODEL_DIR)/Evm.lean
	cd $(LEAN_MODEL_DIR) && $(LAKE) update Sail
	cd $(LEAN_MODEL_DIR) && $(LAKE) build

extract: extract-coq extract-lean extract-c

all: check lint fmt-check

# Build the MkDocs Material specification book from the sources: docinfo
# bundle -> sail-lsp semantic index -> generated pages (sail-book-gen) ->
# strict mkdocs build. Prose lives in /*md and /*! comments in the .sail
# sources. Requires uv, sail_lsp, and the mkdocstrings-sail package
# (override MKDOCSTRINGS_SAIL to test another package checkout).
BOOK ?= book
MKDOCSTRINGS_SAIL ?= mkdocstrings-sail
LEAN_BOOK_ROOT ?= $(LEAN_MODEL_DIR)
docs-site:
	@mkdir -p $(BOOK)/doc $(BOOK)/docs
	$(SAIL) --doc --doc-format identity --doc-embed plain --doc-embed-with-location --doc-bundle doc.json -o $(BOOK)/doc $(MODEL)
	$(UV) run --with-editable '$(abspath $(MKDOCSTRINGS_SAIL))' sail-lsp-index --sail '$(SAIL)' --root . --project $(PROJECT) --module evm --output $(BOOK)/doc/lsp-index.json
	$(UV) run --with-editable '$(abspath $(MKDOCSTRINGS_SAIL))' sail-book-gen --sail '$(SAIL)' --root . --project $(PROJECT) --module evm --book $(BOOK) --site-name "EVM Sail Specification" $(if $(wildcard $(LEAN_BOOK_ROOT)/Evm.lean),--lean $(LEAN_BOOK_ROOT))
	cd $(BOOK) && DISABLE_MKDOCS_2_WARNING=true $(UV) run --with-editable '$(abspath $(MKDOCSTRINGS_SAIL))' mkdocs build --strict -d site
	@echo "book: $(BOOK)/site/index.html"

# Compatibility spelling retained for the docs workflow.
lean-extract: extract-lean

lean-harness:
	bash $(LEAN_DIR)/runner/build_lib.sh

clean:
	rm -rf sail_smt_cache sail/sail_smt_cache $(C_BUILD_DIR) $(C_MODEL_DIR)/compile_commands.json $(LEAN_MODEL_DIR)/.lake/build $(BOOK)/site $(BOOK)/doc $(BOOK)/docs/reference $(BOOK)/docs/extraction $(BOOK)/docs/assets $(BOOK)/mkdocs.yml
