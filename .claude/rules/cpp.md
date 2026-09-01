---
paths:
  - "core/**/*.{cpp,h,inc}"
  - "api/c/**/*.{cpp,h}"
  - "api/cpp/**/*.{cpp,h}"
  - "bingo/**/*.{cpp,h}"
description: "C++ standards for the Indigo core and native API: portability, memory safety, FFI, naming"
---

# C++ rules

Silent-failure rules live in [.memory-bank/invariants.md](../../.memory-bank/invariants.md) — read
those before changing molecule data structures or the C API. This file is the everyday style contract.

## Language and portability

- **C++17.** `cmake/setup.cmake` sets the standard; `std::optional`, `string_view`, structured
  bindings and `if constexpr` are available, C++20 constructs are not.
- The same source compiles under GCC, Clang, MSVC and Emscripten. Guard platform code with
  `#ifdef _WIN32` / `__APPLE__` / `__linux__` / `EMSCRIPTEN`; under Emscripten there is no `pthread`
  and no `cairo`.
- OS-dependent behaviour goes through the abstractions in `core/indigo-core/common/base_c/` rather
  than being written inline a second time.

## Memory and ownership

- RAII, always: every resource has one owner and a destructor that releases it.
- No bare `new` / `delete`. Use the project's containers and pools, or smart pointers.
- Ownership is stated in the type, not in a comment. A raw pointer means "borrowed, outlives me".

## FFI boundary (`api/c/`)

- Every export is wrapped in `INDIGO_BEGIN` / `INDIGO_END`, returns `-1` on failure, and never lets
  an exception escape.
- Throw the project's `Exception` subclasses inside; the message is user-visible and test-visible,
  so treat its wording as an interface (invariant A2).

## Naming

| Kind | Form | Example |
| --- | --- | --- |
| Class | `CamelCase` | `MoleculeSubstructureMatcher` |
| Method | `camelCase` | `countAtoms()` |
| Member | `_prefixed` | `_atomCount` |
| Constant | `UPPER_CASE` | `MAX_ATOMS` |
| File | `snake_case` | `molecule_cis_trans.cpp` |

## Design

- New molecular functionality goes into a companion class (the `MoleculeCisTrans` shape), not into
  `BaseMolecule`.
- No magic numbers for bond orders or query flags — named constants, in tests too.
- A pattern is introduced on the **second** occurrence, not in anticipation of one. Reuse the
  pattern the codebase already has rather than adding a second way to do the same thing.
- Comments are paid for: an invariant a type cannot express, a pointer to an external source of
  truth, a trap a test confirms, a short file header. A comment saying "keep X in sync with Y" is a
  defect report — express the invariant in code instead.

## Performance

- Graph traversal uses `vertexBegin()` / `vertexNext()`; an O(V²) scan where an O(V) walk exists is
  a review comment, not a preference.
- Fingerprints are bit operations, not element-by-element loops.
- Large SDF input streams; it is never loaded whole.
- State the performance effect of any algorithmic change in the commit message.
