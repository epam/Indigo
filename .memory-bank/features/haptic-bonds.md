# Haptic bonds

> **Read when:** a ticket mentions haptic or hapto bonds, attachment groups, or a bond whose end is
> a set of atoms rather than one atom.
> **Skip when:** the work is about ordinary two-atom bonds.

Verified on `f0cc3c423`; the charge and the radical of a group (#3923) on master `55ce330fa` with
that ticket applied. Ticket family #3233.

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
  (`core/indigo-core/molecule/base_molecule.h#addHapticBond`).
- `BaseMolecule::removeAttachmentGroup(idx)` is the only way to remove a group
  (`core/indigo-core/molecule/base_molecule.h#removeAttachmentGroup`); the haptic bonds addressing
  it are removed with it.
- The data lives in `attachment_groups` and `haptic_bonds` on `BaseMolecule`
  (`core/indigo-core/molecule/base_molecule.h#MoleculeAttachmentGroups attachment_groups`), not on
  the atoms.
- A group has a **charge** and a **radical** of its own — those of the pi-system as a whole, not of
  any member atom (`core/indigo-core/molecule/molecule_attachment_groups.h#setRadical`). The radical
  uses the atom encoding, 0 to 3, and nothing else is accepted.
- In KET, a connection is discriminated by type `"haptic"`, with `attachmentGroups` and
  `attachmentGroupId` carrying the sets (`molecule/ket_keys.h`). A group object may carry `charge`
  and `radical` (`core/indigo-core/molecule/ket_keys.h#KetKeyCharge`); a value of 0 is not written.
- In MOL V3000, one bond record carries the group as an `ENDPTS` list with `ATTACH`. The `*` atom
  that ends a haptic record is the format's phantom for the centre of the pi-system: it is not kept
  as an atom, and its `CHG` and `RAD` are the group's. A group keeps an **anchor atom** only where
  the record ends on a real atom — the star of a variable attachment (`ATTACH=ANY`) or an ordinary
  atom; otherwise the anchor is `-1`, as for a group from KET. The anchor is not a member of the
  group and not part of it chemically
  (`core/indigo-core/molecule/molecule_attachment_groups.h#anchorAtom`).
- In CDX and CDXML, the group is a `MultiAttachment` node, which carries the charge and the radical
  as its `Charge` and `Radical`; a `VariableAttachment` node has the same shape and hands over
  neither (`core/indigo-core/molecule/src/molecule_cdxml_loader.cpp#_addAttachmentGroups`).

## Expected behaviour

**When** a KET document containing haptic connections and attachment groups is loaded, **then** the
groups and bonds are reconstructed, and each bond's endpoints resolve to the atoms or groups named
in the document.

**When** such a molecule is saved back to KET, **then** the connections and groups round-trip.

**When** a MOL V3000 file with an `ENDPTS` / `ATTACH` bond record is loaded, **then** an attachment
group is created from the endpoint list and one haptic bond is added from the group to the atom at
the other end (`core/indigo-core/molecule/src/molfile_loader_v3000.cpp#_addHapticBond3000`). The
star of a haptic record leaves the structure and its `CHG` and `RAD` go to the group; two records
ending in one star share one group, which takes the charge once. A `RAD` the format does not define
is read as no radical — the file still loads.

**When** a molecule with haptic bonds is saved to MOL V3000, **then** each group-to-atom bond becomes
one record. If the group has no usable anchor, the saver emits a fresh star atom placed at the
**centroid of the member atoms** (`core/indigo-core/molecule/base_molecule.h#attachmentGroupCentre`),
so the file has something to draw the bond to, and that star carries the charge and the radical of
the group.

**When** a group with a charge or a radical has no such star to carry them — its record ends on an
atom of the structure, or it is a variable attachment, whose star is an atom with a charge of its
own — **then** saving to V3000 fails rather than dropping them
(`core/indigo-core/molecule/src/molfile_saver.cpp#which V3000 can keep only on the star`). CDX and
CDXML refuse a charged variable attachment group the same way.

**When** a KET group object has a `charge` or a `radical` that is not an integer, or a radical
outside 0–3, **then** loading fails with an error naming the group and the key.

**When** a haptic bond connects two atoms rather than a group and an atom, **then** V3000 **drops
it** — the format has no `ENDPTS` form for that case, and it is discarded like any other feature
V3000 cannot express (`core/indigo-core/molecule/src/molfile_saver.cpp#no ENDPTS form`). A
structure like that does not survive the round trip, and nothing warns about it.

**When** an attachment group is removed, **then** every haptic bond referring to it is removed too —
a haptic bond never survives with a dangling endpoint.

**When** atoms belonging to a group are removed from the molecule, **then** the group's membership is
remapped through `MoleculeAttachmentGroups::onAtomsRemoved` — and if **any** member is gone the
whole group is dropped, and its charge and radical with it. A group is never kept in a truncated
form: half a ligand is a different chemical claim, not a smaller one. The anchor is the exception — it is remapped separately and
simply becomes `-1` when its atom disappears, which never forces the group out.

**When** a molecule is copied or merged with `SKIP_ATTACHMENT_GROUPS` or `SKIP_HAPTIC_BONDS`
(`core/indigo-core/molecule/base_molecule.h#SKIP_ATTACHMENT_GROUPS`), **then** that data is
deliberately left behind; without the flags it is remapped into the target.

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
- **The charge and the radical of a group belong to no atom.** Nothing moves them onto a member
  atom, and what is computed from atoms — the gross formula, the mass, implicit hydrogens, the
  valence check — does not see them.
- Endpoints are validated on creation
  (`core/indigo-core/molecule/base_molecule.h#_checkHapticEndpoint`); an endpoint naming a
  nonexistent atom or group is rejected rather than stored.

## Limitations

- **Atom-to-atom haptic bonds are lost in V3000.** Silently — see above.
- **`molfile-saving-mode=auto` writes V2000**, which has no `ENDPTS`: the haptic bonds, the groups
  and the charges of the groups are all left out, silently
  (`core/indigo-core/molecule/src/molfile_saver.cpp#save to v3000 molfile only`). A round trip
  needs mode `3000`.
- **A group that no haptic bond refers to is not written to V3000** — there is no record to hang it
  on — and its charge goes with it. KET and CDXML keep such a group.
- **The net charge the structure checker reports counts atoms only**
  (`core/indigo-core/molecule/src/structure_checker.cpp#check_charge`): a neutral complex whose
  ligand charges sit on the groups is reported as charged.
- The charge of a group is not range-checked, as an atom's is not: V3000 writes whatever it holds,
  and binary CDX keeps one byte of it.
- Binary serialization does not carry attachment groups.
- SMILES, SMARTS, InChI and CML have no collective endpoint and carry neither the groups nor their
  charge and radical.
- The C API and the wrappers reach groups and haptic bonds
  (`api/c/indigo/indigo.h#indigoAddAttachmentGroup`), but not yet the charge and the radical of a
  group — a separate ticket in the family.

Coverage lives in `core/indigo-core/tests/tests/` — `attachment_groups.cpp`, `haptic_bonds.cpp`,
`haptic_bonds_ket.cpp`, `haptic_molfile.cpp`, `haptic_cdxml.cpp` — plus
`api/cpp/tests/rendering/haptic.cpp`.
