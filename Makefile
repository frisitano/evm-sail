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
#   make python-tools-check
#                       lint, format-check, type-check, and test devtools
#   make all            check + lint + fmt-check
#   make clean          remove build artifacts
#
# Requires this repository's custom rems-project Sail compiler, used for
# checks plus the C, Rocq, and Lean backends. It defaults to `sail` on PATH;
# override with `make SAIL=/path/to/sail`. Upstream Sail is not a supported
# fallback -- it produces a wrong model rather than an error. See README.md.
#
# Block EXECUTION (compile to C and run) is validated by the EEST harness
# and the zkVM guest (devtools.harness.cli over the zkvm/native-runner builds).
# ===========================================================================

SAIL ?= sail
SAIL_SOURCE ?=
TOOLCHAIN_CONFIG ?= config/toolchains.toml
Z3_MEMO_PATH ?= $(abspath sail_smt_cache)
SAIL_Z3_FLAGS := --memo-z3 --memo-z3-path $(Z3_MEMO_PATH)
LAKE ?= lake
ROCQ ?= opam exec -- rocq
ROCQ_FLAGS ?= -q -quiet
CLANG_TIDY ?= clang-tidy
CLANG_FORMAT ?= clang-format
CLANG ?= clang
LINT_JOBS ?= 8
LINT_PROFILE ?= comprehensive
C_OPT_LINT_BASELINE ?= config/optimised-c-lint-baseline.json
C_OPT_CONFORMANCE_WAIVERS ?= config/optimised-c-conformance-waivers.toml
UV ?= uv
PYTHON_PROJECT ?= $(UV) run --frozen
PYTHON ?= $(PYTHON_PROJECT) python
PYTHON_RUFF ?= $(PYTHON_PROJECT) ruff
PYTHON_MYPY ?= $(PYTHON_PROJECT) mypy
PYTHON_PYTEST ?= $(PYTHON_PROJECT) pytest
PYTHON_CONFIG ?= $(PYTHON)
PYTHON_EVM ?= $(PYTHON)
GMP_CFLAGS ?= $(shell pkg-config --cflags gmp 2>/dev/null)
DOCS_VENV ?= .venv-docs
DOCS_VENV_ABS := $(abspath $(DOCS_VENV))
DOCS_BIN := $(DOCS_VENV_ABS)/bin
DOCS_ENV_STAMP := $(DOCS_VENV_ABS)/.evm-sail-docs-ready
PROJECT             := sail/evm.sail_project
MODEL               := $(PROJECT) evm
GENERATED_ROOT       ?= build/generated
EEST_CORPUS         ?= zkvm/.fixtures/current-v062-full
EEST_CORPUS_CACHE   ?= $(abspath .agent-tmp/corpus-cache)
EEST_CORPUS_MANIFEST := config/corpora/tests-zkevm.toml
CONTRACT_CANARIES    := config/canaries/contracts.toml
PUBLIC_TARGETS       := config/public-targets.toml
ROCQ_CANARIES        := config/canaries/rocq.toml
LEAN_CANARIES        := config/canaries/lean.toml
PYTHON_CANARIES      := config/canaries/python.toml
EEST_SMOKE          := $(EEST_CORPUS)/blockchain_tests/for_amsterdam/shanghai/eip3855_push0/push0/push0_contracts.json
C_SPEC_BUILD_DIR    := build/c-spec
C_SPEC_MODEL        := $(C_SPEC_BUILD_DIR)/evm
C_SPEC_SPECIALIZATION_LIMIT ?= 256
C_OPT_BUILD_DIR     := build/c-optimised
C_OPT_SPECIALIZATION_LIMIT ?= 256
C_OPT_GENERATED_DIR := $(C_OPT_BUILD_DIR)/generated
C_OPT_GENERATED_INCLUDE_DIR := $(C_OPT_GENERATED_DIR)/include
C_OPT_GENERATED_SOURCE_DIR := $(C_OPT_GENERATED_DIR)/src/spec
C_OPT_GENERATED_MANIFEST := $(C_OPT_GENERATED_SOURCE_DIR)/sources.list
C_OPT_PACKAGE_MANIFEST := $(C_OPT_GENERATED_DIR)/src/sources.list
C_OPT_STAGED_FFI_SOURCE_DIR := $(C_OPT_GENERATED_DIR)/src/ffi
C_OPT_EXTRA_SAIL_FLAGS ?=
C_OPT_INLINE_FLAGS ?= --c-inline-attr --c-always-inline-attr
C_OPT_SPECIALIZE_LOG_FLAGS ?= --c-specialize-log
C_OPT_PACKAGE       := evmsail
C_OPT_COMPDB        := compile_commands.json
C_OPT_QUALITY_DIR   := build/extraction-quality
C_OPT_QUALITY_RECORD := $(C_OPT_QUALITY_DIR)/record.json
C_OPT_QUALITY_SUMMARY := $(C_OPT_QUALITY_DIR)/summary.md
C_OPT_EVALUATION_PURPOSE ?= experiment
C_OPT_SAIL_SOURCE_ARG = $(if $(strip $(SAIL_SOURCE)),--sail-source "$(SAIL_SOURCE)")
C_OPTIMISED_DIR     := sail/optimised
C_OPTIMISED_SPLICE_FLAGS = $(shell $(PYTHON_CONFIG) -m devtools.build_support manifest-words \
	--root $(C_OPTIMISED_DIR) --prefix=--splice $(C_OPTIMISED_DIR)/manifest)
