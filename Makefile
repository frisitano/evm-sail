# ===========================================================================
# evm-sail — specification validation entry point
#
#   make check          type-check the specification project
#   make lint           sail --all-warnings on the program roots
#   make fmt            format every *.sail in place with `sail --fmt`
#   make fmt-check      verify every *.sail matches `sail --fmt`
#   make c-spec         generate and compile-check the specification C model
#   make c-optimised    generate and compile-check the optimized C model
#   make eest-smoke     run one embedded v0.6.2 stateless fixture
#   make extract-python generate and smoke-test the complete Python model
#   make python-lint    lint the generated Python model with pinned Ruff
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
GMP_CFLAGS ?= $(shell pkg-config --cflags gmp 2>/dev/null)
DOCS_VENV ?= .venv-docs
DOCS_VENV_ABS := $(abspath $(DOCS_VENV))
DOCS_BIN := $(DOCS_VENV_ABS)/bin
DOCS_ENV_STAMP := $(DOCS_VENV_ABS)/.evm-sail-docs-ready
RUFF_VERSION ?= 0.15.22
ETHEREUM_TYPES_VERSION ?= 0.4.1
PYDANTIC_VERSION ?= 2.12.5
PYCRYPTODOME_VERSION ?= 3.23.0
PYTHON_RUFF ?= $(UV) run --no-project --with ruff==$(RUFF_VERSION) ruff
PYTHON_EVM ?= $(UV) run --no-project --with ethereum-types==$(ETHEREUM_TYPES_VERSION) --with pydantic==$(PYDANTIC_VERSION) --with pycryptodome==$(PYCRYPTODOME_VERSION) python

PROJECT             := sail/evm.sail_project
MODEL               := $(PROJECT) evm
EEST_CORPUS         ?= zkvm/.fixtures/current-v062-full
EEST_SMOKE          := $(EEST_CORPUS)/blockchain_tests/for_amsterdam/shanghai/eip3855_push0/push0/push0_contracts.json
CONTRACTS_DIR       := extractions/contracts
C_SPEC_BUILD_DIR    := build/c-spec
C_SPEC_MODEL        := $(C_SPEC_BUILD_DIR)/evm
C_OPT_BUILD_DIR     := build/c-optimised
C_OPT_MODEL         := $(C_OPT_BUILD_DIR)/evm
C_OPTIMIZED_SPLICE  := sail/splices/c_optimized.sail
COQ_DIR             := extractions/coq
COQ_CONTRACTS_DIR   := $(COQ_DIR)/contracts
COQ_MODEL_DIR       := $(COQ_DIR)/model
COQ_PROOFS_DIR      := $(COQ_DIR)/proofs
LEAN_DIR            := extractions/lean
LEAN_MODEL_DIR      := $(LEAN_DIR)/evm
LEAN_PROOFS_DIR     := $(LEAN_DIR)/proofs
PYTHON_DIR          := extractions/python
PYTHON_PACKAGE      := $(PYTHON_DIR)/evm
PYTHON_MODEL        := $(PYTHON_PACKAGE)/__init__.py
PYTHON_HOST_CONTRACT := $(CONTRACTS_DIR)/HostContract.py
PYTHON_CACHE_DIR    := $(abspath .agent-tmp/python-cache)
# Run Ruff's complete default error family: import correctness, syntax and
# invalid constructs, unused/rebound/undefined names, and related Python
# errors. In particular F821 checks every generated annotation; explicit
# runtime/type imports ensure typos are not hidden behind wildcard-import
# ambiguity. Preserve Sail's source-level variable spelling and no-op lets,
# which account for E741 and F841 respectively.
PYTHON_RUFF_RULES   := E4,E7,E9,F
PYTHON_RUFF_IGNORES := E741,F841
LEAN_HOST_AXIOMS    := $(CONTRACTS_DIR)/HostAxioms.lean
LEAN_SPECIALIZATION := $(CONTRACTS_DIR)/Specialization.lean
LEAN_SAIL_LIB       ?= $(abspath $(LEAN_MODEL_DIR)/.lake/packages/Sail)
COQ_SEMANTIC_FLAGS  := --coq-semantic-range-types --coq-undef-axioms
C_MODEL_HEADERS     := sail_failure.h region_access.h hash.h precompiles.h output.h \
                       scratch.h memory.h transient_storage.h stack.h frame_stack.h \
                       code_db.h kernel_state.h trie_node_db.h state_db.h
