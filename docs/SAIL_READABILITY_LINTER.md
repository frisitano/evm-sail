# Sail Readability Linter

The custom Sail compiler exposes `--lint-readability` as a backend-neutral
cleanup inventory. Parser and typed-Sail rules are source-actionable and are
enforced by the ordinary `make lint` gate. Post-Jib diagnostics describe
compiler-created structure and remain advisory.

Run the complete source and optimized-lowering inventory with:

```sh
make sail-readability-lint-report
```

The compact report is written to
`build/lint/sail-readability/report.txt`, with the same stable data available
as `report.json` for CI. Complete diagnostics, including source excerpts,
remain in `source.log` and `jib.log` beside it.

## Parser Sail rules

These checks run on the authored syntax before operator and literal
elaboration can introduce compiler helper calls:

- `sail-nested-function-call`: a function argument contains another explicitly
  written named function call. Bind the inner result to a semantic name before
  the outer call.
- `sail-function-call-condition`: an `if`, `while`/`until`, match
  subject/guard, or assertion predicate contains an explicitly written named
  function call. Bind the predicate or subject result before control-flow
  selection.
- `sail-identity-conditional`: an authored conditional returns `true`/`false`
  (or the inverse) directly from its condition.
- `sail-constant-conditional`: an authored conditional has a literal
  condition.
- `sail-else-after-terminal`: an explicit else arm follows a return, throw, or
  exit.
- `sail-prefer-early-return`: an explicit function-tail conditional has one
  unit arm and one non-empty arm. Returning from the empty arm permits the
  continuation to be flattened without changing the function result; non-tail
  conditionals are excluded.
- `sail-nested-else-if`: an `else` block contains only another conditional and
  can be written as a direct `else if` chain.

Ordinary infix operators are deliberately not treated as named calls. This
keeps the rule about source structure rather than compiler-internal overload
or literal helpers.

## Typed Sail rules

These checks run after type checking and transitive effect inference:

- `sail-redundant-bool`: double negation and equality with boolean literals.
- `sail-duplicate-branches`: a pure condition selecting structurally identical
  branches, corresponding to Clang's `bugprone-branch-clone` check.
- `sail-empty-conditional`: a pure condition selecting two unit literals.
- `sail-conditional-assignment`: both arms assigning a value to the same
  simple local, identifying a source-level conditional-value assignment.
- `sail-trivial-alias`: an exact `let x = expression in x` alias.
- `sail-single-use-temporary`: a pure, single-use generically named binding.
- `sail-dead-pure-binding`: an unused wildcard binding of a pure expression.

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
- `jib-dead-pure-temporary`: an unread generated temporary initialized from a
  pure Jib value.
- `jib-redundant-bool`: literal boolean operations and double negation.
- `jib-constant-conditional`: a conditional with a literal condition.
- `jib-empty-conditional`: a conditional with two empty branches.
- `jib-duplicate-branches`: structurally identical branch bodies, corresponding
  to Clang's `bugprone-branch-clone` check.
- `jib-conditional-assignment`: two pure branch values assigned to the same
  destination, identifying the exact join a value-selecting backend may render
  as one conditional assignment.
- `jib-else-after-terminal`: an else branch retained after a terminal return,
  throw, or exit, corresponding to `readability-else-after-return`.
- `jib-partial-branch-initialization`: an immediately consumed local is
  initialized by only one branch that can reach the read, covering the common
  lowering shape behind Clang's `-Wuninitialized` branch diagnostics.
- `jib-identity-copy`: a local copied to itself.
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

Branch checks are divided deliberately:

- parser Sail diagnoses authored literal/identity conditions and terminal else
  arms;
- typed Sail diagnoses effect-sensitive duplicate arms and same-local
  conditional assignments;
- Jib additionally diagnoses partial fallthrough initialization, redundant
  joins, and dead labels;
- Sail's pattern checker owns source enum/union exhaustiveness;
- Clang's `-Wswitch-enum` verifies the generated C mapping; and
- branch polarity is not a generic warning because failure-first guard clauses
  are often intentional; the parser only recommends a guard clause for the
  provably safe function-tail unit-arm case.

Clang remains authoritative for C-only contracts that cannot be expressed as
Sail/Jib readability rules: ABI and prototype compatibility, linkage, pointer
constness and aliasing, alignment, final C enum switches, include ownership,
format strings, recursion/stack policy, `_Noreturn`, and static-storage
initialization. `make c-optimised-clang-tidy` enforces that layer.