ROCQ_DIR            := extractions/rocq
ROCQ_COMMITTED_MODEL_DIR := $(ROCQ_DIR)/src
ROCQ_STAGE_DIR      := $(GENERATED_ROOT)/rocq
ROCQ_MODEL_DIR      := $(ROCQ_STAGE_DIR)/src
ROCQ_PROOFS_DIR     := $(ROCQ_DIR)/proofs
LEAN_DIR            := extractions/lean
LEAN_COMMITTED_MODEL_DIR := $(LEAN_DIR)/src
LEAN_STAGE_DIR      := $(GENERATED_ROOT)/lean
LEAN_MODEL_DIR      := $(LEAN_STAGE_DIR)/src
LEAN_PROOFS_DIR     := $(LEAN_DIR)/proofs
PYTHON_DIR          := extractions/python
PYTHON_COMMITTED_SRC_ROOT := $(PYTHON_DIR)/src
PYTHON_STAGE_DIR    := $(GENERATED_ROOT)/python
PYTHON_SRC_ROOT     := $(PYTHON_STAGE_DIR)/src
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
# Sail's Lean support library. LEAN_SAIL_LIB is where it lives on disk;
# LEAN_SAIL_LIB_REQUIRE is the path recorded in the generated lakefile's
# `require`, resolved by Lake relative to the generated package root. Keeping
# the recorded path relative is what makes the committed lakefile portable, so
# override both together when pointing at a checkout elsewhere. The library is
# an ordinary Lake package rather than part of the compiler's share tree, so
# the target provisions it on first use at the revision the backend expects.
LEAN_SAIL_LIB       ?= $(abspath build/deps/lean-sail)
LEAN_SAIL_LIB_REQUIRE ?= .lake/packages/Sail
LEAN_SAIL_LIB_GIT   ?= https://github.com/rems-project/lean-sail
LEAN_SAIL_LIB_REV   ?= $(shell $(PYTHON_CONFIG) -m devtools.toolchains --manifest $(TOOLCHAIN_CONFIG) lean_sail_commit)
ROCQ_SEMANTIC_FLAGS := --coq-semantic-range-types --coq-undef-axioms
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
C_OPTIMIZED_EXTERNAL_TYPE_FLAGS := \
	$(foreach type,$(C_OPTIMIZED_EXTERNAL_TYPES),--c-optimized-external-type $(type)=$(C_OPTIMIZED_EXTERNAL_TYPES_HEADER)) \
	--c-optimized-external-type StackPointer=evmsail/host/stack.h
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
C_OPT_OPCODE_HANDLER_NAMES := $(shell sed -n 's/^function \(execute_[a-z0-9_]*\).*/\1/p' sail/evm/execute.sail)
C_OPT_OPCODE_HANDLER_PRESERVE_FLAGS := $(foreach name,$(C_OPT_OPCODE_HANDLER_NAMES),--c-preserve $(name))
C_OPT_PRESERVE_FLAGS := --c-preserve main \
                        --c-preserve resume_frame \
                        --c-preserve run_call \
			--c-preserve run_create \
			--c-preserve run_frame_entry_encoded \
			--c-preserve opcode_available \
			--c-preserve execute_push_encoded \
			--c-preserve execute_dup_encoded \
			--c-preserve execute_swap_encoded \
			--c-preserve execute_log_encoded \
                        --c-preserve execute_deep_stack_encoded \
                        --c-preserve frame_output \
                        --c-preserve opcode_frame_status \
                        --c-preserve exceptional_state \
                        --c-preserve account_execution_context \
                        --c-preserve refresh_account_execution_context \
                        --c-preserve validation_debug_record \
                        --c-preserve write_invalid_result \
                        --c-preserve sload_cost \
                        --c-preserve sstore_sentry_cost \
                        --c-preserve sstore_costs \
                        --c-preserve process_transaction \
                        --c-preserve compute_state_root \
                        --c-preserve decode_stateless_input_ref \
                        $(C_OPT_OPCODE_HANDLER_PRESERVE_FLAGS)
