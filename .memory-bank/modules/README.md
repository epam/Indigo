# Modules

> **Read when:** you need the index of subsystem deep-dives, or you are about to write one.
> **Skip when:** you already know which module file you need — open it directly.

One file per subsystem. A module document exists to save an agent a day of reading; a module
document that restates the header file saves nothing and should not be written.

## Format

- **Responsibility** — what this subsystem owns, and what it deliberately does not.
- **Public interface** — the entry points other code is allowed to use.
- **Dependencies / Dependents** — who it calls, who calls it.
- **Constraints & traps** — the silent-failure list. This is the part that earns the file.

## Documented

- [bingo-oracle.md](./bingo-oracle.md) — Oracle cartridge: Docker harness, host venv, extproc path,
  the hand-coded config loader, `mango_*`/`ringo_*` parity, install and `cx_Oracle` gotchas
- [bingo-elastic-python.md](./bingo-elastic-python.md) — standalone Python library: API surface,
  fingerprint pre-filter + postprocess pattern, its own test suite

## Not yet written

Listed so that their absence is visible rather than mistaken for "nothing to know here". Write one
when a task forces you to reconstruct the knowledge anyway.

- `indigo-core-graph` — the graph layer, index pools, automorphism and canonical ranking
- `molecule-loaders` — the loader/saver matrix, shared options plumbing, error contracts
- `layout` — the 2D layout pipeline and its cross-platform reproducibility limits (invariant A9)
- `render2d` — rendering pipeline, cairo backend, font and platform dependencies
- `c-api` — the FFI surface, session/object registry, `INDIGO_BEGIN`/`INDIGO_END` machinery
- `wrappers` — how the Python/Java/.NET/WASM bindings are generated and kept in step
- `indigo-service` — the FastAPI REST service and its Docker packaging
- `bingo-postgres` — the Postgres cartridge and its differences from Oracle