C_OPTIMIZED_HEADERS := word_bytes.h preimage.h htr.h mpt.h state.h \
                       interpreter.h
C_MODEL_INCLUDES    := $(foreach header,$(C_MODEL_HEADERS),--c-include $(header))
C_OPTIMIZED_INCLUDES := $(foreach header,$(C_OPTIMIZED_HEADERS),--c-include $(header))
C_PRESERVE_FLAGS    := --c-preserve main \
                       --c-preserve leaf_child_ref \
                       --c-preserve resume_frame \
                       --c-preserve process_transaction \
                       --c-preserve compute_state_root \
                       --c-preserve trie_root \
                       --c-preserve decode_stateless_input_ref
SAIL_CONTRACTS      :=
EXTERN_CONTRACT     := $(CONTRACTS_DIR)/ExternBoundary.v
# Installed Sail Python plugins are discovered automatically. Set this to the
# in-tree sail_plugin_python.cmxs path when validating an uninstalled build.
SAIL_PYTHON_PLUGIN  ?=
# The EVM artifact is intended for source-level review. Keep the checked AST's
# direct control flow and mirror Sail source paths in an importable package.
# Pydantic reifies dependent record parameters and checks their Sail validity
# constraints at Python construction/update boundaries. Explicit Sail
# conversions remain explicit constructors in Python.
# Override this value to request another backend presentation.
SAIL_PYTHON_FLAGS   ?= --python-preserve-structure \
                       --python-split --python-source-root sail \
                       --python-extern-module evm.HostContract \
                       --python-import-file $(PYTHON_HOST_CONTRACT) \
                       --python-pydantic \
                       --python-ethereum-fixed-bytes address=Bytes20 \
                       --python-ethereum-fixed-bytes hash=Bytes32
# Every Sail source owned by this repository, discovered rather than listed by
# hand.  Keep workspace-local worktrees and generated trees out of formatting.
SAIL_FILES := $(shell find sail extractions/contracts -name '*.sail' | sort)

.PHONY: all c-optimised c-spec check check-contracts clean docs-env docs-site eest-smoke extract extract-coq extract-lean extract-python fmt fmt-check help lean-extract lean-harness lint python-lint runtime-test zisk-guest

help:
	@echo "evm-sail targets:"
	@echo "  make check          - type-check the model ($(MODEL))"
	@echo "  make lint           - sail --all-warnings on the program roots"
	@echo "  make fmt            - format every *.sail with sail --fmt"
	@echo "  make fmt-check      - verify *.sail match sail --fmt"
	@echo "  make runtime-test   - differential-test the bounded Sail C runtime"
	@echo "  make eest-smoke     - run one embedded tests-zkevm@v0.6.2 fixture"
	@echo "  make c-spec         - generate and compile-check the specification C model"
	@echo "  make c-optimised    - generate and compile-check the optimized C model"
	@echo "  make extract-coq    - generate and validate the complete Coq model"
	@echo "  make extract-lean   - generate and compile the complete Lean model"
	@echo "  make lean-harness   - build the executable Lean fixture-harness library"
	@echo "  make docs-env       - create/update the repo-local uv documentation environment"
	@echo "  make extract-python - generate and smoke-test the complete Python model"
	@echo "  make python-lint    - lint generated Python with Ruff $(RUFF_VERSION)"
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
	test -s $(PYTHON_HOST_CONTRACT)
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
	grep -q "^class HostState:" $(PYTHON_HOST_CONTRACT)
	grep -q "^class AcceleratorContract:" $(PYTHON_HOST_CONTRACT)
	grep -q "^def state_checkpoint(" $(PYTHON_HOST_CONTRACT)
	grep -q "^def state_revert(" $(PYTHON_HOST_CONTRACT)

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
	cd $(COQ_MODEL_DIR) && $(COQC) $(abspath $(COQ_PROOFS_DIR))/RlpCursor.v