SAIL_CONTRACTS      :=
EXTERN_CONTRACT     := extractions/rocq/contract/ExternBoundary.v
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

.PHONY: publish-c-extraction extract build-extractions extract-c-spec build-c-spec extract-c-optimised build-c-optimised generate-rocq validate-rocq drift-rocq extract-rocq build-rocq-interface build-rocq publish-rocq generate-lean validate-lean drift-lean extract-lean build-lean publish-lean generate-python validate-python drift-python extract-python build-python publish-python all c-optimised c-optimised-format-generated c-optimised-clang-format c-optimised-clang-tidy c-optimised-compdb c-optimised-conformance c-optimised-evaluate c-optimised-evaluator-test c-optimised-format-report c-optimised-lint-report c-spec check check-contracts check-public-targets check-optimized-ffi check-optimized-ffi-manifest clean clear-z3-memo rocq-contracts-check docs-env docs-site eest-provision eest-verify eest-smoke verify-corpus verify-rocq verify-lean verify-python verify-c-spec verify-c-optimised verify-fast verify-docs extract ffi-clang-format-check fmt fmt-check help lean-extract lean-harness lint python-lint python-tools-check python-tools-fixture-smoke python-tools-fmt-check python-tools-format python-tools-format-check python-tools-lint python-tools-test python-tools-typecheck runtime-test sail-readability-lint-report zisk-guest

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
	@echo "  make c-optimised-compdb - write and smoke-check root clangd metadata"
	@echo "  make c-optimised-evaluate - write the versioned extraction-quality record"
	@echo "  make c-optimised-conformance - compile-check and audit optimized generated-C style"
	@echo "  make c-optimised-lint-report - run comprehensive Clang checks, ordered by compiler cleanup pass"
	@echo "  make c-optimised-clang-tidy - require all clang-tidy checks over generated and FFI C"
	@echo "  make c-optimised-format-report - report clang-format drift over generated and FFI C"
	@echo "  make c-optimised-clang-format - require generated and FFI C to match .clang-format"
	@echo "  make sail-readability-lint-report - inventory typed-Sail and common-Jib cleanup findings"
	@echo "  make check-optimized-ffi - enforce the allocation-free optimized C boundary"
	@echo "  make extract-rocq   - stage and validate the complete Rocq model"
	@echo "  make build-rocq     - compile the staged Rocq model"
	@echo "  make publish-rocq   - publish validated staged Rocq sources"
	@echo "  make extract-lean   - generate and compile the complete Lean model"
	@echo "  make lean-harness   - build the executable Lean fixture-harness library"
	@echo "  make docs-env       - create/update the repo-local uv documentation environment"
	@echo "  make extract-python - generate and smoke-test the complete Python model"
	@echo "  make python-lint    - lint generated Python with the locked Ruff version"
	@echo "  make python-tools-check - run all handwritten Python quality gates"
	@echo "  make python-tools-format - format handwritten Python with Ruff"
	@echo "  make python-tools-fixture-smoke - run one embedded v0.6.2 fixture"
	@echo "  make ffi-clang-format-check - require handwritten FFI C to match .clang-format"
	@echo "  make rocq-contracts-check - compile the hand-maintained Rocq extern contract"
	@echo "  make eest-provision - provision and authenticate the pinned EEST corpus"
	@echo "  make verify-corpus  - run the complete corpus on the optimized native model"
	@echo "  make docs-site      - build the literate specification book"
	@echo "  make zisk-guest     - build the production ZisK guest ELF"
	@echo "  make extract        - run all maintained model extractions"
	@echo "  make all            - check + lint + fmt-check"

