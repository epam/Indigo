# Invariants

> **Read when:** before changing C++ core data structures, the C API surface, or anything in `bingo/`.
> **Skip when:** the change is confined to docs, CI YAML, or a single Python/Java wrapper method.

Rules that must stay true. Each one is here because breaking it fails **silently** — no compiler
error, and often no failing test on the machine where the change was made.

Anchors were verified on `f0cc3c423` (master, 2026-09-03); A12 to A14 on `8efe581f0` (2026-10-27); B5 on
2026-10-23. An anchor that no longer resolves is a defect in this file: fix it in the same change that moved
the code.

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

**A7a — A component number belongs to the decomposition that produced it.**
A molecule has three. `Graph::countComponents()` follows edges only;
`BaseMolecule::moleculeComponents()` also follows haptic bonds
(`core/indigo-core/molecule/base_molecule.h#moleculeComponents`);
`Graph::countComponents(external_neighbors)` follows whatever sets it is given. Graph algorithms use
the first; the public component API, `fragmentedSdf`, `standardize` and the salt helpers the second;
the KET saver and the reaction loaders the third. A number taken from one and used with another
picks the wrong atoms — on a haptic structure always, on an ordinary molecule never, so tests on
ordinary molecules miss it. See
[adr/public-components-follow-haptic-bonds.md](adr/public-components-follow-haptic-bonds.md).

**A7b — A change of connectivity has to reach `BaseMolecule::changed()`.**
`moleculeComponents()` is computed once and kept until `changed()` drops it
(`core/indigo-core/molecule/src/base_molecule.cpp#BaseMolecule::changed`), and a reference to it is
good for as long. `Graph` calls `changed()` from `addVertex`, `addEdge`, `removeEdge`,
`removeAllEdges`, `removeVertex` and `clear`. Whatever changes connectivity another way calls it
itself: the haptic edits through `_hapticConnectivityChanged()`, and `flipBondWithDirection`, which
rewires an edge in place. Two paths report nothing of their own and lean on a vertex added earlier in
the same call: `Graph::_cloneGraph_KeepIndices`, which installs the edges directly, and
`_mergeWithSubmolecule_Sub`, which copies the haptic bonds after the atoms. A path that skips the
call leaves the previous answer standing, and nothing fails.

**A8 — Bond orders and query flags are named constants, tests included.**
`BOND_SINGLE`, `BOND_ZERO`, `_BOND_COORDINATION` and friends — never the underlying integers.

## Formats

**A11 — A V2000 S-group property line is read whole before it is parsed.**
`M  SDT`, `SDD`, `SED`/`SCD`, `SMT` and `SCL` take their line with
`core/indigo-core/molecule/src/molfile_loader_v2000.cpp#_readSGroupLine`, which also reads the S-group
index from that buffer and rejects one beyond the S-groups declared by `M  STY`, then parse the rest.
A fixed `skip` or `readIntFix` on the whole-file scanner, the index included, runs into the next
record when a line ends early: the next line is silently consumed and the next S-group loses its atoms
or its data, while files Indigo writes itself never trigger it (#3959). `M  SCN` and `M  SNC`, which list
several S-groups with a value each, take their line with
`core/indigo-core/molecule/src/molfile_loader_v2000.cpp#_readSGroupEntryLine`, which checks every index the
same way (#3953). `SBV`, `SDS`, `SPA` and `SAP` still parse positionally — a new handler, or a change to one
of these, should read the line first.

## S-group brackets

**A12 — The two brackets of a bracketed S-group keep their order and direction: the opening one first, running
bottom to top, the closing one second, running top to bottom.**
The renderer turns the hooks of a bracket to the right of its direction and puts the label and the repeat pattern
at the last bracket (`core/render2d/src/render_internal.cpp#_positionIndex`). A pair written the other way round
draws its hooks outwards and its label at the left bracket; nothing fails. The one place that
writes a pair is `core/indigo-core/layout/src/sgroup_bracket_layout.cpp#_setBrackets`.

**A13 — Whatever moves the atoms of a molecule moves its S-group brackets with them.**
Brackets are coordinates stored on the group; nothing ties them to the atoms. A rigid move or a scale goes
through `BaseMolecule::offsetCoordinates`, `BaseMolecule::scale` or `Metalayout::adjustMol`, which carry the
brackets along (`core/indigo-core/molecule/molecule_sgroups.h#transformBrackets`); a layout that moves atoms
one by one constructs `SGroupBracketLayout` before the move and calls `update()` after it
(`core/indigo-core/layout/sgroup_bracket_layout.h#void update()`), which places the brackets of the groups whose
atoms moved anew. A mover that does neither leaves the brackets where the atoms were, and only a drawing shows
it. Known gaps: `indigoAlignAtoms`, the centering of the standardizer, the unfolding of hydrogens.

**A14 — A zero-length bracket is a placeholder, not a bracket.**
The KET loader writes a `(0,0)-(0,0)` pair for every group, because the format carries no bracket coordinates.
`SGroup::hasBrackets()` is the test (`core/indigo-core/molecule/molecule_sgroups.h#hasBrackets`): one zero-length
bracket means the group has none. The MOL savers skip such pairs
(`core/indigo-core/molecule/src/molfile_saver.cpp#hasBrackets`), `transformBrackets` leaves them, and the layout
replaces them. Code that tests `brackets.size() > 0` instead takes a bracket at the origin for a real one; the
renderer still checks only the first bracket of a group.

## Language & platform

**A9 — C++17, no C++20.**
`cmake/setup.cmake#CMAKE_CXX_STANDARD 17` sets the standard, and the Emscripten and
oldest-supported-GCC build legs reject C++20 constructs. The code must compile under GCC, Clang and MSVC.

**A10 — Layout calls the elementary functions on double, and that is what keeps it portable.**
The layout chooses between equal alternatives with float comparisons, so the last bit of a
coordinate decides the drawing. For a float argument MSVC calls `cosf`, `sinf`, `atan2f`, `hypotf`
and GCC the double functions; with float calls Windows and Linux disagreed on 11 % of a corpus of
18 237 molecules. Every such call in the layout and in `Vec2f` goes through one of the macros
`HYPOT`, `COS`, `SIN`, `TAN`, `ACOS`, `ASIN`, `ATAN2` of `common/math/algebra.h`, which evaluate in
double and round once, and the two platforms now agree bit for bit on all of them. Write the
promotion out by hand and the rule has no single home; write the call on a float and the divergence
comes back. `tests/tests/layout_math.cpp` sweeps the arguments on which the two spellings differ and
pins every macro, both rotations of `Vec2f` and both tilt angles to the double value. The square
root is left alone: IEEE 754 rounds it exactly. Widening the layout energy to double does not
help and was measured not to. The guarantee covers x86-64 only: a compiler that fuses `a * b + c`
into one FMA instruction — the default on ARM, including macOS — still produces other bits, so a
macOS reference may still be needed.

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

**B5 — A core header the Postgres cartridge reaches must not include `<functional>`.**
The cartridge includes `postgres.h` before the Indigo headers, and on Windows `postgres.h` defines `bind` as a
function-like macro, which breaks the declaration of `std::bind` in MSVC's `<functional>`
(`bingo/postgres/src/pg_common/bingo_pg_fix_pre.h#Thus <functional> must not be included here`). Only the Windows
build of the cartridge fails, in CI; macOS and Linux compile it. `base_molecule.h` and everything it includes are on
that path, so a callback stored in such a header is a template parameter, not a `std::function`
(`core/indigo-core/molecule/molecule_sgroups.h#transformBrackets`).
