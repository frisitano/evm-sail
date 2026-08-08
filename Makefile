# ===========================================================================
# evm-sail — specification validation entry point
#
#   make check          type-check the specification project
#   make lint           Sail warnings plus typed readability checks
#   make fmt            format every *.sail in place with `sail --fmt`
#   make fmt-check      verify every *.sail matches `sail --fmt`
#   make c-spec         generate and compile-check the specification C model
#   make c-optimised    generate and compile-check the optimized C model
#   make c-optimised-conformance
#                       compile-check and audit optimized generated-C style
#   make c-optimised-lint-report
#                       order Clang findings by planned compiler cleanup pass
#   make c-optimised-clang-tidy
#                       enforce the comprehensive clang-tidy policy
#   make c-optimised-format-report
#                       report clang-format drift in optimized C
#   make c-optimised-clang-format
#                       enforce the optimized-C clang-format policy
#   make sail-readability-lint-report
#                       inventory typed-Sail and common-Jib cleanup findings
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
Z3_MEMO_PATH ?= $(abspath sail_smt_cache)
SAIL_Z3_FLAGS := --memo-z3 --memo-z3-path $(Z3_MEMO_PATH)
LAKE ?= lake
COQC ?= opam exec -- rocq c
PYTHON ?= python3
CLANG_TIDY ?= clang-tidy
CLANG_FORMAT ?= clang-format
CLANG ?= clang
LINT_JOBS ?= 8
LINT_PROFILE ?= comprehensive
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
C_SPEC_BUILD_DIR    := build/c-spec
C_SPEC_MODEL        := $(C_SPEC_BUILD_DIR)/evm
C_SPEC_SPECIALIZATION_LIMIT ?= 256
C_OPT_BUILD_DIR     := build/c-optimised
C_OPT_GENERATED_DIR := $(C_OPT_BUILD_DIR)/generated
C_OPT_GENERATED_INCLUDE_DIR := $(C_OPT_GENERATED_DIR)/include
C_OPT_GENERATED_SOURCE_DIR := $(C_OPT_GENERATED_DIR)/src/spec
C_OPT_GENERATED_MANIFEST := $(C_OPT_GENERATED_SOURCE_DIR)/sources.list
C_OPT_PACKAGE_MANIFEST := $(C_OPT_GENERATED_DIR)/src/sources.list
C_OPT_STAGED_FFI_SOURCE_DIR := $(C_OPT_GENERATED_DIR)/src/ffi
C_OPT_EXTRA_SAIL_FLAGS ?=
C_OPT_SPECIALIZE_LOG_FLAGS ?= --c-specialize-log
C_OPT_PACKAGE       := evmsail
C_OPTIMISED_DIR     := sail/optimised
C_OPTIMISED_SPLICES := $(addprefix $(C_OPTIMISED_DIR)/,$(shell sed '/^$$/d' $(C_OPTIMISED_DIR)/manifest))
C_OPTIMISED_SPLICE_FLAGS := $(foreach splice,$(C_OPTIMISED_SPLICES),--splice $(splice))
COQ_DIR             := extractions/coq
COQ_MODEL_DIR       := $(COQ_DIR)/src
COQ_PROOFS_DIR      := $(COQ_DIR)/proofs
LEAN_DIR            := extractions/lean
LEAN_MODEL_DIR      := $(LEAN_DIR)/src
LEAN_PROOFS_DIR     := $(LEAN_DIR)/proofs
PYTHON_DIR          := extractions/python
PYTHON_SRC_ROOT     := $(PYTHON_DIR)/src
PYTHON_PACKAGE      := $(PYTHON_SRC_ROOT)/evm
PYTHON_MODEL        := $(PYTHON_PACKAGE)/__init__.py
PYTHON_HOST_CONTRACT := extractions/python/contract/HostContract.py
PYTHON_CACHE_DIR    := $(abspath .agent-tmp/python-cache)
SAIL_READABILITY_DIR := build/lint/sail-readability
SAIL_READABILITY_SOURCE_LOG := $(SAIL_READABILITY_DIR)/source.log
SAIL_READABILITY_JIB_LOG := $(SAIL_READABILITY_DIR)/jib.log
SAIL_READABILITY_REPORT := $(SAIL_READABILITY_DIR)/report.txt
SAIL_READABILITY_JSON_REPORT := $(SAIL_READABILITY_DIR)/report.json
# Run Ruff's complete default error family: import correctness, syntax and
# invalid constructs, unused/rebound/undefined names, and related Python
# errors. In particular F821 checks every generated annotation; explicit
# runtime/type imports ensure typos are not hidden behind wildcard-import
# ambiguity. Preserve Sail's source-level variable spelling and no-op lets,
# which account for E741 and F841 respectively.
PYTHON_RUFF_RULES   := E4,E7,E9,F
PYTHON_RUFF_IGNORES := E741,F841
LEAN_HOST_AXIOMS    := extractions/lean/contract/HostAxioms.lean
LEAN_SPECIALIZATION := extractions/lean/contract/Specialization.lean
LEAN_SAIL_LIB       ?= $(abspath $(LEAN_MODEL_DIR)/.lake/packages/Sail)
COQ_SEMANTIC_FLAGS  := --coq-semantic-range-types --coq-undef-axioms
C_SPEC_HEADERS      := sail_failure.h exceptions.h region_access.h hash.h precompiles.h output.h \
                       scratch.h memory.h transient_storage.h stack.h frame_stack.h \
                       code_db.h kernel_state.h trie_node_db.h state_db.h
