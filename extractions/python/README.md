# Python extraction

This directory contains the complete executable Python extraction of
`sail/evm.sail_project`, plus a deliberately small EVM-facing adapter and
smoke test. The generated `evm/` package is lowered directly from Sail's
checked typed AST and keeps the source model's nominal dataclasses, enums,
aliases, and typed functions; it is not a serialized JIB program. Generated
package files are kept in version control and must not be edited by hand.

## Generate and validate

With an installed Sail Python plugin:

```sh
rtk make extract-python
```

To validate a different build of the compiler, point `SAIL` at it. An
installed compiler carries its own plugins, so nothing else is needed:

```sh
make SAIL=/path/to/sail extract-python
```

The target regenerates the complete model with an embedded runtime, compiles
the package plus its adapter and smoke test, and runs `smoke.py`. This
repository opts into `--python-preserve-structure`, `--python-split`, and
`--python-source-root sail`. It also selects `--python-pydantic` and maps the
Sail `address` and `hash` aliases to `Bytes20` and `Bytes32`. Generation and
validation require `ethereum-types==0.4.1` for the complete numeric, enum, and
fixed-byte representation model, plus Pydantic 2.12.5 for dependent record
validation. Functions use direct Python control flow where possible and are
placed in modules matching the Sail source tree.

The generated specification trusts Sail's successful type checking rather
than dynamically repairing function arguments, returns, registers, constants,
or host-extern results. The lowering does emit the integer projections and
nominal result constructors required to implement arithmetic over the mapped
`ethereum-types` classes: intermediates use Python mathematical integers and
typed results reconstruct the declared unsigned representation. Conversions
explicitly present in Sail remain explicit Python constructors. Host adapters
are responsible for satisfying their declared Sail signatures. Override
`SAIL_PYTHON_FLAGS` only when selecting a different output presentation or
integration contract.

The package layout separates concerns while retaining one public `evm` API:

- each source-aligned module contains its own records, unions, constructors,
  enums, aliases, functions, immutable top-level values, and registers—for
  example, `ByteSlice` is defined in `evm/primitives/bytes.py`, while machine
  registers are defined in `evm/evm/machine.py`;
- `evm/_types.py` is an imports-only compatibility aggregator that re-exports
  those source-local types;
- `evm/__init__.py` re-exports immutable declarations, exposes current
  register values dynamically, contains metadata, and coordinates the
  compatibility `reset()`/`finish()` API;
- `evm/primitives/`, `evm/host/`, `evm/evm/`, `evm/lib/`, and
  `evm/executor/` mirror the specification's Sail files; and
- `evm/_runtime.py` contains the Sail Python runtime. Source-aligned modules
  import all external numeric, enum, and fixed-byte representations from their
  relative `_runtime`, keeping the mandatory `ethereum-types` dependency
  behind one generated boundary. `_runtime.py` is the only generated package
  module that imports `ethereum_types`; the copied `evm/HostContract.py`
  imports the representations it needs from `_runtime`.

Functions call neighbors in the same Sail file directly. Cross-file references
to types, functions, and immutable top-level values normally use exact imports
from the complete generated package path—for example,
`from evm.primitives.bytes import bytes_list` and
`from evm.lib.bytes import word_to_bytes32`. The backend analyzes the
source-module dependency graph; cyclic value and type edges import the
readable owner module and use qualified access instead. Every import is at the
top of its file, with package aggregation ordered to avoid reading partially
initialized symbols. Mutable registers are never copied through a direct import. Their
owner module reads and writes the Python global directly; a function in
another Sail source imports that owner module and uses qualified access such
as `machine.pc = 0`. Top-level Sail `let` values and registers are initialized
inline in their owner modules. Package `reset()` calls private owner-local
helpers that declare exactly the register globals they reset. The extraction
contains no `_model.py`, separate lifecycle module, or `E402` suppression.

Named Sail `let`, `var`, and assignment destinations remain named in generated
control flow. Pure one-expression blocks and conditionals use direct Python
expressions; `_sail_value_*` appears only when an anonymous Sail expression
requires statement-valued lowering and supplies no source binding name.

Sail enums with numeric representations use
`ethereum_types.enum.UintEnum`. Constructing the class converts from its
zero-based unsigned representation, and `.value` converts a member back—for
example, `evm.CallKind(2)` is `evm.CallKind.DelegateCall`, whose `.value` is
`2`. Non-numeric Sail enums use ordinary Python `Enum`. The extraction does
not expose Sail's compiler-generated numeric conversion helper functions.

## Validity witnesses

Dependent or runtime-bounded non-negative ranges use `Uint` values together
with generated validity witnesses. For example,
`RlpIndexCursorValidity` stores `maximum`, validates
`0 < maximum <= 2**20`, and is embedded in `RlpIndexCursor.validity`. The
cursor's `after` validator then checks that `count` and `position` lie between
zero and that same maximum. Both classes are strict Pydantic dataclasses, the
witness is frozen, and assigning an invalid cursor field raises `ValueError`
without changing the existing value. This transactional check covers direct
record-field and validity-witness assignment; in-place mutation inside a
mutable field remains ordinary Python.

