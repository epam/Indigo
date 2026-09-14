# Invariants

> **Read when:** before changing C++ core data structures, the C API surface, or anything in `bingo/`.
> **Skip when:** the change is confined to docs, CI YAML, or a single Python/Java wrapper method.

Rules that must stay true. Each one is here because breaking it fails **silently** — no compiler
error, and often no failing test on the machine where the change was made.

Anchors were verified on `f0cc3c423` (master, 2026-09-03). An anchor that no longer resolves is a
defect in this file: fix it in the same change that moved the code.

---

## API & FFI

**A1 — No C++ exception may cross the C boundary.**
Every exported function in `api/c/` is wrapped in `INDIGO_BEGIN` / `INDIGO_END`
(`api/c/indigo/src/indigo_internal.h#define INDIGO_BEGIN`), returns `-1` on failure, and leaves
the message retrievable through `indigoGetLastError`. An unwrapped export terminates the host process instead of
returning an error to Python/Java/.NET.

**A2 — Exception message text is part of the public contract.**
Integration tests (`api/tests/integration/`) assert on error strings, and the wrappers surface them
verbatim to users. Rewording an existing `throw Error(...)` is an API change: it needs the same
scrutiny as a signature change, and the reference outputs must be regenerated deliberately, never
"to make the suite green".

**A3 — Loading a structure is not the same as validating it.**
The loaders accept structures the checkers reject, on purpose. Do not "fix" a checker complaint by
tightening a loader, and do not assume that a structure which loaded is chemically valid.

## Molecule model

**A4 — New per-atom or per-bond data must be wired into the whole lifecycle.**
The touch points are `clear()`, `_mergeWithSubmolecule_Sub()`, the `SKIP_*` copy flags
(`core/indigo-core/molecule/base_molecule.h#SKIP_CIS_TRANS`), `removeAtoms()` / `removeBonds()`
(`core/indigo-core/molecule/base_molecule.h#void removeAtoms(`), and both load and save paths.
Missing any one of them produces data that is correct on a freshly parsed molecule and wrong after
a copy, a merge, or an edit.

**A5 — A side table keyed by atom or bond index must be cleared in `removeAtoms` / `removeBonds`.**
The index pools reuse freed indices. A side table that is not cleared silently re-attaches its stale
entry to whatever object takes the recycled index next. This class of bug never crashes; it produces
a wrong molecule several operations later.

**A6 — New molecular functionality goes into a companion class, not into `BaseMolecule`.**
Follow the existing `MoleculeCisTrans` / `MoleculeStereocenters` shape. `BaseMolecule` is already a
god object; every field added to it is added to every molecule ever allocated.

**A7 — A collective endpoint is dropped whole or not at all.**
When atoms are removed, `AttachmentGroup::remapAtoms` returns `false` if any member is gone, and the
caller must then drop the entire group — never a truncated one
(`core/indigo-core/molecule/molecule_attachment_groups.h#remapAtoms`). Half a ligand is a different
chemical claim, not a smaller one, and a haptic bond pointing at a partial group is wrong in a way
nothing downstream can detect. The group's anchor atom is deliberately outside this rule: it is
remapped separately and simply becomes `-1`.

**A8 — Bond orders and query flags are named constants, tests included.**
`BOND_SINGLE`, `BOND_ZERO`, `_BOND_COORDINATION` and friends — never the underlying integers.

## Language & platform

**A9 — C++17, no C++20.**
`cmake/setup.cmake#CMAKE_CXX_STANDARD 17` sets the standard, and the Emscripten and
oldest-supported-GCC build legs reject C++20 constructs. The code must compile under GCC, Clang and MSVC.

**A10 — Layout output is not bit-identical across platforms.**
The 2D layout produces different coordinates on Windows and Linux for a measurable share of the
corpus — floating-point noise crossing epsilon comparisons, not a platform bug to be "fixed" by
widening a type. Never assert on exact coordinates in a cross-platform test; compare topology, or
pin the reference per platform.

## Bingo

**B1 — Adding an Oracle config tunable takes two edits, not one.**
Insert the row into `bingo/oracle/sql/bingo/bingo_config.sql` **and** add a `configGetIntDef` line in
`BingoOracleContext::_loadConfigParameters` (`bingo/oracle/src/oracle/bingo_oracle_context.cpp#configGetIntDef`).
Postgres iterates the table; the Oracle loader is hand-coded, so a row on its own is ignored without
any error.

**B2 — `mango_*` (molecule) and `ringo_*` (reaction) are parallel code paths.**
A bug or a fix in `bingo/oracle/src/oracle/` almost always applies to both. Check the twin before
declaring a fix complete — see [modules/bingo-oracle.md](./modules/bingo-oracle.md).

**B3 — Bingo test adapters return exceptions, they do not raise them.**
Methods in `bingo/tests/dbc/*.py` return the `Exception` object so that the same test body can assert
parity across engines with radically different error reporting. Raising instead of returning breaks
every cross-DB comparison — see [testing.md](./testing.md).

**B4 — Run `pytest` from `bingo/tests/`, not from the repository root.**
`base.SQLAdapter` resolves `db_config.ini` relative to the working directory.