C_OPTIMIZED_INCLUDE_DIR := extractions/c/optimised/contract/include
C_OPTIMIZED_SOURCE_DIR  := extractions/c/optimised/contract/src
C_OPTIMIZED_EXTERNAL_TYPES_HEADER := evmsail/host/types.h
C_OPTIMIZED_EXTERNAL_TYPES := StatelessInputSliceFields \
                              ScratchSliceFields \
                              EvmMemorySliceFields \
                              CodeRegionSliceFields \
                              LogDataSliceFields \
                              OutputSliceFields \
                              PreparedAuthorizationList
C_OPTIMIZED_EXTERNAL_TYPE_FLAGS := $(foreach type,$(C_OPTIMIZED_EXTERNAL_TYPES),--c-optimized-external-type $(type)=$(C_OPTIMIZED_EXTERNAL_TYPES_HEADER))
C_OPTIMIZED_BYTE_POINTER_FLAGS := \
	--c-optimized-byte-pointer-field StatelessInputSliceFields.bytes=__direct \
	--c-optimized-byte-pointer-field ScratchSliceFields.bytes=__direct \
	--c-optimized-byte-pointer-field EvmMemorySliceFields.bytes=__direct \
	--c-optimized-byte-pointer-field CodeRegionSliceFields.bytes=__direct \
	--c-optimized-byte-pointer-field CodeFields.bytes=__direct \
	--c-optimized-byte-pointer-field LogDataSliceFields.bytes=__direct \
	--c-optimized-byte-pointer-field OutputSliceFields.bytes=__direct
C_SPEC_INCLUDES     := $(foreach header,$(C_SPEC_HEADERS),--c-include $(header))
C_SPEC_PRESERVE_FLAGS := --c-preserve main \
                       --c-preserve leaf_child_ref \
                       --c-preserve resume_frame \
                       --c-preserve process_transaction \
                       --c-preserve compute_state_root \
                       --c-preserve trie_root \
                       --c-preserve decode_stateless_input_ref
C_OPT_PRESERVE_FLAGS := --c-preserve main \
                        --c-preserve resume_frame \
                        --c-preserve validation_debug_record \
                        --c-preserve write_invalid_result \
                        --c-preserve sload_cost \
                        --c-preserve sstore_sentry_cost \
                        --c-preserve sstore_costs \
                        --c-preserve process_transaction \
                        --c-preserve compute_state_root \
                        --c-preserve decode_stateless_input_ref
