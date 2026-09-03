# EPAM Indigo

Cheminformatics toolkit: a C++17 core (`core/`, `api/`) with Python/Java/.NET/R/WASM bindings, the
**Bingo** chemistry cartridge for PostgreSQL/Oracle/MSSQL (`bingo/`), the Elasticsearch-backed
**Bingo-Elastic** APIs (`bingo/bingo-elastic/`), and CLI/REST utilities (`utils/`).

This file is an index, not a briefing. It is loaded into every session, so it stays small; the
knowledge lives in `.memory-bank/` and is read on demand.

## Read before you edit

- `core/`, `api/c/`, `api/cpp/`, `bingo/` — [.memory-bank/invariants.md](.memory-bank/invariants.md), the silent-failure rules
- unfamiliar chemistry vocabulary — [.memory-bank/domain.md](.memory-bank/domain.md), then [.memory-bank/glossary.md](.memory-bank/glossary.md)
- where a component lives, how data flows — [.memory-bank/architecture.md](.memory-bank/architecture.md)
- compiling, CMake options, WASM, devcontainer — [.memory-bank/build.md](.memory-bank/build.md)
- running any test suite — [.memory-bank/testing.md](.memory-bank/testing.md)
- code style, and what the CI gate actually checks — [.memory-bank/conventions.md](.memory-bank/conventions.md)
- `bingo/oracle/` — [.memory-bank/modules/bingo-oracle.md](.memory-bank/modules/bingo-oracle.md)
- `bingo/bingo-elastic/python/` — [.memory-bank/modules/bingo-elastic-python.md](.memory-bank/modules/bingo-elastic-python.md)
- writing into `.memory-bank/` itself — [.memory-bank/README.md](.memory-bank/README.md), formats and rules

Open the one row that matches. Reading the whole bank up front is the failure mode this layout
exists to prevent.

## Rules of the repository

- **C++17, portable.** GCC, Clang, MSVC and Emscripten all build this code; no C++20.
- **Never let a C++ exception cross `api/c/`.** `INDIGO_BEGIN` / `INDIGO_END`, return `-1`.
- **Error message text is a public contract** — tests and users read it verbatim.
- **New per-atom or per-bond data must be wired into the full lifecycle** (`clear`, merge, `SKIP_*`,
  `removeAtoms`/`removeBonds`, load, save). Half-wired side data fails silently, not loudly.
- **Comments are paid for.** Acceptable: an invariant a type cannot express, a reference to an
  external source of truth (BIOVIA §, OpenSMILES, KET contract), a trap confirmed by a test, a 1–3
  line file header. Rationale and rejected alternatives belong in the commit message.
- **Commit subject:** `#<ticket>: <Description>` — the ticket number first. Conventional-commit
  prefixes are not used here; check `git log --format=%s -5` before writing one.
- **Branch name:** `<ticket>-<short-description>`, no `feature/` or `fix/` prefix.

## Keeping this current

When a change teaches you something lasting, record it in the same commit: a new silent-failure rule
goes to `invariants.md`, subsystem knowledge to `modules/`, behaviour to `features/`, a structural
decision to `adr/`. Anchors (`path/file.cpp:NNN`) are part of the claim — when code moves, the
anchor moves with it.
