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

SAIL ?= sail
PYTHON ?= python3

PROJECT    := sail/evm.sail_project
MODEL      := $(PROJECT) evm
EEST_SMOKE := harness/fixtures/smoke/state_root_transfer.json
# Every Sail source owned by this repository, discovered rather than listed by
# hand.  Keep workspace-local worktrees and generated trees out of formatting.
SAIL_FILES := $(shell find sail proof/sail -name '*.sail' | sort)

.PHONY: all check clean help lint fmt fmt-check eest-smoke html pdf docs-site

help:
	@echo "evm-sail targets:"
	@echo "  make check          - type-check the model ($(MODEL))"
	@echo "  make lint           - sail --all-warnings on the program roots"
	@echo "  make fmt            - format every *.sail with sail --fmt"
	@echo "  make fmt-check      - verify *.sail match sail --fmt"
	@echo "  make eest-smoke     - run a tiny EEST fixture (root-checked)"
	@echo "  make all            - check + lint + fmt-check"
	@echo "  make html           - render the spec to docs/evm-sail.html"
	@echo "  make pdf            - typeset the spec to docs/evm-sail.pdf"

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

eest-smoke:
	@cd harness && $(PYTHON) run.py fixtures/smoke/state_root_transfer.json --fork Cancun --limit 1 --quiet

all: check lint fmt-check

# Build the MkDocs Material specification book from the sources: docinfo
# bundle -> sail-lsp semantic index -> generated pages (sail-book-gen) ->
# strict mkdocs build. Prose lives in /*md and /*! comments in the .sail
# sources. Requires uv, sail_lsp, and the mkdocstrings-sail package
# (override MKDOCSTRINGS_SAIL with a local path, e.g. the Sail repo's
# mkdocstrings-sail/ directory).
BOOK ?= book
MKDOCSTRINGS_SAIL ?= mkdocstrings-sail
# optional: a local ethereum/EIPs EIPS/ checkout enables EIP hover cards
EIPS_DIR ?=
docs-site:
	@mkdir -p $(BOOK)/doc $(BOOK)/docs
	$(SAIL) --doc --doc-format identity --doc-embed plain --doc-embed-with-location --doc-bundle doc.json -o $(BOOK)/doc $(PROJECT) evm --variable EVM_ENTRY=guest
	uv run --with '$(MKDOCSTRINGS_SAIL)' sail-lsp-index --sail '$(SAIL)' --root . --project $(PROJECT) --module evm --variable EVM_ENTRY=guest --output $(BOOK)/doc/lsp-index.json
	uv run --with '$(MKDOCSTRINGS_SAIL)' python -m mkdocstrings_handlers.sail._book --sail '$(SAIL)' --root . --project $(PROJECT) --module evm --variable EVM_ENTRY=guest --book $(BOOK) --site-name "EVM Sail Specification" $(if $(EIPS_DIR),--eips $(EIPS_DIR))
	cd $(BOOK) && uv run --with '$(MKDOCSTRINGS_SAIL)' --with mkdocs-material mkdocs build --strict -d site
	@echo "book: $(BOOK)/site/index.html"

# Render the whole spec to a single syntax-highlighted HTML page (sail --html).
# sail --html can't take the project module directly, so concatenate the
# project's files in evm.sail_project order into one source and render that.
html:
	@order=$$($(SAIL) --project $(PROJECT) --list-files evm); \
	tmp=$$(mktemp -d); : > $$tmp/evm-sail.sail; \
	for p in $$order; do label=$${p#sail/}; printf '\n/* === %s === */\n\n' "$$label" >> $$tmp/evm-sail.sail; cat "$$p" >> $$tmp/evm-sail.sail; done; \
	( cd $$tmp && $(SAIL) evm-sail.sail --html -o html ); \
	mkdir -p docs && cp $$tmp/html/evm-sail.html docs/evm-sail.html && rm -rf $$tmp; \
	echo "wrote docs/evm-sail.html ($$(wc -c < docs/evm-sail.html | tr -d ' ') bytes)"

# Typeset the spec to a PDF via docs/spec.tex (listings). Same concatenation as
# `html`; perl normalizes the stray em-dash so the source is plain ASCII for TeX.
pdf:
	@order=$$($(SAIL) --project $(PROJECT) --list-files evm); \
	: > docs/evm-sail.sail; \
	for p in $$order; do label=$${p#sail/}; printf '\n/* === %s === */\n\n' "$$label" >> docs/evm-sail.sail; perl -CSD -pe 's/\x{2014}/--/g' "$$p" >> docs/evm-sail.sail; done; \
	( cd docs && pdflatex -interaction=nonstopmode -halt-on-error spec.tex >/dev/null && pdflatex -interaction=nonstopmode spec.tex >/dev/null ); \
	mv docs/spec.pdf docs/evm-sail.pdf; rm -f docs/spec.aux docs/spec.log docs/spec.out docs/spec.toc docs/evm-sail.sail; \
	echo "wrote docs/evm-sail.pdf ($$(wc -c < docs/evm-sail.pdf | tr -d ' ') bytes)"

clean:
	rm -rf sail_smt_cache sail/sail_smt_cache