check:
	$(SAIL) $(SAIL_Z3_FLAGS) $(MODEL)

# Preserve Sail's complete output and exit status in a bounded log, then run
# the tested multi-error banner-box checker over repository Sail sources.
lint:
	@$(PYTHON) -m devtools.build_support lint-command --log build/lint/sail.log -- \
		$(SAIL) $(SAIL_Z3_FLAGS) --all-warnings --lint-readability $(MODEL)
	@$(PYTHON) -m devtools.build_support comment-boxes $(SAIL_FILES)
	@echo "lint: clean"
	@$(PYTHON) -m devtools.docs.lint . || { echo "lint: FAILED (docs style)"; exit 1; }

# one sail call per file: the files $include each other, so a single multi-file
# invocation double-loads them and errors.
fmt:
	@rc=0; for f in $(SAIL_FILES); do $(SAIL) $(SAIL_Z3_FLAGS) --fmt --fmt-emit file "$$f" || { echo "  format failed: $$f"; rc=1; }; done; [ "$$rc" -eq 0 ] && echo "formatted $$(echo $(SAIL_FILES) | wc -w | tr -d ' ') file(s) with sail --fmt" || exit 1

fmt-check:
	@rc=0; for f in $(SAIL_FILES); do $(SAIL) $(SAIL_Z3_FLAGS) --fmt --fmt-emit stdout "$$f" 2>/dev/null | diff -q "$$f" - >/dev/null 2>&1 || { echo "  needs formatting: $$f"; rc=1; }; done; [ "$$rc" -eq 0 ] && echo "fmt-check: clean" || exit 1

runtime-test:
	$(PYTHON) zkvm/runtime/sail256/test_runtime.py

check-optimized-ffi-manifest:
	$(PYTHON) -m devtools.optimised_c.ffi --manifest-only

check-optimized-ffi:
	$(PYTHON) -m devtools.optimised_c.ffi

zisk-guest:
	bash zkvm/zisk/build.sh guest

eest-smoke: eest-provision
	@$(PYTHON) -m devtools.harness.cli --build optimized $(EEST_SMOKE) --limit 1 --quiet

eest-provision:
	$(PYTHON_CONFIG) -m devtools.corpus --manifest $(EEST_CORPUS_MANIFEST) provision \
		$(EEST_CORPUS) --cache $(EEST_CORPUS_CACHE)

eest-verify:
	$(PYTHON_CONFIG) -m devtools.corpus --manifest $(EEST_CORPUS_MANIFEST) verify $(EEST_CORPUS)

verify-corpus: eest-provision
	$(PYTHON) -m devtools.build_support run-command --log build/logs/corpus.log -- \
		$(PYTHON) -m devtools.harness.cli --build optimized --jobs $${EEST_JOBS:-8} --quiet $(EEST_CORPUS)

check-contracts:
	$(PYTHON_CONFIG) -m devtools.canaries --config $(CONTRACT_CANARIES) --root .

check-public-targets:
	$(PYTHON_CONFIG) -m devtools.target_contract --config $(PUBLIC_TARGETS) --makefile Makefile

generate-rocq: check-contracts
	rm -rf $(ROCQ_STAGE_DIR)
	mkdir -p $(ROCQ_MODEL_DIR)
	$(SAIL) $(SAIL_Z3_FLAGS) --coq $(ROCQ_SEMANTIC_FLAGS) --coq-output-dir $(ROCQ_MODEL_DIR) -o evm $(MODEL)
	$(PYTHON_CONFIG) -m devtools.rocq $(ROCQ_MODEL_DIR)