# Sail emits one C translation unit. Both targets keep that generated output in
# ignored build directories and compile-check it against the matching complete
# backend. The Sail model, not generated C, is the readable source of truth.
c-spec:
	mkdir -p $(C_SPEC_BUILD_DIR)
	$(SAIL) -c -O --Oconstant-fold --c-no-main --c-no-rts \
		$(C_PRESERVE_FLAGS) $(C_MODEL_INCLUDES) --c-specialize \
		$(MODEL) --variable EVM_DEBUG=off -o $(C_SPEC_MODEL)
	test -s $(C_SPEC_MODEL).c
	test -s $(C_SPEC_MODEL).h
	@sail_lib="$$($(SAIL) --dir)/lib"; \
		test -f "$$sail_lib/sail.h" || { echo "missing Sail C runtime headers under $$sail_lib"; exit 1; }; \
		$(CC) -O2 -w -Wno-error=implicit-function-declaration \
			-DEVMSAIL_MODEL_H=\"evm.h\" \
			$(GMP_CFLAGS) -I$(C_SPEC_BUILD_DIR) -I"$$sail_lib" -Iffi/spec -Iffi \
			-c $(C_SPEC_MODEL).c -o $(C_SPEC_BUILD_DIR)/evm.o
	test -s $(C_SPEC_BUILD_DIR)/evm.o

c-optimised:
	mkdir -p $(C_OPT_BUILD_DIR)
	$(SAIL) -c -O --Oconstant-fold --c-no-main --c-no-rts \
		$(C_PRESERVE_FLAGS) $(C_MODEL_INCLUDES) $(C_OPTIMIZED_INCLUDES) \
		--c-specialize --c-specialize-log --c-require-bounded-int \
		--splice $(C_OPTIMIZED_SPLICE) \
		$(MODEL) --variable EVM_DEBUG=off -o $(C_OPT_MODEL)
	test -s $(C_OPT_MODEL).c
	test -s $(C_OPT_MODEL).h
	@sail_lib="$$($(SAIL) --dir)/lib"; \
		test -f "$$sail_lib/sail.h" || { echo "missing Sail C runtime headers under $$sail_lib"; exit 1; }; \
		$(CC) -O2 -w -DEVMSAIL_MODEL_H=\"evm.h\" \
			-I$(C_OPT_BUILD_DIR) -Izkvm/runtime/sail256 -Izkvm/runtime \
			-I"$$sail_lib" -Iffi/optimized -Iffi \
			-c $(C_OPT_MODEL).c -o $(C_OPT_BUILD_DIR)/evm.o
	test -s $(C_OPT_BUILD_DIR)/evm.o

extract-lean:
	mkdir -p $(LEAN_MODEL_DIR)
	test -s $(LEAN_SAIL_LIB)/lakefile.toml
	rm -rf $(LEAN_MODEL_DIR)/Evm
	rm -f $(LEAN_MODEL_DIR)/Evm.lean $(LEAN_MODEL_DIR)/lakefile.toml $(LEAN_MODEL_DIR)/lean-toolchain $(LEAN_MODEL_DIR)/.gitignore
	$(SAIL) --lean --lean-executable --lean-explicit-measures --lean-force-output --lean-source-root sail --lean-lib-path $(LEAN_SAIL_LIB) --lean-specialization-file $(LEAN_SPECIALIZATION) --lean-import-file $(LEAN_HOST_AXIOMS) --lean-output-dir $(LEAN_DIR) -o evm $(MODEL)
	find $(LEAN_MODEL_DIR)/Evm -name '*.lean' -exec sed -E -i.bak 's/(^|[^[:alnum:]_])prefix([^[:alnum:]_]|$$)/\1evm_prefix\2/g' {} +
	find $(LEAN_MODEL_DIR) -name '*.bak' -exec rm -f {} +
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