Generated constructors propagate these witnesses. When a type parameter
cannot be recovered from a normal argument, it becomes an explicit generated
implicit parameter; internal calls supply the checked Sail type
instantiation. This retains the source model's dependent validity information
without coercing Python values or changing the optimized C extraction.

## Numeric boundary

The mandatory `ethereum-types==0.4.1` model is:

- Sail `int` uses Python `int`;
- Sail `nat` uses `ethereum_types.numeric.Uint`;
- exact unsigned ranges `0 .. 2**N - 1` use `U8`, `U16`, `U32`, `U64`, or
  `U256` for `N = 8, 16, 32, 64, 256`;
- every other constant non-negative `range(lo, hi)` uses a cached
  `BoundedUint[lo, hi]` subclass of `ethereum_types.numeric.Unsigned`;
- dependent or runtime-bounded non-negative ranges use `Uint` plus the
  Pydantic constraints described above;
- numeric Sail enums use `UintEnum`, while non-numeric enums use `Enum`;
- Sail `bits(N)` uses the exact-width modular `Bits` representation; and
- configured named byte aliases use the corresponding
  `ethereum_types.bytes.BytesN` class.

The extraction therefore preserves the EVM model's separation between numeric
words and byte-oriented hashes:

- `word`, the Sail range `0 .. 2**256 - 1`, is `U256`. Sail ALU helpers project
  operands to Python integers for their specified intermediate arithmetic,
  reduce at the explicit operation boundary, and reconstruct `U256`;
  bit-oriented helpers convert to `Bits(256)` locally;
- `address` is `ethereum_types.bytes.Bytes20`; and
- `b256` and `hash` are `ethereum_types.bytes.Bytes32`.

The mapping is selected from the named Sail aliases and checked against their
expanded 20-byte and 32-byte vector shapes. Because Sail abbreviations are
transparent, checked signatures that contain the expanded vector type receive
the same Python representation. Internal byte-oriented operations may still
use exact `Bits(8)` values; the explicit Sail `Address(...)` and `B256(...)`
functions construct the selected fixed-byte classes.

`adapter.word`, `adapter.address`, and `adapter.b256` make those host
boundaries explicit. The adapter reduces incoming word integers modulo
`2**256` and constructs `U256`; generated Sail functions trust their checked
signatures. Constrained record construction separately validates the
applicable dependent field invariants. The smoke test verifies `U256` word
results and the actual `ethereum-types` byte classes for addresses and hashes,
then exercises modular word addition and shifting, fixed-byte shapes,
word/hash conversion, strict validity witnesses, and assignment revalidation.

For example:

```sh
PYTHONPATH=extractions/python uv run --no-project \
  --with ethereum-types==0.4.1 \
  --with pydantic==2.12.5 python - <<'PY'
import adapter
import evm

assert adapter.word_to_int(evm.word_add_word(adapter.word(-1), adapter.word(1))) == 0
assert len(adapter.address(0x1234)) == 20
assert len(adapter.b256(0x1234)) == 32
assert adapter.b256_to_int(adapter.b256(0x1234)) == 0x1234
PY
```

## Host effects

Every defined Sail function with a value specification is exported as a typed
Python function, and pure helpers can be called immediately. Host-dependent
functions call the named functions in `evm.HostContract` directly; the package
contains no string-keyed `call_extern` path or runtime extern registry.

The maintained contract source is
`extractions/contracts/HostContract.py`. Extraction copies it into the
generated package with `--python-import-file` and links unresolved Sail extern
targets to it with `--python-extern-module evm.HostContract`. Its explicit
`HostState` owns memory frames, operand and continuation stacks, database and
cache overlays, checkpoint journals, access warmth, authorization records,
and logs. `get_state()`, `set_state()`, and `use_state()` let an embedding
select the state instance; package `reset()` installs a fresh host state
alongside resetting Sail registers.

Ordinary state operations are concrete Python functions over that state.
Only genuine cryptographic/native accelerators remain abstract, through the
typed `AcceleratorContract` methods stored on `HostState`. An embedding can
provide an implementation when it constructs the state; the default raises a
named unsupported-operation error. This follows the Lean extraction's
`HostAxioms.lean` split between an explicit semantic host state and a narrow
accelerator boundary.

The module also exports `reset()`, `finish()`, runtime value classes, and the
`__sail_types__`, `__sail_functions__`, `__sail_signatures__`,
`__sail_source_signatures__`, `__sail_effects__`, and `__sail_externs__`
introspection maps. See the Sail Python backend documentation for the generic
runtime contract and current limitations.