SAIL_CONTRACTS      :=
EXTERN_CONTRACT     := extractions/coq/contract/ExternBoundary.v
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
SAIL_FILES := $(shell find sail -name '*.sail' | sort)

.PHONY: publish-c-extraction extract build-extractions extract-c-spec build-c-spec extract-c-optimised build-c-optimised build-coq build-lean build-python all c-optimised c-optimised-clang-format c-optimised-clang-tidy c-optimised-conformance c-optimised-format-report c-optimised-lint-report c-spec check check-contracts check-optimized-ffi check-optimized-ffi-manifest clean clear-z3-memo coq-contracts-check docs-env docs-site eest-smoke extract extract-coq extract-lean extract-python ffi-clang-format-check fmt fmt-check help lean-extract lean-harness lint python-lint python-tools-fmt-check python-tools-lint runtime-test sail-readability-lint-report zisk-guest

help:
	@echo "evm-sail targets:"
	@echo "  make check          - type-check the model ($(MODEL))"
	@echo "  make lint           - Sail warnings plus typed readability checks"
	@echo "  make fmt            - format every *.sail with sail --fmt"
	@echo "  make fmt-check      - verify *.sail match sail --fmt"
	@echo "  make runtime-test   - differential-test the bounded Sail C runtime"
	@echo "  make clear-z3-memo  - remove the persistent Sail/Z3 type-check cache"
	@echo "  make eest-smoke     - run one embedded tests-zkevm@v0.6.2 fixture"
	@echo "  make c-spec         - generate and compile-check the specification C model"
	@echo "  make c-optimised    - generate and compile-check the optimized C model"
	@echo "  make c-optimised-conformance - compile-check and audit optimized generated-C style"
	@echo "  make c-optimised-lint-report - run comprehensive Clang checks, ordered by compiler cleanup pass"
	@echo "  make c-optimised-clang-tidy - require all clang-tidy checks over generated and FFI C"
	@echo "  make c-optimised-format-report - report clang-format drift over generated and FFI C"
	@echo "  make c-optimised-clang-format - require generated and FFI C to match .clang-format"
	@echo "  make sail-readability-lint-report - inventory typed-Sail and common-Jib cleanup findings"
	@echo "  make check-optimized-ffi - enforce the allocation-free optimized C boundary"
	@echo "  make extract-coq    - generate and validate the complete Coq model"
	@echo "  make extract-lean   - generate and compile the complete Lean model"
	@echo "  make lean-harness   - build the executable Lean fixture-harness library"
	@echo "  make docs-env       - create/update the repo-local uv documentation environment"
	@echo "  make extract-python - generate and smoke-test the complete Python model"
	@echo "  make python-lint    - lint generated Python with Ruff $(RUFF_VERSION)"
	@echo "  make python-tools-lint - lint the handwritten harness/tools Python"
	@echo "  make python-tools-fmt-check - report Ruff format drift in handwritten Python"
	@echo "  make ffi-clang-format-check - require handwritten FFI C to match .clang-format"
	@echo "  make coq-contracts-check - compile the hand-maintained Coq extern contract"
	@echo "  make docs-site      - build the literate specification book"
	@echo "  make zisk-guest     - build the production ZisK guest ELF"
	@echo "  make extract        - run all maintained model extractions"
	@echo "  make all            - check + lint + fmt-check"

check:
	$(SAIL) $(SAIL_Z3_FLAGS) $(MODEL)

