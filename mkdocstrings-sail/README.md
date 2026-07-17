# mkdocstrings-sail

A [mkdocstrings](https://mkdocstrings.github.io/) handler for
[Sail](https://github.com/rems-project/sail), letting hand-written MkDocs
pages embed live Sail definitions with `::: identifier` directives. It also
registers a Pygments lexer for Sail, so every ` ```sail ` code fence in the
site is syntax highlighted.

The handler consumes the docinfo JSON bundle produced by the Sail compiler
(no Sail parsing in Python):

```sh
sail --doc --doc-format identity --doc-embed plain --doc-embed-with-location \
     --doc-bundle doc.json -o doc <files or project>
```

## Usage

```yaml
# mkdocs.yml
theme:
  name: material
plugins:
  - search
  - mkdocstrings:
      default_handler: sail
      handlers:
        sail:
          bundle: doc/doc.json   # relative to mkdocs.yml
```

Then, in any Markdown page:

```markdown
The interpreter advances one instruction at a time:

::: step

Registers referenced above link to their definitions, wherever they are
rendered — see [PC][register-PC], or just [step][] in prose.
```

Doc comments on `type` definitions need a Sail whose docinfo emits them
(the accompanying docinfo patch, > 0.20.2); with older bundles types render
without prose and everything else works.

Each rendered definition gets:

- a heading with a stable `<kind>-<id>` anchor (`#function-step`,
  `#register-PC`, …), derived only from the definition kind and name;
- its Sail doc comment (`/*! … */`) rendered as Markdown;
- its source, highlighted by the bundled Sail lexer, with function calls and
  register uses wrapped in `<autoref>` elements that
  [mkdocs-autorefs](https://mkdocstrings.github.io/autorefs/) resolves to
  whichever page renders the target definition. Unresolvable references
  degrade to plain text (`optional` autorefs), so partial sites build clean.

## Optional: semantic highlighting and full identifier links via sail-lsp

The docinfo bundle only records *function call* and *register use* sites.
With a [Sail language server](https://github.com/rems-project) available,
the bundled `sail-lsp-index` tool captures semantic tokens
(`textDocument/semanticTokens/full`) and the server's `sail/sourceMap`
reference graph into one JSON index:

```sh
# run from the project root so paths match the docinfo bundle
sail-lsp-index --root . --project model.sail_project --module core \
               --output doc/lsp-index.json
```

`--project` resolves the model's exact file closure via
`sail --list-files`, which is strongly recommended: the fallback (a
workspace-wide `*.sail` glob, or an explicit file list) can index files
outside the model, and same-named definitions in stray files pollute the
name-based reference graph. `--binary`/`--sail` override which `sail_lsp`
and `sail` executables are used.

```yaml
handlers:
  sail:
    bundle: doc/doc.json
    lsp_index: doc/lsp-index.json   # optional
```

With the index configured, rendered source gains:

- **semantic highlighting** (functions, types, registers, enum members
  classified by the compiler rather than by regex), falling back to the
  lexical lexer for files the index doesn't cover;
- **links on every resolved identifier** — type references, constructor
  uses (anchored at their owning union/enum), mappings, and top-level lets,
  in addition to docinfo's call/register links. docinfo's compiler-proven
  links always take precedence where spans overlap; `sourceMap` references
  are name-based, so heavily overloaded names may need care;
- **hover tooltips**: linked identifiers carry a `title` composed of the
  target's type signature (from `sourceMap`) and the first sentence of its
  doc comment (from the bundle), e.g. `word -> word — Increment a word by
  one.`, followed by a truncated source preview of the target's body
  (`hover_previews: false` to disable) — the same content the Sail LSP
  shows on editor hover, sourced from the bundle. Definition headings
  carry their signature. Browsers show these
  natively; add `content.tooltips` under `theme.features` for styled
  Material tooltips, and disable autorefs' own link titles so they don't
  compete:

  ```yaml
  plugins:
    - autorefs:
        link_titles: false
  ```

  With `hover_cards` (default), in-code links instead get **IDE-style hover
  cards** — the target's header (`kind name : signature`), its doc comment
  rendered as Markdown, and its body syntax-highlighted — matching the Sail
  LSP's editor hover. Cards are embedded as inert `<template>` elements and
  positioned by `assets/sail-hover.js` (`sail-book-gen` installs and wires
  it; for hand-written configs copy it from the package and list it under
  `extra_javascript`). Without the script, links fall back gracefully (set
  `hover_cards: false` to restore plain-text `title` tooltips). Definition
  headings keep their signature `title` either way.

  Identifiers whose target is not rendered anywhere in the site degrade to
  plain text titled with the would-be anchor (mkdocs-autorefs' fallback for
  unresolved optional references).

## Options

Global (under `handlers.sail.options`) or per-directive:

| Option | Default | Effect |
| ------ | ------- | ------ |
| `kind` | `null` | Restrict lookup: `function`, `mapping`, `type`, `register`, `let`, `val`, `anchor`, `span` |
| `heading_level` | `2` | Heading level of the rendered definition |
| `show_comment` | `true` | Render the doc comment |
| `show_source` | `true` | Render the definition source |
| `link_code` | `true` | Wrap use sites in the source with cross-reference links |
| `hover_previews` | `true` | Append a truncated body preview of the target to link tooltips |
| `hover_cards` | `true` | Emit IDE-style hover cards (rendered comment + highlighted body) for in-code links |
| `toc_label` | identifier | Label in the page table of contents |

Per-directive options nest under an `options:` key (mkdocstrings syntax):

```markdown
::: increment
    options:
      kind: val
      heading_level: 3
```

Bare identifiers resolve by priority (`function` before `val`, etc.); use
`kind:` to disambiguate as above.

## Design notes

**Why MkDocs Material?** It consumes plain Markdown with no JavaScript
build toolchain, it is the established convention for Ethereum
specification sites (consensus-specs), and versioning, search, permalinks,
and code-copy are configuration rather than code. mdBook has no
mike-equivalent versioned-deployment story and its single-`SUMMARY.md`
model fits hand-authored narratives poorly once content is generated;
Docusaurus requires a Node/MDX toolchain and copies docs trees per version,
which suits hand-maintained docs rather than regenerated output.

**Anchors.** Rendered definitions get stable `<kind>-<id>` heading anchors
(`#function-step`), derived only from the definition kind and identifier,
so deep links survive regeneration as long as definitions keep their names.

**Versioning with mike.** Set `extra.version.provider: mike` in
`mkdocs.yml`, then deploy each fork/branch/hard-fork variant as a mike
version — `mike deploy --push --update-aliases main latest`,
`mike deploy --push prague`, etc., with `mike set-default --push latest`
once. mike stores each version in its own subdirectory of `gh-pages` with a
`versions.json` manifest, and Material renders a version selector listing
every deployed variant side by side. CI runs bundle generation + `mike
deploy` per published branch, using the branch name as the version name.

## Literate Sail: generating the whole book from sources

`sail-book-gen` generates the complete book directly from the Sail sources,
so no intermediary Markdown files are needed. Prose lives in the sources at
three levels:

- ``/*md ... */`` — Markdown prose blocks. Ordinary Sail comments (invisible
  to the compiler, preserved by `sail --fmt`), interleaved with definitions
  by source position: a block at the top of a file starting with a
  `# Title` heading is the module doc (and names the page in the nav);
  `## Section` blocks between definitions open sections; heading level and
  position express the whole hierarchy with one construct.
- `/*! ... */` — definition-level doc comments, rendered under the
  definition's heading (and summarized into hover tooltips).
- Ordinary `/* ... */` comments stay code-only: visible in source fences
  when inside a definition's span, otherwise omitted.

```sh
sail-book-gen --root . --project model.sail_project --module core \
              --book book --site-name "My Spec"
```

emits one page per source file under `book/docs/reference/` (`/*md` blocks
and `::: name` directives in source order) and `mkdocs.yml` (`--no-config`
to keep yours). Hand-authored pages under `book/docs/` still work:
identifiers they render are skipped in the generated pages.

A `mod.md` in a source directory is that directory's section overview: it
is rendered (EIP references linked) as the directory's `index.md` under
`reference/`, and [mkdocs-section-index] attaches it to the nav section,
so the section title itself opens the overview. List it as the section's
first entry in `SUMMARY.md`.

Navigation uses [mkdocs-literate-nav]: author `book/docs/SUMMARY.md` as a
Markdown list of `[Title](path.md)` links, `Section` headers, and `*.md`
globs to control reading order and section grouping; globs pick up pages
you don't list explicitly (titled by their `# Title` heading), so new
source files appear without touching the nav. When no `SUMMARY.md` exists,
a default is generated that includes everything in alphabetical order.

`EIP-N` references in prose become links carrying full-document cards.
Card content is exclusively **rendered client-side from the actual
source**: on first view the EIP's markdown is fetched from the canonical
`ethereum/EIPs` repository, rendered in the browser (vendored `marked`
12, MIT + `highlight.js` 11, BSD-3; untagged code fences default to
Python per EIP-1 convention; raw HTML is sanitized), and cached in
`localStorage` for a day — upstream EIP edits appear without rebuilding
the site, and no local EIPs checkout or build-time EIP content exists.

With `--lean <dir>` pointing at a project extracted by `sail --lean`,
`sail-book-gen` renders a Lean-extraction section under
`extraction/lean/`: the backend on this branch carries `/*!` doc
comments into the extracted code as `/-- … -/` docstrings and each
file's leading `/*md` block as a `/-! … -/` module docstring, and the
generator renders those back as page prose (EIP references linked)
around `lean4`-highlighted source. Add the section to `SUMMARY.md` with
globs, e.g. `- Lean extraction:` / `extraction/lean/Out/*.md`.

Lean code gets the same hover and navigation experience as Sail code:
a build-time index over the extracted modules (identifier names carry
over from Sail verbatim) anchors every definition site, links every
identifier use to its definition — cross-page, including inductive
constructors resolved to their owning type — and attaches IDE-style
hover cards (docstring + highlighted body). Cards are per-definition
fragments under `assets/lean-cards/`, fetched on first hover so pages
referencing hundreds of definitions stay small.

[mkdocs-literate-nav]: https://oprypin.github.io/mkdocs-literate-nav/
[mkdocs-section-index]: https://oprypin.github.io/mkdocs-section-index/

## Development

```sh
uv run --with-editable . python -m unittest discover tests   # unit tests
tests/run_integration.sh                            # end-to-end mkdocs build
```

Sail's docinfo format is versioned (`"version": 1`); the handler refuses
bundles with other versions.

## Ecosystem note (2026)

MkDocs 1.x is effectively frozen (last release 1.6.1, Aug 2024) and the
announced MkDocs 2.0 removes the plugin system this handler relies on, so
dependencies are pinned to `mkdocs<2`. The stack still builds fine on the
frozen 1.x line; drop-in continuations (ProperDocs) and the Material team's
successor (Zensical, which reads `mkdocs.yml` and is where the mkdocstrings
author now works) are the migration paths if that changes. The handler's
inputs (docinfo JSON) and outputs (plain HTML with stable anchors) are
deliberately generator-neutral, so a port targets a small surface.
