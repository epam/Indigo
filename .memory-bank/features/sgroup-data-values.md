# Values of data S-groups after layout

> **Read when:** a ticket is about where the value of a data S-group (a field and its data, such as a weight percent
> or a note) stands after layout or clean-up.
> **Skip when:** the question is about brackets, which is [sgroup-brackets.md](./sgroup-brackets.md), or about which
> data fields a format keeps.

Verified on `1f71959a5`. Ticket #3975, under the polymer epic #3971.

## Problem

A data S-group can store where its value is drawn: at a point of the sheet (absolute) or at an offset from its
atoms (relative). Before this change `layout()`, and the placing of molecules in a reaction scheme, moved an absolute
value only by the shift of the centre of its atoms, without the turn and without the scale, and left a relative value
and a value without atoms of its own in place; `clean2d()` and the placing of molecules in a pathway scheme left every
value where the atoms had been.

## Interface

Nothing to call. Applies to `layout()` and `clean2d()` of a molecule, a selection, a reaction or a pathway, and to
loading a pathway, which lays it out. The position is read from and saved to MOL V2000 (`M  SDD`) and V3000
(`FIELDDISP`).

## Expected behaviour

**When** `layout()` or `clean2d()` moves the atoms of a data S-group, **then** its value turns and moves with them:
it stays on the same side of them, at the same distance. It is not scaled and not mirrored.

**When** the group has no atoms of its own, **then** an absolute value follows the atoms of its parent group, such as
the component it describes, and a relative one stays as it is: the renderer draws it from a corner of a box around
the structure, which moves by itself. **When** the group has neither atoms nor a parent with atoms, **then** the value
stays where it was.

**When** the atoms of the group do not move, as in a second `layout()` or a clean-up of a selection elsewhere,
**then** the value stays exactly where it was.

**When** the value is relative and the group has atoms, **then** it goes where the same value stored as absolute
would go, and stays an offset from the first atom of the group.

**When** a reaction or a pathway is laid out, or a pathway is loaded, **then** a value moves and scales with its
molecule as the molecule is placed in the scheme, also with `layout-preserve-existing`.

**When** a value has no stored position, **then** nothing changes: the renderer draws it next to the group.

## Guarantees

Deterministic for given coordinates. The three structures of the ticket, a reaction drawn at bond length 1.5, laid
out as drawn and anew, a pathway and a clean-up of a selection are pinned to four decimals.

## Limitations

- A value can end up outside the brackets of its parent: it keeps its distance from its own atoms, while the
  brackets are placed tight around them.
- `layout()` brings a drawing to bond length 1.0 but keeps the distance of a value from its atoms in coordinates, so
  a structure drawn with longer bonds gets its values closer to the atoms, measured in bonds.
- After `clean2d()` of a strongly distorted drawing a value can stand a fraction of a bond length away from where a
  single best turn and shift would put it.
- When the layout mirrors a symmetric group, such as a regular ring, the value is only shifted with its atoms: no
  turn fits such a group better than another.
- Rigid moves outside the layout (aligning atoms, the standardizer's centering, unfolding hydrogens) leave values
  behind, as they leave brackets.
- Ketcher (RC on Indigo 1.48.0-rc.1, checked on 2026-11-02) sends no value positions to Layout and Clean Up, so it
  places the values itself and does not show this behaviour; the positions from Ketcher are #3973.