# Two checks (each recipe is a single-line shell command; Make 3.81 has no
# .ONESHELL). (1) Sail's ordinary warnings and typed readability diagnostics
# type-check every reachable definition and gate source-actionable hygiene.
# Post-Jib findings remain an inventory because they describe compiler-created
# structure rather than source defects. (2) banner-box alignment: inside a
# /* ===...=== */ box, every comment line must be the same width as the divider,
# so the closing */ columns line up.
lint:
	@o=$$($(SAIL) $(SAIL_Z3_FLAGS) --all-warnings --lint-readability $(MODEL) 2>&1); if printf '%s\n' "$$o" | grep -qiE "warning|error"; then printf '%s\n' "$$o" | grep -iE "warning|error" | head -20; echo "lint: FAILED (Sail warnings or readability findings)"; exit 1; fi; \
	awk 'function ck(){if(n&&d)for(i=1;i<=n;i++)if(length(b[i])!=w){print f[i]":"l[i]": comment box width "length(b[i])" != "w;bad=1}} FNR==1{ck();n=0;d=0} /^\/\*.*\*\/$$/{b[++n]=$$0;l[n]=FNR;f[n]=FILENAME;if($$0~/^\/\* =+ \*\/$$/){d=1;w=length($$0)};next} {ck();n=0;d=0} END{ck();exit bad}' $(SAIL_FILES) || { echo "lint: FAILED (misaligned comment boxes)"; exit 1; }; \
	echo "lint: clean"
	@$(PYTHON) tools/docs_lint.py . || { echo "lint: FAILED (docs style)"; exit 1; }

# one sail call per file: the files $include each other, so a single multi-file
# invocation double-loads them and errors.
fmt:
	@rc=0; for f in $(SAIL_FILES); do $(SAIL) $(SAIL_Z3_FLAGS) --fmt --fmt-emit file "$$f" || { echo "  format failed: $$f"; rc=1; }; done; [ "$$rc" -eq 0 ] && echo "formatted $$(echo $(SAIL_FILES) | wc -w | tr -d ' ') file(s) with sail --fmt" || exit 1

fmt-check:
	@rc=0; for f in $(SAIL_FILES); do $(SAIL) $(SAIL_Z3_FLAGS) --fmt --fmt-emit stdout "$$f" 2>/dev/null | diff -q "$$f" - >/dev/null 2>&1 || { echo "  needs formatting: $$f"; rc=1; }; done; [ "$$rc" -eq 0 ] && echo "fmt-check: clean" || exit 1

runtime-test:
	$(PYTHON) zkvm/runtime/sail256/test_runtime.py

check-optimized-ffi-manifest:
	$(PYTHON) tools/check_optimized_ffi.py --manifest-only

check-optimized-ffi:
	$(PYTHON) tools/check_optimized_ffi.py

zisk-guest:
	bash zkvm/zisk/build.sh guest

eest-smoke:
	@$(PYTHON) harness/run.py $(EEST_SMOKE) --limit 1 --quiet

