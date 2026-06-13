# ===========================================================================
# evm-sail — specification validation entry point
#
#   make check          type-check the specification (sail/evm.sail)
#   make lint           sail --all-warnings on the program roots
#   make fmt            format every *.sail in place with `sail --fmt`
#   make fmt-check      verify every *.sail matches `sail --fmt`
#   make all            check + lint + fmt-check
#   make clean          remove build artifacts
#
# Requires the rems-project Sail toolchain (`sail`) on PATH, installed via
# opam (NOT `brew install sail`, which is an unrelated tool). See README.md.
#
# Block EXECUTION (compile to C and run) is validated by the EEST harness
# (revm-eest/: run_eest.py over sail/runner.sail) and the zkVM guest (zkvm/).
# ===========================================================================

SAIL ?= sail

MODEL      := sail/evm.sail
# every Sail source, discovered (not a hand-maintained list)
SAIL_FILES := $(shell find . -name '*.sail' | sort)
# program roots whose $include graph reaches every definition; --all-warnings
# here type-checks the whole tree (formatting/hygiene is covered by fmt-check,
# since `sail --fmt` normalizes trailing whitespace, tabs, and final newlines).
WARN_ROOTS := sail/evm.sail zkvm/zkvm_block.sail

.PHONY: all check clean help lint fmt fmt-check html

help:
	@echo "evm-sail targets:"
	@echo "  make check          - type-check the model ($(MODEL))"
	@echo "  make lint           - sail --all-warnings on the program roots"
	@echo "  make fmt            - format every *.sail with sail --fmt"
	@echo "  make fmt-check      - verify *.sail match sail --fmt"
	@echo "  make all            - check + lint + fmt-check"
	@echo "  make html           - render the spec to docs/evm-sail.html"

check:
	$(SAIL) $(MODEL)

# sail has no $include-following autodiscovery for these, so files come from the
# `find` above; each recipe is a single-line shell command (Make 3.81 has no
# .ONESHELL). --all-warnings on the roots type-checks every reachable definition.
lint:
	@o=$$(for r in $(WARN_ROOTS); do $(SAIL) --all-warnings "$$r" 2>&1; done); if printf '%s\n' "$$o" | grep -qiE "warning|error"; then printf '%s\n' "$$o" | grep -iE "warning|error" | head -20; echo "lint: FAILED"; exit 1; else echo "lint: clean"; fi

# one sail call per file: the files $include each other, so a single multi-file
# invocation double-loads them and errors.
fmt:
	@for f in $(SAIL_FILES); do $(SAIL) --fmt --fmt-emit file "$$f"; done; echo "formatted $$(echo $(SAIL_FILES) | wc -w | tr -d ' ') file(s) with sail --fmt"

fmt-check:
	@rc=0; for f in $(SAIL_FILES); do $(SAIL) --fmt --fmt-emit stdout "$$f" 2>/dev/null | diff -q "$$f" - >/dev/null 2>&1 || { echo "  needs formatting: $$f"; rc=1; }; done; [ "$$rc" -eq 0 ] && echo "fmt-check: clean" || exit 1

all: check lint fmt-check

# Render the whole spec to a single syntax-highlighted HTML page (sail --html).
# sail --html can't take the cross-including fileset at once, so concatenate the
# modules in evm.sail's $include order into one source and render that.
html:
	@order=$$(grep -E '^\$$include "' $(MODEL) | sed -E 's/.*"([^"]+)".*/\1/'); \
	tmp=$$(mktemp -d); : > $$tmp/evm-sail.sail; \
	for p in $$order; do printf '\n/* === sail/%s === */\n\n' "$$p" >> $$tmp/evm-sail.sail; cat "sail/$$p" >> $$tmp/evm-sail.sail; done; \
	( cd $$tmp && $(SAIL) evm-sail.sail --html -o html ); \
	mkdir -p docs && cp $$tmp/html/evm-sail.html docs/evm-sail.html && rm -rf $$tmp; \
	echo "wrote docs/evm-sail.html ($$(wc -c < docs/evm-sail.html | tr -d ' ') bytes)"

clean:
	rm -rf sail_smt_cache sail/sail_smt_cache
