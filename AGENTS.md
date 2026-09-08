# EPAM Indigo — instructions for AI agents

Cheminformatics toolkit: a C++17 core (`core/`, `api/`) with Python/Java/.NET/R/WASM bindings, the
**Bingo** chemistry cartridge for PostgreSQL/Oracle/MSSQL (`bingo/`), the Elasticsearch-backed
**Bingo-Elastic** APIs (`bingo/bingo-elastic/`), and CLI/REST utilities (`utils/`).

This file is read by every session, so it holds only what is needed **before** knowing the task. The
knowledge itself is in `.memory-bank/`, one file per subject, read on demand. Open the one row that
matches; reading the whole bank up front is the failure this layout exists to prevent.

## Read before you edit

- `core/`, `api/c/`, `api/cpp/`, `bingo/` — [.memory-bank/invariants.md](.memory-bank/invariants.md).
  **Read this one.** Everything in it breaks silently: no compiler error, and usually no failing test
  on the machine where the change was made.
- unfamiliar chemistry vocabulary — [.memory-bank/domain.md](.memory-bank/domain.md), then
  [.memory-bank/glossary.md](.memory-bank/glossary.md)
- where a component lives, how data flows — [.memory-bank/architecture.md](.memory-bank/architecture.md)
- compiling, CMake options, WASM, devcontainer — [.memory-bank/build.md](.memory-bank/build.md)
- running any test suite — [.memory-bank/testing.md](.memory-bank/testing.md)
- code style, and what the CI gate actually checks — [.memory-bank/conventions.md](.memory-bank/conventions.md)
- a subsystem in depth — [.memory-bank/modules/README.md](.memory-bank/modules/README.md) is the index
- what a capability promises callers — [.memory-bank/features/README.md](.memory-bank/features/README.md)
- why a structural decision was made — [.memory-bank/adr/README.md](.memory-bank/adr/README.md)
- writing into the bank — [.memory-bank/README.md](.memory-bank/README.md) for the formats and rules

Per-language conventions live in `.claude/rules/` — one file per area, each scoped to the paths it
governs. Agents that load them by path get them automatically; agents that do not should read the
file matching the code they are about to touch.

## Rules of the repository

- **Commit subject:** `#<ticket>: <Description>` — the ticket number first, so the origin is visible
  in `git log`. Conventional-commit prefixes are not used here; check `git log --format=%s -5`
  before writing one.
- **Branch name:** `<ticket>-<short-description>`, no `feature/` or `fix/` prefix.
- **Verify before asserting.** Read exit codes honestly: a suite that was not run is reported as not
  run, never as passing.
- **Say what you did not do.** A change that covers four of five wrappers, or skips a platform, is
  reported that way rather than left to be discovered.

Everything else that governs code — the language standard, the FFI contract, what may be commented,
what must never be broken — is in the two places above, next to the code it applies to, so that it
is corrected when the code changes rather than drifting here.

## Keeping this current

When a change teaches something lasting, record it in the same commit: a new silent-failure rule
goes to `invariants.md`, subsystem knowledge to `modules/`, behaviour to `features/`, a structural
decision to `adr/`.

Anchors into code are written as `` `<path>#<Symbol>` `` — never a line number, which goes
stale silently on the next edit above it. `.claude/scripts/check-anchors.sh` verifies them all.