validate-rocq: generate-rocq
	$(PYTHON_CONFIG) -m devtools.canaries --config $(ROCQ_CANARIES) --root $(ROCQ_MODEL_DIR)

extract-rocq: validate-rocq

drift-rocq: validate-rocq
	$(PYTHON) -m devtools.build_support compare-trees $(ROCQ_COMMITTED_MODEL_DIR) $(ROCQ_MODEL_DIR)

build-rocq-interface: validate-rocq
	mkdir -p $(ROCQ_STAGE_DIR)/contract $(ROCQ_STAGE_DIR)/proofs
	$(ROCQ) c $(ROCQ_FLAGS) -noglob -o $(abspath $(ROCQ_STAGE_DIR)/contract)/ExternBoundary.vo $(EXTERN_CONTRACT)
	cd $(ROCQ_MODEL_DIR) && $(ROCQ) c $(ROCQ_FLAGS) evm_types.v
	cd $(ROCQ_MODEL_DIR) && $(ROCQ) c $(ROCQ_FLAGS) -o $(abspath $(ROCQ_STAGE_DIR)/proofs)/RlpCursor.vo $(abspath $(ROCQ_PROOFS_DIR))/RlpCursor.v

# The complete executable model is one very large transparent Rocq definition.
# Keep its exhaustive elaboration available as an explicit deep build, while CI
# verifies the regenerated source, type layer, proof, and extern contract below.
build-rocq: build-rocq-interface
	cd $(ROCQ_MODEL_DIR) && $(ROCQ) c $(ROCQ_FLAGS) evm.v

publish-rocq: validate-rocq
	$(PYTHON) -m devtools.build_support publish-tree $(ROCQ_MODEL_DIR) $(ROCQ_COMMITTED_MODEL_DIR)
	git diff --stat -- $(ROCQ_COMMITTED_MODEL_DIR)

verify-rocq: drift-rocq build-rocq-interface

# Both targets keep generated output in ignored build directories and
# compile-check it against the matching complete backend. The optimized model
# uses Sail's package/module layout; handwritten FFI remains under extractions/c/.
# The Sail model, not generated C, is the readable source of truth.
# Aggregates: extraction is Sail -> target language; build is target language
# -> compiled artifact. Splitting them means a generated tree stays available
# for inspection and publication even when its backend cannot compile it yet.
extract: extract-c-spec extract-c-optimised extract-rocq extract-lean extract-python
build-extractions: build-c-spec build-c-optimised build-rocq build-lean build-python

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
# implementation, matching the lean/rocq/python target layout. Sources only:
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
		--c-specialization-limit $(C_OPT_SPECIALIZATION_LIMIT) \
		$(C_OPT_INLINE_FLAGS) \
		$(C_OPT_PRESERVE_FLAGS) $(C_OPT_SPECIALIZE_LOG_FLAGS) \
		$(C_OPT_EXTRA_SAIL_FLAGS) \
		$(C_OPTIMISED_SPLICE_FLAGS) \
		$(MODEL) --variable EVM_DEBUG=off
	$(PYTHON) -m devtools.optimised_c.package $(C_OPT_GENERATED_DIR)
	test -s $(C_OPT_GENERATED_MANIFEST)
	test -s $(C_OPT_PACKAGE_MANIFEST)
	test -s $(C_OPT_GENERATED_INCLUDE_DIR)/$(C_OPT_PACKAGE)/spec.h

build-c-optimised: extract-c-optimised
	$(MAKE) --no-print-directory -C $(C_OPT_GENERATED_DIR) CC="$(CC)"
	test -s $(C_OPT_GENERATED_DIR)/libevmsail.a

# Backwards-compatible alias: extract then build.
c-optimised: build-c-optimised

# clangd discovers this root database automatically. It deliberately names
# generated translation units plus the original editable FFI sources, never
# the package's staged FFI copies.
c-optimised-compdb: extract-c-optimised
	$(PYTHON) -m devtools.optimised_c.compdb --sail $(SAIL) --clang $(CLANG) \
		--output $(C_OPT_COMPDB) --check $(C_OPT_GENERATED_DIR)