extract-python:
	mkdir -p $(PYTHON_DIR) $(PYTHON_CACHE_DIR)
	rm -rf $(PYTHON_PACKAGE)
	rm -f $(PYTHON_DIR)/evm.py
	$(SAIL) $(if $(SAIL_PYTHON_PLUGIN),-plugin $(SAIL_PYTHON_PLUGIN)) --python $(SAIL_PYTHON_FLAGS) -o $(PYTHON_PACKAGE) $(MODEL)
	test -s $(PYTHON_MODEL)
	test -s $(PYTHON_PACKAGE)/HostContract.py
	! grep -R -q "call_extern" $(PYTHON_PACKAGE)
	! grep -R -q "register_extern" $(PYTHON_PACKAGE)
	! grep -R -q "_sail_extern\\." $(PYTHON_PACKAGE)
	PYTHONPYCACHEPREFIX=$(PYTHON_CACHE_DIR) $(PYTHON_EVM) -m compileall -q $(PYTHON_PACKAGE)
	PYTHONPYCACHEPREFIX=$(PYTHON_CACHE_DIR) $(PYTHON_EVM) -m py_compile $(PYTHON_DIR)/adapter.py $(PYTHON_DIR)/smoke.py
	$(PYTHON_RUFF) check --select $(PYTHON_RUFF_RULES) --ignore $(PYTHON_RUFF_IGNORES) --output-format concise $(PYTHON_DIR)
	PYTHONDONTWRITEBYTECODE=1 $(PYTHON_EVM) $(PYTHON_DIR)/smoke.py

python-lint:
	test -s $(PYTHON_MODEL)
	$(PYTHON_RUFF) check --select $(PYTHON_RUFF_RULES) --ignore $(PYTHON_RUFF_IGNORES) --output-format concise $(PYTHON_DIR)

extract: extract-coq extract-lean extract-python

all: check lint fmt-check

# Build the MkDocs Material specification book from the sources: docinfo
# bundle -> sail-lsp semantic index -> generated pages (sail-book-gen) ->
# strict mkdocs build. Prose lives in /*md and /*! comments in the .sail
# sources. Requires uv, sail_lsp, and the mkdocstrings-sail package
# (override MKDOCSTRINGS_SAIL to test another package checkout).
BOOK ?= book
BOOK_BUILD := $(BOOK)/.build
MKDOCSTRINGS_SAIL ?= mkdocstrings-sail

docs-env:
	@if [ ! -x '$(DOCS_BIN)/python' ] || [ ! -f '$(DOCS_ENV_STAMP)' ] || [ '$(MKDOCSTRINGS_SAIL)/pyproject.toml' -nt '$(DOCS_ENV_STAMP)' ] || [ '$(MKDOCSTRINGS_SAIL)/uv.lock' -nt '$(DOCS_ENV_STAMP)' ]; then \
		echo "docs-env: syncing $(DOCS_VENV) from $(MKDOCSTRINGS_SAIL)/uv.lock"; \
		UV_PROJECT_ENVIRONMENT='$(DOCS_VENV_ABS)' $(UV) sync --project '$(abspath $(MKDOCSTRINGS_SAIL))' --locked || exit $$?; \
		touch '$(DOCS_ENV_STAMP)'; \
	fi

docs-site: docs-env
	'$(DOCS_BIN)/sail-book-gen' --book '$(BOOK)' --clean
	rm -rf $(BOOK)/site $(BOOK_BUILD)
	@mkdir -p $(BOOK_BUILD) $(BOOK)/docs
	$(SAIL) --doc --doc-format identity --doc-embed plain --doc-embed-with-location --doc-bundle doc.json -o $(BOOK_BUILD) $(MODEL)
	'$(DOCS_BIN)/sail-lsp-index' --sail '$(SAIL)' --root . --workspace-root sail --project $(PROJECT) --module evm --output $(BOOK_BUILD)/lsp-index.json
	'$(DOCS_BIN)/sail-book-gen' --sail '$(SAIL)' --root . --project $(PROJECT) --module evm --book $(BOOK) --site-name "EVM Sail Specification" --no-config
	cd $(BOOK) && DISABLE_MKDOCS_2_WARNING=true '$(DOCS_BIN)/mkdocs' build --strict -d site
	@echo "book: $(BOOK)/site/index.html"

# Compatibility spelling retained for the docs workflow.
lean-extract: extract-lean

lean-harness:
	bash $(LEAN_DIR)/runner/build_lib.sh

clean:
	@if [ -x '$(DOCS_BIN)/sail-book-gen' ]; then '$(DOCS_BIN)/sail-book-gen' --book '$(BOOK)' --clean; fi
	rm -rf sail_smt_cache sail/sail_smt_cache $(C_SPEC_BUILD_DIR) $(C_OPT_BUILD_DIR) $(LEAN_MODEL_DIR)/.lake/build $(PYTHON_CACHE_DIR) $(BOOK)/site $(BOOK_BUILD) $(BOOK)/docs/extraction $(BOOK)/docs/assets/generated
