# Sail Readability Linter

The custom Sail compiler exposes `--lint-readability` as an opt-in,
backend-neutral cleanup inventory. It does not change model semantics or make
readability warnings part of the ordinary `make lint` gate.

Run the complete source and optimized-lowering inventory with:

```sh
make sail-readability-lint-report
```

The compact report is written to
`build/lint/sail-readability/report.txt`. Complete diagnostics, including
source excerpts, remain in `source.log` and `jib.log` beside it.

## Typed Sail rules

These checks run after type checking and transitive effect inference:

- `sail-redundant-bool`: double negation and equality with boolean literals.
- `sail-identity-conditional`: conditionals equivalent to their condition or
  its negation.
- `sail-trivial-alias`: an exact `let x = expression in x` alias.

The rules preserve evaluation count and order. A named effectful expression
used more than once is deliberately not treated as an inlineable alias.
Scope cleanup is deliberately deferred to Jib: the typed Sail AST represents a
multi-statement braced body as one nested expression, so source-level block
cardinality would not be a reliable readability signal.

## Common Jib rules

These checks run after the common Jib lowering and representation
specialization, before any backend renders the program:

- `jib-declaration-assignment-split`: a declaration immediately followed by
  assignment to the same local.
- `jib-unit-plumbing`: a unit-valued local that a backend can erase.
- `jib-single-use-pure-temporary`: a generated, source-tracked, single-use
  pure value that can be inlined without changing effect order.
- `jib-redundant-join`: a jump to the immediately following label.
- `jib-redundant-scope`: an empty or single-instruction Jib block.
- `jib-dead-label`: a label with no incoming Jib jump.
- `jib-lost-source-name`: a generated local without source-name provenance.

These diagnostics identify structure introduced by lowering. A backend may
legitimately discharge them in a presentation pass; they do not imply that
the Sail source should be rewritten.

## Relationship to Clang checks

The source and Jib rules encode the cleanup lessons that are meaningful before
C emission: boolean simplification, unit erasure opportunities, pure temporary
inlining, declaration-plus-initialization, trivial scope removal, join/label
cleanup, and source-name retention.

Clang remains authoritative for C-only contracts that cannot be expressed as
Sail/Jib readability rules: ABI and prototype compatibility, linkage, pointer
constness and aliasing, alignment, enum exhaustiveness, include ownership,
format strings, recursion/stack policy, `_Noreturn`, and static-storage
initialization. `make c-optimised-clang-tidy` enforces that layer.