c-optimised-evaluator-test:
	$(PYTHON_PYTEST) -q devtools/tests/test_optimised_c_evaluator.py

# A valid record may intentionally contain red gates. The evaluator owns the
# package build so that a compilation failure is recorded instead of preventing
# publication. Use --require-pass when an experiment or release is ready to
# make every recorded blocking gate fatal.
c-optimised-evaluate: c-optimised-compdb
	$(PYTHON) -m devtools.optimised_c.evaluate --sail $(SAIL) --clang $(CLANG) \
		$(C_OPT_SAIL_SOURCE_ARG) \
		--purpose $(C_OPT_EVALUATION_PURPOSE) --compdb $(C_OPT_COMPDB) \
		--built-library $(C_OPT_GENERATED_DIR)/libevmsail.a \
		--output $(C_OPT_QUALITY_RECORD) --summary $(C_OPT_QUALITY_SUMMARY) \
		$(C_OPT_GENERATED_DIR)

c-optimised-conformance: c-optimised
	$(PYTHON) -m devtools.optimised_c.check \
		--waivers $(C_OPT_CONFORMANCE_WAIVERS) $(C_OPT_GENERATED_DIR)

# Advisory by default: this is the compiler-pass work queue. The deterministic
# conformance target and semantic fixtures remain gates while we burn down the
# baseline. Diagnostics are deduplicated and ordered by the pass that should
# own the fix instead of by translation-unit traversal order.
c-optimised-lint-report: c-optimised-conformance
	$(PYTHON) -m devtools.optimised_c.lint --sail $(SAIL) --clang $(CLANG) --clang-tidy $(CLANG_TIDY) \
		--baseline $(C_OPT_LINT_BASELINE) --profile $(LINT_PROFILE) \
		--jobs $(LINT_JOBS) $(C_OPT_GENERATED_DIR)

# This is intentionally separate from c-optimised-conformance: extraction and
# the deterministic source contract remain runnable on machines without
# clang-tidy, while reviewers and CI can opt into the richer AST diagnostics.
c-optimised-clang-tidy: c-optimised-format-generated
	$(PYTHON) -m devtools.optimised_c.lint --sail $(SAIL) --clang $(CLANG) --clang-tidy $(CLANG_TIDY) \
		--baseline $(C_OPT_LINT_BASELINE) --require-clang-tidy --strict \
		--profile $(LINT_PROFILE) --jobs $(LINT_JOBS) $(C_OPT_GENERATED_DIR)

# Keep the report target read-only. The explicit clang-format target produces
# the pretty-C review artifact from canonical compiler output and verifies both
# generated and handwritten optimized C against the same policy.
c-optimised-format-report: c-optimised-conformance
	$(PYTHON) -m devtools.optimised_c.format --clang-format $(CLANG_FORMAT) \
		--jobs $(LINT_JOBS) $(C_OPT_GENERATED_DIR)

c-optimised-format-generated: c-optimised-conformance
	$(PYTHON) -m devtools.optimised_c.format --clang-format $(CLANG_FORMAT) \
		--scope generated --fix --strict --jobs $(LINT_JOBS) $(C_OPT_GENERATED_DIR)

c-optimised-clang-format: c-optimised-format-generated
	$(PYTHON) -m devtools.optimised_c.format --clang-format $(CLANG_FORMAT) \
		--strict --jobs $(LINT_JOBS) $(C_OPT_GENERATED_DIR)

verify-c-spec: build-c-spec

# One extraction is shared across the optimized generated-C policy gates.
verify-c-optimised: c-optimised-clang-format c-optimised-clang-tidy

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
	$(PYTHON) -m devtools.docs.sail_readability \
		--source-log $(SAIL_READABILITY_SOURCE_LOG) --jib-log $(SAIL_READABILITY_JIB_LOG) \
		--output $(SAIL_READABILITY_REPORT) --json-output $(SAIL_READABILITY_JSON_REPORT)
	@cat $(SAIL_READABILITY_REPORT)

