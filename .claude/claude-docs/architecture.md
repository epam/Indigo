# Architecture

Cross-cutting design notes for the parts of the codebase that aren't obvious from a quick read of the source.

## Bingo test adapter pattern

`bingo/tests/dbc/` contains DB adapters — `PostgresSQL.py`, `OracleDB.py`, `BingoNoSQL.py`, `BingoElastic.py` — all inheriting from `base.SQLAdapter`. The root `conftest.py` reads the `--db` CLI option and injects the appropriate adapter as the `db` fixture. Each `test_*/conftest.py` handles setup/teardown for that function's test data.

Each adapter implements operation methods (`checkmolecule`, `aam`, `substructure`, `similarity`, etc.) that return a single value, a list, or an `Exception`. SQL templates use `{bingo_schema}` / `{test_schema}` / `{table_name}` placeholders filled by `_execute_query`.

Errors are extracted from DB exceptions by matching prefixes:

- Oracle: `ORA-`, `bingo:`, `(oracledb.exceptions.`
- Postgres: `bingo:`, `<class '`

This is what lets the same test body produce parity assertions across radically different backends.

## Bingo project layout

- `api/`, `core/` — Indigo C++ core library and its language bindings (Python, Java, .NET, R, WASM)
- `bingo/` — chemistry cartridge for PostgreSQL, Oracle, and MSSQL
- `bingo/bingo-elastic/` — Elasticsearch search APIs in Java and Python
- `utils/` — CLI tools (`indigo-cano`, `indigo-deco`, `indigo-depict`), `indigo-service` REST API, `chemdiff`

## Related

- [claude-docs/oracle.md](oracle.md) — Oracle-specific architecture notes (extproc, hand-coded config loader, connect hook)
- [claude-docs/testing.md](testing.md) — running the suites that exercise this adapter pattern
- [claude-docs/build.md](build.md) — how the components above are compiled
