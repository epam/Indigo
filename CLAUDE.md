# EPAM Indigo

Cheminformatics library suite: a C++ core (`core/`, `api/`) with Python/Java/.NET/R/WASM bindings, the **Bingo** chemistry cartridge for PostgreSQL/Oracle/MSSQL (`bingo/`), the Elasticsearch-backed **Bingo-Elastic** APIs (`bingo/bingo-elastic/`), and CLI/REST utilities (`utils/`). Cross-platform development is expected to happen inside `.devcontainer/` — native macOS toolchains drift fast.

## Quick Reference

| Topic        | File                                       | What's inside                                                              |
|--------------|--------------------------------------------|----------------------------------------------------------------------------|
| Build        | [.claude/claude-docs/build.md](.claude/claude-docs/build.md)             | CMake configure/build, targets, WASM, dev container                        |
| Testing      | [.claude/claude-docs/testing.md](.claude/claude-docs/testing.md)         | Indigo, Bingo (Postgres/Elastic/NoSQL), and indigo-service test workflows  |
| Oracle       | [.claude/claude-docs/oracle.md](.claude/claude-docs/oracle.md)           | Oracle Docker harness, host-venv setup, extproc/install/cx_Oracle gotchas  |
| Architecture | [.claude/claude-docs/architecture.md](.claude/claude-docs/architecture.md) | Bingo test adapter pattern, project layout                              |
| Conventions  | [.claude/claude-docs/conventions.md](.claude/claude-docs/conventions.md) | Python style and linting tools                                             |

## Key Rules

- **Adding an Oracle config tunable requires two edits, not one.** Insert into `bingo/oracle/sql/bingo/bingo_config.sql` AND add a `configGetIntDef` line in `BingoOracleContext::_loadConfigParameters` (`bingo/oracle/src/oracle/bingo_oracle_context.cpp`). Postgres iterates the table; Oracle's loader is hand-coded, so a row alone is silently ignored.
- **Bingo Oracle C++ has parallel `mango_*` (molecule) and `ringo_*` (reaction) paths.** Bugs often live in only one — when fixing or porting a function in `bingo/oracle/src/oracle/`, check both sides. See [.claude/claude-docs/oracle.md](.claude/claude-docs/oracle.md).
- **Bingo test adapters return errors as Exceptions, not raise.** When writing or modifying a method in `bingo/tests/dbc/*.py`, return the exception so cross-DB parity assertions still work — see [.claude/claude-docs/architecture.md](.claude/claude-docs/architecture.md).
- **Run `pytest` from `bingo/tests/`**, not from the repo root. `db_config.ini` is resolved relative to the working directory by `base.SQLAdapter`.

## See Also

- [.claude/claude-docs/oracle.md](.claude/claude-docs/oracle.md) before touching anything Oracle-related — there are several silent-failure traps (extproc path, missing grants, the `_init_bingo_context` commit).
- [.claude/claude-docs/architecture.md](.claude/claude-docs/architecture.md) before adding a new Bingo test or DB operation.
- [.claude/claude-docs/build.md](.claude/claude-docs/build.md) for any change that affects compilation or the dev container.