check-contracts:
	@for f in $(SAIL_CONTRACTS); do $(SAIL) $(SAIL_Z3_FLAGS) "$$f"; done
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
	grep -q "Inductive StateJournalEntry" $(EXTERN_CONTRACT)
	grep -q "Definition split_at_open_checkpoint" $(EXTERN_CONTRACT)
	grep -q "Definition journal_commit" $(EXTERN_CONTRACT)
	grep -q "Definition reference_journal_revert" $(EXTERN_CONTRACT)
	grep -q "Definition journal_step_recorded" $(EXTERN_CONTRACT)
	grep -q "Definition reference_read_storage_original" $(EXTERN_CONTRACT)
	grep -q "Definition world_delta_describes" $(EXTERN_CONTRACT)
	grep -q "Record WorldStateContract" $(EXTERN_CONTRACT)
	grep -q "world_journal_denotes" $(EXTERN_CONTRACT)
	grep -q "Definition reference_world_state_contract" $(EXTERN_CONTRACT)
	grep -q "Definition world_state_boundary" $(EXTERN_CONTRACT)
	grep -q "Record RegionAccessContract" $(EXTERN_CONTRACT)
	grep -q "Record ScratchArenaContract" $(EXTERN_CONTRACT)
	grep -q "Record AcceleratorContract" $(EXTERN_CONTRACT)
	grep -q "Record AncestorHashContract" $(EXTERN_CONTRACT)
	grep -q "Record CodeStoreContract" $(EXTERN_CONTRACT)
	grep -q "Inductive BalIterEntry" $(EXTERN_CONTRACT)
	grep -q "Record BalRecorderContract" $(EXTERN_CONTRACT)
	grep -q "Record AuthorizationTrackerContract" $(EXTERN_CONTRACT)
	grep -q "Record WitnessDbContract" $(EXTERN_CONTRACT)
	grep -q "Record GuestExternContract" $(EXTERN_CONTRACT)
	grep -q "^structure PersistentWorld where" $(LEAN_HOST_AXIOMS)
	grep -q "^structure TransactionSnapshot where" $(LEAN_HOST_AXIOMS)
	grep -q "^def frame_stack_reset " $(LEAN_HOST_AXIOMS)
	grep -q "^def frame_stack_push " $(LEAN_HOST_AXIOMS)
	grep -q "^def frame_stack_pop " $(LEAN_HOST_AXIOMS)
	grep -q "^def referenceReadStorageOriginal" $(LEAN_HOST_AXIOMS)
	grep -q "^def worldDeltaDescribes" $(LEAN_HOST_AXIOMS)
	grep -q "^def splitAtOpenCheckpoint" $(LEAN_HOST_AXIOMS)
	grep -q "journalDenotes" $(LEAN_HOST_AXIOMS)
	grep -q "^def referenceWorldStateContract" $(LEAN_HOST_AXIOMS)
	grep -q "^def worldStateBoundary" $(LEAN_HOST_AXIOMS)
	grep -q "^class HostState:" $(PYTHON_HOST_CONTRACT)
	grep -q "^class AcceleratorContract:" $(PYTHON_HOST_CONTRACT)
	grep -q "^def state_journal_checkpoint(" $(PYTHON_HOST_CONTRACT)
	grep -q "^def state_journal_revert(" $(PYTHON_HOST_CONTRACT)

extract-coq: check-contracts
	mkdir -p $(COQ_MODEL_DIR)
	$(SAIL) $(SAIL_Z3_FLAGS) --coq $(COQ_SEMANTIC_FLAGS) --coq-output-dir $(COQ_MODEL_DIR) -o evm $(MODEL)
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

build-coq: extract-coq
	$(COQC) -q -noglob -o $(abspath $(COQ_DIR)/contract)/ExternBoundary.vo $(EXTERN_CONTRACT)
	cd $(COQ_MODEL_DIR) && $(COQC) evm_types.v
	cd $(COQ_MODEL_DIR) && $(COQC) evm.v
	cd $(COQ_MODEL_DIR) && $(COQC) $(abspath $(COQ_PROOFS_DIR))/RlpCursor.v

# Both targets keep generated output in ignored build directories and
# compile-check it against the matching complete backend. The optimized model
# uses Sail's package/module layout; handwritten FFI remains under extractions/c/.
# The Sail model, not generated C, is the readable source of truth.
# Aggregates: extraction is Sail -> target language; build is target language
# -> compiled artifact. Splitting them means a generated tree stays available
# for inspection and publication even when its backend cannot compile it yet.
extract: extract-c-spec extract-c-optimised extract-coq extract-lean extract-python
build-extractions: build-c-spec build-c-optimised build-coq build-lean build-python

extract-c-spec:
	mkdir -p $(C_SPEC_BUILD_DIR)
	$(SAIL) $(SAIL_Z3_FLAGS) -c -O --Oconstant-fold --c-no-main --c-no-rts \
		$(C_SPEC_PRESERVE_FLAGS) $(C_SPEC_INCLUDES) --c-specialize \
		--c-specialization-limit $(C_SPEC_SPECIALIZATION_LIMIT) \
		$(MODEL) --variable EVM_DEBUG=off -o $(C_SPEC_MODEL)
	test -s $(C_SPEC_MODEL).c
	test -s $(C_SPEC_MODEL).h