# Clone the support library the Lean backend expects, pinned to the revision
# it names. Lake would fetch it too, but only from a lakefile that records a
# git require; the committed lakefile records a path, so the checkout has to
# exist before generation rather than after it.
$(LEAN_SAIL_LIB)/lakefile.toml:
	mkdir -p $(dir $(LEAN_SAIL_LIB))
	rm -rf $(LEAN_SAIL_LIB)
	git init $(LEAN_SAIL_LIB)
	git -C $(LEAN_SAIL_LIB) remote add origin $(LEAN_SAIL_LIB_GIT)
	git -C $(LEAN_SAIL_LIB) fetch --depth 1 origin $(LEAN_SAIL_LIB_REV)
	git -C $(LEAN_SAIL_LIB) checkout --detach FETCH_HEAD
	test "$$(git -C $(LEAN_SAIL_LIB) rev-parse HEAD)" = "$(LEAN_SAIL_LIB_REV)"
	test -s $@

generate-lean: $(LEAN_SAIL_LIB)/lakefile.toml
	rm -rf $(LEAN_STAGE_DIR)
	mkdir -p $(LEAN_STAGE_DIR)
	$(SAIL) $(SAIL_Z3_FLAGS) --lean --lean-executable --lean-explicit-measures --lean-force-output --lean-source-root sail --lean-lib-path $(LEAN_SAIL_LIB_REQUIRE) --lean-specialization-file $(LEAN_SPECIALIZATION) --lean-import-file $(LEAN_HOST_AXIOMS) --lean-output-dir $(LEAN_STAGE_DIR) -o evm $(MODEL)
	mv $(LEAN_STAGE_DIR)/evm $(LEAN_MODEL_DIR)
	cp $(LEAN_COMMITTED_MODEL_DIR)/lake-manifest.json $(LEAN_MODEL_DIR)/lake-manifest.json
	rm -f $(LEAN_MODEL_DIR)/.gitignore
	$(PYTHON) -m devtools.build_support normalize-lean $(LEAN_MODEL_DIR)

validate-lean: generate-lean
	$(PYTHON_CONFIG) -m devtools.canaries --config $(LEAN_CANARIES) --root $(LEAN_MODEL_DIR)

extract-lean: validate-lean

drift-lean: validate-lean
	$(PYTHON) -m devtools.build_support compare-trees $(LEAN_COMMITTED_MODEL_DIR) $(LEAN_MODEL_DIR)

build-lean: validate-lean
	mkdir -p $(LEAN_MODEL_DIR)/.lake/packages
	ln -sfn $(LEAN_SAIL_LIB) $(LEAN_MODEL_DIR)/.lake/packages/Sail
	cd $(LEAN_MODEL_DIR) && $(LAKE) build

publish-lean: validate-lean
	$(PYTHON) -m devtools.build_support publish-tree $(LEAN_MODEL_DIR) $(LEAN_COMMITTED_MODEL_DIR)
	git diff --stat -- $(LEAN_COMMITTED_MODEL_DIR)

verify-lean: drift-lean build-lean

generate-python:
	rm -rf $(PYTHON_STAGE_DIR)
	mkdir -p $(PYTHON_SRC_ROOT) $(PYTHON_CACHE_DIR)
	$(SAIL) $(if $(SAIL_PYTHON_PLUGIN),-plugin $(SAIL_PYTHON_PLUGIN)) --python $(SAIL_PYTHON_FLAGS) -o $(PYTHON_PACKAGE) $(MODEL)
	$(PYTHON_CONFIG) -m devtools.python_extraction $(PYTHON_SRC_ROOT)

validate-python: generate-python
	$(PYTHON_CONFIG) -m devtools.canaries --config $(PYTHON_CANARIES) --root $(PYTHON_SRC_ROOT)

extract-python: validate-python

drift-python: validate-python
	$(PYTHON) -m devtools.build_support compare-trees $(PYTHON_COMMITTED_SRC_ROOT) $(PYTHON_SRC_ROOT)

