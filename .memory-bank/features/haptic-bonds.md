# Haptic bonds

> **Read when:** a ticket mentions haptic or hapto bonds, attachment groups, or a bond whose end is
> a set of atoms rather than one atom.
> **Skip when:** the work is about ordinary two-atom bonds.

Verified on `f0cc3c423`. Ticket family #3233.

## Problem

In organometallic chemistry a ligand binds a metal through several of its atoms at once — the
η⁵-cyclopentadienyl ring bonds through all five carbons, not through any one of them. A model in
which every bond connects exactly two atoms cannot express that, and drawing five separate bonds
says something chemically different.

## Interface

A **haptic bond** connects two endpoints, and an endpoint is either an atom or an **attachment
group** — a named set of atoms acting as one collective end
(`HapticBond::Endpoint::atom(idx)` / `Endpoint::group(idx)`).

- `BaseMolecule::addHapticBond(begin, end, type = _BOND_HAPTIC)` is the **only** way to create one
  (`base_molecule.h:474`).
- `BaseMolecule::removeAttachmentGroup(idx)` is the only way to remove a group
  (`base_molecule.h:479`); the haptic bonds addressing it are removed with it.
- The data lives in `attachment_groups` and `haptic_bonds` on `BaseMolecule`
  (`base_molecule.h:572`, `:574`), not on the atoms.
- In KET, a connection is discriminated by type `"haptic"`, with `attachmentGroups` and
  `attachmentGroupId` carrying the sets (`molecule/ket_keys.h`).
- In MOL V3000, one bond record carries the group as an `ENDPTS` list with `ATTACH`; the record's
  visible end is the group's **anchor atom** — the star atom of the `ENDPTS` record. A group's
  anchor is `-1` when the source named none, as KET does; it is not a member of the group and not
  part of it chemically (`molecule_attachment_groups.h:69-73`).

## Expected behaviour

**When** a KET document containing haptic connections and attachment groups is loaded, **then** the
groups and bonds are reconstructed, and each bond's endpoints resolve to the atoms or groups named
in the document.

**When** such a molecule is saved back to KET, **then** the connections and groups round-trip.

**When** a MOL V3000 file with an `ENDPTS` / `ATTACH` bond record is loaded, **then** an attachment
group is created from the endpoint list, its anchor is set to the record's star atom, and one haptic
bond is added from the group to the atom at the other end
(`molfile_loader_v3000.cpp:877` `_addHapticBond3000`).

**When** a molecule with haptic bonds is saved to MOL V3000, **then** each group-to-atom bond becomes
one record. If the group has no usable anchor, the saver emits a fresh star atom placed at the
**centre of the member atoms' bounding box** (`MolfileSaver::_attachmentGroupCentre`), so the file
has something to draw the bond to.

**When** a haptic bond connects two atoms rather than a group and an atom, **then** V3000 **drops
it** — the format has no `ENDPTS` form for that case, and it is discarded like any other feature
V3000 cannot express (`molfile_saver.cpp:465`). A structure like that does not survive the round
trip, and nothing warns about it.

**When** an attachment group is removed, **then** every haptic bond referring to it is removed too —
a haptic bond never survives with a dangling endpoint.

**When** atoms belonging to a group are removed from the molecule, **then** the group's membership is
remapped through `MoleculeAttachmentGroups::onAtomsRemoved` — and if **any** member is gone the
whole group is dropped. A group is never kept in a truncated form: half a ligand is a different
chemical claim, not a smaller one. The anchor is the exception — it is remapped separately and
simply becomes `-1` when its atom disappears, which never forces the group out.

**When** a molecule is copied or merged with `SKIP_ATTACHMENT_GROUPS` or `SKIP_HAPTIC_BONDS`
(`base_molecule.h:137-138`), **then** that data is deliberately left behind; without the flags it is
remapped into the target.

**When** the structure is rendered, **then** the haptic bond is drawn to the group as a whole —
`core/render2d/` knows about attachment groups.

## Guarantees

- **An attachment group is not a vertex of the molecular graph.** It never appears in
  `vertexBegin()`/`vertexNext()`, so every algorithm that walks the graph — valence, aromaticity,
  canonical SMILES, InChI, fingerprints, substructure search — is unaffected by its presence. See
  [../adr/haptic-bond-endpoints.md](../adr/haptic-bond-endpoints.md).
- **A group has no stored position.** Anything needing one derives it from the member atoms, so it
  cannot go stale when the ligand moves. The anchor atom is not an exception: it is a reference to
  an atom a file drew the group with, not a coordinate, and a group whose anchor is gone is still a
  valid group.
- Endpoints are validated on creation (`_checkHapticEndpoint`, `base_molecule.h:790`); an endpoint
  naming a nonexistent atom or group is rejected rather than stored.

## Limitations

At `f0cc3c423` the feature is model, KET, MOL V3000 and rendering:

- **Atom-to-atom haptic bonds are lost in V3000.** Silently — see above.
- **No C API functions**, and therefore nothing in the Python, Java, .NET, R or WASM wrappers: the
  feature is reachable from C++ only. (In progress as a separate ticket in the family.)
- **Layout does not place attachment groups** — `core/indigo-core/layout/` has no knowledge of them,
  so a structure laid out from scratch does not position the group sensibly.
- Charge and radical state on a group are deliberately out of scope, tracked separately.

Coverage lives in `core/indigo-core/tests/tests/` — `attachment_groups.cpp`, `haptic_bonds.cpp`,
`haptic_bonds_ket.cpp`, `haptic_molfile.cpp` — plus `api/cpp/tests/rendering/haptic.cpp`.