build-c-spec: extract-c-spec
	@sail_lib="$$($(SAIL) --dir)/lib"; \
		test -f "$$sail_lib/sail.h" || { echo "missing Sail C runtime headers under $$sail_lib"; exit 1; }; \
		$(CC) -O2 -w -Wno-error=implicit-function-declaration \
			-DEVMSAIL_MODEL_H=\"evm.h\" \
			$(GMP_CFLAGS) -I$(C_SPEC_BUILD_DIR) -I"$$sail_lib" -Iextractions/c/spec/contract -Iextractions/c \
			-c $(C_SPEC_MODEL).c -o $(C_SPEC_BUILD_DIR)/evm.o
	test -s $(C_SPEC_BUILD_DIR)/evm.o

# Backwards-compatible alias: extract then build.
c-spec: build-c-spec

# Publish the generated C sources into each backend's src/ tree so the
# extracted code is visible in the repository alongside its contract
# implementation, matching the lean/coq/python target layout. Sources only:
# object files, archives, and staging manifests stay in build/.
publish-c-extraction: extract-c-spec extract-c-optimised
	rm -rf extractions/c/spec/src extractions/c/optimised/src
	mkdir -p extractions/c/spec/src
	cp $(C_SPEC_MODEL).c $(C_SPEC_MODEL).h extractions/c/spec/src/
	mkdir -p extractions/c/optimised/src
	cp -R $(C_OPT_GENERATED_DIR)/include extractions/c/optimised/src/include
	mkdir -p extractions/c/optimised/src/src
	cp -R $(C_OPT_GENERATED_SOURCE_DIR) extractions/c/optimised/src/src/spec
	@echo "published: $$(find extractions/c/spec/src extractions/c/optimised/src -type f | wc -l | tr -d ' ') generated C files"


extract-c-optimised: check-optimized-ffi
	rm -rf $(C_OPT_GENERATED_DIR)
	mkdir -p $(C_OPT_GENERATED_DIR)
	$(SAIL) $(SAIL_Z3_FLAGS) -c -O --Oconstant-fold --all-modules \
		--c-optimized-model --c-package $(C_OPT_PACKAGE) \
		--c-output-dir $(C_OPT_GENERATED_DIR) \
		--c-optimized-source-root sail \
		--c-optimized-include-dir $(C_OPTIMIZED_INCLUDE_DIR) \
		$(C_OPTIMIZED_EXTERNAL_TYPE_FLAGS) \
		$(C_OPTIMIZED_BYTE_POINTER_FLAGS) \
		$(C_OPT_PRESERVE_FLAGS) $(C_OPT_SPECIALIZE_LOG_FLAGS) \
		$(C_OPT_EXTRA_SAIL_FLAGS) \
		$(C_OPTIMISED_SPLICE_FLAGS) \
		$(MODEL) --variable EVM_DEBUG=off
	$(PYTHON) tools/package_optimised_c.py $(C_OPT_GENERATED_DIR)
	test -s $(C_OPT_GENERATED_MANIFEST)
	test -s $(C_OPT_PACKAGE_MANIFEST)
	test -s $(C_OPT_GENERATED_INCLUDE_DIR)/$(C_OPT_PACKAGE)/spec.h

build-c-optimised: extract-c-optimised
	$(MAKE) --no-print-directory -C $(C_OPT_GENERATED_DIR) CC="$(CC)"
	test -s $(C_OPT_GENERATED_DIR)/libevmsail.a

# Backwards-compatible alias: extract then build.
c-optimised: build-c-optimised

c-optimised-conformance: c-optimised
	$(PYTHON) tools/check_optimised_c.py $(C_OPT_GENERATED_DIR)

