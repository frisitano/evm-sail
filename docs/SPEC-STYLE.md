# Specification style

Conventions for the documentation comments that produce the specification
book. Enforced by `tools/docs_lint.py` (part of `make lint`).

## Section overviews

Every definition-bearing source directory carries a `mod.md`: the
section's overview page, opened by clicking the section title in the
nav. It states the section's scope in a few paragraphs and guides the
reading order with links to the section's pages. Its `# Title` matches
the nav section title (40 characters or fewer); headings carry no
citations; the same banned-vocabulary and prose rules apply as for
source pages, and host-interface overviews carry the Non-normative
banner.

## Page structure

Each source file is one page. Its first `/*md` block must open with a
`# Title` heading followed by the introduction (no "Introduction" heading).
Subsequent `##` sections come from a controlled vocabulary, in this
canonical order where the code permits:

| Section | Content |
| ------- | ------- |
| `## Constants` | Protocol constants; a summary table when four or more are related |
| `## Types` | The page's data types and containers |
| `## Helpers` | Supporting definitions, with free-form `###` topic subsections |
| `## The …` | An operative unit (e.g. `The run loop`, `The trie builder`); one per normative core, named as a definite noun phrase |

`###` subsections are free-form under any `##` section.

## Headings

Headings never carry citations — no `EIP-…` or Yellow Paper references in
heading text. Citations belong in the section's or definition's prose, so
heading text and anchors stay stable as citation sets evolve.

## Definition documentation (`/*!`)

- The first sentence is definitional and self-contained, ending in a
  period (it is rendered as the hover summary).
- Cite the Yellow Paper section or EIP a rule implements inline
  (`(YP §9.4.2)`, `(EIP-2929)`).
- Non-normative content — implementation strategy, performance, proof
  notes — goes in a `!!! note "Implementation"` admonition, never in the
  definitional prose.
- The specification describes the protocol; never reference other
  specification codebases, the documentation tooling, or how this document
  is produced.

## Ordering

Definitions cannot be reordered for documentation (documentation changes
must be comments-only). For new code, declare constants before types
before helpers before the operative functions, so pages follow the
canonical section order naturally.

## Coverage

In every module, including the host interface:

- every `type`, `enum`, `struct`, `union`, `register`, and `mapping`
  carries a `/*!` doc comment;
- every substantial function (four lines or more) carries `/*!`;
  one-line definitions may rely on their section's prose;
- every `val` carries `/*!` unless it declares a function defined
  elsewhere in the specification (the function carries the docs) — in
  particular, every host-interface extern `val` documents its contract;
- a page declaring four or more capitalized constants has a
  `## Constants` section or a summary table;
- within every `enum` and `union` body, each member line is covered by
  an ordinary comment above it. A comment may head the contiguous group
  of members it introduces (a blank line ends a group), as with the
  instruction set's opcode ranges. When an enum or union member, or a
  struct field, carries its own rule, place its comment immediately above
  its declaration, never to the right of it. Member comments are rendered
  with the definition source.

Every page's introduction contains prose, not just headings.

## Non-normative pages

Pages documenting the host interface (`sail/host/`, excluding
`sail/host/kernel/`) carry a `!!! note "Non-normative"` banner immediately
after the introduction.
