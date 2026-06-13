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

.PHONY: all check clean help lint fmt fmt-check html pdf

help:
	@echo "evm-sail targets:"
	@echo "  make check          - type-check the model ($(MODEL))"
	@echo "  make lint           - sail --all-warnings on the program roots"
	@echo "  make fmt            - format every *.sail with sail --fmt"
	@echo "  make fmt-check      - verify *.sail match sail --fmt"
	@echo "  make all            - check + lint + fmt-check"
	@echo "  make html           - render the spec to docs/evm-sail.html"
	@echo "  make pdf            - typeset the spec to docs/evm-sail.pdf"

check:
	$(SAIL) $(MODEL)

# Two checks (each recipe is a single-line shell command; Make 3.81 has no
# .ONESHELL). (1) sail --all-warnings on the roots type-checks every reachable
# definition (sail has no $include-following autodiscovery, so files come from
# the `find` above). (2) banner-box alignment: inside a /* ===...=== */ box,
# every comment line must be the same width as the divider, so the closing */
# columns line up.
lint:
	@o=$$(for r in $(WARN_ROOTS); do $(SAIL) --all-warnings "$$r" 2>&1; done); if printf '%s\n' "$$o" | grep -qiE "warning|error"; then printf '%s\n' "$$o" | grep -iE "warning|error" | head -20; echo "lint: FAILED (sail warnings)"; exit 1; fi; \
	awk 'function ck(){if(n&&d)for(i=1;i<=n;i++)if(length(b[i])!=w){print f[i]":"l[i]": comment box width "length(b[i])" != "w;bad=1}} FNR==1{ck();n=0;d=0} /^\/\*.*\*\/$$/{b[++n]=$$0;l[n]=FNR;f[n]=FILENAME;if($$0~/^\/\* =+ \*\/$$/){d=1;w=length($$0)};next} {ck();n=0;d=0} END{ck();exit bad}' $(SAIL_FILES) || { echo "lint: FAILED (misaligned comment boxes)"; exit 1; }; \
	echo "lint: clean"

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

# Typeset the spec to a PDF via docs/spec.tex (listings). Same concatenation as
# `html`; perl normalizes the stray em-dash so the source is plain ASCII for TeX.
pdf:
	@order=$$(grep -E '^\$$include "' $(MODEL) | sed -E 's/.*"([^"]+)".*/\1/'); \
	: > docs/evm-sail.sail; \
	for p in $$order; do printf '\n/* === sail/%s === */\n\n' "$$p" >> docs/evm-sail.sail; perl -CSD -pe 's/\x{2014}/--/g' "sail/$$p" >> docs/evm-sail.sail; done; \
	( cd docs && pdflatex -interaction=nonstopmode -halt-on-error spec.tex >/dev/null && pdflatex -interaction=nonstopmode spec.tex >/dev/null ); \
	mv docs/spec.pdf docs/evm-sail.pdf; rm -f docs/spec.aux docs/spec.log docs/spec.out docs/spec.toc docs/evm-sail.sail; \
	echo "wrote docs/evm-sail.pdf ($$(wc -c < docs/evm-sail.pdf | tr -d ' ') bytes)"

clean:
	rm -rf sail_smt_cache sail/sail_smt_cache