# Advisory by default: this is the compiler-pass work queue. The deterministic
# conformance target and semantic fixtures remain gates while we burn down the
# baseline. Diagnostics are deduplicated and ordered by the pass that should
# own the fix instead of by translation-unit traversal order.
c-optimised-lint-report: c-optimised-conformance
	$(PYTHON) tools/lint_optimised_c.py --sail $(SAIL) --clang $(CLANG) --clang-tidy $(CLANG_TIDY) \
		--profile $(LINT_PROFILE) --jobs $(LINT_JOBS) $(C_OPT_GENERATED_DIR)

# This is intentionally separate from c-optimised-conformance: extraction and
# the deterministic source contract remain runnable on machines without
# clang-tidy, while reviewers and CI can opt into the richer AST diagnostics.
c-optimised-clang-tidy: c-optimised-conformance
	$(PYTHON) tools/lint_optimised_c.py --sail $(SAIL) --clang $(CLANG) --clang-tidy $(CLANG_TIDY) \
		--require-clang-tidy --strict --profile $(LINT_PROFILE) --jobs $(LINT_JOBS) $(C_OPT_GENERATED_DIR)

# Keep the report target read-only. The explicit clang-format target produces
# the pretty-C review artifact from canonical compiler output and verifies both
# generated and handwritten optimized C against the same policy.
c-optimised-format-report: c-optimised-conformance
	$(PYTHON) tools/check_optimised_c_format.py --clang-format $(CLANG_FORMAT) \
		--jobs $(LINT_JOBS) $(C_OPT_GENERATED_DIR)

c-optimised-clang-format: c-optimised-conformance
	$(PYTHON) tools/check_optimised_c_format.py --clang-format $(CLANG_FORMAT) \
		--fix --strict --jobs $(LINT_JOBS) $(C_OPT_GENERATED_DIR)

# Source diagnostics run both before elaboration and after type/effect
# checking. Post-Jib diagnostics run in the common lowering, before backend
# presentation passes. Keep both raw
# logs: the report is an inventory, while the logs retain complete locations
# and source excerpts for individual fixes.
sail-readability-lint-report:
	mkdir -p $(SAIL_READABILITY_DIR)
	$(SAIL) $(SAIL_Z3_FLAGS) --no-color --lint-readability --just-check $(MODEL) \
		> $(SAIL_READABILITY_SOURCE_LOG) 2>&1
	$(MAKE) --no-print-directory c-optimised SAIL="$(SAIL)" \
		C_OPT_EXTRA_SAIL_FLAGS="--no-color --lint-readability" C_OPT_SPECIALIZE_LOG_FLAGS= \
		> $(SAIL_READABILITY_JIB_LOG) 2>&1
	$(PYTHON) tools/summarize_sail_readability.py \
		--source-log $(SAIL_READABILITY_SOURCE_LOG) --jib-log $(SAIL_READABILITY_JIB_LOG) \
		--output $(SAIL_READABILITY_REPORT) --json-output $(SAIL_READABILITY_JSON_REPORT)
	@cat $(SAIL_READABILITY_REPORT)

extract-lean:
	mkdir -p $(LEAN_MODEL_DIR)
	test -s $(LEAN_SAIL_LIB)/lakefile.toml
	rm -rf $(LEAN_MODEL_DIR)/Evm
	rm -f $(LEAN_MODEL_DIR)/Evm.lean $(LEAN_MODEL_DIR)/lakefile.toml $(LEAN_MODEL_DIR)/lean-toolchain $(LEAN_MODEL_DIR)/.gitignore
	$(SAIL) $(SAIL_Z3_FLAGS) --lean --lean-executable --lean-explicit-measures --lean-force-output --lean-source-root sail --lean-lib-path $(LEAN_SAIL_LIB) --lean-specialization-file $(LEAN_SPECIALIZATION) --lean-import-file $(LEAN_HOST_AXIOMS) --lean-output-dir $(LEAN_DIR) -o evm $(MODEL)
	@# Sail derives the output directory from -o, which also names the Lake
	@# package; move the generated tree into src/ (preserving any .lake
	@# packages already there) so the layout matches the other targets.
	cp -R $(LEAN_DIR)/evm/. $(LEAN_MODEL_DIR)/
	rm -rf $(LEAN_DIR)/evm
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

