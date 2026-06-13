# ===========================================================================
# evm-sail — specification validation entry point
#
#   make check          type-check the specification (evm/evm.sail)
#   make lint           sail --all-warnings + source hygiene (trailing ws/tabs/newline)
#   make fmt            format *.sail with the official `sail --fmt` (opt-in)
#   make fmt-check      verify *.sail match sail --fmt
#   make all            check + lint + fmt-check
#   make clean          remove build artifacts
#
# Requires the rems-project Sail toolchain (`sail`) on PATH, installed via
# opam (NOT `brew install sail`, which is an unrelated tool). See README.md.
#
# Block EXECUTION (compile to C and run) is validated by the EEST harness
# (revm-eest/: run_eest.py over evm/runner.sail) and the zkVM guest (zkvm/).
# ===========================================================================

SAIL ?= sail

MODEL := evm/evm.sail

.PHONY: all check clean help lint fmt fmt-check

help:
	@echo "evm-sail targets:"
	@echo "  make check          - type-check the model ($(MODEL))"
	@echo "  make lint           - sail --all-warnings + source hygiene"
	@echo "  make fmt            - format every *.sail with sail --fmt (opinionated; opt-in)"
	@echo "  make fmt-check      - verify *.sail match sail --fmt"
	@echo "  make all            - check + lint + fmt-check"

check:
	$(SAIL) $(MODEL)

lint:
	@SAIL=$(SAIL) bash scripts/lint-sail.sh

fmt:
	@SAIL=$(SAIL) bash scripts/fmt-sail.sh

fmt-check:
	@SAIL=$(SAIL) bash scripts/fmt-sail.sh --check

all: check lint fmt-check

clean:
	rm -rf sail_smt_cache evm/sail_smt_cache