build-python: validate-python
	mkdir -p $(PYTHON_CACHE_DIR)
	PYTHONPYCACHEPREFIX=$(PYTHON_CACHE_DIR) $(PYTHON_EVM) -m compileall -q $(PYTHON_PACKAGE)
	PYTHONPYCACHEPREFIX=$(PYTHON_CACHE_DIR) PYTHONPATH=$(abspath $(PYTHON_SRC_ROOT)):$(abspath $(PYTHON_DIR)) $(PYTHON_EVM) -m py_compile $(PYTHON_DIR)/adapter.py $(PYTHON_DIR)/smoke.py
	$(PYTHON_RUFF) check --select $(PYTHON_RUFF_RULES) --ignore $(PYTHON_RUFF_IGNORES) --output-format concise $(PYTHON_SRC_ROOT) $(PYTHON_DIR)/adapter.py $(PYTHON_DIR)/smoke.py
	PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=$(abspath $(PYTHON_SRC_ROOT)) $(PYTHON_EVM) $(PYTHON_DIR)/smoke.py

publish-python: validate-python
	$(PYTHON) -m devtools.build_support publish-tree $(PYTHON_SRC_ROOT) $(PYTHON_COMMITTED_SRC_ROOT)
	git diff --stat -- $(PYTHON_COMMITTED_SRC_ROOT)

verify-python: drift-python build-python

python-lint:
	test -s $(PYTHON_COMMITTED_SRC_ROOT)/evm/__init__.py
	$(PYTHON_RUFF) check --select $(PYTHON_RUFF_RULES) --ignore $(PYTHON_RUFF_IGNORES) --output-format concise $(PYTHON_COMMITTED_SRC_ROOT) $(PYTHON_DIR)/adapter.py $(PYTHON_DIR)/smoke.py

# Handwritten Python lives under one package tree. The generated model keeps
# the separate python-lint policy above.
PYTHON_TOOLS_DIRS := devtools

python-tools-check: python-tools-lint python-tools-format-check python-tools-typecheck python-tools-test

python-tools-lint:
	$(PYTHON_RUFF) check --output-format concise $(PYTHON_TOOLS_DIRS)

python-tools-format:
	$(PYTHON_RUFF) format $(PYTHON_TOOLS_DIRS)

python-tools-format-check:
	$(PYTHON_RUFF) format --check $(PYTHON_TOOLS_DIRS)

python-tools-fmt-check: python-tools-format-check

python-tools-typecheck:
	$(PYTHON_MYPY) devtools

python-tools-test:
	$(PYTHON_PYTEST) -q devtools/tests

python-tools-fixture-smoke:
	@$(PYTHON) -m devtools.harness.cli $(EEST_SMOKE) --limit 1 --quiet

# Handwritten FFI formatting only. Unlike c-optimised-clang-format this needs
# neither the custom Sail compiler nor a generated build tree, so it can run
# on any machine with clang-format and the repository .clang-format policy.
ffi-clang-format-check:
	$(PYTHON) -m devtools.optimised_c.format --clang-format $(CLANG_FORMAT) \
		--scope ffi --strict --jobs $(LINT_JOBS)

# Type-check the hand-maintained Rocq extern contract without regenerating the
# model (no custom Sail needed; requires the Rocq prover from opam).
rocq-contracts-check:
	mkdir -p build/rocq-contract
	$(ROCQ) c $(ROCQ_FLAGS) -noglob -o $(abspath build/rocq-contract)/ExternBoundary.vo $(EXTERN_CONTRACT)

all: check lint fmt-check

verify-fast: check lint fmt-check runtime-test check-contracts check-public-targets check-optimized-ffi python-tools-check

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

verify-docs: docs-site

# Compatibility spelling retained for the docs workflow.
lean-extract: extract-lean

lean-harness:
	bash $(LEAN_DIR)/runner/build_lib.sh

clean:
	@if [ -x '$(DOCS_BIN)/sail-book-gen' ]; then '$(DOCS_BIN)/sail-book-gen' --book '$(BOOK)' --clean; fi
	rm -rf $(C_SPEC_BUILD_DIR) $(C_OPT_BUILD_DIR) $(C_OPT_QUALITY_DIR) $(C_OPT_COMPDB) $(GENERATED_ROOT) build/rocq-contract $(PYTHON_CACHE_DIR) $(BOOK)/site $(BOOK_BUILD) $(BOOK)/docs/extraction $(BOOK)/docs/assets/generated

clear-z3-memo:
	rm -f "$(Z3_MEMO_PATH)" sail/sail_smt_cache