build-lean: extract-lean
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

build-python: extract-python
	mkdir -p $(PYTHON_CACHE_DIR)
	PYTHONPYCACHEPREFIX=$(PYTHON_CACHE_DIR) $(PYTHON_EVM) -m compileall -q $(PYTHON_PACKAGE)
	PYTHONPYCACHEPREFIX=$(PYTHON_CACHE_DIR) PYTHONPATH=$(abspath $(PYTHON_SRC_ROOT)) $(PYTHON_EVM) -m py_compile $(PYTHON_DIR)/adapter.py $(PYTHON_DIR)/smoke.py
	$(PYTHON_RUFF) check --select $(PYTHON_RUFF_RULES) --ignore $(PYTHON_RUFF_IGNORES) --output-format concise $(PYTHON_DIR)
	PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=$(abspath $(PYTHON_SRC_ROOT)) $(PYTHON_EVM) $(PYTHON_DIR)/smoke.py

python-lint:
	test -s $(PYTHON_MODEL)
	$(PYTHON_RUFF) check --select $(PYTHON_RUFF_RULES) --ignore $(PYTHON_RUFF_IGNORES) --output-format concise $(PYTHON_DIR)

# Handwritten Python: the fixture harness and repository tooling. The generated
# model keeps the python-lint policy above; these targets own the
# hand-maintained scripts. Pyflakes correctness plus syntax errors gate;
# compact one-line statement style (E401/E70x) and deliberate sys.path setup
# before imports (E402) are accepted in these scripts.
PYTHON_TOOLS_DIRS         := harness tools
PYTHON_TOOLS_RUFF_RULES   := E9,F
PYTHON_TOOLS_RUFF_IGNORES := E741

python-tools-lint:
	$(PYTHON_RUFF) check --select $(PYTHON_TOOLS_RUFF_RULES) --ignore $(PYTHON_TOOLS_RUFF_IGNORES) --output-format concise $(PYTHON_TOOLS_DIRS)

# Advisory for now: the handwritten scripts predate a formatter policy. CI runs
# this without failing the build until the tree is formatted once.
python-tools-fmt-check:
	$(PYTHON_RUFF) format --check $(PYTHON_TOOLS_DIRS)

# Handwritten FFI formatting only. Unlike c-optimised-clang-format this needs
# neither the custom Sail compiler nor a generated build tree, so it can run
# on any machine with clang-format and the repository .clang-format policy.
ffi-clang-format-check:
	$(PYTHON) tools/check_optimised_c_format.py --clang-format $(CLANG_FORMAT) \
		--scope ffi --strict --jobs $(LINT_JOBS)

# Type-check the hand-maintained Coq extern contract without regenerating the
# model (no custom Sail needed; requires the Rocq prover from opam).
coq-contracts-check:
	mkdir -p $(COQ_DIR)/contract
	$(COQC) -q -noglob -o $(abspath $(COQ_DIR)/contract)/ExternBoundary.vo $(EXTERN_CONTRACT)

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
	$(SAIL) $(SAIL_Z3_FLAGS) --doc --doc-format identity --doc-embed plain --doc-embed-with-location --doc-bundle doc.json -o $(BOOK_BUILD) $(MODEL)
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
	rm -rf $(C_SPEC_BUILD_DIR) $(C_OPT_BUILD_DIR) $(LEAN_MODEL_DIR)/.lake/build $(PYTHON_CACHE_DIR) $(BOOK)/site $(BOOK_BUILD) $(BOOK)/docs/extraction $(BOOK)/docs/assets/generated

clear-z3-memo:
	rm -f "$(Z3_MEMO_PATH)" sail/sail_smt_cache
